/*****************************************************************************
 *
 * dPVS
 * -----------------------------------------
 *
 * (C) 1999-2004 Hybrid Graphics, Ltd.
 * All Rights Reserved.
 *
 * This file consists of unpublished, proprietary source code of
 * Hybrid Graphics, and is considered Confidential Information for
 * purposes of non-disclosure agreement. Disclosure outside the terms
 * outlined in signed agreement may result in irreparable harm to
 * Hybrid Graphics and legal action against the party in breach.
 *
 * Description:     New silhouette cache code
 *
 * Notes:           The file contains the "heart" of the silhouette cache,
 *                  the silhouette extraction and incremental silhouette
 *                  generation code. 
 *
 * $Archive: /dpvs/implementation/sources/dpvsSilhouetteCache.cpp $
 * $Author: wili $ 
 * $Revision: #2 $
 * $Modtime: 19.06.01 20:28 $
 * $Date: 2004/02/06 $
 * 
 ******************************************************************************/

#include "dpvsSilhouetteCache.hpp"
#include "dpvsAABB.hpp"
#include "dpvsMath.hpp"
#include "dpvsArray.hpp"
#include "dpvsIntersect.hpp"
#include "dpvsSort.hpp"
#include "dpvsRandom.hpp"
#include "dpvsSet.hpp"
#include "dpvsStatistics.hpp"
#include "dpvsTempArray.hpp"
#include "dpvsSystem.hpp"

using namespace DPVS;

/*
DEBUG DEBUG TODO:

- speed up the Node searching (use some kind of a tree structure)
- make some "maximum distance" decision initially (in the beginning of the extraction),
  as this would make the most expensive parts of the extraction (sorting) perform less
  work.
- use local heap for allocating the edge data? (we need a heap class from somewhere?)
*/

//------------------------------------------------------------------------
// Some constants affecting the behavior of the cache
//------------------------------------------------------------------------

namespace DPVS
{

enum
{
    CACHE_TIMEOUT               = 200,          // number of frames that a silhouette has to be unused in order to be auto-kicked out
    MIN_OBJECT_SILHOUETTE_EDGES = 64            // models having less silhouette edges than this are never cached
};

/******************************************************************************
 *
 * Struct:          DPVS::SilhouetteCache::Node
 *
 * Description:     Node that stores a "volumetric" silhouette, i.e. the
 *                  silhouette for a volume of view points
 *
 *****************************************************************************/

/******************************************************************************
 *
 * Struct:          DPVS::SilhouetteCache::Node::Edge
 *
 * Description:     Structure for encoding the incremental silhouette
 *
 * Notes:           The distance parameter is currently encoded into
 *                  a special fixed point. The maximum value is MAX_DISTANCE, which 
 *                  corresponds with the radius of the Node's sphere. OVER_DISTANCE has a 
 *                  special meaning that is used for all "in" edges that have
 *                  distances greater than 'dist'. TERMINATE is used as
 *                  as terminator value (removes some comparisons from
 *                  the traversal).
 *
 *                  The encoding used places an upper limit on the size of the
 *                  occluders we can handle. The maximum size is determined
 *                  by the size of the m_index variable.
 *
 *****************************************************************************/

struct SilhouetteCache::Node
{
    struct Edge
    {
        enum    // These are special m_rDistance enumerations...
        {
            MAX_DISTANCE    = (1<<15)-3,    // max distance in the encoding we use
            OVER_DISTANCE   = (1<<15)-2,    // "in" edges that are beyond the maximum distance
            TERMINATE       = (1<<15)-1,    // termination value
            MAX_EDGE_INDEX  = 0xFFFF        // largest supported edge index
        };

        UINT32  m_rDistance:15;             // event distance (see documentation above)
        UINT32  m_flip:1;                   // flip bit (used only for "in" edges)
        UINT32  m_index:16;                 // index to edge in original mesh (or its Derived structure)
    };

    struct SetupEdge
    {
    private:
        DPVS_FORCE_INLINE UINT32    getDistance (void) const { DPVS_ASSERT(m_distance >= 0.0f); return Math::bitPattern(m_distance); }
    public:
        float   m_distance;                 // event distance
        UINT32  m_index:31;                 // index to the mesh
        UINT32  m_flip:1;                   // single bit for flipping

        DPVS_FORCE_INLINE bool  operator<   (const SetupEdge& s) const  { return getDistance() < s.getDistance(); } 
        DPVS_FORCE_INLINE bool  operator>   (const SetupEdge& s) const  { return getDistance() > s.getDistance(); } 
        DPVS_FORCE_INLINE bool  operator<=  (const SetupEdge& s) const  { return getDistance() <= s.getDistance(); } 
        DPVS_FORCE_INLINE bool  operator>=  (const SetupEdge& s) const  { return getDistance() >= s.getDistance(); } 
    };
    
                                Node            (void)          { memset (this,0,sizeof(SilhouetteCache::Node)); } // careful with this... 
                                ~Node           (void)          { DELETE_ARRAY(m_edges); }
    int                         getMemoryUsed   (void) const    { return sizeof(Node) + (m_numInEdges+ 1 + 1+m_numOutEdges)*sizeof(Edge); }

