//======================================================================
//
// CuiFontSizer.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiFontSizer_H
#define INCLUDED_CuiFontSizer_H

//======================================================================

#include "UIEventCallback.h"
#include "clientUserInterface/CuiMediator.h"

class UIButton;
class UIPage;
class UIText;
class UIWidget;
class UISliderbar;
class UITextStyle;
class UILowerString;

// ======================================================================

/**
* CuiFontSizer
* Simple font sizer until we get a real ttf font system
*/

class CuiFontSizer :
public CuiMediator,
public UIEventCallback
{
public:

	struct Properties
	{
		static const UILowerString FontSize;
		static const UILowerString FontIndex;
	};

	typedef stdvector<int>::fwd SizeVector;

	explicit                      CuiFontSizer         (UIPage & page);

	virtual void                  performActivate      ();
	virtual void                  performDeactivate    ();

	virtual void                  OnButtonPressed      (UIWidget * context);
	virtual void                  OnSliderbarChanged   (UIWidget * context);

	void                          releaseTarget        ();
	void                          setTargetText        (UIText & text, const SizeVector & sv);

	static CuiFontSizer *         createInto           (UIPage & parent);

	void                          updateSample         ();

	static UITextStyle *          getTextStyle         (UIText & text, const SizeVector & sv, const std::string & fontPrefix, int index, int & size);
	static bool                   constructFontPrefix  (const UIText & text, const SizeVector & sv, std::string & prefix, int & size);

private:
	                             ~CuiFontSizer ();
	                              CuiFontSizer ();
	                              CuiFontSizer (const CuiFontSizer & rhs);
	CuiFontSizer &                operator=      (const CuiFontSizer & rhs);

private:

	void                          ok                   ();

	UIButton *                    m_buttonOk;
	UIButton *                    m_buttonCancel;

	UISliderbar *                 m_slider;
	UIText *                      m_textSample;

	UIText *                      m_targetText;

	SizeVector *                  m_sizeVector;

	std::string                   m_fontPrefix;

	int                           m_index;
	int                           m_size;
};

//======================================================================

#endif

