//===================================================================
//
// SwgCuiSpaceAssignPlayerFormation.cpp
// copyright 2004, sony online entertainment
// tford
//
//===================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiSpaceAssignPlayerFormation.h"

#include "clientGame/ClientCommandQueue.h"
#include "clientGame/ClientObject.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/GroupObject.h"
#include "clientUserInterface/CuiManager.h"
#include "sharedFoundation/Crc.h"
#include "sharedGame/PlayerFormationManager.h"
#include "sharedMath/Vector2d.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "UIButton.h"
#include "UIManager.h"
#include "UIMessage.h"
#include "UIPopupMenu.h"
#include "UIText.h"
#include "UIVolumePage.h"
#include "clientGame/Game.h"

#include <vector>

//===================================================================

namespace SwgCuiSpaceAssignPlayerFormationNamespace
{
	namespace DataProperties
	{
		UILowerString const IsPlayer = UILowerString ("IsPlayer");
		UILowerString const IsShip = UILowerString ("IsShip");
		UILowerString const IsFormationName = UILowerString ("IsFormationName");
		UILowerString const Index = UILowerString ("Index");
		UILowerString const ShipCurrentPlayerIndex = UILowerString ("ShipCurrentPlayerIndex");
		UILowerString const PlayerCurrentShipIndex = UILowerString ("PlayerCurrentShipIndex");
		UILowerString const PlayerName = UILowerString ("PlayerName");
		UILowerString const PlayerNetworkID = UILowerString ("PlayerNetworkID");
		UILowerString const FormationNameIndex = UILowerString ("FormationNameIndex");
	}

	float const cs_timeUntilGroupCheck = 5.0f;
	int const cs_textBufferLength = 32;
	char s_textBuffer[cs_textBufferLength];

	void setTextValue(UIText * const uiText, std::string const & text);
	void setTextValue(UIPage const * uiPage, std::string const & text);
	void setTextValue(UIButton * uiButton, std::string const & text);

	UIBaseObject * duplicateObjectAsPlayer(UIBaseObject const * uiBaseObject);
	UIBaseObject * duplicateObjectAsShip(UIBaseObject const * uiBaseObject);
	UIBaseObject * duplicateObjectAsFormationName(UIBaseObject const * uiBaseObject);

	bool isPlayer(UIBaseObject const * uiBaseObject);
	bool isShip(UIBaseObject const * uiBaseObject);
	bool isFormationName(UIBaseObject const * uiBaseObject);

	int getPlayerIndex(UIBaseObject const * uiBaseObject);
	void setPlayerIndex(UIBaseObject * uiBaseObject, int const index);

	int getPlayerCurrentShipIndex(UIBaseObject const * uiBaseObject);
	void setPlayerCurrentShipIndex(UIBaseObject * uiBaseObject, int const index);

	int getShipIndex(UIBaseObject const * uiBaseObject);
	void setShipIndex(UIBaseObject * uiBaseObject, int const index);

	int getShipCurrentPlayerIndex(UIBaseObject const * uiBaseObject);
	void setShipCurrentPlayerIndex(UIBaseObject * uiBaseObject, int const index);

	void populatePlayerData(UIPage * uiPlayerPage, GroupObject::GroupMember const & data);
	void populateShipData(UIPage * uiPlayerPage, GroupObject::GroupMember const & data);
	void clearShipOfPlayerData(UIPage * uiShipPage);
	void populateShipWithPlayerData(UIPage * uiShipPage, UIPage * uiPlayerPage);

	NetworkId getPlayerNetworkId(UIBaseObject const * uiBaseObject);
	NetworkId getShipNetworkId(UIBaseObject const * uiBaseObject);

	int getFormationNameIndex(UIBaseObject const * const uiBaseObject);
	void setFormationNameIndex(UIBaseObject * uiBaseObject, int const index);

	Vector minimumValues(Vector const & vector1, Vector const & vector2);
	Vector maximumValues(Vector const & vector1, Vector const & vector2);

	Vector2d minimumValues(Vector2d const & vector2d1, Vector2d const & vector2d2);
	Vector2d maximumValues(Vector2d const & vector2d1, Vector2d const & vector2d2);
}

//======================================================================

void SwgCuiSpaceAssignPlayerFormationNamespace::setTextValue(UIText * const uiText, std::string const & text)
{
	if (uiText != 0)
	{
		snprintf(s_textBuffer, cs_textBufferLength, "%s", text.c_str());
		uiText->SetPreLocalized(true);
		uiText->SetLocalText(Unicode::narrowToWide(s_textBuffer));
	}
}

//----------------------------------------------------------------------

void SwgCuiSpaceAssignPlayerFormationNamespace::setTextValue(UIPage const * const uiPage, std::string const & text)
{
	UIText * const uiText = safe_cast<UIText *>(uiPage->GetChild("text"));
	if (uiText != 0)
	{
		setTextValue(uiText, text);
	}
}

