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

#ifndef TOPICCHOOSERIMPL_H
#define TOPICCHOOSERIMPL_H

#include "topicchooser.h"

#include <qstringlist.h>

class TopicChooser : public TopicChooserBase
{
    Q_OBJECT
    
public:
    TopicChooser( QWidget *parent, const QStringList &lnkNames,
		  const QStringList &lnks, const QString &title );
    
    QString link() const;
    
    static QString getLink( QWidget *parent, const QStringList &lnkNames,
			    const QStringList &lnks, const QString &title );
    
private:
    QString theLink;
    QStringList links, linkNames;

};

#endif
