//======================================================================
//
// SwgCuiPlayerQuestConversation.cpp
// copyright(c) 2009 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiPlayerQuestConversation.h"

#include "clientAudio/Audio.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/ProsePackageManagerClient.h"
#include "clientGame/ShipObject.h"
#include "clientGame/ShipStation.h"
#include "clientGame/SpaceTargetBracketOverlay.h"
#include "clientObject/InteriorEnvironmentBlock.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientSkeletalAnimation/Skeleton.h"
#include "clientTerrain/GroundEnvironment.h"
#include "clientUserInterface/CuiConversationManager.h"
#include "clientUserInterface/CuiSpatialChatManager.h"
#include "clientUserInterface/CuiStringIds.h"
#include "clientUserInterface/CuiStringVariablesData.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "clientUserInterface/CuiWidget3dObjectListViewer.h"
#include "clientUserInterface/CuiWorkspace.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/Timer.h"
#include "sharedGame/ProsePackage.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/MessageQueueSpatialChat.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/CachedNetworkId.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedUtility/CallbackReceiver.h"
#include "swgClientUserInterface/SwgCuiChatWindow.h"
#include "swgClientUserInterface/SwgCuiHud.h"
#include "swgClientUserInterface/SwgCuiHudFactory.h"

#include "UIButton.h"
#include "UIDataSource.h"
#include "UIPopupMenu.h"
#include "UIText.h"
#include "UIUtils.h"

#include <set>
#include <vector>

//======================================================================

namespace SwgCuiPlayerQuestConversationNamespace
{
	//------------------------------------------------------------------

	int const cms_buffer = 8;
	float const cms_minimumDisplayTime = 17.0f;

	char const * const cms_faceFocusBoneName = "face_focus";
	char const * const cms_headBoneName = "head";

	//------------------------------------------------------------------

	struct ConversationNode
	{
		enum Type { cn_timed_message, cn_conversation, cn_untimed_message, cn_taunt };

		ConversationNode();
		~ConversationNode();

		void setType(Type const type);
		Type const & getType() const;

		void setTarget(NetworkId const & id);
		NetworkId const & getTarget() const;

		void generateId();
		uint32 getId() const;

		void setResponses(SwgCuiPlayerQuestConversation::StringVector const & responses);
		SwgCuiPlayerQuestConversation::StringVector const & getResponses() const;

		void setOverrideAppearanceTemplate(uint32 appearanceCrc);
		uint32 getOverrideAppearanceTemplate() const;

		void setExpireTime(float const time);
		float getExpireTime() const;
		bool getExpireTimeSet() const;

		// use this to set an absolute expiration time.
		// this is currently used on taunts to prevent them from stacking.
		void setTimestamp(double time);
		double getTimetamp() const;

		void setSoundEffect(std::string const & soundEffect);
		std::string const & getSoundEffect() const;

		void setText(Unicode::String const & text);
		Unicode::String const & getText() const;

		bool operator <(ConversationNode const & rhs) const;
		bool operator >(ConversationNode const & rhs) const;
		bool operator ==(ConversationNode const & rhs) const;

	private:
		static uint32 m_conversationNodeId;

		uint32 m_id;
		Type m_type;

		CachedNetworkId m_targetId;
		SwgCuiPlayerQuestConversation::StringVector m_responses;
		uint32 m_appearanceCrc;
		std::string m_soundEffect;
		Unicode::String m_text;

		float m_expireTime;
		bool m_expireTimeSet;

		double m_timestamp;
	};

	//------------------------------------------------------------------

	typedef std::set<ConversationNode> ConversationNodePrioritySet;

	//------------------------------------------------------------------

	ConversationNodePrioritySet s_conversationNodeSet;
	ConversationNode s_conversationNode;

	uint32 ConversationNode::m_conversationNodeId = 0;
	bool s_conversationActive = false;
	bool s_conversationHasFocus = false;