//----------------------------------------------------------------------

void SwgCuiSpaceAssignPlayerFormationNamespace::setTextValue(UIButton * const uiButton, std::string const & text)
{
	uiButton->SetText(Unicode::narrowToWide(text.c_str()));
}

//----------------------------------------------------------------------

UIBaseObject * SwgCuiSpaceAssignPlayerFormationNamespace::duplicateObjectAsPlayer(UIBaseObject const * const uiBaseObject)
{
	UIBaseObject * const player = uiBaseObject->DuplicateObject();
	player->SetPropertyBoolean(DataProperties::IsPlayer, true);
	return player;
}

//----------------------------------------------------------------------

UIBaseObject * SwgCuiSpaceAssignPlayerFormationNamespace::duplicateObjectAsShip(UIBaseObject const * const uiBaseObject)
{
	UIBaseObject * const ship = uiBaseObject->DuplicateObject();
	ship->SetPropertyBoolean(DataProperties::IsShip, true);
	return ship;
}

//----------------------------------------------------------------------

UIBaseObject * SwgCuiSpaceAssignPlayerFormationNamespace::duplicateObjectAsFormationName(UIBaseObject const * const uiBaseObject)
{
	UIBaseObject * const formationName = uiBaseObject->DuplicateObject();
	formationName->SetPropertyBoolean(DataProperties::IsFormationName, true);
	return formationName;
}

//----------------------------------------------------------------------

bool SwgCuiSpaceAssignPlayerFormationNamespace::isPlayer(UIBaseObject const * const uiBaseObject)
{
	bool value = false;
	return ((uiBaseObject->GetPropertyBoolean(DataProperties::IsPlayer, value)) && (value));
}

//----------------------------------------------------------------------

bool SwgCuiSpaceAssignPlayerFormationNamespace::isShip(UIBaseObject const * const uiBaseObject)
{
	bool value = false;
	return ((uiBaseObject->GetPropertyBoolean(DataProperties::IsShip, value)) && (value));
}

//----------------------------------------------------------------------

bool SwgCuiSpaceAssignPlayerFormationNamespace::isFormationName(UIBaseObject const * const uiBaseObject)
{
	bool value = false;
	return ((uiBaseObject->GetPropertyBoolean(DataProperties::IsFormationName, value)) && (value));
}

//----------------------------------------------------------------------

int SwgCuiSpaceAssignPlayerFormationNamespace::getPlayerIndex(UIBaseObject const * const uiBaseObject)
{
	FATAL(!isPlayer(uiBaseObject), ("SwgCuiSpaceAssignPlayerFormationNamespace::getPlayerIndex"));

	int index;
	if (uiBaseObject->GetPropertyInteger(DataProperties::Index, index))
	{
		return index;
	}
	return -1;
}

//----------------------------------------------------------------------

void SwgCuiSpaceAssignPlayerFormationNamespace::setPlayerIndex(UIBaseObject * const uiBaseObject, int const index)
{
	FATAL(!isPlayer(uiBaseObject), ("SwgCuiSpaceAssignPlayerFormationNamespace::setPlayerIndex"));

	uiBaseObject->SetPropertyInteger(DataProperties::Index, index);
}

//----------------------------------------------------------------------

int SwgCuiSpaceAssignPlayerFormationNamespace::getPlayerCurrentShipIndex(UIBaseObject const * const uiBaseObject)
{
	FATAL(!isPlayer(uiBaseObject), ("SwgCuiSpaceAssignPlayerFormationNamespace::getPlayerCurrentShipIndex"));

	int index;
	if (uiBaseObject->GetPropertyInteger(DataProperties::PlayerCurrentShipIndex, index))
	{
		return index;
	}
	return -1;
}

//----------------------------------------------------------------------

void SwgCuiSpaceAssignPlayerFormationNamespace::setPlayerCurrentShipIndex(UIBaseObject * const uiBaseObject, int const index)
{
	FATAL(!isPlayer(uiBaseObject), ("SwgCuiSpaceAssignPlayerFormationNamespace::setPlayerCurrentShipIndex"));

	uiBaseObject->SetPropertyInteger(DataProperties::PlayerCurrentShipIndex, index);
}

//----------------------------------------------------------------------

int SwgCuiSpaceAssignPlayerFormationNamespace::getShipIndex(UIBaseObject const * const uiBaseObject)
{
	FATAL(!isShip(uiBaseObject), ("SwgCuiSpaceAssignPlayerFormationNamespace::getShipIndex"));

	int index;
	if (uiBaseObject->GetPropertyInteger(DataProperties::Index, index))
	{
		return index;
	}
	return -1;
}

//----------------------------------------------------------------------

