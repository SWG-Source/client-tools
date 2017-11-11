//======================================================================
//
// SwgCuiSpaceRadarPlanar.cpp
// copyright(c) 2004 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiSpaceRadarPlanar.h"
#include "swgClientUserInterface/SwgCuiSpaceRadarPlanar_Blip.h"

#include "clientGame/BuildingObject.h"
#include "clientGame/ClientWaypointObject.h"
#include "clientGame/ClientWorld.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/GroundScene.h"
#include "clientGame/GroupObject.h"

#include "clientGraphics/Camera.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/ShaderTemplateList.h"

#include "clientUserInterface/CuiAction.h"
#include "clientUserInterface/CuiActionManager.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiSettings.h"
#include "clientUserInterface/CuiWorkspaceIcon.h"

#include "sharedFoundation/PointerDeleter.h"

#include "sharedGame/SharedBuildingObjectTemplate.h"
#include "sharedGame/SharedCreatureObjectTemplate.h"
#include "sharedGame/SharedWaypointObjectTemplate.h"

#include "clientObject/GameCamera.h"

#include "UIButton.h"
#include "UIColorEffector.h"
#include "UIData.h"
#include "UIGBuffer.h"
#include "UIWidget.h"
#include "UIImage.h"
#include "UIMessage.h"
#include "UnicodeUtils.h"

#include <algorithm>
#include <set>

//======================================================================

namespace SwgCuiSpaceRadarPlanarNamespace
{
	float const c_DecayTime = 3.0f;
}

using namespace SwgCuiSpaceRadarPlanarNamespace;

//-----------------------------------------------------------------

SwgCuiSpaceRadarPlanar::SwgCuiSpaceRadarPlanar(UIPage & page) :
CuiMediator("SwgCuiSpaceRadarPlanar", page),
UIEventCallback(),
m_transformView(),
m_viewOffset(),
m_viewScale(),
m_blipCache(new BlipCache),
m_blipMap(new BlipMap),
m_headImage(NULL),
m_tailImage(NULL),
m_gbuffer(new UIGBuffer)
{
	getCodeDataObject(TUIWidget, m_headImage, "blipHead");
	if (m_headImage)
	{
		m_headImage->SetVisible(false);
		m_headImage->SetEnabled(false);
	}

	getCodeDataObject(TUIWidget, m_tailImage, "blipTail");
	if (m_tailImage)
	{
		m_tailImage->SetVisible(false);
		m_tailImage->SetEnabled(false);
	}

	m_gbuffer->Attach(NULL);
	getPage().AddChild(m_gbuffer);
	m_gbuffer->Link();
	m_gbuffer->Center();
	m_gbuffer->SetVisible(true);
	m_gbuffer->SetEnabled(true);

	registerMediatorObject(getPage(), true);

	// Setup the rotation of the plane.
	m_transformView.yaw_l(30.f * PI / 180.f);
	m_transformView.pitch_l(30.f * PI / 180.f);
}

//----------------------------------------------------------------------

SwgCuiSpaceRadarPlanar::~SwgCuiSpaceRadarPlanar()
{
	m_gbuffer->Detach(0);
	m_gbuffer = NULL;

	//-- Return all objects to the cache.
	for (BlipMap::iterator itBlipMap = m_blipMap->begin(); itBlipMap != m_blipMap->end(); ++itBlipMap)
	{
		//-- Place the object back into the cache.
		blipRelinquish(itBlipMap->second);
	}

	delete m_blipMap;
	m_blipMap = NULL;

	std::for_each(m_blipCache->begin(), m_blipCache->end(), PointerDeleter());

	delete m_blipCache;
	m_blipCache = NULL;

}

//-----------------------------------------------------------------

void SwgCuiSpaceRadarPlanar::performActivate()
{
	setIsUpdating(true);
}

//-----------------------------------------------------------------

void SwgCuiSpaceRadarPlanar::performDeactivate()
{
	setIsUpdating(false);
}

