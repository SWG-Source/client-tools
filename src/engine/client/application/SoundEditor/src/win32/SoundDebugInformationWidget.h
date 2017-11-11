// ============================================================================
//
// SoundDebugInformationWidget.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef SOUNDDEBUGINFORMATIONWIDGET
#define SOUNDDEBUGINFORMATIONWIDGET

#include "BaseSoundDebugInformationWidget.h"

//-----------------------------------------------------------------------------
class SoundDebugInformationWidget : public BaseSoundDebugInformationWidget
{
	Q_OBJECT

public:

	SoundDebugInformationWidget(QWidget *parent, char const *name, WFlags flags = 0);

	void append(const char *text);

public slots:

	void slotRoomTypeComboBoxActivated(int index);
	void slot3dProviderComboBoxActivated(const QString &text);

signals:

private:
};

// ============================================================================

#endif // SOUNDDEBUGINFORMATIONWIDGET