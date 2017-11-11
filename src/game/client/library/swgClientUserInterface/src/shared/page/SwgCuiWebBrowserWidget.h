// ======================================================================
//
// SwgCuiWebBrowserWidget.h
// copyright (c) 2008 Sony Online Entertainment LLC
//
// ======================================================================

#ifndef INCLUDED_SwgCuiWebBrowserWidget_H
#define INCLUDED_SwgCuiWebBrowserWidget_H

#include "clientGraphics/StaticShader.h"
#include "clientGraphics/Texture.h"

#include "clientUserInterface/CuiLayer_TextureCanvas.h"

#include "libMozilla/libMozilla.h"

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Timer.h"

#include "UIImage.h"
#include "UIText.h"
#include "UIWidget.h"

#include <string>

// ======================================================================
class UIMozillaCallbacks;

class SwgCuiWebBrowserWidget : public UIWidget
{
public:
	SwgCuiWebBrowserWidget();
	virtual ~SwgCuiWebBrowserWidget();

	virtual void Render(UICanvas & canvas) const;
	virtual bool ProcessMessage(const UIMessage & msg);
	
	void    alter(float deltaTime);
	void    setUIImage(UIImage* image);
	void    setUIText(UIText* text);

	virtual UIBaseObject * Clone() const;
	virtual UIStyle * GetStyle() const;

	virtual void OnSizeChanged(UISize const & newSize, UISize const & oldSize);
	virtual void SetSelected(const bool selected);
	virtual bool IsA(const UITypeID Type ) const { return Type == TUIWebBrowser || UIWidget::IsA(Type); } 

	void    debugOutput();
	void    setURL(std::string url, const char * postData = NULL, int postDataLength = 0);

	void    NavigateForward();
	void    NavigateBack();
	void    NavigateStop();
	void    RefreshPage();

	void    createMozillaWindow();

	void    setHomePage(std::string const & home);

	std::string getCurrentURL() const;

private: //disabled
	SwgCuiWebBrowserWidget(SwgCuiWebBrowserWidget const & rhs);
	SwgCuiWebBrowserWidget& operator= (SwgCuiWebBrowserWidget const & rhs);

	friend UIMozillaCallbacks;

	// Callbacks
	void					  updateURI();
	void					  updateProgress();
	void					  updateStatus();
	bool					  validateURI(const char * uri);

	libMozilla::Window*       getMozillaWindow();

	libMozilla::Window*       m_MozillaWindow;
	UIMozillaCallbacks*       m_Callbacks;
	Texture*				  m_Texture;
	CuiLayer::TextureCanvas * m_Canvas;
	UIImage*				  m_Image;
	UIText*                   m_Text;

	StaticShader*             m_Shader;

	Timer                     m_caratBlink;
	bool                      m_drawCarat;
};

inline void SwgCuiWebBrowserWidget::setUIText(UIText *text)
{
	m_Text = text;
}
// ======================================================================

#endif