	void queuePush(ConversationNode const & conversationNode)
	{
		s_conversationNodeSet.insert(conversationNode);
	}

	bool queueEmpty()
	{
		return s_conversationNodeSet.empty();
	}

	void queuePop()
	{
		if (!s_conversationNodeSet.empty())
		{
			s_conversationNodeSet.erase(s_conversationNodeSet.begin());
		}
	}

	ConversationNode const & queueTop()
	{
		DEBUG_FATAL(s_conversationNodeSet.empty(), ("Trying to retrieve conversion from empty list.!\n"));
		ConversationNodePrioritySet::const_iterator const it = s_conversationNodeSet.begin();
		ConversationNode const & node = *it;
		return node;
	}

	//------------------------------------------------------------------

	Timer s_timedMessageTimer;

	//------------------------------------------------------------------

	SoundId * s_lastSound = NULL;

	void playSound(std::string const & sound)
	{
		if (!sound.empty() && sound != "null") 
		{
			Audio::stopSound(*s_lastSound, 1.0f);
			*s_lastSound = Audio::playSound(sound.c_str());
		}
	}
}

using namespace SwgCuiPlayerQuestConversationNamespace;

//======================================================================

void SwgCuiPlayerQuestConversation::install()
{
	s_lastSound = new SoundId();
	ExitChain::add(SwgCuiPlayerQuestConversation::remove, "SwgCuiPlayerQuestConversation::remove");
}

//----------------------------------------------------------------------

void SwgCuiPlayerQuestConversation::remove()
{
	delete s_lastSound;
	s_lastSound = NULL;
}

//======================================================================

ConversationNode::ConversationNode() :
m_id(m_conversationNodeId++),
m_type(cn_taunt),
m_targetId(),
m_responses(),
m_appearanceCrc(0),
m_soundEffect(),
m_text(),
m_expireTime(0.0f),
m_expireTimeSet(false),
m_timestamp(0.0)
{
}

//----------------------------------------------------------------------

ConversationNode::~ConversationNode()
{
}

//----------------------------------------------------------------------

void ConversationNode::setType(Type const type)
{
	m_type = type;
}

//----------------------------------------------------------------------

ConversationNode::Type const & ConversationNode::getType() const
{
	return m_type;
}

//----------------------------------------------------------------------

void ConversationNode::setTarget(NetworkId const & id)
{
	m_targetId = id;
}

//----------------------------------------------------------------------

NetworkId const & ConversationNode::getTarget() const
{
	return m_targetId;
}

//----------------------------------------------------------------------

void ConversationNode::generateId()
{
	m_id = m_conversationNodeId++;
}

//----------------------------------------------------------------------

uint32 ConversationNode::getId() const
{
	return m_id;
}

//----------------------------------------------------------------------

void ConversationNode::setResponses(SwgCuiPlayerQuestConversation::StringVector const & responses)
{
	m_responses = responses;
}

//----------------------------------------------------------------------

SwgCuiPlayerQuestConversation::StringVector const & ConversationNode::getResponses() const
{
	return m_responses;
}

//----------------------------------------------------------------------

void ConversationNode::setOverrideAppearanceTemplate(uint32 appearanceCrc)
{
	m_appearanceCrc = appearanceCrc;
}

//----------------------------------------------------------------------

uint32 ConversationNode::getOverrideAppearanceTemplate() const
{
	return m_appearanceCrc;
}

//----------------------------------------------------------------------

void ConversationNode::setExpireTime(float const time)
{
	m_expireTime = time;
	m_expireTimeSet = (m_expireTime > 0.0f);
}

//----------------------------------------------------------------------

float ConversationNode::getExpireTime() const
{
	return m_expireTime;
}

//----------------------------------------------------------------------

bool ConversationNode::getExpireTimeSet() const
{
	return m_expireTimeSet;
}

//----------------------------------------------------------------------

void ConversationNode::setTimestamp(double time)
{
	m_timestamp = time;
}

//----------------------------------------------------------------------

