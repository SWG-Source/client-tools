/****************************************************************************
** $Id: metal.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "metal.h"

#ifndef QT_NO_STYLE_WINDOWS

#include "qapplication.h"
#include "qcombobox.h"
#include "qpainter.h"
#include "qdrawutil.h" // for now
#include "qpixmap.h" // for now
#include "qpalette.h" // for now
#include "qwidget.h"
#include "qlabel.h"
#include "qimage.h"
#include "qpushbutton.h"
#include "qwidget.h"
#include "qrangecontrol.h"
#include "qscrollbar.h"
#include "qslider.h"
#include <limits.h>


/////////////////////////////////////////////////////////
//#include "stonedark.xpm"
#include "stone1.xpm"
#include "marble.xpm"
///////////////////////////////////////////////////////



MetalStyle::MetalStyle() : QWindowsStyle() { }

/*!
  Reimplementation from QStyle
 */
void MetalStyle::polish( QApplication *app)
{
    oldPalette = app->palette();

    // we simply create a nice QColorGroup with a couple of fancy
    // pixmaps here and apply to it all widgets

    QFont f("times", app->font().pointSize() );
    f.setBold( TRUE );
    f.setItalic( TRUE );
    app->setFont( f, TRUE, "QMenuBar");
    app->setFont( f, TRUE, "QPopupMenu");



    //    QPixmap button( stonedark_xpm );

    QColor gold("#B9B9A5A54040"); //same as topgrad below
    QPixmap button( 1, 1 ); button.fill( gold );

    QPixmap background(marble_xpm);
    QPixmap dark( 1, 1 ); dark.fill( red.dark() );
    QPixmap mid( stone1_xpm );
    QPixmap light( stone1_xpm );//1, 1 ); light.fill( green );

    QPalette op = app->palette();

    QColor backCol( 227,227,227 );

    // QPalette op(white);
    QColorGroup active (op.active().foreground(),
		     QBrush(op.active().button(),button),
		     QBrush(op.active().light(), light),
		     QBrush(op.active().dark(), dark),
		     QBrush(op.active().mid(), mid),
		     op.active().text(),
		     Qt::white,
		     op.active().base(),//		     QColor(236,182,120),
		     QBrush(backCol, background)
		     );
    active.setColor( QColorGroup::ButtonText,  Qt::white  );
    active.setColor( QColorGroup::Shadow,  Qt::black  );
    QColorGroup disabled (op.disabled().foreground(),
		     QBrush(op.disabled().button(),button),
		     QBrush(op.disabled().light(), light),
		     op.disabled().dark(),
		     QBrush(op.disabled().mid(), mid),
		     op.disabled().text(),
		     Qt::white,
		     op.disabled().base(),//		     QColor(236,182,120),
		     QBrush(backCol, background)
		     );

    QPalette newPalette( active, disabled, active );
    app->setPalette( newPalette, TRUE );
}

/*!
  Reimplementation from QStyle
 */
void MetalStyle::unPolish( QApplication *app)
{
    app->setPalette(oldPalette, TRUE);
    app->setFont( app->font(), TRUE );
}

/*!
  Reimplementation from QStyle
 */
void MetalStyle::polish( QWidget* w)
{

   // the polish function sets some widgets to transparent mode and
    // some to translate background mode in order to get the full
    // benefit from the nice pixmaps in the color group.

    if (w->inherits("QPushButton")){
	w->setBackgroundMode( QWidget::NoBackground );
	return;
    }

    if ( !w->isTopLevel() ) {
	if ( w->backgroundPixmap() )
	    w->setBackgroundOrigin( QWidget::WindowOrigin );
    }
}

void MetalStyle::unPolish( QWidget* w)
{

   // the polish function sets some widgets to transparent mode and
    // some to translate background mode in order to get the full
    // benefit from the nice pixmaps in the color group.

    if (w->inherits("QPushButton")){
	w->setBackgroundMode( QWidget::PaletteButton );
	return;
    }
    if ( !w->isTopLevel() ) {
	if ( w->backgroundPixmap() )
	    w->setBackgroundOrigin( QWidget::WidgetOrigin );
    }

}

