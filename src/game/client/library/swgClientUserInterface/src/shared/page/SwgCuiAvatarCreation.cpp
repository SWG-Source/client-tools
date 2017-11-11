// ======================================================================
//
// SwgCuiAvatarCreation.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiAvatarCreation.h"

#include "UIButton.h"
#include "UIData.h"
#include "UIDataSource.h"
#include "UIList.h"
#include "UIMessage.h"
#include "UIPage.h"
#include "UIPalette.h"
#include "UIText.h"
#include "UITextbox.h"
#include "UnicodeUtils.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGame/CreatureController.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientUserInterface/CuiCharacterHairManager.h"
#include "clientUserInterface/CuiCharacterLoadoutManager.h"
#include "clientUserInterface/CuiLoginManager.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiSkillManager.h"
#include "clientUserInterface/CuiStringIds.h"
#include "clientUserInterface/CuiTransition.h"
#include "clientUserInterface/CuiTurntableAdapter.h"
#include "clientUserInterface/CuiWidget3dObjectListViewer.h"
#include "sharedCollision/BoxExtent.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedGame/Command.h"
#include "sharedGame/CommandTable.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedNetworkMessages/ClientCentralMessages.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/SeesawDynamics.h"
#include "sharedSkillSystem/SkillManager.h"
#include "sharedSkillSystem/SkillObject.h"
#include "swgClientUserInterface/SwgCuiAvatarCreationHelper.h"
#include "swgClientUserInterface/SwgCuiMediatorTypes.h"
#include "swgClientUserInterface/SwgCuiSceneSelection.h"
#include "swgSharedUtility/SpeciesRestrictions.h"
#include "unicodeArchive/UnicodeArchive.h"
#include "sharedDebug/DebugFlags.h"

#include <algorithm>
#include <list>

//-----------------------------------------------------------------

namespace
{
	bool s_testJediCreation = false;

	const UILowerString s_maleProp   = UILowerString ("appearanceMale");
	const UILowerString s_femaleProp = UILowerString ("appearanceFemale");

	namespace Properties
	{
		const UILowerString DefaultViewerPitch = UILowerString ("DefaultViewerPitch");
		const UILowerString DefaultViewerYaw   = UILowerString ("DefaultViewerYaw");
	}
}

//-----------------------------------------------------------------

SwgCuiAvatarCreation::SwgCuiAvatarCreation (UIPage & newPage) :
CuiMediator               ("SwgCuiAvatarCreation", newPage),
UIEventCallback           (),
m_okButton                (0),
m_cancelButton            (0),
m_avatarList              (0),
m_infoText                (0),
m_avatarNameText          (0),
m_avatar                  (0),
m_objectViewer            (0),
m_avatarMap               (new AvatarMap_t),
m_messageBox              (0),
m_buttonGender            (0),
m_gender                  (G_Female),
m_autoConnected           (false),
m_purgePoolOnDeactivate   (false)
{

	static bool installed = false;

	if (!installed)
	{
		DebugFlags::registerFlag(s_testJediCreation,  "ClientUserInterface", "testJediCreation");
	}

	UIWidget *widget = 0;
	getCodeDataObject (TUIWidget, widget, "ViewerWidget");
	m_objectViewer = NON_NULL (dynamic_cast<CuiWidget3dObjectListViewer *>(widget));

	m_objectViewer->SetLocalTooltip (CuiStringIds::tooltip_viewer_3d_controls.localize ());
	m_objectViewer->SetPropertyFloat (Properties::DefaultViewerPitch, m_objectViewer->getCameraPitch ());
	m_objectViewer->SetPropertyFloat (Properties::DefaultViewerYaw,   m_objectViewer->getCameraYaw   ());

	getCodeDataObject (TUIText,    m_infoText,             "textInfo");
	getCodeDataObject (TUIText,    m_avatarNameText,       "textName");
	getCodeDataObject (TUIButton,  m_okButton,             "buttonNext");
	getCodeDataObject (TUIButton,  m_cancelButton,         "buttonPrev");
	getCodeDataObject (TUIList,    m_avatarList,           "listSpecies");

	getCodeDataObject (TUIButton,  m_buttonGender,         "buttonGender");

	m_avatarNameText->SetPreLocalized   (true);
	m_avatarNameText->SetTextUnroll     (false);
	m_avatarNameText->SetTextUnrollOnce (true);

	m_infoText->SetPreLocalized         (true);
	m_infoText->SetTextUnroll           (false);
	m_infoText->SetTextUnrollOnce       (true);

	bool b = false;
	if (m_buttonGender->GetPropertyBoolean (UILowerString ("ismale"), b))
		m_gender = b ? G_Male : G_Female;
	if (m_gender == G_Female)
	{
		m_buttonGender->Press ();
		m_gender = G_Male;
	}

	// Filter out characters that we aren't authorized to create
	// This assumes that if the male template is restricted, so is the female one
	UIDataSource * const ds = m_avatarList->GetDataSource ();
	if (ds)
	{
		UIDataList olist = ds->GetData ();
		for (UIDataList::iterator it = olist.begin (); it != olist.end (); ++it)
		{
			UIData * const data = *it;
			NOT_NULL (data);
				
			std::string maleTemplate;
			data->GetPropertyNarrow(s_maleProp, maleTemplate);
			const std::string serverTemplate = SwgCuiAvatarCreationHelper::convertSharedToServer (maleTemplate);
			
			if (!SpeciesRestrictions::canCreateCharacter(Game::getGameFeatureBits(), Crc::calculate(serverTemplate.c_str())))
			{
				ds->RemoveChild(data);
			}
		}
	}
	
}

