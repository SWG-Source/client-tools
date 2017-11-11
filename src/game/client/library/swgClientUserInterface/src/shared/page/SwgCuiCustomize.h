//======================================================================
//
// SwgCuiCustomize.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiCustomize_H
#define INCLUDED_SwgCuiCustomize_H

//======================================================================

#include "clientUserInterface/CuiMediator.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/NetworkId.h"
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

class SwgCuiCustomize :
public CuiMediator,
public UIEventCallback
{
public:

	typedef stdvector<Watcher<TangibleObject> >::fwd ObjectWatcherVector;

	explicit                    SwgCuiCustomize   (UIPage & page);

	void                        performActivate        ();
	void                        performDeactivate      ();

	void                        OnButtonPressed           (UIWidget * context);

	void                        update                 (float);

	void                        setCustomizationObject (const NetworkId &object, const std::string & customVarName1, int minVar1, int maxVar1,
														const std::string & customVarName2, int minVar2, int maxVar2, 
														const std::string & customVarName3, int minVar3, int maxVar3,
														const std::string & customVarName4, int minVar4, int maxVar4);

private:
	                           ~SwgCuiCustomize   ();
	                            SwgCuiCustomize   (const SwgCuiCustomize &);
	SwgCuiCustomize &      operator=              (const SwgCuiCustomize &);

	void                        setupCustomizations    ();
	void                        storeCustomizationInfo (const Unicode::String & name, int limit);

	UIButton *                  m_buttonOK;
	UIButton *                  m_buttonCancel;
	UIButton *                  m_buttonClose;

	MessageDispatch::Callback * m_callback;

	UIComposite *               m_compositeColor;

	enum 
	{
		NumColorPickers = 4
	};

	CuiColorPicker *            m_colorPicker [NumColorPickers];

	UIPage *                    m_pageCustom;

	CuiWidget3dObjectListViewer * m_viewer;

	ObjectWatcherVector *       m_objects;

	bool                        m_ignoreTextboxLimit;

	SwgCuiInventoryInfo *       m_info;

	bool                        m_sessionEnded;

	NetworkId                   m_currentCustomizationObject;
	std::string                 m_currentCustomVarName1;
	int							m_minVar1;
	int							m_maxVar1;
	std::string                 m_currentCustomVarName2;
	int							m_minVar2;
	int							m_maxVar2;
	std::string                 m_currentCustomVarName3;
	int							m_minVar3;
	int							m_maxVar3;
	std::string                 m_currentCustomVarName4;
	int                         m_minVar4;
	int                         m_maxVar4;

};

//======================================================================

#endif
