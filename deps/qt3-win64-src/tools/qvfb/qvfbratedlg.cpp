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

#include "qvfbratedlg.h"

#include <qlayout.h>
#include <qlabel.h>
#include <qslider.h>
#include <qpushbutton.h>

QVFbRateDialog::QVFbRateDialog( int rate, QWidget *parent, const char *name,
    bool modal )
    : QDialog( parent, name, modal )
{
    oldRate = rate;

    QVBoxLayout *tl = new QVBoxLayout( this, 5 );

    QLabel *label = new QLabel( "Target frame rate:", this );
    tl->addWidget( label );

    QHBoxLayout *hl = new QHBoxLayout( tl );
    rateSlider = new QSlider( 1, 100, 10, rate, QSlider::Horizontal, this );
    hl->addWidget( rateSlider );
    connect( rateSlider, SIGNAL(valueChanged(int)), this, SLOT(rateChanged(int)) );
    rateLabel = new QLabel( QString( "%1fps" ).arg(rate), this );
    hl->addWidget( rateLabel );

    hl = new QHBoxLayout( tl );
    QPushButton *pb = new QPushButton( "OK", this );
    connect( pb, SIGNAL(clicked()), this, SLOT(accept()) );
    hl->addWidget( pb );
    pb = new QPushButton( "Cancel", this );
    connect( pb, SIGNAL(clicked()), this, SLOT(cancel()) );
    hl->addWidget( pb );
}

void QVFbRateDialog::rateChanged( int r )
{
    if ( rateSlider->value() != r )
	rateSlider->setValue( r );
    rateLabel->setText( QString( "%1fps" ).arg(r) );
    emit updateRate(r);
}

void QVFbRateDialog::cancel()
{
    rateChanged( oldRate );
    reject();
}
