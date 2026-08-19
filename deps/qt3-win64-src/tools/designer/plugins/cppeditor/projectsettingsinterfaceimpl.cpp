/**********************************************************************
**
** Copyright (C) 2005-2007 Trolltech ASA.  All rights reserved.
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

#include "projectsettingsinterfaceimpl.h"
#include "projectsettings.h"

ProjectSettingsInterfaceImpl::ProjectSettingsInterfaceImpl( QUnknownInterface *outer )
    : parent( outer ),
      ref( 0 ),
      settingsTab( 0 )
{
}

ulong ProjectSettingsInterfaceImpl::addRef()
{
    return parent ? parent->addRef() : ref++;
}

ulong ProjectSettingsInterfaceImpl::release()
{
    if ( parent )
	return parent->release();
    if ( !--ref ) {
	delete this;
	return 0;
    }
    return ref;
}

ProjectSettingsInterface::ProjectSettings *ProjectSettingsInterfaceImpl::projectSetting()
{
    if ( !settingsTab ) {
	settingsTab = new CppProjectSettings( 0 );
	settingsTab->hide();
    }
    ProjectSettings *pf = 0;
    pf = new ProjectSettings;
    pf->tab = settingsTab;
    pf->title = "C++";
    pf->receiver = pf->tab;
    pf->init_slot = SLOT( reInit( QUnknownInterface * ) );
    pf->accept_slot = SLOT( save( QUnknownInterface * ) );
    return pf;
}

QStringList ProjectSettingsInterfaceImpl::projectSettings() const
{
    return QStringList();
}

void ProjectSettingsInterfaceImpl::connectTo( QUnknownInterface * )
{
}

void ProjectSettingsInterfaceImpl::deleteProjectSettingsObject( ProjectSettings *pf )
{
    delete pf;
}

QRESULT ProjectSettingsInterfaceImpl::queryInterface( const QUuid &uuid, QUnknownInterface **iface )
{
    if ( parent )
	return parent->queryInterface( uuid, iface );

    *iface = 0;
    if ( uuid == IID_QUnknown )
	*iface = (QUnknownInterface*)this;
    else if ( uuid == IID_ProjectSettings )
	*iface = (ProjectSettingsInterface*)this;
    else
	return QE_NOINTERFACE;

    (*iface)->addRef();
    return QS_OK;
}
