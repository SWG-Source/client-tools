// ============================================================================
//
// SoundDebugInformationWidget.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstSoundEditor.h"
#include "SoundDebugInformationWidget.h"
#include "SoundDebugInformationWidget.moc"

#include "clientAudio/Audio.h"
#include "SoundEditorUtility.h"

//-----------------------------------------------------------------------------
SoundDebugInformationWidget::SoundDebugInformationWidget(QWidget *parent, char const *name, WFlags flags)
 : BaseSoundDebugInformationWidget(parent, name, flags)
{
	// Get the sound providers

	std::vector<std::string> providers(Audio::get3dProviders());

	if (providers.empty())
	{
		m_3dProviderComboBox->setEnabled(false);
		m_3dProviderComboBox->insertItem("No 3d support.");
	}
	else
	{
		// Display all the supported sound providers

		std::vector<std::string>::const_iterator iterProviders = providers.begin();
		int index = 0;
		int current3dProvider = -1;

		for (; iterProviders != providers.end(); ++iterProviders)
		{
			if (Audio::getCurrent3dProvider() == *iterProviders)
			{
				current3dProvider = index;
			}

			m_3dProviderComboBox->insertItem(iterProviders->c_str());
			++index;
		}

		if (current3dProvider == -1)
		{
			m_3dProviderComboBox->insertItem("Fix Me");
		}
		else
		{
			m_3dProviderComboBox->setCurrentItem(current3dProvider);
		}
	}

	// Set the default reverb effect

	int const index = static_cast<int>(Audio::getRoomType());
	m_roomTypeComboBox->setCurrentItem(index);

	// Connections

	connect(m_roomTypeComboBox, SIGNAL(activated(int)), this, SLOT(slotRoomTypeComboBoxActivated(int)));
	connect(m_3dProviderComboBox, SIGNAL(activated(const QString &)), this, SLOT(slot3dProviderComboBoxActivated(const QString &)));
}

//-----------------------------------------------------------------------------
void SoundDebugInformationWidget::append(const char *text)
{
	m_outputTextEdit->append(text);
}

//-----------------------------------------------------------------------------
void SoundDebugInformationWidget::slotRoomTypeComboBoxActivated(int index)
{
	Audio::setRoomType(static_cast<Audio::RoomType>(index));
}

//-----------------------------------------------------------------------------
void SoundDebugInformationWidget::slot3dProviderComboBoxActivated(const QString &text)
{
	QString newProvider(text);
	QString currentProvider(Audio::getCurrent3dProvider().c_str());

	if (currentProvider != newProvider)
	{
		QString text("WARNING: Unable to set 3d sound provider: " + newProvider);
		SoundEditorUtility::report(text);
	}

	m_roomTypeComboBox->setEnabled(Audio::isRoomTypeSupported());
}

// ============================================================================