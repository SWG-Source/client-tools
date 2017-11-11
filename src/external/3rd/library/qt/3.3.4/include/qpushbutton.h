/****************************************************************************
** $Id: qt/qpushbutton.h   3.3.4   edited May 27 2003 $
**
** Definition of QPushButton class
**
** Created : 940221
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
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

#ifndef QPUSHBUTTON_H
#define QPUSHBUTTON_H

#ifndef QT_H
#include "qbutton.h"
#include "qiconset.h"
#endif // QT_H

#ifndef QT_NO_PUSHBUTTON
class QPushButtonPrivate;
class QPopupMenu;

class Q_EXPORT QPushButton : public QButton
{
    Q_OBJECT

    Q_PROPERTY( bool autoDefault READ autoDefault WRITE setAutoDefault )
    Q_PROPERTY( bool default READ isDefault WRITE setDefault )
    Q_PROPERTY( bool menuButton READ isMenuButton DESIGNABLE false )
    Q_PROPERTY( QIconSet iconSet READ iconSet WRITE setIconSet )
    Q_OVERRIDE( bool toggleButton WRITE setToggleButton )
    Q_OVERRIDE( bool on WRITE setOn )
    Q_PROPERTY( bool flat READ isFlat WRITE setFlat )
    Q_OVERRIDE( bool autoMask DESIGNABLE true SCRIPTABLE true )

public:
    QPushButton( QWidget *parent, const char* name=0 );
    QPushButton( const QString &text, QWidget *parent, const char* name=0 );
#ifndef QT_NO_ICONSET
    QPushButton( const QIconSet& icon, const QString &text, QWidget *parent, const char* name=0 );
#endif
    ~QPushButton();

    QSize	sizeHint() const;

    void	move( int x, int y );
    void	move( const QPoint &p );
    void	resize( int w, int h );
    void	resize( const QSize & );
    void	setGeometry( int x, int y, int w, int h );

    void	setGeometry( const QRect & );

    void setToggleButton( bool );

    bool	autoDefault()	const	{ return autoDefButton; }
    virtual void setAutoDefault( bool autoDef );
    bool	isDefault()	const	{ return defButton; }
    virtual void setDefault( bool def );

    virtual void setIsMenuButton( bool enable ) {  // obsolete functions
	if ( (bool)hasMenuArrow == enable )
	    return;
	hasMenuArrow = enable ? 1 : 0;
	update();
	updateGeometry();
    }
    bool	isMenuButton() const { return hasMenuArrow; }

#ifndef QT_NO_POPUPMENU
    void setPopup( QPopupMenu* popup );
    QPopupMenu* popup() const;
#endif
#ifndef QT_NO_ICONSET
    void setIconSet( const QIconSet& );
    QIconSet* iconSet() const;
#endif
    void setFlat( bool );
    bool isFlat() const;

public slots:
    virtual void setOn( bool );

protected:
    void	drawButton( QPainter * );
    void	drawButtonLabel( QPainter * );
    void	focusInEvent( QFocusEvent * );
    void	focusOutEvent( QFocusEvent * );
    void	resizeEvent( QResizeEvent * );
    void	updateMask();
private slots:
#ifndef QT_NO_POPUPMENU
    void popupPressed();
#endif
private:
    void	init();

    uint	autoDefButton	: 1;
    uint	defButton	: 1;
    uint	flt		: 1;
    uint	reserved		: 1; // UNUSED
    uint	lastEnabled	: 1; // UNUSED
    uint	hasMenuArrow	: 1;

    QPushButtonPrivate* d;

    friend class QDialog;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QPushButton( const QPushButton & );
    QPushButton &operator=( const QPushButton & );
#endif
};


#endif // QT_NO_PUSHBUTTON

#endif // QPUSHBUTTON_H
