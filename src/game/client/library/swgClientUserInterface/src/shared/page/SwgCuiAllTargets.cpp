//======================================================================
//
// SwgCuiAllTargets.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiAllTargets.h"

#include "clientGame/ClientWaypointObject.h"
#include "clientGame/ClientWorld.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGame/ContainerInterface.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/GroundScene.h"
#include "clientGame/PlayerShipController.h"
#include "clientGame/WeaponObject.h"
#include "clientGraphics/Graphics.h"
#include "clientObject/GameCamera.h"
#include "clientUserInterface/ConfigClientUserInterface.h"
#include "clientUserInterface/CuiDamageManager.h"
#include "clientUserInterface/CuiGameColorManager.h"
#include "clientUserInterface/CuiIoWin.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiObjectTextManager.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiTextManager.h"
#include "clientUserInterface/CuiUtils.h"
#include "sharedCollision/BoxExtent.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/Timer.h"
#include "sharedGame/Command.h"
#include "sharedGame/CommandChecks.h"
#include "sharedGame/CommandTable.h"
#include "sharedGame/SharedBuildingObjectTemplate.h"
#include "sharedGame/SharedCreatureObjectTemplate.h"
#include "sharedGame/SharedCreatureObjectTemplate.h"
#include "sharedMath/Sphere.h"
#include "sharedMath/Volume.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/ShipDamageMessage.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/CachedNetworkId.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/ContainedByProperty.h"
#include "swgClientUserInterface/SwgCuiHud.h"
#include "swgClientUserInterface/SwgCuiHudFactory.h"
#include "swgClientUserInterface/SwgCuiReticle.h"
#include "swgClientUserInterface/SwgCuiStatusFactory.h"
#include "swgClientUserInterface/SwgCuiStatusGround.h"

#include "UIData.h"
#include "UIOpacityEffector.h"
#include "UIPage.h"
#include "UISizeEffector.h"
#include "UIText.h"
#include "UIUtils.h"

#include <algorithm>
#include <stack>

//lint -esym(754,MyRenderer::MyRenderer)

//======================================================================

namespace SwgCuiAllTargetsNamespace
{
	const float headpoint_offset = 0.0f;

	class MyRenderer : public UIPage
	{
	public:
	friend SwgCuiAllTargets;

	public:
		explicit MyRenderer (SwgCuiAllTargets & alltargets) :
		UIPage (),
		m_alltargets (alltargets),
		m_waypointArrowPages    (new std::vector<UIPage *>)

		{
			SetName ("SwgCuiAllTargetsMyRenderer");
			UIWidget::SetSize (m_alltargets.getPage ().GetSize ());
			m_alltargets.getPage().AddChild(this);
		}

		~MyRenderer()
		{
			std::for_each (m_waypointArrowPages->begin (), m_waypointArrowPages->end (), UIBaseObject::DetachFunctor (0));
			delete m_waypointArrowPages;
			m_waypointArrowPages = NULL;
		}

		virtual UIBaseObject * Clone () const { return 0; }
		virtual UIStyle * GetStyle () const { return 0; }
		void Render (UICanvas & c) const
		{
			m_alltargets.updateOnRender ();
			UIPage::Render(c);
		}

		SwgCuiAllTargets & m_alltargets; //lint !e1925 //public yes i know

	private:
		MyRenderer ();
		MyRenderer & operator= (const MyRenderer & rhs);
		MyRenderer (const MyRenderer & rhs); //lint !e754

	private:
		std::vector<UIPage *> *        m_waypointArrowPages;
	};

	struct TextOpacityZero: public std::unary_function<UISmartPointer<UIText>, bool>
	{
		bool operator() (UISmartPointer<UIText> const & t) const 
		{
			return t.pointer() && (t->GetOpacity() <= 0.0f);
		}
	};


	//--------------------------------------------------------------------
	
	typedef stdmultimap<float, SwgCuiStatusGround *>::fwd DistanceMap;
	DistanceMap s_distanceMap;

