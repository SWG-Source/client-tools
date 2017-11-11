/*
	Based on code by Brad Grantham of SGI.
*/

#ifndef MESHIFIER
#define MESHIFIER

#ifndef TRUE

#define TRUE	1
#define FALSE	0

#endif /* ifndef TRUE */

// error signal
#define MESHIFIER_ERROR -1


/*
 * Change this if you need more or less precision
 */
#define REAL	float


typedef REAL meshVec3[3];
typedef REAL meshVec4[4];


#define X	0	/* indices into vector coordinate or normal */
#define Y	1
#define Z	2


#define R	0	/* indices into texture coordinate */
#define S	1
#define T	2


	/* "R" already defined */
#define G	1	/* green */
#define B	2	/* blue */
#define A	3	/* alpha */


#define MAX_SHARE	32	/* # of edges that can share a vertex */


#define MAX_TRIS	10000	/* # of maximum triangles */


/*
 * On some architectures, shorter strips may perform better than longer
 * strips because parallel transform engines may have to share or duplicate
 * vertices if the engines have small FIFOs.  (e.g. a GE with a 12 vertex
 * FIFO would swallow 12 vertices immediately and then the 13th and on
 * would block until the FIFO was empty, or if another GE is available it
 * would have to copy the previous two vertices to process the next
 * part of the strip.  Here's the maximum strip size to create, in vertices.
 *
 * Currently it tries to do the best job, but you might want to set it to
 * suit your hardware.
 */

// test
// #define MAX_STRIP 6
// #define MAX_STRIP 3
// #define MAX_STRIP 4

#define MAX_STRIP	(MAX_TRIS + 2) 	/* # verts = # tris + 2 */


struct meshifierVertex {
	int v_index; /* index in some other system..., unused by actual meshifier */
    meshVec3	v;			/* coordinate */
    meshVec3	n;			/* normal */
	int c_index; // colour_index
    meshVec4	c;			/* color */
	int t_index; // texture index
    meshVec3	t;			/* texture, but normally only x,y */
    int		conncnt;		/* # of edges to which this belongs */
    int		conn[MAX_SHARE][2];	/* [0] == index of connected vertex */
					/* [1] == index of connecting edge */
};


/*
 * Notice that this implementation won't properly handle more than 2 tris
 * sharing an edge but wouldn't be too hard to extend.
 */

struct meshifierEdge {
    int t[2];			/* triangles sharing this edge */
};

struct meshifierTriangle {
    int	v[3];			/* indices of vertices */
    int	tcnt;			/* # of adjacent triangles */
    int	t[3];			/* adjacent triangles */
    int	done;			/* been inserted into strip? */
    int	next;			/* next triangle in strip */
};

typedef void (*meshBeginStripProc)(meshifierVertex *v1, meshifierVertex *v2);
typedef void (*meshContStripProc)(meshifierVertex *v);
typedef void (*meshEndStripProc)(void);

extern void meshSetProcs(meshBeginStripProc bs, meshContStripProc cs, meshEndStripProc es);
extern int  meshCreateStrips(bool MakeStrips);
extern void meshReset(void);
extern void meshAddVertex(int v_index, meshVec3 v, meshVec3 n, int c_index, meshVec4 c, int t_index, meshVec3 t);
extern void meshAddTriangle(int v1, int v2, int v3);

#endif
