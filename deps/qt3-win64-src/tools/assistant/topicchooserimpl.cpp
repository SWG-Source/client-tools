/**********************************************************************
** Copyright (C) 2000-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of Qt Assistant.
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

#include "topicchooserimpl.h"

#include <qlabel.h>
#include <qlistbox.h>
#include <qpushbutton.h>

TopicChooser::TopicChooser( QWidget *parent, const QStringList &lnkNames,
			    const QStringList &lnks, const QString &title )
    : TopicChooserBase( parent, 0, TRUE ), links( lnks ), linkNames( lnkNames )
{
    label->setText( tr( "Choose a topic for <b>%1</b>" ).arg( title ) );
    listbox->insertStringList( linkNames );
    listbox->setCurrentItem( listbox->firstItem() );
    listbox->setFocus();
}

QString TopicChooser::link() const
{
    if ( listbox->currentItem() == -1 )
	return QString::null;
    QString s = listbox->currentText();
    if ( s.isEmpty() )
	return s;
    int i = linkNames.findIndex( s );
    return links[ i ];
}

QString TopicChooser::getLink( QWidget *parent, const QStringList &lnkNames,
				      const QStringList &lnks, const QString &title )
{
    TopicChooser *dlg = new TopicChooser( parent, lnkNames, lnks, title );
    QString lnk;
    if ( dlg->exec() == QDialog::Accepted )
	lnk = dlg->link();
    delete dlg;
    return lnk;
}