	typedef std::set<CachedNetworkId> CurrentObjectSet;
	CurrentObjectSet s_currentObjectSet;

	void addTempPage(CachedNetworkId const & id)
	{
		if (id.getObject())
		{
			IGNORE_RETURN(s_currentObjectSet.insert(id));
		}
	}

	long s_lastTextSize = -1;

	// Do not clean up pages every frame.
	Timer s_targetPageCleanupTimer(0.25f);
}

using namespace SwgCuiAllTargetsNamespace;

//----------------------------------------------------------------------

SwgCuiAllTargets::SwgCuiAllTargets(char const * const mediatorDebugName, UIPage & page) :
CuiMediator             (mediatorDebugName, page),
m_statii                (new StatusMap),
m_sampleStatus          (0),
m_sampleMinSize(),
m_sampleSizeDelta(),
m_sampleReticle         (0),
m_reticleLookAt         (0),
m_reticleCombat         (0),
m_topReticle            (0),
m_sampleArrow           (0),
m_sampleWaypointArrow   (0),
m_arrowTarget           (0),
m_arrowCombat           (0),
m_textDamageVector      (new TextVector),
m_textDamageSample      (0),
m_effectorDamageOpacity (new UIOpacityEffector),
m_effectorDamageSize    (new UISizeEffector),
m_myRenderer(0),
m_sceneType(Game::getHudSceneType()),
m_updateTimeDelta(0.0f)
{
	m_myRenderer = new MyRenderer (*this);
	m_myRenderer->Attach (0);

	getCodeDataObject (TUIPage,  m_sampleStatus,          "sampleStatus");
	getCodeDataObject (TUIPage,  m_sampleReticle,         "sampleReticle");
	getCodeDataObject (TUIPage,  m_sampleArrow,           "sampleArrow");
	getCodeDataObject (TUIPage,  m_sampleWaypointArrow,   "sampleWaypointArrow");

	getCodeDataObject (TUIText,  m_textDamageSample,      "textDamageSample");

	m_textDamageSample->SetVisible (false);
	m_textDamageSample->SetPreLocalized (true);
	m_textDamageSample->SetSize (m_textDamageSample->GetMinimumSize ());

	m_effectorDamageOpacity->SetTargetOpacity (0.0f);
	m_effectorDamageOpacity->SetOpacitySpeed  (1.0f);
	m_effectorDamageOpacity->Attach           (0);

	m_effectorDamageSize->SetTargetSize       (m_textDamageSample->GetMaximumSize ());
	m_effectorDamageSize->SetSpeed            (UISize (0, 100));
	m_effectorDamageSize->Attach              (0);

	m_sampleStatus->SetVisible        (false);
	m_sampleArrow->SetVisible         (false);
	m_sampleWaypointArrow->SetVisible (false);
	m_sampleArrow->Center             ();
	m_sampleWaypointArrow->Center     ();

	m_sampleMinSize        = m_sampleStatus->GetMinimumSize ();
	const UISize & maxSize = m_sampleStatus->GetMaximumSize ();
	m_sampleSizeDelta      = maxSize - m_sampleMinSize;

	m_sampleReticle->SetVisible (false);

	{
		UIPage * const dupe = NON_NULL (safe_cast<UIPage *>(m_sampleReticle->DuplicateObject ()));
		getPage ().AddChild (dupe);
		dupe->Link ();
		dupe->SetColor (UIColor::green);
		dupe->SetName ("lookAt");
		m_reticleLookAt = new SwgCuiReticle (*dupe);
		m_reticleLookAt->fetch ();
		m_reticleLookAt->deactivate ();
		m_topReticle = m_reticleLookAt;
	}

	{
		UIPage * const dupe = NON_NULL (safe_cast<UIPage *>(m_sampleReticle->DuplicateObject ()));
		getPage ().AddChild (dupe);
		dupe->Link ();
		dupe->SetColor (UIColor::red);
		dupe->SetName ("combat");
		m_reticleCombat = new SwgCuiReticle (*dupe);
		m_reticleCombat->fetch ();
		m_reticleCombat->deactivate ();
	}

	{
		if(ConfigClientUserInterface::getDrawTargetArrow())
		{			
			m_arrowTarget = safe_cast<UIPage *>(m_sampleArrow->DuplicateObject ());
			const UIColor & arrowColor = CuiGameColorManager::getColorForType (CuiGameColorManager::T_target);
			m_arrowTarget->SetColor (arrowColor);
			m_arrowTarget->SetName ("arrowTarget");
			const long theSize = m_arrowTarget->GetHeight ();
			m_arrowTarget->SetSize (UIPoint (theSize, theSize));
			getPage ().AddChild (m_arrowTarget);
			m_arrowTarget->Link ();
			m_arrowTarget->Center ();
			m_arrowTarget->SetVisible (true);
			m_arrowTarget->SetEnabled (false);
		}
	}

	{
		m_arrowCombat = safe_cast<UIPage *>(m_sampleArrow->DuplicateObject ());
		const UIColor & arrowColor = CuiGameColorManager::getColorForType (CuiGameColorManager::T_combatTarget);
		m_arrowCombat->SetColor (arrowColor);
		m_arrowCombat->SetName ("arrowCombat");
		const long theSize = m_arrowCombat->GetHeight () * 3 / 2;
		m_arrowCombat->SetSize (UIPoint (theSize, theSize));
		getPage ().AddChild (m_arrowCombat);
		m_arrowCombat->Link ();
		m_arrowCombat->Center ();
		m_arrowCombat->SetVisible (true);
		m_arrowCombat->SetEnabled (false);
	}

	m_sampleWaypointArrow->SetRotation    (0.0f);
	m_sampleWaypointArrow->SetMaximumSize (m_arrowCombat->GetSize () + UIPoint (8L, 8L));
	m_sampleWaypointArrow->SetSize        (m_sampleWaypointArrow->GetMaximumSize ());
	m_sampleWaypointArrow->Pack           ();
	m_sampleWaypointArrow->Center         ();
}