    UINT32                      m_timeStamp;                // access time stamp
    SilhouetteCache::Client*    m_client;                   // pointer back to client
    Sphere                      m_sphere;                   // bounding sphere for the edge list
    Node*                       m_clientPrev;               // previous node in client's linked list
    Node*                       m_clientNext;               // next node in client's linked list
    Node*                       m_prev;                     // previous node in global linked list (used for LRU cache removal)
    Node*                       m_next;                     // next node in global linked list (used for LRU cache removal)
    INT32                       m_numInEdges;               // number of "in" edges in the node
    INT32                       m_numOutEdges;              // number of "out" edges in the node
    Edge*                       m_edges;                    // edge data ("in" edges first, FLT_MAX, "out" edges, FLT_MAX)
private:
                                Node            (const Node&);  // not allowed
    Node&                       operator=       (const Node&);  // not allowed
};

DPVS_CT_ASSERT(sizeof(SilhouetteCache::Node::Edge)==4);
DPVS_CT_ASSERT(sizeof(SilhouetteCache::Node::SetupEdge)==8);

//------------------------------------------------------------------------
// A special version of the quickSort routine that returns only the
// 'K' smallest elements in sorted order. This is used for the highly
// special case of sorting "out" edges. The amount of edges can be
// pretty large and we only need a very small (on average sqrt(N)) edges
// in sorted order.
//------------------------------------------------------------------------

template <class T> void quickSortPartial(T* elements, int K, int low, int high)
{
    if (low > K || (high-low)<=1)                           // we're done..
        return;

    //--------------------------------------------------------------------
    // Reached cut-off point --> switch to insertionSort..
    //--------------------------------------------------------------------

    const int SWITCHPOINT = 15;                             // optimal value, see paper :)
    if((high - low) <= SWITCHPOINT)
    {
        insertionSort(&elements[low], high - low);
        return;
    }

    //--------------------------------------------------------------------
    // Select pivot using median-3
    //--------------------------------------------------------------------

    int pivotIndex  = median3(elements, low, high);
    swap(elements[high-1], elements[pivotIndex]);           // hide pivot to highest entry

    T pivot(elements[high-1]);

    //--------------------------------------------------------------------
    // Partition data
    //--------------------------------------------------------------------

    int i = low - 1;
    int j = high - 1;

    while(i < j)
    {
        do { i++; } while(i < j && elements[i] <= pivot);
        do { j--; } while(i < j && elements[j] > pivot);

        DPVS_ASSERT(i>=low && j>=low && i < high && j < high);
        swap(elements[i], elements[j]);
    }

    //--------------------------------------------------------------------
    // Restore pivot
    //--------------------------------------------------------------------

    T tmp(elements[j]);
    elements[j] = elements[i];
    elements[i] = elements[high-1];
    elements[high-1] = tmp;

    //--------------------------------------------------------------------
    // Sort sub-partitions
    //--------------------------------------------------------------------

    quickSortPartial(elements, K, low, i);
    quickSortPartial(elements, K, i+1, high);
}

template <class T> DPVS_FORCE_INLINE int quickSortPartial(T* a, int K, int N)
{
    DPVS_ASSERT(K>= 0 && K <= N);
    quickSortPartial(a, K, 0, N);           // sort K smallest elements
    int i = K;
    while (i < N)                           // handle duplicates of the (K-1)th element
    {
        if (a[i] > a[K-1])                  // ok, we found a larger element.. time to stop..
            break;
        i++;
    }

#if defined (DPVS_DEBUG)
    DPVS_ASSERT( i>= K && i<=N);
    DPVS_ASSERT(isSorted(a, i));            // make sure that the data is sorted
    for (int j =i; j < N; j++)
    {
        if (!(a[j] > a[i-1]))
        {
            DPVS_ASSERT(false);
        }
    }
#endif

    return i;                               // return actual number of sorted elements
}

} // DPVS

/*****************************************************************************
 *
 * Function:        SilhouetteCache::isFrontFacing()
 *
 * Description:     Determines whether a point is in the positive half-space
 *                  of a plane
 *
 * Parameters:      plEq = reference to plane equation
 *                  pt   = reference to point (homogenous)
 *
 * Returns:         true if point is in positive half-space
 *
 *****************************************************************************/

static DPVS_FORCE_INLINE bool isFrontFacing (const Vector4& plEq, const Vector4& pt)
{
    return (plEq.x*pt.x + plEq.y*pt.y + plEq.z*pt.z + plEq.w)>=0.0f;
}

/*****************************************************************************
 *
 * Function:        SilhouetteCache::updateTimeStamp()
 *
 * Description:     Updates silhouette cache time stamp
 *
 * Notes:           Call this function once per frame (or camera traversal)
 *
 *****************************************************************************/

void SilhouetteCache::updateTimeStamp (void)
{
    //--------------------------------------------------------------------
    // Update timestamp and statistics (these need to be set every frame
    // as a typical app clears the Library::Statistics now and then)
    //--------------------------------------------------------------------

    m_nodeTimeStamp++;  
    DPVS_PROFILE(Statistics::setStatistic(Library::STAT_SILHOUETTECACHEMEMORYUSED,m_nodeMemoryUsed));
    DPVS_PROFILE(Statistics::setStatistic(Library::STAT_MODELDERIVEDMEMORYUSED,   Client::Derived::s_derivedMemoryUsed));
    DPVS_PROFILE(Statistics::setStatistic(Library::STAT_MODELDERIVED,               Client::Derived::s_numDerived));

    //--------------------------------------------------------------------
    // Here we have a naive "cleanup" system here that kicks out very old
    // nodes from the cache even if the cache is not full. All nodes that
    // have not been accessed for CACHE_TIMEOUT frames are automatically
    // deleted.
    //
    // This approach conserves memory somewhat (as parts of the cache can
    // now be released for other subsystems) and reduces query times
    // (as useless nodes don't slow down the queries). The CACHE_TIMEOUT 
    // value must be sufficiently high so that it doesn't affect much the 
    // overall performance by kicking out useful nodes.
    //--------------------------------------------------------------------

    while (m_nodeTail)
    {
        if ((int)(m_nodeTimeStamp - m_nodeTail->m_timeStamp) < CACHE_TIMEOUT)   // node is too new
            break;
        deleteNode(m_nodeTail);                                                 // kill it!
    }   

    //--------------------------------------------------------------------
    // Free derived structures of models that have not been used for a
    // while if memory is tight
    //--------------------------------------------------------------------

    while (/*Client::Derived::s_derivedMemoryUsed > 512*1024 ||*/ !(g_random.getI()&15))
    {
        if (!Client::s_activeTail)                                          // doh..
            break;

        Client::s_activeTail->releaseDerived();                             // release derived data structures
        Client::s_activeTail->setInactive();                                // set model as inactive
    }

    //--------------------------------------------------------------------
    // Occasionally free internal temporary data structures (recyclers).
    //--------------------------------------------------------------------

    if ((m_nodeTimeStamp & 1023)==207)                                      // once every 1024 frames (the value 207 selected by random)
        freeRecyclers();
}

/*****************************************************************************
 *
 * Function:        SilhouetteCache::createNode()
 *
 * Description:     Function for creating a new node
 *
 * Parameters:      client = pointer to client (non-null)
 *                  sphere = bounding volume for the node
 *
 * Returns:         pointer to new node
 *
 * Notes:           This function computes a new "Node" for a specified
 *                  camera position. 
 *
 *****************************************************************************/

