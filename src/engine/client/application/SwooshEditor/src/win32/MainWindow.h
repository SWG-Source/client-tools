// ============================================================================
//
// MainWindow.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_MainWindow_H
#define INCLUDED_MainWindow_H

#include "BaseMainWindow.h"
#include "sharedUtility/DataTable.h"

class SwooshEditorIoWin;

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
	void slotAlphaComboBoxChanged(int);
	void slotAlphaLineEditReturnPressed();
	void slotColorButtonPressed();
	void slotSelectShaderPushButton();
	void slotClearShaderPushButton();
	void slotSelectStartAppearancePushButton();
	void slotClearStartAppearancePushButton();
	void slotSelectEndAppearancePushButton();
	void slotClearEndAppearancePushButton();
	void slotSelectSoundPushButton();
	void slotClearSoundPushButton();
	void slotUpdateTimerTimeOut();
	void slotCirclingSwooshSpeedComboBoxActivated(int);
	void slotDebugAnimationComboBoxActivated(int);
	void slotOptionsDrawExtentsActionActivated();
	void slotOptionsShowQuadsActionActivated();
	void slotVolumeSliderValueChanged(int);
	void slotFullVisibilitySpeedLineEditReturnPressed();
	void slotNoVisibilitySpeedLineEditReturnPressed();
	void slotPauseAfterAnimationCheckBoxClicked();
	void slotDefaultSamplesPerSecondPushButtonPressed();

private:

	SwooshEditorIoWin *m_ioWin;
	QPoint             m_mousePosition;
	QColor             m_color;
	QFileInfo          m_fileInfo;
	QTimer *           m_updateTimer;
	DataTable          m_animationDataTable;

	void         reset();
	void         validate();
	void         writeSwooshIff(Iff &iff);
	void         save(QString const &path);
	void         loadIff(Iff &iff);
	virtual void paintEvent(QPaintEvent *paintEvent);
	void         selectAppearance(QLineEdit *lineEdit);
	void         loadAnimationDataTable();

private:

	// Disabled

	MainWindow(MainWindow const &);
	MainWindow &operator =(MainWindow const &);
};

// ============================================================================

#endif // INCLUDED_MainWindow_H
