// ======================================================================
//
// GameWidget.h
// copyright(c) 2005 Sony Online Entertainment
//
// ======================================================================

#ifndef GAMEWIDGET_H
#define GAMEWIDGET_H

// ======================================================================

class QTimer;
class QPopupMenu;
class GroundScene;

// ======================================================================

#include "sharedMessageDispatch/Receiver.h" // in the precompiled header, but included by QT-generated code that doesn't use our precompiled headers
#include <qcursor.h>                        // in the precompiled header, but included by QT-generated code that doesn't use our precompiled headers
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/StlForwardDeclaration.h"

#include <qwidget.h>
#include <qdatetime.h>

//-----------------------------------------------------------------

class GameWidget : public QWidget, public MessageDispatch::Receiver
{
	Q_OBJECT; //lint !e1511 !e1516 !e19 !e1924

public:

	explicit GameWidget(QWidget *parent, const char *name, const char * configFileName, WFlags const flags = 0);
	virtual ~GameWidget();

	void receiveMessage(const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message);

signals: //implicitly public

	void gameExclusiveFocusChanged(bool);
	void framesPerSecond(int);
	void pivotDistanceChanged(float);
	void cursorWorldPositionChanged(float x, float y, float z);

public slots:

	void onGameFocusAllowedChanged(bool b);
	void runGameLoop();

protected slots:

	void grabExclusiveFocus();

	virtual void resizeEvent(QResizeEvent*);

protected:

	void focusInEvent(QFocusEvent*);
	void keyPressEvent(QKeyEvent*);
	void keyReleaseEvent(QKeyEvent*);
	void mouseMoveEvent(QMouseEvent*);
	void mousePressEvent(QMouseEvent*);
	void mouseReleaseEvent(QMouseEvent*);
//	void mouseDoubleClickEvent(QMouseEvent*);
	void wheelEvent(QWheelEvent*);

protected slots:

	void secondElapsed();
	void onPopupItemActivated(int id);

protected:

	void releaseExclusiveFocus();
	void updateSceneData();

private:
	GameWidget();
	GameWidget(const GameWidget & rhs);
	GameWidget & operator=(const GameWidget & rhs);

protected:
	int                m_frameCounter;
	QCursor            m_defaultCursor;
	QCursor            m_blankCursor;
	int                m_previousSecondframeCounter;
	bool               m_gameHasFocus;

	QPoint             m_lastMousePoint;

	QTimer*            m_timer;
	QTimer*            m_secondTimer;

	struct KeyStates;

	KeyStates*         m_keyStates;

	QPopupMenu*        m_pop;
//	QPopupMenu*        m_templateMenu;

	size_t             m_lastOID;

	QRect              m_rubberBandRect;
	bool               m_rubberBanding;

	bool               m_rubberBandOk;
	QPoint             m_mouseDownPoint;

	bool               m_flyButtonDown;

	struct PopupData
	{
		QPoint           point;
	};

	PopupData          m_popupData;

	GroundScene*       m_gs;

	bool               m_discardNextMouseRelease;
	bool               m_warpMouse;
	bool               m_autoDraggingObjects;
	unsigned int       m_autoDragObjectId;
	int                m_minFrameLength;
	QTime              m_lastFrameTimestamp;

protected:

	static GameWidget *ms_globalGameWidget;
	static void        translatePointFromGameToScreen(int &x, int &y);
};

// ======================================================================

#endif