double ConversationNode::getTimetamp() const
{
	return m_timestamp;
}

//----------------------------------------------------------------------

void ConversationNode::setSoundEffect(std::string const & soundEffect)
{
	m_soundEffect = soundEffect;
}

//----------------------------------------------------------------------

std::string const & ConversationNode::getSoundEffect() const
{
	return m_soundEffect;
}

//----------------------------------------------------------------------

void ConversationNode::setText(Unicode::String const & text)
{
	m_text = text;
}

//----------------------------------------------------------------------

Unicode::String const & ConversationNode::getText() const
{
	return m_text;
}

//----------------------------------------------------------------------

bool ConversationNode::operator <(ConversationNode const & rhs) const
{
	if (m_type<rhs.m_type)
	{
		return true;
	}
	else if (m_type>rhs.m_type)
	{
		return false;
	}
	else
	{
		return m_id<rhs.m_id;
	}
}

//----------------------------------------------------------------------

bool ConversationNode::operator >(ConversationNode const & rhs) const
{
	if (m_type>rhs.m_type)
	{
		return true;
	}
	else if (m_type<rhs.m_type)
	{
		return false;
	}
	else
	{
		return m_id>rhs.m_id;
	}
}

//----------------------------------------------------------------------

bool ConversationNode::operator ==(ConversationNode const & rhs) const
{
	return m_type==rhs.m_type && m_id==rhs.m_id;
}

//======================================================================

SwgCuiPlayerQuestConversation::SwgCuiPlayerQuestConversation(UIPage & page) :
SwgCuiLockableMediator("SwgCuiPlayerQuestConversation", page),
m_callback(new MessageDispatch::Callback),
m_targetViewer(NULL),
m_text(NULL),
m_popup(NULL),
m_viewerPage(NULL),
m_textPage(NULL),
m_currentResponses(),
m_createdViewerObject(NULL),
m_currentNodeId(static_cast<uint32>(-1))
{

	UIWidget * wid = 0;
	getCodeDataObject(TUIWidget, wid, "viewer");
	m_targetViewer = safe_cast<CuiWidget3dObjectListViewer *>(wid);
	registerMediatorObject(*m_targetViewer, true);

	getCodeDataObject(TUIText, m_text, "text");
	m_text->Clear();
	getCodeDataObject(TUIPopupMenu, m_popup, "buttons");
	m_popup->ClearItems();
	m_popup->SetPopupButtonBehavior(false);

	getCodeDataObject(TUIPage, m_viewerPage, "viewerpage");
	getCodeDataObject(TUIPage, m_textPage, "textpage");

	m_callback->connect (*this, &SwgCuiPlayerQuestConversation::onTargetChanged, static_cast<CuiConversationManager::Messages::TargetChanged *>(0));
	m_callback->connect (*this, &SwgCuiPlayerQuestConversation::onResponsesChanged, static_cast<CuiConversationManager::Messages::ResponsesChanged *>(0));
	m_callback->connect (*this, &SwgCuiPlayerQuestConversation::onConversationEnded, static_cast<CuiConversationManager::Messages::ConversationEnded *>(0));

	onTargetChanged(false);

	setStickyVisible(true);

	//we don't want a glow rect around this widget
	setShowFocusedGlowRect(false);

	setState(MS_closeable);
	setState(MS_closeDeactivates);

	registerMediatorObject(getPage(), true);
}

//----------------------------------------------------------------------

SwgCuiPlayerQuestConversation::~SwgCuiPlayerQuestConversation()
{
	m_callback->disconnect (*this, &SwgCuiPlayerQuestConversation::onTargetChanged, static_cast<CuiConversationManager::Messages::TargetChanged *>(0));
	m_callback->disconnect (*this, &SwgCuiPlayerQuestConversation::onResponsesChanged, static_cast<CuiConversationManager::Messages::ResponsesChanged *>(0));
	m_callback->disconnect (*this, &SwgCuiPlayerQuestConversation::onConversationEnded, static_cast<CuiConversationManager::Messages::ConversationEnded *>(0));

	delete m_callback;
	m_callback = NULL;

	m_targetViewer = NULL;
	m_text = NULL;
	m_popup = NULL;

	delete m_createdViewerObject;
	m_createdViewerObject = NULL;
}

