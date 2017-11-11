/****************************************************************************
** $Id: qt/qdialog.h   3.3.4   edited Oct 8 2003 $
**
** Definition of QDialog class
**
** Created : 950502
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
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

#ifndef QDIALOG_H
#define QDIALOG_H

#ifndef QT_H
#include "qwidget.h"
#endif // QT_H
#ifndef QT_NO_DIALOG
#if 0
Q_OBJECT
#endif

class QPushButton;
class QDialogPrivate;

class Q_EXPORT QDialog : public QWidget
{
friend class QPushButton;
    Q_OBJECT
    Q_PROPERTY( bool sizeGripEnabled READ isSizeGripEnabled WRITE setSizeGripEnabled )
    Q_PROPERTY( bool modal READ isModal WRITE setModal )

public:
    Q_EXPLICIT QDialog( QWidget* parent=0, const char* name=0, bool modal=FALSE,
	     WFlags f=0 );
    ~QDialog();

    enum DialogCode { Rejected, Accepted };

    int		result() const { return rescode; }

    void	show();
    void	hide();
    void	move( int x, int y );
    void	move( const QPoint &p );
    void	resize( int w, int h );
    void	resize( const QSize & );
    void	setGeometry( int x, int y, int w, int h );
    void	setGeometry( const QRect & );

    void	setOrientation( Orientation orientation );
    Orientation	orientation() const;

    void	setExtension( QWidget* extension );
    QWidget*	extension() const;

    QSize	sizeHint() const;
    QSize	minimumSizeHint() const;

    void setSizeGripEnabled( bool );
    bool isSizeGripEnabled() const;

    void setModal( bool modal );
    bool isModal() const;
#ifdef Q_OS_TEMP
    bool	event( QEvent * );
#endif

public slots:
    int exec();

protected slots:
    virtual void done( int );
    virtual void accept();
    virtual void reject();

    void	showExtension( bool );

protected:
    void	setResult( int r )	{ rescode = r; }
    void	keyPressEvent( QKeyEvent * );
    void	closeEvent( QCloseEvent * );
    void	resizeEvent( QResizeEvent * );
    void	contextMenuEvent( QContextMenuEvent * );
    bool	eventFilter( QObject *, QEvent * );
    void	adjustPosition( QWidget*);

private:
    void	setDefault( QPushButton * );
    void	setMainDefault( QPushButton * );
    void	hideDefault();
#ifdef Q_OS_TEMP
    void	hideSpecial();
#endif

    int		rescode;
    uint	did_move   : 1;
    uint	has_relpos : 1;
    uint	did_resize : 1;
    uint	in_loop: 1;
    void adjustPositionInternal( QWidget*, bool useRelPos = FALSE );
    QDialogPrivate* d;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QDialog( const QDialog & );
    QDialog &operator=( const QDialog & );
#endif
};

#endif // QT_NO_DIALOG
#endif // QDIALOG_H
