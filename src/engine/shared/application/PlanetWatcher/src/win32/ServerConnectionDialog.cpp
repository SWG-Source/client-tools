// ============================================================================
//
// ServerConnectionDialog.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstPlanetWatcher.h"
#include "ServerConnectionDialog.h"
#include "ServerConnectionDialog.moc"

#include "PlanetWatcherUtility.h"

///////////////////////////////////////////////////////////////////////////////
//
// ServerConnectionDialogNamespace
//
///////////////////////////////////////////////////////////////////////////////

namespace ServerConnectionDialogNamespace
{
	QString const s_qaServer("swo-dev17");
	QString const s_qaPort("60002");
	QString const s_betaServer("sdt-01-01.starwarsgalaxies.net");
	QString const s_betaPort("60002");
};

using namespace ServerConnectionDialogNamespace;

///////////////////////////////////////////////////////////////////////////////
//
// ServerConnectionDialog
//
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
ServerConnectionDialog::ServerConnectionDialog(QWidget *parent, char const *name)
 : BaseServerConnectionDialog(parent, name)
 , m_server()
 , m_port()
{
	connect(m_connectPushButton, SIGNAL(clicked()), this, SLOT(slotAcceptPushButtonClicked()));
	connect(m_cancelPushButton, SIGNAL(clicked()), this, SLOT(close()));
	connect(m_customServerRadioButton, SIGNAL(clicked()), this, SLOT(onCustomServerRadioButtonClicked()));
	connect(m_qaServerRadioButton, SIGNAL(clicked()), this, SLOT(onQaServerRadioButtonClicked()));
	connect(m_betaServerRadioButton, SIGNAL(clicked()), this, SLOT(onBetaServerRadioButtonClicked()));

	// Get the last used values

	m_server = PlanetWatcherUtility::readEntry("server", s_qaServer);
	m_port = PlanetWatcherUtility::readEntry("port", s_qaPort);

	// Set the initial values

	m_serverLineEdit->setText(m_server);
	m_portLineEdit->setText(m_port);

	// Do not allow anything but ints in the port

	QIntValidator *intValidator = new QIntValidator(this);
	m_portLineEdit->setValidator(intValidator);

	// Default the custom radio button

	m_customServerRadioButton->setChecked(true);
}

//-----------------------------------------------------------------------------
void ServerConnectionDialog::slotAcceptPushButtonClicked()
{
	// Make sure something was typed into both fields

	if (!m_serverLineEdit->text().isEmpty() && !m_portLineEdit->text().isEmpty())
	{
		// Save the last used server and port if it differs from one of the
		// selectable options.

		if ((m_serverLineEdit->text() != s_qaServer) ||
		    (m_portLineEdit->text() != s_qaPort) ||
		    (m_serverLineEdit->text() != s_betaServer) ||
		    (m_portLineEdit->text() != s_betaPort))
		{
			m_server = m_serverLineEdit->text();
			m_port = m_portLineEdit->text();

			PlanetWatcherUtility::writeEntry("server", m_server);
			PlanetWatcherUtility::writeEntry("port", m_port);
		}

		accept();
	}
}

//-----------------------------------------------------------------------------
QString const &ServerConnectionDialog::getServer() const
{
	return m_server;
}

//-----------------------------------------------------------------------------
QString const &ServerConnectionDialog::getPort() const
{
	return m_port;
}

//-----------------------------------------------------------------------------
void ServerConnectionDialog::onCustomServerRadioButtonClicked()
{
	m_serverLineEdit->setText(m_server);
	m_portLineEdit->setText(m_port);
}

//-----------------------------------------------------------------------------
void ServerConnectionDialog::onQaServerRadioButtonClicked()
{
	m_serverLineEdit->setText(s_qaServer);
	m_portLineEdit->setText(s_qaPort);
}

//-----------------------------------------------------------------------------
void ServerConnectionDialog::onBetaServerRadioButtonClicked()
{
	m_serverLineEdit->setText(s_betaServer);
	m_portLineEdit->setText(s_betaPort);
}

//-----------------------------------------------------------------------------
void ServerConnectionDialog::keyPressEvent(QKeyEvent *keyEvent)
{
	if (keyEvent != NULL)
	{
		int const key = keyEvent->key();

		if (key == Qt::Key_Return)
		{
			slotAcceptPushButtonClicked();

			keyEvent->accept();
		}
	}
}

// ============================================================================
