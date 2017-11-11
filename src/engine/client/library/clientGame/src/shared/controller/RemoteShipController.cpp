// ======================================================================
//
// RemoteShipController.cpp
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/RemoteShipController.h"

#include "clientGame/ClientShipObjectInterface.h"
#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "clientGame/ShipObject.h"
#include "clientGraphics/Camera.h"
#include "clientGraphics/DebugPrimitive.h"
#include "sharedCollision/BoxExtent.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/Exitchain.h"
#include "sharedGame/ShipDynamicsModel.h"
#include "sharedMath/Quaternion.h"
#include "sharedMathArchive/TransformArchive.h"
#include "sharedNetworkMessages/ShipUpdateTransformMessage.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/CellProperty.h"

// ======================================================================
// RemoteShipControllerNamespace
// ======================================================================

namespace RemoteShipControllerNamespace
{
	bool ms_disableModeling = false;
	bool ms_disableSmoothing = false;
	bool ms_logHandleNetUpdateTransform = false;
	bool ms_renderClientTransform = false;
	bool ms_renderServerTransform = false;
	bool ms_renderModeledServerTransform = false;
	bool ms_renderShipExtent = false;
	float const cms_hyperspaceSpeed = 600.0f;
	float const cms_maximumHyperspaceTimeSeconds = 2.0f;
}

using namespace RemoteShipControllerNamespace;

// ======================================================================
// STATIC PUBLIC RemoteShipController
// ======================================================================

void RemoteShipController::install()
{
	InstallTimer const installTimer("RemoteShipController::install");

	DebugFlags::registerFlag(ms_disableModeling, "ClientGame/RemoteShipController", "disableModeling");
	DebugFlags::registerFlag(ms_disableSmoothing, "ClientGame/RemoteShipController", "disableSmoothing");
	DebugFlags::registerFlag(ms_logHandleNetUpdateTransform, "ClientGame/RemoteShipController", "logHandleNetUpdateTransform*");
	DebugFlags::registerFlag(ms_renderClientTransform, "ClientGame/RemoteShipController", "renderClientTransform");
	DebugFlags::registerFlag(ms_renderServerTransform, "ClientGame/RemoteShipController", "renderServerTransform");
	DebugFlags::registerFlag(ms_renderModeledServerTransform, "ClientGame/RemoteShipController", "renderModeledServerTransform");
	DebugFlags::registerFlag(ms_renderShipExtent, "ClientGame/RemoteShipController", "renderShipExtent");
	ExitChain::add(&remove, "RemoteShipController::remove");
}

// ======================================================================
// STATIC PRIVATE RemoteShipController
// ======================================================================

void RemoteShipController::remove()
{
	DebugFlags::unregisterFlag(ms_disableModeling);
	DebugFlags::unregisterFlag(ms_disableSmoothing);
	DebugFlags::unregisterFlag(ms_logHandleNetUpdateTransform);
	DebugFlags::unregisterFlag(ms_renderClientTransform);
	DebugFlags::unregisterFlag(ms_renderServerTransform);
	DebugFlags::unregisterFlag(ms_renderModeledServerTransform);
	DebugFlags::unregisterFlag(ms_renderShipExtent);
}

// ======================================================================
// PUBLIC RemoteShipController
// ======================================================================

RemoteShipController::RemoteShipController(ShipObject * const newOwner) :
	ShipController(newOwner),
	m_serverToClientLastSyncStamp(0),
	m_serverTransform_p(),
	m_isInHyperspace(false),
	m_killShipWhenFinishedHyperspace(false),
	m_totalElapsedTimeInHyperspaceSeconds(0.0f),
	m_transformAfterHyperspace()
{
}

// ----------------------------------------------------------------------

RemoteShipController::~RemoteShipController()
{
	if (m_isInHyperspace)
	{
		Object * const owner = getOwner();
		if (owner != 0)
		{
			owner->setTransform_o2p(m_transformAfterHyperspace);
		}
	}
}

// ----------------------------------------------------------------------

void RemoteShipController::endBaselines()
{
	Object * const owner = getOwner();
	if (owner)
		m_serverTransform_p = owner->getTransform_o2p();

	ShipController::endBaselines();
}