void MetalStyle::drawPrimitive( PrimitiveElement pe,
				QPainter *p,
				const QRect &r,
				const QColorGroup &cg,
				SFlags flags, const QStyleOption& opt ) const
{
    switch( pe ) {
    case PE_HeaderSection:
	if ( flags & Style_Sunken )
	    flags ^= Style_Sunken | Style_Raised;
	// fall through
    case PE_ButtonBevel:
    case PE_ButtonCommand:
	    drawMetalButton( p, r.x(), r.y(), r.width(), r.height(),
			     (flags & (Style_Sunken|Style_On|Style_Down)),
			     TRUE, !(flags & Style_Raised) );
	    break;
    case PE_PanelMenuBar:
	drawMetalFrame( p, r.x(), r.y(), r.width(), r.height() );
	break;
    case PE_ScrollBarAddLine:	
	drawMetalButton( p, r.x(), r.y(), r.width(), r.height(),
			 flags & Style_Down, !( flags & Style_Horizontal ) );
	drawPrimitive( (flags & Style_Horizontal) ? PE_ArrowRight :PE_ArrowDown,
		       p, r, cg, flags, opt );
	break;
    case PE_ScrollBarSubLine:
	drawMetalButton( p, r.x(), r.y(), r.width(), r.height(),
			 flags & Style_Down, !( flags & Style_Horizontal ) );
	drawPrimitive( (flags & Style_Horizontal) ? PE_ArrowLeft : PE_ArrowUp,
		       p, r, cg, flags, opt );
	break;

	
    case PE_ScrollBarSlider:
	drawMetalButton( p, r.x(), r.y(), r.width(), r.height(), FALSE,
			 flags & Style_Horizontal );
	break;
    default:
	QWindowsStyle::drawPrimitive( pe, p, r, cg, flags, opt );
	break;
    }
}

