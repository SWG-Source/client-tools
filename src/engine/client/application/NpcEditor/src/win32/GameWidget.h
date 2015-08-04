// ============================================================================
//
// GameWidget.h
// Copyright 2004, Sony Online Entertainment
//
// ============================================================================

#ifndef GAMEWIDGET_H
#define GAMEWIDGET_H

// ============================================================================

class QTimer;
class ClientObject;
class QPopupMenu;
class Object;
class Vector;
class GroundScene;
class QComboBox;
class CustomizationVariable;
class CuiBackdrop;
class CustomizationData;
class PaletteArgb;

#include <qcursor.h>
#include <qwidget.h>
#include <string>

// ----------------------------------------------------------------------------

class NpcCuiViewer;

class GameWidget : public QWidget
{
	Q_OBJECT  //lint !e1924 !e1511 !e1516

public:

	explicit GameWidget(QWidget *parent, const char *name, WFlags const flags = 0);
	virtual ~GameWidget();

	int  getFrameRateLimit() const;

	void setModel(const char * templateName);
	void setAppearance(const char * filename);
	void setAvatar(const char * filename);

	void testCustomization();
	void testClear();
	void addFilteredWearables(const char * filter, QComboBox * page);
	void removeSelectedWearable();

	void getIntRange(const char *currentText, int &minValue, int &maxValue);
	int getIntValue(const char *currentText);
	void setIntValue(const char *currentText, int val);

	void getWearableIntRange(const char *currentText, int &minValue, int &maxValue);
	int getWearableIntValue(const char *currentText);
	void setWearableIntValue(const char *currentText, int val);

	CustomizationData * getWearableCustomizationData();
	CustomizationData * getAvatarCustomizationData();

	NpcCuiViewer * getViewer();
	CuiBackdrop * getBackdrop();
	void setBackdrop(int backdrop);

	void setDirty(bool dirty = true);

	void setupWearableFilterPresets(QComboBox * presets) const;
	const char * getWearableFilterPreset(int i) const;

	void onGameFocusAllowedChanged(bool b);
	void fillVariableComboBox(QComboBox* comboBoxVariables);
	void fillWearableVariableComboBox(QComboBox* comboBoxVariables);
	void setHeight(float h);
	float getHeight();
	void wear(const char * mgName);

	static void callbackRandomize(const std::string &fullVariablePathName, CustomizationVariable *customizationVariable, void *context);
	static void callbackFillComboBox(const std::string &fullVariablePathName, CustomizationVariable *customizationVariable, void *context);

	bool isPaletteVariable(const char *currentText, bool wearable);
	PaletteArgb const * getPalette(const char *currentText, bool wearable);

signals: //implicitly public

	void gameExclusiveFocusChanged(bool);
	void viewerDirty();

public slots:

	void runGameLoop();

protected slots:

	void grabExclusiveFocus();

	virtual void resizeEvent(QResizeEvent*);

protected:

	void focusInEvent(QFocusEvent *);
	void keyPressEvent(QKeyEvent *);
	void keyReleaseEvent(QKeyEvent *);
	void mouseMoveEvent(QMouseEvent *);
	void mousePressEvent(QMouseEvent *);
	void mouseReleaseEvent(QMouseEvent *);
	void mouseDoubleClickEvent(QMouseEvent *);
	void wheelEvent(QWheelEvent *);
	void enterEvent(QEvent *);
	void leaveEvent(QEvent *);

private slots:

	void setFrameRateLimit(int fps);

private:

	void releaseExclusiveFocus(bool force = false);

private:

	//-- disabled
	GameWidget();
	GameWidget(const GameWidget & rhs);
	GameWidget & operator=(const GameWidget & rhs);

private:

	QCursor m_defaultCursor;
	QCursor m_blankCursor;

	bool m_gameHasFocus;

	QTimer * m_timer;

	int m_minFrameLength;

	NpcCuiViewer * m_npcViewer;
	CuiBackdrop * m_npcBackdrop;

	bool m_isDirty;

private:

	static GameWidget * ms_globalGameWidget;
	static void translatePointFromGameToScreen(int &ix, int &iy);
};

// ----------------------------------------------------------------------------

inline NpcCuiViewer * GameWidget::getViewer()
{
	return m_npcViewer;
}

// ----------------------------------------------------------------------------

inline void GameWidget::setDirty(bool dirty)
{
	m_isDirty = dirty;
}

// ----------------------------------------------------------------------------

inline CuiBackdrop * GameWidget::getBackdrop()
{
	return m_npcBackdrop;
}

// ============================================================================

#endif