void SwgCuiSpaceAssignPlayerFormationNamespace::setShipIndex(UIBaseObject * const uiBaseObject, int const index)
{
	FATAL(!isShip(uiBaseObject), ("SwgCuiSpaceAssignPlayerFormationNamespace::setShipIndex"));

	uiBaseObject->SetPropertyInteger(DataProperties::Index, index);
}

//----------------------------------------------------------------------

int SwgCuiSpaceAssignPlayerFormationNamespace::getShipCurrentPlayerIndex(UIBaseObject const * const uiBaseObject)
{
	FATAL(!isShip(uiBaseObject), ("SwgCuiSpaceAssignPlayerFormationNamespace::getShipCurrentPlayerIndex"));

	int index;
	if (uiBaseObject->GetPropertyInteger(DataProperties::ShipCurrentPlayerIndex, index))
	{
		return index;
	}
	return -1;
}

//----------------------------------------------------------------------

void SwgCuiSpaceAssignPlayerFormationNamespace::setShipCurrentPlayerIndex(UIBaseObject * const uiBaseObject, int const index)
{
	FATAL(!isShip(uiBaseObject), ("SwgCuiSpaceAssignPlayerFormationNamespace::setShipCurrentPlayerIndex"));

	uiBaseObject->SetPropertyInteger(DataProperties::ShipCurrentPlayerIndex, index);
}

//----------------------------------------------------------------------

void SwgCuiSpaceAssignPlayerFormationNamespace::populatePlayerData(UIPage * const uiPlayerPage, GroupObject::GroupMember const & data)
{
	FATAL(!isPlayer(uiPlayerPage), ("SwgCuiSpaceAssignPlayerFormationNamespace::populatePlayerData"));

	setTextValue(uiPlayerPage, data.second.c_str());
	uiPlayerPage->SetPropertyNarrow(DataProperties::PlayerNetworkID, data.first.getValueString());
	uiPlayerPage->SetPropertyNarrow(DataProperties::PlayerName, data.second);
}

//----------------------------------------------------------------------

void SwgCuiSpaceAssignPlayerFormationNamespace::populateShipData(UIPage * const uiShipPage, GroupObject::GroupMember const & data)
{
	FATAL(!isShip(uiShipPage), ("SwgCuiSpaceAssignPlayerFormationNamespace::populateShipData"));

	setTextValue(uiShipPage, data.second.c_str());
	uiShipPage->SetPropertyNarrow(DataProperties::PlayerNetworkID, data.first.getValueString());
	uiShipPage->SetPropertyNarrow(DataProperties::PlayerName, data.second);
}


//----------------------------------------------------------------------

void SwgCuiSpaceAssignPlayerFormationNamespace::clearShipOfPlayerData(UIPage * const uiShipPage)
{
	FATAL(!isShip(uiShipPage), ("SwgCuiSpaceAssignPlayerFormationNamespace::clearShipOfPlayerData"));

	setTextValue(uiShipPage, "");
	uiShipPage->RemoveProperty(DataProperties::ShipCurrentPlayerIndex);
	uiShipPage->RemoveProperty(DataProperties::PlayerName);
	uiShipPage->RemoveProperty(DataProperties::PlayerNetworkID);
}

//----------------------------------------------------------------------

void SwgCuiSpaceAssignPlayerFormationNamespace::populateShipWithPlayerData(UIPage * const uiShipPage, UIPage * const uiPlayerPage)
{
	FATAL(!isShip(uiShipPage), ("SwgCuiSpaceAssignPlayerFormationNamespace::populateShipWithPlayerData"));
	FATAL(!isPlayer(uiPlayerPage), ("SwgCuiSpaceAssignPlayerFormationNamespace::populateShipWithPlayerData"));

	std::string playerName;
	std::string playerNetworkID;

	uiPlayerPage->GetPropertyNarrow(DataProperties::PlayerName, playerName);
	uiPlayerPage->GetPropertyNarrow(DataProperties::PlayerNetworkID, playerNetworkID);

	setTextValue(uiShipPage, playerName.c_str());
	uiShipPage->SetPropertyNarrow(DataProperties::PlayerName, playerName);
	uiShipPage->SetPropertyNarrow(DataProperties::PlayerNetworkID, playerNetworkID);

	// update the index associations

	setShipCurrentPlayerIndex(uiShipPage, getPlayerIndex(uiPlayerPage));
	setPlayerCurrentShipIndex(uiPlayerPage, getShipIndex(uiShipPage));
}

//----------------------------------------------------------------------

NetworkId SwgCuiSpaceAssignPlayerFormationNamespace::getPlayerNetworkId(UIBaseObject const * const uiBaseObject)
{
	FATAL(!isPlayer(uiBaseObject), ("SwgCuiSpaceAssignPlayerFormationNamespace::getPlayerNetworkId"));
	std::string playerNetworkID;
	uiBaseObject->GetPropertyNarrow(DataProperties::PlayerNetworkID, playerNetworkID);
	return NetworkId(playerNetworkID);
}

