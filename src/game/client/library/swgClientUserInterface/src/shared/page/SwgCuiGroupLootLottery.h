//===================================================================
//
// SwgCuiGroupLootLottery.h
// copyright 2005, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_SwgCuiGroupLootLottery_H
#define INCLUDED_SwgCuiGroupLootLottery_H

//===================================================================

#include "UIEventCallback.h"
#include "clientGame/ClientObject.h"
#include "clientUserInterface/CuiMediator.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Watcher.h"

//===================================================================

class SwgCuiInventoryContainer;
class SwgCuiInventoryContainerFilter;
class SwgCuiContainerProviderDefault;
class UIVolumePage;
class UIButton;

//-----------------------------------------------------------------

namespace MessageDispatch
{
	class Callback;
}

//===================================================================

class SwgCuiGroupLootLottery
: public UIEventCallback
, public CuiMediator
{
public:
	static SwgCuiGroupLootLottery * createInto(UIPage & parent, ClientObject & container);
	static bool closeForContainer(ClientObject const & container);

	void OnButtonPressed(UIWidget * context);
	virtual void update(float deltaTimeSecs);
	bool objectIsInLottery(Object const * const object) const;
	ClientObject * getContainer() const;

protected:
	virtual void performActivate();
	virtual void performDeactivate();

private:
	SwgCuiGroupLootLottery();
	SwgCuiGroupLootLottery(SwgCuiGroupLootLottery const &);
	SwgCuiGroupLootLottery(UIPage & page, ClientObject & container);
	SwgCuiGroupLootLottery & operator=(SwgCuiGroupLootLottery const &);
	virtual ~SwgCuiGroupLootLottery();

	enum ResponseMode
	{
		RM_none,
		RM_selection,
		RM_all
	};

	void sendSelectionToServer(ResponseMode const responseMode);

private:
	UIButton * m_buttonNone;
	UIButton * m_buttonSelection;
	UIButton * m_buttonAll;
	UIVolumePage * m_volumeLoot;
	UIPage * m_timerHolster;
	UIPage * m_timer;

	SwgCuiInventoryContainer * m_containerMediator;
	SwgCuiContainerProviderDefault * m_containerProvider;
	SwgCuiInventoryContainerFilter * m_containerFilter;

	float m_lifeTimeSeconds;
};

//===================================================================

#endif
