/****************************************************************************
** $Id: qt/qglcolormap.h   3.3.4   edited May 27 2003 $
**
** Definition of QGLColormap class
**
** Created : 20010326
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the opengl module of the Qt GUI Toolkit.
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
** Licensees holding valid Qt Enterprise Edition licenses may use this
** file in accordance with the Qt Commercial License Agreement provided
** with the Software.
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

#ifndef QGLCOLORMAP_H
#define QGLCOLORMAP_H

#ifndef QT_H
#include "qcolor.h"
#include "qmemarray.h"
#include "qshared.h"
#endif // QT_H

#if !defined( QT_MODULE_OPENGL ) || defined( QT_LICENSE_PROFESSIONAL )
#define QM_EXPORT_OPENGL
#else
#define QM_EXPORT_OPENGL Q_EXPORT
#endif

class QWidget;
class QM_EXPORT_OPENGL QGLColormap
{
public:
    QGLColormap();
    QGLColormap( const QGLColormap & );
    ~QGLColormap();
    
    QGLColormap &operator=( const QGLColormap & );
    
    bool   isEmpty() const;
    int    size() const;
    void   detach();

    void   setEntries( int count, const QRgb * colors, int base = 0 );
    void   setEntry( int idx, QRgb color );
    void   setEntry( int idx, const QColor & color );
    QRgb   entryRgb( int idx ) const;
    QColor entryColor( int idx ) const;
    int    find( QRgb color ) const;
    int    findNearest( QRgb color ) const;
    
private:
    class Private : public QShared
    {
    public:
	Private() {
	    cells.resize( 256 ); // ### hardcoded to 256 entries for now
	    cmapHandle = 0;
	}

	~Private() {
	}

	QMemArray<QRgb> cells;
	Qt::HANDLE      cmapHandle;
    };
    
    Private * d;

    friend class QGLWidget;
};

#endif
