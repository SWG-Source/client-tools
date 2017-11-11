/**********************************************************************
** $Id: qt/qmultilineedit.h   3.3.4   edited May 27 2003 $
**
** Definition of QMultiLineEdit widget class
**
** Created : 961005
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
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

#ifndef QMULTILINEEDIT_H
#define QMULTILINEEDIT_H

#ifndef QT_H
#include "qtextedit.h"
#endif // QT_H

#ifndef QT_NO_MULTILINEEDIT

class QMultiLineEditCommand;
class QValidator;
class QMultiLineEditData;

class Q_EXPORT QMultiLineEdit : public QTextEdit
{
    Q_OBJECT
    Q_PROPERTY( int numLines READ numLines )
    Q_PROPERTY( bool atBeginning READ atBeginning )
    Q_PROPERTY( bool atEnd READ atEnd )
    Q_PROPERTY( Alignment alignment READ alignment WRITE setAlignment )
    Q_PROPERTY( bool edited READ edited WRITE setEdited DESIGNABLE false )

public:
    QMultiLineEdit( QWidget* parent=0, const char* name=0 );
    ~QMultiLineEdit();

    QString textLine( int line ) const;
    int numLines() const;

    virtual void insertLine( const QString &s, int line = -1 );
    virtual void insertAt( const QString &s, int line, int col ) {
	insertAt( s, line, col, FALSE );
    }
    virtual void insertAt( const QString &s, int line, int col, bool mark );
    virtual void removeLine( int line );
    virtual void setCursorPosition( int line, int col ) {
	setCursorPosition( line, col, FALSE );
    }
    virtual void setCursorPosition( int line, int col, bool mark );
    bool atBeginning() const;
    bool atEnd() const;

    void setAlignment( int flags );
    int alignment() const;

    void setEdited( bool );
    bool edited() const;

    bool hasMarkedText() const;
    QString markedText() const;

    void cursorWordForward( bool mark );
    void cursorWordBackward( bool mark );

    // noops
    bool autoUpdate() const { return TRUE; }
    virtual void setAutoUpdate( bool ) {}

    int totalWidth() const { return contentsWidth(); }
    int totalHeight() const { return contentsHeight(); }

    int maxLines() const { return QWIDGETSIZE_MAX; }
    void setMaxLines( int ) {}

public slots:
    void deselect() { selectAll( FALSE ); }

protected:
    QPoint	cursorPoint() const;

protected:
    virtual void insertAndMark( const QString&, bool mark );
    virtual void newLine();
    virtual void killLine();
    virtual void pageUp( bool mark=FALSE );
    virtual void pageDown( bool mark=FALSE );
    virtual void cursorLeft( bool mark=FALSE, bool wrap = TRUE );
    virtual void cursorRight( bool mark=FALSE, bool wrap = TRUE );
    virtual void cursorUp( bool mark=FALSE );
    virtual void cursorDown( bool mark=FALSE );
    virtual void backspace();
    virtual void home( bool mark=FALSE );
    virtual void end( bool mark=FALSE );

    bool getMarkedRegion( int *line1, int *col1,
			  int *line2, int *col2 ) const;
    int lineLength( int row ) const;

private:
    QMultiLineEditData *d;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QMultiLineEdit( const QMultiLineEdit & );
    QMultiLineEdit &operator=( const QMultiLineEdit & );
#endif
};

#endif // QT_NO_MULTILINEEDIT

#endif // QMULTILINED_H
