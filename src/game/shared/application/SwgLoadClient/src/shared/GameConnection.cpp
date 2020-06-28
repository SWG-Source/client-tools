// GameConnection.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "FirstSwgLoadClient.h"
#include "Archive/ByteStream.h"
#include "Client.h"
#include "ConfigSwgLoadClient.h"
#include "GameConnection.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/Timer.h"
#include "sharedMath/Transform.h"
#include "sharedMath/Vector.h"
#include "sharedMath/Vector2d.h"
#include "sharedMathArchive/TransformArchive.h"
#include "sharedMathArchive/VectorArchive.h"
#include "sharedNetworkMessages/ClientLoginMessages.h"
#include "sharedNetworkMessages/ClientCentralMessages.h"
#include "sharedNetworkMessages/ClientPermissionsMessage.h"
#include "sharedNetworkMessages/CommandChannelMessages.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedNetworkMessages/MessageQueueCommandQueueEnqueue.h"
#include "sharedNetworkMessages/MessageQueueDataTransform.h"
#include "sharedNetworkMessages/MessageQueueDataTransformWithParent.h"
#include "sharedNetworkMessages/MessageQueueTeleportAck.h"
#include "sharedNetworkMessages/ObjectChannelMessages.h"
#include "sharedNetworkMessages/ShipUpdateTransformMessage.h"
#include "sharedNetworkMessages/UpdateContainmentMessage.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedRandom/Random.h"
#include "SwgLoadClient.h"
#include "UnicodeUtils.h"

//-----------------------------------------------------------------------

const unsigned long gs_defaultUnreliableSendRateMilliseconds = 250;
const unsigned long gs_maxUnreliableSendRateMilliseconds = 10000;
const float gs_shipTransformUpdateTime = 0.2f;
const float gs_shipTransformReliableUpdateTime = 4.f;

#if 0
// Old load client info with bad customization data, here for future troubleshooting of crash condition
const char *gs_newCharacterTemplate = "object/creature/player/human_male.iff";
const char *gs_newCharacterHairTemplate = "object/tangible/hair/human/hair_human_male_s06.iff";
const char *gs_newCharacterProfession = "combat_engineer";
const char *gs_newCharacterBodyCustomizationString = "@RD";
const char *gs_newCharacterHairCustomizationString = "2#0#1#private#15#1#index_color_1#1#e0#";
#else
// Load clients are purty zabrak female artisans
const char *gs_newCharacterTemplate = "object/creature/player/zabrak_female.iff";
const char *gs_newCharacterHairTemplate = "object/tangible/hair/zabrak/hair_zabrak_female_s18.iff";
const char *gs_newCharacterProfession = "crafting_artisan";
const char *gs_newCharacterBodyCustomizationString =
	"\x01\x27\xAB\x35\xFF\x01\x17\x38\x18\xFF\x01\x1C\x6B\x1B"
	"\xFF\x01\x05\xFF\x01\x1A\xFF\x01\x19\x6B\x0D\xFF\x01\x09"
	"\x3D\x12\xFF\x01\x13\xD1\x20\xFF\x01\x10\xFF\x01\x21\x57"
	"\x0F\xFF\x02\x14\x22\x11\xFF\x01\x0E\x97\x03\xD1\x0B\x80"
	"\x0C\xFF\x01\x06\xFF\x01\x08\xDC\x15\xFF\x02\x16\xFF\x01"
	"\x04\x7B\x07\x61\x0A\xFF\x01\x36\x19\x2C\x49\x25\xFF\x01"
	"\x2D\x01\x35\x03\x24\xFF\x01\x2A\xFF\x01\x01\x06\x22\xFF"
	"\x01\x2E\x15\xFF\x03";
const char *gs_newCharacterHairCustomizationString =
	"\x01\x02\x01\x04\x02\x09\xFF\x03";
#endif

//-----------------------------------------------------------------------

