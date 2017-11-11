//======================================================================
//
// SwgCuiCraftCustomize.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiCraftCustomize_H
#define INCLUDED_SwgCuiCraftCustomize_H

//======================================================================

#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"

class CachedNetworkId;
class CuiColorPicker;
class CuiWidget3dObjectListViewer;
class SwgCuiInventoryInfo;
class TangibleObject;
class UIButton;
class UIComboBox;
class UIComposite;
class UIPage;
class UISliderbar;
class UITextbox;
template <typename T> class Watcher;

namespace MessageDispatch
{
	class Callback;
};

//----------------------------------------------------------------------

class SwgCuiCraftCustomize :
public CuiMediator,
public UIEventCallback
{
public:

	typedef stdvector<Watcher<TangibleObject> >::fwd ObjectWatcherVector;

	explicit                    SwgCuiCraftCustomize   (UIPage & page);

	void                        performActivate        ();
	void                        performDeactivate      ();

	void                        OnButtonPressed           (UIWidget * context);
	void                        OnGenericSelectionChanged (UIWidget * context);
	void                        OnSliderbarChanged        (UIWidget * context);
	void                        OnTextboxChanged          (UIWidget * context);

	void                        update                 (float);
	bool                        close                  ();

private:
	                           ~SwgCuiCraftCustomize   ();
	                            SwgCuiCraftCustomize   (const SwgCuiCraftCustomize &);
	SwgCuiCraftCustomize &      operator=              (const SwgCuiCraftCustomize &);

	void                        onComplete             (const bool &);
	void                        setupCustomizations    ();
	void                        storeCustomizationInfo (const Unicode::String & name, int limit);
	void                        onSessionEnded         (const bool &);

	UIButton *                  m_buttonNext;
	UIButton *                  m_buttonPractice;
	UIButton *                  m_buttonCancel;
	UIButton *                  m_buttonReset;

	bool                        m_transition;

	MessageDispatch::Callback * m_callback;

	UITextbox *                 m_textboxName;
	UITextbox *                 m_textboxLimit;
	bool                        m_wasPrototype;

	UIComboBox *                m_comboAppearance;
	UISliderbar *               m_sliderLimit;

	UIComposite *               m_compositeColor;

	enum 
	{
		NumColorPickers = 4
	};

	CuiColorPicker *            m_colorPicker [NumColorPickers];

	UIPage *                    m_pageCustom;

	CuiWidget3dObjectListViewer * m_viewer;

	UICheckbox *                m_checkPractice;

	ObjectWatcherVector *       m_objects;

	bool                        m_ignoreTextboxLimit;

	bool                        m_practice;

	SwgCuiInventoryInfo *       m_info;

	bool                        m_sessionEnded;
};

//======================================================================

#endif