SilhouetteCache::Node* SilhouetteCache::createNode (Client* c, const Vector4& center)
{
    DPVS_ASSERT(c && c->m_derived);
    DPVS_PROFILE(Statistics::incStatistic(Library::STAT_SILHOUETTECACHEINSERTIONS,1));

    Client::Derived* d = c->m_derived;

    //--------------------------------------------------------------------
    // Allocate memory for the node and copy some input parameters..
    //--------------------------------------------------------------------

    SilhouetteCache::Node* n = NEW<SilhouetteCache::Node>();    

    n->m_client         = c;                                // copy client pointer
    n->m_timeStamp      = m_nodeTimeStamp;                  // initialize timestamp

    m_nodeCount++;

    //--------------------------------------------------------------------
    // Link to model-local list (so that model can find this)
    //--------------------------------------------------------------------
    
    n->m_clientPrev = null;
    n->m_clientNext = c->m_derived->m_firstNode;
    if (n->m_clientNext)
        n->m_clientNext->m_clientPrev = n;
    c->m_derived->m_firstNode = n;

    //--------------------------------------------------------------------
    // Link to global LRU-list (so that we can maintain the cache)
    //--------------------------------------------------------------------

    n->m_prev = null;
    n->m_next = m_nodeHead;
    if (n->m_next)
        n->m_next->m_prev = n;
    else
    {
        DPVS_ASSERT(m_nodeHead==null);
        DPVS_ASSERT(m_nodeTail==null);
        m_nodeTail = n;
    }
    m_nodeHead = n;

    //--------------------------------------------------------------------
    // First, calculate distances to each plane
    //--------------------------------------------------------------------

    int                     numPlanes   = d->m_numPlanes;
    const Vector4*          plEq        = d->m_plEq;
    const Client::Edge*     edges       = d->m_edges;                   // input edges
    int                     edgeCount   = d->m_numSilhouetteEdges;      // number of input edges (that may be potentially silhouette edges)

    DPVS_ASSERT( edgeCount <= Node::Edge::MAX_EDGE_INDEX);              // Cannot handle larger meshes due to the encoding used

    //--------------------------------------------------------------------
    // Perform temporary memory allocation using the recycler. We need
    // temporary memory for all the point->plane distances and the
    // in + out edges...
    //--------------------------------------------------------------------

    TempArray<float,false>              dist(d->m_numPlanes);           // distances to planes
    TempArray<Node::SetupEdge,false>    inEdges(edgeCount+1);           // edges..

    //--------------------------------------------------------------------
    // Compute distances to all planes using the optimized Math::dot()
    // routine.
    //--------------------------------------------------------------------

    Math::dot (&dist[0], plEq, center, numPlanes);                      

    //--------------------------------------------------------------------
    // Now find minimum unsigned distances for each straddling edge (this is the
    // place where the visual event may happen). Also classify edges as
    // either "in" or "out". Place "in" edges to the beginning of the
    // output buffer and "out" edges to the very end.
    //--------------------------------------------------------------------

    int numInEdges  = 0;                            // # of edges that are initially "in" (i.e. silhouette edges for the current viewpoint)
    int numOutEdges = 0;                            // # of edges that are initially "out" (i.e. non-silhouette edges)

    for (int i = 0; i < edgeCount; i++)         
    {
        register int    p0      = edges[i].m_plane[0];      // index of first plane
        register int    p1      = edges[i].m_plane[1];      // index of second plane

        DPVS_ASSERT(p0>=0 && p0 < numPlanes);

        register float  A       = dist[p0];         // distance to first plane
        register float  d       = 0.0f;             // use 0.0 to indicate that edge "must" be taken
        register bool   in      = true;             // is the edge an "in edge"
        register bool   flip    = false;            // should we perform a flip?
        
        if (p1 == Client::Edge::NOT_CONNECTED)      // if edge is connected only to a single plane...
        {
            if (A < 0.0f)                           // check if edge is not initially "in"
                in = false;
            d = Math::fabs(A);                      // compute absolute distance to visual event
        } else
        {
            DPVS_ASSERT(p1>=0 && p1 < numPlanes);
            float B = dist[p1];                     // distance to triangle B

            if (A*B > 0.0f)                         // A and B have same facing so edge is not "in"
                in = false;
            else                                    // A and B form a silhouette edge (in = true)
            {
                if (A < 0.0f)
                    flip = true;                    // B is initially the "facing edge"
            }

            float dA = Math::fabs(A);               // take absolute values of the distances..
            float dB = Math::fabs(B);

            d = Math::min(dA, dB);                  // use min instead of compare
        }

        register UINT32 index = i;                  // index of the edge

        if (in)                                     // is this an "in" edge?
        {
            inEdges[numInEdges].m_distance      = d;
            inEdges[numInEdges].m_index         = index;
            inEdges[numInEdges].m_flip          = flip ? 1 : 0;
            numInEdges++;
        } else                                      // it's an "out" edge (we don't encode the FLIP_MASK here)
        {
            inEdges[(edgeCount-1)-numOutEdges].m_distance   = d;
            inEdges[(edgeCount-1)-numOutEdges].m_index      = index;
            inEdges[(edgeCount-1)-numOutEdges].m_flip       = 0;
            numOutEdges++;
        }
    }

    //DPVS_ASSERT((numInEdges+numOutEdges)==edgeCount);

    //--------------------------------------------------------------------
    // Sort the "in" edges
    //--------------------------------------------------------------------

    quickSort (&inEdges[0],numInEdges);

    //--------------------------------------------------------------------
    // Select an amount of "outEdges" that is proportional to the number
    // of "in" edges. Then get the first K smallest outEdges in sorted 
    // order (the remaining edge data is left unsorted)..
    //--------------------------------------------------------------------

    Node::SetupEdge*    outEdges    = &inEdges[edgeCount-numOutEdges];
    float               maxValue    = 0.0f;                                         // distance to outEdge[K-1]
    int                 cnt         = numInEdges+1;     
    if (cnt >= numOutEdges)
        cnt = numOutEdges;

    if (cnt)
    {
        numOutEdges = quickSortPartial (&outEdges[0], cnt, numOutEdges);    
        maxValue    = outEdges[numOutEdges-1].m_distance;
    } else
        numOutEdges = 0;

    //--------------------------------------------------------------------
    // Allocate node edge data and copy the edge information. Note the
    // rather tricky fixed point conversion we use here..
    //--------------------------------------------------------------------

    n->m_numInEdges     = numInEdges;
    n->m_numOutEdges    = numOutEdges;
    n->m_sphere         = Sphere(Vector3(center.x, center.y, center.z), maxValue);  // this Node is valid inside this sphere (as we have info up to that visual event)
    n->m_edges          = NEW_ARRAY<Node::Edge>(numInEdges+numOutEdges+2);                  // +2 for the terminators

    float ooMaxValue = maxValue ? Math::reciprocal(maxValue) : 0.0f;                // don't div by zero

    for (int i = 0; i < numInEdges; i++)                                            // copy "in" edges
    {
        DPVS_ASSERT(inEdges[i].m_index<=0xFFFF);

        float d = inEdges[i].m_distance * ooMaxValue;
        DPVS_ASSERT(d >= 0.0f);
        n->m_edges[i].m_rDistance = (d > 1.0f) ? Node::Edge::OVER_DISTANCE : Math::intChop(d * Node::Edge::MAX_DISTANCE);
        n->m_edges[i].m_index     = inEdges[i].m_index;
        n->m_edges[i].m_flip      = inEdges[i].m_flip;
    }

    for (int i = 0; i < numOutEdges; i++)                               // copy "out" edges
    {
        DPVS_ASSERT(outEdges[i].m_distance <= maxValue);
        DPVS_ASSERT(outEdges[i].m_index<=0xFFFF);
        DPVS_ASSERT(outEdges[i].m_flip==0);
        
        float d = outEdges[i].m_distance * ooMaxValue;
        DPVS_ASSERT(d >= 0.0f && d <= 1.0f);
        n->m_edges[i+1+numInEdges].m_rDistance = Math::intChop(d * Node::Edge::MAX_DISTANCE);
        n->m_edges[i+1+numInEdges].m_index     = outEdges[i].m_index;
        n->m_edges[i+1+numInEdges].m_flip      = 0;
    }

    n->m_edges[numInEdges].m_rDistance                  = Node::Edge::TERMINATE;
    n->m_edges[numInEdges].m_index                      = n->m_edges[0].m_index;    // a "safe" index
    n->m_edges[numInEdges+numOutEdges+1].m_rDistance    = Node::Edge::TERMINATE;    
    n->m_edges[numInEdges+numOutEdges+1].m_index        = n->m_edges[0].m_index;    // another "safe" index

    m_nodeMemoryUsed += n->getMemoryUsed();                             // update memory usage counter

    return n;                                                           // we're done
}

