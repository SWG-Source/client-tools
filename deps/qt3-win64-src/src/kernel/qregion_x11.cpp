/****************************************************************************
** $Id: qregion_x11.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of QRegion class for X11
**
** Created : 940729
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech ASA of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Unix/X11 may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qregion.h"
#include "qpointarray.h"
#include "qbuffer.h"
#include "qimage.h"
#include "qbitmap.h"
#include "qt_x11_p.h"

#include <stdlib.h>

// inline QRect::setCoords
inline void qt_setCoords( QRect *r, int xp1, int yp1, int xp2, int yp2 )
{
    r->x1 = (QCOORD)xp1;
    r->y1 = (QCOORD)yp1;
    r->x2 = (QCOORD)xp2;
    r->y2 = (QCOORD)yp2;
}

/*
 *   clip region
 */

struct QRegionPrivate {
    int numRects;
    QMemArray<QRect> rects;
    QRect extents;

    QRegionPrivate() { numRects = 0; }
    QRegionPrivate( const QRect &r ) : rects(1) {
	numRects = 1;
	rects[0] = r;
	extents = r;
    }

    QRegionPrivate( const QRegionPrivate &r ) {
	rects = r.rects.copy();
	numRects = r.numRects;
	extents = r.extents;
    }

    QRegionPrivate &operator=( const QRegionPrivate &r ) {
	rects = r.rects.copy();
	numRects = r.numRects;
	extents = r.extents;
	return *this;
    }

};


static void UnionRegion(QRegionPrivate *reg1, QRegionPrivate *reg2, QRegionPrivate *newReg);
static void IntersectRegion(QRegionPrivate *reg1, QRegionPrivate *reg2, register QRegionPrivate *newReg);
static void miRegionOp(register QRegionPrivate *newReg, QRegionPrivate *reg1, QRegionPrivate *reg2,
	void (*overlapFunc)(...),
	void (*nonOverlap1Func)(...),
	void (*nonOverlap2Func)(...));
#define RectangleOut 0
#define RectangleIn  1
#define RectanglePart 2
#define EvenOddRule             0
#define WindingRule             1

// START OF region.h extract
/* $XConsortium: region.h,v 11.14 94/04/17 20:22:20 rws Exp $ */
/************************************************************************

Copyright (c) 1987  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.


Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

************************************************************************/

#ifndef _XREGION_H
#define _XREGION_H

#include <limits.h>

#ifndef MAX
#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif


/*  1 if two BOXs overlap.
 *  0 if two BOXs do not overlap.
 *  Remember, x2 and y2 are not in the region
 */
#define EXTENTCHECK(r1, r2) \
	((r1)->right() >= (r2)->left() && \
	 (r1)->left() <= (r2)->right() && \
	 (r1)->bottom() >= (r2)->top() && \
	 (r1)->top() <= (r2)->bottom())

/*
 *  update region extents
 */
#define EXTENTS(r,idRect){\
            if((r)->left() < (idRect)->extents.left())\
              (idRect)->extents.setLeft( (r)->left() );\
            if((r)->top() < (idRect)->extents.top())\
              (idRect)->extents.setTop( (r)->top() );\
            if((r)->right() > (idRect)->extents.right())\
              (idRect)->extents.setRight( (r)->right() );\
            if((r)->bottom() > (idRect)->extents.bottom())\
              (idRect)->extents.setBottom( (r)->bottom() );\
        }

/*
 *   Check to see if there is enough memory in the present region.
 */
#define MEMCHECK(reg, rect, firstrect){\
        if ((reg)->numRects >= (int)((reg)->rects.size()-1)){\
	  firstrect.resize(firstrect.size() * 2); \
	  (rect) = (firstrect).data() + (reg)->numRects;\
	}\
      }


#define EMPTY_REGION(pReg) pReg->numRects = 0

#define REGION_NOT_EMPTY(pReg) pReg->numRects

/*
 * number of points to buffer before sending them off
 * to scanlines() :  Must be an even number
 */
#define NUMPTSTOBUFFER 200

/*
 * used to allocate buffers for points and link
 * the buffers together
 */
typedef struct _POINTBLOCK {
    QPoint pts[NUMPTSTOBUFFER];
    struct _POINTBLOCK *next;
} POINTBLOCK;

#endif
// END OF region.h extract

// START OF Region.c extract
/* $XConsortium: Region.c /main/30 1996/10/22 14:21:24 kaleb $ */
/************************************************************************

Copyright (c) 1987, 1988  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.


Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

************************************************************************/
/*
 * The functions in this file implement the Region abstraction, similar to one
 * used in the X11 sample server. A Region is simply an area, as the name
 * implies, and is implemented as a "y-x-banded" array of rectangles. To
 * explain: Each Region is made up of a certain number of rectangles sorted
 * by y coordinate first, and then by x coordinate.
 *
 * Furthermore, the rectangles are banded such that every rectangle with a
 * given upper-left y coordinate (y1) will have the same lower-right y
 * coordinate (y2) and vice versa. If a rectangle has scanlines in a band, it
 * will span the entire vertical distance of the band. This means that some
 * areas that could be merged into a taller rectangle will be represented as
 * several shorter rectangles to account for shorter rectangles to its left
 * or right but within its "vertical scope".
 *
 * An added constraint on the rectangles is that they must cover as much
 * horizontal area as possible. E.g. no two rectangles in a band are allowed
 * to touch.
 *
 * Whenever possible, bands will be merged together to cover a greater vertical
 * distance (and thus reduce the number of rectangles). Two bands can be merged
 * only if the bottom of one touches the top of the other and they have
 * rectangles in the same places (of the same width, of course). This maintains
 * the y-x-banding that's so nice to have...
 */
/* $XFree86: xc/lib/X11/Region.c,v 1.1.1.2.2.2 1998/10/04 15:22:50 hohndel Exp $ */

typedef void (*voidProcp)(...);


static
void UnionRectWithRegion(register const QRect *rect, QRegionPrivate *source, QRegionPrivate *dest)
{
    QRegionPrivate region;

    if (!rect->width() || !rect->height())
	return;
    region.rects.resize(1);
    region.numRects = 1;
    region.rects[0] = *rect;
    region.extents = *rect;

    UnionRegion(&region, source, dest);
    return;
}

/*-
 *-----------------------------------------------------------------------
 * miSetExtents --
 *	Reset the extents of a region to what they should be. Called by
 *	miSubtract and miIntersect b/c they can't figure it out along the
 *	way or do so easily, as miUnion can.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The region's 'extents' structure is overwritten.
 *
 *-----------------------------------------------------------------------
 */
static void
miSetExtents (QRegionPrivate *pReg)
{
    register QRect	*pBox,
			*pBoxEnd,
			*pExtents;

    if (pReg->numRects == 0)
    {
	qt_setCoords(&pReg->extents, 0, 0, 0, 0);
	return;
    }

    pExtents = &pReg->extents;
    pBox = pReg->rects.data();
    pBoxEnd = &pBox[pReg->numRects - 1];

    /*
     * Since pBox is the first rectangle in the region, it must have the
     * smallest y1 and since pBoxEnd is the last rectangle in the region,
     * it must have the largest y2, because of banding. Initialize x1 and
     * x2 from  pBox and pBoxEnd, resp., as good things to initialize them
     * to...
     */
    pExtents->setLeft( pBox->left() );
    pExtents->setTop( pBox->top() );
    pExtents->setRight( pBoxEnd->right() );
    pExtents->setBottom( pBoxEnd->bottom() );

    Q_ASSERT(pExtents->top() <= pExtents->bottom());
    while (pBox <= pBoxEnd)
    {
	if (pBox->left() < pExtents->left())
	{
	    pExtents->setLeft( pBox->left() );
	}
	if (pBox->right() > pExtents->right())
	{
	    pExtents->setRight( pBox->right() );
	}
	pBox++;
    }
    Q_ASSERT(pExtents->left() <= pExtents->right());
}


/* TranslateRegion(pRegion, x, y)
   translates in place
   added by raymond
*/

static
int
OffsetRegion(register QRegionPrivate *pRegion, register int x, register int y)
{
    register int nbox;
    register QRect *pbox;

    pbox = pRegion->rects.data();
    nbox = pRegion->numRects;

    while(nbox--)
    {
	pbox->moveBy(x, y);
	pbox++;
    }
    pRegion->extents.moveBy(x, y);
    return 1;
}

/*======================================================================
 *	    Region Intersection
 *====================================================================*/
/*-
 *-----------------------------------------------------------------------
 * miIntersectO --
 *	Handle an overlapping band for miIntersect.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Rectangles may be added to the region.
 *
 *-----------------------------------------------------------------------
 */
/* static void*/
static
int
miIntersectO (register QRegionPrivate *pReg, register QRect *r1, QRect *r1End,
	register QRect *r2, QRect *r2End, int y1, int y2)
{
    register int  	x1;
    register int  	x2;
    register QRect	*pNextRect;

    pNextRect = pReg->rects.data() + pReg->numRects;

    while ((r1 != r1End) && (r2 != r2End))
    {
	x1 = QMAX(r1->left(),r2->left());
	x2 = QMIN(r1->right(),r2->right());

	/*
	 * If there's any overlap between the two rectangles, add that
	 * overlap to the new region.
	 * There's no need to check for subsumption because the only way
	 * such a need could arise is if some region has two rectangles
	 * right next to each other. Since that should never happen...
	 */
	if (x1 <= x2)
	{
	    Q_ASSERT(y1<=y2);

	    MEMCHECK(pReg, pNextRect, pReg->rects)
	    qt_setCoords( pNextRect, x1, y1, x2, y2 );
	    pReg->numRects++;
	    pNextRect++;
	}

	/*
	 * Need to advance the pointers. Shift the one that extends
	 * to the right the least, since the other still has a chance to
	 * overlap with that region's next rectangle, if you see what I mean.
	 */
	if (r1->right() < r2->right())
	{
	    r1++;
	}
	else if (r2->right() < r1->right())
	{
	    r2++;
	}
	else
	{
	    r1++;
	    r2++;
	}
    }
    return 0;	/* lint */
}

