// ======================================================================
//
// SpaceTargetBracketOverlay.cpp
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/SpaceTargetBracketOverlay.h"

#include "clientGame/ClientWorld.h"
#include "clientGame/ClientObject.h"
#include "clientGame/Game.h"
#include "clientGame/GroundScene.h"
#include "clientGame/GroupObject.h"
#include "clientGame/ShipObject.h"
#include "clientGame/ShipObjectAttachments.h"
#include "clientGraphics/DynamicVertexBuffer.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientObject/GameCamera.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedGame/ShipComponentAttachmentManager.h"
#include "sharedObject/Appearance.h"
#include "sharedMath/AxialBox.h"
#include "sharedMath/Line2d.h"
#include "sharedMath/Transform.h"
#include "sharedMath/Transform2d.h"
#include "sharedMath/Vector2d.h"
#include "sharedUtility/CurrentUserOptionManager.h"

#include <vector>

// ======================================================================

namespace SpaceTargetBracketOverlayNamespace
{
	uint32 const cms_blue = PackedArgb(192, 0, 0, 255).getArgb();
	uint32 const cms_cyan = PackedArgb(192, 0, 255, 255).getArgb();
	uint32 const cms_green = PackedArgb(192, 0, 255, 0).getArgb();
	uint32 const cms_orange = PackedArgb(192, 255, 128, 0).getArgb();
	uint32 const cms_purple = PackedArgb(192, 160, 0, 160).getArgb();
	uint32 const cms_red = PackedArgb(192, 255, 0, 0).getArgb();
	uint32 const cms_white = PackedArgb(192, 255, 255, 255).getArgb();
	uint32 const cms_yellow = PackedArgb(192, 255, 255, 0).getArgb();
	uint32 const cms_darkOffset = PackedArgb(128, 0, 0, 0).getArgb();

	class Indicator
	{
	public:

		void renderBrackets(VertexBufferWriteIterator & vi) const;
		int getNumberOfBracketVertices() const;

		void renderSelection(VertexBufferWriteIterator & vi) const;
		int getNumberOfSelectionVertices() const;

	public:

		float m_x;
		float m_y;
		float m_radius;
		uint32 m_color;
		bool m_selected;
	};

	bool ms_disable = false;
	bool ms_userDisable = false;

	typedef std::vector<Indicator> IndicatorList;
	IndicatorList ms_indicatorList;

	bool ms_showTargetStatusForAllShips = true;
	NetworkId ms_commTarget;

	DynamicVertexBuffer * ms_vertexBuffer;

	Timer ms_fadeTimer(0.5f);
	bool ms_fadeToWhite = false;

	void remove();
	void createTargetBrackets(Camera const * camera, IndicatorList & indicatorList);
	void createTargetBrackets(Camera const * camera, Object const * object, IndicatorList & indicatorList);
	void createTargetBrackets(Camera const * camera, Vector const & position_w, float radius, uint32 color, bool selected, bool communication, std::vector<Indicator> & indicatorList);
	uint32 computeColor(ShipObject const * targetShipObject);
	void renderOffScreenIndicator(Rectangle2d const & screenRect, Vector2d const & start, Vector2d end, uint32 color, float edgeOffset);
}

using namespace SpaceTargetBracketOverlayNamespace;

// ======================================================================

void SpaceTargetBracketOverlay::install()
{
	InstallTimer const installTimer("SpaceTargetBracketOverlay::install");

	DebugFlags::registerFlag(ms_disable, "ClientGame/SpaceTargetBracketOverlay", "disable");

//	CurrentUserOptionManager::registerOption(ms_showTargetStatusForAllShips, "ClientGame/SpaceTargetBracketOverlay", "showTargetStatusForAllShips");

	VertexBufferFormat format;
	format.setPosition();
	format.setTransformed();
	format.setColor0();
	ms_vertexBuffer = new DynamicVertexBuffer(format);

	ExitChain::add(SpaceTargetBracketOverlayNamespace::remove, "SpaceTargetBracketOverlayNamespace::remove");
}

// ----------------------------------------------------------------------