GameConnection::GameConnection(Client * o, const std::string & a, const unsigned short p, const NetworkId &characterId, const NetworkSetupData &setupData) :
	LoadConnection(a, p, setupData),
	m_characterObjectId(characterId),
	m_characterContainerId(NetworkId::cms_invalid),
	m_characterInShip(false),
	m_owner(o),
	m_transform(),
	m_readyToSimulate(false),
	m_sequenceNumber(0),
	m_updateTransformTimer(2.0f),
	m_velocity(),
	m_chatEventTimer(Random::randomReal(ConfigSwgLoadClient::getChatEventTimerMin(), ConfigSwgLoadClient::getChatEventTimerMax())),
	m_chatTextCount(0),
	m_socialEventTimer(Random::randomReal(ConfigSwgLoadClient::getSocialEventTimerMin(), ConfigSwgLoadClient::getSocialEventTimerMax())),
	m_timeOfLastUnreliableSendMilliseconds(0),
	m_timeOfLastReceiveMilliseconds(0),
	m_unreliableSendRateMilliseconds(gs_defaultUnreliableSendRateMilliseconds),
	m_sentThisFrame(false),
	m_receiveThisFrame(false),
	m_shipTransformUpdateTimer(gs_shipTransformUpdateTime),
	m_shipTransformReliableUpdateTimer(gs_shipTransformReliableUpdateTime)
{
	m_velocity = Vector(Random::randomReal(-1.0f, 1.0f), 0, Random::randomReal(-1.0f, 1.0f));
	m_velocity.normalize();
	m_velocity = m_velocity * (Random::randomReal(0.5f, 5.0f));

	for(int i = 0; chatText[i] != 0; ++i)
	{
		m_chatTextCount++;
	}
}

//-----------------------------------------------------------------------

GameConnection::~GameConnection()
{
	REPORT_LOG(true, ("[%s] Game connection closed\n", m_owner->getLoginId().c_str()));
	m_owner->onConnectionClosed(this);
}

//-----------------------------------------------------------------------

void GameConnection::onConnectionClosed()
{
}

//-----------------------------------------------------------------------

void GameConnection::chat(char const *text)
{
	++m_sequenceNumber;
	static unsigned long commandHash = Crc::normalizeAndCalculate("spatialChatInternal");

	Unicode::String params(Unicode::narrowToWide("0 0 0 0 0 "));
	params += Unicode::narrowToWide(text);

	MessageQueueCommandQueueEnqueue msg(m_sequenceNumber, commandHash, NetworkId::cms_invalid, params);
	ObjControllerMessage message(m_characterObjectId, CM_commandQueueEnqueue, 0.0f, GameControllerMessageFlags::SEND | GameControllerMessageFlags::DEST_SERVER, &msg);
	send(message, true);

	REPORT_LOG(true, ("[%s] Chats : \"%s\"\n", m_owner->getLoginId().c_str(), text));
}

//-----------------------------------------------------------------------

static int socialTypes[] = {328, 78};

void GameConnection::social()
{
	++m_sequenceNumber;
	int socialEntry = socialTypes[Random::random(0, sizeof(socialTypes) / sizeof(int) - 1)];
	
	static unsigned long commandHash = Crc::normalizeAndCalculate("socialInternal");
	static NetworkId targetId;
	char buf[256] = {"\0"};
	snprintf(buf, sizeof (buf), "0 %d ", socialEntry);
	Unicode::String params(Unicode::narrowToWide(buf));

	MessageQueueCommandQueueEnqueue msg(m_sequenceNumber, commandHash, targetId, params);
	ObjControllerMessage message(m_characterObjectId, CM_commandQueueEnqueue, 0.0f, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE |GameControllerMessageFlags::DEST_AUTH_SERVER, &msg);
	send(message, true);

	REPORT_LOG(true, ("[%s] plays a social\n", m_owner->getLoginId().c_str()));
}

//-----------------------------------------------------------------------

const Vector & GameConnection::getVelocity() const
{
	return m_velocity;
}

//-----------------------------------------------------------------------

const bool GameConnection::getReadyToSimulate() const
{
	return m_readyToSimulate;
}