/*****************************************************************************
 *
 * Function:        SilhouetteCache::setupVertexCache()
 *
 * Description:     Makes sure vertex cache is a-ok for the client
 *
 * Parameters:      client = pointer to client (non-null)
 *
 * Notes:           The vertex cache is used during the "refinement" operation
 *                  to avoid storing duplicate vertices in the output silhouette
 *                  structure. This speeds up later processing by ~50%.
 *
 *****************************************************************************/

void SilhouetteCache::setupVertexCache (const Client* client)
{
    DPVS_ASSERT( client);

    if (client->getVertexCount() > m_nodeVertexCacheSize || (m_nodeVertexCacheTimeStamp == 0xFFFF))
    {
        m_nodeVertexCacheSize = ((client->getVertexCount()*5)>>2) + 32; // allocate some extra (geometric series alloc)
        m_nodeVertexCache.reset(m_nodeVertexCacheSize);                 // reset the array
        m_nodeVertexCacheTimeStamp = 0;                                 // reset timestamp
        for (int i = 0; i < m_nodeVertexCacheSize; i++)                 // clear timestamp values
        {
            m_nodeVertexCache[i].m_timeStamp = 0;
            m_nodeVertexCache[i].m_index     = 0;
        }
    }

    m_nodeVertexCacheTimeStamp++;                               // advance timestamp
}

/*****************************************************************************
 *
 * Function:        SilhouetteCache::refineSilhouette()
 *
 * Description:     Calculates exact silhouette using specified node and
 *                  exact camera position
 *
 * Parameters:      info = reference to output structure (the function will allocate necessary data and setup data ptrs)
 *                  node = reference to node
 *                  camInObjectSpace = camera location in object space
 *
 * Notes:           The camera positions MUST be inside the node
 *
 *                  The only route to this function is through SilhouetteCache::getSilhouette()
 *
 *****************************************************************************/