//-----------------------------------------------------------------

SwgCuiAvatarCreation::~SwgCuiAvatarCreation ()
{
	m_okButton       = 0;
	m_cancelButton   = 0;
	m_avatarList     = 0;
	m_infoText       = 0;
	m_avatarNameText = 0;
	m_avatar         = 0;
	m_objectViewer   = 0;
	m_messageBox     = 0;

	std::for_each (m_avatarMap->begin (), m_avatarMap->end (), PointerDeleterPairSecond ());

	m_avatarMap->clear ();
	delete m_avatarMap;
	m_avatarMap = 0;
}

//-----------------------------------------------------------------

void SwgCuiAvatarCreation::performActivate ()
{
	if (s_testJediCreation)
		SwgCuiAvatarCreationHelper::setCreatingJedi (true);

	m_purgePoolOnDeactivate = false;

	setPointerInputActive  (true);
	setKeyboardInputActive (true);
	setInputToggleActive   (false);

	if (m_avatarList->GetLastSelectedRow () < 0)
		m_avatarList->SelectRow (0);
	
	m_okButton->AddCallback      (this);
	m_cancelButton->AddCallback  (this);
	m_avatarList->AddCallback (this);
	
	m_buttonGender->AddCallback      (this);
	
	{
		float f = 0.0f;
		if (m_objectViewer->GetPropertyFloat (Properties::DefaultViewerPitch, f))
			m_objectViewer->setCameraPitch (f);
		if (m_objectViewer->GetPropertyFloat (Properties::DefaultViewerYaw, f))
			m_objectViewer->setCameraYaw (f, true);
	}

	m_objectViewer->setPaused (false);

	m_avatarList->SetFocus ();

	if (!autoConnectOk ())
	{
		loadAllPlayerModels ();
		updateAvatarSelection ();
		setIsUpdating (true);
		m_isLoading = true;
	}
	else
	{
		CuiMediatorFactory::activate (CuiMediatorTypes::AvatarSummary);
		m_autoConnected = true;
		SwgCuiAvatarCreationHelper::finishCreation (true);
		deactivate ();
	}
}

//-----------------------------------------------------------------

void SwgCuiAvatarCreation::performDeactivate ()
{
	setIsUpdating (false);

	if (m_purgePoolOnDeactivate)
	{
		SwgCuiAvatarCreationHelper::purgePool                    ();
		m_purgePoolOnDeactivate = false;
	}

	m_objectViewer->setPaused (true);

	m_objectViewer->clearObjects ();

	m_okButton->RemoveCallback      (this);
	m_cancelButton->RemoveCallback  (this);
	m_avatarList->RemoveCallback (this);

	m_buttonGender->RemoveCallback      (this);

	std::for_each (m_avatarMap->begin (), m_avatarMap->end (), PointerDeleterPairSecond ());
	m_avatarMap->clear ();
}