void MetalStyle::drawControl( ControlElement element,
			      QPainter *p,
			      const QWidget *widget,
			      const QRect &r,
			      const QColorGroup &cg,
			      SFlags how,
			      const QStyleOption& opt ) const
{
    switch( element ) {
    case CE_PushButton:
	{
	    const QPushButton *btn;
	    btn = (const QPushButton*)widget;
	    int x1, y1, x2, y2;
	
	    r.coords( &x1, &y1, &x2, &y2 );
	
	    p->setPen( cg.foreground() );
	    p->setBrush( QBrush(cg.button(), NoBrush) );

	
	    QBrush fill;
	    if ( btn->isDown() )
		fill = cg.brush( QColorGroup::Mid );
	    else if ( btn->isOn() )
		fill = QBrush( cg.mid(), Dense4Pattern );
	    else
		fill = cg.brush( QColorGroup::Button );
	
	    if ( btn->isDefault() ) {
		QPointArray a;
		a.setPoints( 9,
			     x1, y1, x2, y1, x2, y2, x1, y2, x1, y1+1,
			     x2-1, y1+1, x2-1, y2-1, x1+1, y2-1, x1+1, y1+1 );
		p->setPen( Qt::black );
		p->drawPolyline( a );
		x1 += 2;
		y1 += 2;
		x2 -= 2;
		y2 -= 2;
	    }
	    SFlags flags = Style_Default;
	    if ( btn->isOn() )
		flags |= Style_On;
	    if ( btn->isDown() )
		flags |= Style_Down;
	    if ( !btn->isFlat() && !btn->isDown() )
		flags |= Style_Raised;
	    drawPrimitive( PE_ButtonCommand, p,
			   QRect( x1, y1, x2 - x1 + 1, y2 - y1 + 1),
			   cg, flags, opt );
	
	    if ( btn->isMenuButton() ) {
		flags = Style_Default;
		if ( btn->isEnabled() )
		    flags |= Style_Enabled;
		
		int dx = ( y1 - y2 - 4 ) / 3;
		drawPrimitive( PE_ArrowDown, p,
			       QRect(x2 - dx, dx, y1, y2 - y1),
			       cg, flags, opt );
	    }
	    if ( p->brush().style() != NoBrush )
		p->setBrush( NoBrush );
	    break;
	}
    case CE_PushButtonLabel:
	{
	    const QPushButton *btn;
	    btn = (const QPushButton*)widget;
	    int x, y, w, h;
	    r.rect( &x, &y, &w, &h );
	
	    int x1, y1, x2, y2;
	    r.coords( &x1, &y1, &x2, &y2 );
	    int dx = 0;
	    int dy = 0;
	    if ( btn->isMenuButton() )
		dx = ( y2 - y1 ) / 3;
	    if ( btn->isOn() || btn->isDown() ) {
		dx--;
		dy--;
	    }
	    if ( dx || dy )
		p->translate( dx, dy );
	    x += 2;
	    y += 2;
	    w -= 4;
	    h -= 4;
	    drawItem( p, QRect( x, y, w, h ),
		      AlignCenter|ShowPrefix,
		      cg, btn->isEnabled(),
		      btn->pixmap(), btn->text(), -1,
		      (btn->isDown() || btn->isOn())? &cg.brightText() : &cg.buttonText() );
	    if ( dx || dy )
		p->translate( -dx, -dy );
	    break;
	}
    default:
	QWindowsStyle::drawControl( element, p, widget, r, cg, how, opt );
	break;
    }
}
void MetalStyle::drawComplexControl( ComplexControl cc,
				     QPainter *p,
				     const QWidget *widget,
				     const QRect &r,
				     const QColorGroup &cg,
				     SFlags how,
				     SCFlags sub,
				     SCFlags subActive,
				     const QStyleOption& opt ) const
{
    switch ( cc ) {
    case CC_Slider:
	{
	    const QSlider *slider = ( const QSlider* ) widget;
	    QRect handle = querySubControlMetrics( CC_Slider, widget,
						   SC_SliderHandle, opt);
	    if ( sub & SC_SliderGroove )
		QWindowsStyle::drawComplexControl( cc, p, widget, r, cg, how,
						   SC_SliderGroove, subActive, opt );
	    if ( (sub & SC_SliderHandle) && handle.isValid() )
		drawMetalButton( p, handle.x(), handle.y(), handle.width(),
				 handle.height(), FALSE,
				 slider->orientation() == QSlider::Horizontal);
	    break;
	}
    case CC_ComboBox:
	{
	    // not exactly correct...
	    const QComboBox *cmb = ( const QComboBox* ) widget;
	
	    qDrawWinPanel( p, r.x(), r.y(), r.width(), r.height(), cg, TRUE,
			   cmb->isEnabled() ? &cg.brush( QColorGroup::Base ) :
			                      &cg.brush( QColorGroup::Background ) );
	    drawMetalButton( p, r.x() + r.width() - 2 - 16, r.y() + 2, 16, r.height() - 4,
			     how & Style_Sunken, TRUE );
	    drawPrimitive( PE_ArrowDown, p,
			   QRect( r.x() + r.width() - 2 - 16 + 2,
				  r.y() + 2 + 2, 16 - 4, r.height() - 4 -4 ),
			   cg,
			   cmb->isEnabled() ? Style_Enabled : Style_Default,
			   opt );
	    break;
	}
    default:
	QWindowsStyle::drawComplexControl( cc, p, widget, r, cg, how, sub, subActive,
					   opt );
	break;
    }
}
		

/*!
  Draw a metallic button, sunken if \a sunken is TRUE, horizontal if
  /a horz is TRUE.
*/

void MetalStyle::drawMetalButton( QPainter *p, int x, int y, int w, int h,
				  bool sunken, bool horz, bool flat  ) const
{

    drawMetalFrame( p, x, y, w, h );
    drawMetalGradient( p, x, y, w, h, sunken, horz, flat );
}




