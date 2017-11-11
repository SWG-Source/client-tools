//======================================================================
//
// CuiColorPicker.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiColorPicker_H
#define INCLUDED_CuiColorPicker_H

//======================================================================

#include "UIEventCallback.h"
#include "clientUserInterface/CuiMediator.h"

class CachedNetworkId;
class CustomizationManagerPaletteColumns;
class NetworkId;
class Object;
class PaletteArgb;
class TangibleObject;
class UIButton;
class UIDataSource;
class UILowerString;
class UIPage;
class UIText;
class UIVolumePage;
class UIWidget;

template <typename T> class Watcher;

// ======================================================================

/**
* CuiColorPicker
*/

class CuiColorPicker :
public CuiMediator,
public UIEventCallback
{
public:

	typedef Watcher<TangibleObject>       ObjectWatcher;
	typedef stdvector<ObjectWatcher>::fwd ObjectWatcherVector;

	struct Properties
	{
		static const UILowerString AutoSizePaletteCells;
	};

	struct DataProperties
	{
		static const UILowerString  TargetNetworkId;
		static const UILowerString  TargetVariable;
		static const UILowerString  TargetValue;
		static const UILowerString  TargetRangeMin;
		static const UILowerString  TargetRangeMax;
		static const UILowerString  SelectedIndex;
	};

	enum PathFlags
	{
		PF_shared   = 0x0001,
		PF_private  = 0x0002,
		PF_any      = 0x0003
	};

	explicit                      CuiColorPicker (UIPage & page);

	virtual void                  performActivate   ();
	virtual void                  performDeactivate ();

	virtual void                  OnButtonPressed              (UIWidget * context);
	virtual void                  OnVolumePageSelectionChanged (UIWidget * context);

	void                          setTarget                    (const NetworkId & id, const std::string & var, int rangeMin, int rangeMax);
	void                          setPalette(PaletteArgb const * palette);
	PaletteArgb const *           getPalette() const;
	void                          setTarget                    (Object * obj, const std::string & var, int rangeMin, int rangeMax);
	void                          setIndex(int index);

	int                           getValue                     () const;
	const std::string &           getTargetVariable            () const;

	void                          setLinkedObjects             (const ObjectWatcherVector & v, bool doUpdate);
	const ObjectWatcherVector &   getLinkedObjects             () const;

	void                          updateCellSizes              ();

	void                          setText                      (const Unicode::String & str);

	void                          setForceColumns              (int cols);
	void                          setAutoForceColumns          (bool b);
	void                          setMaximumPaletteIndex       (int index);

//	typedef stdmap<std::string, int>::fwd StringIntMap;
//	static void                   setupPaletteColumnData       (const UIDataSource & ds);
	static void                   setupPaletteColumnData       (stdmap<std::string, CustomizationManagerPaletteColumns>::fwd const & data);


	bool                          checkAndResetChanged         ();
	bool                          checkAndResetUSerChanged     ();

	void                          update                       (float deltaTimeSecs);
	void                          reset                        ();

	void                          handleMediatorPropertiesChanged ();
private:
	                             ~CuiColorPicker ();
	                              CuiColorPicker ();
	                              CuiColorPicker (const CuiColorPicker & rhs);
	CuiColorPicker &              operator=      (const CuiColorPicker & rhs);

private:

	void                     updateValue     (int index);
	void                     updateValue     (TangibleObject & obj, int index, PathFlags flags = PF_any);
	void                     revert          ();
	void                     storeProperties ();

	UIVolumePage *           m_volumePage;
	UIButton *               m_buttonCancel;
	UIButton *               m_buttonRevert;
	UIButton *               m_buttonClose;
	UIPage *                 m_pageSample;

	int                      m_originalSelection;
	int                      m_rangeMin;
	int                      m_rangeMax;
	PaletteArgb const *      m_palette;
	ObjectWatcher *          m_targetObject;
	std::string              m_targetVariable;

	UIWidget *               m_sampleElement;

	ObjectWatcherVector *    m_linkedObjects;
	bool                     m_autoSizePaletteCells;

	UIText *                 m_text;

	int                      m_forceColumns;

	bool                     m_autoForceColumns;

	bool                     m_changed;
	bool                     m_userChanged;
	UISize                   m_lastSize;

	enum PaletteSource
	{
		PS_target,
		PS_palette
	};

	PaletteSource            m_paletteSource;
};

//----------------------------------------------------------------------

inline const CuiColorPicker::ObjectWatcherVector & CuiColorPicker::getLinkedObjects             () const
{
	return *NON_NULL (m_linkedObjects);
}

//----------------------------------------------------------------------

inline const std::string & CuiColorPicker::getTargetVariable            () const
{
	return m_targetVariable;
}

//----------------------------------------------------------------------

inline bool CuiColorPicker::checkAndResetChanged ()
{
	const bool val = m_changed;
	m_changed = false;
	return val;
}

//----------------------------------------------------------------------

inline bool CuiColorPicker::checkAndResetUSerChanged ()
{
	bool val = m_userChanged;
	m_userChanged = false;
	return val;
}

//======================================================================

#endif