//----------------------------------------------------------------------

bool SwgCuiAvatarCreation::autoConnectOk () const
{
	return !m_autoConnected && ConfigClientGame::getAutoConnectToGameServer () && ConfigClientGame::getAutoConnectToCentralServer () && CuiLoginManager::getConnectedClusterId () != 0;
}

//-----------------------------------------------------------------

void SwgCuiAvatarCreation::OnButtonPressed( UIWidget *Context )
{
	if (Context == m_okButton )
	{
		const UIData * const selectedData = m_avatarList->GetDataAtRow (m_avatarList->GetLastSelectedRow ());

		std::string avatarTemplateName;
		if (selectedData)
			IGNORE_RETURN (selectedData->GetPropertyNarrow (m_gender == G_Male ? s_maleProp : s_femaleProp, avatarTemplateName));

		static const Unicode::unicode_char_t badChars [] = { '/', '\\', '\\', '?', '!', '*', ':', ';', '$', '%', '<', '>', '|', 0 };

		if (avatarTemplateName.empty ())
		{
			IGNORE_RETURN (CuiMessageBox::createInfoBox (CuiStringIds::avatar_err_no_species_selected.localize ()));
		}

		else
		{
			CreatureObject * const object = NON_NULL (dynamic_cast<CreatureObject *>(m_objectViewer->getLastObject ()));

			const AvatarMap_t::iterator it = m_avatarMap->find (object->getObjectTemplateName ());
			DEBUG_FATAL (it == m_avatarMap->end (), ("bogosity."));

			//-- remove the avatar from the map, and let the customization screen own it
			m_avatarMap->erase (it);

			SwgCuiAvatarCreationHelper::purgePool                    ();
			IGNORE_RETURN (SwgCuiAvatarCreationHelper::setCreature   (*object));
			SwgCuiAvatarCreationHelper::setCreatureCustomized        (false);
			SwgCuiAvatarCreationHelper::requestRandomName            (false);

			//-- go to avatar customization screen
			CuiTransition::startTransition (CuiMediatorTypes::AvatarCreation, CuiMediatorTypes::AvatarCustomize);
		}
	}
	else if (Context == m_cancelButton)
	{
		SwgCuiAvatarCreationHelper::purgePool  ();
		
		if (Game::getSinglePlayer ())
			CuiTransition::startTransition (CuiMediatorTypes::AvatarCreation, CuiMediatorTypes::SceneSelection);
		else
			CuiTransition::startTransition (CuiMediatorTypes::AvatarCreation, CuiMediatorTypes::AvatarSelection);
	}

	else if (Context == m_buttonGender)
	{
		m_gender = m_gender == G_Male ? G_Female : G_Male;
		updateAvatarSelection ();
	}
} //lint !e818 //stfu noob

//----------------------------------------------------------------------

bool SwgCuiAvatarCreation::areAllDetailLevelsAvailable ()
{
	bool available = true;
	for (AvatarMap_t::const_iterator it = m_avatarMap->begin (); it != m_avatarMap->end (); ++it)
	{
		const CreatureObject * const creature = (*it).second;
		if (creature)
		{
			available = SwgCuiAvatarCreationHelper::areAllDetailLevelsAvailable (*creature) && available;
		}
	}

	return available;
}

//----------------------------------------------------------------------

void SwgCuiAvatarCreation::loadAllPlayerModels ()
{
	UIDataSource * const ds = m_avatarList->GetDataSource ();
	
	if (ds)
	{
		const UIDataList & olist = ds->GetData ();
		
		for (UIDataList::const_iterator it = olist.begin (); it != olist.end (); ++it)
		{
			const UIData * const data = *it;
			NOT_NULL (data);
			
			{
				std::string templateName;
				if (data->GetPropertyNarrow (s_maleProp,   templateName))
				{
					const AvatarMap_t::const_iterator fit = m_avatarMap->find (templateName);
					if (fit == m_avatarMap->end () || (*fit).second == 0)
					{
						(*m_avatarMap) [templateName]        = createPlayerModel (templateName);
					}
				}
			}
			
			{
				std::string templateName;
				if (data->GetPropertyNarrow (s_femaleProp,   templateName))
				{
					const AvatarMap_t::const_iterator fit = m_avatarMap->find (templateName);
					if (fit == m_avatarMap->end () || (*fit).second == 0)
					{
						(*m_avatarMap) [templateName]        = createPlayerModel (templateName);
					}
				}
			}
		}
	}

	//-- force a load of all assets
	areAllDetailLevelsAvailable ();
}

