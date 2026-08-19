/**********************************************************************
**
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

#include <filterinterface.h>
#include <qapplication.h>

#include "dlg2ui.h"

class DlgFilter : public ImportFilterInterface, public QLibraryInterface
{
public:
    DlgFilter();

    QRESULT queryInterface( const QUuid&, QUnknownInterface **iface );
    Q_REFCOUNT;

    QStringList featureList() const;
    QStringList import( const QString& filter, const QString& filename );

    bool init();
    void cleanup();
    bool canUnload() const;
};

DlgFilter::DlgFilter()
{
}

QRESULT DlgFilter::queryInterface( const QUuid &uuid, QUnknownInterface **iface )
{
    *iface = 0;
    if ( uuid == IID_QUnknown )
	*iface = (QUnknownInterface*)(ImportFilterInterface*)this;
    else if ( uuid == IID_QFeatureList )
	*iface = (QFeatureListInterface*)this;
    else if ( uuid == IID_ImportFilter )
	*iface = (ImportFilterInterface*)this;
    else if ( uuid == IID_QLibrary )
	*iface = (QLibraryInterface*)this;
    else
	return QE_NOINTERFACE;

    (*iface)->addRef();
    return QS_OK;
}

QStringList DlgFilter::featureList() const
{
    QStringList list;
    list << "Qt Architect Dialog Files (*.dlg)" ;
    return list;
}

QStringList DlgFilter::import( const QString &, const QString& filename )
{
    Dlg2Ui d;
    return d.convertQtArchitectDlgFile( filename );
}

bool DlgFilter::init()
{
    return TRUE;
}

void DlgFilter::cleanup()
{
}

bool DlgFilter::canUnload() const
{
    return TRUE;
}

Q_EXPORT_COMPONENT()
{
    Q_CREATE_INSTANCE( DlgFilter )
}
