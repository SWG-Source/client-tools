// ======================================================================
//
// GameWidget.h
// copyright(c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_GameWidget_H
#define INCLUDED_GameWidget_H

// ======================================================================

class ClientObject;
class GodClientData;
class GroundScene;
class QPopupMenu;
class QTimer;
class Object;
class Vector;

struct UpdateObjects
{
};

namespace MessageDispatch
{
	class Callback;
}

// ======================================================================

//@todo get in the precompiled headers 
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/StlForwardDeclaration.h"
#include "sharedFoundationTypes/FoundationTypes.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/NetworkId.h"
#include "sharedMessageDispatch/Receiver.h"    // in the precompiled header, but included by QT-generated code that doesn't use our precompiled headers
#include "sharedMessageDispatch/Transceiver.h" // in the precompiled header, but included by QT-generated code that doesn't use our precompiled headers

#include "Unicode.h"
#include "clientGame/FormManagerClient.h"

#include <qcursor.h>                           // in the precompiled header, but included by QT-generated code that doesn't use our precompiled headers
#include <qdatetime.h>

//-----------------------------------------------------------------

/**
* GameWidget is the widget window in which the game runs.  The GameWidget sets up all
* the game and engine related classes, and starts the game.  The game is rendered to the
* GameWidget's associated Win32 HWND.  The game always receives WM_CHAR and WM_IME_CHAR
* messages through the GameWidget's Qt event handler.  Otherwise, the game receives input
* through DirectX.
*
* The GameWidget's mouse/key/wheel handlers handle godclient camera and object manipulation
* when the game is not grabbing input through DirectX.
*/

class GameWidget : public QWidget, public MessageDispatch::Receiver
{
	Q_OBJECT; //lint !e1516 !e19 !e1924 !e1762 various deficiencies in the Qt macro

public:
	explicit GameWidget(QWidget*parent=0, const char*name=0);
	virtual ~GameWidget();

	void receiveMessage   (const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message);
	int  getFrameRateLimit() const;

signals: //implicitly public
	void gameExclusiveFocusChanged (bool);
	void interiorCameraFocusChanged(bool);
	void framesPerSecond           (int);
	void pivotDistanceChanged      (float);
	void cursorWorldPositionChanged(float x, float y, float z);
	void buildoutRegionChanged     (const char *, const char *);

public slots:
	void onGameFocusAllowedChanged(bool b);
	void onInteriorCameraChanged(bool b);
	void runGameLoop              ();

protected slots:
	void grabExclusiveFocus              ();
	void contextMenuEvent                (QContextMenuEvent* e);
	void createObjectFromSelectedTemplate();
	void onCreateTemplateFromMenu        (int id);
	void onAddObjectBookmark             ();

protected:
	void dragEnterEvent       (QDragEnterEvent* event);
	void dropEvent            (QDropEvent* event);
	void focusInEvent         (QFocusEvent*);
	void keyPressEvent        (QKeyEvent*);
	void keyReleaseEvent      (QKeyEvent*);
	void mouseMoveEvent       (QMouseEvent*);
	void mousePressEvent      (QMouseEvent*);
	void mouseReleaseEvent    (QMouseEvent*);
	void mouseDoubleClickEvent(QMouseEvent*);
	void wheelEvent           (QWheelEvent*);

private slots:
	void secondElapsed       ();
	void onPopupItemActivated(int id) const;
	void onPopupMenuShow     () const;
	void onPopupMenuHide     () const;
	void onTemplateMenuShow  ();
	void setFrameRateLimit   (int fps);

private:
	void releaseExclusiveFocus          ();
	Object* createObjectFromTemplateAtPoint(const std::string & templateName, const QPoint & pt);
	Object* createObjectFromTemplate       (const std::string & templateName);
	void pullSelection                  (float dx, bool alongGround);
	void updateSceneData                ();
	NetworkId getCursorPositionIntoWorld (Vector & result/*OUT*/);

	void onEditFormData(FormManagerClient::Messages::EditFormData::Payload const & payload);

private:
	//disabled
	GameWidget(const GameWidget & rhs);
	GameWidget & operator=(const GameWidget & rhs);

private:

	struct PopupMenuIds
	{
		int gameFocus;
		int setBookmark;
		int removeBookmark;

		NetworkId objNetworkId;
	};

	PopupMenuIds       m_menuIds;

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
	QPopupMenu*        m_templateMenu;

	size_t             m_lastOID;

	QRect              m_rubberBandRect;
	bool               m_rubberBanding;

	bool               m_rubberBandOk;
	QPoint             m_mouseDownPoint;

	bool               m_flyButtonDown;

	bool               m_interiorCameraOn;

	struct PopupData
	{
		QPoint           point;
	};

	PopupData          m_popupData;

	GroundScene*       m_gs;

	bool               m_discardNextMouseRelease;

	bool               m_warpMouse;

	bool               m_autoDraggingObjects;
	const NetworkId*   m_autoDragNetworkId;

	int                m_minFrameLength;
	QTime              m_lastFrameTimestamp;

	MessageDispatch::Callback *   m_callback;

private:

	static GameWidget *ms_globalGameWidget;
	static void        translatePointFromGameToScreen(int &x, int &y);

private:
	MessageDispatch::Transceiver<const UpdateObjects &> m_updateObjects;
};

// ======================================================================

#endif