void MetalStyle::drawMetalFrame( QPainter *p, int x, int y, int w, int h ) const
{
    QColor top1("#878769691515");
    QColor top2("#C6C6B4B44949");

    QColor bot2("#70705B5B1414");
    QColor bot1("#56564A4A0E0E"); //first from the bottom


    int x2 = x + w - 1;
    int y2 = y + h - 1;

    //frame:

    p->setPen( top1 );
    p->drawLine( x, y2, x, y );
    p->drawLine( x, y, x2-1, y );
    p->setPen( top2 );
    p->drawLine( x+1, y2 -1, x+1, y+1 );
    p->drawLine( x+1, y+1 , x2-2, y+1 );

    p->setPen( bot1 );
    p->drawLine( x+1, y2, x2, y2 );
    p->drawLine( x2, y2, x2, y );
    p->setPen( bot2 );
    p->drawLine( x+1, y2-1, x2-1, y2-1 );
    p->drawLine( x2-1, y2-1, x2-1, y+1 );


}


void MetalStyle::drawMetalGradient( QPainter *p, int x, int y, int w, int h,
				    bool sunken, bool horz, bool flat  ) const

{
    QColor highlight("#E8E8DDDD6565");
    QColor subh1("#CECEBDBD5151");
    QColor subh2("#BFBFACAC4545");

    QColor topgrad("#B9B9A5A54040");
    QColor botgrad("#89896C6C1A1A");



    if ( flat && !sunken ) {
	    p->fillRect( x + 2, y + 2, w - 4,h -4, topgrad );
    } else {
	// highlight:
	int i = 0;
	int x1 = x + 2;
	int y1 = y + 2;
	int x2 = x + w - 1;
	int y2 = y + h - 1;
	if ( horz )
	    x2 = x2 - 2;
	else
	    y2 = y2 - 2;
	
#define DRAWLINE if (horz) \
                    p->drawLine( x1, y1+i, x2, y1+i ); \
		 else \
                    p->drawLine( x1+i, y1, x1+i, y2 ); \
                 i++;

	if ( !sunken ) {
	    p->setPen( highlight );
	    DRAWLINE;
	    DRAWLINE;
	    p->setPen( subh1 );
	    DRAWLINE;
	    p->setPen( subh2 );
	    DRAWLINE;
	}
	// gradient:
	int ng = (horz ? h : w) - 8; // how many lines for the gradient?
	
	int h1, h2, s1, s2, v1, v2;
	if ( !sunken ) {
	    topgrad.hsv( &h1, &s1, &v1 );
	    botgrad.hsv( &h2, &s2, &v2 );
	} else {
	    botgrad.hsv( &h1, &s1, &v1 );
	    topgrad.hsv( &h2, &s2, &v2 );
	}
	
	if ( ng > 1 ) {	
	    for ( int j =0; j < ng; j++ ) {
		p->setPen( QColor( h1 + ((h2-h1)*j)/(ng-1),
				   s1 + ((s2-s1)*j)/(ng-1),
				   v1 + ((v2-v1)*j)/(ng-1),  QColor::Hsv ) );
		DRAWLINE;
	    }
	} else if ( ng == 1 ) {
	    p->setPen( QColor((h1+h2)/2, (s1+s2)/2, (v1+v2)/2, QColor::Hsv) );
	    DRAWLINE;
	}
	if ( sunken ) {
	    p->setPen( subh2 );
	    DRAWLINE;
	    
	    p->setPen( subh1 );
	    DRAWLINE;
	    
	    p->setPen( highlight );
	    DRAWLINE;
	    DRAWLINE;
	}
    }    
}



int MetalStyle::pixelMetric( PixelMetric metric, const QWidget *w ) const
{
    switch ( metric ) {
    case PM_MenuBarFrameWidth:
	return 2;
    default:
	return QWindowsStyle::pixelMetric( metric, w );
    }
}

#endif
