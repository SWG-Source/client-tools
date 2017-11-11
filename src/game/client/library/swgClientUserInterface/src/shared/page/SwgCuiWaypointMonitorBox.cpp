//======================================================================
//
// SwgCuiWaypointMonitorBox.cpp
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiWaypointMonitorBox.h"

#include "UIComposite.h"
#include "UIData.h"
#include "UIDataSource.h"
#include "UIManager.h"
#include "UIMessage.h"
#include "UIOpacityEffector.h"
#include "UIPage.h"
#include "UIPopupMenu.h"
#include "UIString.h"
#include "UIText.h"
#include "UIUtils.h"
#include "clientGame/ClientWaypointObject.h"
#include "clientGame/Game.h"
#include "clientGame/PlayerObject.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiUtils.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedObject/NetworkIdManager.h"
#include "swgClientUserInterface/SwgCuiWaypointMonitor.h"
#include <algorithm>
#include <vector>

//======================================================================

namespace
{
	const UILowerString prop_wasActive  ("WasActive");
}

//----------------------------------------------------------------------

SwgCuiWaypointMonitorBox::SwgCuiWaypointMonitorBox (UIPage & page) :
CuiMediator      ("SwgCuiWaypointMonitorBox", page),
UIEventCallback  (),
m_comp           (0),
m_sample         (0),
m_monitorVector  (new MonitorVector),
m_callback       (new MessageDispatch::Callback),
m_needsReset     (false),
m_minSize        (),
m_effectorFadeIn (0)
{
	getCodeDataObject (TUIPage,            m_comp,           "comp");
	getCodeDataObject (TUIPage,            m_sample,         "sample");
	getCodeDataObject (TUIOpacityEffector, m_effectorFadeIn, "effectorFadeIn");

	DEBUG_FATAL (m_sample->GetParent () != m_comp, ("SwgCuiWaypointMonitorBox bad sample parent"));

	m_sample->Attach     (0);
	m_comp->Clear        ();

	m_minSize = getPage ().GetMinimumSize ();

	setStickyVisible (true);
	setSettingsAutoSizeLocation (true, true);

	setState (MS_closeable);
	setState (MS_closeDeactivates);

	registerMediatorObject (getPage (), true);
	getPage ().SetContextCapable (true, true);
}

//----------------------------------------------------------------------

SwgCuiWaypointMonitorBox::~SwgCuiWaypointMonitorBox ()
{
	std::for_each (m_monitorVector->begin (), m_monitorVector->end (), ReleaserFunctor ());
	delete m_monitorVector;
	m_monitorVector = 0;

	delete m_callback;
	m_callback = 0;

	m_sample->Detach     (0);
	m_sample = 0;
}

//----------------------------------------------------------------------

void  SwgCuiWaypointMonitorBox::performActivate ()
{
	CuiPreferences::setUseWaypointMonitor (true);

	reset ();

	m_callback->connect (*this, &SwgCuiWaypointMonitorBox::onPlayerWaypointsChanged, static_cast<PlayerObject::Messages::WaypointsChanged *>(0));
	m_callback->connect (*this, &SwgCuiWaypointMonitorBox::onWaypointChanged,        static_cast<ClientWaypointObject::Messages::ActivatedChanged *>(0));

	setIsUpdating (true);
}

//----------------------------------------------------------------------

void  SwgCuiWaypointMonitorBox::performDeactivate ()
{
	for (MonitorVector::const_iterator it = m_monitorVector->begin (); it != m_monitorVector->end (); ++it)
		(*it)->deactivate ();

	m_callback->disconnect (*this, &SwgCuiWaypointMonitorBox::onPlayerWaypointsChanged, static_cast<PlayerObject::Messages::WaypointsChanged *>(0));
	m_callback->disconnect (*this, &SwgCuiWaypointMonitorBox::onWaypointChanged,        static_cast<ClientWaypointObject::Messages::ActivatedChanged *>(0));

	setIsUpdating (false);
}

//----------------------------------------------------------------------

bool SwgCuiWaypointMonitorBox::close ()
{
	CuiPreferences::setUseWaypointMonitor (false);
	return CuiMediator::close ();
}

//----------------------------------------------------------------------

void SwgCuiWaypointMonitorBox::onPlayerWaypointsChanged (const PlayerObject::Messages::WaypointsChanged::Payload & )
{
	m_needsReset = true;
}

//----------------------------------------------------------------------

void SwgCuiWaypointMonitorBox::onWaypointChanged (const ClientWaypointObject & )
{
	m_needsReset = true;
}