//-----------------------------------------------------------------------

void GameConnection::onConnectionOpened()
{
	const LoginClientToken * token = m_owner->getLoginClientToken();

	ClientIdMsg l(token->getToken(), token->getTokenSize(), 0);
	send(l, true);
}

//-----------------------------------------------------------------------

void GameConnection::onReceive(const Archive::ByteStream & data)
{
	// adjust send rate if necessary
	if(! m_receiveThisFrame)
	{
		unsigned long timeMs = Clock::timeMs();
		if(timeMs < m_timeOfLastReceiveMilliseconds + m_unreliableSendRateMilliseconds)
		{
			if(m_unreliableSendRateMilliseconds / 2 >= gs_defaultUnreliableSendRateMilliseconds)
			{
				m_unreliableSendRateMilliseconds = m_unreliableSendRateMilliseconds / 2;
			}
			else
			{
				m_unreliableSendRateMilliseconds = gs_defaultUnreliableSendRateMilliseconds;
			}	
		}
		m_receiveThisFrame = true;
		m_timeOfLastReceiveMilliseconds = timeMs;
	}

	static Archive::ReadIterator ri;
	ri = data.begin();
	GameNetworkMessage msg(ri);
	
	ri = data.begin();

	if(msg.isType("ClientPermissionsMessage"))
	{
		REPORT_LOG(true, ("[%s] received ClientPermissionMessage : ", m_owner->getLoginId().c_str()));
		ClientPermissionsMessage cpm(ri);
		if(! cpm.getCanLogin())
		{
			REPORT_LOG(true, ("CAN'T LOG IN! ABORTING!\n"));
			SwgLoadClient::quit();
		}
		else
		{
			REPORT_LOG(true, ("can login "));
			if(cpm.getCanCreateRegularCharacter())
			{
				REPORT_LOG(true, ("and can create character "));
				if(m_characterObjectId == NetworkId::cms_invalid)
				{
					REPORT_LOG(true, ("and need to create character. Sending ClientCreateCharacter message\n"));
					ClientCreateCharacter c(
						Unicode::narrowToWide(m_owner->getLoginId()),
						gs_newCharacterTemplate,
						1.0f,
						ConfigSwgLoadClient::getCreationLocation(),
						gs_newCharacterBodyCustomizationString,
						gs_newCharacterHairTemplate,
						gs_newCharacterHairCustomizationString,
						gs_newCharacterProfession,
						false,
						Unicode::String(),
						false, 
						"smuggler_2a", 
						"combat_brawler_2handmelee_01");
					send(c, true);
				}
				else
				{
					REPORT_LOG(true, ("and I will send a SelectCharacter message\n"));
					SelectCharacter s(m_characterObjectId);
					send(s, true);
				}
			}
			else
			{
				REPORT_LOG(true, ("and I will send a SelectCharacter message\n"));
				SelectCharacter s(m_characterObjectId);
				send(s, true);
			}
		}
	}
	else if(msg.isType("ClientCreateCharacterSuccess"))
	{
		REPORT_LOG(true, ("[%s] received ClientCreateCharacterSuccess message\n", m_owner->getLoginId().c_str()));
		ClientCreateCharacterSuccess cccs(ri);
		m_characterObjectId = cccs.getNetworkId();
		SelectCharacter s(m_characterObjectId);
		send(s, true);
	}
	else if(msg.isType("CmdStartScene"))
	{
		REPORT_LOG(true, ("[%s] received CmdStartScene message : ", m_owner->getLoginId().c_str()));
		CmdStartScene start(ri);
		if(! ConfigSwgLoadClient::getResetStart())
		{
			REPORT_LOG(true, ("and starting at the given coordinates of %f, %f, %f\n", start.getStartPosition().x, start.getStartPosition().y, start.getStartPosition().z));
			m_transform.setPosition_p(start.getStartPosition());
		}
		else
		{
			REPORT_LOG(true, ("and warping to new coordinates of %f, %f\n", ConfigSwgLoadClient::getStartX(), ConfigSwgLoadClient::getStartZ()));
			Vector start(ConfigSwgLoadClient::getStartX(), 0.0f, ConfigSwgLoadClient::getStartZ());
			m_transform.setPosition_p(start);
		}
		REPORT_LOG(true, ("[%s] My character object ID is %s\n", m_owner->getLoginId().c_str(), m_characterObjectId.getValueString().c_str()));
		DEBUG_FATAL(m_characterObjectId != start.getObjectId(), ("Got a CmdStartScene for an object we didn't log in with?"));
		m_characterContainerId = NetworkId::cms_invalid;
		m_characterInShip = false;
		m_readyToSimulate = true;
		CmdSceneReady s;
		send(s, true);
		chat(ConfigSwgLoadClient::getScriptSetupText());
	}
	else if(msg.isType("UpdateContainmentMessage"))
	{
		UpdateContainmentMessage o(ri);
		if (o.getNetworkId() == m_characterObjectId)
		{
			m_characterContainerId = o.getContainerId();
			m_characterInShip = o.getSlotArrangement() != -1;
		}
	}
	else if(msg.isType("ObjControllerMessage"))
	{
		ObjControllerMessage c(ri);
		if (   c.getNetworkId() == m_characterObjectId
		    || (   m_characterInShip
				    && c.getNetworkId() == m_characterContainerId
						&& c.getMessage() == CM_netUpdateTransform))
		{
			// acknowledge teleports
			if (c.getMessage() == CM_netUpdateTransform)
			{
				MessageQueueDataTransform const *messageData = dynamic_cast<MessageQueueDataTransform const *>(c.getData());
				if (messageData)
				{
					if (m_characterInShip && c.getNetworkId() == m_characterContainerId)
						m_transform = messageData->getTransform(); // the ship is teleporting
					MessageQueueTeleportAck ackData(messageData->getSequenceNumber());
					ObjControllerMessage const msg(
						c.getNetworkId(),
						CM_teleportAck,
						0.0f,
						GameControllerMessageFlags::SEND | 
						GameControllerMessageFlags::RELIABLE |
						GameControllerMessageFlags::DEST_AUTH_SERVER,
						&ackData);
					send(msg, true);
				}
			}			
			if (c.getMessage() == CM_netUpdateTransformWithParent)
			{
				MessageQueueDataTransformWithParent const *messageData = dynamic_cast<MessageQueueDataTransformWithParent const *>(c.getData());
				if (messageData)
				{
					MessageQueueTeleportAck ackData(messageData->getSequenceNumber());
					ObjControllerMessage const msg(
						m_characterObjectId,
						CM_teleportAck,
						0.0f,
						GameControllerMessageFlags::SEND | 
						GameControllerMessageFlags::RELIABLE |
						GameControllerMessageFlags::DEST_AUTH_SERVER,
						&ackData);
					send(msg, true);
				}
			}
		}
	}
}