//----------------------------------------------------------------------

NetworkId SwgCuiSpaceAssignPlayerFormationNamespace::getShipNetworkId(UIBaseObject const * const uiBaseObject)
{
	FATAL(!isShip(uiBaseObject), ("SwgCuiSpaceAssignPlayerFormationNamespace::getShipNetworkId"));
	std::string playerNetworkID;
	uiBaseObject->GetPropertyNarrow(DataProperties::PlayerNetworkID, playerNetworkID);
	return NetworkId(playerNetworkID);
}

//----------------------------------------------------------------------

int SwgCuiSpaceAssignPlayerFormationNamespace::getFormationNameIndex(UIBaseObject const * const uiBaseObject)
{
	FATAL(!isFormationName(uiBaseObject), ("SwgCuiSpaceAssignPlayerFormationNamespace::getFormationNameIndex"));

	int index;
	if (uiBaseObject->GetPropertyInteger(DataProperties::FormationNameIndex, index))
	{
		return index;
	}
	return -1;
}

//----------------------------------------------------------------------

void SwgCuiSpaceAssignPlayerFormationNamespace::setFormationNameIndex(UIBaseObject * const uiBaseObject, int const index)
{
	FATAL(!isFormationName(uiBaseObject), ("SwgCuiSpaceAssignPlayerFormationNamespace::setFormationNameIndex"));
	uiBaseObject->SetPropertyInteger(DataProperties::FormationNameIndex, index);
}

//----------------------------------------------------------------------

Vector SwgCuiSpaceAssignPlayerFormationNamespace::minimumValues(Vector const & vector1, Vector const & vector2)
{
	float const x = std::min(vector1.x, vector2.x);
	float const y = std::min(vector1.y, vector2.y);
	float const z = std::min(vector1.z, vector2.z);
	return Vector(x, y, z);
}

//----------------------------------------------------------------------

Vector SwgCuiSpaceAssignPlayerFormationNamespace::maximumValues(Vector const & vector1, Vector const & vector2)
{
	float const x = std::max(vector1.x, vector2.x);
	float const y = std::max(vector1.y, vector2.y);
	float const z = std::max(vector1.z, vector2.z);
	return Vector(x, y, z);
}

//----------------------------------------------------------------------

Vector2d SwgCuiSpaceAssignPlayerFormationNamespace::minimumValues(Vector2d const & vector2d1, Vector2d const & vector2d2)
{
	float const x = std::min(vector2d1.x, vector2d2.x);
	float const y = std::min(vector2d1.y, vector2d2.y);
	return Vector2d(x, y);
}

//----------------------------------------------------------------------

Vector2d SwgCuiSpaceAssignPlayerFormationNamespace::maximumValues(Vector2d const & vector2d1, Vector2d const & vector2d2)
{
	float const x = std::max(vector2d1.x, vector2d2.x);
	float const y = std::max(vector2d1.y, vector2d2.y);
	return Vector2d(x, y);
}

//======================================================================

using namespace SwgCuiSpaceAssignPlayerFormationNamespace;

//======================================================================

SwgCuiSpaceAssignPlayerFormation::SwgCuiSpaceAssignPlayerFormation(UIPage& page)
: UIEventCallback()
, CuiMediator("SwgCuiSpaceAssignPlayerFormation",page)
, m_callback(new MessageDispatch::Callback)
, m_done(0)
, m_volumeMembers(0)
, m_volumeFormations(0)
, m_formgraphShips(0)
, m_formationNameText(0)
, m_formationButtonNameSample(0)
, m_playerSample(0)
, m_shipSample(0)
, m_playerData()
, m_playerShips()
, m_formationNames()
, m_currentFormation(0)
, m_elapsedTime(0.0f)
{
	getCodeDataObject(TUIButton,     m_done,              "buttonDone");
	getCodeDataObject(TUIVolumePage, m_volumeMembers,     "volumeMembers");
	getCodeDataObject(TUIVolumePage, m_volumeFormations,  "volumeFormations");
	getCodeDataObject(TUIPage,       m_formgraphShips,    "formationShips");
	getCodeDataObject(TUIText,       m_formationNameText, "formationNameText");

	getCodeDataObject(TUIButton, m_formationButtonNameSample, "formationButtonNameSample");
	getCodeDataObject(TUIPage, m_playerSample, "playerSample");
	getCodeDataObject(TUIPage, m_shipSample, "shipSample");

	registerMediatorObject(*m_done,      true);

	populateFormationNames();

	setState(MS_closeable);
	setState(MS_closeDeactivates);
}

