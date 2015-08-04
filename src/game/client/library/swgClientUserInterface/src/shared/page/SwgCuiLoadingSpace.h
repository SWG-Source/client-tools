//======================================================================
//
// SwgCuiLoadingSpace.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiLoadingSpace_H
#define INCLUDED_SwgCuiLoadingSpace_H

//======================================================================

#include "UIEventCallback.h"
#include "clientUserInterface/CuiMediator.h"
#include "sharedMessageDispatch/Receiver.h"
#include "UINotification.h"

class Object;
class StringId;
class UIButton;
class UIImage;
class UIPie;
class UIWidget;
class UIText;

//===================================================================

class SwgCuiLoadingSpace : 
public CuiMediator,
public UIEventCallback,
public UINotification
{
public:
	explicit       SwgCuiLoadingSpace     (UIPage & page);
	virtual void   performActivate        ();
	virtual void   performDeactivate      ();
	virtual void   Notify                 (UINotificationServer *, UIBaseObject *, Code);
	virtual void   OnButtonPressed        (UIWidget * context);
	void           setupPage              (std::string const & planetName, StringId const & textId, StringId const & titleId, std::string const & picture);

private:
	//disabled
	~SwgCuiLoadingSpace                   ();
	SwgCuiLoadingSpace                    (SwgCuiLoadingSpace const &);
	SwgCuiLoadingSpace& operator=         (SwgCuiLoadingSpace const &);

private:
	void           update                 ();
	void setupBackground(std::string const & planetName);

private:
	UIText *       m_textScreenshotName;
	UIText *       m_text;
	UIPie *        m_pie;
	UIButton *     m_backButton;
	UIButton *     m_escButton;
	UIImage *      m_image;
	UIText *       m_textLoad;
	UIPage *       m_defaultBgPage;
};

//======================================================================

#endif
