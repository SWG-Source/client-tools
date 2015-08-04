// ============================================================================
//
// SoundGroupTemplateWidget.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef SOUNDGROUPTEMPLATEWIDGET_H
#define SOUNDGROUPTEMPLATEWIDGET_H

#include "BaseSoundGroupTemplateWidget.h"

class SoundDebugInformationWidget;

//-----------------------------------------------------------------------------
class SoundGroupTemplateWidget : public BaseSoundGroupTemplateWidget
{
	Q_OBJECT

public:

	SoundGroupTemplateWidget(QWidget *parent, char const *name, WFlags flags = 0);

public slots:

	void closePushButtonClicked();

signals:

private:

	// Disabled

	SoundGroupTemplateWidget();
	SoundGroupTemplateWidget(const SoundGroupTemplateWidget &);
	SoundGroupTemplateWidget &operator =(const SoundGroupTemplateWidget &);
};

// ============================================================================

#endif // SOUNDGROUPTEMPLATEWIDGET_H