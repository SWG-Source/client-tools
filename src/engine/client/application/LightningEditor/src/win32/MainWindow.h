// ============================================================================
//
// MainWindow.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_MainWindow_H
#define INCLUDED_MainWindow_H

#include "BaseMainWindow.h"

class LightningEditorIoWin;

//-----------------------------------------------------------------------------
class MainWindow : public BaseMainWindow
{
	Q_OBJECT

public:

	MainWindow(QWidget *parent, char const *name);
	virtual ~MainWindow();

public slots:

	void slotFileNewAction();
	void slotFileOpenAction();
	void slotFileSaveAction();
	void slotFileSaveAsAction();
	void slotFileExitAction();
	void slotValidate();
	void slotValidate(int);
	void slotLargeBoltAlphaComboBoxChanged(int);
	void slotLargeBoltAlphaLineEditReturnPressed();
	void slotLargeBoltColorButtonPressed();
	void slotLargeBoltChaosComboBoxChanged(int);
	void slotLargeBoltChaosLineEditReturnPressed();
	void slotSmallBoltAlphaComboBoxChanged(int);
	void slotSmallBoltAlphaLineEditReturnPressed();
	void slotSmallBoltColorButtonPressed();
	void slotSmallBoltChaosComboBoxChanged(int);
	void slotSmallBoltChaosLineEditReturnPressed();
	void slotSelectShaderPushButton();
	void slotClearShaderPushButton();
	void slotSelectStartAppearancePushButton();
	void slotClearStartAppearancePushButton();
	void slotSelectEndAppearancePushButton();
	void slotClearEndAppearancePushButton();
	void slotSelectSoundPushButton();
	void slotClearSoundPushButton();
	void slotLargeBoltArcLineEditReturnPressed();
	void slotLargeBoltArcComboBoxChanged(int);
	void slotLargeBoltStartUntilComboBoxActivated(int);
	void slotLargeBoltEndFromComboBoxActivated(int);
	void slotLargeBoltStartUntilLineEditReturnPressed();
	void slotLargeBoltEndFromLineEditReturnPressed();
	void slotMoveEndPointsComboBoxActivated(int);
	void slotUpdateTimerTimeOut();
	void slotSmallBoltStartUntilLineEditReturnPressed();
	void slotSmallBoltStartUntilComboBoxActivated(int);
	void slotSmallBoltEndFromLineEditReturnPressed();
	void slotSmallBoltEndFromComboBoxActivated(int);
	void slotSmallBoltArcLineEditReturnPressed();
	void slotSmallBoltArcComboBoxChanged(int);
	void slotPlayPushButtonPressed();

private:

	LightningEditorIoWin *m_ioWin;
	QPoint                m_mousePosition;
	QColor                m_largeBoltColor;
	QColor                m_smallBoltColor;
	QFileInfo             m_fileInfo;
	QTimer               *m_updateTimer;

	void         reset();
	float        getLargeBoltPercent();
	void         validate();
	void         writeLightningIff(Iff &iff);
	void         save(QString const &path);
	void         loadIff(Iff &iff);
	virtual void paintEvent(QPaintEvent *paintEvent);

private:

	// Disabled

	MainWindow(MainWindow const &);
	MainWindow &operator =(MainWindow const &);
};

// ============================================================================

#endif // INCLUDED_MainWindow_H