// ----------------------------------------------------------------------

void RemoteShipController::receiveTransform(ShipUpdateTransformMessage const & shipUpdateTransformMessage)
{
	uint32 const syncStampLong = shipUpdateTransformMessage.getSyncStampLong();
	if (   m_serverToClientLastSyncStamp == 0
	    || GameNetwork::syncStampLongDeltaTime(m_serverToClientLastSyncStamp, syncStampLong) > 0)
	{
		m_serverToClientLastSyncStamp = syncStampLong;
		m_serverTransform_p = shipUpdateTransformMessage.getTransform();
		m_shipDynamicsModel->setTransform(m_serverTransform_p);
		m_shipDynamicsModel->setVelocity(shipUpdateTransformMessage.getVelocity());
		m_shipDynamicsModel->setYawRate(shipUpdateTransformMessage.getYawRate());
		m_shipDynamicsModel->setPitchRate(shipUpdateTransformMessage.getPitchRate());
		m_shipDynamicsModel->setRollRate(shipUpdateTransformMessage.getRollRate());

		ShipObject const * const owner = getShipOwner();
		NOT_NULL(owner);

		//-- Advance by delta time
		float const deltaTime = GameNetwork::computeDeltaTimeInSeconds(shipUpdateTransformMessage.getSyncStampLong());
		ClientShipObjectInterface const clientShipObjectInterface(owner);
		m_shipDynamicsModel->predict(deltaTime, clientShipObjectInterface);

		DEBUG_REPORT_LOG(ms_logHandleNetUpdateTransform,("RemoteShipController::receiveTransform[%s]: %1.2f %1.2f %1.2f\n", owner->getNetworkId().getValueString().c_str(), shipUpdateTransformMessage.getYawRate(), shipUpdateTransformMessage.getPitchRate(), shipUpdateTransformMessage.getRollRate(), shipUpdateTransformMessage.getVelocity().magnitude()));
	}
	else
		DEBUG_REPORT_LOG(true,("RemoteShipController::receiveTransform[%s]: disregarding older packet\n", getOwner()->getNetworkId().getValueString().c_str()));
}

// ======================================================================
// PROTECTED RemoteShipController
// ======================================================================