void SpaceTargetBracketOverlay::setDisabled(bool b)
{
	ms_userDisable = b;
}

// ----------------------------------------------------------------------

void SpaceTargetBracketOverlay::update(float const elapsedTime)
{
	if (ms_fadeTimer.updateZero(elapsedTime))
		ms_fadeToWhite = !ms_fadeToWhite;
}

// ----------------------------------------------------------------------

void SpaceTargetBracketOverlayNamespace::remove()
{
	DebugFlags::unregisterFlag(ms_disable);

	delete ms_vertexBuffer;
	ms_vertexBuffer = 0;
}

// ----------------------------------------------------------------------

bool SpaceTargetBracketOverlay::getShowTargetStatusForAllShips()
{
	return ms_showTargetStatusForAllShips;
}

// ----------------------------------------------------------------------

void SpaceTargetBracketOverlay::setShowTargetStatusForAllShips(bool const showTargetStatusForAllShips)
{
	ms_showTargetStatusForAllShips = showTargetStatusForAllShips;
}

// ----------------------------------------------------------------------

void SpaceTargetBracketOverlay::setCommTarget(NetworkId const & commTarget)
{
	ms_commTarget = commTarget;
}

// ======================================================================

SpaceTargetBracketOverlay::SpaceTargetBracketOverlay()
{
}

// ----------------------------------------------------------------------

SpaceTargetBracketOverlay::~SpaceTargetBracketOverlay()
{
}

// ----------------------------------------------------------------------

void SpaceTargetBracketOverlay::render() const
{
	if (ms_disable || ms_userDisable)
		return;

	//-- Make sure we're running a GroundScene
	GroundScene const * const groundScene = dynamic_cast<GroundScene const *>(Game::getConstScene());
	if (!groundScene)
		return;

	//-- Make sure we're in the cockpit camera or the ship turret camera
	if (groundScene->getCurrentView() != GroundScene::CI_cockpit && groundScene->getCurrentView() != GroundScene::CI_shipTurret)
		return;

	//-- Get the camera
	Camera const * const camera = groundScene->getCurrentCamera();
	if (!camera)
		return;

	//-- Get the player's ship
	if (!Game::getPlayer() || !Game::getPlayerContainingShip())
		return;

	//-- Fill out the indicator list
	ms_indicatorList.clear();
	createTargetBrackets(camera, ms_indicatorList);

	//-- Render the indicator list
	if (!ms_indicatorList.empty())
	{
		size_t const numberOfIndicators = ms_indicatorList.size();
		size_t selection = numberOfIndicators;

		int numberOfVertices = 0;
		{
			for (size_t i = 0; i < numberOfIndicators; ++i)
			{
				numberOfVertices += ms_indicatorList[i].getNumberOfBracketVertices();

				if (ms_indicatorList[i].m_selected)
					selection = i;
			}
		}

		Graphics::setStaticShader(ShaderTemplateList::get2dVertexColorAStaticShader());

		if (selection != numberOfIndicators)
		{
			ms_vertexBuffer->lock(ms_indicatorList[selection].getNumberOfSelectionVertices());

				VertexBufferWriteIterator vi = ms_vertexBuffer->begin();
				ms_indicatorList[selection].renderSelection(vi);

			ms_vertexBuffer->unlock();

			Graphics::setVertexBuffer(*ms_vertexBuffer);
			Graphics::drawTriangleList();
		}

		{
			ms_vertexBuffer->lock(numberOfVertices);

				VertexBufferWriteIterator vi = ms_vertexBuffer->begin();
				for (size_t i = 0; i < numberOfIndicators; ++i)
				{
					Indicator const & indicator = ms_indicatorList[i];
					indicator.renderBrackets(vi);
				}

			ms_vertexBuffer->unlock();

			Graphics::setVertexBuffer(*ms_vertexBuffer);
			Graphics::drawLineList();
		}
	}

	//-- Render off screen indicator
	{
		CreatureObject const * const playerCreatureObject = Game::getPlayerCreature();
		Object const * const object = playerCreatureObject->getLookAtTarget().getObject();
		if (object && object->getAppearance())
		{
			ClientObject const * const clientObject = object->asClientObject();
			if (clientObject)
			{
				ShipObject const * const targetShipObject = clientObject->asShipObject();
				if (targetShipObject)
				{
					Vector const objectPosition_w = object->getPosition_w();
					Vector objectPosition_c = camera->rotateTranslate_w2o(objectPosition_w);
					float screenX = 0.f;
					float screenY = 0.f;
					if (!camera->projectInCameraSpace(objectPosition_c, &screenX, &screenY, 0))
					{
						objectPosition_c.normalize();
						objectPosition_c.z = -objectPosition_c.y;
						float const theta = objectPosition_c.theta();

						Transform2d transform;
						transform.yaw_l(theta);

						float const edgeOffset = 20.f;
						Rectangle2d const screenRect(edgeOffset, edgeOffset, static_cast<float>(Graphics::getCurrentRenderTargetWidth()) - edgeOffset, static_cast<float>(Graphics::getCurrentRenderTargetHeight() - edgeOffset));
						Vector2d const start(screenRect.getCenter());
						Vector2d const end = transform.rotate_l2p(Vector2d(0.f, static_cast<float>(Graphics::getCurrentRenderTargetWidth()))) + start;
						renderOffScreenIndicator(screenRect, start, end, computeColor(targetShipObject), edgeOffset);
					}
				}
			}
		}
	}
}

