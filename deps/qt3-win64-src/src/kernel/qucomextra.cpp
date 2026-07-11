/****************************************************************************
** $Id: qucomextra.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of extra QUcom classes
**
** Created : 990101
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech ASA of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
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
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qucomextra_p.h"
#include <qvariant.h>


#ifndef QT_NO_VARIANT
// 6dc75d58-a1d9-4417-b591-d45c63a3a4ea
const QUuid TID_QUType_QVariant( 0x6dc75d58, 0xa1d9, 0x4417, 0xb5, 0x91, 0xd4, 0x5c, 0x63, 0xa3, 0xa4, 0xea );
QUType_QVariant static_QUType_QVariant;

const QUuid *QUType_QVariant::uuid() const { return &TID_QUType_QVariant; }
const char *QUType_QVariant::desc() const { return "QVariant"; }

void QUType_QVariant::set( QUObject *o, const QVariant& v )
{
    o->payload.ptr = new QVariant( v );
    o->type = this;
}

QVariant &QUType_QVariant::get( QUObject * o )
{
    return *(QVariant*)o->payload.ptr;
}

bool QUType_QVariant::canConvertFrom( QUObject *o, QUType *t )
{
    if ( isEqual( o->type, &static_QUType_QString )
      || isEqual( o->type, &static_QUType_int )
      || isEqual( o->type, &static_QUType_bool )
      || isEqual( o->type, &static_QUType_double )
      || isEqual( o->type, &static_QUType_charstar ) )
	return TRUE;
    return t->canConvertTo( o, this );
}

bool QUType_QVariant::canConvertTo( QUObject * /*o*/, QUType * /*t*/ )
{
    return FALSE;
}

bool QUType_QVariant::convertFrom( QUObject *o, QUType *t )
{
    QVariant *var = 0;
    if ( isEqual( o->type, &static_QUType_QString ) )
	var = new QVariant( static_QUType_QString.get( o ) );
    else if ( isEqual( o->type, &static_QUType_int ) )
	var = new QVariant( static_QUType_int.get( o ) );
    else if ( isEqual( o->type, &static_QUType_bool ) )
	var = new QVariant( static_QUType_bool.get( o ), 42 );
    else if ( isEqual( o->type, &static_QUType_double ) )
	var = new QVariant( static_QUType_double.get( o ) );
    else if ( isEqual( o->type, &static_QUType_charstar ) )
	var = new QVariant( static_QUType_charstar.get( o ) );
    else
	return t->convertTo( o, this );

    o->type->clear( o );
    o->payload.ptr = var;
    o->type = this;
    return TRUE;
}

bool QUType_QVariant::convertTo( QUObject * /*o*/, QUType * /*t*/ )
{
    return FALSE;
}

void QUType_QVariant::clear( QUObject *o )
{
    delete (QVariant*)o->payload.ptr;
    o->payload.ptr = 0;
}

int QUType_QVariant::serializeTo( QUObject *, QUBuffer * )
{
    return 0;
}

int QUType_QVariant::serializeFrom( QUObject *, QUBuffer * )
{
    return 0;
}


#endif

const QUuid TID_QUType_varptr( 0x8d48b3a8, 0xbd7f, 0x11d5, 0x8d, 0x74, 0x00, 0xc0, 0xf0, 0x3b, 0xc0, 0xf3 );
QUType_varptr static_QUType_varptr;
const QUuid *QUType_varptr::uuid() const  { return &TID_QUType_varptr; }
const char *QUType_varptr::desc() const { return "varptr"; }

void QUType_varptr::set( QUObject *o, const void* v )
{
    o->payload.ptr = (void*) v;
    o->type = this;
}

bool QUType_varptr::canConvertFrom( QUObject *o, QUType *t )
{
     if ( isEqual( t, &static_QUType_ptr ) )
	return TRUE;
   return t->canConvertTo( o, this );
}

bool QUType_varptr::canConvertTo( QUObject *, QUType * t)
{
    return isEqual( t, &static_QUType_ptr );
}

bool QUType_varptr::convertFrom( QUObject *o, QUType *t )
{
     if ( isEqual( t, &static_QUType_ptr ) )
	;
    else
	return t->convertTo( o, this );

    o->type = this;
    return TRUE;
}

bool QUType_varptr::convertTo( QUObject *o, QUType * t)
{
    if ( isEqual( t,  &static_QUType_ptr ) ) {
	o->type = &static_QUType_ptr;
	return TRUE;
    }
    return FALSE;
}

int QUType_varptr::serializeTo( QUObject *, QUBuffer * )
{
    return 0;
}

int QUType_varptr::serializeFrom( QUObject *, QUBuffer * )
{
    return 0;
}
