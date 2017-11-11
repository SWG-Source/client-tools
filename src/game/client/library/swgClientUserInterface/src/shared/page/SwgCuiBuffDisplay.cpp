//======================================================================
//
// SwgCuiBuffDisplay.cpp
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiBuffDisplay.h"

#include "clientGame/ClientBuffManager.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/PlayerObject.h"
#include "sharedGame/Buff.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "UIEffector.h"
#include "UIImage.h"
#include "UIImageStyle.h"
#include "UIManager.h"
#include "UIVolumePage.h"

//======================================================================

namespace SwgCuiBuffDisplayNamespace
{
	const float UPDATE_FREQUENCY = 2.5;

	const int NUM_BUFFS_DEBUFFS = 10;
	
	static const UIColor s_debuffColor = UIColor(255, 0, 0);
	static const UIColor s_buffColor = UIColor(255, 0, 255);
	
	static const UILowerString BUFF_TIMESTAMP_PROPERTY = UILowerString("BuffTimestamp");
	static const int PLAYER_BUFF_BLINK_TIME = 10;
}

using namespace SwgCuiBuffDisplayNamespace;

SwgCuiBuffDisplay::SwgCuiBuffDisplay(UIPage & page) :
CuiMediator("SwgCuiBuffDisplay", page),
UIEventCallback(),
m_objectId(),
m_callback(new MessageDispatch::Callback),
m_volume(0),
m_blank(0),
m_internalTimer(0.0f),
m_effectorBlink(NULL)
{
	getCodeDataObject (TUIVolumePage, m_volume, "volume", true);
	getCodeDataObject (TUIImageStyle, m_blank, "blank", true);	
	getCodeDataObject (TUIEffector, m_effectorBlink,      "effectorBlink", true);
}

//----------------------------------------------------------------------

SwgCuiBuffDisplay::~SwgCuiBuffDisplay ()
{
	delete m_callback;
	m_callback = 0;
	
	m_volume = 0;
}

// ----------------------------------------------------------------------

void SwgCuiBuffDisplay::performActivate()
{
	setIsUpdating(true);
}

//-----------------------------------------------------------------------------

void SwgCuiBuffDisplay::performDeactivate()
{
	setIsUpdating(false);
}

//-----------------------------------------------------------------------------

void SwgCuiBuffDisplay::update(float deltaTimeSecs)
{
	m_internalTimer += deltaTimeSecs;
	if (m_internalTimer < UPDATE_FREQUENCY)
		return;
	m_internalTimer = 0.0f;

	const CreatureObject * const creatureObject = dynamic_cast<const CreatureObject *>(m_objectId.getObject());
	if (!creatureObject)
		return;
	std::vector<Buff> creatureBuffs;
	creatureObject->getBuffs(creatureBuffs);
	
	for (int j = 0; j < NUM_BUFFS_DEBUFFS * 2; j++)
	{
		UIWidget * wid = m_volume->FindCell(j);
		UIImage * img = dynamic_cast<UIImage *>(wid);
		if(!img)
			return;
		img->SetStyle(m_blank);
	}

	int curBuffIndex = -1;
	int curDebuffIndex = -1;
	for (std::vector<Buff>::const_iterator i = creatureBuffs.begin(); i != creatureBuffs.end(); ++i)
	{
		const Buff & buff = *i;
		if (!ClientBuffManager::getBuffIsGroupVisible(buff.m_nameCrc))
			continue;
		UIImageStyle * imageStyle = ClientBuffManager::getBuffIconStyle(buff.m_nameCrc);
		bool isDebuff = ClientBuffManager::getBuffIsDebuff(buff.m_nameCrc);
		int targetIndex = isDebuff ? (NUM_BUFFS_DEBUFFS + (++curDebuffIndex)) : (++curBuffIndex);
		if ((curBuffIndex >= NUM_BUFFS_DEBUFFS) || (curDebuffIndex >= NUM_BUFFS_DEBUFFS))
		{
			WARNING(true, ("Character has too many buffs or debuffs to display in the UI\n"));
			continue;
		}
		UIWidget * wid = m_volume->FindCell(targetIndex);
		UIImage * img = dynamic_cast<UIImage *>(wid);
		img->SetStyle(imageStyle);		
		
		int timeLeft = 0;
		if (buff.m_timestamp > creatureObject->getPlayedTime())
			timeLeft = buff.m_timestamp - creatureObject->getPlayedTime();
		
		Unicode::String tooltipStr;
		Unicode::String result;
		ClientBuffManager::getBuffDescription(buff, tooltipStr);
		if (timeLeft >= 0)
		{
			if (m_effectorBlink && (timeLeft <= PLAYER_BUFF_BLINK_TIME))
				UIManager::gUIManager().ExecuteEffector(m_effectorBlink, img, false);	
			ClientBuffManager::addTimestampToBuffDescription(tooltipStr, timeLeft, result);
		}
		img->SetLocalTooltip(result);
		
		if (imageStyle)
		{
			img->SetColor(UIColor::white);
		}
	}
		
}

// ----------------------------------------------------------------------

void SwgCuiBuffDisplay::setTarget(CreatureObject * creature)
{
	if (creature)
		setTarget(creature->getNetworkId());
	else
		setTarget(NetworkId::cms_invalid);
}
//-----------------------------------------------------------------------------

void SwgCuiBuffDisplay::setTarget(const NetworkId & id)
{
	if (id != m_objectId)
	{		
		m_objectId = id;		
		update(1000.0f);		
	}
}

//======================================================================
