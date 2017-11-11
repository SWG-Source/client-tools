/****************************************************************************
** $Id: qt/qmotifstyle.h   3.3.4   edited Apr 2 2004 $
**
** Definition of Motif-like style class
**
** Created : 981231
**
** Copyright (C) 1998-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
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

#ifndef QMOTIFSTYLE_H
#define QMOTIFSTYLE_H

#ifndef QT_H
#include "qcommonstyle.h"
#endif // QT_H

#if !defined(QT_NO_STYLE_MOTIF) || defined(QT_PLUGIN)

class QPalette;

#if defined(QT_PLUGIN)
#define Q_EXPORT_STYLE_MOTIF
#else
#define Q_EXPORT_STYLE_MOTIF Q_EXPORT
#endif


class Q_EXPORT_STYLE_MOTIF QMotifStyle : public QCommonStyle
{
    Q_OBJECT
public:
    QMotifStyle( bool useHighlightCols=FALSE );
    virtual ~QMotifStyle();

    void setUseHighlightColors( bool );
    bool useHighlightColors() const;

    void polish( QPalette& );
    void polish( QWidget* );
    void polish( QApplication* );

    void polishPopupMenu( QPopupMenu* );

    // new style API
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

    void drawComplexControl( ComplexControl control,
			     QPainter *p,
			     const QWidget* widget,
			     const QRect& r,
			     const QColorGroup& cg,
			     SFlags how = Style_Default,
#ifdef Q_QDOC
			     SCFlags sub = SC_All,
#else
			     SCFlags sub = (uint)SC_All,
#endif
			     SCFlags subActive = SC_None,
			     const QStyleOption& = QStyleOption::Default ) const;

    QRect querySubControlMetrics( ComplexControl control,
				  const QWidget *widget,
				  SubControl sc,
				  const QStyleOption& = QStyleOption::Default ) const;

    int pixelMetric( PixelMetric metric, const QWidget *widget = 0 ) const;

    QSize sizeFromContents( ContentsType contents,
			    const QWidget *widget,
			    const QSize &contentsSize,
			    const QStyleOption& = QStyleOption::Default ) const;

    QRect subRect( SubRect r, const QWidget *widget ) const;

    QPixmap stylePixmap(StylePixmap, const QWidget * = 0, const QStyleOption& = QStyleOption::Default) const;

    int styleHint(StyleHint sh, const QWidget *, const QStyleOption & = QStyleOption::Default,
		  QStyleHintReturn* = 0) const;

private:
    bool highlightCols;

    // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QMotifStyle( const QMotifStyle & );
    QMotifStyle& operator=( const QMotifStyle & );
#endif
};

#endif // QT_NO_STYLE_MOTIF

#endif // QMOTIFSTYLE_H
