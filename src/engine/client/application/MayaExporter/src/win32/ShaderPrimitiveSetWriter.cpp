// ======================================================================
//
// ShaderPrimitiveSetWriter.cpp
// Todd Fiala
//
// copyright 1999, Bootprint Entertainment
//
// ======================================================================

#include "FirstMayaExporter.h"

#include "ShaderPrimitiveSetWriter.h"
#include "clientGraphics/ShaderPrimitiveSetTags.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/ShaderTemplate.h"

#include "sharedMath/Triangle3d.h"
#include "sharedCollision/Containment3d.h"
#include "sharedCollision/Overlap3d.h"

#include "sharedFile/Iff.h"
#include "sharedFile/TreeFile.h"
#include "sharedMath/Sphere.h"
#include "sharedMath/Vector.h"
#include "clientGraphics/SystemVertexBuffer.h"
#include "SetDirectoryCommand.h"
#include "PluginMain.h"

#include <algorithm>
#include <vector>
#include <set>
#include <string>

#define WRITE_SORTED_INDICES

using namespace ShaderPrimitiveSetTags;
using namespace Containment3d;

typedef std::vector<Vector> TVectorVec;
typedef std::vector<Index> TIndexVec;

// ======================================================================

static const Tag TAG_INDX = TAG (I,N,D,X);
static const Tag TAG_SIDX = TAG (S,I,D,X);

// ----------------------------------------------------------------------

DirectionSortedIndexArray::DirectionSortedIndexArray()
{
}

DirectionSortedIndexArray::~DirectionSortedIndexArray()
{
}



// ----------------------------------------------------------------------

const int gkNumSortDirs = 10;

Vector	gSortDirs[gkNumSortDirs] =
{
	Vector( 1, 0, 0),
	Vector(-1, 0, 0),
	Vector( 0, 1, 0),
	Vector( 0,-1, 0),
	Vector( 0, 0, 1),
	Vector( 0, 0,-1),

	Vector( 1, 0, 1),
	Vector(-1, 0, 1),
	Vector( 1, 0,-1),
	Vector(-1, 0,-1)
};

enum ESortResult
{
	kTriAFirst,
	kTriBFirst,
	kUnsortable,
	kDoesntMatter
};

// ----------------------------------------------------------------------

int sortOrder2( Triangle3d const & A, Triangle3d const & B, Vector const & direction )
{
	// This epsilon can be rather large since this is only an approximate sort - 
	// it's better to classify triangle A as touching triangle B's plane instead
	// of crossing it.

	real triTestEpsilon = 0.001f;

	bool overlap = Overlap3d::TestTriTri_proj(A,B,direction,triTestEpsilon);

	if(!overlap) 
	{
		return kDoesntMatter;
	}

	// ----------

	int sideA = Containment3d::TestTriTri(A,B,triTestEpsilon);
	int sideB = Containment3d::TestTriTri(B,A,triTestEpsilon);

	bool facingA = A.getNormal().dot(direction) < 0;
	bool facingB = B.getNormal().dot(direction) < 0;

	// ----------
	
	if((sideA == CR_Outside) || (sideA == CR_TouchingOutside))
	{
		// A is in front of B, draw B first if B is facing the camera.

		return facingB ? kTriBFirst : kTriAFirst;
	}
	else if ((sideA == CR_Inside) || (sideA == CR_TouchingInside))
	{
		// A is behind B, draw A first if B is facing the camera.

		return facingB ? kTriAFirst : kTriBFirst;
	}
	else if((sideA == CR_Boundary) || (sideA == CR_Overlap))
	{
		// A intersects B's plane, sort based on B's side.

		if((sideB == CR_Outside) || (sideB == CR_TouchingOutside))
		{
			// B is in front of A, draw A first if A is facing the camera

			return facingA ? kTriAFirst : kTriBFirst;
		}
		else if ((sideB == CR_Inside) || (sideB == CR_TouchingInside))
		{
			// B is behind A, draw B first if A is facing the camera

			return facingA ? kTriBFirst : kTriAFirst;
		}
		else if ((sideB == CR_Boundary) || (sideB == CR_Overlap))
		{
			// A intersects B's plane, B intersects A's plane.

			// The two triangles might be interpenetrating, but we don't have code to check for that yet.
			// Assume that they are interpenetrating and thus unsortable.

			return kUnsortable;
		}
		else
		{
			DEBUG_FATAL(true,("sortOrder - Got a bad side ID from Containment3d::TestTriTri\n"));
		}
	}
	else
	{
		DEBUG_FATAL(true,("sortOrder - Got a bad side ID from Containment3d::TestTriTri\n"));
	}

	return kUnsortable;
}