//----------------------------------------------------------------------

void SwgCuiPlayerQuestConversation::performActivate()
{
	setIsUpdating(true);
	m_popup->AddCallback(this);
}

//----------------------------------------------------------------------

void SwgCuiPlayerQuestConversation::performDeactivate()
{
	setIsUpdating(false);
	m_popup->RemoveCallback(this);
	SpaceTargetBracketOverlay::setCommTarget(NetworkId::cms_invalid);
	s_conversationNodeSet.clear();
	s_conversationActive = false;
}

//----------------------------------------------------------------------

void SwgCuiPlayerQuestConversation::onTargetChanged(bool const &)
{
	CachedNetworkId const & nid = CuiConversationManager::getTarget();
	NetworkId n = NetworkId(nid.getValue());
	s_conversationActive = false;

	if (Game::isHudSceneTypeSpace()) 
	{
		if (n != NetworkId::cms_invalid)
		{
			s_conversationNode.setType(ConversationNode::cn_conversation);

			s_conversationNode.generateId();
			s_conversationNode.setExpireTime(0);
			s_conversationNode.setTarget(n);

			uint32 const appearanceOverrideTemplateCrc = CuiConversationManager::getAppearanceOverrideTemplateCrc();
			s_conversationNode.setOverrideAppearanceTemplate(appearanceOverrideTemplateCrc);

			std::string const & soundEffect = CuiConversationManager::getSoundEffect();
			s_conversationNode.setSoundEffect(soundEffect);

			StringVector const & response = CuiConversationManager::getResponses();
			s_conversationNode.setResponses(response);

			s_conversationNode.setText(CuiConversationManager::getLastMessage());

			queuePush(s_conversationNode);

			playSound(soundEffect);

			s_conversationActive = true;

			CuiConversationManager::start(n);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiPlayerQuestConversation::onConversationEnded(bool const &)
{
	s_conversationActive = false;
}

//-----------------------------------------------------------------

void SwgCuiPlayerQuestConversation::onResponsesChanged(const bool &)
{
	if (s_conversationActive)
	{
		s_conversationNode.setResponses(CuiConversationManager::getResponses());
		s_conversationNode.setText(CuiConversationManager::getLastMessage());
		s_conversationNode.generateId();

		queuePush(s_conversationNode);
	}
}

//----------------------------------------------------------------------

bool SwgCuiPlayerQuestConversation::setViewerTarget(NetworkId const & target, uint32 appearanceOverrideTemplateCrc)
{
	if (!m_targetViewer)
		return false;

	m_targetViewer->clearObjects();

	Object * objectForViewer = NULL;
	bool setupViewerForCreatureObject = false;

	bool result = false;
	bool faceFocusFound = false;
	bool headFocusFound = false;

	if (0 != appearanceOverrideTemplateCrc)
	{
		SharedObjectTemplate const * const ot = safe_cast<SharedObjectTemplate const *>(ObjectTemplateList::fetch(appearanceOverrideTemplateCrc));
		if (ot)
		{
			if (m_createdViewerObject)
				delete m_createdViewerObject;
			m_createdViewerObject = ot->createObject();
			ClientObject * const co = safe_cast<ClientObject *>(m_createdViewerObject);
			if (co)
			{
				co->endBaselines();
				objectForViewer = co;
				setupViewerForCreatureObject = true;
			}
			ot->releaseReference();
		}
		// We're hooking up the viewer to a new object which is only for this, so it needs to get altered by the viewer
		m_targetViewer->setAlterObjects(true);
	}
	else
	{
		Object * const o = NetworkIdManager::getObjectById(target);
		ClientObject * const co = o ? o->asClientObject() : NULL;
		CreatureObject * const targetCreature = co ? co->asCreatureObject() : NULL;
		if (targetCreature)
		{
			objectForViewer = targetCreature;
			setupViewerForCreatureObject = true;
		}
		else if (co)
		{
			objectForViewer = co;
			setupViewerForCreatureObject = false;
		}
		// We're hooking up the viewer to an object which is already responsible for altering itself
		m_targetViewer->setAlterObjects(false);
	}

	if (objectForViewer)
	{
		ClientObject * const co = objectForViewer->asClientObject();
		CreatureObject * const creature = co ? co->asCreatureObject() : NULL;
		if (creature)
		{
			Appearance * const app = creature->getAppearance();
			if (app)
			{
				SkeletalAppearance2 * const skelApp = app->asSkeletalAppearance2();
				if (skelApp)
				{
					skelApp->setDetailLevel(0);
					skelApp->setUserControlledDetailLevel(true);

					Skeleton const & skeleton = skelApp->getSkeleton(0);

					Transform const * const boneTransform = skeleton.findTransform(ConstCharCrcString(cms_faceFocusBoneName));
					if (boneTransform)
						faceFocusFound = true;

					Transform const * const headTransform = skeleton.findTransform(ConstCharCrcString(cms_headBoneName));
					if (headTransform) 
						headFocusFound = true;
				}
			}
		}

		m_targetViewer->addObject(*objectForViewer);
		if (setupViewerForCreatureObject)
		{
			if (faceFocusFound)
			{
				m_targetViewer->setCameraLookAtBone(cms_faceFocusBoneName);
				m_targetViewer->setCameraFitBone(true);
			}
			else if (headFocusFound)
			{
				m_targetViewer->setCameraLookAtBone(cms_headBoneName);
				m_targetViewer->setCameraFitBone(true);
			}

			m_targetViewer->setCameraYaw(PI);
			m_targetViewer->setCameraPitch(0.0f);
			m_targetViewer->setCameraForceTarget(true);
			m_targetViewer->recomputeZoom();
		}
		else
		{
			m_targetViewer->setCameraPitch(0.3f);
			m_targetViewer->setCameraForceTarget(true);
			m_targetViewer->recomputeZoom();
			m_targetViewer->setRotateSpeed(0.2f);
		}
		result = true;
	}

	// Setup the environment texture...
	if (m_targetViewer) 
	{
		InteriorEnvironmentBlock const * const interior = GroundEnvironment::getInstance().getCurrentInteriorEnvironmentBlock();
		if (interior)
		{
			Texture const * const texture = interior->getEnvironmentTexture();
			if (texture) 
			{
				m_targetViewer->setEnvironmentTexture(texture);
			}
		}
	}

	return result;
}

//----------------------------------------------------------------------

void SwgCuiPlayerQuestConversation::setText(Unicode::String const & text)
{
	m_text->SetPreLocalized(true);
	m_text->SetLocalText(text);
}

//----------------------------------------------------------------------

Unicode::String const & SwgCuiPlayerQuestConversation::getText() const
{
	return m_text->GetLocalText();
}

//----------------------------------------------------------------------

void SwgCuiPlayerQuestConversation::update(float deltaTimeSecs)
{
	CuiMediator::update(deltaTimeSecs);

	CreatureObject * const player = Game::getPlayerCreature();
	if (player && !queueEmpty()) 
	{
		bool const isSpaceScene = Game::isHudSceneTypeSpace();

		// update the command queue.
		while(!queueEmpty())
		{
			ConversationNode const & currentNode = queueTop();
			s_conversationHasFocus = currentNode.getType() == ConversationNode::cn_conversation;

			// special case code for conversations
			if (s_conversationHasFocus && ((currentNode.getId() != s_conversationNode.getId()) || !s_conversationActive || !isSpaceScene))
			{
				// set this node as the current node.
				m_currentNodeId = currentNode.getId();

				// acquire a new conversation from the queue.
				queuePop();
				continue;
			}

			// check to see if we have a new convo node.
			if (m_currentNodeId != currentNode.getId()) 
			{
				// setup all the goodness.
				m_currentNodeId = currentNode.getId();

				// check to see if the message has expired.
				if (currentNode.getTimetamp() > 0.0 && Clock::getCurrentTime() > currentNode.getTimetamp()) 
				{
					// time has expired, get a new node.
					queuePop();
					continue;
				}

				// setup the convo stuff.
				setViewerTarget(currentNode.getTarget(), currentNode.getOverrideAppearanceTemplate());
				updateResponses(currentNode.getResponses());
				setText(currentNode.getText());

				playSound(currentNode.getSoundEffect());

				// timer setup.
				if (currentNode.getExpireTimeSet()) 
				{
					s_timedMessageTimer.setExpireTime(currentNode.getExpireTime());
					s_timedMessageTimer.reset();
				}

				// space comm stuff.
				SpaceTargetBracketOverlay::setCommTarget(currentNode.getTarget());

				// auto target taunter.
				if (player->getLookAtTarget() == NetworkId::cms_invalid)
				{
					Object const * const taunterObject = NetworkIdManager::getObjectById(currentNode.getTarget());
					ClientObject const * const taunterClientObject = taunterObject ? taunterObject->asClientObject() : NULL;
					ShipObject const * const taunterShip = taunterClientObject ? taunterClientObject->asShipObject() : NULL;
					if (taunterShip) 
					{
						player->setLookAtTarget(currentNode.getTarget());
					}
				}

				// pack the page.
				getPage().Pack();
			}
			else
			{
				// update current node.
				if (currentNode.getExpireTimeSet() && s_timedMessageTimer.updateZero(deltaTimeSecs)) 
				{
					// the time has come!
					queuePop();
					continue;
				}
			}

			// exit.
			break;
		}
	}
	else
	{
		closeNextFrame();
	}

	if (m_popup) 
	{
		UIString text;

		UIButton const * const button = UI_ASOBJECT(UIButton, m_popup->GetHoverButton());
		if (button) 
		{
			button->GetText(text);
		}

		getPage().SetTooltip(text);
		getPage().SetTooltipDelay(true);
	}

	//resize to fit buttons as necessary
	UISize scrollExtent;
	getPage().GetScrollExtent(scrollExtent);
	getPage().SetSize(scrollExtent);
}

//----------------------------------------------------------------------

void SwgCuiPlayerQuestConversation::updateResponses(StringVector const & strings)
{
	clearResponses();

	m_popup->SetMinimumDesiredWidth(getPage().GetWidth());

	m_currentResponses = strings;

	if (!strings.empty())
	{
		int i = 0;
		for (CuiConversationManager::StringVector::const_iterator it = strings.begin(); it != strings.end(); ++it, ++i)
		{
			const Unicode::String & response = *it;
			char buf [64];
			_itoa(i, buf, 10);
			m_popup->AddItem(buf, response);
		}
	}

	m_popup->AddItem("-1", (strings.empty() && !s_conversationHasFocus) ? CuiStringIds::convo_close_message.localize() : CuiStringIds::convo_stop_conversing.localize());
	m_popup->Link();
	m_popup->SetFocus();

	int const height = m_viewerPage->GetHeight() + cms_buffer + m_textPage->GetHeight() + cms_buffer + m_popup->GetHeight();
	getPage().SetHeight(height);
	getPage().Pack();

	CuiWorkspace * const workspace = getContainingWorkspace();
	if (workspace)
		workspace->updateGlowRect();
}

//----------------------------------------------------------------------

void SwgCuiPlayerQuestConversation::clearResponses()
{
	m_popup->ClearItems();
	m_popup->Link();

	getPage().Pack();

	CuiWorkspace * const workspace = getContainingWorkspace();
	if (workspace)
		workspace->updateGlowRect();
}

//----------------------------------------------------------------------

void SwgCuiPlayerQuestConversation::OnPopupMenuSelection(UIWidget * context)
{


	if (!context->IsA(TUIPopupMenu))
		return;

	if (context == m_popup)
	{
		// Erase the first node if the conversation doesn't have focus.
		if (!s_conversationHasFocus || !s_conversationActive) 
		{
			queuePop();
			return;
		}

		const UIPopupMenu * const pop = safe_cast<UIPopupMenu *>(context);

		const long index = pop->GetSelectedIndex ();

		if (index >= 0)
		{
			if (index >= static_cast<long>(CuiConversationManager::getResponses ().size ()))
			{
				IGNORE_RETURN(CuiConversationManager::stop ());
				onConversationEnded(true);
			}
			else
				IGNORE_RETURN(CuiConversationManager::respond (CuiConversationManager::getTarget (), index));
		}
	}
	else
	{
		SwgCuiLockableMediator::OnPopupMenuSelection(context);
	}


}

//----------------------------------------------------------------------

bool SwgCuiPlayerQuestConversation::setTauntInfo(NetworkId const & taunter, Unicode::String const & text, uint32 const appearanceOverloadSharedTemplateCrc, std::string const & soundEffect, float duration)
{
	ConversationNode convoNode;

	if (duration < 0.0f)
	{
		convoNode.setType(ConversationNode::cn_untimed_message);
		convoNode.setExpireTime(duration);
	}
	else
	{
		if (duration > 0.0f) 
		{
			convoNode.setType(ConversationNode::cn_timed_message);
			convoNode.setExpireTime(duration);
		}
		else
		{
			convoNode.setType(ConversationNode::cn_taunt);
			convoNode.setExpireTime(cms_minimumDisplayTime);

			// set an absolute expire time.
			convoNode.setTimestamp(Clock::getCurrentTime() + static_cast<double>(convoNode.getExpireTime()));
		}
	}

	CuiStringVariablesData data;
	data.target = Game::getPlayer() ? Game::getPlayer()->asClientObject() : NULL ;
	data.targetName = Game::getPlayerCreature() ? Game::getPlayerCreature()->getLocalizedNameUnfiltered() : Unicode::emptyString;

	Unicode::String resultText;

	CuiStringVariablesManager::process(text, data, resultText);

	convoNode.setText(resultText);
	convoNode.setTarget(taunter);
	convoNode.setOverrideAppearanceTemplate(appearanceOverloadSharedTemplateCrc);
	convoNode.setSoundEffect(soundEffect);

	queuePush(convoNode);

	// Go ahead and play the sound effect.
	if (s_conversationActive) 
	{
		playSound(soundEffect);
	}

	Unicode::String chatWindowText;
	ProsePackage pp;
	pp.stringId = CuiStringIds::comm_window;
	//@TODO: The following code doesn't work because the object is not observable
	//Object const * const taunterObject = NetworkIdManager::getObjectById(taunter);	
	//ClientObject const * const taunterClientObject = taunterObject ? taunterObject->asClientObject() : NULL;
	//if(taunterObject == NULL)
	//DEBUG_WARNING(true, ("taunterObject is null"));
	//DEBUG_WARNING(true, ("taunt id '%s' '%x' '%x' '%s'", taunter.getValueString(), taunterObject, taunterClientObject, taunterClientObject ? Unicode::wideToNarrow(taunterClientObject->getObjectName()).c_str() : "NULL"));
	//pp.target.id = taunter;	
	ProsePackageManagerClient::appendTranslation(pp, chatWindowText);
	chatWindowText.append(resultText);
	SwgCuiHud * const hud = SwgCuiHudFactory::findMediatorForCurrentHud();
	if (hud)
	{
		SwgCuiChatWindow * const chatWindow = NON_NULL (hud->getChatWindow  ());					
		if(chatWindow)
			chatWindow->onSpatialChatReceived(chatWindowText);
	}

	return true;
}

//======================================================================

