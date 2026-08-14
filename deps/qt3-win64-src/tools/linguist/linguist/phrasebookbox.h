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

#ifndef PHRASEBOOKBOX_H
#define PHRASEBOOKBOX_H

#include "phrase.h"

#include <qdialog.h>
#include <qvaluelist.h>

class QLineEdit;
class QPushButton;
class QListViewItem;

class PhraseLV;

class PhraseBookBox : public QDialog
{
    Q_OBJECT
public:
    PhraseBookBox( const QString& filename, const PhraseBook& phraseBook,
		   QWidget *parent = 0, const char *name = 0,
		   bool modal = FALSE );

    const PhraseBook& phraseBook() const { return pb; }

protected:
    virtual void keyPressEvent( QKeyEvent *ev );

private slots:
    void newPhrase();
    void removePhrase();
    void save();
    void sourceChanged( const QString& source );
    void targetChanged( const QString& target );
    void definitionChanged( const QString& definition );
    void selectionChanged( QListViewItem *item );

private:
    void selectItem( QListViewItem *item );
    void enableDisable();

    QLineEdit *sourceLed;
    QLineEdit *targetLed;
    QLineEdit *definitionLed;
    QPushButton *newBut;
    QPushButton *removeBut;
    PhraseLV *lv;
    QString fn;
    PhraseBook pb;
};

#endif
