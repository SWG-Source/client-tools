/****************************************************************************
** $Id: qt/qtextbrowser.h   3.3.4   edited May 27 2003 $
**
** Definition of the QTextBrowser class
**
** Created : 990101
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

#ifndef QTEXTBROWSER_H
#define QTEXTBROWSER_H

#ifndef QT_H
#include "qptrlist.h"
#include "qpixmap.h"
#include "qcolor.h"
#include "qtextedit.h"
#endif // QT_H

#ifndef QT_NO_TEXTBROWSER

class QTextBrowserData;

class Q_EXPORT QTextBrowser : public QTextEdit
{
    Q_OBJECT
    Q_PROPERTY( QString source READ source WRITE setSource )
    Q_OVERRIDE( int undoDepth DESIGNABLE false SCRIPTABLE false )
    Q_OVERRIDE( bool overwriteMode DESIGNABLE false SCRIPTABLE false )
    Q_OVERRIDE( bool modified SCRIPTABLE false)
    Q_OVERRIDE( bool readOnly DESIGNABLE false SCRIPTABLE false )
    Q_OVERRIDE( bool undoRedoEnabled DESIGNABLE false SCRIPTABLE false )

public:
    QTextBrowser( QWidget* parent=0, const char* name=0 );
    ~QTextBrowser();

    QString source() const;

public slots:
    virtual void setSource(const QString& name);
    virtual void backward();
    virtual void forward();
    virtual void home();
    virtual void reload();
    void setText( const QString &txt ) { setText( txt, QString::null ); }
    virtual void setText( const QString &txt, const QString &context );

signals:
    void backwardAvailable( bool );
    void forwardAvailable( bool );
    void sourceChanged( const QString& );
    void highlighted( const QString& );
    void linkClicked( const QString& );
    void anchorClicked( const QString&, const QString& );

protected:
    void keyPressEvent( QKeyEvent * e);

private:
    void popupDetail( const QString& contents, const QPoint& pos );
    bool linksEnabled() const { return TRUE; }
    void emitHighlighted( const QString &s );
    void emitLinkClicked( const QString &s );
    QTextBrowserData *d;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QTextBrowser( const QTextBrowser & );
    QTextBrowser& operator=( const QTextBrowser & );
#endif
};

#endif // QT_NO_TEXTBROWSER

#endif // QTEXTBROWSER_H