DPVS_FORCE_INLINE void SilhouetteCache::refineSilhouette (EdgeSilhouetteInfo& info, Node& node, const Vector4& camInObjectSpace)
{
    Client* client = node.m_client;

    DPVS_ASSERT(client);
    DPVS_ASSERT(client->m_derived);

    //--------------------------------------------------------------------
    // Calculate fixed-point distance between the current camera point
    // and the sphere center
    //--------------------------------------------------------------------

    Client::Derived*    d           = client->m_derived;                                            // pointer to derived data (plane equations, edges)
    float               fDist       = (Vector3(camInObjectSpace.x,camInObjectSpace.y,camInObjectSpace.z)-node.m_sphere.getCenter()).length();   // distance to original measuring point
    float               ooRadius    = node.m_sphere.getRadius() ? 1.0f / node.m_sphere.getRadius() : 0.0f;
    fDist *= ooRadius;
    DPVS_ASSERT(fDist >= 0.0f && fDist <= 1.0f);

    UINT32              dist        = Math::intChop(fDist * Node::Edge::MAX_DISTANCE);
    DPVS_ASSERT(dist <= Node::Edge::MAX_DISTANCE);  

    //--------------------------------------------------------------------
    // We first measure the distance between the current camera position and
    // the point used to extract the silhouette. Then we traverse all edges
    // that have a visual event closer to this point. If the edge is
    // a silhouette edge, we output its vertices into the output silhouette.
    //
    // The edges have been split into two lists: "in" and "out" edges. The
    // "in" edges are the ones that were originally silhouette edges. For
    // these we can skip a lot of work by noticing that no plane equation
    // tests need to be performed _until_ we cross the visual event
    // boundary.
    //
    // Allocate memory (using the recycler) -- can't use a TempArray
    // here since the pointers are passed out from the function!
    //--------------------------------------------------------------------

    int                 maxPotentialEdges   = node.m_numInEdges + node.m_numOutEdges+1;                 // max potential output edges (+1 needed for some trickery)
    Vector2i*           outEdges            = reinterpret_cast<Vector2i*>(m_recycler.allocate(maxPotentialEdges*sizeof(Vector2i) + maxPotentialEdges*2*sizeof(Vector3)));
    Vector2i*           pOutEdges           = outEdges;
    const Client::Edge* edgeData            = d->m_edges;                                           // get edge data
    const Vector4*      plEq                = d->m_plEq;                                            // plane equations

    if (node.m_numInEdges)
    {
        //--------------------------------------------------------------------
        // First traverse the "in" edges. These are edges that were initial
        // silhouette edges (the edge distance now indicates when is the 
        // earliest time for the edge to become a non-silhouette edge). We
        // use the value FLT_MAX as a terminator..
        //--------------------------------------------------------------------

        for (const Node::Edge* edge = node.m_edges; edge->m_rDistance != Node::Edge::TERMINATE; edge++) 
        {

            register int    index   = edge->m_index;
            register bool   flip    = edge->m_flip;

            DPVS_ASSERT(index>=0 && index < d->m_numSilhouetteEdges);               // HUH?

            const Client::Edge& cEdge = edgeData[index];

            //--------------------------------------------------------------------
            // Check if we cross the visual event boundary.. Perform facing tests
            // only in that case...
            //--------------------------------------------------------------------

            if (edge->m_rDistance <= dist)                                          // if we cross the visual event boundary...
            {
                register int A  = cEdge.m_plane[0];                                 // fetch first plane connected to the edge
                register int B  = cEdge.m_plane[1];                                 // second plane 

                DPVS_ASSERT( A >= 0 && A < d->m_numPlanes);

                register bool fA = isFrontFacing(plEq[A],camInObjectSpace);         // is first triangle front-facing?

                if (B == Client::Edge::NOT_CONNECTED)                               // edge has only single connected triangle
                { 
                    if (!fA)                                                        // first triangle is back-facing -> discard edge
                        continue;
                } else
                {
                    DPVS_ASSERT( B >= 0 && B < d->m_numPlanes);

                    if (fA == isFrontFacing(plEq[B],camInObjectSpace))              // both triangles have same facing -> discard edge
                        continue;
                }

                //----------------------------------------------------------------
                // The idea here is that if the "first triangle" is back-facing,
                // we must swap the edge direction (as the visible edge really
                // belongs to the second triangle).
                //----------------------------------------------------------------

                flip = !fA;                                                         // re-calc flip...
            }

            int v0 = cEdge.m_vertex[0], v1 = cEdge.m_vertex[1];         // add edge and flip if necessary
            if (flip)
                swap(v0,v1);
            *pOutEdges++ = Vector2i(v0,v1);
        }
    }

    //--------------------------------------------------------------------
    // Then traverse the "out" edges. We must traverse this list only as
    // long as we have visual events... NOTE: we use FLT_MAX as a 
    // terminator so we can't break array boundaries
    //--------------------------------------------------------------------

    if (node.m_numOutEdges)
    {
        int cnt = 0;
        for (const Node::Edge* edge = node.m_edges+node.m_numInEdges+1; edge->m_rDistance <= dist; edge++)      
        {
            cnt++;
            DPVS_ASSERT(cnt <= node.m_numOutEdges);

            int     index   = edge->m_index;
            
            DPVS_ASSERT (index >= 0 && index < d->m_numSilhouetteEdges);        // HUH?

            const Client::Edge& cEdge   = edgeData[index];
            register int    A       = cEdge.m_plane[0];                         // first plane connected to the edge
            register int    B       = cEdge.m_plane[1];                         // second plane (or NOT_CONNECTED)
            
            DPVS_ASSERT( A >= 0 && A < d->m_numPlanes);
            
            register bool   fA      = isFrontFacing(plEq[A],camInObjectSpace);  // is first plane front-facing?

            if (B == Client::Edge::NOT_CONNECTED)                               // edge has only single connected plane
            { 
                if (!fA)                                                        // first triangle is back-facing -> discard edge
                    continue;
            } else
            {
                DPVS_ASSERT( B >= 0 && B < d->m_numPlanes);
                if (fA == isFrontFacing(plEq[B],camInObjectSpace))              // both planes have same facing -> discard edge
                    continue;
            }

            int v0 = cEdge.m_vertex[0], v1 = cEdge.m_vertex[1];                 // add edge (flip if necessary)
            if (!fA)
                swap(v0,v1);
            *pOutEdges++ = Vector2i(v0,v1);

        }
    }

    //--------------------------------------------------------------------
    // Fetch the vertex data, remove duplicate vertices and remap the
    // output edge indices
    //--------------------------------------------------------------------

    int         outEdgeCount        = pOutEdges - outEdges;                     // number of output edges
    int         outVertexCount      = 0;                                        // number of output vertices
    Vector3*    outVertices         = reinterpret_cast<Vector3*>(outEdges + outEdgeCount + 1);  // store immediately after the edge data..

    if (outEdgeCount)                                                           // if there were any output edges..
    {
        setupVertexCache (client);                                              // make sure vertex cache is ok

        CachedVertex*       vertexCache         = &m_nodeVertexCache[0];        
        UINT32              curTimeStamp        = m_nodeVertexCacheTimeStamp;
        const Vector3*      vertices            = client->getVertices();        // vertex positions
        int*                intEdges            = reinterpret_cast<int*>(outEdges);     // cast to ints..
        int                 N                   = outEdgeCount*2;

        for (int i = 0; i < N; i++)                                             
        {
            register int v = intEdges[i];                                       // get vertex index

            if (vertexCache[v].m_timeStamp == curTimeStamp)                     // vertex already cached?
            {
                intEdges[i] = vertexCache[v].m_index;                           // perform remapping..
                continue;                                                       // .. and we're done..
            }

            vertexCache[v].m_timeStamp          = (UINT16)curTimeStamp;         // update timestamp
            vertexCache[v].m_index              = (UINT16)outVertexCount;       // find remap value
            intEdges[i]                         = outVertexCount;
            outVertices[outVertexCount++]       = vertices[v];                  // copy vertex position
            DPVS_ASSERT(outVertexCount <= 65536);                               // must fit into 16 bits
        }
    }

    //--------------------------------------------------------------------
    // Setup the output structure properly
    //--------------------------------------------------------------------

    info.m_edges        = outEdges;
    info.m_loc          = outVertices;
    info.m_numEdges     = outEdgeCount;
    info.m_numVertices  = outVertexCount;
}

/*****************************************************************************
 *
 * Function:        SilhouetteCache::getExactSilhouette()
 *
 * Description:     Calculates exact silhouette without any node
 *                  information (O(N) algorithm). This is used when
 *                  we want to by-pass the cache completely - for
 *                  example when the objects have very few vertices or
 *                  when the cache is congested.
 *
 * Parameters:      info                = reference to output structure (the function will allocate necessary data and setup data ptrs)
 *                  c                   = pointer to client (non-null)
 *                  camInObjectSpace    = camera location in object space (homogenous)
 *
 * Notes:           The only route to this function is through SilhouetteCache::getSilhouette()
 *
 *****************************************************************************/