// ----------------------------------------------------------------------

struct TriSortEntry
{
	TriSortEntry() : tri(Vector::zero,Vector::zero,Vector::zero), direction(Vector::zero), index(-1) {}

	TriSortEntry( Triangle3d t, Vector d, int i ) 
		: tri(t), direction(d), index(i) {}

	Triangle3d   tri;
	Vector       direction;
	int          index;

	real getKey(void) const
	{
		Vector A = tri.getCornerA();
		Vector B = tri.getCornerB();
		Vector C = tri.getCornerC();

		real keyA = -A.dot(direction);
		real keyB = -B.dot(direction);
		real keyC = -C.dot(direction);

		real minKey = std::min(std::min(keyA,keyB),keyC);

		return minKey;
	}

	static bool less ( const TriSortEntry & A, const TriSortEntry & B )
	{
		real keyA = A.getKey();
		real keyB = B.getKey();

		bool keySortOrder = keyA < keyB;

		int sideSortOrder = sortOrder2(A.tri,B.tri,A.direction);

		if(sideSortOrder == kTriAFirst)
		{
			return true;
		}
		else if(sideSortOrder == kTriBFirst)
		{
			return false;
		}
		else
		{
			return keySortOrder;
		}
	}
};

// ----------------------------------------------------------------------
// For each pair of triangles in the given indexed triangle array, determine how many pairs are sorted
// correctly, incorrectly, are unsortable, or are sort-independent when viewed from the given direction.

typedef int (*triangleOrderFunction)(Triangle3d const & A, Triangle3d const & B, Vector const & direction);

void sort_metric ( int nTris, 
				   Index * indices, 
				   Vector * verts, 
				   Vector const & direction,
				   triangleOrderFunction testFunc,
				   int & outRight,
				   int & outWrong,
				   int & outUnsortable,
				   int & outIndependent )
{
	outRight = 0;
	outWrong = 0;
	outUnsortable = 0;
	outIndependent = 0;

	// ----------
	
	for(int i = 0; i < nTris-1; i++)
	{
		int indexA = i * 3;

		Triangle3d A( verts[indices[indexA+0]],
					  verts[indices[indexA+1]],
					  verts[indices[indexA+2]] );

		for(int j = i+1; j < nTris; j++)
		{
			int indexB = j * 3;

			Triangle3d B( verts[indices[indexB+0]],
						  verts[indices[indexB+1]],
						  verts[indices[indexB+2]] );


			int sort = testFunc(A,B,direction);

			if(sort == kTriAFirst)    outRight++;
			if(sort == kTriBFirst)    outWrong++;
			if(sort == kUnsortable)   outUnsortable++;
			if(sort == kDoesntMatter) outIndependent++;
		}
	}
}

// ----------------------------------------------------------------------

typedef std::vector<TriSortEntry> TTriSortVec;

void sort_triangles	( int nTris,
					  Index * indices,
					  Vector * verts, 
					  Vector const & direction, 
					  TIndexVec & outIndices )
{
	TTriSortVec sortVec;

	sortVec.reserve(nTris);

	for(int i = 0; i < nTris; i++)
	{
		int index = i * 3;

		Vector A = verts[ indices[index + 0] ];
		Vector B = verts[ indices[index + 1] ];
		Vector C = verts[ indices[index + 2] ];

		sortVec.push_back( TriSortEntry( Triangle3d(A,B,C), direction, i ) );
	}

	//NOTE FIXME TODO: changing this to std::sort can cause very rare (but reproducible) memory corruption, this should be investigated further
	std::stable_sort(sortVec.begin(),sortVec.end(),TriSortEntry::less);

	// ----------
	// Pull the sorted triangle indices from the sort set and use them to fill
	// the output array

	FATAL(sortVec.size() != static_cast<uint32>(nTris),("sort_triangles - Sorted set doesn't contain the same number of triangles as we started with\n"));

	outIndices.clear();

	for(TTriSortVec::iterator it = sortVec.begin(); it != sortVec.end(); ++it)
	{
		TriSortEntry const & entry = *it;

		int index = entry.index * 3;

		outIndices.push_back( indices[index + 0] );
		outIndices.push_back( indices[index + 1] );
		outIndices.push_back( indices[index + 2] );
	}

	// ----------
	// The vertex indices are now in some sort of order, but they may not be perfect.
	// Just out of curiosity, let's see how many pairs of triangles we believe are sorted
	// incorrectly

	/*	
	int right = 0;
	int wrong = 0;
	int unsortable = 0;
	int independent = 0;

	sort_metric(nTris,outIndices.begin(),verts,direction,sortOrder2,
		        right,wrong,unsortable,independent);
	*/
}

