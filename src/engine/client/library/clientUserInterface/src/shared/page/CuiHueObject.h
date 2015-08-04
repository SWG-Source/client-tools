//======================================================================
//
// CuiHueObject.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiHueObject_H
#define INCLUDED_CuiHueObject_H

//======================================================================

#include "UIEventCallback.h"
#include "clientUserInterface/CuiMediator.h"

class CachedNetworkId;
class NetworkId;
class PaletteArgb;
class TangibleObject;
class UIButton;
class UIDataSource;
class UILowerString;
class UIPage;
class UIText;
class UIVolumePage;
class UIWidget;

// ======================================================================

/**
* CuiHueObject
*/

class CuiHueObject :
public CuiMediator,
public UIEventCallback
{
public:

	typedef stdvector<CachedNetworkId>::fwd CachedNetworkIdVector;

	struct Properties
	{
		static const UILowerString AutoSizePaletteCells;
	};

	struct DataProperties
	{
		static const UILowerString  TargetNetworkId;
		static const UILowerString  TargetVariable;
		static const UILowerString  TargetValue;
		static const UILowerString  TargetRange;
		static const UILowerString  SelectedIndex;
	};

	enum PathFlags
	{
		PF_shared   = 0x0001,
		PF_private  = 0x0002,
		PF_any      = 0x0003
	};

	explicit                      CuiHueObject (UIPage & page);

	virtual void                  performActivate   ();
	virtual void                  performDeactivate ();

	virtual void                  OnButtonPressed              (UIWidget * context);
	virtual void                  OnVolumePageSelectionChanged (UIWidget * context);

	void                          setTarget                    (const NetworkId & id, int maxIndex1, int maxIndex2);

	int                           getValue                     ();

	void                          setLinkedObjects             (const CachedNetworkIdVector & v);
	const CachedNetworkIdVector & getLinkedObjects             () const;

	void                          updateCellSizes              ();

	void                          setText (const Unicode::String & str);

	void                          setForceColumns              (int cols);
	void                          setAutoForceColumns          (bool b);

	static CuiHueObject *         createInto (UIPage & parent);

	typedef stdmap<std::string, int>::fwd StringIntMap;
	static void                   setupPaletteColumnData (const UIDataSource & ds);

private:
	                             ~CuiHueObject ();
	                              CuiHueObject ();
	                              CuiHueObject (const CuiHueObject & rhs);
	CuiHueObject &              operator=      (const CuiHueObject & rhs);

private:

	void                     reset ();
	void                     updateValue (int index);
	void                     updateValue (TangibleObject & obj, int index, PathFlags flags = PF_any);
	void                     revert ();

	UIVolumePage *           m_volumePage;
	UIButton *               m_buttonCancel;
	UIButton *               m_buttonRevert;
//	UIPage *                 m_pageSample;

	int                      m_originalSelection;
	int                      m_range;
	const PaletteArgb *      m_palette;
	CachedNetworkId *        m_targetObject;
	std::string              m_targetVariable;

//	UIWidget *               m_sampleElement;

	CachedNetworkIdVector *  m_linkedObjects;
	bool                     m_autoSizePaletteCells;

//	UIText *                 m_text;

	int                      m_forceColumns;

	bool                     m_autoForceColumns;

	int                      m_maxIndex1;
	int                      m_maxIndex2;
};

//----------------------------------------------------------------------

inline const CuiHueObject::CachedNetworkIdVector & CuiHueObject::getLinkedObjects             () const
{
	return *NON_NULL (m_linkedObjects);
}

//======================================================================

#endif