void SilhouetteCache::getExactSilhouette (EdgeSilhouetteInfo& info, Client* c, const Vector4& camInObjectSpace)
{
    DPVS_ASSERT(c && c->m_derived);

    Client::Derived* d = c->m_derived;

    //--------------------------------------------------------------------
    // Allocate temporary memory for the operations (WE NEED TO USE
    // A RECYCLER (NOT A TEMPARRAY) BECAUSE THE DATA IS PASSED OUT OF
    // THE FUNCTION!)
    //--------------------------------------------------------------------

    unsigned char* tmpMemory = 
        m_recycler.allocate(d->m_numSilhouetteEdges*sizeof(Vector2i) +
                            c->getVertexCount() * sizeof(Vector3) +
                            ((d->m_numPlanes+31)>>5)*sizeof(UINT32));

    Vector2i* outEdges      = reinterpret_cast<Vector2i*>(tmpMemory);
    Vector3*  outVertices   = reinterpret_cast<Vector3*>(outEdges + d->m_numSilhouetteEdges);
    UINT32*   planeMask     = reinterpret_cast<UINT32*>(outVertices + c->getVertexCount());

    //--------------------------------------------------------------------
    // First classify all triangles as either front- or back-facing.
    // We use TRUE for front-facing triangles and FALSE for back-facing.
    // The values are encoded using a single bit per triangle (1 = TRUE).
    //--------------------------------------------------------------------

    int numPlanes = d->m_numPlanes;

    fillDWord (planeMask, 0, (numPlanes+31)>>5);                    // clear the array (init to bit zero)
    
    const Vector4*  plEq        = d->m_plEq;
    for (int i=0; i < numPlanes; i++)                               // classify all planes
    if (isFrontFacing(plEq[i],camInObjectSpace))
        planeMask[i>>5] |= 1<<(i&31);

    //--------------------------------------------------------------------
    // Traverse through all edges and classify them either as silhouette
    // or non-silhouette. For silhouette edges output vertices
    //--------------------------------------------------------------------

    setupVertexCache(c);                                            // make sure vertex cache is ok

    CachedVertex*       vertexCache     = &m_nodeVertexCache[0];        
    UINT32              curTimeStamp    = m_nodeVertexCacheTimeStamp;
    int                 edgeCount       = d->m_numSilhouetteEdges;
    const Client::Edge* edgeData        = d->m_edges;
    const Vector3*      vertices        = c->getVertices();
    int                 outEdgeCount    = 0;
    int                 outVertexCount  = 0;

    for (int i = 0; i < edgeCount; i++)
    {
        int     A   = edgeData[i].m_plane[0];                   // index to first plane
        int     B   = edgeData[i].m_plane[1];                   // index to second plane

        DPVS_ASSERT( A >= 0 && A < d->m_numPlanes);         // HUH?

        bool    fA  = (planeMask[A>>5]&(1<<(A&31)))!=0;         // status of first triangle

        if (B == Client::Edge::NOT_CONNECTED)                   // edge has only single connected triangle 
        { 
            if (!fA)                                            // the only connected triangle is back-facing
                continue;
        } else
        {
            DPVS_ASSERT( B >= 0 && B < d->m_numPlanes);     // HUH?
            bool fB = (planeMask[B>>5]&(1<<(B&31)))!=0;         // status of second triangle
            if (fA == fB)                                       // both triangles have same facing (not a silhouette edge)
                continue;   
        }

        //----------------------------------------------------------------
        // Output edge and vertices into the output structures. We use
        // the vertex cache here to detect duplicate vertices.
        //----------------------------------------------------------------

        int vertex0 = edgeData[i].m_vertex[0];
        DPVS_ASSERT(vertex0 >= 0 && vertex0 < c->getVertexCount());

        if (vertexCache[vertex0].m_timeStamp != curTimeStamp)               // vertex not cached yet..
        {
            vertexCache[vertex0].m_timeStamp    = (UINT16)curTimeStamp;     // update timestamp
            vertexCache[vertex0].m_index        = (UINT16)outVertexCount;   // find remap value
            outVertices[outVertexCount++]       = vertices[vertex0];        // copy vertex position
        }

        int vertex1 = edgeData[i].m_vertex[1];
        DPVS_ASSERT(vertex1 >= 0 && vertex1 < c->getVertexCount());

        if (vertexCache[vertex1].m_timeStamp != curTimeStamp)               // vertex not cached yet..
        {
            vertexCache[vertex1].m_timeStamp    = (UINT16)curTimeStamp;     // update timestamp
            vertexCache[vertex1].m_index        = (UINT16)outVertexCount;   // find remap value
            outVertices[outVertexCount++]       = vertices[vertex1];        // copy vertex position
        }

        //----------------------------------------------------------------
        // The idea here is that if the "first triangle" is back-facing,
        // we must swap the edge direction (as the visible edge really
        // belongs to the second triangle).
        //----------------------------------------------------------------

        if (!fA)
            swap(vertex0,vertex1);

        outEdges[outEdgeCount][0] = vertexCache[vertex0].m_index;           // remap index
        outEdges[outEdgeCount][1] = vertexCache[vertex1].m_index;           // remap index
        outEdgeCount++;                                                     // advance output edge count
    }

    DPVS_ASSERT(outEdgeCount  <= d->m_numSilhouetteEdges);
    DPVS_ASSERT(outVertexCount<= 65536 && outVertexCount <= c->getVertexCount());   // must fit into 16 bits

    //--------------------------------------------------------------------
    // Setup the output structure properly
    //--------------------------------------------------------------------

    info.m_edges        = outEdges;
    info.m_loc          = outVertices;
    info.m_numEdges     = outEdgeCount;
    info.m_numVertices  = outVertexCount;
}

/*****************************************************************************
 *
 * Function:        SilhouetteCache::deleteNode()
 *
 * Description:     Deletes a node
 *
 * Parameters:      n = pointer to node (may be null - we just return)
 *
 *****************************************************************************/