// ======================================================================

void SpaceTargetBracketOverlayNamespace::Indicator::renderBrackets(VertexBufferWriteIterator & vi) const
{
	float const Z = 1.f;
	float const OOZ = 1.f;
	float const bracketSize = m_radius * 0.33f;
	float const selectionSize = clamp(10.f, m_radius * 0.33f, 20.f);

	uint32 color = m_color;
	if (m_selected)
		color = PackedArgb::linearInterpolate(m_color, cms_white, ms_fadeToWhite ? ms_fadeTimer.getElapsedRatio() : ms_fadeTimer.getRemainingRatio()).getArgb();

	//-- Upper left
	Vector2d const upperLeft(m_x - m_radius, m_y - m_radius);
	vi.setPosition(Vector(upperLeft.x, upperLeft.y + 1.f, Z));
	vi.setOoz(OOZ);
	vi.setColor0(color);
	++vi;

	vi.setPosition(Vector(upperLeft.x, upperLeft.y + bracketSize, Z));
	vi.setOoz(OOZ);
	vi.setColor0(color);
	++vi;

	vi.setPosition(Vector(upperLeft.x + 1.f, upperLeft.y, Z));
	vi.setOoz(OOZ);
	vi.setColor0(color);
	++vi;

	vi.setPosition(Vector(upperLeft.x + bracketSize, upperLeft.y, Z));
	vi.setOoz(OOZ);
	vi.setColor0(color);
	++vi;

	if (m_selected)
	{
		vi.setPosition(Vector(upperLeft.x, upperLeft.y, Z));
		vi.setOoz(OOZ);
		vi.setColor0(color);
		++vi;

		Vector2d const vertex1(upperLeft.x - selectionSize, upperLeft.y - selectionSize / 2.f); 
		vi.setPosition(Vector(vertex1.x, vertex1.y, Z));
		vi.setOoz(OOZ);
		vi.setColor0(color);
		++vi;

		vi.setPosition(Vector(vertex1.x, vertex1.y, Z));
		vi.setOoz(OOZ);
		vi.setColor0(color);
		++vi;

		Vector2d const vertex2(upperLeft.x - selectionSize / 2.f, upperLeft.y - selectionSize);
		vi.setPosition(Vector(vertex2.x, vertex2.y, Z));
		vi.setOoz(OOZ);
		vi.setColor0(color);
		++vi;

		vi.setPosition(Vector(vertex2.x, vertex2.y, Z));
		vi.setOoz(OOZ);
		vi.setColor0(color);
		++vi;

		vi.setPosition(Vector(upperLeft.x, upperLeft.y, Z));
		vi.setOoz(OOZ);
		vi.setColor0(color);
		++vi;
	}

	//-- Upper right
	Vector2d const upperRight(m_x + m_radius, m_y - m_radius);
	vi.setPosition(Vector(upperRight.x - bracketSize, upperRight.y, Z));
	vi.setOoz(OOZ);
	vi.setColor0(color);
	++vi;

	vi.setPosition(Vector(upperRight.x - 1.f, upperRight.y, Z));
	vi.setOoz(OOZ);
	vi.setColor0(color);
	++vi;

	vi.setPosition(Vector(upperRight.x, upperRight.y + 1.f, Z));
	vi.setOoz(OOZ);
	vi.setColor0(color);
	++vi;

	vi.setPosition(Vector(upperRight.x, upperRight.y + bracketSize, Z));
	vi.setOoz(OOZ);
	vi.setColor0(color);
	++vi;

	if (m_selected)
	{
		vi.setPosition(Vector(upperRight.x, upperRight.y, Z));
		vi.setOoz(OOZ);
		vi.setColor0(color);
		++vi;

		Vector2d const vertex1(upperRight.x + selectionSize / 2.f, upperRight.y - selectionSize); 
		vi.setPosition(Vector(vertex1.x, vertex1.y, Z));
		vi.setOoz(OOZ);
		vi.setColor0(color);
		++vi;

		vi.setPosition(Vector(vertex1.x, vertex1.y, Z));
		vi.setOoz(OOZ);
		vi.setColor0(color);
		++vi;

		Vector2d const vertex2(upperRight.x + selectionSize, upperRight.y - selectionSize / 2.f);
		vi.setPosition(Vector(vertex2.x, vertex2.y, Z));
		vi.setOoz(OOZ);
		vi.setColor0(color);
		++vi;

		vi.setPosition(Vector(vertex2.x, vertex2.y, Z));
		vi.setOoz(OOZ);
		vi.setColor0(color);
		++vi;

		vi.setPosition(Vector(upperRight.x, upperRight.y, Z));
		vi.setOoz(OOZ);
		vi.setColor0(color);
		++vi;
	}

	//-- Lower right
	Vector2d const lowerRight(m_x + m_radius, m_y + m_radius);
	vi.setPosition(Vector(lowerRight.x, lowerRight.y - bracketSize, Z));
	vi.setOoz(OOZ);
	vi.setColor0(color);
	++vi;

	vi.setPosition(Vector(lowerRight.x, lowerRight.y - 1.f, Z));
	vi.setOoz(OOZ);
	vi.setColor0(color);
	++vi;

	vi.setPosition(Vector(lowerRight.x - bracketSize, lowerRight.y, Z));
	vi.setOoz(OOZ);
	vi.setColor0(color);
	++vi;

	vi.setPosition(Vector(lowerRight.x - 1.f, lowerRight.y, Z));
	vi.setOoz(OOZ);
	vi.setColor0(color);
	++vi;

	if (m_selected)
	{
		vi.setPosition(Vector(lowerRight.x, lowerRight.y, Z));
		vi.setOoz(OOZ);
		vi.setColor0(color);
		++vi;

		Vector2d const vertex1(lowerRight.x + selectionSize, lowerRight.y + selectionSize / 2.f); 
		vi.setPosition(Vector(vertex1.x, vertex1.y, Z));
		vi.setOoz(OOZ);
		vi.setColor0(color);
		++vi;

		vi.setPosition(Vector(vertex1.x, vertex1.y, Z));
		vi.setOoz(OOZ);
		vi.setColor0(color);
		++vi;

		Vector2d const vertex2(lowerRight.x + selectionSize / 2.f, lowerRight.y + selectionSize);
		vi.setPosition(Vector(vertex2.x, vertex2.y, Z));
		vi.setOoz(OOZ);
		vi.setColor0(color);
		++vi;

		vi.setPosition(Vector(vertex2.x, vertex2.y, Z));
		vi.setOoz(OOZ);
		vi.setColor0(color);
		++vi;

		vi.setPosition(Vector(lowerRight.x, lowerRight.y, Z));
		vi.setOoz(OOZ);
		vi.setColor0(color);
		++vi;
	}

	//-- Lower left
	Vector2d const lowerLeft(m_x - m_radius, m_y + m_radius);
	vi.setPosition(Vector(lowerLeft.x, lowerLeft.y - bracketSize, Z));
	vi.setOoz(OOZ);
	vi.setColor0(color);
	++vi;

	vi.setPosition(Vector(lowerLeft.x, lowerLeft.y - 1.f, Z));
	vi.setOoz(OOZ);
	vi.setColor0(color);
	++vi;

	vi.setPosition(Vector(lowerLeft.x + 1.f, lowerLeft.y, Z));
	vi.setOoz(OOZ);
	vi.setColor0(color);
	++vi;

	vi.setPosition(Vector(lowerLeft.x + bracketSize, lowerLeft.y, Z));
	vi.setOoz(OOZ);
	vi.setColor0(color);
	++vi;

	if (m_selected)
	{
		vi.setPosition(Vector(lowerLeft.x, lowerLeft.y, Z));
		vi.setOoz(OOZ);
		vi.setColor0(color);
		++vi;

		Vector2d const vertex1(lowerLeft.x - selectionSize / 2.f, lowerLeft.y + selectionSize); 
		vi.setPosition(Vector(vertex1.x, vertex1.y, Z));
		vi.setOoz(OOZ);
		vi.setColor0(color);
		++vi;

		vi.setPosition(Vector(vertex1.x, vertex1.y, Z));
		vi.setOoz(OOZ);
		vi.setColor0(color);
		++vi;

		Vector2d const vertex2(lowerLeft.x - selectionSize, lowerLeft.y + selectionSize / 2.f);
		vi.setPosition(Vector(vertex2.x, vertex2.y, Z));
		vi.setOoz(OOZ);
		vi.setColor0(color);
		++vi;

		vi.setPosition(Vector(vertex2.x, vertex2.y, Z));
		vi.setOoz(OOZ);
		vi.setColor0(color);
		++vi;

		vi.setPosition(Vector(lowerLeft.x, lowerLeft.y, Z));
		vi.setOoz(OOZ);
		vi.setColor0(color);
		++vi;
	}
}