static
void
IntersectRegion(QRegionPrivate *reg1, QRegionPrivate *reg2, register QRegionPrivate *newReg)
{
   /* check for trivial reject */
    if ( (!(reg1->numRects)) || (!(reg2->numRects))  ||
	(!EXTENTCHECK(&reg1->extents, &reg2->extents)))
        newReg->numRects = 0;
    else
	miRegionOp (newReg, reg1, reg2,
    		(voidProcp) miIntersectO, (voidProcp) NULL, (voidProcp) NULL);

    /*
     * Can't alter newReg's extents before we call miRegionOp because
     * it might be one of the source regions and miRegionOp depends
     * on the extents of those regions being the same. Besides, this
     * way there's no checking against rectangles that will be nuked
     * due to coalescing, so we have to examine fewer rectangles.
     */
    miSetExtents(newReg);
    return;
}

/*======================================================================
 *	    Generic Region Operator
 *====================================================================*/

/*-
 *-----------------------------------------------------------------------
 * miCoalesce --
 *	Attempt to merge the boxes in the current band with those in the
 *	previous one. Used only by miRegionOp.
 *
 * Results:
 *	The new index for the previous band.
 *
 * Side Effects:
 *	If coalescing takes place:
 *	    - rectangles in the previous band will have their y2 fields
 *	      altered.
 *	    - pReg->numRects will be decreased.
 *
 *-----------------------------------------------------------------------
 */
/* static int*/
static
int
miCoalesce (register QRegionPrivate *pReg, int prevStart, int curStart)
    //Region	pReg;	    	/* Region to coalesce */
    //prevStart;  	/* Index of start of previous band */
    //curStart;   	/* Index of start of current band */
{
    register QRect	*pPrevBox;   	/* Current box in previous band */
    register QRect	*pCurBox;    	/* Current box in current band */
    register QRect	*pRegEnd;    	/* End of region */
    int	    	  	curNumRects;	/* Number of rectangles in current
					 * band */
    int	    	  	prevNumRects;	/* Number of rectangles in previous
					 * band */
    int	    	  	bandY1;	    	/* Y1 coordinate for current band */

    pRegEnd = pReg->rects.data() + pReg->numRects;

    pPrevBox = pReg->rects.data() + prevStart;
    prevNumRects = curStart - prevStart;

    /*
     * Figure out how many rectangles are in the current band. Have to do
     * this because multiple bands could have been added in miRegionOp
     * at the end when one region has been exhausted.
     */
    pCurBox = pReg->rects.data() + curStart;
    bandY1 = pCurBox->top();
    for (curNumRects = 0;
	 (pCurBox != pRegEnd) && (pCurBox->top() == bandY1);
	 curNumRects++)
    {
	pCurBox++;
    }

    if (pCurBox != pRegEnd)
    {
	/*
	 * If more than one band was added, we have to find the start
	 * of the last band added so the next coalescing job can start
	 * at the right place... (given when multiple bands are added,
	 * this may be pointless -- see above).
	 */
	pRegEnd--;
	while ((pRegEnd-1)->top() == pRegEnd->top())
	{
	    pRegEnd--;
	}
	curStart = pRegEnd - pReg->rects.data();
	pRegEnd = pReg->rects.data() + pReg->numRects;
    }

    if ((curNumRects == prevNumRects) && (curNumRects != 0)) {
	pCurBox -= curNumRects;
	/*
	 * The bands may only be coalesced if the bottom of the previous
	 * matches the top scanline of the current.
	 */
	if (pPrevBox->bottom() == pCurBox->top() - 1)
	{
	    /*
	     * Make sure the bands have boxes in the same places. This
	     * assumes that boxes have been added in such a way that they
	     * cover the most area possible. I.e. two boxes in a band must
	     * have some horizontal space between them.
	     */
	    do
	    {
		if ((pPrevBox->left() != pCurBox->left()) ||
		    (pPrevBox->right() != pCurBox->right()))
		{
		    /*
		     * The bands don't line up so they can't be coalesced.
		     */
		    return (curStart);
		}
		pPrevBox++;
		pCurBox++;
		prevNumRects -= 1;
	    } while (prevNumRects != 0);

	    pReg->numRects -= curNumRects;
	    pCurBox -= curNumRects;
	    pPrevBox -= curNumRects;

	    /*
	     * The bands may be merged, so set the bottom y of each box
	     * in the previous band to that of the corresponding box in
	     * the current band.
	     */
	    do
	    {
		pPrevBox->setBottom( pCurBox->bottom() );
		pPrevBox++;
		pCurBox++;
		curNumRects -= 1;
	    } while (curNumRects != 0);

	    /*
	     * If only one band was added to the region, we have to backup
	     * curStart to the start of the previous band.
	     *
	     * If more than one band was added to the region, copy the
	     * other bands down. The assumption here is that the other bands
	     * came from the same region as the current one and no further
	     * coalescing can be done on them since it's all been done
	     * already... curStart is already in the right place.
	     */
	    if (pCurBox == pRegEnd)
	    {
		curStart = prevStart;
	    }
	    else
	    {
		do
		{
		    *pPrevBox++ = *pCurBox++;
		} while (pCurBox != pRegEnd);
	    }

	}
    }
    return (curStart);
}

/*-
 *-----------------------------------------------------------------------
 * miRegionOp --
 *	Apply an operation to two regions. Called by miUnion, miInverse,
 *	miSubtract, miIntersect...
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The new region is overwritten.
 *
 * Notes:
 *	The idea behind this function is to view the two regions as sets.
 *	Together they cover a rectangle of area that this function divides
 *	into horizontal bands where points are covered only by one region
 *	or by both. For the first case, the nonOverlapFunc is called with
 *	each the band and the band's upper and lower extents. For the
 *	second, the overlapFunc is called to process the entire band. It
 *	is responsible for clipping the rectangles in the band, though
 *	this function provides the boundaries.
 *	At the end of each band, the new region is coalesced, if possible,
 *	to reduce the number of rectangles in the region.
 *
 *-----------------------------------------------------------------------
 */