//----------------------------------------------------------------------

CreatureObject * SwgCuiAvatarCreation::createPlayerModel (const std::string & templateName)
{
	CreatureObject * const creature = safe_cast<CreatureObject*>(ObjectTemplate::createObject (templateName.c_str()));
	if (!creature)
	{
		WARNING (true, ("SwgCuiAvatarCreation failed [%s]", templateName.c_str ()));
		return 0;
	}
	
	const SharedObjectTemplate * const tmpl = dynamic_cast<const SharedObjectTemplate *>(creature->getObjectTemplate ());
	
	if (tmpl)
	{
		//-- set avatar scale to the midpoint
		creature->setScaleFactor ((tmpl->getScaleMax () + tmpl->getScaleMin ()) * 0.5f);
	}
	
	CreatureController* const controller = new CreatureController (creature);
	creature->setController (controller);
	creature->endBaselines ();
	
	Appearance * const app = creature->getAppearance ();
	if (app)
	{
		SkeletalAppearance2 * const skelApp = app->asSkeletalAppearance2 ();
		if (skelApp)
		{
			//-- preload all lod assets
			const int count = skelApp->getDetailLevelCount ();
			for (int i = 0; i < count; ++i)
				skelApp->isDetailLevelAvailable (i);
		}
	}
	
	if (ConfigClientGame::getCharacterCreationLoadoutsEnabled ())
	{
		CuiCharacterLoadoutManager::setupLoadout  (*creature);
		CuiCharacterHairManager::setupDefaultHair (*creature);
	}

	return creature;
}

//-----------------------------------------------------------------