// ----------------------------------------------------------------------

int SpaceTargetBracketOverlayNamespace::Indicator::getNumberOfBracketVertices() const
{
	return 4 * (m_selected ? 10 : 4);
}

// ----------------------------------------------------------------------

void SpaceTargetBracketOverlayNamespace::Indicator::renderSelection(VertexBufferWriteIterator & vi) const
{
	float const Z = 1.f;
	float const OOZ = 1.f;
	float const selectionSize = clamp(10.f, m_radius * 0.33f, 20.f);

	uint32 const color = m_color - cms_darkOffset;

	//-- Upper left
	{
		Vector2d const upperLeft(m_x - m_radius, m_y - m_radius);
		vi.setPosition(Vector(upperLeft.x, upperLeft.y, Z));
		vi.setOoz(OOZ);
		vi.setColor0(color);
		++vi;

		Vector2d const vertex1(upperLeft.x - selectionSize, upperLeft.y - selectionSize / 2.f); 
		vi.setPosition(Vector(vertex1.x, vertex1.y, Z));
		vi.setOoz(OOZ);
		vi.setColor0(color);
		++vi;

		Vector2d const vertex2(upperLeft.x - selectionSize / 2.f, upperLeft.y - selectionSize);
		vi.setPosition(Vector(vertex2.x, vertex2.y, Z));
		vi.setOoz(OOZ);
		vi.setColor0(color);
		++vi;
	}

	//-- Upper right
	{
		Vector2d const upperRight(m_x + m_radius, m_y - m_radius);
		vi.setPosition(Vector(upperRight.x, upperRight.y, Z));
		vi.setOoz(OOZ);
		vi.setColor0(color);
		++vi;

		Vector2d const vertex1(upperRight.x + selectionSize / 2.f, upperRight.y - selectionSize); 
		vi.setPosition(Vector(vertex1.x, vertex1.y, Z));
		vi.setOoz(OOZ);
		vi.setColor0(color);
		++vi;

		Vector2d const vertex2(upperRight.x + selectionSize, upperRight.y - selectionSize / 2.f);
		vi.setPosition(Vector(vertex2.x, vertex2.y, Z));
		vi.setOoz(OOZ);
		vi.setColor0(color);
		++vi;
	}

	//-- Lower right
	{
		Vector2d const lowerRight(m_x + m_radius, m_y + m_radius);
		vi.setPosition(Vector(lowerRight.x, lowerRight.y, Z));
		vi.setOoz(OOZ);
		vi.setColor0(color);
		++vi;

		Vector2d const vertex1(lowerRight.x + selectionSize, lowerRight.y + selectionSize / 2.f); 
		vi.setPosition(Vector(vertex1.x, vertex1.y, Z));
		vi.setOoz(OOZ);
		vi.setColor0(color);
		++vi;

		Vector2d const vertex2(lowerRight.x + selectionSize / 2.f, lowerRight.y + selectionSize);
		vi.setPosition(Vector(vertex2.x, vertex2.y, Z));
		vi.setOoz(OOZ);
		vi.setColor0(color);
		++vi;
	}

	//-- Lower left
	{
		Vector2d const lowerLeft(m_x - m_radius, m_y + m_radius);
		vi.setPosition(Vector(lowerLeft.x, lowerLeft.y, Z));
		vi.setOoz(OOZ);
		vi.setColor0(color);
		++vi;

		Vector2d const vertex1(lowerLeft.x - selectionSize / 2.f, lowerLeft.y + selectionSize); 
		vi.setPosition(Vector(vertex1.x, vertex1.y, Z));
		vi.setOoz(OOZ);
		vi.setColor0(color);
		++vi;

		Vector2d const vertex2(lowerLeft.x - selectionSize, lowerLeft.y + selectionSize / 2.f);
		vi.setPosition(Vector(vertex2.x, vertex2.y, Z));
		vi.setOoz(OOZ);
		vi.setColor0(color);
		++vi;
	}
}