//----------------------------------------------------------------------

SwgCuiSpaceAssignPlayerFormation::~SwgCuiSpaceAssignPlayerFormation()
{
	delete m_callback;
	m_callback = 0;

	m_done = 0;
	m_volumeMembers = 0;
	m_volumeFormations = 0;
	m_formgraphShips = 0;
	m_formationNameText = 0;
	m_formationButtonNameSample = 0;
	m_playerSample = 0;
	m_shipSample = 0;

	m_playerData.clear();
	m_playerShips.clear();
	m_formationNames.clear();
}

//----------------------------------------------------------------------

void SwgCuiSpaceAssignPlayerFormation::performActivate()
{
	CuiMediator::performActivate();	
	CuiManager::requestPointer(true);
	m_callback->connect(*this, &SwgCuiSpaceAssignPlayerFormation::onMembersChanged, static_cast<GroupObject::Messages::MembersChanged*>(0));
	m_callback->connect(*this, &SwgCuiSpaceAssignPlayerFormation::onMemberAdded, static_cast<GroupObject::Messages::MemberAdded*>(0));
	m_callback->connect(*this, &SwgCuiSpaceAssignPlayerFormation::onMemberRemoved, static_cast<GroupObject::Messages::MemberRemoved*>(0));
	m_callback->connect(*this, &SwgCuiSpaceAssignPlayerFormation::onGroupChanged, static_cast<CreatureObject::Messages::GroupChanged*>(0));
	m_callback->connect(*this, &SwgCuiSpaceAssignPlayerFormation::onMemberShipsChanged, static_cast<GroupObject::Messages::MemberShipsChanged*>(0));
	m_callback->connect(*this, &SwgCuiSpaceAssignPlayerFormation::onMemberShipAdded, static_cast<GroupObject::Messages::MemberShipAdded*>(0));
	m_callback->connect(*this, &SwgCuiSpaceAssignPlayerFormation::onMemberShipRemoved, static_cast<GroupObject::Messages::MemberShipRemoved*>(0));

	populateGroupMembersAndShips();
	updateShipLayoutForCurrentFormation();
}

//-------------------------------------------------------------------

void SwgCuiSpaceAssignPlayerFormation::performDeactivate()
{
	CuiMediator::performDeactivate();
	CuiManager::requestPointer(false);	
	m_callback->disconnect(*this, &SwgCuiSpaceAssignPlayerFormation::onMembersChanged, static_cast<GroupObject::Messages::MembersChanged*>(0));
	m_callback->disconnect(*this, &SwgCuiSpaceAssignPlayerFormation::onMemberAdded, static_cast<GroupObject::Messages::MemberAdded*>(0));
	m_callback->disconnect(*this, &SwgCuiSpaceAssignPlayerFormation::onMemberRemoved, static_cast<GroupObject::Messages::MemberRemoved*>(0));
	m_callback->disconnect(*this, &SwgCuiSpaceAssignPlayerFormation::onGroupChanged, static_cast<CreatureObject::Messages::GroupChanged*>(0));
	m_callback->disconnect(*this, &SwgCuiSpaceAssignPlayerFormation::onMemberShipsChanged, static_cast<GroupObject::Messages::MemberShipsChanged*>(0));
	m_callback->disconnect(*this, &SwgCuiSpaceAssignPlayerFormation::onMemberShipAdded, static_cast<GroupObject::Messages::MemberShipAdded*>(0));
	m_callback->disconnect(*this, &SwgCuiSpaceAssignPlayerFormation::onMemberShipRemoved, static_cast<GroupObject::Messages::MemberShipRemoved*>(0));
}

//-------------------------------------------------------------------

void SwgCuiSpaceAssignPlayerFormation::OnButtonPressed(UIWidget * const context)
{
	if (isFormationName(context))
	{
		m_currentFormation = getFormationNameIndex(context);
		updateShipLayoutForCurrentFormation();
	}
	else if(context == m_done)
	{
		closeThroughWorkspace();
		updateServer();
	}
}

//-------------------------------------------------------------------

bool SwgCuiSpaceAssignPlayerFormation::OnMessage(UIWidget * const context, UIMessage const & message)
{
	if (message.Type == UIMessage::DragEnd)
	{
		PageList::const_iterator ii = m_playerShips.begin();
		PageList::const_iterator iiEnd = m_playerShips.end();

		for (; ii != iiEnd; ++ii)
		{
			UIPage * const ship = *ii;
			if ((ship == context) && (message.DragSource != context))
			{
				associatePlayerWithShip(message.DragSource, context);
			}
		}
	}

	return true;
}

//-------------------------------------------------------------------

void SwgCuiSpaceAssignPlayerFormation::onMembersChanged(const GroupObject & /*group*/)
{
	populateGroupMembersAndShips();
	updateShipLayoutForCurrentFormation();
}

