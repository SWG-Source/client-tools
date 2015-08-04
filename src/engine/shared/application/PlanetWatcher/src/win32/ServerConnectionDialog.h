// ============================================================================
//
// ServerConnectionDialog.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_ServerConnectionDialog_H
#define INCLUDED_ServerConnectionDialog_H

#include "BaseServerConnectionDialog.h"

//-----------------------------------------------------------------------------
class ServerConnectionDialog : public BaseServerConnectionDialog
{
	Q_OBJECT

public:

	ServerConnectionDialog(QWidget *parent, char const *name);

	QString const &getServer() const;
	QString const &getPort() const;

protected:

	virtual void keyPressEvent(QKeyEvent *keyEvent);

private slots:

	void slotAcceptPushButtonClicked();
	void onCustomServerRadioButtonClicked();
	void onQaServerRadioButtonClicked();
	void onBetaServerRadioButtonClicked();

	QString m_server;
	QString m_port;
};

// ============================================================================

#endif // INCLUDED_ServerConnectionDialog_H