//----------------------------------------------------------------------

void SwgCuiWaypointMonitorBox::reset ()
{
	m_needsReset = false;
	m_comp->Clear ();
	
	const ClientWaypointObject::WaypointVector & activeWv = ClientWaypointObject::getActiveWaypoints   ();

	size_t index = 0;
	if (!activeWv.empty ())
	{
		ClientWaypointObject::ConstWaypointVector wv            = ClientWaypointObject::getRegularWaypoints ();
		const ClientWaypointObject::ConstWaypointVector & wvpoi = ClientWaypointObject::getPoiWaypoints     ();
		wv.insert(wv.end(), wvpoi.begin(), wvpoi.end());

		typedef stdmap<NetworkId, float>::fwd NetworkIdFloatMap;
		static NetworkIdFloatMap oldIdsOpacities;
		oldIdsOpacities.clear ();
		
		{
			for (MonitorVector::const_iterator it = m_monitorVector->begin (); it != m_monitorVector->end (); ++it)
			{
				SwgCuiWaypointMonitor * const mon = *it;
				oldIdsOpacities [mon->getTarget ()] = mon->getPage ().GetOpacity ();
				mon->deactivate ();
			}
		}
		
		if (m_monitorVector->capacity() < activeWv.size())
			m_monitorVector->reserve(activeWv.size());
		
		for (ClientWaypointObject::ConstWaypointVector::const_iterator it = wv.begin (); it != wv.end (); ++it)
		{
			const ClientWaypointObject * const clientWaypoint = *it;
			NOT_NULL (clientWaypoint);
			
			if (clientWaypoint && clientWaypoint->isWaypointVisible () && clientWaypoint->isWaypointActive ())
			{
				const NetworkId & waypointId = clientWaypoint->getNetworkId ();

				if (index >= m_monitorVector->size())
					m_monitorVector->resize(index + 1, 0);

				SwgCuiWaypointMonitor * mon = (*m_monitorVector) [index];
				
				if (!mon)
				{
					m_sample->SetParent (&getPage ());
					UIPage * const dupe = safe_cast<UIPage *>(m_sample->DuplicateObject ());
					NOT_NULL (dupe);
					if (dupe)
					{
						dupe->SetPropertyNarrow (UIWidget::PropertyName::PackSize, "1");
						mon = new SwgCuiWaypointMonitor (*dupe, waypointId);
						mon->fetch ();
						(*m_monitorVector) [index] = mon;
					}
					m_sample->SetParent (0);
				}
				else
					mon->setTarget (waypointId);
				
				++index;
				
				NOT_NULL (mon);
				if (!mon)
					continue;
				
				m_comp->AddChild (&mon->getPage ());
				mon->getPage ().SetWidth (m_comp->GetWidth ());
				mon->getPage ().SetLocation (UIPoint::zero);

				mon->activate ();
				
				//-- waypoint was not previously in list, make it flash (fade in will do)
				
				float opacity = 0.0f;
				const NetworkIdFloatMap::const_iterator nifit = oldIdsOpacities.find (waypointId);
				if (nifit != oldIdsOpacities.end ())
					opacity = (*nifit).second;
				
				if (opacity < 1.0f)
				{
					mon->getPage ().CancelEffector  (*m_effectorFadeIn);
					mon->getPage ().SetOpacity      (opacity);
					mon->getPage ().ExecuteEffector (*m_effectorFadeIn);
				}
			}
		}		
		m_comp->Link ();
	}

	long height = m_minSize.y;
	
	if (index)
		height += getPage ().GetSizeIncrement ().y * static_cast<long>(index - 1);

	//-- clamp waypoint monitor to no more than 3/4 of screen size
	const UIWidget * const parentWidget = getPage ().GetParentWidget ();
	if (parentWidget)
		height = std::min (height, static_cast<long>(parentWidget->GetHeight () * 3 / 4));

	const UISize maxSize (getPage ().GetMaximumSize ().x, height);
	const UISize minSize (m_minSize.x, height);
	getPage ().SetMaximumSize (maxSize);
	getPage ().SetMinimumSize (minSize);
	getPage ().SetMaximumSize (maxSize);
	getPage ().SetHeight (height);
}	

//----------------------------------------------------------------------

void SwgCuiWaypointMonitorBox::update (float)
{
	if (m_needsReset)
		reset ();
}

//----------------------------------------------------------------------