//-------------------------------------------------------------------

void SwgCuiSpaceAssignPlayerFormation::onGroupChanged(const CreatureObject & /*creature*/)
{
	populateGroupMembersAndShips();
	updateShipLayoutForCurrentFormation();
}

//-------------------------------------------------------------------

void SwgCuiSpaceAssignPlayerFormation::onMemberAdded(const std::pair<GroupObject *, const GroupMember *> & /*payload*/)
{
	populateGroupMembersAndShips();
	updateShipLayoutForCurrentFormation();
}

//-------------------------------------------------------------------

void SwgCuiSpaceAssignPlayerFormation::onMemberRemoved(const std::pair<GroupObject *, const GroupMember *> & /*payload*/)
{
	populateGroupMembersAndShips();
	updateShipLayoutForCurrentFormation();
}

//-------------------------------------------------------------------

void SwgCuiSpaceAssignPlayerFormation::onMemberShipsChanged(const GroupObject & /*group*/)
{
	populateGroupMembersAndShips();
	updateShipLayoutForCurrentFormation();
}

//-------------------------------------------------------------------

void SwgCuiSpaceAssignPlayerFormation::onMemberShipAdded(const std::pair<GroupObject *, const GroupShipFormationMember *> & /*payload*/)
{
	populateGroupMembersAndShips();
	updateShipLayoutForCurrentFormation();
}

//-------------------------------------------------------------------

void SwgCuiSpaceAssignPlayerFormation::onMemberShipRemoved(const std::pair<GroupObject *, const GroupShipFormationMember *> & /*payload*/)
{
	populateGroupMembersAndShips();
	updateShipLayoutForCurrentFormation();
}

//-------------------------------------------------------------------

void SwgCuiSpaceAssignPlayerFormation::previousFormation()
{
	int const m_formationNamesSize = static_cast<int>(m_formationNames.size());
	if (m_currentFormation == 0)
	{
		m_currentFormation = m_formationNamesSize;
	}

	--m_currentFormation;

	DEBUG_FATAL(!WithinRangeInclusiveInclusive(0, m_currentFormation, m_formationNamesSize - 1), ("SwgCuiSpaceAssignPlayerFormation::previousFormation:WithinRangeInclusiveInclusive"));
}

//-------------------------------------------------------------------

void SwgCuiSpaceAssignPlayerFormation::nextFormation()
{
	++m_currentFormation;

	int const m_formationNamesSize = static_cast<int>(m_formationNames.size());
	if (m_currentFormation == m_formationNamesSize)
	{
		m_currentFormation = 0;
	}

	DEBUG_FATAL(!WithinRangeInclusiveInclusive(0, m_currentFormation, m_formationNamesSize - 1), ("SwgCuiSpaceAssignPlayerFormation::nextFormation:WithinRangeInclusiveInclusive"));
}

//-------------------------------------------------------------------

void SwgCuiSpaceAssignPlayerFormation::associatePlayerWithShip(UIWidget * const playerWidget, UIWidget * const shipWidget) const
{
	UIPage * const player = (isPlayer(playerWidget)) ? safe_cast<UIPage *>(playerWidget) : 0;
	UIPage * const ship = (isShip(shipWidget)) ? safe_cast<UIPage *>(shipWidget) : 0;

	FATAL(!player, ("SwgCuiSpaceAssignPlayerFormation::AssociatePlayerWithShip: playerWidget is not a player."));
	FATAL(!ship, ("SwgCuiSpaceAssignPlayerFormation::AssociatePlayerWithShip: shipWidget is not a ship."));
	FATAL((m_playerData.size() > m_playerShips.size()), ("SwgCuiSpaceAssignPlayerFormation::associatePlayerWithShip: m_playerData <= m_playerShips"));

	int const currentShipIndex = getPlayerCurrentShipIndex(player);

	if ((currentShipIndex >= 0) && (currentShipIndex < static_cast<int>(m_playerShips.size())))
	{
		UIPage * const previousShip = m_playerShips[static_cast<unsigned int>(currentShipIndex)];
		int const currentPlayerIndex = getShipCurrentPlayerIndex(ship);

		clearShipOfPlayerData(previousShip);

		if ((currentPlayerIndex >= 0) && (currentPlayerIndex < static_cast<int>(m_playerData.size())))
		{
			UIPage * const previousPlayer = m_playerData[static_cast<unsigned int>(currentPlayerIndex)];

			populateShipWithPlayerData(previousShip, previousPlayer);
		}
	}

	populateShipWithPlayerData(ship, player);
}

//-------------------------------------------------------------------