void SilhouetteCache::deleteNode (Node* n)
{
    if (!n)
        return;

    DPVS_PROFILE(Statistics::incStatistic(Library::STAT_SILHOUETTECACHEREMOVALS,1));

    //--------------------------------------------------------------------
    // Remove from client's local list
    //--------------------------------------------------------------------

    DPVS_ASSERT(n->m_client && n->m_client->m_derived);

    if (n->m_clientPrev)
        n->m_clientPrev->m_clientNext = n->m_clientNext;
    else
    {
        DPVS_ASSERT(n->m_client->m_derived->m_firstNode == n);
        n->m_client->m_derived->m_firstNode = n->m_clientNext;
    }

    if (n->m_clientNext)
        n->m_clientNext->m_clientPrev = n->m_clientPrev;

    //--------------------------------------------------------------------
    // Remove from global node list (cache LRU)
    //-------------------------------------------------------------------- 

    if (n->m_prev)
        n->m_prev->m_next = n->m_next;
    else
    {
        DPVS_ASSERT(m_nodeHead==n);
        m_nodeHead = n->m_next;
    }
    if (n->m_next)
        n->m_next->m_prev = n->m_prev;
    else
    {
        DPVS_ASSERT(m_nodeTail==n);
        m_nodeTail = n->m_prev;
    }

    m_nodeCount--;
    DPVS_ASSERT(m_nodeCount >= 0);

    m_nodeMemoryUsed -= n->getMemoryUsed();
    DPVS_ASSERT(m_nodeMemoryUsed >= 0);

    DELETE(n);                                  // free the memory used..
}

/*****************************************************************************
 *
 * Function:        SilhouetteCache::deleteClientNodes()
 *
 * Description:     Deletes all nodes of a client
 *
 * Parameters:      c = pointer to client (may not be null)
 *
 *****************************************************************************/

void SilhouetteCache::deleteClientNodes (Client* c)
{
    DPVS_ASSERT(c);
    while (c->m_derived && c->m_derived->m_firstNode)
        deleteNode (c->m_derived->m_firstNode);
}

/*****************************************************************************
 *
 * Function:        SilhouetteCache::isCacheFull()
 *
 * Description:     Returns boolean value indicating whether cache is full or not
 *
 * Returns:         true if cache is full, false otherwise
 *
 *****************************************************************************/

inline bool SilhouetteCache::isCacheFull (void) const
{
    return (m_nodeMemoryUsed > m_maxCacheSizeInBytes);
}

/*****************************************************************************
 *
 * Function:        SilhouetteCache::isCacheCongested()
 *
 * Description:     Returns boolean value indicating whether cache is congested or not
 *
 * Returns:         true if cache is congested, false otherwise
 *
 * Notes:           Being congested means that the cache is full and all items 
 *                  in the cache have been accessed during the current "frame".
 *                  Some algorithms (such as the cache replacement policy) have
 *                  to be modified in such situations.
 *
 *****************************************************************************/

DPVS_FORCE_INLINE bool SilhouetteCache::isCacheCongested (void) const
{
    // The idea here is that we get all cached volumes a lifetime of at
    // least a couple of frames. This resolves correctly the congestion cases.

    return (isCacheFull() && m_nodeTail && ((m_nodeTimeStamp-m_nodeTail->m_timeStamp) <= 2));
}

/*****************************************************************************
 *
 * Function:        SilhouetteCache::keepCacheSizeInBounds()
 *
 * Description:     Checks node cache size
 *
 * Notes:           If too much memory is used, the least-recently-used
 *                  nodes will be kicked out.  Note that cache congestion
 *                  situation is _not_ respected here - as we really need
 *                  to make the cache smaller.
 *
 *****************************************************************************/

void SilhouetteCache::keepCacheSizeInBounds(void)
{
    while (isCacheFull())                   // .. while cache is full..
    {
        DPVS_ASSERT(m_nodeTail);            // HHHHUUUUHHH???
        deleteNode (m_nodeTail);            // kill the tail node...
    }

    if (m_maxCacheSizeInBytes == 0)         // if zero..
        DPVS_ASSERT(m_nodeTail == null);    // .. make sure we managed to clean the entire cache..
}

/*****************************************************************************
 *
 * Function:        SilhouetteCache::touchNode()
 *
 * Description:     Marks node as used (so that we can update the LRU)
 *
 * Parameters:      node    = pointer to node
 *
 *****************************************************************************/

DPVS_FORCE_INLINE void SilhouetteCache::touchNode (Node* node)
{
    DPVS_ASSERT(node);

    //--------------------------------------------------------------------
    // Advance node time stamp
    //--------------------------------------------------------------------

    node->m_timeStamp = m_nodeTimeStamp;

    //--------------------------------------------------------------------
    // Move node to head of global LRU list
    // DEBUG DEBUG OPTIMIZE: if the node's left neighbor has "current"
    // time stamp, we don't need to move the node. Does this help really??
    // The code here is "opened up" - could be implemented in just a couple
    // of lines of code if STL is used
    //--------------------------------------------------------------------

    if (node!=m_nodeHead)
    {   
        if (node->m_prev)
            node->m_prev->m_next = node->m_next;
        else
        {
            DPVS_ASSERT(node==m_nodeHead);      // WHAT??
            m_nodeHead = node->m_next;
        }
        
        if (node->m_next)
            node->m_next->m_prev = node->m_prev;
        else
        {
            DPVS_ASSERT(node == m_nodeTail);
            m_nodeTail = node->m_prev;
        }

        node->m_prev = null;
        node->m_next = m_nodeHead;
        if (node->m_next)
            node->m_next->m_prev = node;
        else
            m_nodeTail = node;
        m_nodeHead = node;
    }

    if (node != node->m_client->m_derived->m_firstNode)
    {
        if (node->m_clientPrev)
            node->m_clientPrev->m_clientNext = node->m_clientNext;
        else
        {
            DPVS_ASSERT(node==node->m_client->m_derived->m_firstNode);
            node->m_client->m_derived->m_firstNode = node->m_clientNext;
        }
        if (node->m_clientNext)
            node->m_clientNext->m_clientPrev = node->m_clientPrev;

        node->m_clientPrev = null;
        node->m_clientNext = node->m_client->m_derived->m_firstNode;
        if (node->m_clientNext)
            node->m_clientNext->m_clientPrev = node;
        node->m_client->m_derived->m_firstNode = node;
    }

}

/*****************************************************************************
 *
 * Function:        SilhouetteCache::getBestNode()
 *
 * Description:     Returns best node for the specified camera position
 *
 * Parameters:      client              = pointer to client (non-null)
 *                  info                = reference to edge silhouette info structure
 *                  camInObjectSpace    = camera object space location (note: homogenous!)
 *
 * Returns:         pointer to best node or null if none found
 *
 * Notes:           The "best" node is one that contains the camera position
 *                  and has the least number of straddling edges
 *
 *****************************************************************************/

