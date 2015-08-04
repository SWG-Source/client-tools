/*

This is a simple stripifier using basic SGI style algorithms appropriate for OpenGL (as opposed to
IRISGL) on the grounds that the complexity of the materials, smoothing groups etc liable to be found
in real data can make sophisticated algorithms little more effective than basic ones.

The source is a modified version of code written by Brad Grantham of SGI.

Changes for this version were made by Neal Tringham, at VX.

The original files are marked "Copyright 1995, 1996 Brad Grantham.
All rights reserved."  I have the right to distribute my changed version 
of the source, but anyone who wants to add further changes and distribute
the result should contact Brad Grantham directly.  (I waive any rights I
may have to my changes, though it would be nice if anyone who used this 
version gave me a mention in the credits somewhere:-))

Brad Grantham's email address is "grantham@hadron.org" (current as of 
February 2000) and his "Meshifier" web pages are at: 
http://tofu.alt.net/~grantham/meshifier/show.html

The core of the stripification algorithm is:

		1) Pick a triangle which is one of the least connected in the merge group.

		2) Walk along a strip formed by picking the next connected triangle (this is done using
			a database of shared edges) until you reach a triangle with no forwards connections.

		3) Reverse the current strip.

		4) Start sgain at the new end (the original start) and walk in the other direction until 
			you again reach an unconnected triangle.

Note that to support single sided rendering all output strips have an anti-clockwise winding (i.e. the first 3 vertices form
an anti-clockwise triangle and later vertices produce triangles of alternate winding).  
In order to support this behaviour, the winding is determined for each triangle
in the strips obtained using the algorithm above, and if the final start triangle (end of the first walk)
is clockwise, the algorithm either "backs up one" to invert the winding or (if there was
only one triangle in the strip) abandons the current strip and produces a
one triangle strip with guaranteed anti-clockwise winding.

*/

#include "FirstEngine.h"
#include "Meshifier.h"

#include <cstdlib>
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <cmath>


/* Vertices */
static int		vertcnt;
static meshifierVertex	verts[MAX_TRIS * 3];


/* Triangles */
static int		tricnt;
static meshifierTriangle	ts[MAX_TRIS];


/* Edges */
static int		edgecnt;
static meshifierEdge		edges[MAX_TRIS * 3];


static void nullVertPair(meshifierVertex *v1, meshifierVertex *v2)
{
	UNREF (v1);
	UNREF (v2);
}


static void nullContStrip(meshifierVertex *v)
{
	UNREF (v);
}


static void nullEndStrip(void)
{
}


static meshBeginStripProc	drawBeginStrip =	nullVertPair;
static meshContStripProc	drawContStrip =		nullContStrip;
static meshEndStripProc		drawEndStrip =		nullEndStrip;


void meshSetProcs(meshBeginStripProc bs, meshContStripProc cs, meshEndStripProc es)
{
    drawBeginStrip = (bs == NULL) ? nullVertPair : bs;
    drawContStrip = (cs == NULL) ? nullContStrip : cs;
    drawEndStrip = (es == NULL) ? nullEndStrip : es;
}


static int notSharedVertex(int	t1, int	t2)
{
    if((ts[t1].v[0] != ts[t2].v[0])  &&
        (ts[t1].v[0] != ts[t2].v[1])   &&
        (ts[t1].v[0] != ts[t2].v[2]))
    {
        return(ts[t1].v[0]);
    }

    if((ts[t1].v[1] != ts[t2].v[0])  &&
        (ts[t1].v[1] != ts[t2].v[1])   &&
        (ts[t1].v[1] != ts[t2].v[2]))
    {
        return(ts[t1].v[1]);
    }

    /*
     * Well, must be the last one; if they shared all three, then they 
     *  would be the same triangle.
     */

    return(ts[t1].v[2]);
}


static int firstSharedVertex(int t1, int t2)
{
    if((ts[t1].v[0] == ts[t2].v[0])  ||
        (ts[t1].v[0] == ts[t2].v[1]) ||
        (ts[t1].v[0] == ts[t2].v[2]))
    {
        return(ts[t1].v[0]);
    }

    if((ts[t1].v[1] == ts[t2].v[0]) ||
        (ts[t1].v[1] == ts[t2].v[1]) ||
        (ts[t1].v[1] == ts[t2].v[2]))
    {
        return(ts[t1].v[1]);
    }

    /*
     * Well, can't be either; if this was the first shared, it's the ONLY
     * one shared.
     */

	return MESHIFIER_ERROR;
}


static int secondSharedVertex(int t1, int t2)
{
    if((ts[t1].v[2] == ts[t2].v[0])  ||
        (ts[t1].v[2] == ts[t2].v[1]) ||
        (ts[t1].v[2] == ts[t2].v[2]))
    {
        return(ts[t1].v[2]);
    }

    if((ts[t1].v[1] == ts[t2].v[0]) ||
        (ts[t1].v[1] == ts[t2].v[1]) ||
        (ts[t1].v[1] == ts[t2].v[2]))
    {
        return(ts[t1].v[1]);
    }

    /*
     * Well, can't be either; if we think the first vertex was the
     * SECOND shared, something is way out of whack.
     */
	return MESHIFIER_ERROR;
}