void SwgCuiAvatarCreation::updateAvatarSelection ()
{
	const UIData * const selectedData = m_avatarList->GetDataAtRow (m_avatarList->GetLastSelectedRow ());

	if (selectedData)
	{
		UIString selectionName;
		selectedData->GetProperty (UILowerString ("LocalText"), selectionName);

		UINarrowString templateName;
		selectedData->GetPropertyNarrow (m_gender == G_Male ? s_maleProp : s_femaleProp, templateName);

		m_avatarNameText->SetLocalText (selectionName);

		if (m_avatar)
			m_objectViewer->clearObjects ();

		const AvatarMap_t::const_iterator fit = m_avatarMap->find (templateName);
		if (fit != m_avatarMap->end ())
			m_avatar = (*fit).second;

		if (m_avatar == 0)
		{
			m_avatar = createPlayerModel (templateName);
			if (m_avatar)
			{
				(*m_avatarMap) [templateName]        = m_avatar;
			}
			else
			{
				IGNORE_RETURN (CuiMessageBox::createInfoBox (CuiStringIds::avatar_err_avatar_not_found.localize ()));
				m_avatarList->SelectRow (-1);
				return;
			}
		}

		m_avatar->resetRotateTranslate_o2p ();

		TangibleObject * const pedestal = SwgCuiAvatarCreationHelper::getPedestal ();

		if (pedestal)
		{
			pedestal->resetRotateTranslate_o2p ();
			m_objectViewer->addObject (*pedestal);
			
			const BoxExtent * const box = dynamic_cast<const BoxExtent *>(pedestal->getAppearance ()->getExtent ());
			
			if (box)
				m_avatar->move_o (Vector::unitY * (box->getTop () + SwgCuiAvatarCreationHelper::getPedestalOffsetFromTop ()));
		}
		else
			WARNING (true, ("No pedestal for SwgCuiAvatarCreation"));

		m_objectViewer->addObject (*m_avatar);

		m_objectViewer->setViewDirty             (true);
		m_objectViewer->setCameraForceTarget     (true);
		m_objectViewer->recomputeZoom            ();
		m_objectViewer->setCameraForceTarget     (false);
		m_objectViewer->setViewDirty             (true);

		UIString desc;

		std::string descEncoded;

		if (!selectedData->GetPropertyNarrow (UILowerString ("Description"), descEncoded))
			desc = Unicode::narrowToWide ("NO DESCRIPTION SPECIFIED");
		else
			desc = StringId (descEncoded).localize ();

		desc.push_back ('\n');

//		description += CuiStringIds::avatar_species_abilities.localize ();

		std::string skillName;
		if (!selectedData->GetPropertyNarrow (UILowerString ("skill"), skillName))
			WARNING (true, ("SwgCuiAvatarCreation data [%s] has no skill property", selectedData->GetFullPath ().c_str ()));
		else
		{
			const SkillObject * const skill = SkillManager::getInstance ().getSkill (skillName);
			
			if (!skill)
			{
				WARNING (true, ("SwgCuiAvatarCreation data [%s] has bogus skill property [%s]", selectedData->GetFullPath ().c_str (), skillName.c_str ()));
			}
			else
			{
				Unicode::String tmp;
				CuiSkillManager::localizeSkillName (*skill, tmp);
				
				desc.push_back ('\n');
				desc += tmp;
				desc.push_back ('\n');

				static char buf [128];
				static const size_t buf_size = sizeof (buf);
				
				{
					const SkillObject::GenericModVector & skillMods = skill->getStatisticModifiers ();
					for (SkillObject::GenericModVector::const_iterator it = skillMods.begin (); it != skillMods.end (); ++it)
					{
						const std::string & skillModName  = (*it).first;
						const int           skillModValue = (*it).second;
						
						tmp.clear ();
						CuiSkillManager::localizeSkillModName (skillModName, tmp);
						snprintf (buf, buf_size, "\\#.: \\>240%+d\\>000\n", skillModValue);
						
						desc += Unicode::narrowToWide ("\\>024\\#pcontrast1 ");
						desc += tmp;
						desc += Unicode::narrowToWide (buf);
					}
				}
				/*
				{
					const SkillObject::StringVector & commands = skill->getCommandsProvided ();	
					
					for (SkillObject::StringVector::const_iterator it = commands.begin (); it != commands.end (); ++it)
					{
						const std::string & cmdName       = Unicode::toLower ((*it));
						
						const bool isCommand = !CommandTable::getCommand (Crc::normalizeAndCalculate (cmdName.c_str ())).isNull ();
						UNREF (isCommand);
						
						tmp.clear ();
						CuiSkillManager::localizeCmdName (cmdName, tmp);
						desc += Unicode::narrowToWide ("\\>024\\#pcontrast1 ");
						desc += tmp;
						desc += Unicode::narrowToWide ("\\>048\\#.\n");
						tmp.clear  ();
						CuiSkillManager::localizeCmdDescription (cmdName, tmp);
						desc += tmp;
						desc += Unicode::narrowToWide ("\\>000\n");
					}
				}
				*/
			}
		}
		
		m_infoText->SetLocalText   (desc);
		m_infoText->ScrollToPoint  (UIPoint::zero);
		m_infoText->SetTextUnroll     (true);
		m_infoText->ResetTextUnroll   ();
	}
	else
	{
		if (m_avatar)
			m_objectViewer->clearObjects ();

		m_avatarNameText->Clear ();
		m_infoText->Clear ();
	}
}

//-----------------------------------------------------------------

void SwgCuiAvatarCreation::OnGenericSelectionChanged (UIWidget * context)
{
	if (context == m_avatarList)
		updateAvatarSelection ();
} //lint !e818 //stfu noob

//----------------------------------------------------------------------

void SwgCuiAvatarCreation::update (float deltaTimeSecs)
{
	CuiMediator::update (deltaTimeSecs);

	if (m_isLoading)
	{
		if (!m_avatar || SwgCuiAvatarCreationHelper::areAllDetailLevelsAvailable (*m_avatar))
		{
			m_isLoading = false;
			CuiTransition::signalTransitionReady (CuiMediatorTypes::AvatarCreation);
		}
	}
}

//-----------------------------------------------------------------
