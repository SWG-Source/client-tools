// ======================================================================
//
// SwgCuiTcgControl.h
// copyright (c) 2008 Sony Online Entertainment LLC
//
// ======================================================================

#ifndef INCLUDED_SwgCuiTcgControl_H
#define INCLUDED_SwgCuiTcgControl_H

#include "clientUserInterface/CuiLayer.h"
#include "UIWidget.h"

// ----------------------------------------------------------------------

namespace libEverQuestTCG
{
	class Window;
};

class Texture;
class UIImage;

// ======================================================================

class SwgCuiTcgControl : public UIWidget
{
public:
	SwgCuiTcgControl();
	virtual ~SwgCuiTcgControl();

	virtual void Render(UICanvas & canvas) const;
	virtual bool ProcessMessage(const UIMessage & msg);

	virtual UIBaseObject * Clone() const;
	virtual UIStyle * GetStyle() const;

	void setEqTcgWindow(libEverQuestTCG::Window * eqTcgWindow);
	libEverQuestTCG::Window * getEqTcgWindow() const;
	void initializeEqTcgWindow();

	void setImage(UIImage * image);
	UIImage * getImage() const;

	void alter(float deltaTime);

	virtual void OnSizeChanged(UISize const & newSize, UISize const & oldSize);
	virtual void OnLocationChanged(UIPoint const & newLocation, UIPoint const & oldLocation);
	virtual bool IsA(UITypeID const Type) const;
	virtual void SetSelected(const bool selected);

protected:

private: //disabled
	void fetchTexture();

	SwgCuiTcgControl(SwgCuiTcgControl const & rhs);
	SwgCuiTcgControl& operator= (SwgCuiTcgControl const & rhs);

private:

	libEverQuestTCG::Window * m_eqTcgWindow;
	UIImage * m_image;
	Texture * m_texture;
};

// ----------------------------------------------------------------------

inline void SwgCuiTcgControl::setEqTcgWindow(libEverQuestTCG::Window * eqTcgWindow)
{
	m_eqTcgWindow = eqTcgWindow;
}

// ----------------------------------------------------------------------

inline libEverQuestTCG::Window * SwgCuiTcgControl::getEqTcgWindow() const
{
	return m_eqTcgWindow;
}

// ----------------------------------------------------------------------

inline UIImage * SwgCuiTcgControl::getImage() const
{
	return m_image;
}


// ----------------------------------------------------------------------

inline bool SwgCuiTcgControl::IsA(const UITypeID Type) const
{
	return Type == TUIWebBrowser || UIWidget::IsA(Type);
}

// ======================================================================

#endif