// ----------

void	sort_index_buffer ( int nIndices, Index * indexArray, SystemVertexBuffer & vertBuffer, Vector const & direction, TIndexVec & outIndices )
{
	TIndexVec indices;
	TVectorVec verts;
	int nTris = nIndices / 3;

	for(int i = 0; i < nIndices; i++)
	{
		indices.push_back(indexArray[i]);
	}

	// ----------

	for(VertexBufferReadIterator readIt = vertBuffer.begin(); readIt != vertBuffer.end(); ++readIt)
	{
		verts.push_back( readIt.getPosition() );

	}

	sort_triangles( nTris, &(*indices.begin()), &(*verts.begin()), direction, outIndices );

	FATAL(outIndices.size() != static_cast<uint32>(nIndices),("sort_triangles - didn't end up with the same number of indices that we started with"));
}

//-----------------------------------------------------------------

ShaderPrimitiveSetWriter::PerPrimitiveData::PerPrimitiveData(const char *newShaderTemplateName, int newPriority, ShaderPrimitiveSetPrimitiveType newPrimitiveType, SystemVertexBuffer *newVertexBuffer, int newIndexCount, Index *newIndexArray, bool newOwnsData)
:
	nextValue(0),
	shaderTemplateName(newShaderTemplateName),
	priority(newPriority),
	primitiveType(newPrimitiveType),
	vertexBuffer(0),
	indexCount(newIndexCount),
	indexArray(0),
	ownsData(newOwnsData)
{
	DEBUG_FATAL(indexCount < 0, ("invalid indexCount arg %d", indexCount));
	DEBUG_FATAL(!newVertexBuffer, ("null newVertexBuffer arg"));
	DEBUG_FATAL(newVertexBuffer->getNumberOfVertices() < 1, ("invalid VertexBuffer arg, has %d vertices, must have at least one", newVertexBuffer->getNumberOfVertices()));

	if (ownsData)
	{
		// we own it, we keep the values passed into us and will delete them later
		vertexBuffer = newVertexBuffer;
		indexArray  = newIndexArray;
	}
	else
	{
		// we don't own any of the data passed into us, copy all of it
		int i;

		// copy vertex array
		vertexBuffer = new SystemVertexBuffer(newVertexBuffer->getFormat(), newVertexBuffer->getNumberOfVertices());
		vertexBuffer->begin().copy(newVertexBuffer->begin(), newVertexBuffer->getNumberOfVertices());

		// copy index array
		if (newIndexCount)
		{
			DEBUG_FATAL(!newIndexArray, ("null newIndexArray arg"));

			indexArray = new Index [static_cast<size_t>(newIndexCount)];
			for (i = 0; i < newIndexCount; ++i)
				indexArray[i] = newIndexArray[i];
		}
	}

	// ----------
	// If this is a SPS with transparency, generate our sets of pre-sorted indices

	if(getPrimitiveType() == SPSPT_indexedTriangleList)
	{
		sortedIndexArrays.resize(gkNumSortDirs);

		int i;

		for(i = 0; i < gkNumSortDirs; i++)
		{
			sortedIndexArrays[i].sortDir = gSortDirs[i];
			sortedIndexArrays[i].sortDir.normalize();

			sort_index_buffer(	indexCount, 
								indexArray, 
								*vertexBuffer, 
								sortedIndexArrays[i].sortDir, 
								sortedIndexArrays[i].indices );
		}

		// HACK - Overwrite the current set of indices for testing

		for(i = 0; i < indexCount; i++)
		{
			indexArray[i] = sortedIndexArrays[0].indices[i];
		}
	}
}

