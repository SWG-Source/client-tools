/**********************************************************************
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of Qt/Embedded virtual framebuffer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef GAMMAVIEW_H
#define GAMMAVIEW_H

#include <qwidget.h>

class GammaView: public QWidget
{
    Q_OBJECT
public:
    GammaView( QWidget *parent = 0,
		const char *name = 0, WFlags f = 0 ) :
	QWidget(parent,name,f)
    { }
};

#endif
