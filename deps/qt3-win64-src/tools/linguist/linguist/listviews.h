/**********************************************************************
** Copyright (C) 2000-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of Qt Linguist.
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

#ifndef LISTVIEWS_H
#define LISTVIEWS_H

#include "msgedit.h"

#include <qstring.h>
#include <qptrlist.h>
#include <qregexp.h>
#include <qlistview.h>

class LVI : public QListViewItem
{
public:
    LVI( QListView *parent, QString text = QString::null );
    LVI( QListViewItem *parent, QString text = QString::null );
    virtual int compare( QListViewItem *other, int column,
			 bool ascending ) const;
    virtual bool danger() const { return FALSE; }

protected:
    void drawObsoleteText( QPainter * p, const QColorGroup & cg, int column,
			   int width, int align );

private:
    static int count;
};

class MessageLVI;
class ContextLVI : public LVI
{
public:
    ContextLVI( QListView *lv, const QString& context );

    virtual bool danger() const { return dangerCount > 0; }

    void appendToComment( const QString& x );
    void incrementUnfinishedCount();
    void decrementUnfinishedCount();
    void incrementDangerCount();
    void decrementDangerCount();
    void incrementObsoleteCount();
    bool isContextObsolete();
    void updateStatus();

    QString context() const { return text( 1 ); }
    QString comment() const { return com; }
    QString fullContext() const;
    bool    finished() const { return unfinishedCount == 0; }

    MessageLVI * firstMessageItem() { return messageItems.first(); }
    MessageLVI * nextMessageItem() { return messageItems.next(); }
    MessageLVI * takeMessageItem( int i ) { return messageItems.take( i ); }
    void         appendMessageItem( QListView * lv, MessageLVI * i );
    void         instantiateMessageItem( QListView * lv, MessageLVI * i );
    int          messageItemsInList() { return messageItems.count(); }

    void paintCell( QPainter * p, const QColorGroup & cg, int column,
		    int width, int align );
private:
    QPtrList<MessageLVI> messageItems;
    QString com;
    int unfinishedCount;
    int dangerCount;
    int obsoleteCount;
    int itemCount;
};

class MessageLVI : public LVI
{
public:
    MessageLVI( QListView *parent, const MetaTranslatorMessage & message,
		const QString& text, const QString& comment, ContextLVI * c );

    virtual bool danger() const { return d; }

    void setTranslation( const QString& translation );
    void setFinished( bool finished );
    void setDanger( bool danger );

    void setContextLVI( ContextLVI * c ) { ctxt = c; }
    ContextLVI * contextLVI() const { return ctxt; }
    void updateTranslationText();
    
    QString context() const;
    QString sourceText() const { return tx; }
    QString comment() const { return com; }
    QString translation() const { return m.translation(); }
    bool finished() const { return fini; }
    MetaTranslatorMessage message() const;

    void paintCell( QPainter * p, const QColorGroup & cg, int column,
		    int width, int align );
private:
    MetaTranslatorMessage m;
    QString tx;
    QString com;
    bool fini;
    bool d;
    ContextLVI * ctxt;
};

#endif