SilhouetteCache::Node* SilhouetteCache::getBestNode (Client *c, const Vector4& camInObjectSpace) const
{
    DPVS_ASSERT(c && c->m_derived); // we may not come here if the 'derived' is not created..

    //--------------------------------------------------------------------
    // Traverse through volumes of the model trying to find any volume
    // that contains the camera point. If there are multiple candidates,
    // select the one with the least amount of straddling edges.
    //--------------------------------------------------------------------

    Node*   best            = null;                         // best volume found so far
    float   bestDistance    = FLT_MAX;

    //--------------------------------------------------------------------
    // Return pointer to best valid node or null if none is found..
    //--------------------------------------------------------------------

    // DEBUG DEBUG OPTIMIZE: BUILD A BETTER SEARCH STRUCTURE WHEN THE
    // NUMBER OF SILHOUETTES FOR A SINGLE MODEL IS VERY LARGE

    int     cnt = 0;
    Vector3 loc(camInObjectSpace.x,camInObjectSpace.y,camInObjectSpace.z);

    for (Node* n = c->m_derived->m_firstNode; n; n = n->m_clientNext)
    {
        cnt++;

        Vector3 delta   = n->m_sphere.getCenter() - loc;
        float   d2      = delta.x*delta.x + delta.y*delta.y + delta.z*delta.z;
        float   r2      = n->m_sphere.getRadius() * n->m_sphere.getRadius();

        if (d2 <= r2 && d2 < bestDistance)
        {
            bestDistance    = d2;
            best            = n;
//          break;
        }
    }

    DPVS_ASSERT(cnt <= m_nodeCount);    // HUUUUH?
    DPVS_PROFILE(Statistics::incStatistic(Library::STAT_SILHOUETTECACHEQUERYITERS,cnt));

    return best;
}

/*****************************************************************************
 *
 * Function:        SilhouetteCache::getSilhouette()
 *
 * Description:     Returns exact silhouette for specified camera position
 *
 * Parameters:      client              = pointer to client
 *                  info                = reference to edge silhouette info structure (data is stored there)
 *                  camInObjectSpace    = camera object space location (homogenous, i.e. w = 1 if perspective projection)
 *
 * Notes:           Note that the data pointers of the previous call to
 *                  getSilhouette() _ARE_ not valid anymore...
 *
 *                  The "camInObjectSpace" parameter supports both ortho and
 *                  perspective projections. If the "W" value is 1.0, then
 *                  it is a location in object space. If "W" value is 0.0, then
 *                  it is a _direction_ (OpenGL uses a similar convention).
 *
 *                  The function may fail in a case where the model has more
 *                  that Client::MAX_TRIANGLES triangles (65535?) or more than
 *                  Client::MAX_VERTICES vertices (65535). In such a case
 *                  an empty silhouette is returend.
 *
 *****************************************************************************/

void SilhouetteCache::getSilhouette (Client* c, EdgeSilhouetteInfo& info, const Vector4& camInObjectSpace)
{
    //--------------------------------------------------------------------
    // Update statistics
    //--------------------------------------------------------------------

    DPVS_PROFILE(Statistics::incStatistic(Library::STAT_SILHOUETTECACHEQUERIES,1));

    //--------------------------------------------------------------------
    // Initialize to valid values if we need to early-exit
    //--------------------------------------------------------------------

    info.m_edges        = null;
    info.m_loc          = null;
    info.m_numEdges     = 0;
    info.m_numVertices  = 0;

    //--------------------------------------------------------------------
    // Make sure derived data structures are a-ok (we're going to need
    // them in the code below). If the setup fails (perhaps the mesh
    // has more than 65536 triangles??), we return immediately. Also,
    // we cannot handle meshes with >= MAX_EDGE_INDEX silhouette edges 
    // due to the coding used internally.
    //--------------------------------------------------------------------

    if (!c->getDerived() || c->m_derived->m_numSilhouetteEdges >= Node::Edge::MAX_EDGE_INDEX)
        return;

    //--------------------------------------------------------------------
    // Find best node for the specified camera position/orientation
    //--------------------------------------------------------------------
    
    Node* best = getBestNode(c,camInObjectSpace);

    //--------------------------------------------------------------------
    // Here we have a special rule that is used to gradually improve the
    // quality of the cache. It is only used in situations when the cache
    // is not congested. The rule says that there's an approx. 1% chance
    // of discarding the selected node and force-creating a new one (for
    // the current viewpoint).
    //--------------------------------------------------------------------

    if (best && !isCacheCongested() && !(g_random.getI()&127))
        best = null;

    //--------------------------------------------------------------------
    // If we have a node, update its timestamps correspondingly.
    //--------------------------------------------------------------------

    if (best)
    {
        DPVS_PROFILE(Statistics::incStatistic(Library::STAT_SILHOUETTECACHEHITS,1));
        touchNode(best);
    } else
    {
        //---------------------------------------------------------------------
        // If no node was found, we must decide whether we want to extract
        // the silhouette directly from the model (O(N) operation) or if we
        // want to create a volume and cache it for future use.
        //---------------------------------------------------------------------

        bool bruteForce = false;

        if (c->m_derived->m_numSilhouetteEdges < MIN_OBJECT_SILHOUETTE_EDGES ||     // model is too simple  (not worth caching)
            isCacheCongested())                                                     // cache is congested   (switch effectively to MRU algorithm).
            bruteForce = true;

        //----------------------------------------------------------------
        // If brute force extraction was selected, perform it and
        // return immediately.
        //----------------------------------------------------------------

        if (bruteForce)        
        {
            if (isCacheCongested())
                DPVS_PROFILE(Statistics::incStatistic(Library::STAT_SILHOUETTECACHECONGESTED,1));   

            DPVS_PROFILE(Statistics::incStatistic(Library::STAT_SILHOUETTECACHEHITS,1));        // count this as a hit...
            getExactSilhouette (info, c, camInObjectSpace);
            return;                             // we're done!
        }

        //----------------------------------------------------------------
        // Make sure that we have enough space in the cache and then 
        // create a new node..
        //----------------------------------------------------------------

        keepCacheSizeInBounds();                

        if (!c->getDerived())                   // if we lost our derived structure (how can that happen?) -> return
            return;

        best = createNode(c, camInObjectSpace); // create a new node
    }

    //---------------------------------------------------------------------
    // OK. We have a node now. Time to extract the exact silhouette from it.
    //---------------------------------------------------------------------

    DPVS_ASSERT (best);
    refineSilhouette (info, *best, camInObjectSpace);
}

//------------------------------------------------------------------------