/* static void*/
static void
miRegionOp(register QRegionPrivate *newReg, QRegionPrivate *reg1, QRegionPrivate *reg2,
	void (*overlapFunc)(...),
	void (*nonOverlap1Func)(...),
	void (*nonOverlap2Func)(...))
    //register Region 	newReg;	    	    	/* Place to store result */
    //Region	  	reg1;	    	    	/* First region in operation */
    //Region	  	reg2;	    	    	/* 2d region in operation */
    //void    	  	(*overlapFunc)();   	/* Function to call for over-
						 //* lapping bands */
    //void    	  	(*nonOverlap1Func)();	/* Function to call for non-
						 //* overlapping bands in region
						 //* 1 */
    //void    	  	(*nonOverlap2Func)();	/* Function to call for non-
						 //* overlapping bands in region
						 //* 2 */
{
    register QRect	*r1; 	    	    	/* Pointer into first region */
    register QRect	*r2; 	    	    	/* Pointer into 2d region */
    QRect  	  	*r1End;	    	    	/* End of 1st region */
    QRect  	  	*r2End;	    	    	/* End of 2d region */
    register int  	ybot;	    	    	/* Bottom of intersection */
    register int  	ytop;	    	    	/* Top of intersection */
    int	    	  	prevBand;   	    	/* Index of start of
						 * previous band in newReg */
    int	    	  	curBand;    	    	/* Index of start of current
						 * band in newReg */
    register QRect 	*r1BandEnd;  	    	/* End of current band in r1 */
    register QRect 	*r2BandEnd;  	    	/* End of current band in r2 */
    int     	  	top;	    	    	/* Top of non-overlapping
						 * band */
    int     	  	bot;	    	    	/* Bottom of non-overlapping
						 * band */

    /*
     * Initialization:
     *	set r1, r2, r1End and r2End appropriately, preserve the important
     * parts of the destination region until the end in case it's one of
     * the two source regions, then mark the "new" region empty, allocating
     * another array of rectangles for it to use.
     */
    r1 = reg1->rects.data();
    r2 = reg2->rects.data();
    r1End = r1 + reg1->numRects;
    r2End = r2 + reg2->numRects;

    QMemArray<QRect> oldRects = newReg->rects;

    newReg->rects.detach();
    EMPTY_REGION(newReg);

    /*
     * Allocate a reasonable number of rectangles for the new region. The idea
     * is to allocate enough so the individual functions don't need to
     * reallocate and copy the array, which is time consuming, yet we don't
     * have to worry about using too much memory. I hope to be able to
     * nuke the realloc() at the end of this function eventually.
     */
    newReg->rects.resize( QMAX(reg1->numRects,reg2->numRects) * 2 );

    /*
     * Initialize ybot and ytop.
     * In the upcoming loop, ybot and ytop serve different functions depending
     * on whether the band being handled is an overlapping or non-overlapping
     * band.
     * 	In the case of a non-overlapping band (only one of the regions
     * has points in the band), ybot is the bottom of the most recent
     * intersection and thus clips the top of the rectangles in that band.
     * ytop is the top of the next intersection between the two regions and
     * serves to clip the bottom of the rectangles in the current band.
     *	For an overlapping band (where the two regions intersect), ytop clips
     * the top of the rectangles of both regions and ybot clips the bottoms.
     */
    if (reg1->extents.top() < reg2->extents.top())
	ybot = reg1->extents.top() - 1;
    else
	ybot = reg2->extents.top() - 1;

    /*
     * prevBand serves to mark the start of the previous band so rectangles
     * can be coalesced into larger rectangles. qv. miCoalesce, above.
     * In the beginning, there is no previous band, so prevBand == curBand
     * (curBand is set later on, of course, but the first band will always
     * start at index 0). prevBand and curBand must be indices because of
     * the possible expansion, and resultant moving, of the new region's
     * array of rectangles.
     */
    prevBand = 0;

    do
    {
	curBand = newReg->numRects;

	/*
	 * This algorithm proceeds one source-band (as opposed to a
	 * destination band, which is determined by where the two regions
	 * intersect) at a time. r1BandEnd and r2BandEnd serve to mark the
	 * rectangle after the last one in the current band for their
	 * respective regions.
	 */
	r1BandEnd = r1;
	while ((r1BandEnd != r1End) && (r1BandEnd->top() == r1->top()))
	{
	    r1BandEnd++;
	}

	r2BandEnd = r2;
	while ((r2BandEnd != r2End) && (r2BandEnd->top() == r2->top()))
	{
	    r2BandEnd++;
	}

	/*
	 * First handle the band that doesn't intersect, if any.
	 *
	 * Note that attention is restricted to one band in the
	 * non-intersecting region at once, so if a region has n
	 * bands between the current position and the next place it overlaps
	 * the other, this entire loop will be passed through n times.
	 */
	if (r1->top() < r2->top())
	{
	    top = QMAX(r1->top(),ybot+1);
	    bot = QMIN(r1->bottom(),r2->top()-1);

	    if ((nonOverlap1Func != (voidProcp)NULL) && bot >= top)
	    {
		(* nonOverlap1Func) (newReg, r1, r1BandEnd, top, bot);
	    }

	    ytop = r2->top();
	}
	else if (r2->top() < r1->top())
	{
	    top = QMAX(r2->top(),ybot+1);
	    bot = QMIN(r2->bottom(),r1->top()-1);

	    if ((nonOverlap2Func != (voidProcp)NULL) && bot >= top)
	    {
		(* nonOverlap2Func) (newReg, r2, r2BandEnd, top, bot);
	    }

	    ytop = r1->top();
	}
	else
	{
	    ytop = r1->top();
	}

	/*
	 * If any rectangles got added to the region, try and coalesce them
	 * with rectangles from the previous band. Note we could just do
	 * this test in miCoalesce, but some machines incur a not
	 * inconsiderable cost for function calls, so...
	 */
	if (newReg->numRects != curBand)
	{
	    prevBand = miCoalesce (newReg, prevBand, curBand);
	}

	/*
	 * Now see if we've hit an intersecting band. The two bands only
	 * intersect if ybot >= ytop
	 */
	ybot = QMIN(r1->bottom(), r2->bottom());
	curBand = newReg->numRects;
	if (ybot >= ytop)
	{
	    (* overlapFunc) (newReg, r1, r1BandEnd, r2, r2BandEnd, ytop, ybot);

	}

	if (newReg->numRects != curBand)
	{
	    prevBand = miCoalesce (newReg, prevBand, curBand);
	}

	/*
	 * If we've finished with a band (y2 == ybot) we skip forward
	 * in the region to the next band.
	 */
	if (r1->bottom() == ybot)
	{
	    r1 = r1BandEnd;
	}
	if (r2->bottom() == ybot)
	{
	    r2 = r2BandEnd;
	}
    } while ((r1 != r1End) && (r2 != r2End));

    /*
     * Deal with whichever region still has rectangles left.
     */
    curBand = newReg->numRects;
    if (r1 != r1End)
    {
	if (nonOverlap1Func != (voidProcp)NULL)
	{
	    do
	    {
		r1BandEnd = r1;
		while ((r1BandEnd < r1End) && (r1BandEnd->top() == r1->top()))
		{
		    r1BandEnd++;
		}
		(* nonOverlap1Func) (newReg, r1, r1BandEnd,
				     QMAX(r1->top(),ybot+1), r1->bottom());
		r1 = r1BandEnd;
	    } while (r1 != r1End);
	}
    }
    else if ((r2 != r2End) && (nonOverlap2Func != (voidProcp)NULL))
    {
	do
	{
	    r2BandEnd = r2;
	    while ((r2BandEnd < r2End) && (r2BandEnd->top() == r2->top()))
	    {
		 r2BandEnd++;
	    }
	    (* nonOverlap2Func) (newReg, r2, r2BandEnd,
				QMAX(r2->top(),ybot+1), r2->bottom());
	    r2 = r2BandEnd;
	} while (r2 != r2End);
    }

    if (newReg->numRects != curBand)
    {
	(void) miCoalesce (newReg, prevBand, curBand);
    }

    /*
     * A bit of cleanup. To keep regions from growing without bound,
     * we shrink the array of rectangles to match the new number of
     * rectangles in the region. This never goes to 0, however...
     *
     * Only do this stuff if the number of rectangles allocated is more than
     * twice the number of rectangles in the region (a simple optimization...).
     */
    if (newReg->numRects < (int)(newReg->rects.size() >> 1))
    {
	if (REGION_NOT_EMPTY(newReg))
	{
	    newReg->rects.resize(newReg->numRects);
	}
	else
	{
	    /*
	     * No point in doing the extra work involved in an realloc if
	     * the region is empty
	     */
	    newReg->rects.resize(1);
	}
    }
    return;
}


/*======================================================================
 *	    Region Union
 *====================================================================*/

/*-
 *-----------------------------------------------------------------------
 * miUnionNonO --
 *	Handle a non-overlapping band for the union operation. Just
 *	Adds the rectangles into the region. Doesn't have to check for
 *	subsumption or anything.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	pReg->numRects is incremented and the final rectangles overwritten
 *	with the rectangles we're passed.
 *
 *-----------------------------------------------------------------------
 */
/* static void*/
static
int
miUnionNonO (register QRegionPrivate *pReg, register QRect * r,
	QRect * rEnd, register int y1, register int y2)
{
    register QRect *	pNextRect;

    pNextRect = pReg->rects.data() + pReg->numRects;

    Q_ASSERT(y1 <= y2);

    while (r != rEnd)
    {
	Q_ASSERT(r->left() <= r->right());
	MEMCHECK(pReg, pNextRect, pReg->rects)
	qt_setCoords( pNextRect, r->left(), y1, r->right(), y2 );
	pReg->numRects++;
	pNextRect++;

	r++;
    }
    return 0;	/* lint */
}


/*-
 *-----------------------------------------------------------------------
 * miUnionO --
 *	Handle an overlapping band for the union operation. Picks the
 *	left-most rectangle each time and merges it into the region.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Rectangles are overwritten in pReg->rects and pReg->numRects will
 *	be changed.
 *
 *-----------------------------------------------------------------------
 */

/* static void*/
static
int
miUnionO (register QRegionPrivate *pReg, register QRect *r1, QRect *r1End,
	register QRect *r2, QRect *r2End, register int y1, register int y2)
{
    register QRect *pNextRect;

    pNextRect = pReg->rects.data() + pReg->numRects;

#define MERGERECT(r) \
    if ((pReg->numRects != 0) &&  \
	(pNextRect[-1].top() == y1) &&  \
	(pNextRect[-1].bottom() == y2) &&  \
	(pNextRect[-1].right() >= r->left()-1)) { \
	if (pNextRect[-1].right() < r->right()) { \
	    pNextRect[-1].setRight( r->right() );  \
	    Q_ASSERT(pNextRect[-1].left() <= pNextRect[-1].right()); \
	}  \
    } else { \
	MEMCHECK(pReg, pNextRect, pReg->rects)  \
	qt_setCoords( pNextRect, r->left(), y1, r->right(), y2 ); \
	pReg->numRects++;  \
        pNextRect++;  \
    }  \
    r++;

    Q_ASSERT (y1<=y2);
    while ((r1 != r1End) && (r2 != r2End)) {
	if (r1->left() < r2->left()) {
	    MERGERECT(r1)
	} else {
	    MERGERECT(r2)
	}
    }

    if (r1 != r1End)
    {
	do
	{
	    MERGERECT(r1)
	} while (r1 != r1End);
    }
    else while (r2 != r2End)
    {
	MERGERECT(r2)
    }
    return 0;	/* lint */
}

static void UnionRegion(QRegionPrivate *reg1, QRegionPrivate *reg2, QRegionPrivate *newReg)
{
    /*  checks all the simple cases */

    /*
     * Region 1 and 2 are the same or region 1 is empty
     */
    if ( (reg1 == reg2) || (!(reg1->numRects)) )
    {
	*newReg = *reg2;
	return;
    }

    /*
     * if nothing to union (region 2 empty)
     */
    if (!(reg2->numRects))
    {
        *newReg = *reg1;
        return;
    }

    /*
     * Region 1 completely subsumes region 2
     */
    if ((reg1->numRects == 1) &&
	(reg1->extents.left() <= reg2->extents.left()) &&
	(reg1->extents.top() <= reg2->extents.top()) &&
	(reg1->extents.right() >= reg2->extents.right()) &&
	(reg1->extents.bottom() >= reg2->extents.bottom()))
    {
	*newReg = *reg1;
        return;
    }

    /*
     * Region 2 completely subsumes region 1
     */
    if ((reg2->numRects == 1) &&
	(reg2->extents.left() <= reg1->extents.left()) &&
	(reg2->extents.top() <= reg1->extents.top()) &&
	(reg2->extents.right() >= reg1->extents.right()) &&
	(reg2->extents.bottom() >= reg1->extents.bottom()))
    {
	*newReg = *reg2;
        return;
    }

    miRegionOp (newReg, reg1, reg2, (voidProcp) miUnionO,
    		(voidProcp) miUnionNonO, (voidProcp) miUnionNonO);

    qt_setCoords( &newReg->extents,
		  QMIN(reg1->extents.left(), reg2->extents.left()),
		  QMIN(reg1->extents.top(), reg2->extents.top()),
		  QMAX(reg1->extents.right(), reg2->extents.right()),
		  QMAX(reg1->extents.bottom(), reg2->extents.bottom()) );

    return;
}