// ----------------------------------------------------------------------

ShaderPrimitiveSetWriter::PerPrimitiveData::~PerPrimitiveData(void)
{
	delete vertexBuffer;
	delete [] indexArray;

	nextValue = 0;
}

// ----------------------------------------------------------------------

// This is not done as a single array write because the size of the Index 
// type may change depending upon the platform, and we do not want the data
// format to depend upon that.  so the data is stored on disk in int32 form,
// and we'll read it in and convert it to the Index type as we go.

static void writeIndexArray (const int32 indexCount, const Index* const indices, Iff& iff)
{
	iff.insertChunkData(indexCount);

	for(int i = 0; i < indexCount; i++)
		iff.insertChunkData(static_cast<uint16>(indices [i]));
}

// ----------------------------------------------------------------------

/*
bool doesShaderUseAlpha ( char const * const shaderTemplateName )
{
	std::string name = shaderTemplateName;
	std::string dir = SetDirectoryCommand::getDirectoryString(APPEARANCE_WRITE_DIR_INDEX);

//	std::string fullName = dir + name;

//	TreeFile::addSearchAbsolute(0);

	// chop off the trailing slash
	dir.resize(dir.size() - 1);

	TreeFile::addSearchPath(dir.c_str(),1);

//	ShaderTemplate const * testTemplate = ShaderTemplateList::fetch(fullName.c_str());
	ShaderTemplate const * testTemplate = ShaderTemplateList::fetch(name.c_str());

	bool usesAlpha = testTemplate->canHaveAlpha();

	testTemplate->release();

	return usesAlpha;
}
*/

// ----------------------------------------------------------------------

void ShaderPrimitiveSetWriter::PerPrimitiveData::write(Iff & iff, bool bHasAlpha) const
{
	bool hasIndices = (indexCount != 0);

#ifdef WRITE_SORTED_INDICES
	bool hasSortedIndices = (sortedIndexArrays.size() != 0) && bHasAlpha;
#else
	bool hasSortedIndices = false;
#endif

	// ----------

	// ----------

	iff.insertForm(TAG_0001);

	// ----------

	iff.insertChunk(TAG_INFO);

	iff.insertChunkData(static_cast<int32>(primitiveType));
	iff.insertChunkData(hasIndices);
	iff.insertChunkData(hasSortedIndices);

	iff.exitChunk(TAG_INFO);

	// ----------
	// insert vertex array
	vertexBuffer->write(iff);

	// ----------
	// insert index list

	if (hasIndices)
	{
		DEBUG_FATAL(!indexArray, ("null indexArray member"));

		iff.insertChunk(TAG_INDX);

		writeIndexArray( indexCount, indexArray, iff );

		iff.exitChunk(TAG_INDX);
	}

	// ----------
	// Direction-sorted index arrays

	if(hasSortedIndices)
	{
		iff.insertChunk(TAG_SIDX);

		int32 nArrays = sortedIndexArrays.size();

		iff.insertChunkData(nArrays);

		for(int i = 0; i < nArrays; i++)
		{
			DirectionSortedIndexArray const & D = sortedIndexArrays[i];

			iff.insertChunkData( D.sortDir );

			int32 nIndices = D.indices.size();

			writeIndexArray( nIndices, &(*D.indices.begin()), iff );
		}

		iff.exitChunk(TAG_SIDX);
	}

	// ----------

	iff.exitForm(TAG_0001);
}

// ======================================================================

ShaderPrimitiveSetWriter::ShaderPrimitiveSetWriter(bool writerOwnsAllData)
:	primitiveCount(0),
	firstPrimitive(0),
	ownsData(writerOwnsAllData),
	hasAlpha(false)
{
}

// ======================================================================

ShaderPrimitiveSetWriter::~ShaderPrimitiveSetWriter(void)
{
	PerPrimitiveData *deadPrimitive;

	while (firstPrimitive)
	{
		deadPrimitive  = firstPrimitive;
		firstPrimitive = firstPrimitive->next();
		delete deadPrimitive;
	}
}

// ======================================================================

void ShaderPrimitiveSetWriter::setHasAlpha( bool bHasAlpha )
{
	hasAlpha = bHasAlpha;
}

