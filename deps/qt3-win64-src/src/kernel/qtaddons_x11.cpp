/*
 * $XFree86: xc/lib/Xft/xftname.c,v 1.10 2001/03/30 18:50:18 keithp Exp $
 *
 * Copyright © 2000 Keith Packard, member of The XFree86 Project, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Keith Packard not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Keith Packard makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * KEITH PACKARD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL KEITH PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include "qt_x11_p.h"

#if !defined(QT_NO_XFTFREETYPE) && !defined(QT_XFT2)

#include <X11/Xft/Xft.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

extern bool qt_use_xrender; // defined in qapplication_x11.cpp

extern "C" {

#define XFT_DRAW_N_SRC 2

struct _XftDraw {
    Display *dpy;
    Drawable drawable;
    Visual *visual;
    Colormap colormap;
    Region clip;
    Bool core_set;
    Bool render_set;
    Bool render_able;
    struct {
	Picture pict;
	struct {
	    Picture pict;
	    XRenderColor color;
	} src[XFT_DRAW_N_SRC];
    } render;
    struct {
	GC draw_gc;
	unsigned long fg;
	Font font;
    } core;
};

Picture XftDrawPicture( XftDraw *draw )
{
    if ( ! draw ) return 0;
    if ( ! draw->render_set ) {
	// force the RENDER Picture to be created...
	XftColor color;
	color.color.red = color.color.green = color.color.blue = color.color.alpha =
	    color.pixel = 0;
	XftDrawRect( draw, &color, -100, -100, 1, 1 );
    }
    return draw->render.pict;
}

XftDraw *XftDrawCreateAlpha( Display *display,
			     Pixmap pixmap,
			     int depth )
{
    // taken from Xft 1 sources, see copyright above
    XftDraw     *draw;

    draw = (XftDraw *) malloc (sizeof (XftDraw));
    if (!draw)
	return 0;
    draw->dpy = display;
    draw->drawable = pixmap;
    draw->visual = 0;
    draw->colormap = 0;
    draw->core_set = False;
    draw->clip = 0;

    // Qt addition - go ahead and create the render picture now
    draw->render_set = True;
    draw->render_able = False;

    if ( qt_use_xrender ) {
	draw->render_able = True;

	XRenderPictFormat *format = 0;
	XRenderPictFormat req;
	unsigned long mask = PictFormatType | PictFormatDepth | PictFormatAlphaMask;
	req.type = PictTypeDirect;
	req.depth = depth;
	req.direct.alphaMask = 0xff;
	format = XRenderFindFormat(draw->dpy, mask, &req, 0);
	if (format) {
	    draw->render.pict =
		XRenderCreatePicture(draw->dpy, draw->drawable, format, 0, 0);
	}

	// to keep Xft from trying to free zero pixmaps/pictures, we need to create
	// 2 more pictures (that are identical to draw->render.pict) :/
	draw->render.src[0].pict =
	    XRenderCreatePicture( draw->dpy, draw->drawable, format, 0, 0 );
	draw->render.src[1].pict =
	    XRenderCreatePicture( draw->dpy, draw->drawable, format, 0, 0 );
    }

    return draw;
}

void XftDrawSetClipRectangles(XftDraw *draw, int xoff, int yoff, XRectangle *rects, int count)
{
    if (!draw) return;

    Picture pict = XftDrawPicture(draw);
    XRenderSetPictureClipRectangles(draw->dpy, pict, xoff, yoff, rects, count);
}

void XftDrawSetSubwindowMode(XftDraw *draw, int mode)
{
    if (!draw) return;

    Picture pict = XftDrawPicture(draw);
    XRenderPictureAttributes pattr;
    pattr.subwindow_mode = mode;
    XRenderChangePicture(draw->dpy, pict, CPSubwindowMode, &pattr);
}

} // extern "C"

#endif // !QT_NO_XFTFREETYPE && !QT_XFT2
