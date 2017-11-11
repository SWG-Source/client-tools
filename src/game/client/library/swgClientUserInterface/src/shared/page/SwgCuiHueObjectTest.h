//======================================================================
//
// SwgCuiHueObjectTest.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiHueObjectTest_H
#define INCLUDED_SwgCuiHueObjectTest_H

//======================================================================

#include "clientUserInterface/CuiMediator.h"

class UIPage;
class UIVolumePage;
class UIButton;
class PaletteArgb;
class CachedNetworkId;
class UIWidget;
class CuiColorPicker;
class CustomizationVariable;
class ClientObject;
class UIComposite;
class NetworkId;

// ======================================================================

/**
* SwgCuiHueObjectTest
*/

class SwgCuiHueObjectTest :
public CuiMediator
{
public:

	explicit                 SwgCuiHueObjectTest   (UIPage & thePage);

	virtual void             performActivate   ();
	virtual void             performDeactivate ();

	void                     setObject         (ClientObject & obj);
	void                     setMax1           (int maxIndex1);
	void                     setMax2           (int maxIndex2);

	typedef void (*IteratorCallback)(const std::string &fullVariablePathName, CustomizationVariable *customizationVariable, void *context);

	static void              customizationIteratorCallback (const std::string &fullVariablePathName, CustomizationVariable *customizationVariable, void *context);

	static SwgCuiHueObjectTest * createInto (UIPage & parent);

private:
	                        ~SwgCuiHueObjectTest ();
	                         SwgCuiHueObjectTest ();
	                         SwgCuiHueObjectTest (const SwgCuiHueObjectTest & rhs);
	SwgCuiHueObjectTest &        operator=       (const SwgCuiHueObjectTest & rhs);

private:

	void                     clear ();

	UIPage *                 m_pickerSample;
	UIComposite *            m_composite;

	typedef stdvector<CuiColorPicker *>::fwd PickerVector;
	PickerVector *           m_pickers;

	NetworkId  *             m_temporaryNetworkId;
	int                      m_maxIndex1;
	int                      m_maxIndex2;
};

//======================================================================

#endif
