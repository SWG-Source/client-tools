#ifndef STARTUP_H
#define STARTUP_H

#include <qobject.h>

class QSocket;
class RemoteCtrlImpl;
class MainDialog;

class StartUp : public QObject
{
    Q_OBJECT

public:
    StartUp();
    ~StartUp();

private slots:
    void startRemoteCtrl();
    void startMainDialog();

private:
    QSocket *socket;
    RemoteCtrlImpl *remoteCtrl;
    MainDialog *mainDialog;
};

#endif // STARTUP_H
