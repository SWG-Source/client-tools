#include "ipcserver.h"

#include <qsocket.h>
#include <qvariant.h>
#include <qimage.h>
#include <qpalette.h>
#include <qapplication.h>

class IpcSocket : public QSocket
{
    Q_OBJECT

public:
    IpcSocket( QObject *parent) : QSocket( parent )
    {
	packetSize = 0;
	connect( this, SIGNAL(readyRead()), SLOT(read()) );
    }

signals:
    void receivedText( const QString& );
    void receivedPixmap( const QPixmap& );

private slots:
    void read()
    {
	Q_ULONG bytesAvail = bytesAvailable();
	for ( ;; ) {
	    if ( packetSize == 0 ) {
		QDataStream ds( this );
		if ( bytesAvail < 4 )
		    return;
		ds >> packetSize;
		bytesAvail -= 4;
	    } else {
		if ( bytesAvail < packetSize )
		    return;
		// read the packet in a byte array to be sure that you don't
		// read too much or too less
		QByteArray ba( packetSize );
		readBlock( ba.data(), packetSize );
		bytesAvail -= packetSize;
		packetSize = 0;

		QVariant variant;
		QDataStream ds( ba, IO_ReadOnly );
		ds >> variant;
		switch ( variant.type() ) {
		    case QVariant::String:
			emit receivedText( variant.toString() );
			break;
		    case QVariant::Image:
			emit receivedPixmap( QPixmap(variant.toImage()) );
			break;
		    case QVariant::Palette:
			QApplication::setPalette( variant.toPalette(), TRUE );
			break;
		    default:
			break;
		}
	    }
	}
    }

private:
    Q_UINT32 packetSize;
};

IpcServer::IpcServer( Q_UINT16 port, QObject *parent ) :
    QServerSocket( 0x7f000001, port, 1, parent )
{
}

void IpcServer::newConnection( int socket )
{
    IpcSocket *s = new IpcSocket( this );
    s->setSocket( socket );
    connect( s, SIGNAL(receivedText(const QString&)),
	    SIGNAL(receivedText(const QString&)) );
    connect( s, SIGNAL(receivedPixmap(const QPixmap&)),
	    SIGNAL(receivedPixmap(const QPixmap&)) );
}

#include "ipcserver.moc"
