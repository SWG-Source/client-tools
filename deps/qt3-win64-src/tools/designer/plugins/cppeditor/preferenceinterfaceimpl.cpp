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

#include "preferenceinterfaceimpl.h"
#include <preferences.h>

PreferenceInterfaceImpl::PreferenceInterfaceImpl( QUnknownInterface *outer )
    : parent( outer ),
      ref( 0 ),
      cppEditorSyntax( 0 )
{
}

PreferenceInterfaceImpl::~PreferenceInterfaceImpl()
{
}

ulong PreferenceInterfaceImpl::addRef()
{
    return parent ? parent->addRef() : ref++;
}

ulong PreferenceInterfaceImpl::release()
{
    if ( parent )
	return parent->release();
    if ( !--ref ) {
	delete this;
	return 0;
    }
    return ref;
}

QRESULT PreferenceInterfaceImpl::queryInterface( const QUuid &uuid, QUnknownInterface** iface )
{
    if ( parent )
	return parent->queryInterface( uuid, iface );

    *iface = 0;
    if ( uuid == IID_QUnknown )
	*iface = (QUnknownInterface*)this;
    else if ( uuid == IID_Preference )
	*iface = (PreferenceInterface*)this;
    else
	return QE_NOINTERFACE;

    (*iface)->addRef();
    return QS_OK;
}

PreferenceInterface::Preference *PreferenceInterfaceImpl::preference()
{
    if ( !cppEditorSyntax ) {
	cppEditorSyntax = new PreferencesBase( 0, "cppeditor_syntax" );
	( (PreferencesBase*)cppEditorSyntax )->setPath( "/Trolltech/CppEditor/" );
	cppEditorSyntax->hide();
    }
    Preference *pf = 0;
    pf = new Preference;
    pf->tab = cppEditorSyntax;
    pf->title = "C++ Editor";
    pf->receiver = pf->tab;
    pf->init_slot = SLOT( reInit() );
    pf->accept_slot = SLOT( save() );
    return pf;
}

void PreferenceInterfaceImpl::deletePreferenceObject( Preference *p )
{
    delete p;
}
