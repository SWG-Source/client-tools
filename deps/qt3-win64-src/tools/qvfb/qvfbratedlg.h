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

#include <qdialog.h>

class QLabel;
class QSlider;

class QVFbRateDialog : public QDialog
{
    Q_OBJECT
public:
    QVFbRateDialog( int value, QWidget *parent=0, const char *name=0,
		    bool modal=FALSE );

signals:
    void updateRate( int r );

protected slots:
    void rateChanged( int r );
    void cancel();

private:
    QLabel *rateLabel;
    QSlider *rateSlider;
    int oldRate;
};


