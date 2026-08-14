#include "remotectrlimpl.h"

#include <qpushbutton.h>
#include <qlineedit.h>
#include <qsocket.h>
#include <qfiledialog.h>
#include <qcolordialog.h>
#include <qimage.h>

RemoteCtrlImpl::RemoteCtrlImpl( QSocket *s )
{
    socket = s;
    connect( sImage, SIGNAL(clicked()), SLOT(sendImage()) );
    connect( sText, SIGNAL(clicked()), SLOT(sendText()) );
    connect( sPalette, SIGNAL(clicked()), SLOT(sendPalette()) );
}

void RemoteCtrlImpl::sendPacket( const QVariant &v )
{
    QByteArray ba;
    QDataStream varDs( ba, IO_WriteOnly );
    varDs << v;

    QDataStream ds( socket );
    ds << (Q_UINT32) ba.size();
    socket->writeBlock( ba.data(), ba.size() );
}

void RemoteCtrlImpl::sendImage()
{
    QString imageName = QFileDialog::getOpenFileName( QString::null,
	    "Images (*.png *.xpm *.jpg)", this );
    QImage image( imageName );
    if ( !image.isNull() ) {
	sendPacket( image );
    }
}

void RemoteCtrlImpl::sendText()
{
    sendPacket( textToSend->text() );
}

void RemoteCtrlImpl::sendPalette()
{
    QColor col = QColorDialog::getColor( white, this );
    if ( col.isValid() ) {
	sendPacket( QPalette(col,col) );
    }
}