float RemoteShipController::realAlter(const float elapsedTime)
{
	NP_PROFILER_AUTO_BLOCK_DEFINE ("RemoteShipController::realAlter");

	ShipObject * const owner = getShipOwner();
	if (owner && owner->isInitialized())
	{
		//-- Update the server position
		m_shipDynamicsModel->predict(elapsedTime, ClientShipObjectInterface(owner));

		if (m_isInHyperspace)
		{
			m_totalElapsedTimeInHyperspaceSeconds += elapsedTime;

			bool const finished = (m_totalElapsedTimeInHyperspaceSeconds >= cms_maximumHyperspaceTimeSeconds);

			if (finished)
			{
				owner->setTransform_o2p(m_transformAfterHyperspace);
				m_isInHyperspace = false;

				if (m_killShipWhenFinishedHyperspace)
				{
					m_killShipWhenFinishedHyperspace = false;
					owner->kill();
				}
			}
			else
			{
				float const distanceToMove = cms_hyperspaceSpeed * elapsedTime;
				owner->move_o(Vector(0.0f, 0.0f, distanceToMove));
			}

			m_shipDynamicsModel->setTransform(owner->getTransform_o2p());
		}
		else
		{
			Transform const & serverTransform = ms_disableModeling ? m_serverTransform_p : m_shipDynamicsModel->getTransform();

			//-- Lerp client position to server position
			if (ms_disableSmoothing)
				owner->setTransform_o2p(serverTransform);
			else
			{
				const Transform & clientTransform = owner->getTransform_o2p();
				if (!clientTransform.approximates(serverTransform))
				{
					//determine new position
					const Object& cell = owner->getParentCell()->getOwner();
					Vector const serverPosition_w = cell.rotateTranslate_o2w(serverTransform.getPosition_p());
					Vector const clientPosition_w = owner->getPosition_w();
					Vector const position_w = Vector::linearInterpolate(clientPosition_w, serverPosition_w, clamp(0.0f, 2.f * elapsedTime, 1.0f));

					//determine new rotation
					Transform const startTransform(owner->getTransform_o2p());
					Quaternion const startOrientation(startTransform);
					Quaternion const endOrientation(serverTransform);
					Quaternion const orientation(startOrientation.slerp(endOrientation, clamp(0.0f, 2.f * elapsedTime, 1.0f)));

					Transform transform_p;
					orientation.getTransform(&transform_p);
					transform_p.reorthonormalize();
					transform_p.setPosition_p(position_w);

					//apply new rotation/position
					owner->setTransform_o2p(transform_p);
				}
			}
		}

#ifdef _DEBUG
		Camera const * const camera = Game::getCamera();
		if (camera)
		{
			//-- render ship extent
			if (ms_renderShipExtent)
			{
				BoxExtent const * const boxExtent = dynamic_cast<BoxExtent const *>(owner->getAppearance()->getExtent());
				if (boxExtent)
				{
					AxialBox const axialBox = boxExtent->getBoundingBox();
					Transform const transform_w = owner->getTransform_o2w();
					BoxDebugPrimitive * const boxDebugPrimitive = new BoxDebugPrimitive(BoxDebugPrimitive::S_z, transform_w, axialBox);
					if (boxDebugPrimitive)
					{
						boxDebugPrimitive->setColor(PackedArgb::solidYellow);
						camera->addDebugPrimitive(boxDebugPrimitive);
					}
				}
			}

			//-- render client transform
			if (ms_renderClientTransform)
				camera->addDebugPrimitive(new FrameDebugPrimitive(FrameDebugPrimitive::S_none, owner->getTransform_o2w(), 10.0f));

			//-- render server transform
			if (ms_renderServerTransform)
				camera->addDebugPrimitive(new FrameDebugPrimitive(FrameDebugPrimitive::S_none, m_serverTransform_p, 1.f));

			//-- render modeled server transform
			if (ms_renderModeledServerTransform)
				camera->addDebugPrimitive(new FrameDebugPrimitive(FrameDebugPrimitive::S_none, m_shipDynamicsModel->getTransform(), 1.f));
		}
#endif
	}

	float alterResult = ShipController::realAlter(elapsedTime);
	AlterResult::incorporateAlterResult(alterResult, AlterResult::cms_alterNextFrame);
	return alterResult;
}

// ----------------------------------------------------------------------

void RemoteShipController::handleNetUpdateTransform(MessageQueueDataTransform const & /*message*/)
{
	DEBUG_FATAL(true, ("RemoteShipController::handleNetUpdateTransform: not implemented (nor should it be)"));
}

// ----------------------------------------------------------------------

void RemoteShipController::handleNetUpdateTransformWithParent(MessageQueueDataTransformWithParent const & /*message*/)
{
	DEBUG_FATAL(true, ("RemoteShipController::handleNetUpdateTransformWithParent: not implemented"));
}

// ----------------------------------------------------------------------

void RemoteShipController::enterByHyperspace()
{
	m_isInHyperspace = true;
	m_killShipWhenFinishedHyperspace = false;
	m_totalElapsedTimeInHyperspaceSeconds = 0.0f;

	ShipObject * const owner = getShipOwner();
	if (owner != 0)
	{
		m_transformAfterHyperspace = owner->getTransform_o2p();
		// get the transform and back the ship up
		float const distanceToMove = cms_hyperspaceSpeed * cms_maximumHyperspaceTimeSeconds;
		owner->move_o(Vector(0.0f, 0.0f, -distanceToMove));
	}
}

// ----------------------------------------------------------------------

void RemoteShipController::leaveByHyperspace()
{
	m_isInHyperspace = true;
	m_killShipWhenFinishedHyperspace = true;
	m_totalElapsedTimeInHyperspaceSeconds = 0.0f;

	ShipObject const * const owner = getShipOwner();
	if (owner != 0)
	{
		m_transformAfterHyperspace = owner->getTransform_o2p();
		float const distanceToMove = cms_hyperspaceSpeed * cms_maximumHyperspaceTimeSeconds;
		m_transformAfterHyperspace.move_l(Vector(0.0f, 0.0f, distanceToMove));
	}
}


// ======================================================================