/*======================================================================
 * 	    	  Region Subtraction
 *====================================================================*/

/*-
 *-----------------------------------------------------------------------
 * miSubtractNonO --
 *	Deal with non-overlapping band for subtraction. Any parts from
 *	region 2 we discard. Anything from region 1 we add to the region.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	pReg may be affected.
 *
 *-----------------------------------------------------------------------
 */
/* static void*/
static
int
miSubtractNonO1 (register QRegionPrivate *pReg, register QRect *r,
		 QRect *rEnd, register int y1, register int y2)
{
    register QRect *pNextRect;

    pNextRect = pReg->rects.data() + pReg->numRects;

    Q_ASSERT(y1<=y2);

    while (r != rEnd)
    {
	Q_ASSERT(r->left()<=r->right());
	MEMCHECK(pReg, pNextRect, pReg->rects)
	qt_setCoords( pNextRect, r->left(), y1, r->right(), y2 );
	pReg->numRects++;
	pNextRect++;

	r++;
    }
    return 0;	/* lint */
}

/*-
 *-----------------------------------------------------------------------
 * miSubtractO --
 *	Overlapping band subtraction. x1 is the left-most point not yet
 *	checked.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	pReg may have rectangles added to it.
 *
 *-----------------------------------------------------------------------
 */
/* static void*/
static
int
miSubtractO (register QRegionPrivate *pReg, register QRect *r1, QRect *r1End,
	     register QRect *r2, QRect *r2End, register int y1, register int y2)
{
    register QRect *pNextRect;
    register int  	x1;

    x1 = r1->left();

    Q_ASSERT(y1<=y2);
    pNextRect = pReg->rects.data() + pReg->numRects;

    while ((r1 != r1End) && (r2 != r2End))
    {
	if (r2->right() < x1)
	{
	    /*
	     * Subtrahend missed the boat: go to next subtrahend.
	     */
	    r2++;
	}
	else if (r2->left() <= x1)
	{
	    /*
	     * Subtrahend precedes minuend: nuke left edge of minuend.
	     */
	    x1 = r2->right()+1;
	    if (x1 > r1->right())
	    {
		/*
		 * Minuend completely covered: advance to next minuend and
		 * reset left fence to edge of new minuend.
		 */
		r1++;
		if (r1 != r1End)
		    x1 = r1->left();
	    }
	    else
	    {
		/*
		 * Subtrahend now used up since it doesn't extend beyond
		 * minuend
		 */
		r2++;
	    }
	}
	else if (r2->left() <= r1->right())
	{
	    /*
	     * Left part of subtrahend covers part of minuend: add uncovered
	     * part of minuend to region and skip to next subtrahend.
	     */
	    Q_ASSERT(x1<r2->left());
	    MEMCHECK(pReg, pNextRect, pReg->rects)
	    qt_setCoords( pNextRect, x1, y1, r2->left() - 1, y2 );
	    pReg->numRects++;
	    pNextRect++;

	    x1 = r2->right() + 1;
	    if (x1 > r1->right())
	    {
		/*
		 * Minuend used up: advance to new...
		 */
		r1++;
		if (r1 != r1End)
		    x1 = r1->left();
	    }
	    else
	    {
		/*
		 * Subtrahend used up
		 */
		r2++;
	    }
	}
	else
	{
	    /*
	     * Minuend used up: add any remaining piece before advancing.
	     */
	    if (r1->right() >= x1)
	    {
		MEMCHECK(pReg, pNextRect, pReg->rects)
		qt_setCoords( pNextRect, x1, y1, r1->right(), y2 );
		pReg->numRects++;
		pNextRect++;
	    }
	    r1++;
	    if ( r1 != r1End )
		x1 = r1->left();
	}
    }

    /*
     * Add remaining minuend rectangles to region.
     */
    while (r1 != r1End)
    {
	Q_ASSERT(x1<=r1->right());
	MEMCHECK(pReg, pNextRect, pReg->rects)
	qt_setCoords( pNextRect, x1, y1, r1->right(), y2 );
	pReg->numRects++;
	pNextRect++;

	r1++;
	if (r1 != r1End)
	{
	    x1 = r1->left();
	}
    }
    return 0;	/* lint */
}

/*-
 *-----------------------------------------------------------------------
 * miSubtract --
 *	Subtract regS from regM and leave the result in regD.
 *	S stands for subtrahend, M for minuend and D for difference.
 *
 * Side Effects:
 *	regD is overwritten.
 *
 *-----------------------------------------------------------------------
 */

static void SubtractRegion(QRegionPrivate *regM, QRegionPrivate *regS, register QRegionPrivate *regD)
{
   /* check for trivial reject */
    if ( (!(regM->numRects)) || (!(regS->numRects))  ||
	(!EXTENTCHECK(&regM->extents, &regS->extents)) )
    {
	*regD = *regM;
	return;
    }

    miRegionOp (regD, regM, regS, (voidProcp) miSubtractO,
    		(voidProcp) miSubtractNonO1, (voidProcp) NULL);

    /*
     * Can't alter newReg's extents before we call miRegionOp because
     * it might be one of the source regions and miRegionOp depends
     * on the extents of those regions being the unaltered. Besides, this
     * way there's no checking against rectangles that will be nuked
     * due to coalescing, so we have to examine fewer rectangles.
     */
    miSetExtents (regD);
}

static void XorRegion( QRegionPrivate *sra, QRegionPrivate *srb, QRegionPrivate *dr )
{
    QRegionPrivate tra, trb;

    SubtractRegion(sra,srb,&tra);
    SubtractRegion(srb,sra,&trb);
    UnionRegion(&tra,&trb,dr);
}

/*
 *	Check to see if two regions are equal
 */
static bool EqualRegion( QRegionPrivate *r1, QRegionPrivate *r2 )
{
    int i;

    if( r1->numRects != r2->numRects ) return FALSE;
    else if( r1->numRects == 0 ) return TRUE;
    else if ( r1->extents.left() != r2->extents.left() ||
	      r1->extents.right() != r2->extents.right() ||
	      r1->extents.top() != r2->extents.top() ||
	      r1->extents.bottom() != r2->extents.bottom() )
	return FALSE;
    else {
	QRect *rr1 = r1->rects.data();
	QRect *rr2 = r2->rects.data();
	for( i=0; i < r1->numRects; i++, rr1++, rr2++ ) {
	    if ( rr1->left() != rr2->left() ||
		 rr1->right() != rr2->right() ||
		 rr1->top() != rr2->top() ||
		 rr1->bottom() != rr2->bottom() )
		return FALSE;
	}
    }
    return TRUE;
}

static bool PointInRegion( QRegionPrivate *pRegion, int x, int y )
{
    int i;

    if (pRegion->numRects == 0)
        return FALSE;
    if (!pRegion->extents.contains(x, y))
        return FALSE;
    for (i=0; i<pRegion->numRects; i++)
    {
        if (pRegion->rects[i].contains(x, y))
	    return TRUE;
    }
    return FALSE;
}

static bool RectInRegion(register QRegionPrivate *region,
			 int rx, int ry, unsigned int rwidth, unsigned int rheight)
{
    register QRect *pbox;
    register QRect *pboxEnd;
    QRect rect(rx, ry, rwidth, rheight);
    register QRect *prect = &rect;
    int      partIn, partOut;

    /* this is (just) a useful optimization */
    if ((region->numRects == 0) || !EXTENTCHECK(&region->extents, prect))
        return(RectangleOut);

    partOut = FALSE;
    partIn = FALSE;

    /* can stop when both partOut and partIn are TRUE, or we reach prect->y2 */
    for (pbox = region->rects.data(), pboxEnd = pbox + region->numRects;
	 pbox < pboxEnd;
	 pbox++)
    {

	if (pbox->bottom() < ry)
	   continue;	/* getting up to speed or skipping remainder of band */

	if (pbox->top() > ry)
	{
	   partOut = TRUE;	/* missed part of rectangle above */
	   if (partIn || (pbox->top() > prect->bottom()))
	      break;
	   ry = pbox->top();	/* x guaranteed to be == prect->x1 */
	}

	if (pbox->right() < rx)
	   continue;		/* not far enough over yet */

	if (pbox->left() > rx)
	{
	   partOut = TRUE;	/* missed part of rectangle to left */
	   if (partIn)
	      break;
	}

	if (pbox->left() <= prect->right())
	{
	    partIn = TRUE;	/* definitely overlap */
	    if (partOut)
	       break;
	}

	if (pbox->right() >= prect->right())
	{
	   ry = pbox->bottom() + 1;	/* finished with this band */
	   if (ry > prect->bottom())
	      break;
	   rx = prect->left();	/* reset x out to left again */
	} else
	{
	    /*
	     * Because boxes in a band are maximal width, if the first box
	     * to overlap the rectangle doesn't completely cover it in that
	     * band, the rectangle must be partially out, since some of it
	     * will be uncovered in that band. partIn will have been set true
	     * by now...
	     */
	    break;
	}

    }

    return(partIn ? ((ry <= prect->bottom()) ? RectanglePart : RectangleIn) :
		RectangleOut);
}
// END OF Region.c extract
// START OF poly.h extract
/* $XConsortium: poly.h,v 1.4 94/04/17 20:22:19 rws Exp $ */
/************************************************************************

Copyright (c) 1987  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.


Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

************************************************************************/

/*
 *     This file contains a few macros to help track
 *     the edge of a filled object.  The object is assumed
 *     to be filled in scanline order, and thus the
 *     algorithm used is an extension of Bresenham's line
 *     drawing algorithm which assumes that y is always the
 *     major axis.
 *     Since these pieces of code are the same for any filled shape,
 *     it is more convenient to gather the library in one
 *     place, but since these pieces of code are also in
 *     the inner loops of output primitives, procedure call
 *     overhead is out of the question.
 *     See the author for a derivation if needed.
 */


