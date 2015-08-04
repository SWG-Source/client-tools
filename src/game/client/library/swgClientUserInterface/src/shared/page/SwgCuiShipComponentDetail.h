//======================================================================
//
// SwgCuiShipComponentDetail.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiShipComponentDetail_H
#define INCLUDED_SwgCuiShipComponentDetail_H

//======================================================================

#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Watcher.h"
#include "clientGame/ShipObject.h"

//----------------------------------------------------------------------

class CuiWidget3dObjectListViewer;
class StringId;
class UIButton;
class UIComposite;
class UIImage;
class UIText;

namespace MessageDispatch
{
	class Callback;
}

//----------------------------------------------------------------------

class SwgCuiShipComponentDetail :
public CuiMediator,
public UIEventCallback
{
public:
	explicit SwgCuiShipComponentDetail(UIPage & page);
	void setShip(ShipObject * ship);
	virtual void OnButtonPressed (UIWidget * context);
	void update (float deltaTimeSecs);

protected:
	void performActivate();
	void performDeactivate();

private:
	//disabled
	SwgCuiShipComponentDetail(SwgCuiShipComponentDetail const & rhs);
	SwgCuiShipComponentDetail & operator= (SwgCuiShipComponentDetail const & rhs);

private:
	~SwgCuiShipComponentDetail();
	void populateUI();
	void updateUI();
	void appendShipItemToText(Unicode::String & text, StringId const & preLabel, float value, StringId const & units) const;
	void updateShipSelectionVisuals() const;
	UIButton * getButtonFromSlotPage(UIPage const & slotPage) const;
	UIImage * getHitpointsImageFromSlotPage(UIPage const & slotPage) const;
	UIImage * getUnpoweredImageFromSlotPage(UIPage const & slotPage) const;
	UIImage * getDisabledImageFromSlotPage(UIPage const & slotPage) const;
	void clearUI();

private:
	MessageDispatch::Callback * m_callback;
	Watcher<ShipObject> m_ship;
	CuiWidget3dObjectListViewer * m_shipViewer;
	UIPage * m_detailsPage;
	UIPage * m_indicatorLine;
	UIComposite * m_slotsComposite;
	UIPage * m_slotSample;
	UIText * m_slotInfoText;
	UIText * m_slotName;
	UIPage * m_healthBar;
	UIPage * m_healthBarHolster;
	UIText * m_shipInfoText;
	std::string m_selectedSlotName;
	UIText * m_damageText;
};

//======================================================================

#endif
