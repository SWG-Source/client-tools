// ============================================================================
//
// ObjectLocatorDialog.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstPlanetWatcher.h"
#include "ObjectLocatorDialog.h"
#include "ObjectLocatorDialog.moc"

#include "PlanetWatcherUtility.h"

///////////////////////////////////////////////////////////////////////////////
//
// ObjectLocatorDialog
//
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
ObjectLocatorDialog::ObjectLocatorDialog(QWidget *parent, char const *name)
 : BaseObjectLocatorDialog(parent, name)
{
	connect(m_acceptPushButton, SIGNAL(clicked()), this, SLOT(slotAcceptPushButtonClicked()));
	connect(m_cancelPushButton, SIGNAL(clicked()), this, SLOT(close()));

	m_objectIdLineEdit->setText(PlanetWatcherUtility::readEntry("findObjectId", ""));
}

//-----------------------------------------------------------------------------
void ObjectLocatorDialog::slotAcceptPushButtonClicked()
{
	if (!m_objectIdLineEdit->text().isEmpty())
	{
		PlanetWatcherUtility::writeEntry("findObjectId", m_objectIdLineEdit->text());

		accept();
	}
}

//-----------------------------------------------------------------------------
const char * ObjectLocatorDialog::getObjectId() const
{
	return m_objectIdLineEdit->text().ascii();
}

//-----------------------------------------------------------------------------
void ObjectLocatorDialog::keyPressEvent(QKeyEvent *keyEvent)
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
