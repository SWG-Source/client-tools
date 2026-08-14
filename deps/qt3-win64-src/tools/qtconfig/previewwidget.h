/**********************************************************************
** Copyright (C) 2000-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of Qt Configuration.
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

#ifndef PREVIEWWIDGET_H
#define PREVIEWWIDGET_H

#include "previewwidgetbase.h"

class PreviewWidget : public PreviewWidgetBase
{
    Q_OBJECT

public:
    PreviewWidget( QWidget *parent = 0, const char *name = 0 );

    void closeEvent(QCloseEvent *);
    bool eventFilter(QObject *, QEvent *);
};

#endif
