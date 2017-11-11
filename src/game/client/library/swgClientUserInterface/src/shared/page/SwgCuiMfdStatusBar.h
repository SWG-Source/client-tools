//======================================================================
//
// SwgCuiMfdStatusBar.h
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiMfdStatusBar_H
#define INCLUDED_SwgCuiMfdStatusBar_H

//======================================================================

#include "sharedMessageDispatch/Receiver.h"
#include "clientUserInterface/CuiMediator.h"

#include "UIEventCallback.h"
#include "UILowerString.h"
#include "UITypes.h"

//----------------------------------------------------------------------

class UIPage;
class UIText;

//----------------------------------------------------------------------

class SwgCuiMfdStatusBar :
public CuiMediator,
public UIEventCallback
{
public:
	explicit            SwgCuiMfdStatusBar (UIPage & page, bool useVerboseTooltip = false);
	void                updateBar(int scaleMax, int normalMax, int currentMax, int current, bool showRecharge, int recharge, float deltaTimeSecs);

	struct Properties
	{
		static const UILowerString Orientation;
		static const UILowerString RechargeUpdateMultiplier;
		static const UILowerString CurrentUpdateMultiplier;
	};

	void                setAttributeIndex    (int attributeIndex);
	void                setUseVerboseTooltip (bool useVerboseTooltip);

protected:
	virtual void        performActivate ();
	virtual void        performDeactivate ();

private:

	enum Orientation
	{
		O_vertical,
		O_horizontal
	};

	virtual            ~SwgCuiMfdStatusBar ();
	SwgCuiMfdStatusBar ();
	SwgCuiMfdStatusBar (const SwgCuiMfdStatusBar &);
	SwgCuiMfdStatusBar & operator= (const SwgCuiMfdStatusBar &);

	long                getOrientedScalar   (const UIPoint & pt) const;

	long                getOrientedLength   (const UIWidget & widget) const;
	long                getOrientedLocation (const UIWidget & widget, long const parentLength = 0) const;

	void                setOrientedLength   (UIWidget & widget, long length) const;
	void                setOrientedLocation (UIWidget & widget, long parentLength, long length) const;

	UIText *            m_valueText;
	UIPage *            m_rechargePage;
	UIPage *            m_currentPage;
	UIPage *            m_currentMaxPage;
	int                 m_currentMax;
	int                 m_normalMax;

	Orientation         m_orientation;

	UIPoint             m_currentMargin;
	UIPoint             m_currentMaxMargin;

	UISize              m_lastSize;

	bool                m_useVerboseTooltip;
	int                 m_attributeIndex;

	UIPage *            m_currentTickPage;

	int                 m_currentDesired;
	float               m_currentInterpolated;
	float               m_currentUpdateMultiplier;

	int                 m_rechargeDesired;
	float               m_rechargeInterpolated;
	float               m_rechargeUpdateMultiplier;
};

//======================================================================

#endif
