#ifndef REMOTECTRLIMPL_H
#define REMOTECTRLIMPL_H

#include "remotectrl.h"

class QSocket;

class RemoteCtrlImpl : public RemoteCtrl
{
    Q_OBJECT

public:
    RemoteCtrlImpl( QSocket * );

private slots:
    void sendImage();
    void sendText();
    void sendPalette();

private:
    void sendPacket( const QVariant & );

    QSocket *socket;
};

#endif // REMOTECTRLIMPL_H