// ----------------------------------------------------------------------

int SpaceTargetBracketOverlayNamespace::Indicator::getNumberOfSelectionVertices() const
{
	return 4 * 3;
}

// ======================================================================

void SpaceTargetBracketOverlayNamespace::createTargetBrackets(Camera const * const camera, IndicatorList & indicatorList)
{
	CreatureObject const * const playerCreatureObject = Game::getPlayerCreature();

	if (ms_showTargetStatusForAllShips)
	{
		int const numberOfObjects = ClientWorld::getNumberOfObjects(WOL_Tangible);
		for (int i = 0; i < numberOfObjects; ++i)
		{
			Object const * const object = ClientWorld::getConstObject(WOL_Tangible, i);
			createTargetBrackets(camera, object, indicatorList);
		}
	}
	else
	{
		Object const * const object = playerCreatureObject->getLookAtTarget().getObject();
		createTargetBrackets(camera, object, indicatorList);
	}
}

// ----------------------------------------------------------------------

void SpaceTargetBracketOverlayNamespace::createTargetBrackets(Camera const * const camera, Object const * const object, IndicatorList & indicatorList)
{
	ShipObject const * const playerShipObject = Game::getPlayerContainingShip();

	if (object && object->getAppearance())
	{
		ClientObject const * const clientObject = object->asClientObject();
		if (clientObject)
		{
			ShipObject const * const targetShipObject = clientObject->asShipObject();
			if (targetShipObject && targetShipObject != playerShipObject)
			{
				//-- Grab the tangible extent for the object and create brackets around that
				AxialBox const tangibleExtent = targetShipObject->getTangibleExtent();
				if (!tangibleExtent.isEmpty())
				{
					bool const selected = NON_NULL(Game::getPlayerCreature())->getLookAtTarget() == targetShipObject->getNetworkId();

					Vector position_w = targetShipObject->rotateTranslate_o2w(tangibleExtent.getCenter());
					float const radius = (tangibleExtent.getWidth() + tangibleExtent.getDepth() + tangibleExtent.getHeight()) / 6.f;

					//-- For space stations, look for the bridge1 hardpoint
					if (targetShipObject->getObjectTemplate() && targetShipObject->getObjectTemplate()->asSharedObjectTemplate() && targetShipObject->getObjectTemplate()->asSharedObjectTemplate()->getGameObjectType() == SharedObjectTemplate::GOT_ship_station)
					{
						Transform hardpointTransform_o2p;
						ConstCharCrcString const hardpointName1("bridge1");
						if (targetShipObject->getAppearance()->findHardpoint(hardpointName1, hardpointTransform_o2p))
							position_w = targetShipObject->rotateTranslate_o2w(hardpointTransform_o2p.getPosition_p());
					}

					createTargetBrackets(camera, position_w, radius, computeColor(targetShipObject), selected, false, indicatorList);

					//-- Render additional brackets if the ship is comming with you
					if (targetShipObject->getNetworkId() == ms_commTarget)
						createTargetBrackets(camera, position_w, radius, cms_white, false, true, indicatorList);

					//-- Render additional brackets around the subtarget
					ShipChassisSlotType::Type const lookAtTargetSlot = selected ? Game::getPlayerCreature()->getLookAtTargetSlot() : ShipChassisSlotType::SCST_invalid;
					if (lookAtTargetSlot != ShipChassisSlotType::SCST_invalid)
					{
						ShipObjectAttachments const & shipObjectAttachments = targetShipObject->getShipObjectAttachments();
						Vector componentPosition_w;
						if (shipObjectAttachments.getFirstComponentPosition_w(lookAtTargetSlot, componentPosition_w))
						{
							float const componentRadius = ShipComponentAttachmentManager::getComponentHitRange(targetShipObject->getChassisType(), targetShipObject->getComponentCrc(lookAtTargetSlot), lookAtTargetSlot);
							createTargetBrackets(camera, componentPosition_w, componentRadius, cms_yellow, false, false, indicatorList);
						}
					}
				}
			}
		}
	}
}

