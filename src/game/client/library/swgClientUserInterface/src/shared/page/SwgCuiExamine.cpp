//======================================================================
//
// SwgCuiExamine.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiExamine.h"

#include "clientGame/CellObject.h"
#include "clientGame/ClientObject.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "clientGame/PlayerObject.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMenuInfoTypes.h"
#include "clientUserInterface/CuiWidget3dObjectListViewer.h"
#include "sharedFoundation/Watcher.h"
#include "sharedMath/Quaternion.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "swgClientUserInterface/SwgCuiInventoryInfo.h"

#include "UIData.h"
#include "UIManager.h"
#include "UIMessage.h"
#include "UIPage.h"
#include "UIPopupMenu.h"
#include "UIPopupMenustyle.h"

namespace SwgCuiExamineNamespace
{
	namespace PopupIds
	{
		const std::string rotate_apply	= "rotate_apply";
		const std::string rotate_reset	= "rotate_reset";
	}

	const std::string collectionName	= "force_shui_tier_02";
   	const float       pitchOffset		= 0.2f;
}

//======================================================================

SwgCuiExamine::SwgCuiExamine (UIPage & page) :
CuiMediator         ("SwgCuiExamine", page),
UIEventCallback     (),
m_info              (0),
m_callback          (new MessageDispatch::Callback),
m_watcher           (new ObjectWatcher)
{
	registerMediatorObject(getPage(), true);

	setState (MS_closeable);
	setState (MS_closeDeactivates);

	UIPage * infoPage = 0;
	getCodeDataObject (TUIPage, infoPage, "info");

	m_info      = new SwgCuiInventoryInfo (*infoPage, false, true);
	m_info->fetch ();
}

//----------------------------------------------------------------------

SwgCuiExamine::~SwgCuiExamine ()
{
	delete m_callback;
	m_callback = 0;

	m_info->release ();
	m_info = 0;

	delete m_watcher;
	m_watcher = 0;
 }

//----------------------------------------------------------------------

void SwgCuiExamine::performActivate ()
{
	m_info->activate ();
	CuiManager::requestPointer (true);
	setTarget (m_watcher->getPointer ());
	setIsUpdating (true);
	
	CuiWidget3dObjectListViewer * viewer = m_info->getViewer();
	
	if (viewer)
		viewer->setNoCameraClamp(true);
}

//----------------------------------------------------------------------

void SwgCuiExamine::performDeactivate ()
{
	m_info->deactivate ();
	CuiManager::requestPointer (false);
	setIsUpdating (false);
}

//----------------------------------------------------------------------

void SwgCuiExamine::setTarget                     (Object * obj)
{
	*m_watcher = obj;

	m_info->setInfoObject (obj);
}

//----------------------------------------------------------------------

void SwgCuiExamine::onObjectDestroyed             (const NetworkId & id)
{
	UNREF (id);
}

//----------------------------------------------------------------------

void SwgCuiExamine::update                        (float deltaTimeSecs)
{
	CuiMediator::update (deltaTimeSecs);

	if (!m_info->getInfoObject ())
	{
		closeNextFrame ();
		return;
	}

	CreatureObject const * const creatureObject = CreatureObject::asCreatureObject(m_info->getInfoObject());
	if (creatureObject && (creatureObject->getNetworkId() != Game::getPlayerNetworkId()) && !PlayerObject::isAdmin() && !creatureObject->getCoverVisibility() && !creatureObject->isPassiveRevealPlayerCharacter(Game::getPlayerNetworkId()))
		closeNextFrame();
}

//----------------------------------------------------------------------

void SwgCuiExamine::OnPopupMenuSelection (UIWidget * context)
{
	if (!context || !context->IsA(TUIPopupMenu))
		return;
	
	UIPopupMenu * const pop = safe_cast<UIPopupMenu *>(context);

	if (!pop)
		return;

	const std::string & selection = pop->GetSelectedName();

	if (selection == SwgCuiExamineNamespace::PopupIds::rotate_reset)
	{
		const Quaternion q (Quaternion::identity);
		GenericValueTypeMessage<std::pair<NetworkId, std::pair<std::pair<float, float>, std::pair<float, float> > > > const msg("RotateFurnitureSetQuaternion", std::make_pair(m_watcher->getPointer ()->getNetworkId(), std::make_pair(std::make_pair(q.w, q.x), std::make_pair(q.y, q.z))));
		GameNetwork::send(msg, true);
	}
	else if (selection == SwgCuiExamineNamespace::PopupIds::rotate_apply)
	{
		applyRotationToServer();
	}
 }

//----------------------------------------------------------------------

bool SwgCuiExamine::OnMessage (UIWidget *context, const UIMessage & msg)
{
	if (context)
	{
		if ( msg.Type == UIMessage::RightMouseUp )
		{
			if (!canBeRotated())
			{
				return true;
			}
						
			UIPopupMenu * const pop = new UIPopupMenu(&getPage()); 
			pop->SetStyle(getPage().FindPopupStyle());

			pop->AddItem(SwgCuiExamineNamespace::PopupIds::rotate_apply, Cui::MenuInfoTypes::getLocalizedLabel(Cui::MenuInfoTypes::ROTATE_APPLY, 0));
			pop->AddItem(SwgCuiExamineNamespace::PopupIds::rotate_reset, Cui::MenuInfoTypes::getLocalizedLabel(Cui::MenuInfoTypes::ROTATE_RESET, 0));
						
			pop->SetLocation(context->GetWorldLocation() + msg.MouseCoords);
			UIManager::gUIManager().PushContextWidget(*pop);
			pop->AddCallback(this);

			return false;
		}
	}
	return true;
}

//----------------------------------------------------------------------

bool SwgCuiExamine::canBeRotated  (void)
{
	PlayerObject * const player = Game::getPlayerObject();

	if (!player)
		return false;

	const Object * object = m_watcher->getPointer();

	// If the  object is not a playerobject and is contained within a cell, 
	// and is not a gold item or buildout, and the collection is done, and player is NOT admin
	if (object)
	{
		if (object->asClientObject() && object->asClientObject()->asCreatureObject() && object->asClientObject()->asCreatureObject()->getPlayerObject())
		{
			return false;
		}

		const ClientObject * containedBy = object->asClientObject()->getContainedBy();

		if (containedBy && containedBy->asCellObject() && (containedBy->getNetworkId().getValue() > 10000000))
		{
			if (player->hasCompletedCollection(SwgCuiExamineNamespace::collectionName) && !player->isAdmin())
				return true;
		}
	}
	return false;
}

//----------------------------------------------------------------------

void SwgCuiExamine::applyRotationToServer         (void)
{
	const CuiWidget3dObjectListViewer * viewer = m_info->getViewer();

	if (!viewer)
		return;

	Transform t;
		
	t.pitch_l(-(viewer->getCameraPitch() - SwgCuiExamineNamespace::pitchOffset));
	t.yaw_l(-viewer->getCameraYaw());

	const Quaternion q(t);

	GenericValueTypeMessage<std::pair<NetworkId, std::pair<std::pair<float, float>, std::pair<float, float> > > > const msg("RotateFurnitureSetQuaternion", std::make_pair(m_watcher->getPointer ()->getNetworkId(), std::make_pair(std::make_pair(q.w, q.x), std::make_pair(q.y, q.z))));
	GameNetwork::send(msg, true);

}

//======================================================================