//----------------------------------------------------------------------

SwgCuiAllTargets::~SwgCuiAllTargets ()
{
	m_myRenderer->Detach (0);
	m_myRenderer = 0;

	delete m_statii;
	m_statii = 0;

	m_sampleReticle = 0;
	m_sampleStatus  = 0;

	getPage ().RemoveChild (&m_reticleLookAt->getPage ());
	getPage ().RemoveChild (&m_reticleCombat->getPage ());

	m_reticleLookAt->release ();
	m_reticleCombat->release ();

	m_reticleLookAt  = 0;
	m_reticleCombat  = 0;

	m_effectorDamageOpacity->Detach (0);
	m_effectorDamageSize->Detach    (0);

	delete m_textDamageVector;
	m_textDamageVector = 0;

	m_topReticle = NULL;
	m_arrowTarget = NULL;
	m_arrowCombat = NULL;
	m_effectorDamageOpacity = NULL;
	m_effectorDamageSize = NULL;
}

//----------------------------------------------------------------------
void SwgCuiAllTargets::performActivate ()
{
	setIsUpdating(true);
}

//----------------------------------------------------------------------

void SwgCuiAllTargets::performDeactivate ()
{
	removeUnusedStatusPages(true);
	setIsUpdating(false);
	m_reticleLookAt->deactivate ();
	m_reticleCombat->deactivate ();
}

//----------------------------------------------------------------------