/*
 *  In scan converting polygons, we want to choose those pixels
 *  which are inside the polygon.  Thus, we add .5 to the starting
 *  x coordinate for both left and right edges.  Now we choose the
 *  first pixel which is inside the pgon for the left edge and the
 *  first pixel which is outside the pgon for the right edge.
 *  Draw the left pixel, but not the right.
 *
 *  How to add .5 to the starting x coordinate:
 *      If the edge is moving to the right, then subtract dy from the
 *  error term from the general form of the algorithm.
 *      If the edge is moving to the left, then add dy to the error term.
 *
 *  The reason for the difference between edges moving to the left
 *  and edges moving to the right is simple:  If an edge is moving
 *  to the right, then we want the algorithm to flip immediately.
 *  If it is moving to the left, then we don't want it to flip until
 *  we traverse an entire pixel.
 */
#define BRESINITPGON(dy, x1, x2, xStart, d, m, m1, incr1, incr2) { \
    int dx;      /* local storage */ \
\
    /* \
     *  if the edge is horizontal, then it is ignored \
     *  and assumed not to be processed.  Otherwise, do this stuff. \
     */ \
    if ((dy) != 0) { \
        xStart = (x1); \
        dx = (x2) - xStart; \
        if (dx < 0) { \
            m = dx / (dy); \
            m1 = m - 1; \
            incr1 = -2 * dx + 2 * (dy) * m1; \
            incr2 = -2 * dx + 2 * (dy) * m; \
            d = 2 * m * (dy) - 2 * dx - 2 * (dy); \
        } else { \
            m = dx / (dy); \
            m1 = m + 1; \
            incr1 = 2 * dx - 2 * (dy) * m1; \
            incr2 = 2 * dx - 2 * (dy) * m; \
            d = -2 * m * (dy) + 2 * dx; \
        } \
    } \
}

#define BRESINCRPGON(d, minval, m, m1, incr1, incr2) { \
    if (m1 > 0) { \
        if (d > 0) { \
            minval += m1; \
            d += incr1; \
        } \
        else { \
            minval += m; \
            d += incr2; \
        } \
    } else {\
        if (d >= 0) { \
            minval += m1; \
            d += incr1; \
        } \
        else { \
            minval += m; \
            d += incr2; \
        } \
    } \
}


/*
 *     This structure contains all of the information needed
 *     to run the bresenham algorithm.
 *     The variables may be hardcoded into the declarations
 *     instead of using this structure to make use of
 *     register declarations.
 */
typedef struct {
    int minor_axis;	/* minor axis        */
    int d;		/* decision variable */
    int m, m1;		/* slope and slope+1 */
    int incr1, incr2;	/* error increments */
} BRESINFO;


#define BRESINITPGONSTRUCT(dmaj, min1, min2, bres) \
	BRESINITPGON(dmaj, min1, min2, bres.minor_axis, bres.d, \
                     bres.m, bres.m1, bres.incr1, bres.incr2)

#define BRESINCRPGONSTRUCT(bres) \
        BRESINCRPGON(bres.d, bres.minor_axis, bres.m, bres.m1, bres.incr1, bres.incr2)



/*
 *     These are the data structures needed to scan
 *     convert regions.  Two different scan conversion
 *     methods are available -- the even-odd method, and
 *     the winding number method.
 *     The even-odd rule states that a point is inside
 *     the polygon if a ray drawn from that point in any
 *     direction will pass through an odd number of
 *     path segments.
 *     By the winding number rule, a point is decided
 *     to be inside the polygon if a ray drawn from that
 *     point in any direction passes through a different
 *     number of clockwise and counter-clockwise path
 *     segments.
 *
 *     These data structures are adapted somewhat from
 *     the algorithm in (Foley/Van Dam) for scan converting
 *     polygons.
 *     The basic algorithm is to start at the top (smallest y)
 *     of the polygon, stepping down to the bottom of
 *     the polygon by incrementing the y coordinate.  We
 *     keep a list of edges which the current scanline crosses,
 *     sorted by x.  This list is called the Active Edge Table (AET)
 *     As we change the y-coordinate, we update each entry in
 *     in the active edge table to reflect the edges new xcoord.
 *     This list must be sorted at each scanline in case
 *     two edges intersect.
 *     We also keep a data structure known as the Edge Table (ET),
 *     which keeps track of all the edges which the current
 *     scanline has not yet reached.  The ET is basically a
 *     list of ScanLineList structures containing a list of
 *     edges which are entered at a given scanline.  There is one
 *     ScanLineList per scanline at which an edge is entered.
 *     When we enter a new edge, we move it from the ET to the AET.
 *
 *     From the AET, we can implement the even-odd rule as in
 *     (Foley/Van Dam).
 *     The winding number rule is a little trickier.  We also
 *     keep the EdgeTableEntries in the AET linked by the
 *     nextWETE (winding EdgeTableEntry) link.  This allows
 *     the edges to be linked just as before for updating
 *     purposes, but only uses the edges linked by the nextWETE
 *     link as edges representing spans of the polygon to
 *     drawn (as with the even-odd rule).
 */

/*
 * for the winding number rule
 */
#define CLOCKWISE          1
#define COUNTERCLOCKWISE  -1

typedef struct _EdgeTableEntry {
     int ymax;             /* ycoord at which we exit this edge. */
     BRESINFO bres;        /* Bresenham info to run the edge     */
     struct _EdgeTableEntry *next;       /* next in the list     */
     struct _EdgeTableEntry *back;       /* for insertion sort   */
     struct _EdgeTableEntry *nextWETE;   /* for winding num rule */
     int ClockWise;        /* flag for winding number rule       */
} EdgeTableEntry;


typedef struct _ScanLineList{
     int scanline;              /* the scanline represented */
     EdgeTableEntry *edgelist;  /* header node              */
     struct _ScanLineList *next;  /* next in the list       */
} ScanLineList;


typedef struct {
     int ymax;                 /* ymax for the polygon     */
     int ymin;                 /* ymin for the polygon     */
     ScanLineList scanlines;   /* header node              */
} EdgeTable;


/*
 * Here is a struct to help with storage allocation
 * so we can allocate a big chunk at a time, and then take
 * pieces from this heap when we need to.
 */
#define SLLSPERBLOCK 25

typedef struct _ScanLineListBlock {
     ScanLineList SLLs[SLLSPERBLOCK];
     struct _ScanLineListBlock *next;
} ScanLineListBlock;



/*
 *
 *     a few macros for the inner loops of the fill code where
 *     performance considerations don't allow a procedure call.
 *
 *     Evaluate the given edge at the given scanline.
 *     If the edge has expired, then we leave it and fix up
 *     the active edge table; otherwise, we increment the
 *     x value to be ready for the next scanline.
 *     The winding number rule is in effect, so we must notify
 *     the caller when the edge has been removed so he
 *     can reorder the Winding Active Edge Table.
 */
#define EVALUATEEDGEWINDING(pAET, pPrevAET, y, fixWAET) { \
   if (pAET->ymax == y) {          /* leaving this edge */ \
      pPrevAET->next = pAET->next; \
      pAET = pPrevAET->next; \
      fixWAET = 1; \
      if (pAET) \
         pAET->back = pPrevAET; \
   } \
   else { \
      BRESINCRPGONSTRUCT(pAET->bres) \
      pPrevAET = pAET; \
      pAET = pAET->next; \
   } \
}


/*
 *     Evaluate the given edge at the given scanline.
 *     If the edge has expired, then we leave it and fix up
 *     the active edge table; otherwise, we increment the
 *     x value to be ready for the next scanline.
 *     The even-odd rule is in effect.
 */
#define EVALUATEEDGEEVENODD(pAET, pPrevAET, y) { \
   if (pAET->ymax == y) {          /* leaving this edge */ \
      pPrevAET->next = pAET->next; \
      pAET = pPrevAET->next; \
      if (pAET) \
         pAET->back = pPrevAET; \
   } \
   else { \
      BRESINCRPGONSTRUCT(pAET->bres) \
      pPrevAET = pAET; \
      pAET = pAET->next; \
   } \
}
// END OF poly.h extract
// START OF PolyReg.c extract
/* $XConsortium: PolyReg.c,v 11.23 94/11/17 21:59:37 converse Exp $ */
/************************************************************************

Copyright (c) 1987  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.


Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

************************************************************************/
/* $XFree86: xc/lib/X11/PolyReg.c,v 1.1.1.2.8.2 1998/10/04 15:22:49 hohndel Exp $ */

#define LARGE_COORDINATE 1000000
#define SMALL_COORDINATE -LARGE_COORDINATE

/*
 *     InsertEdgeInET
 *
 *     Insert the given edge into the edge table.
 *     First we must find the correct bucket in the
 *     Edge table, then find the right slot in the
 *     bucket.  Finally, we can insert it.
 *
 */
static void
InsertEdgeInET(EdgeTable *ET, EdgeTableEntry *ETE, int scanline,
		ScanLineListBlock **SLLBlock, int *iSLLBlock)
{
    register EdgeTableEntry *start, *prev;
    register ScanLineList *pSLL, *pPrevSLL;
    ScanLineListBlock *tmpSLLBlock;

    /*
     * find the right bucket to put the edge into
     */
    pPrevSLL = &ET->scanlines;
    pSLL = pPrevSLL->next;
    while (pSLL && (pSLL->scanline < scanline))
    {
        pPrevSLL = pSLL;
        pSLL = pSLL->next;
    }

    /*
     * reassign pSLL (pointer to ScanLineList) if necessary
     */
    if ((!pSLL) || (pSLL->scanline > scanline))
    {
        if (*iSLLBlock > SLLSPERBLOCK-1)
        {
            tmpSLLBlock =
		  (ScanLineListBlock *)malloc(sizeof(ScanLineListBlock));
            (*SLLBlock)->next = tmpSLLBlock;
            tmpSLLBlock->next = (ScanLineListBlock *)NULL;
            *SLLBlock = tmpSLLBlock;
            *iSLLBlock = 0;
        }
        pSLL = &((*SLLBlock)->SLLs[(*iSLLBlock)++]);

        pSLL->next = pPrevSLL->next;
        pSLL->edgelist = (EdgeTableEntry *)NULL;
        pPrevSLL->next = pSLL;
    }
    pSLL->scanline = scanline;

    /*
     * now insert the edge in the right bucket
     */
    prev = (EdgeTableEntry *)NULL;
    start = pSLL->edgelist;
    while (start && (start->bres.minor_axis < ETE->bres.minor_axis))
    {
        prev = start;
        start = start->next;
    }
    ETE->next = start;

    if (prev)
        prev->next = ETE;
    else
        pSLL->edgelist = ETE;
}