//----------------------------------------------------------------------

void SwgCuiSpaceRadarPlanar::update(float deltaTimeSecs)
{
	CreatureObject * const player = Game::getPlayerCreature();
	if (!player)
		return;

	updateViewInfo();

	m_gbuffer->ClearBuffer();

	Vector const & pos_w = player->getPosition_w();

	Transform const & xformPlayer_o2w = player->getTransform_o2w();

	float const maxRange = Game::getRadarRange();
	ClientWorld::ObjectVector cov;
	ClientWorld::findObjectsInRange(pos_w, maxRange, cov);

	ClientWorld::ObjectVector::const_iterator end = cov.end();
	for (ClientWorld::ObjectVector::const_iterator it = cov.begin(); it != end; ++it)
	{
		const ClientObject * const obj = dynamic_cast<ClientObject*>(*it);
		if (obj && obj->isActive() && obj->getAppearance())
		{
			//don't render waypoints here
			if (obj->getObjectType() == SharedWaypointObjectTemplate::SharedWaypointObjectTemplate_tag)
				continue;

			updateBlipFor(obj, xformPlayer_o2w);
		}
	}

	decayBlip(deltaTimeSecs, pos_w);
}

//----------------------------------------------------------------------

void SwgCuiSpaceRadarPlanar::blipRelinquish(Blip *blipObject)
{
	if (blipObject)
	{
		blipObject->setVisible(false);

		if (m_blipCache->find(blipObject) != m_blipCache->end())
		{
			IGNORE_RETURN(m_blipCache->insert(blipObject));
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiSpaceRadarPlanar::blipRelinquishById(NetworkId const & objectId)
{
	BlipMap::iterator const itBlip = m_blipMap->find(objectId);
	if (itBlip != m_blipMap->end())
	{
		blipRelinquish(itBlip->second);
		m_blipMap->erase(itBlip);
	}
}

//----------------------------------------------------------------------

SwgCuiSpaceRadarPlanar::Blip * SwgCuiSpaceRadarPlanar::getBlipReference(NetworkId const & netId)
{
	Blip * blipRef = NULL;

	BlipMap::iterator const itBlip = m_blipMap->find(netId);
	if (itBlip != m_blipMap->end())
	{
		blipRef = itBlip->second;
	}
	else
	{
		blipRef = blipAcquire();
		IGNORE_RETURN(m_blipMap->insert(std::make_pair(netId, blipRef)));
	}

	return blipRef;
}


//----------------------------------------------------------------------

SwgCuiSpaceRadarPlanar::Blip * SwgCuiSpaceRadarPlanar::blipAcquire()
{
	Blip * blipObject = NULL;

	if (!m_blipCache->empty())
	{
		blipObject = *(m_blipCache->begin());
		IGNORE_RETURN(m_blipCache->erase(blipObject));
	}

	if (!blipObject)
	{
		blipObject = new Blip(*this);
		IGNORE_RETURN(m_blipCache->insert(blipObject));
	}

	return blipObject;
}

//----------------------------------------------------------------------

void SwgCuiSpaceRadarPlanar::updateBlipFor(ClientObject const * obj, Transform const & playerXform_w2p)
{
	if (obj)
	{
		Blip * const blip = getBlipReference(obj->getNetworkId());

		if (blip)
		{
			//-- Reset decay time.
			blip->m_decayTimer = c_DecayTime;

			//-- Get the blip position into player object space.
			blip->m_blipvert = obj->getPosition_w();
			Vector blipPositionObj;
			playerXform_w2p.rotateTranslate_p2l(&(blip->m_blipvert), &blipPositionObj, 1);
			float zoffset = blipPositionObj.y;

			//-- Rotate by the local tweak matrix.
			Vector blipPositionTweak;
			m_transformView.rotate_p2l(&blipPositionObj, &blipPositionTweak, 1);

			//-- Now transform it by the widget view transform.
			UIPoint blipPositionView;
			getViewOffset(blipPositionTweak, blipPositionView);

			// Render the blips.
			UIScalar CenterLineWidget = 0;
			if (blip->m_imageHead)
			{
				blip->m_imageHead->SetLocation(blipPositionView);
				m_gbuffer->AddWidget(blip->m_imageHead, zoffset);
				CenterLineWidget = blip->m_imageHead->GetWidth() / 2;
			}

			if (blip->m_imageTail)
			{
				bool isAbove = zoffset >= 0.0f;

				blipPositionObj.y = 0.0f;
				m_transformView.rotate_p2l(&blipPositionObj, &blipPositionTweak, 1);
				UIPoint blipPositionView2;
				getViewOffset(blipPositionTweak, blipPositionView2);

				blipPositionView.x += CenterLineWidget;
				blipPositionView2.x += CenterLineWidget;

				blip->m_imageTail->SetLocation(isAbove ? blipPositionView : blipPositionView2);
				blip->m_imageTail->SetHeight(abs(blipPositionView2.y - blipPositionView.y));
				blip->m_imageTail->SetWidth(1);

				m_gbuffer->AddWidget(blip->m_imageTail, zoffset);
			}
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiSpaceRadarPlanar::decayBlip(float const timeDelta, Vector const & /*playerPos*/)
{
	for (BlipMap::iterator itBlipMap = m_blipMap->begin(); itBlipMap != m_blipMap->end(); /*increment in body*/)
	{
		Blip * const blip = itBlipMap->second;

		blip->m_decayTimer -= timeDelta;

		if (blip->m_decayTimer < 0.0f )
		{
			BlipMap::iterator itBlipNext = itBlipMap;
			itBlipNext++;

			// erase and return to cache.
			blipRelinquishById(itBlipMap->first);

			itBlipMap = itBlipNext;
		}
		else
		{
			++itBlipMap;
		}
	}
}

//----------------------------------------------------------------------

UIWidget * SwgCuiSpaceRadarPlanar::duplicate(UIWidget const * source )
{
	UIWidget * image = safe_cast<UIWidget*>(source->DuplicateObject());

	getPage().AddChild(image);

	image->Attach(NULL);
	image->Link();
	image->Center();
	image->SetEnabled(true);
	image->SetVisible(false);

	return image;
}

//----------------------------------------------------------------------

UIWidget * SwgCuiSpaceRadarPlanar::newHeadImage()
{
	return safe_cast<UIWidget*>(duplicate(m_headImage));
}

//----------------------------------------------------------------------

UIWidget * SwgCuiSpaceRadarPlanar::newTailImage()
{
	return safe_cast<UIWidget*>(duplicate(m_tailImage));
}

//----------------------------------------------------------------------

void SwgCuiSpaceRadarPlanar::deleteWidget(UIWidget * widget)
{
	if (widget)
	{
		getPage().RemoveChild(widget);
		widget->SetEnabled(false);
		widget->SetVisible(false);
		widget->Detach(NULL);
	}
}

//----------------------------------------------------------------------

void SwgCuiSpaceRadarPlanar::updateViewInfo()
{
	m_viewOffset.set(1.f, 1.f, 0.0f);
	m_viewScale.set(getPage().GetWidth() * 0.5f, getPage().GetHeight() * 0.5f, 1.f);
}

//----------------------------------------------------------------------

void SwgCuiSpaceRadarPlanar::getViewOffset(Vector const &input, UIPoint& output) const
{
	Vector positionView = input / Game::getRadarRange();

	positionView = positionView + m_viewOffset;
	positionView.x = positionView.x * m_viewScale.x;
	positionView.y = positionView.y * m_viewScale.y;

	output.Set(static_cast<UIScalar>(positionView.x), static_cast<UIScalar>(2.0f * m_viewScale.y - positionView.y));
}

//======================================================================