SwgCuiStatusGround * SwgCuiAllTargets::addStatus(TangibleObject & tangible)
{
	CachedNetworkId const id(tangible);

	// don't create a status page for client-side only objects
	if (id == CachedNetworkId::cms_cachedInvalid)
		return NULL;

	StatusMap::iterator const it = m_statii->find(id);

	SwgCuiStatusGround * status = 0;

	if (it != m_statii->end())
	{
		status = (*it).second;
	}
	else
	{
		UIPage * const page = safe_cast<UIPage *>(m_sampleStatus->DuplicateObject ());
		getPage().AddChild(page);
		page->Link();

		status = new SwgCuiStatusGround(*page, SwgCuiStatusGround::ST_target);

		IGNORE_RETURN(m_statii->insert(std::make_pair(id, status)));

		status->setTarget(id);
		status->activate();
	}

	status->getPage().SetVisible(false);

	return status;
} //lint !e550 //state not accessed (for debugging)

//----------------------------------------------------------------------

void SwgCuiAllTargets::updateStatusOpacity(CachedNetworkId const & id)
{
	if (id == CachedNetworkId::cms_cachedInvalid)
		return;

	StatusMap::iterator it = m_statii->find(id);

	if (it != m_statii->end() && it->second)
	{
		SwgCuiStatusGround * status = it->second;

		for (it = m_statii->begin(); it != m_statii->end(); ++it)
		{
			if (it->second && it->second != status)
			{
				if (status->isStatusBarVisibleAndOverlapping(it->second))
				{
					status->getPage().SetOpacity(0.25f);
					break;
				}
			}
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiAllTargets::update(const Camera & camera)
{
	// Remove unused pages.
	removeUnusedStatusPages(false);

	UIPoint screenCenter;
	CuiManager::getIoWin().getScreenCenter(screenCenter);
	getPage().SetLocation(screenCenter, true);

	UIPoint screenCenterOffset;
	CuiManager::getIoWin().getScreenCenterOffset(screenCenterOffset);

	CreatureObject const * const player = Game::getPlayerCreature ();
	if (!player)
		return;

	CachedNetworkId const playerId(player->getNetworkId());
	CachedNetworkId const & lookAtTarget = player->getLookAtTarget();
	CachedNetworkId const & intendedTarget = player->getIntendedTarget();
	CachedNetworkId selectedId;

	// Add per frame objects to the distance map.
	int const drawNetworkIds = CuiPreferences::getDrawNetworkIds();
	
	// we only want to show the HAM state of what they have targeted
	addTempPage(lookAtTarget);
	if (CuiPreferences::getShowStatusOverIntendedTarget())
		addTempPage(intendedTarget);

	CachedNetworkId const & assistId = SwgCuiHud::getHudAssistId();
	if(assistId != CachedNetworkId::cms_invalid)
		addTempPage(assistId);

	// Prevent the hidden mouse cursor from selecting objects.
	if (CuiIoWin::MouseLookState_Disabled == CuiIoWin::getMouseLookState())
	{
		SwgCuiHud * const hud = SwgCuiHudFactory::findMediatorForCurrentHud();
		if (hud != NULL)
		{
			Object * selected = hud->getLastSelectedObject();
			if (selected)
			{
				selectedId = CachedNetworkId(*selected);
				addTempPage(selectedId);
			}
		}
	}
	
	// also add status widgets to things in the area if they have the appropriate options turned on
	float const statusRange = CuiPreferences::getObjectNameRange();
	if (statusRange > 0)
	{
		ClientWorld::ObjectVector objects;
		CuiObjectTextManager::getVisibleObjects(objects);
	
		for (ClientWorld::ObjectVector::const_iterator it = objects.begin (); it != objects.end (); ++it)
		{
			Object const * object = *it;

			addTempPage(CachedNetworkId(*object));
		}
	}

	long const fontSize = static_cast<long>(CuiPreferences::getObjectNameFontSizeFactor() * 4.5f + 10.0f);
	UITextStyle const * style = CuiTextManager::getTextStyle(fontSize);

	for (std::set<CachedNetworkId>::const_iterator it = s_currentObjectSet.begin (); it != s_currentObjectSet.end (); /*++it*/)
	{
		CachedNetworkId tangibleId = *it;
		
		Object * const o = tangibleId.getObject();
		if (o == NULL || o->asClientObject() == NULL || o->asClientObject()->asTangibleObject() == NULL)
		{
			s_currentObjectSet.erase(it++);
			continue;
		}

		TangibleObject * const tangible = o->asClientObject()->asTangibleObject();

		UIPoint parentHeadPt;
		if ((!tangible->asShipObject() || drawNetworkIds) && CuiObjectTextManager::getObjectHeadPoint(*tangible, camera, 0.0f, parentHeadPt))
		{
			UIPoint pt;
			SwgCuiStatusGround * const status = addStatus(*tangible);
			if (status)
			{
				Vector const & cameraPos_w = camera.getPosition_w();
				float const cameraDistance = tangible->getPosition_w().magnitudeBetween(cameraPos_w);
				float const cameraScale = clamp(0.0f, (camera.getFarPlane() - cameraDistance) / camera.getFarPlane(), 1.0f);
				float const playerDistance = tangible->getPosition_w().magnitudeBetween(player->getPosition_w());
				
				IGNORE_RETURN(s_distanceMap.insert(std::make_pair(cameraDistance, status)));


				UIPage & statusPage = status->getPage();
				{
					statusPage.PackIfDirty();
					statusPage.WrapChildren();

					if (m_sceneType == Game::ST_space) 
						parentHeadPt -= screenCenterOffset;

					parentHeadPt.x -= (statusPage.GetWidth() / 2);
					parentHeadPt.y -= static_cast<UIScalar>((static_cast<float>(status->getRenderHeight()) * cameraScale));

					statusPage.SetLocation(parentHeadPt);

					if ((tangibleId != lookAtTarget) && (tangibleId != intendedTarget) && (tangibleId != assistId) && (selectedId != tangibleId) && (playerDistance > statusRange) && !drawNetworkIds) 
					{
						float const opacity = 1.0f - clamp(0.0f, (playerDistance - statusRange) / statusRange, 1.0f);
						statusPage.SetOpacity(opacity);
					}
					else
					{
						statusPage.SetOpacity(1.0f);
					}

					statusPage.SetTextStyle(style, false);

					status->setDisplayNameOnly(false);
					status->setResizeTargetName(true);
					status->setPulseNameOnCombat(true);
					status->setLookAtTarget((lookAtTarget == tangibleId) || (tangibleId == intendedTarget) || (tangibleId == assistId));
					status->setIsHoverTarget(selectedId == tangibleId);
					
					if( !Game::isHudSceneTypeSpace() 
						&& ((tangibleId == lookAtTarget) || (tangibleId == assistId) || (tangibleId == selectedId) || (tangibleId == intendedTarget))
						&& (tangible && tangible->isAttackable()) )
					{
						status->setShowOutOfRange(!isTargetInPlayerWeaponRange(*tangible));
					}
					else
					{
						status->setShowOutOfRange(false);
					}
					
					status->showDebugInformation(drawNetworkIds > 0);
					
					if (drawNetworkIds && tangible) 
					{
						static char buffer[128];
						int64 const value = tangible->getNetworkId().getValue();
						unsigned const long procId = tangible->getAuthServerProcessId();
						
						snprintf(buffer, sizeof(buffer), "(%d) "INT64_FORMAT_SPECIFIER, procId, value);
						
						status->setDebugInformation(buffer);
					}
				}
			}
			else
			{
				s_currentObjectSet.erase(it++);
				continue;
			}
		}

		++it;
	}

	for (StatusMap::iterator itStat = m_statii->begin(); itStat != m_statii->end (); ++itStat)
	{
		CuiObjectTextManager::updateNameWidgetHeight(itStat->first, 0);
	}

	for (DistanceMap::const_iterator dit = s_distanceMap.begin (); dit != s_distanceMap.end (); ++dit)
	{
		SwgCuiStatusGround * const status = dit->second;
		
		if (status)
		{
			UIPage & statusPage = status->getPage();

			statusPage.SetVisible(true);
			
			// Depth sort.
			IGNORE_RETURN(getPage().MoveChild(&statusPage, UIBaseObject::Bottom));

			// UI z depth. (this is not implemented yet)
			statusPage.SetDepthOverride(true);
			statusPage.SetDepth(dit->first);
			
			// Update the status widget height of this object.
			UIScalar const renderHeight = status->getRenderHeight();
			CuiObjectTextManager::updateNameWidgetHeight(status->getTarget(), renderHeight);
		}
	}

#if 0
	// adjust the opacity of the status bars if they overlap
	if (CuiPreferences::getShowStatusOverIntendedTarget())
		updateStatusOpacity(intendedTarget);
#endif

	Object const * const lookAtTargetObj = lookAtTarget.getObject ();

	if (lookAtTargetObj && m_arrowTarget)
	{
		const Vector & targetPos = lookAtTargetObj->getAppearanceSphereCenter_w ();
		const Sphere s (targetPos, lookAtTargetObj->getAppearanceSphereRadius ());
		const bool isVisible = camera.testVisibility_w (s);

		if (!isVisible || m_arrowTarget->GetOpacity () > 0.0f)
		{
			float theta = 0.0f;
			if (Game::isHudSceneTypeSpace())
			{
				Vector targetPos_camera = camera.rotateTranslate_w2o (targetPos);
				targetPos_camera.normalize ();
				targetPos_camera.z = targetPos_camera.y;
				theta = targetPos_camera.theta ();
			}
			else
			{
				const float camera_angle  = camera.getObjectFrameK_w ().theta ();
				const Vector delta_vector = targetPos - camera.getPosition_w ();
				const float target_angle  = delta_vector.theta ();
				theta = target_angle - camera_angle;
			}

			m_arrowTarget->SetRotation (theta / PI_TIMES_2);
		}

		m_arrowTarget->SetEnabled (!isVisible);
	}
	else if (m_arrowTarget)
		m_arrowTarget->SetEnabled (false);

	if (!CuiPreferences::getShowWaypointArrowsOnscreen ())
	{
		if (!m_myRenderer->m_waypointArrowPages->empty ())
		{
			for (PageVector::iterator pit = m_myRenderer->m_waypointArrowPages->begin (); pit != m_myRenderer->m_waypointArrowPages->end (); ++pit)
			{
				UIPage * const page = *pit;
				NOT_NULL (page);
				m_myRenderer->RemoveChild (page);
				page->Detach (0);
			}

			m_myRenderer->m_waypointArrowPages->clear();
		}
	}

	else
	{
		const ClientWaypointObject::WaypointVector & wv = ClientWaypointObject::getActiveWaypoints   ();
		const size_t numWaypoints = wv.size ();
		size_t       numPages     = m_myRenderer->m_waypointArrowPages->size ();

		if (numWaypoints > numPages)
		{
			m_myRenderer->m_waypointArrowPages->resize (numWaypoints);
			PageVector::iterator init_it = m_myRenderer->m_waypointArrowPages->begin ();
			std::advance (init_it, numPages);
			for(PageVector::iterator pit = init_it; pit != m_myRenderer->m_waypointArrowPages->end (); ++pit)
			{
				*pit = 0;
			}
			numPages     = m_myRenderer->m_waypointArrowPages->size ();
			DEBUG_FATAL (numPages != numWaypoints, (""));
		}
		else if (numWaypoints < numPages)
		{
			PageVector::iterator cut_it = m_myRenderer->m_waypointArrowPages->begin ();
			std::advance (cut_it, numWaypoints);

			for (PageVector::iterator pit = cut_it; pit != m_myRenderer->m_waypointArrowPages->end (); ++pit)
			{
				UIPage * const page = *pit;
				NOT_NULL (page);
				m_myRenderer->RemoveChild (page);
				page->Detach (0);
			}

			IGNORE_RETURN(m_myRenderer->m_waypointArrowPages->erase (cut_it, m_myRenderer->m_waypointArrowPages->end ()));
		}

		for (size_t i = 0; i < numWaypoints; ++i)
		{
			const ClientWaypointObject * const waypoint = wv [i];
			NOT_NULL (waypoint);
			const ClientWaypointObject * const waypoint_world = waypoint ? waypoint->getWorldObject () : NULL;
			if (waypoint) //lint !e774 //not always true
			{
				UIPage * & page           = (*m_myRenderer->m_waypointArrowPages) [i];

				if (!page)
				{
					page = safe_cast<UIPage *>(m_sampleWaypointArrow->DuplicateObject ());
					NOT_NULL (page);
					page->Attach (0);
					m_myRenderer->AddChild (page);
					page->Link ();
					page->SetName ("waypoint");
					page->Center ();
					page->SetVisible (false);
				}

				if (!waypoint_world)
				{
					page->SetVisible (false);
					continue;
				}

				const Appearance * const app = waypoint_world->getAppearance ();
				const Vector & waypointPos_w = waypoint->getWorldWaypointLocation_w();
				Vector targetPos_camera (camera.rotateTranslate_w2o (waypointPos_w));
				bool isVisible=isWaypointVisible(targetPos_camera, camera, app);

				const VectorArgb & colorArgb = waypoint->getColorArgb ();
				page->SetColor  (CuiUtils::convertColor (colorArgb));

				if (!isVisible || page->GetOpacity () > 0.0f)
				{
					float theta = 0.0f;
					if (Game::isHudSceneTypeSpace())
					{
						targetPos_camera.normalize ();
						targetPos_camera.z = targetPos_camera.y;
						theta = targetPos_camera.theta ();
					}
					else
					{
						const float camera_angle  = camera.getObjectFrameK_w ().theta ();
						const Vector delta_vector = waypointPos_w - camera.getPosition_w ();
						const float target_angle  = delta_vector.theta ();
						theta = target_angle - camera_angle;
					}

					page->SetRotation (theta / PI_TIMES_2);
				}

				page->SetVisible (true);
				page->SetEnabled (!isVisible);
			}
		}
	}

	updateDamageText();
}

//----------------------------------------------------------------------

void SwgCuiAllTargets::update (float const deltaTimeSecs)
{
	m_updateTimeDelta = deltaTimeSecs;

	// Update all info.
	CuiMediator::update(deltaTimeSecs);
}

//----------------------------------------------------------------------

void SwgCuiAllTargets::updateOnRender ()
{
	GroundScene * const gs = dynamic_cast<GroundScene *>(Game::getScene());
	const Camera * const camera = gs ? NON_NULL(gs->getCurrentCamera()) : 0;
	if (camera != NULL)
	{
		update(*camera);

		IGNORE_RETURN(getPage().MoveChild(m_myRenderer, UIBaseObject::Bottom));
	}
}

//----------------------------------------------------------------------

void SwgCuiAllTargets::raiseReticle (SwgCuiReticle & reticle)
{
	if (m_topReticle != &reticle)
	{
		IGNORE_RETURN(getPage ().MoveChild (&reticle.getPage (), UIBaseObject::Up));

		m_topReticle = &reticle;
	}
}

//----------------------------------------------------------------------

void SwgCuiAllTargets::updateDamageText()
{
	IGNORE_RETURN(m_textDamageVector->erase(std::remove_if(m_textDamageVector->begin(), m_textDamageVector->end(), TextOpacityZero()), m_textDamageVector->end()));
}

//----------------------------------------------------------------------

void SwgCuiAllTargets::displayDamageText (const UIPoint & pt, bool leftRight, int h, int a, int m)
{
	VectorArgb color;
	CuiDamageManager::findHamColor (color, h, a, m);
	UISmartPointer<UIText> text(UI_ASOBJECT(UIText, m_textDamageSample->DuplicateObject()));
	Unicode::String str;
	UIUtils::FormatInteger (str, h + a + m);

	text->SetTextColor    (CuiUtils::convertColor (color));
	text->SetLocalText    (str);
	text->SetVisible      (true);

	UIPoint actualPt = pt;

	const UISize & pageSize = getPage ().GetSize ();
	const UISize & textSize = text->GetSize ();

	if (!leftRight)
		actualPt.x -= textSize.x;

	if (actualPt.x > pageSize.x + textSize.x)
		actualPt.x = pageSize.x - textSize.x;
	if (actualPt.y > pageSize.y + textSize.y)
		actualPt.y = pageSize.y - textSize.y;

	actualPt.x = std::max (0L, actualPt.x);
	actualPt.y = std::max (0L, actualPt.y);

	text->SetLocation     (actualPt);

	text->ExecuteEffector (*m_effectorDamageSize);
	text->ExecuteEffector (*m_effectorDamageOpacity);

	getPage().AddChild(text);
	IGNORE_RETURN(getPage().MoveChild(text, UIBaseObject::Top));
	text->Link();

	m_textDamageVector->push_back(text);
}

//----------------------------------------------------------------------

void SwgCuiAllTargets::removeUnusedStatusPages(bool const forceUpdate)
{
	if (forceUpdate) 
	{
		for (StatusMap::iterator it = m_statii->begin(); it != m_statii->end (); ++it)
		{
			SwgCuiStatusGround * const status = it->second;
			if (status)
			{
				status->getPage().RemoveFromParent();
				status->getPage().SetVisible(false);
				status->deactivate();
			}
		}

		m_statii->clear();
	}
	else
	{
		if (s_targetPageCleanupTimer.updateZero(m_updateTimeDelta)) 
		{
			for (StatusMap::iterator it = m_statii->begin(); it != m_statii->end (); /*++it*/)
			{
				SwgCuiStatusGround * const status = it->second;
				if (!status || (s_currentObjectSet.find(it->first) == s_currentObjectSet.end()) || !CuiObjectTextManager::canSee(it->first.getObject())) 
				{					
					if (status)
					{
						status->getPage().RemoveFromParent();
						status->getPage().SetVisible(false);
						status->deactivate();
					}
					
					m_statii->erase(it++);
				}
				else
				{
					if (status)
					{
						status->getPage().SetVisible(false);
					}

					it++;
				}
			}
		}
		else
		{
			// Just hide for now.
			for (StatusMap::iterator it = m_statii->begin(); it != m_statii->end (); ++it)
			{
				SwgCuiStatusGround * const status = it->second;
				if (status)
				{
					status->getPage().SetVisible(false);
				}
			}
		}
	}

	s_currentObjectSet.clear();
	s_distanceMap.clear();
}

//----------------------------------------------------------------------

bool SwgCuiAllTargets::isTargetInPlayerWeaponRange(Object const & target)
{
	bool result = false;

	CreatureObject const * const player = Game::getPlayerCreature();
	WeaponObject const * const weapon = (player != NULL) ? player->getCurrentWeapon() : NULL;

	if (   (weapon != NULL)
		&& (player != NULL))
	{
		float const minRange = weapon->getMinRange();
		float const maxRange = std::max(weapon->getMaxRange(), 7.0f); // 7.0f is magic! <- I did not type this. MSB.
		float const distance = player->getDistanceBetweenCollisionSpheres_w(target);

		if(player->getPrimaryActionOverridden())
		{
			std::string const & commandName = player->getCurrentPrimaryActionName();
			Command const & command = CommandTable::getCommand(Crc::normalizeAndCalculate(commandName.c_str()));

			float commandMinRange = -1.0f;
			float commandMaxRange = -1.0f;
			CommandChecks::getRangeForCommand(&command, minRange, maxRange, false, commandMinRange, commandMaxRange);

			result = ((commandMinRange <= 0.0f || distance >= commandMinRange)
					&& (commandMaxRange <= 0.0f || distance <= commandMaxRange));
		}
		else
		{
			result = (distance >= minRange) && (distance <= maxRange);
		}
	}

	return result;
}

//======================================================================
