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

#include <qapplication.h>

void FindDialog::init()
{
    lastBrowser = 0;
    onceFound = FALSE;
    findExpr = "";
    sb = new QStatusBar( this );
    FindDialogLayout->addWidget( sb );
    sb->message( tr( "Enter the text you are looking for." ) );
}

void FindDialog::destroy()
{
}

void FindDialog::doFind()
{
    doFind(radioForward->isChecked());
}

void FindDialog::doFind(bool forward)
{
    QTextBrowser *browser = (QTextBrowser*) mainWindow()->browsers()->currentBrowser();
    sb->clear();

    if (comboFind->currentText() != findExpr || lastBrowser != browser)
	onceFound = false;
    findExpr = comboFind->currentText();

    bool found;
    if (browser->hasSelectedText()) { // Search either forward or backward from cursor.
	found = browser->find(findExpr, checkCase->isChecked(), checkWords->isChecked(),
			      forward);
    } else {
	int para = forward ? 0 : INT_MAX;
	int index = forward ? 0 : INT_MAX;
	found = browser->find(findExpr, checkCase->isChecked(), checkWords->isChecked(),
			      forward, &para, &index);
    }

    if (!found) {
	if (onceFound) {
	    if (forward)
		statusMessage(tr("Search reached end of the document"));
	    else
		statusMessage(tr("Search reached start of the document"));
	} else {
	    statusMessage(tr( "Text not found" ));
	}
    }
    onceFound |= found;
    lastBrowser = browser;
}



MainWindow* FindDialog::mainWindow()
{
    return (MainWindow*) parent();
}

bool FindDialog::hasFindExpression()
{
    return !findExpr.isEmpty();
}

void FindDialog::statusMessage(const QString &message)
{
    if (isVisible())
	sb->message(message);
    else
	((MainWindow*) parent())->statusBar()->message(message, 2000);

}
