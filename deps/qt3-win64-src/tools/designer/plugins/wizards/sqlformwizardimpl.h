 /**********************************************************************
** Copyright (C) 2000-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of Qt Designer.
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

#ifndef SQLFORMWIZARDIMPL_H
#define SQLFORMWIZARDIMPL_H

#include "sqlformwizard.h"
#include <templatewizardiface.h>
#include <designerinterface.h>
#include <qvaluelist.h>

class SqlFormWizard : public SqlFormWizardBase
{
    Q_OBJECT

public:
    SqlFormWizard( QUnknownInterface *aIface, QWidget *w, QWidget* parent = 0, DesignerFormWindow *fw = 0,
		   const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~SqlFormWizard();

    void accept() const;

protected slots:
    void connectionSelected( const QString & );
    void tableSelected( const QString & );
    void autoPopulate( bool populate );
    void fieldDown();
    void fieldUp();
    void removeField();
    void addField();
    void setupDatabaseConnections();
    void accept();
    void addSortField();
    void reSortSortField();
    void removeSortField();
    void sortFieldUp();
    void sortFieldDown();
    void nextPageClicked();

private:
    void setupPage1();

private:
    QWidget *widget;
    QUnknownInterface *appIface;
    DesignerFormWindow *formWindow;
    enum Mode {
	None,
	View,
	Browser,
	Table
    };
    Mode mode;

};

#endif // SQLFORMWIZARDIMPL_H