// ======================================================================

void ShaderPrimitiveSetWriter::add(const char *shaderTemplateName, ShaderPrimitiveSetPrimitiveType primitiveType, SystemVertexBuffer *vertexBuffer, int indexCount, Index *indexArray, int priority)
{
	PerPrimitiveData *primitive;
	PerPrimitiveData *prev;
	CrcLowerString    searchName(shaderTemplateName);

	for (prev = 0, primitive = firstPrimitive; primitive; prev = primitive, primitive = primitive->next())
	{
		// Order by priority, shaderTemplateName
		// Priorities sorted in descending order.
		// Names sorted in ascending order.
		const int primitivePriority = primitive->getPriority();
		if (primitivePriority > priority)
		{
			// keep moving until we're LTE to the add priority
			continue;
		}
		else if (primitivePriority < priority)
		{
			// we've found our insertion point, stop here
			break;
		}
		else if (primitive->getShaderTemplateName() > searchName)
		{
			// we've got equal priority primitives, order by smallest name first
			break;
		}
	}

	PerPrimitiveData *newData = new PerPrimitiveData(shaderTemplateName, priority, primitiveType, vertexBuffer, indexCount, indexArray, ownsData);
	newData->next() = primitive;
	if (prev)
	{
		prev->next() = newData;
	}
	else
	{
		firstPrimitive = newData;
	}

	++primitiveCount;
}

// ----------------------------------------------------------------------

int ShaderPrimitiveSetWriter::getShaderTemplateCount(void) const
{
	if (!firstPrimitive)
		return 0;

	const PerPrimitiveData *ppd         = firstPrimitive;
	int                     shaderCount = 0;

	while (ppd)
	{
		// loop through all primitives using the same shader template
		const CrcLowerString &shaderTemplateName = ppd->getShaderTemplateName();
		do
		{
			ppd = ppd->next();
		} while (ppd && (ppd->getShaderTemplateName() == shaderTemplateName));

		++shaderCount;
	}

	return shaderCount;
}

// ----------------------------------------------------------------------
/**
 * retrieve the world-axis-aligned bounding box for the vertices
 * contained in this shader primitive set.
 */

void ShaderPrimitiveSetWriter::getBoundingBox(Vector *minVector, Vector *maxVector) const
{
	DEBUG_FATAL(!minVector, ("null minVector arg"));
	DEBUG_FATAL(!maxVector, ("null maxVector arg"));

	Vector  globalMin;
	Vector  globalMax;
	Vector  primMin;
	Vector  primMax;

	if (!firstPrimitive)
	{
		*minVector = Vector::zero;
		*maxVector = Vector::zero;
		return;
	}

	// initialize min-max to that of first primitive
	const SystemVertexBuffer *va = firstPrimitive->getVertexBuffer();

	va->getMinMax(globalMin, globalMax);

	for (const PerPrimitiveData *ppd = firstPrimitive->next(); ppd; ppd = ppd->next())
	{
		// get primitive min-max
		va = ppd->getVertexBuffer();
		va->getMinMax(primMin, primMax);

		// update global min-max
		globalMin.x = std::min(globalMin.x, primMin.x);
		globalMin.y = std::min(globalMin.y, primMin.y);
		globalMin.z = std::min(globalMin.z, primMin.z);

		globalMax.x = std::max(globalMax.x, primMax.x);
		globalMax.y = std::max(globalMax.y, primMax.y);
		globalMax.z = std::max(globalMax.z, primMax.z);
	}

	*minVector = globalMin;
	*maxVector = globalMax;
}

// ----------------------------------------------------------------------
/**
 * get a bounding sphere for the all points contained within the
 * shader primitive set.
 * 
 * This is not guaranteed to be an optimal bounding sphere.  It
 * essentially gets the bounding box and circumscribes a sphere around
 * it.
 */

Sphere ShaderPrimitiveSetWriter::getBoundingSphere(void) const
{
	Vector  minv;
	Vector  maxv;

	getBoundingBox(&minv, &maxv);
	Vector center = (minv + maxv) * CONST_REAL(0.5);
	float radius = maxv.magnitudeBetween(center);

	return Sphere(center,radius);
}

// ----------------------------------------------------------------------

