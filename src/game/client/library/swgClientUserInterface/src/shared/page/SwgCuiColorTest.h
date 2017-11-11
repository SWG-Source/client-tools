//======================================================================
//
// SwgCuiColorTest.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiColorTest_H
#define INCLUDED_SwgCuiColorTest_H

//======================================================================

#include "clientUserInterface/CuiMediator.h"
#include "sharedObject/Object.h"

#include "UIEventCallback.h"

class CachedNetworkId;
class ClientObject;
class CuiColorPicker;
class CustomizationVariable;
class NetworkId;
class PaletteArgb;
class UIButton;
class UIComposite;
class UIPage;
class UISliderbar;
class UIText;
class UIVolumePage;
class UIWidget;

// ======================================================================

/**
* SwgCuiColorTest
*/

template <typename T> class Watcher;
typedef Watcher<Object> ObjectWatcher;

//----------------------------------------------------------------------

class SwgCuiColorTest :
public CuiMediator,
public UIEventCallback
{
public:

	explicit                 SwgCuiColorTest   (UIPage & thePage);

	virtual void             performActivate   ();
	virtual void             performDeactivate ();

	void                     setObject         (ClientObject & obj);

	typedef void (*IteratorCallback)(const std::string &fullVariablePathName, CustomizationVariable *customizationVariable, void *context);

	static void              customizationIteratorCallback (const std::string &fullVariablePathName, CustomizationVariable *customizationVariable, void *context);

	static SwgCuiColorTest * createInto (UIPage & parent);

	virtual void             OnSliderbarChanged( UIWidget * context);

private:
	                        ~SwgCuiColorTest ();
	                         SwgCuiColorTest ();
	                         SwgCuiColorTest (const SwgCuiColorTest & rhs);
	SwgCuiColorTest &        operator=       (const SwgCuiColorTest & rhs);

private:

	void                     clear ();

	UIPage *                 m_pickerSample;
	UISliderbar *            m_sliderSample;
	UIComposite *            m_composite;
	UIText *                 m_textCaption;

	typedef stdvector<CuiColorPicker *>::fwd PickerVector;
	PickerVector *           m_pickers;

	typedef stdvector<UISliderbar *>::fwd SliderbarVector;
	SliderbarVector *        m_sliders;

	ObjectWatcher *          m_target;
};

//======================================================================

#endif
