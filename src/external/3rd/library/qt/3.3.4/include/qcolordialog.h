/****************************************************************************
** $Id: qt/qcolordialog.h   3.3.4   edited May 27 2003 $
**
** Definition of QColorDialog class
**
** Created : 990222
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the dialogs module of the Qt GUI Toolkit.
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

#ifndef QCOLORDIALOG_H
#define QCOLORDIALOG_H

#ifndef QT_H
#include "qdialog.h"
#endif // QT_H

#ifndef QT_NO_COLORDIALOG

class QColorDialogPrivate;

class Q_EXPORT QColorDialog : public QDialog
{
    Q_OBJECT

public:
    static QColor getColor( const QColor& init = white, QWidget* parent=0, const char* name=0 );
    static QRgb getRgba( QRgb, bool* ok = 0,
			 QWidget* parent=0, const char* name=0 );

    static int customCount();
    static QRgb customColor( int );
    static void setCustomColor( int, QRgb );
    static void setStandardColor( int, QRgb );

private:
    ~QColorDialog();
    QColorDialog( QWidget* parent=0, const char* name=0, bool modal=FALSE );

    void setColor( const QColor& );
    QColor color() const;

    bool selectColor( const QColor& );

    void setSelectedAlpha( int );
    int selectedAlpha() const;

    void showCustom( bool=TRUE );

private:	// Disabled copy constructor and operator=
    QColorDialogPrivate *d;
    friend class QColorDialogPrivate;
    friend class QColorShower;

#if defined(Q_DISABLE_COPY)
    QColorDialog( const QColorDialog & );
    QColorDialog& operator=( const QColorDialog & );
#endif
};

#endif

#endif //QCOLORDIALOG_H