void SwgCuiSpaceAssignPlayerFormation::populateFormationNames()
{
	PlayerFormationManager::getFormationNames(m_formationNames);

	if (m_formationButtonNameSample != 0)
	{
		typedef std::vector<UIButton *> Buttons;
		Buttons buttons;
		buttons.reserve(m_formationNames.size());

		int const numberOfFormationNames = static_cast<int>(m_formationNames.size());
		for (int i = 0; i < numberOfFormationNames; ++i)
		{
			UIButton * const button = safe_cast<UIButton *>(duplicateObjectAsFormationName(m_formationButtonNameSample));
			if (button != 0)
			{
				setTextValue(button, m_formationNames[static_cast<unsigned int>(i)]);
				setFormationNameIndex(button, i);
				buttons.push_back(button);
			}
		}

		// clear the other buttons from page
		m_volumeFormations->Clear();

		// add the new buttons in
		Buttons::const_iterator ii = buttons.begin();
		Buttons::const_iterator iiEnd = buttons.end();

		for (; ii != iiEnd; ++ii)
		{
			UIButton * const button = *ii;
			button->SetVisible(true);
			registerMediatorObject(*button, true);
			m_volumeFormations->AddChild(button);
		}

		m_volumeFormations->Link();
	}
}

//-------------------------------------------------------------------

void SwgCuiSpaceAssignPlayerFormation::populateGroupMembersAndShips()
{
	m_playerData.clear();
	m_playerShips.clear();

	// set up the player data
	if (m_playerSample != 0)
	{
		m_playerSample->SetVisible(false);

		Object const * const playerObject = Game::getConstPlayer();
		ClientObject const * const playerClientObject = (playerObject != 0) ? playerObject->asClientObject() : 0;
		CreatureObject const * const playerCreatureObject = (playerClientObject != 0) ? playerClientObject->asCreatureObject() : 0;
		if (playerCreatureObject != 0)
		{
			CachedNetworkId const id(playerCreatureObject->getGroup ());
			GroupObject const * const group = safe_cast<const GroupObject *>(id.getObject());
			if(group != 0)
			{
				GroupObject::GroupMemberVector const & groupMembers = group->getGroupMembers ();
				GroupObject::GroupMemberVector::const_iterator ii = groupMembers.begin();
				GroupObject::GroupMemberVector::const_iterator iiEnd = groupMembers.end();

				int const numberOfGroupMembers = static_cast<int>(groupMembers.size());
				int const maximumShips = PlayerFormationManager::getMaximumFormationGroupMembers();

				for (int i = 0; i < numberOfGroupMembers && i < maximumShips && ii != iiEnd; ++ii, ++i)
				{
					GroupObject::GroupMember const & member = *ii;
					NetworkId const & shipId = group->getShipFromMember(member.first);

					if (shipId != NetworkId::cms_invalid)
					{
						UIPage * const player = safe_cast<UIPage *>(duplicateObjectAsPlayer(m_playerSample));
						setPlayerIndex(player, i);
						m_playerData.push_back(player);

						populatePlayerData(player, member);
					}
				}
			}
		}

		// clear the samples
		m_volumeMembers->Clear();

		// add the new controls
		PageList::const_iterator ii = m_playerData.begin();
		PageList::const_iterator iiEnd = m_playerData.end();
		for (; ii != iiEnd; ++ii)
		{
			UIPage * const player = *ii;	
			player->SetVisible(true);
			registerMediatorObject(*player, true);
			m_volumeMembers->AddChild(player);
		}

		m_volumeMembers->Link();

	}

	// set up the ships
	if (m_shipSample != 0)
	{
		m_shipSample->SetVisible(false);

		// don't need groups...

		Object const * const playerObject = Game::getConstPlayer();
		ClientObject const * const playerClientObject = (playerObject != 0) ? playerObject->asClientObject() : 0;
		CreatureObject const * const playerCreatureObject = (playerClientObject != 0) ? playerClientObject->asCreatureObject() : 0;
		if (playerCreatureObject != 0)
		{
			CachedNetworkId const id(playerCreatureObject->getGroup ());
			GroupObject const * const group = safe_cast<const GroupObject *>(id.getObject());
			if(group != 0)
			{
				int const maximumShips = PlayerFormationManager::getMaximumFormationGroupMembers();
				for (int i = 0; i < maximumShips; ++i)
				{
					UIPage * const ship = safe_cast<UIPage *>(duplicateObjectAsShip(m_shipSample));
					setShipIndex(ship, i);
					setTextValue(ship, "");
					m_playerShips.push_back(ship);
				}
			}
		}

		// clear the samples
		m_formgraphShips->Clear();

		// add the new controls
		PageList::const_iterator ii = m_playerShips.begin();
		PageList::const_iterator iiEnd = m_playerShips.end();
		for (; ii != iiEnd; ++ii)
		{
			UIPage * const ship = *ii;
			ship->SetVisible(true);
			registerMediatorObject(*ship, true);
			m_formgraphShips->AddChild(ship);

		}

		m_formgraphShips->Link();
	}

	{
		// associate the players with a ship
		unsigned int const numberOfPlayerData = m_playerData.size();
		for (unsigned int i = 0; i < numberOfPlayerData; ++i)
		{
			UIPage * const player = m_playerData[i];	
			UIPage * const ship = m_playerShips[i];	
			associatePlayerWithShip(player, ship);
		}
	}

}

