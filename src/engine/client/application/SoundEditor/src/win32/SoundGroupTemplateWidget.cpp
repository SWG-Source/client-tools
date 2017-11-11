// ============================================================================
//
// SoundGroupTemplateWidget.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstSoundEditor.h"
#include "SoundGroupTemplateWidget.h"
#include "SoundGroupTemplateWidget.moc"

//-----------------------------------------------------------------------------
SoundGroupTemplateWidget::SoundGroupTemplateWidget(QWidget *parent, char const *name, WFlags flags)
 : BaseSoundGroupTemplateWidget(parent, name, flags)
{
	connect(m_closePushButton, SIGNAL(clicked()), this, SLOT(closePushButtonClicked()));
}

//-----------------------------------------------------------------------------
void SoundGroupTemplateWidget::closePushButtonClicked()
{
	close();
}

// ============================================================================