//-----------------------------------------------------------------------

const bool GameConnection::tryToSendUnreliable()
{
	bool result = false;
	unsigned long timeMs = Clock::timeMs();
	if(m_sentThisFrame || timeMs > m_timeOfLastUnreliableSendMilliseconds + m_unreliableSendRateMilliseconds)
	{
		result = true;
		m_sentThisFrame = true;
		m_timeOfLastUnreliableSendMilliseconds = timeMs;
	}
	else
	{
		result = false;
	}
	return result;
}

// ----------------------------------------------------------------------

void GameConnection::update()
{
	float frameTime = Clock::frameTime();

	if (m_characterInShip)
	{
		// If we would reach our goal position this frame, pick a new goal.
		// The new goal is a random point around the origin, either in a sphere
		// or cube depending on configuration.

		float const shipSpeed = ConfigSwgLoadClient::getShipSpeed();
		float const shipYawRate = 20.f*PI_OVER_180;
		float const shipPitchRate = 20.f*PI_OVER_180;

		if (m_shipGoalPosition.magnitudeBetweenSquared(m_transform.getPosition_p()) <= sqr(shipSpeed*frameTime))
		{
			if (ConfigSwgLoadClient::getShipLoiterInCube())
				m_shipGoalPosition = Vector::randomCube(ConfigSwgLoadClient::getShipLoiterRadius());
			else
				m_shipGoalPosition = Vector::randomUnit() * Random::randomReal(0.f, ConfigSwgLoadClient::getShipLoiterRadius());
			m_shipGoalPosition.x += ConfigSwgLoadClient::getShipLoiterCenterX();
			m_shipGoalPosition.y += ConfigSwgLoadClient::getShipLoiterCenterY();
			m_shipGoalPosition.z += ConfigSwgLoadClient::getShipLoiterCenterZ();
		}

		Vector const direction_o = m_transform.rotateTranslate_p2l(m_shipGoalPosition);

		float const desiredYaw = direction_o.theta();
		float const maxYaw = shipYawRate * frameTime;
		float const actualYaw = clamp(-maxYaw, desiredYaw, maxYaw);
		m_transform.yaw_l(actualYaw);

		float const desiredPitch = direction_o.phi();
		float const maxPitch = shipPitchRate * frameTime;
		float const actualPitch = clamp(-maxPitch, desiredPitch, maxPitch);
		m_transform.pitch_l(actualPitch);
		
		m_transform.move_l(Vector::unitZ * shipSpeed * frameTime);			
		m_velocity = m_transform.getLocalFrameK_p() * shipSpeed;

		if (m_shipTransformUpdateTimer.updateZero(frameTime))
		{
			bool const reliable = m_shipTransformReliableUpdateTimer.updateZero(frameTime);
			ShipUpdateTransformMessage const msg(
				0,
				m_transform,
				m_velocity,
				0.f,
				0.f,
				0.f,
				getServerSyncStampLong());
			send(msg, reliable);
		}
	}
	else
	{
		Vector v = m_velocity;
		v.normalize();
		Vector face = m_transform.rotate_p2l(v);		
		m_transform.yaw_l(face.theta());
		m_transform.move_l(m_velocity * frameTime);
		if(tryToSendUnreliable())
		{
			if(m_updateTransformTimer.updateZero(frameTime))
			{
				m_velocity = Vector(Random::randomReal(-1.0f, 1.0f), 0, Random::randomReal(-1.0f, 1.0f));
				m_velocity.normalize();
				m_velocity = m_velocity * (Random::randomReal(0.5f, 0.5f));
				MessageQueueDataTransform data(0, ++m_sequenceNumber, m_transform, 0.0f, 0.0f, false);
				ObjControllerMessage message(m_characterObjectId, CM_netUpdateTransform, 0.0f, GameControllerMessageFlags::SEND | GameControllerMessageFlags::DEST_SERVER, &data);
				send(message, true);
			}
		}
	}
	
	if(m_chatEventTimer.updateZero(frameTime))
	{
		chat(chatText[Random::random(m_chatTextCount - 1)]);
		m_chatEventTimer.setExpireTime(Random::randomReal(ConfigSwgLoadClient::getChatEventTimerMin(), ConfigSwgLoadClient::getChatEventTimerMax()));
	}
	
	if(m_socialEventTimer.updateZero(frameTime))
	{
		social();
		m_socialEventTimer.setExpireTime(Random::randomReal(ConfigSwgLoadClient::getSocialEventTimerMin(), ConfigSwgLoadClient::getSocialEventTimerMax()));
	}

	if(m_sentThisFrame)
	{
		// adjust rate
		unsigned long timeMs = Clock::timeMs();
		if (timeMs > m_timeOfLastReceiveMilliseconds + m_unreliableSendRateMilliseconds && m_unreliableSendRateMilliseconds < gs_maxUnreliableSendRateMilliseconds)
		{
			m_unreliableSendRateMilliseconds = m_unreliableSendRateMilliseconds * 2;
		}
		m_sentThisFrame = false;
	}
	m_receiveThisFrame = false;
}

//-----------------------------------------------------------------------

