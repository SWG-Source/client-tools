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

#ifndef SETTINGSDIALOGIMPL_H
#define SETTINGSDIALOGIMPL_H

#include "settingsdialog.h"

#include <qstringlist.h>
#include <qptrlist.h>
#include <qlistview.h>

class SettingsDialog : public SettingsDialogBase
{
    Q_OBJECT

public:
    SettingsDialog( QWidget *parent, const char* name = 0 );

protected slots:
    void selectColor();
    void browseWebApp();
    void browsePDFApplication();
    void browseHomepage();
    void accept();
    void reject();

private:
    void init();
    void setFile( QLineEdit *le, const QString &caption );
};

#endif