/*
 *     CreateEdgeTable
 *
 *     This routine creates the edge table for
 *     scan converting polygons.
 *     The Edge Table (ET) looks like:
 *
 *    EdgeTable
 *     --------
 *    |  ymax  |        ScanLineLists
 *    |scanline|-->------------>-------------->...
 *     --------   |scanline|   |scanline|
 *                |edgelist|   |edgelist|
 *                ---------    ---------
 *                    |             |
 *                    |             |
 *                    V             V
 *              list of ETEs   list of ETEs
 *
 *     where ETE is an EdgeTableEntry data structure,
 *     and there is one ScanLineList per scanline at
 *     which an edge is initially entered.
 *
 */

static void
CreateETandAET(register int count, register QPoint *pts,
	EdgeTable *ET, EdgeTableEntry *AET, register EdgeTableEntry *pETEs,
	ScanLineListBlock   *pSLLBlock)
{
    register QPoint *top, *bottom;
    register QPoint *PrevPt, *CurrPt;
    int iSLLBlock = 0;
    int dy;

    if (count < 2)  return;

    /*
     *  initialize the Active Edge Table
     */
    AET->next = (EdgeTableEntry *)NULL;
    AET->back = (EdgeTableEntry *)NULL;
    AET->nextWETE = (EdgeTableEntry *)NULL;
    AET->bres.minor_axis = SMALL_COORDINATE;

    /*
     *  initialize the Edge Table.
     */
    ET->scanlines.next = (ScanLineList *)NULL;
    ET->ymax = SMALL_COORDINATE;
    ET->ymin = LARGE_COORDINATE;
    pSLLBlock->next = (ScanLineListBlock *)NULL;

    PrevPt = &pts[count-1];

    /*
     *  for each vertex in the array of points.
     *  In this loop we are dealing with two vertices at
     *  a time -- these make up one edge of the polygon.
     */
    while (count--)
    {
        CurrPt = pts++;

        /*
         *  find out which point is above and which is below.
         */
        if (PrevPt->y() > CurrPt->y() )
        {
            bottom = PrevPt, top = CurrPt;
            pETEs->ClockWise = 0;
        }
        else
        {
            bottom = CurrPt, top = PrevPt;
            pETEs->ClockWise = 1;
        }

        /*
         * don't add horizontal edges to the Edge table.
         */
        if ( bottom->y() != top->y() )
        {
            pETEs->ymax = bottom->y()-1;  /* -1 so we don't get last scanline */

            /*
             *  initialize integer edge algorithm
             */
            dy = bottom->y() - top->y();
            BRESINITPGONSTRUCT(dy, top->x(), bottom->x(), pETEs->bres)

            InsertEdgeInET(ET, pETEs, top->y(), &pSLLBlock, &iSLLBlock);

	    if (PrevPt->y() > ET->ymax)
		ET->ymax = PrevPt->y();
	    if (PrevPt->y() < ET->ymin)
		ET->ymin = PrevPt->y();
            pETEs++;
        }

        PrevPt = CurrPt;
    }
}

/*
 *     loadAET
 *
 *     This routine moves EdgeTableEntries from the
 *     EdgeTable into the Active Edge Table,
 *     leaving them sorted by smaller x coordinate.
 *
 */

static void
loadAET(register EdgeTableEntry *AET, register EdgeTableEntry *ETEs)
{
    register EdgeTableEntry *pPrevAET;
    register EdgeTableEntry *tmp;

    pPrevAET = AET;
    AET = AET->next;
    while (ETEs)
    {
        while (AET && (AET->bres.minor_axis < ETEs->bres.minor_axis))
        {
            pPrevAET = AET;
            AET = AET->next;
        }
        tmp = ETEs->next;
        ETEs->next = AET;
        if (AET)
            AET->back = ETEs;
        ETEs->back = pPrevAET;
        pPrevAET->next = ETEs;
        pPrevAET = ETEs;

        ETEs = tmp;
    }
}

/*
 *     computeWAET
 *
 *     This routine links the AET by the
 *     nextWETE (winding EdgeTableEntry) link for
 *     use by the winding number rule.  The final
 *     Active Edge Table (AET) might look something
 *     like:
 *
 *     AET
 *     ----------  ---------   ---------
 *     |ymax    |  |ymax    |  |ymax    |
 *     | ...    |  |...     |  |...     |
 *     |next    |->|next    |->|next    |->...
 *     |nextWETE|  |nextWETE|  |nextWETE|
 *     ---------   ---------   ^--------
 *         |                   |       |
 *         V------------------->       V---> ...
 *
 */
static void
computeWAET(register EdgeTableEntry *AET)
{
    register EdgeTableEntry *pWETE;
    register int inside = 1;
    register int isInside = 0;

    AET->nextWETE = (EdgeTableEntry *)NULL;
    pWETE = AET;
    AET = AET->next;
    while (AET)
    {
        if (AET->ClockWise)
            isInside++;
        else
            isInside--;

        if ((!inside && !isInside) ||
            ( inside &&  isInside))
        {
            pWETE->nextWETE = AET;
            pWETE = AET;
            inside = !inside;
        }
        AET = AET->next;
    }
    pWETE->nextWETE = (EdgeTableEntry *)NULL;
}

/*
 *     InsertionSort
 *
 *     Just a simple insertion sort using
 *     pointers and back pointers to sort the Active
 *     Edge Table.
 *
 */

static int
InsertionSort(register EdgeTableEntry *AET)
{
    register EdgeTableEntry *pETEchase;
    register EdgeTableEntry *pETEinsert;
    register EdgeTableEntry *pETEchaseBackTMP;
    register int changed = 0;

    AET = AET->next;
    while (AET)
    {
        pETEinsert = AET;
        pETEchase = AET;
        while (pETEchase->back->bres.minor_axis > AET->bres.minor_axis)
            pETEchase = pETEchase->back;

        AET = AET->next;
        if (pETEchase != pETEinsert)
        {
            pETEchaseBackTMP = pETEchase->back;
            pETEinsert->back->next = AET;
            if (AET)
                AET->back = pETEinsert->back;
            pETEinsert->next = pETEchase;
            pETEchase->back->next = pETEinsert;
            pETEchase->back = pETEinsert;
            pETEinsert->back = pETEchaseBackTMP;
            changed = 1;
        }
    }
    return(changed);
}

/*
 *     Clean up our act.
 */
static void
FreeStorage(register ScanLineListBlock   *pSLLBlock)
{
    register ScanLineListBlock   *tmpSLLBlock;

    while (pSLLBlock)
    {
        tmpSLLBlock = pSLLBlock->next;
        free((char *)pSLLBlock);
        pSLLBlock = tmpSLLBlock;
    }
}

/*
 *     Create an array of rectangles from a list of points.
 *     If indeed these things (POINTS, RECTS) are the same,
 *     then this proc is still needed, because it allocates
 *     storage for the array, which was allocated on the
 *     stack by the calling procedure.
 *
 */
static int PtsToRegion(register int numFullPtBlocks, register int iCurPtBlock,
    POINTBLOCK *FirstPtBlock, QRegionPrivate *reg)
{
    register QRect  *rects;
    register QPoint *pts;
    register POINTBLOCK *CurPtBlock;
    register int i;
    register QRect *extents;
    register int numRects;

    extents = &reg->extents;

    numRects = ((numFullPtBlocks * NUMPTSTOBUFFER) + iCurPtBlock) >> 1;

    reg->rects.resize(numRects);

    CurPtBlock = FirstPtBlock;
    rects = reg->rects.data() - 1;
    numRects = 0;
    extents->setLeft( INT_MAX );
    extents->setRight( INT_MIN );

    for ( ; numFullPtBlocks >= 0; numFullPtBlocks--) {
	/* the loop uses 2 points per iteration */
	i = NUMPTSTOBUFFER >> 1;
	if (!numFullPtBlocks)
	    i = iCurPtBlock >> 1;
	for (pts = CurPtBlock->pts; i--; pts += 2) {
	    if ( pts->x() == pts[1].x() )
		continue;
	    if (numRects && pts->x() == rects->left() && pts->y() == rects->bottom() + 1 &&
		pts[1].x() == rects->right() &&
		(numRects == 1 || rects[-1].top() != rects->top()) &&
		(i && pts[2].y() > pts[1].y() )) {
		rects->setBottom( pts[1].y() );
		continue;
	    }
	    numRects++;
	    rects++;
	    qt_setCoords( rects, pts->x(), pts->y(), pts[1].x() - 1, pts[1].y() );
	    if (rects->left() < extents->left())
		extents->setLeft( rects->left() );
	    if (rects->right() > extents->right())
		extents->setRight( rects->right() );
        }
	CurPtBlock = CurPtBlock->next;
    }

    if (numRects) {
	extents->setTop( reg->rects[0].top() );
	extents->setBottom( rects->bottom() );
    } else {
	qt_setCoords(extents, 0, 0, 0, 0);
    }
    reg->numRects = numRects;

    return(TRUE);
}

/*
 *     polytoregion
 *
 *     Scan converts a polygon by returning a run-length
 *     encoding of the resultant bitmap -- the run-length
 *     encoding is in the form of an array of rectangles.
 */
