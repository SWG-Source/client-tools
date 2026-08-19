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

#ifndef PREFERENCEINTERFACEIMPL_H
#define PREFERENCEINTERFACEIMPL_H

#include <preferenceinterface.h>
class QWidget;

class PreferenceInterfaceImpl : public PreferenceInterface
{
public:
    PreferenceInterfaceImpl( QUnknownInterface *outer = 0 );
    virtual ~PreferenceInterfaceImpl();

    ulong addRef();
    ulong release();

    QRESULT queryInterface( const QUuid&, QUnknownInterface** );

    Preference *preference();
    void connectTo( QUnknownInterface * ) {}
    void deletePreferenceObject( Preference * );

private:
    QUnknownInterface *parent;
    int ref;
    QWidget *cppEditorSyntax;

};

#endif

