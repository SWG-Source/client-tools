#ifndef IPCSERVER_H
#define IPCSERVER_H

#include <qserversocket.h>

class IpcServer : public QServerSocket
{
    Q_OBJECT

public:
    IpcServer( Q_UINT16 port, QObject *parent );

    void newConnection( int socket );

signals:
    void receivedText( const QString& );
    void receivedPixmap( const QPixmap& );
};

#endif // IPCSERVER_H