// ----------------------------------------------------------------------

void SpaceTargetBracketOverlayNamespace::createTargetBrackets(Camera const * const camera, Vector const & position_w, float const radius, uint32 const color, bool const selected, bool const communication, std::vector<Indicator> & indicatorList) 
{
	Vector const position_c = camera->rotateTranslate_w2o(position_w);
	if (position_c.z > camera->getNearPlane())
	{
		Indicator indicator;
		if (camera->projectInCameraSpace(position_c, &indicator.m_x, &indicator.m_y, 0))
		{
			if (camera->computeRadiusInScreenSpace(position_c, radius, indicator.m_radius))
			{
				indicator.m_selected = selected;
				indicator.m_radius = clamp(10.f, indicator.m_radius, 100.f) + (communication ? 5.f : 0.f);
				indicator.m_color = color;
				indicatorList.push_back(indicator);
			}
		}
	}
}

// ----------------------------------------------------------------------

uint32 SpaceTargetBracketOverlayNamespace::computeColor(ShipObject const * const targetShipObject)
{
	CreatureObject const * const playerCreatureObject = Game::getPlayerCreature();
	if (playerCreatureObject)
	{
		if (targetShipObject->isEnemy())
		{
			//-- Orange
			if (playerCreatureObject->isGroupMissionCriticalObject(targetShipObject->getNetworkId()))
				return cms_orange;

			//-- Red
			return cms_red;
		}

		//-- Cyan
		if (playerCreatureObject->isGroupMissionCriticalObject(targetShipObject->getNetworkId()))
			return cms_cyan;

		GroupObject const * const groupObject = safe_cast<GroupObject const *>(playerCreatureObject->getGroup().getObject());
		if (groupObject)
		{
			//-- Purple
			if (groupObject->getLeader() == targetShipObject->getNetworkId())
				return cms_purple;

			//-- Green
			if (groupObject->isGroupMember(targetShipObject->getNetworkId()))
				return cms_green;
		}
	}

	//-- Blue
	return cms_blue;
}

