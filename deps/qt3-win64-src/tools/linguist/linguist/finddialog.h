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

#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <qdialog.h>

class QCheckBox;
class QLineEdit;

class FindDialog : public QDialog
{
    Q_OBJECT
public:
    enum { SourceText = 0x1, Translations = 0x2, Comments = 0x4 };

    FindDialog( bool replace, QWidget *parent = 0, const char *name = 0, bool modal = FALSE );

signals:
    void findNext( const QString& text, int where, bool matchCase );
    void replace( const QString& before, const QString& after, bool matchCase, bool all );

private slots:
    void emitFindNext();
    void emitReplace();
    void emitReplaceAll();

private:
    QLineEdit *led;
    QLineEdit *red;
    QCheckBox *sourceText;
    QCheckBox *translations;
    QCheckBox *comments;
    QCheckBox *matchCase;
};

#endif
