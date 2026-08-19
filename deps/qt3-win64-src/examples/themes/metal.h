/****************************************************************************
** $Id: metal.h 2051 2007-02-21 10:04:20Z chehrlic $
**
** Definition of the Metal Style for the themes example
**
** Created : 979899
**
** Copyright (C) 1997-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef METAL_H
#define METAL_H


#include <qpalette.h>

#ifndef QT_NO_STYLE_WINDOWS

#include <qwindowsstyle.h>


class MetalStyle : public QWindowsStyle
{
public:
    MetalStyle();
    void polish( QApplication*);
    void unPolish( QApplication*);
    void polish( QWidget* );
    void unPolish( QWidget* );

    void drawPrimitive( PrimitiveElement pe,
			QPainter *p,
			const QRect &r,
			const QColorGroup &cg,
			SFlags flags = Style_Default,
			const QStyleOption& = QStyleOption::Default) const;

    void drawControl( ControlElement element,
		      QPainter *p,
		      const QWidget *widget,
		      const QRect &r,
		      const QColorGroup &cg,
		      SFlags how = Style_Default,
		      const QStyleOption& = QStyleOption::Default ) const;

    void drawComplexControl( ComplexControl cc,
			     QPainter *p,
			     const QWidget *widget,
			     const QRect &r,
			     const QColorGroup &cg,
			     SFlags how = Style_Default,
			     SCFlags sub = SC_All,
			     SCFlags subActive = SC_None,
			     const QStyleOption& = QStyleOption::Default ) const;
    int pixelMetric( PixelMetric, const QWidget * ) const;


private:
    void drawMetalFrame(  QPainter *p, int x, int y, int w, int h ) const;
    void drawMetalGradient( QPainter *p, int x, int y, int w, int h,
			  bool sunken, bool horz, bool flat=FALSE ) const;
    void drawMetalButton( QPainter *p, int x, int y, int w, int h,
			  bool sunken, bool horz, bool flat=FALSE ) const;
    QPalette oldPalette;
};

#endif

#endif