void SwgCuiWaypointMonitorBox::OnPopupMenuSelection (UIWidget * context)
{
	UIPopupMenu * const pop = safe_cast<UIPopupMenu *>(context);
	NOT_NULL (pop);
	const std::string & sel = pop->GetSelectedName ();

	const NetworkId waypointId (sel);

	ClientWaypointObject * const clientWaypoint = safe_cast<ClientWaypointObject *>(NetworkIdManager::getObjectById (waypointId));
	if (clientWaypoint)
	{
		const UIDataSource * const ds = pop->GetSelectedData ();
		if (ds)
		{
			bool wasActive = false;
			ds->GetPropertyBoolean (prop_wasActive, wasActive);
			clientWaypoint->setWaypointActive (!wasActive);
		}
	}
}

//----------------------------------------------------------------------

bool SwgCuiWaypointMonitorBox::OnMessage                (UIWidget * context, const UIMessage & msg)
{
	if (context == &getPage ())
	{
		if (msg.Type == UIMessage::ContextRequest)
		{
			UIPopupMenu * const pop = UIPopupMenu::CreatePopupMenu (getPage ());
			NOT_NULL (pop);
			if (!pop)
				return true;

			const Object * const player = Game::getPlayer ();
			if (!player)
				return true;

			const Vector & playerPos_w = player->getPosition_w ();

			NetworkId networkIdUnderMouse;
			{
				SwgCuiWaypointMonitor * monitorUnderMouse = 0;
				const UIPoint & pt = msg.MouseCoords - m_comp->GetLocation ();

				for (MonitorVector::const_iterator it = m_monitorVector->begin (); it != m_monitorVector->end (); ++it)
				{
					SwgCuiWaypointMonitor * const mon = *it;
					if (mon->getPage ().GetParent () == m_comp)
					{
						if (mon->getPage ().HitTest (pt - mon->getPage ().GetLocation ()))
						{
							monitorUnderMouse   = mon;
							networkIdUnderMouse = mon->getTarget ();
							break;
						}
					}
				}
			}

			const std::string & sceneId = Game::getSceneId ();

			int offsetIndex = 0;

			int index = 0;
			ClientWaypointObject::ConstWaypointVector wv =            ClientWaypointObject::getRegularWaypoints ();			
			const ClientWaypointObject::ConstWaypointVector & wvpoi = ClientWaypointObject::getPoiWaypoints     ();
			wv.insert(wv.end(), wvpoi.begin(), wvpoi.end());
			
			for (ClientWaypointObject::ConstWaypointVector::const_iterator it = wv.begin (); it != wv.end (); ++it)
			{
				const ClientWaypointObject * const clientWaypoint = *it;
				NOT_NULL (clientWaypoint);
				
				if (clientWaypoint && clientWaypoint->isWaypointVisible ())
				{
					if(clientWaypoint->isPoi() && !clientWaypoint->isWaypointActive ())
						continue;

					if (clientWaypoint->getPlanetName () != sceneId)
						continue;

					const bool isActive = clientWaypoint->isWaypointActive ();
					const NetworkId & waypointId = clientWaypoint->getNetworkId ();

					if (networkIdUnderMouse == waypointId)
						offsetIndex = index;
					
					++index;

					static Unicode::String label;
					label.clear ();
					
					static const Unicode::String prefix_inactive = Unicode::narrowToWide ("   \\>32f\\");
					static const Unicode::String prefix_active   = Unicode::narrowToWide ("[x]\\>32f\\");
					
					if (isActive)
						label += prefix_active;
					else
						label += prefix_inactive;
					
					static Unicode::String tmp;
					tmp.clear ();
					UIUtils::FormatColor (tmp, CuiUtils::convertColor (clientWaypoint->getColorArgb ()));

					label += tmp;
					label += clientWaypoint->getLocalizedName ();

					static char buf [64];
					static const size_t buf_size = sizeof (buf);

					snprintf (buf, buf_size, "  \\#.(%dm)", static_cast<int>(clientWaypoint->getLocation ().magnitudeBetween (playerPos_w)));
					label += Unicode::narrowToWide (buf);

					UIDataSource * const ds = pop->AddItem (waypointId.getValueString (), label);
					ds->SetPropertyBoolean (prop_wasActive, isActive);
				}
			}

			pop->SetOffsetIndex (offsetIndex);
			pop->AddCallback (this);
			pop->SetLocation (context->GetWorldLocation () + msg.MouseCoords + pop->GetOffset ());
			UIManager::gUIManager ().PushContextWidget (*pop);

			return false;
		}
	}
	return true;
}

//======================================================================
