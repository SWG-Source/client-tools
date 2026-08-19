/****************************************************************************
** $Id: wood.h 2051 2007-02-21 10:04:20Z chehrlic $
**
** Definition of something or other
**
** Created : 979899
**
** Copyright (C) 1997-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef WOOD_H
#define WOOD_H


#include <qpalette.h>

#ifndef QT_NO_STYLE_WINDOWS

#include <qwindowsstyle.h>


class NorwegianWoodStyle : public QWindowsStyle
{
public:
    NorwegianWoodStyle();
    void polish( QApplication*);
    void polish( QWidget* );
    void unPolish( QWidget* );
    void unPolish( QApplication*);

    void drawPrimitive( PrimitiveElement pe,
			QPainter *p,
			const QRect &r,
			const QColorGroup &cg,
			SFlags flags = Style_Default,
			const QStyleOption& = QStyleOption::Default ) const;

    void drawControl( ControlElement element,
		      QPainter *p,
		      const QWidget *widget,
		      const QRect &r,
		      const QColorGroup &cg,
		      SFlags how = Style_Default,
		      const QStyleOption& = QStyleOption::Default ) const;

    void drawControlMask( ControlElement element,
			  QPainter *p,
			  const QWidget *widget,
			  const QRect &r,
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

    void drawComplexControlMask( ComplexControl control,
				 QPainter *p,
				 const QWidget *widget,
				 const QRect &r,
				 const QStyleOption& = QStyleOption::Default ) const;

    QRect querySubControlMetrics( ComplexControl control,
				  const QWidget *widget,
				  SubControl sc,
				  const QStyleOption& = QStyleOption::Default ) const;

    QRect subRect( SubRect r, const QWidget *widget ) const;


private:
    void drawSemicircleButton(QPainter *p, const QRect &r, int dir,
			      bool sunken, const QColorGroup &g ) const;
    QPalette oldPalette;
    QPixmap *sunkenDark;
    QPixmap *sunkenLight;

};

#endif

#endif