static QRegionPrivate *PolygonRegion(QPoint *Pts, int Count, int rule)
    //Point     *Pts;		     /* the pts                 */
    //int       Count;                 /* number of pts           */
    //int	rule;			     /* winding rule */
{
    QRegionPrivate *region;
    register EdgeTableEntry *pAET;   /* Active Edge Table       */
    register int y;                  /* current scanline        */
    register int iPts = 0;           /* number of pts in buffer */
    register EdgeTableEntry *pWETE;  /* Winding Edge Table Entry*/
    register ScanLineList *pSLL;     /* current scanLineList    */
    register QPoint *pts;             /* output buffer           */
    EdgeTableEntry *pPrevAET;        /* ptr to previous AET     */
    EdgeTable ET;                    /* header node for ET      */
    EdgeTableEntry AET;              /* header node for AET     */
    EdgeTableEntry *pETEs;           /* EdgeTableEntries pool   */
    ScanLineListBlock SLLBlock;      /* header for scanlinelist */
    int fixWAET = FALSE;
    POINTBLOCK FirstPtBlock, *curPtBlock; /* PtBlock buffers    */
    POINTBLOCK *tmpPtBlock;
    int numFullPtBlocks = 0;

    if ( !(region = new QRegionPrivate) )
	return 0;

    /* special case a rectangle */
    pts = Pts;
    if (((Count == 4) ||
	 ((Count == 5) && (pts[4].x() == pts[0].x() ) && (pts[4].y() == pts[0].y() ))) &&
	(((pts[0].y() == pts[1].y()) &&
	  (pts[1].x() == pts[2].x()) &&
	  (pts[2].y() == pts[3].y()) &&
	  (pts[3].x() == pts[0].x())) ||
	 ((pts[0].x() == pts[1].x()) &&
	  (pts[1].y() == pts[2].y()) &&
	  (pts[2].x() == pts[3].x()) &&
	  (pts[3].y() == pts[0].y())))) {
	region->extents.setLeft( QMIN(pts[0].x(), pts[2].x()) );
	region->extents.setTop( QMIN(pts[0].y(), pts[2].y()) );
	region->extents.setRight( QMAX(pts[0].x(), pts[2].x()) );
	region->extents.setBottom( QMAX(pts[0].y(), pts[2].y()) );
	if ((region->extents.left() <= region->extents.right()) &&
	    (region->extents.top() <= region->extents.bottom())) {
	    region->numRects = 1;
	    region->rects.resize(1);
	    region->rects[0] = region->extents;
	}
	return region;
    }

    if (! (pETEs = (EdgeTableEntry *)
	   malloc((unsigned) (sizeof(EdgeTableEntry) * Count))))
	return 0;

    pts = FirstPtBlock.pts;
    CreateETandAET(Count, Pts, &ET, &AET, pETEs, &SLLBlock);
    pSLL = ET.scanlines.next;
    curPtBlock = &FirstPtBlock;

    if (rule == EvenOddRule) {
        /*
         *  for each scanline
         */
        for (y = ET.ymin; y < ET.ymax; y++) {
            /*
             *  Add a new edge to the active edge table when we
             *  get to the next edge.
             */
            if (pSLL != NULL && y == pSLL->scanline) {
                loadAET(&AET, pSLL->edgelist);
                pSLL = pSLL->next;
            }
            pPrevAET = &AET;
            pAET = AET.next;

            /*
             *  for each active edge
             */
            while (pAET) {
                pts->setX( pAET->bres.minor_axis ),  pts->setY( y );
                pts++, iPts++;

                /*
                 *  send out the buffer
                 */
                if (iPts == NUMPTSTOBUFFER) {
                    tmpPtBlock = (POINTBLOCK *)malloc(sizeof(POINTBLOCK));
                    curPtBlock->next = tmpPtBlock;
                    curPtBlock = tmpPtBlock;
                    pts = curPtBlock->pts;
                    numFullPtBlocks++;
                    iPts = 0;
                }
                EVALUATEEDGEEVENODD(pAET, pPrevAET, y)
            }
            (void) InsertionSort(&AET);
        }
    }
    else {
        /*
         *  for each scanline
         */
        for (y = ET.ymin; y < ET.ymax; y++) {
            /*
             *  Add a new edge to the active edge table when we
             *  get to the next edge.
             */
            if (pSLL != NULL && y == pSLL->scanline) {
                loadAET(&AET, pSLL->edgelist);
                computeWAET(&AET);
                pSLL = pSLL->next;
            }
            pPrevAET = &AET;
            pAET = AET.next;
            pWETE = pAET;

            /*
             *  for each active edge
             */
            while (pAET) {
                /*
                 *  add to the buffer only those edges that
                 *  are in the Winding active edge table.
                 */
                if (pWETE == pAET) {
                    pts->setX( pAET->bres.minor_axis),  pts->setY( y );
                    pts++, iPts++;

                    /*
                     *  send out the buffer
                     */
                    if (iPts == NUMPTSTOBUFFER) {
                        tmpPtBlock = (POINTBLOCK *)malloc(sizeof(POINTBLOCK));
                        curPtBlock->next = tmpPtBlock;
                        curPtBlock = tmpPtBlock;
                        pts = curPtBlock->pts;
                        numFullPtBlocks++;    iPts = 0;
                    }
                    pWETE = pWETE->nextWETE;
                }
                EVALUATEEDGEWINDING(pAET, pPrevAET, y, fixWAET)
            }

            /*
             *  recompute the winding active edge table if
             *  we just resorted or have exited an edge.
             */
            if (InsertionSort(&AET) || fixWAET) {
                computeWAET(&AET);
                fixWAET = FALSE;
            }
        }
    }
    FreeStorage(SLLBlock.next);
    (void) PtsToRegion(numFullPtBlocks, iPts, &FirstPtBlock, region);
    for (curPtBlock = FirstPtBlock.next; --numFullPtBlocks >= 0;) {
	tmpPtBlock = curPtBlock->next;
	free((char *)curPtBlock);
	curPtBlock = tmpPtBlock;
    }
    free((char *)pETEs);
    return region;
}
// END OF PolyReg.c extract

QRegionPrivate *qt_bitmapToRegion(const QBitmap& bitmap)
{
    QImage image = bitmap.convertToImage();

    QRegionPrivate *region = new QRegionPrivate;
    QRect xr;

#define AddSpan \
	{ \
	    qt_setCoords( &xr, prev1, y, x-1, y ); \
	    UnionRectWithRegion( &xr, region, region ); \
	}

    const int zero=0;
    bool little = image.bitOrder() == QImage::LittleEndian;

    int x, y;
    for (y=0; y<image.height(); y++) {
	uchar *line = image.scanLine(y);
	int w = image.width();
	uchar all=zero;
	int prev1 = -1;
	for (x=0; x<w; ) {
	    uchar byte = line[x/8];
	    if ( x>w-8 || byte!=all ) {
		if ( little ) {
		    for ( int b=8; b>0 && x<w; b-- ) {
			if ( !(byte&0x01) == !all ) {
			    // More of the same
			} else {
			    // A change.
			    if ( all!=zero ) {
				AddSpan
				all = zero;
			    } else {
				prev1 = x;
				all = ~zero;
			    }
			}
			byte >>= 1;
			x++;
		    }
		} else {
		    for ( int b=8; b>0 && x<w; b-- ) {
			if ( !(byte&0x80) == !all ) {
			    // More of the same
			} else {
			    // A change.
			    if ( all!=zero ) {
				AddSpan
				all = zero;
			    } else {
				prev1 = x;
				all = ~zero;
			    }
			}
			byte <<= 1;
			x++;
		    }
		}
	    } else {
		x+=8;
	    }
	}
	if ( all != zero ) {
	    AddSpan
	}
    }

    return region;
}

// NOT REVISED

static QRegion *empty_region = 0;

static void cleanup_empty_region()
{
    delete empty_region;
    empty_region = 0;
}


/*!
    Constructs a null region.

    \sa isNull()
*/

QRegion::QRegion()
{
    if ( !empty_region ) {			// avoid too many allocs
	qAddPostRoutine( cleanup_empty_region );
	empty_region = new QRegion( TRUE );
	Q_CHECK_PTR( empty_region );
    }
    data = empty_region->data;
    data->ref();
}

/*! \internal
  Internal constructor that creates a null region.
*/

QRegion::QRegion( bool is_null )
{
    data = new QRegionData;
    Q_CHECK_PTR( data );
    data->region = new QRegionPrivate;
    data->is_null = is_null;
    data->rgn = 0;
    data->xrectangles = 0;
}

/*!
    \overload

    Create a region based on the rectange \a r with region type \a t.

    If the rectangle is invalid a null region will be created.

    \sa QRegion::RegionType
*/

QRegion::QRegion( const QRect &r, RegionType t )
{
    if ( r.isEmpty() ) {
	if ( !empty_region ) {			// avoid too many allocs
	    qAddPostRoutine( cleanup_empty_region );
	    empty_region = new QRegion( TRUE );
	    Q_CHECK_PTR( empty_region );
	}
	data = empty_region->data;
	data->ref();
    } else {
	data = new QRegionData;
	Q_CHECK_PTR( data );
	data->is_null = FALSE;
	data->rgn = 0;
	data->xrectangles = 0;
	if ( t == Rectangle ) {			// rectangular region
	    data->region = new QRegionPrivate( r );
	} else if ( t == Ellipse ) {		// elliptic region
	    QPointArray a;
	    a.makeEllipse( r.x(), r.y(), r.width(), r.height() );
	    data->region = PolygonRegion( (QPoint*)a.data(), a.size(),
					  EvenOddRule );
	}
    }
}


/*!
    Constructs a polygon region from the point array \a a.

    If \a winding is TRUE, the polygon region is filled using the
    winding algorithm, otherwise the default even-odd fill algorithm
    is used.

    This constructor may create complex regions that will slow down
    painting when used.
*/

