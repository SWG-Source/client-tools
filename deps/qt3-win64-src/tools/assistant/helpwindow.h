/**********************************************************************
** Copyright (C) 2000-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of the Qt Assistant.
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
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef HELPWINDOW_H
#define HELPWINDOW_H

#include <qtextbrowser.h>

class MainWindow;
class QKeyEvent;
class QMime;
class QMouseEvent;

class HelpWindow : public QTextBrowser
{
    Q_OBJECT
public:
    HelpWindow( MainWindow *m, QWidget *parent = 0, const char *name = 0 );
    void setSource( const QString &name );
    QPopupMenu *createPopupMenu( const QPoint& pos );
    void blockScrolling( bool b );
    void openLinkInNewWindow( const QString &link );
    void openLinkInNewPage( const QString &link );
    void addMimePath( const QString &path );

    void contentsMousePressEvent(QMouseEvent *e);
    void keyPressEvent(QKeyEvent *);

    bool isBackwardAvailable() const { return backAvail; }
    bool isForwardAvailable() const { return fwdAvail; }

signals:
    void chooseWebBrowser();

public slots:
    void copy();
protected slots:
    void ensureCursorVisible();

private slots:
    void openLinkInNewWindow();
    void openLinkInNewPage();
    void updateForward(bool);
    void updateBackward(bool);

private:
    MainWindow *mw;
    QString lastAnchor;
    bool blockScroll;
    bool shiftPressed;
    bool newWindow;
    QMimeSourceFactory *mimeSourceFactory;
    bool fwdAvail;
    bool backAvail;
};

#endif
