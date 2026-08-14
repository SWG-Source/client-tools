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

#ifndef YYREG_H
#define YYREG_H

#include <qstringlist.h>
#include <qvaluelist.h>

class CppFunction
{
public:
    CppFunction() : cnst( FALSE ), lineno1( 0 ), lineno2( 0 ) { }

    void setReturnType( const QString& r ) { ret = r; }
    void setScopedName( const QString& n ) { nam = n; }
    void setParameterList( const QStringList& p ) { params = p; }
    void setConst( bool c ) { cnst = c; }
    void setBody( const QString& b ) { bod = b; }
    void setDocumentation( const QString& d ) { doc = d; }
    void setLineNums( int functionStart, int openingBrace, int closingBrace ) {
	lineno0 = functionStart;
	lineno1 = openingBrace;
	lineno2 = closingBrace;
    }

    const QString& returnType() const { return ret; }
    const QString& scopedName() const { return nam; }
    const QStringList& parameterList() const { return params; }
    bool isConst() const { return cnst; }
    QString prototype() const;
    const QString& body() const { return bod; }
    const QString& documentation() const { return doc; }
    int functionStartLineNum() const { return lineno0; }
    int openingBraceLineNum() const { return lineno1; }
    int closingBraceLineNum() const { return lineno2; }

#if defined(Q_FULL_TEMPLATE_INSTANTIATION)
    bool operator==( const CppFunction& ) const { return FALSE; }
#endif

private:
    QString ret;
    QString nam;
    QStringList params;
    bool cnst;
    QString bod;
    QString doc;
    int lineno0;
    int lineno1;
    int lineno2;
};

void extractCppFunctions( const QString& code, QValueList<CppFunction> *flist );
QString canonicalCppProto( const QString& proto );

#endif
