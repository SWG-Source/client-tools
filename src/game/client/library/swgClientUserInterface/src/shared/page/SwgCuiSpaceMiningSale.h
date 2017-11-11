//======================================================================
//
// SwgCuiSpaceMiningSale.h
// copyright (c) 2005 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiSpaceMiningSale_H
#define INCLUDED_SwgCuiSpaceMiningSale_H

#include "clientUserInterface/CuiMediator.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/NetworkId.h"
#include "UIEventCallback.h"

class UIText;
class UIComposite;
class UISliderbar;
class UITable;
class UITableModelDefault;
class UIButton;
class ShipObject;

namespace MessageDispatch
{
	class Callback;
}

//======================================================================

class SwgCuiSpaceMiningSale :
public CuiMediator,
public UIEventCallback
{
public:

	typedef stdmap<std::string /*resource class*/, int /*price per unit */>::fwd StringIntMap;

	explicit SwgCuiSpaceMiningSale(UIPage & page);

	void setStation(NetworkId const & spaceStationId, std::string const & spaceStationName);

	virtual void update(float updateDeltaSeconds);

	void OnGenericSelectionChanged(UIWidget * context);
	void OnSliderbarChanged(UIWidget * context);
	void OnButtonPressed(UIWidget * context);

	void onShipCargoChanged(ShipObject & payload);

protected:
	virtual ~SwgCuiSpaceMiningSale();
	virtual void performActivate();
	virtual void performDeactivate();

private:
	SwgCuiSpaceMiningSale();
	SwgCuiSpaceMiningSale(const SwgCuiSpaceMiningSale &);
	SwgCuiSpaceMiningSale & operator=(const SwgCuiSpaceMiningSale &);

	void reset();
	void updateSellingWindow();
	void resetSellingWindow();
	void sellResources();

private:

	UIButton * m_buttonSell;
	UIText * m_textCaption;
	UISliderbar * m_sliderSelling;
	UITable * m_tableWillPurchase;
	UITable * m_tableWillNotPurchase;
	UIText * m_textInfoSelling;
	UIText * m_textPriceSelling;
	UIText * m_textPriceUnits;
	UIPage * m_pageSelling;
	UIPage * m_pageWidgets;

	NetworkId m_spaceStationId;
	std::string m_spaceStationName;
	StringIntMap * m_resourceClassPrices;

	NetworkId m_sellingResourceId;
	int m_sellingPricePerUnit;

	MessageDispatch::Callback *            m_callback;

	bool m_dirty;
};

//======================================================================

#endif
