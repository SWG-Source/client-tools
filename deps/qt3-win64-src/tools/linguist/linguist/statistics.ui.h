/**********************************************************************
** Copyright (C) 2005-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of Qt Linguist.
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

void Statistics::init()
{
    setFixedHeight( sizeHint().height() );
}

void Statistics::updateStats( int w1, int c1, int cs1, int w2, int c2, int cs2 )
{
    untrWords->setText( QString::number( w1 ) );
    untrChars->setText( QString::number( c1 ) );
    untrCharsSpc->setText( QString::number( cs1 ) );
    trWords->setText( QString::number( w2 ) );
    trChars->setText( QString::number( c2 ) );
    trCharsSpc->setText( QString::number( cs2 ) );
}

void Statistics::closeEvent( QCloseEvent * e )
{
    emit closed();
    QDialog::closeEvent( e );
}