static int triangleUsesVertex(int t, int v)
{
    if((v == ts[t].v[0]) || (v == ts[t].v[1]) || (v == ts[t].v[2]))
        return(1);
    return(0);
}


static int followStrip(int strip[], int *count, int last, int* final)
{
    int	i;
    int	next;

	// winding = 0 is clockwise, winding = 1 is anti-clockwise
	int winding = 0;

    while(*count < MAX_STRIP){

        /* find next triangle to add */
        next = -1;
        if(*count == 0){
                /* for 2nd tri, just pick one. */
            for(i = 0; i < ts[last].tcnt; i++)
                if(!ts[ts[last].t[i]].done){
                    next = ts[last].t[i];
                    break;
                }
            if(next != -1) {

                strip[0] = notSharedVertex(last, next);
                strip[1] = firstSharedVertex(last, next);

				if (strip[1] == MESHIFIER_ERROR)
					{
					return strip[1];
					}

                strip[2] = secondSharedVertex(last, next);

				if (strip[2] == MESHIFIER_ERROR)
					{
					return strip[2];
					}

				// decode initial winding
				if ((strip[0] == ts[last].v[0]) &&
					(strip[1] == ts[last].v[1]) &&
					(strip[2] == ts[last].v[2]))
					{
					winding = 1; // anti-clock
					}
				else if ((strip[0] == ts[last].v[1]) &&
						 (strip[1] == ts[last].v[0]) &&
						 (strip[2] == ts[last].v[2]))
					{
					winding = 0; // clock
					}		
				else if ((strip[0] == ts[last].v[2]) &&
						 (strip[1] == ts[last].v[0]) &&
						 (strip[2] == ts[last].v[1]))
					{
					winding = 1; // anti-clock
					}		
				else
					{
					printf("winding analysis failure\n");
					exit(1);
					}

                *count = 3;
            }else{
                strip[0] = ts[last].v[0];
                strip[1] = ts[last].v[1];
                strip[2] = ts[last].v[2];

				winding = 1; // anti-clock
                *count = 3;
            }
        }else{
            /* third and later tris must share prev two verts */
            for(i = 0; i < ts[last].tcnt; i++)
                if(!ts[ts[last].t[i]].done &&
                    triangleUsesVertex(ts[last].t[i], strip[*count - 2]) &&
                    triangleUsesVertex(ts[last].t[i], strip[*count - 1]))
                {
                    next = ts[last].t[i];
                    break;
                }
        }

        if(next == -1)
            break;

        strip[*count] = notSharedVertex(next, last);

		// alternate winding
		winding ^= 1;

        *count = *count + 1;

        ts[next].done = 1;

        last = next;
    }

	*final = last;

	return winding; // no error
}


static int getNextStrip(int start, int *cnt, int strip[], bool MakeStrips)
{
    int	i;
    int	istrip[MAX_STRIP];
	int final;
    int 	min, mincnt;

	if (MakeStrips)
	{
    /* do slightly more wise strips; pick one of least connected
        triangles */

    mincnt = 4;
    min = -1;	/* so compiler doesn't complain */
    for(i = 0; i < tricnt; i++)
        if((! ts[i].done) && (ts[i].tcnt < mincnt))
	{
	    min = i;
	    mincnt = ts[i].tcnt;
        }

    if(min != -1)
    {
        start = min;
    }
    else
    {
		return MESHIFIER_ERROR;
    }

    ts[start].done = 1;
    *cnt = 0;

	int winding;

    winding = followStrip(istrip, cnt, start, &final);

	if (winding == MESHIFIER_ERROR)
		{
		return winding;
		}

    for(i = 0; i < *cnt; i++)
        strip[i] = istrip[*cnt - i - 1];

	// winding should be inverted by strip reversal
	winding ^= 1;

	// the value of winding is the value of the winding order
	// for the final triangle in the strip, which is the start
	// after the reversal of the pass 1 data

	// if winding is clockwise, back up one and undo "done" marker on last triangle...
	// backing up should flip winding automatically
	if (winding == 0)
		{
		// test for infinite looping...
		if (final != start)
			{
			// recopy istrip into strip, starting at i=1 to allow
			// for skipping old end (new start)
		    for(i = 1; i < *cnt; i++)
				{
    		    strip[i-1] = istrip[*cnt - i - 1];
				}

			// back up count
			*cnt -= 1;

			// mark triangle corresponding to backed up vertex as not done
			ts[final].done = 0;

			// pass 2 - follow reversed strip out in the other direction
		    winding = followStrip(strip, cnt, start, &final);

			if (winding == MESHIFIER_ERROR)
				{
				return winding;
				}
			}
		else
			{
			// rebuild single triangle strip in anti-clockwise order
			strip[0] = ts[start].v[0];
			strip[1] = ts[start].v[1];
			strip[2] = ts[start].v[2];

			}
		}
	else
		{
		// pass 2 - follow reversed strip out in the other direction
		winding = followStrip(strip, cnt, start, &final);

		if (winding == MESHIFIER_ERROR)
			{
			return winding;
			}
		}
	}
	else
	{

    *cnt = 3;
    strip[0] = ts[start].v[0];
    strip[1] = ts[start].v[1];
    strip[2] = ts[start].v[2];
    ts[start].done = 1;

	}

	return 0; // no error
}