// ----------------------------------------------------------------------

void SpaceTargetBracketOverlayNamespace::renderOffScreenIndicator(Rectangle2d const & screenRect, Vector2d const & start, Vector2d end, uint32 color, float const edgeOffset)
{
	if (end.x < screenRect.x0)
	{
		//-- Clip left
		float const t = (screenRect.x0 - start.x) / (end.x - start.x);
		end = Vector2d::linearInterpolate(start, end, t);
	}
	else
		if (end.x > screenRect.x1)
		{
			//-- Clip right
			float const t = (screenRect.x1 - start.x) / (end.x - start.x);
			end = Vector2d::linearInterpolate(start, end, t);
		}

	if (end.y < screenRect.y0)
	{
		//-- Clip top
		float const t = (screenRect.y0 - start.y) / (end.y - start.y);
		end = Vector2d::linearInterpolate(start, end, t);
	}
	else
		if (end.y > screenRect.y1)
		{
			//-- Clip bottom
			float const t = (screenRect.y1 - start.y) / (end.y - start.y);
			end = Vector2d::linearInterpolate(start, end, t);
		}

	Graphics::setStaticShader(ShaderTemplateList::get2dVertexColorAStaticShader());

	//-- Inside
	{
		float const Z = 1.f;
		float const OOZ = 1.f;

		ms_vertexBuffer->lock(4);

			VertexBufferWriteIterator vi = ms_vertexBuffer->begin();

			float const radius = edgeOffset * 0.75f;
			uint32 const desiredColor = color - cms_darkOffset;

			vi.setPosition(Vector(end.x, end.y - radius, Z));
			vi.setOoz(OOZ);
			vi.setColor0(desiredColor);
			++vi;

			vi.setPosition(Vector(end.x + radius, end.y, Z));
			vi.setOoz(OOZ);
			vi.setColor0(desiredColor);
			++vi;

			vi.setPosition(Vector(end.x, end.y + radius, Z));
			vi.setOoz(OOZ);
			vi.setColor0(desiredColor);
			++vi;

			vi.setPosition(Vector(end.x - radius, end.y, Z));
			vi.setOoz(OOZ);
			vi.setColor0(desiredColor);
			++vi;

		ms_vertexBuffer->unlock();

		Graphics::setVertexBuffer(*ms_vertexBuffer);
		Graphics::drawTriangleFan();
	}

	//-- Outside
	{
		float const Z = 1.f;
		float const OOZ = 1.f;

		ms_vertexBuffer->lock(8);

			VertexBufferWriteIterator vi = ms_vertexBuffer->begin();

			float const radius = edgeOffset * 0.75f;
			uint32 const desiredColor = PackedArgb::linearInterpolate(color, cms_white, ms_fadeToWhite ? ms_fadeTimer.getElapsedRatio() : ms_fadeTimer.getRemainingRatio()).getArgb();

			vi.setPosition(Vector(end.x, end.y - radius, Z));
			vi.setOoz(OOZ);
			vi.setColor0(desiredColor);
			++vi;

			vi.setPosition(Vector(end.x + radius, end.y, Z));
			vi.setOoz(OOZ);
			vi.setColor0(desiredColor);
			++vi;

			vi.setPosition(Vector(end.x + radius, end.y, Z));
			vi.setOoz(OOZ);
			vi.setColor0(desiredColor);
			++vi;

			vi.setPosition(Vector(end.x, end.y + radius, Z));
			vi.setOoz(OOZ);
			vi.setColor0(desiredColor);
			++vi;

			vi.setPosition(Vector(end.x, end.y + radius, Z));
			vi.setOoz(OOZ);
			vi.setColor0(desiredColor);
			++vi;

			vi.setPosition(Vector(end.x - radius, end.y, Z));
			vi.setOoz(OOZ);
			vi.setColor0(desiredColor);
			++vi;

			vi.setPosition(Vector(end.x - radius, end.y, Z));
			vi.setOoz(OOZ);
			vi.setColor0(desiredColor);
			++vi;

			vi.setPosition(Vector(end.x, end.y - radius, Z));
			vi.setOoz(OOZ);
			vi.setColor0(desiredColor);
			++vi;

		ms_vertexBuffer->unlock();

		Graphics::setVertexBuffer(*ms_vertexBuffer);
		Graphics::drawLineList();
	}
}

// ======================================================================