bool ShaderPrimitiveSetWriter::write(Iff *iff) const
{
	DEBUG_FATAL(!iff, ("null iff arg"));

	if(!firstPrimitive)
		return false;

	iff->insertForm(TAG_SPS);
		iff->insertForm(TAG_0001);

			// insert # per shader data items
			const int shaderTemplateCount = getShaderTemplateCount();
			iff->insertChunk(TAG_CNT);
				iff->insertChunkData(static_cast<int32> (shaderTemplateCount));
			iff->exitChunk(TAG_CNT);

			// prepare to write shaders
			const PerPrimitiveData *ppdBookmark;
			const PerPrimitiveData *ppd          = firstPrimitive;
			int                     shaderIndex  = 0;

			while (ppd)
			{
				// save current ppd pointer so we can get back to it
				ppdBookmark = ppd;

				// collect # primitive entries to be drawn using current shader template
				int                   shaderPrimitiveCount = 0;
				const CrcLowerString &shaderTemplateName = ppd->getShaderTemplateName();
				do
				{
					++shaderPrimitiveCount;
					ppd = ppd->next();
				} while (ppd && (ppd->getShaderTemplateName() == shaderTemplateName));

				// write per shader info
				const Tag  formTag = ConvertIntToTag(shaderIndex+1);

				iff->insertForm(formTag);

					// this is misleading: this doesn't imply a static shader
					iff->insertChunk(TAG_NAME);
						iff->insertChunkString(shaderTemplateName.getString());
					iff->exitChunk(TAG_NAME);

					// write primitive count
					iff->insertChunk(TAG_INFO);
						iff->insertChunkData(static_cast<int32>(shaderPrimitiveCount));
					iff->exitChunk(TAG_INFO);

					// write primitives
					ppd = ppdBookmark;
					do
					{
						ppd->write(*iff,hasAlpha);
						ppd = ppd->next();
					} while (ppd && (ppd->getShaderTemplateName() == shaderTemplateName));

				iff->exitForm(formTag);

				++shaderIndex;
			}

		iff->exitForm(TAG_0001);
	iff->exitForm(TAG_SPS);

	return true;
}


// ----------------------------------------------------------------------

bool ShaderPrimitiveSetWriter::write(const char *filename) const
{
	Iff iff(128 * 1024);

	write(&iff);
	return iff.write(filename, true);
}


// ----------------------------------------------------------------------

const ShaderPrimitiveSetWriter::PerPrimitiveData *ShaderPrimitiveSetWriter::getPerPrimitiveData(int index) const
{
	const PerPrimitiveData *ppd;
	int                     i;

	for (i = 0, ppd = firstPrimitive; ppd && (i < index); ++i, ppd = ppd->next())
		{}
	DEBUG_FATAL(!ppd, ("ran out of per primitive datas, index out of range (%d)", index));
	return ppd;
}

// ----------------------------------------------------------------------

const char *ShaderPrimitiveSetWriter::getShaderTemplateName(int primitiveIndex) const
{
	const PerPrimitiveData *ppd = getPerPrimitiveData(primitiveIndex);
	return ppd->getShaderTemplateName().getString();
}

// ----------------------------------------------------------------------

ShaderPrimitiveSetPrimitiveType ShaderPrimitiveSetWriter::getPrimitiveType(int primitiveIndex) const
{
	const PerPrimitiveData *ppd = getPerPrimitiveData(primitiveIndex);
	return ppd->getPrimitiveType();
}

// ----------------------------------------------------------------------

const SystemVertexBuffer *ShaderPrimitiveSetWriter::getVertexBuffer(int primitiveIndex) const
{
	const PerPrimitiveData *ppd = getPerPrimitiveData(primitiveIndex);
	return ppd->getVertexBuffer();
}

// ----------------------------------------------------------------------

int ShaderPrimitiveSetWriter::getIndexCount(int primitiveIndex) const
{
	const PerPrimitiveData *ppd = getPerPrimitiveData(primitiveIndex);
	return ppd->getIndexCount();
}

// ----------------------------------------------------------------------

const Index *ShaderPrimitiveSetWriter::getIndexArray(int primitiveIndex) const
{
	const PerPrimitiveData *ppd = getPerPrimitiveData(primitiveIndex);
	return ppd->getIndexArray();
}

// ======================================================================