int meshCreateStrips(bool MakeStrips)
{
    int		done;
    int		first;
    int		sl, sv[MAX_STRIP*3];
    int		i;
    int		meshcnt;

    done = 0;

    meshcnt = 0;

    do{
        /* Find out if we are done (no more triangles left) and */
        /*  if so, use that first triangle as a starting suggestion */
        for(first = 0; ts[first].done && first < tricnt; first++);

        if(first == tricnt){
            break;	/* all done; no more triangles */
        }

        /* extract the next strip out of the data set */
        int ret = getNextStrip(first, &sl, sv, MakeStrips);

		if (ret == MESHIFIER_ERROR)
			{
			return ret;
			}

        meshcnt ++;

        /* draw the triangles in each strip */
	drawBeginStrip(&verts[sv[0]], &verts[sv[1]]);

        for(i = 2; i < sl; i++)
	    drawContStrip(&verts[sv[i]]);

	drawEndStrip();

    }while(!done);


    return(meshcnt);
}


static void addEdge(int tnum, int vnum1, int vnum2)
{
    meshifierTriangle		*t, *t2;
    meshifierVertex		*v1, *v2;
    int			e;	/* index of edge record */

    t = &ts[tnum];
    v1 = &verts[vnum1];
    v2 = &verts[vnum2];

    /* set up edge between v1 and v2 unless one exists */

    for(e = 0; (e < v1->conncnt) && (v1->conn[e][0] != vnum2); e++);

    if(e < v1->conncnt){
            /* found existing edge */
        e = v1->conn[e][1];
        edges[e].t[1] = tnum;
        t2 = &ts[edges[e].t[0]];
        t->t[t->tcnt++] = edges[e].t[0];
        t2->t[t2->tcnt++] = tnum;

    }else{
            /* have to make new edge */

        e = edgecnt++;
        v1->conn[v1->conncnt][0] = vnum2;
        v1->conn[v1->conncnt++][1] = e;
        v2->conn[v2->conncnt][0] = vnum1;
        v2->conn[v2->conncnt++][1] = e;
        edges[e].t[0] = tnum;
    }
}


static void prepareTriangle(int t)
{
    /* set up edge connectivity */

    addEdge(t, ts[t].v[0], ts[t].v[1]);
    addEdge(t, ts[t].v[1], ts[t].v[2]);
    addEdge(t, ts[t].v[2], ts[t].v[0]);
}


void meshReset(void)
{
	// code does not reset properly; emulate start conditions for
	// program run (bss segment cleared)
	// no doubt this could be done rather more elegantly and faster,
	// but it is a tool and this will work...
	memset(&verts[0], 0, vertcnt * sizeof(meshifierVertex));
	memset(&ts[0], 0, tricnt * sizeof(meshifierTriangle));
	memset(&edges[0], 0, edgecnt * sizeof(meshifierEdge));

    vertcnt = 0;
    edgecnt = 0;
    tricnt = 0;
}


void meshAddVertex(int v_index, meshVec3 v, meshVec3 n, int c_index, 
					meshVec4 c, int t_index, meshVec3 t)
{
	// pass through to make Neal's stuff easier...
	verts[vertcnt].v_index = v_index;
	verts[vertcnt].c_index = c_index;
	verts[vertcnt].t_index = t_index;

    verts[vertcnt].v[X] = v[X];
    verts[vertcnt].v[Y] = v[Y];
    verts[vertcnt].v[Z] = v[Z];

    verts[vertcnt].n[X] = n[X];
    verts[vertcnt].n[Y] = n[Y];
    verts[vertcnt].n[Z] = n[Z];

    verts[vertcnt].c[R] = c[R];
    verts[vertcnt].c[G] = c[G];
    verts[vertcnt].c[B] = c[B];
    verts[vertcnt].c[A] = c[A];

    verts[vertcnt].t[R] = t[R];
    verts[vertcnt].t[S] = t[S];
    verts[vertcnt].t[T] = t[T];

    verts[vertcnt].conncnt = 0;

    vertcnt ++;
}


void meshAddTriangle(int v1, int v2, int v3)
{
    ts[tricnt].v[0] = v1;
    ts[tricnt].v[1] = v2;
    ts[tricnt].v[2] = v3;
    ts[tricnt].done = 0;

    prepareTriangle(tricnt);

    tricnt ++;
}