QRegion::QRegion( const QPointArray &a, bool winding )
{
    if (a.size() > 2) {
	data = new QRegionData;
	Q_CHECK_PTR( data );
	data->is_null = FALSE;
	data->rgn = 0;
	data->xrectangles = 0;
	data->region = PolygonRegion( (QPoint*)a.data(), a.size(),
				    winding ? WindingRule : EvenOddRule );
    } else {
	if ( !empty_region ) {
	    qAddPostRoutine( cleanup_empty_region );
	    empty_region = new QRegion( TRUE );
	    Q_CHECK_PTR( empty_region );
	}
	data = empty_region->data;
	data->ref();
    }
}


/*!
    Constructs a new region which is equal to region \a r.
*/

QRegion::QRegion( const QRegion &r )
{
    data = r.data;
    data->ref();
}


/*!
    Constructs a region from the bitmap \a bm.

    The resulting region consists of the pixels in bitmap \a bm that
    are \c color1, as if each pixel was a 1 by 1 rectangle.

    This constructor may create complex regions that will slow down
    painting when used. Note that drawing masked pixmaps can be done
    much faster using QPixmap::setMask().
*/
QRegion::QRegion( const QBitmap & bm )
{
    if ( bm.isNull() ) {
	if ( !empty_region ) {			// avoid too many allocs
	    qAddPostRoutine( cleanup_empty_region );
	    empty_region = new QRegion( TRUE );
	    Q_CHECK_PTR( empty_region );
	}
	data = empty_region->data;
	data->ref();
    } else {
	data = new QRegionData;
	Q_CHECK_PTR( data );
	data->is_null = FALSE;
	data->rgn = 0;
	data->xrectangles = 0;
	data->region = qt_bitmapToRegion(bm);
    }
}

/*!
    Destroys the region.
*/

QRegion::~QRegion()
{
    if ( data->deref() ) {
	delete data->region;
	if ( data->rgn )
	    XDestroyRegion( data->rgn );
	if ( data->xrectangles )
	    free( data->xrectangles );
	delete data;
    }
}


/*!
    Assigns \a r to this region and returns a reference to the region.
*/

QRegion &QRegion::operator=( const QRegion &r )
{
    r.data->ref();				// beware of r = r
    if ( data->deref() ) {
	delete data->region;
	if ( data->rgn )
	    XDestroyRegion( data->rgn );
	if ( data->xrectangles )
	    free( data->xrectangles );
	delete data;
    }
    data = r.data;
    return *this;
}


/*!
    Returns a \link shclass.html deep copy\endlink of the region.

    \sa detach()
*/

QRegion QRegion::copy() const
{
    QRegion r( data->is_null );
    *r.data->region = *data->region;
    return r;
}

/*!
    Returns TRUE if the region is a null region; otherwise returns
    FALSE.

    A null region is a region that has not been initialized. A null
    region is always empty.

    \sa isEmpty()
*/

bool QRegion::isNull() const
{
    return data->is_null;
}


/*!
    Returns TRUE if the region is empty; otherwise returns FALSE. An
    empty region is a region that contains no points.

    Example:
    \code
	QRegion r1( 10, 10, 20, 20 );
	QRegion r2( 40, 40, 20, 20 );
	QRegion r3;
        r1.isNull();             // FALSE
        r1.isEmpty();            // FALSE
        r3.isNull();             // TRUE
        r3.isEmpty();            // TRUE
        r3 = r1.intersect( r2 ); // r3 = intersection of r1 and r2
        r3.isNull();             // FALSE
        r3.isEmpty();            // TRUE
        r3 = r1.unite( r2 );     // r3 = union of r1 and r2
        r3.isNull();             // FALSE
        r3.isEmpty();            // FALSE
    \endcode

    \sa isNull()
*/

bool QRegion::isEmpty() const
{
    return data->is_null || ( data->region->numRects == 0 );
}


/*!
    Returns TRUE if the region contains the point \a p; otherwise
    returns FALSE.
*/

bool QRegion::contains( const QPoint &p ) const
{
    return PointInRegion( data->region, p.x(), p.y() );
}

/*!
    \overload

    Returns TRUE if the region overlaps the rectangle \a r; otherwise
    returns FALSE.
*/

bool QRegion::contains( const QRect &r ) const
{
    return RectInRegion( data->region, r.left(), r.top(),
			  r.width(), r.height() ) != RectangleOut;
}


/*!
    Translates (moves) the region \a dx along the X axis and \a dy
    along the Y axis.
*/

void QRegion::translate( int dx, int dy )
{
    if ( empty_region && data == empty_region->data )
	return;
    detach();
    OffsetRegion( data->region, dx, dy );
    if ( data->xrectangles ) {
	free( data->xrectangles );
	data->xrectangles = 0;
    }
}


/*!
    Returns a region which is the union of this region and \a r.

    \img runion.png Region Union

    The figure shows the union of two elliptical regions.
*/

QRegion QRegion::unite( const QRegion &r ) const
{
    QRegion result( FALSE );
    UnionRegion( data->region, r.data->region, result.data->region );
    return result;
}

/*!
    Returns a region which is the intersection of this region and \a r.

    \img rintersect.png Region Intersection

    The figure shows the intersection of two elliptical regions.
*/

QRegion QRegion::intersect( const QRegion &r ) const
{
    QRegion result( FALSE );
    IntersectRegion( data->region, r.data->region, result.data->region );
    return result;
}

/*!
    Returns a region which is \a r subtracted from this region.

    \img rsubtract.png Region Subtraction

    The figure shows the result when the ellipse on the right is
    subtracted from the ellipse on the left. (\c left-right )
*/

QRegion QRegion::subtract( const QRegion &r ) const
{
    QRegion result( FALSE );
    SubtractRegion( data->region, r.data->region, result.data->region );
    return result;
}

/*!
    Returns a region which is the exclusive or (XOR) of this region
    and \a r.

    \img rxor.png Region XORed

    The figure shows the exclusive or of two elliptical regions.
*/

QRegion QRegion::eor( const QRegion &r ) const
{
    QRegion result( FALSE );
    XorRegion( data->region, r.data->region, result.data->region );
    return result;
}

/*!
    Returns the bounding rectangle of this region. An empty region
    gives a rectangle that is QRect::isNull().
*/

QRect QRegion::boundingRect() const
{
    return data->region->extents;
}


/*!
    Returns an array of non-overlapping rectangles that make up the
    region.

    The union of all the rectangles is equal to the original region.
*/

QMemArray<QRect> QRegion::rects() const
{
    QMemArray<QRect> rects;
    rects.duplicate( data->region->rects, data->region->numRects );
    return rects;
}

/*!
  Sets the region to be the given set of rectangles.  The rectangles
  \e must be optimal Y-X sorted bands as follows:
   <ul>
    <li> The rectangles must not intersect
    <li> All rectangles with a given top coordinate must have the same height.
    <li> No two rectangles may abut horizontally (they should be combined
		into a single wider rectangle in that case).
    <li> The rectangles must be sorted ascendingly by Y as the major sort key
		and X as the minor sort key.
   </ul>
  \internal
  Only some platforms have that restriction (QWS and X11).
*/
void QRegion::setRects( const QRect *rects, int num )
{
    *this = QRegion( FALSE );
    if ( !rects || (num == 1 && rects->isEmpty()) )
	num = 0;

    data->region->rects.duplicate( rects, num );
    data->region->numRects = num;
    if ( num == 0 ) {
	data->region->extents = QRect();
    } else {
	int left = INT_MAX, right = INT_MIN, top = INT_MAX, bottom = INT_MIN;
	int i;
	for ( i = 0; i < num; i++ ) {
	    left = QMIN( rects[i].left(), left );
	    right = QMAX( rects[i].right(), right );
	    top = QMIN( rects[i].top(), top );
	    bottom = QMAX( rects[i].bottom(), bottom );
	}
	data->region->extents = QRect( QPoint(left, top), QPoint(right, bottom) );
    }
}

/*!
    Returns TRUE if the region is equal to \a r; otherwise returns
    FALSE.
*/

bool QRegion::operator==( const QRegion &r ) const
{
    return data == r.data ?
	TRUE : EqualRegion( data->region, r.data->region );
}

/*!
    \fn bool QRegion::operator!=( const QRegion &r ) const

    Returns TRUE if the region is different from \a r; otherwise
    returns FALSE.
*/

/*
  This is how X represents regions internally.
*/

struct BOX {
    short x1, x2, y1, y2;
};

struct _XRegion {
    long size;
    long numRects;
    BOX *rects;
    BOX  extents;
};


void QRegion::updateX11Region() const
{
    data->rgn = XCreateRegion();

    for( int i = 0; i < data->region->numRects; i++ ) {
	XRectangle r;
	const QRect &rect = data->region->rects[i];
	r.x = QMAX( SHRT_MIN, rect.x() );
	r.y = QMAX( SHRT_MIN, rect.y() );
	r.width = QMIN( USHRT_MAX, rect.width() );
	r.height = QMIN( USHRT_MAX, rect.height() );
	XUnionRectWithRegion( &r, data->rgn, data->rgn );
    }
}


void *QRegion::clipRectangles( int &num ) const
{
    if ( !data->xrectangles ) {
	XRectangle *r = (XRectangle *) malloc( data->region->numRects * sizeof( XRectangle ) );
	data->xrectangles = r;
	for( int i = 0; i < data->region->numRects; i++ ) {
	    const QRect &rect = data->region->rects[i];
	    r->x = QMAX( SHRT_MIN, rect.x() );
	    r->y = QMAX( SHRT_MIN, rect.y() );
	    r->width = QMIN( USHRT_MAX, rect.width() );
	    r->height = QMIN( USHRT_MAX, rect.height() );
	    r++;
	}
    }
    num = data->region->numRects;
    return data->xrectangles;
}
