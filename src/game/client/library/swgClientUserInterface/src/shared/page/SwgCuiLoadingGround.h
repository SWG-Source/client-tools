//======================================================================
//
// SwgCuiLoadingGround.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiLoadingGround_H
#define INCLUDED_SwgCuiLoadingGround_H

//======================================================================

#include "UIEventCallback.h"
#include "clientUserInterface/CuiMediator.h"
#include "sharedMessageDispatch/Receiver.h"
#include "UINotification.h"

class Object;
class UICheckbox;
class UIImage;
class UIWidget;
class UIText;
class UIButton;
class StringId;

//===================================================================

class SwgCuiLoadingGround : 
public CuiMediator,
public UIEventCallback,
public UINotification,
public MessageDispatch::Receiver
{
public:

	explicit       SwgCuiLoadingGround (UIPage & page);

	void           performActivate        ();
	void           performDeactivate      ();
	void           Notify                 (UINotificationServer *, UIBaseObject *, Code);
	void           OnButtonPressed        (UIWidget *context);
	void           setupPage              (const std::string& planetName, const StringId& textId, const StringId& titleId, const std::string& picture);

	virtual void receiveMessage(const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message);

private:

	              ~SwgCuiLoadingGround ();
	               SwgCuiLoadingGround  (const SwgCuiLoadingGround&);
	SwgCuiLoadingGround& operator=      (const SwgCuiLoadingGround&);

	void           update          ();
	void           clearAllLoadingChecks();

private:

	UIWidget *     m_bar;
	UIText *       m_text;
	UIText *       m_textPlanetName;
	UIText *       m_textScreenshotName;
	UIText *       m_textPercentScroll;
	UIImage *      m_flagImage;
	UIImage *      m_planetImage;
	UIImage *      m_genericImage;
	UIImage *      m_tipImage;
	int            m_idleValue;
	bool           m_wasIdling;
	bool           m_idleUp;
	UIButton *     m_buttonBack;
	UIButton *     m_buttonEsc;
	UIText *       m_textLoad;
	UICheckbox *   m_checkServerObjects;
	UICheckbox *   m_checkFileCaching;
	UICheckbox *   m_checkWorldLoaded;
	UICheckbox *   m_checkTerrainGenerated;
	UICheckbox *   m_checkPlayerReady;
};

//======================================================================

#endif