//-------------------------------------------------------------------

void SwgCuiSpaceAssignPlayerFormation::updateShipLayoutForCurrentFormation()
{
	// set the name
	std::string const & currentFormationName = m_formationNames[static_cast<unsigned int>(m_currentFormation)];
	StringId const currentFormationNameID("space/space_formation", currentFormationName);
	m_formationNameText->SetPreLocalized(false);
	m_formationNameText->SetText(currentFormationNameID.localize());

	uint32 const currentFormationCrc(Crc::normalizeAndCalculate(currentFormationName.c_str()));

	// cache all of the offsets
	typedef std::vector<Vector2d> CachedOffsets;
	CachedOffsets cachedOffsets;

	int const maximumShips = std::min(PlayerFormationManager::getMaximumFormationGroupMembers(), static_cast<int>(m_playerShips.size()));
	cachedOffsets.reserve(static_cast<unsigned int>(maximumShips));
	{
		for (int i = 0; i < maximumShips; ++i)
		{
			Vector2d offset;
			IGNORE_RETURN(PlayerFormationManager::getPositionOffset(currentFormationCrc, i, offset));
			cachedOffsets.push_back(offset);
		}
	}

	// find the min and the max for scaling	
	Vector2d minimumOffset(REAL_MAX, REAL_MAX);
	Vector2d maximumOffset(-minimumOffset);
	{
		for (int i = 0; i < maximumShips; ++i)
		{
			Vector2d const & offset = cachedOffsets[static_cast<unsigned int>(i)];
			minimumOffset = minimumValues(minimumOffset, offset);
			maximumOffset = maximumValues(maximumOffset, offset);
		}

		// going from corner to corner and then projecting along
		// xyz111 isn't the exact bounds but it works well enough
		float const spanFromCenter = ((maximumOffset - minimumOffset).magnitude() * 0.5f);
		maximumOffset = Vector2d(1.0f, 1.0f) * -spanFromCenter;
		minimumOffset = -maximumOffset;
	}

	// scale a border
	minimumOffset *= 1.10f;
	maximumOffset *= 1.10f;

	// convert each offset into view space and position ship
	UIScalar const width = m_formgraphShips->GetWidth();
	UIScalar const height = m_formgraphShips->GetHeight();
	Vector2d const spanOfOffsets(maximumOffset - minimumOffset);

	{
		for (int i = 0; i < maximumShips; ++i)
		{
			Vector2d const & offset = cachedOffsets[static_cast<unsigned int>(i)];
			UIPage * const ship = m_playerShips[static_cast<unsigned int>(i)];

			// make sure it's not zero...
			UIScalar const x = (spanOfOffsets.x != 0.0f) ? static_cast<UIScalar>(static_cast<float>(width) * (1.0f - (offset.x - minimumOffset.x) / spanOfOffsets.x)) : 0;
			UIScalar const y = (spanOfOffsets.y != 0.0f) ? static_cast<UIScalar>(static_cast<float>(height) * (offset.y - minimumOffset.y) / spanOfOffsets.y) : 0;

			ship->SetLocation(x, y, true);
		}
	}
}

//-------------------------------------------------------------------

void SwgCuiSpaceAssignPlayerFormation::updateServer() const
{
	std::string const & formationName = m_formationNames[static_cast<unsigned int>(m_currentFormation)];
	ClientCommandQueue::enqueueCommand ("setFormation", NetworkId::cms_invalid, Unicode::narrowToWide(formationName));

	{
		int const numberOfPlayerData = static_cast<int>(m_playerData.size());
		int const numberOfPlayerShips = static_cast<int>(m_playerShips.size());

		for (int i = 0; i < numberOfPlayerData; ++i)
		{
			UIPage const * const player = m_playerData[static_cast<unsigned int>(i)];
			if (player != 0)
			{
				NetworkId const target(getPlayerNetworkId(player));
				int const currentShipIndex = getPlayerCurrentShipIndex(player);
				if ((currentShipIndex >= 0) && (currentShipIndex < numberOfPlayerShips))
				{
					snprintf(s_textBuffer, cs_textBufferLength, "%d", currentShipIndex);
					ClientCommandQueue::enqueueCommand("setFormationSlot", target, Unicode::narrowToWide(s_textBuffer));
				}
			}
		}
	}
}

//======================================================================

