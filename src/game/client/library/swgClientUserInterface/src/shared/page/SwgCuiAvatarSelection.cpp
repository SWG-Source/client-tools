// ======================================================================
//
// SwgCuiAvatarSelection.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiAvatarSelection.h"

#include "Archive/ByteStream.h"
#include "UIButton.h"
#include "UICheckbox.h"
#include "UIData.h"
#include "UIDataSource.h"
#include "UIMessage.h"
#include "UIPage.h"
#include "UITable.h"
#include "UITableModelDefault.h"
#include "UIText.h"
#include "UIUtils.h"
#include "UnicodeUtils.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGame/ConnectionManager.h"
#include "clientGame/CreatureController.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "clientGame/RoadmapManager.h"
#include "clientUserInterface/CuiCachedAvatarManager.h"
#include "clientUserInterface/CuiLoadingManager.h"
#include "clientUserInterface/CuiLoginManager.h"
#include "clientUserInterface/CuiLoginManagerAvatarInfo.h"
#include "clientUserInterface/CuiLoginManagerClusterInfo.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiSettings.h"
#include "clientUserInterface/CuiStringIds.h"
#include "clientUserInterface/CuiStringIdsServer.h"
#include "clientUserInterface/CuiTransition.h"
#include "clientUserInterface/CuiWidget3dObjectListViewer.h"
#include "sharedFoundation/ApplicationVersion.h"
#include "sharedFoundation/Branch.h"
#include "sharedFoundation/Production.h"
#include "sharedMathArchive/VectorArchive.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/ClientCentralMessages.h"
#include "sharedNetworkMessages/CommandChannelMessages.h"
#include "sharedNetworkMessages/DeleteCharacterMessage.h"
#include "sharedNetworkMessages/DeleteCharacterReplyMessage.h"
#include "sharedUtility/LocalMachineOptionManager.h"
#include "swgClientUserInterface/SwgCuiAvatarCreationHelper.h"
#include "swgClientUserInterface/SwgCuiDeleteAvatarConfirmation.h"
#include "swgClientUserInterface/SwgCuiMediatorTypes.h"
#include "swgClientUserInterface/SwgCuiSceneSelection.h"
#include "unicodeArchive/UnicodeArchive.h"
#include <algorithm>

//-----------------------------------------------------------------

namespace
{
	namespace UnnamedMessages
	{
		const char * const GameConnectionOpened             = "GameConnectionOpened";
	}

	namespace Properties
	{
		const UILowerString AvatarNetworkId    = UILowerString ("AvatarNetworkId");
		const UILowerString ClusterId          = UILowerString ("ClusterId");
		const UILowerString DefaultViewerPitch = UILowerString ("DefaultViewerPitch");
		const UILowerString DefaultViewerYaw   = UILowerString ("DefaultViewerYaw");
	}

	bool s_autoSelectedAvatar = false;
	CuiLoginManagerAvatarInfo   s_avatarToDelete;

	const Unicode::String s_unlockedSlotCharacterSuffix = Unicode::narrowToWide(" \\#FF0000[UNLOCKED SLOT]");


	Unicode::String CreateTooltipText (const CuiLoginManagerAvatarInfo & avatarInfo)
	{
		Unicode::String tooltipText;
		if (avatarInfo.characterLevel > 0)
		{
			const StringId levelId ("ui_charsheet", "level");
			tooltipText = levelId.localize () + Unicode::narrowToWide (" ");
			Unicode::String tmpStr1;
			UIUtils::FormatLong (tmpStr1, avatarInfo.characterLevel);
			tooltipText+= tmpStr1;
		}

		if (!avatarInfo.characterSkillTemplate.empty ())
		{
			const std::string roadmapName = "title_" + RoadmapManager::getRoadmapNameForTemplateName (avatarInfo.characterSkillTemplate);
			const StringId roadmapNameStringId ("ui_roadmap", roadmapName.c_str());
			tooltipText += Unicode::narrowToWide (" ");
			tooltipText += roadmapNameStringId.localize ();
		}

		return tooltipText;
	}

	// SWG Source Change 2021 - Aconite
	// Remove closed server check process for obvious reasons
	/*
	// must be in ascending sorted order (and no dupes either)!!!
	const uint32 ms_closedServerIds[] = {  4, // Corbantis
										   9, // Kauri
										  10, // Lowca
										  13, // Intrepid 
										  14, // Kettemoor
										  15, // Naritus
										  16, // Scylla
										  18, // Valcyn
										  19, // Tempest
										  26, // Tarquinas
										  27, // Wanderhome
										  36, // Europe-Infinity
										  40, // Japan-Katana
										  41  // Japan-Harla										   
										 };

	const int ms_closedServerTotal = sizeof(ms_closedServerIds) / sizeof(ms_closedServerIds[0]);

	bool isClosedServer(uint32 serverId)
	{
#if PRODUCTION != 1
		for (int i = 1; i < ms_closedServerTotal; ++i)
		{
			FATAL((ms_closedServerIds[i-1] >= ms_closedServerIds[i]), ("ms_closedServerIds must be in ascending sorted order (and no dupes either) - ms_closedServerIds[%d]=%lu >= ms_closedServerIds[%d]=%lu", (i-1), ms_closedServerIds[i-1], i, ms_closedServerIds[i]));
		}
#endif

		return std::binary_search(&ms_closedServerIds[0], &ms_closedServerIds[ms_closedServerTotal], serverId);
	}
	*/
}

//----------------------------------------------------------------------

SwgCuiAvatarSelection::SwgCuiAvatarSelection (UIPage & page) :
CuiMediator              ("SwgCuiAvatarSelection", page),
UIEventCallback          (),
MessageDispatch::Receiver(),
m_okButton               (0),
m_cancelButton           (0),
m_createButton           (0),
m_deleteButton           (0),
m_avatarNameText         (0),
m_table                  (0),
m_objectViewer           (0),
m_messageBox             (0),
m_messageBoxDeleteWait   (0),
m_messageBoxLoginWait    (0),
m_waitingLoginForDelete  (false),
m_waitingLogin           (false),
m_waitingLoginForSelect  (false),
m_waitingLoginForCreate  (false),
m_autoConnected          (false),
m_proceed                (false),
m_callback               (new MessageDispatch::Callback),
m_selectedAvatar         (new CuiLoginManagerAvatarInfo),
m_waitingDeletion        (false),
m_deletingAvatar         (new CuiLoginManagerAvatarInfo),
m_updateAvatar           (0),
m_waitingForConnection   (false),
m_dropFromCluster        (false),
m_waitingForClusterId    (0),
m_connectionTimeout      (0.0f),
m_avatarPopulateFirstTime (true),
m_deleteAvatarConfirmationPage(NULL),
m_deleteAvatarConfirmationMediator(NULL),
m_waitForConnectionRetry(false),
m_hasAlreadyRetriedConnection(false),
m_hideClosed (NULL)
{
	UIWidget *widget = 0;
	getCodeDataObject (TUIWidget, widget, "ViewerWidget");
	m_objectViewer = NON_NULL (dynamic_cast<CuiWidget3dObjectListViewer *>(widget));
	m_objectViewer->SetLocalTooltip (CuiStringIds::tooltip_viewer_3d_controls.localize ());
	m_objectViewer->SetPropertyFloat (Properties::DefaultViewerPitch, m_objectViewer->getCameraPitch ());
	m_objectViewer->SetPropertyFloat (Properties::DefaultViewerYaw,   m_objectViewer->getCameraYaw   ());
	m_objectViewer->setRotationSlowsToStop(true);

	getCodeDataObject (TUIText,       m_avatarNameText, "textName");
	getCodeDataObject (TUIButton,     m_okButton,       "buttonNext");
	getCodeDataObject (TUIButton,     m_cancelButton,   "buttonPrev");
	getCodeDataObject (TUIButton,     m_createButton,   "buttonCreate");
	getCodeDataObject (TUIButton,     m_deleteButton,   "buttonDelete");
	getCodeDataObject (TUICheckbox,   m_hideClosed,     "checkHideClosed");

	registerMediatorObject(*m_hideClosed, true);
	m_hideClosed->SetChecked(CuiPreferences::getHideCharactersOnClosedGalaxies());

	getCodeDataObject (TUIPage,       m_deleteAvatarConfirmationPage, "deleteConfirmation");
	m_deleteAvatarConfirmationPage->SetVisible(false);
	m_deleteAvatarConfirmationMediator = new SwgCuiDeleteAvatarConfirmation(*m_deleteAvatarConfirmationPage);
	m_deleteAvatarConfirmationMediator->fetch();

	getCodeDataObject (TUITable,      m_table,          "table");

	m_table->SetVisible         (false);
	m_table->SetVisible         (true);

	UITableModelDefault * const model = NON_NULL (safe_cast<UITableModelDefault *>(m_table->GetTableModel ()));
	model->ClearData   ();
	m_table->SelectRow (-1);
}

//-----------------------------------------------------------------

SwgCuiAvatarSelection::~SwgCuiAvatarSelection ()
{
	delete m_deletingAvatar;
	m_deletingAvatar = 0;

	delete m_callback;
	m_callback = 0;

	m_okButton      = 0;
	m_cancelButton  = 0;
	m_createButton  = 0;
	m_deleteButton  = 0;
	m_table         = 0;
	m_hideClosed    = 0;

	m_avatarNameText = 0;

	m_objectViewer = 0;
	m_messageBox = 0;
	m_messageBoxDeleteWait = 0;

	delete m_selectedAvatar;
	m_selectedAvatar = 0;

	m_messageBoxLoginWait = 0;

	m_deleteAvatarConfirmationMediator->release();
}

//-----------------------------------------------------------------

void SwgCuiAvatarSelection::performActivate ()
{
	SwgCuiAvatarCreationHelper::setCreatingJedi (false);

	s_avatarToDelete.clear ();

	m_connectingToGame       = false;
	m_connectionTimeout      = 0.0f;
	m_avatarNameText->Clear ();

	m_table->SelectRow (-1);

	GameNetwork::setAcceptSceneCommand (false);
	
	m_dropFromCluster       = false;
	m_waitingForConnection  = false;
	m_proceed               = false;
	m_waitingDeletion       = false;
	m_waitingLoginForDelete = false;
	m_waitingLoginForSelect = false;
	m_waitingLoginForCreate = false;
	m_waitingLogin          = false;

	m_deletingAvatar->clear ();

	CuiLoginManager::setAllPingsDisabled ();

	m_callback->connect (*this, &SwgCuiAvatarSelection::onClusterConnection,      static_cast<CuiLoginManager::Messages::ClusterConnection *>     (0));
	m_callback->connect (*this, &SwgCuiAvatarSelection::onAvatarListChanged,      static_cast<CuiLoginManager::Messages::AvatarListChanged*>      (0));
	m_callback->connect (*this, &SwgCuiAvatarSelection::onClusterStatusChanged,   static_cast<CuiLoginManager::Messages::ClusterStatusChanged*>   (0));
	m_callback->connect (*this, &SwgCuiAvatarSelection::onDeleteAvatarConfirmation, static_cast<SwgCuiDeleteAvatarConfirmation::Message::DeleteAvatarConfirmation*> (0));

	setPointerInputActive  (true);
	setKeyboardInputActive (true);

	m_okButton->AddCallback      (this);
	m_cancelButton->AddCallback  (this);
	m_createButton->AddCallback  (this);
	m_deleteButton->AddCallback  (this);
	m_table->AddCallback         (this);

	connectToMessage (UnnamedMessages::GameConnectionOpened);
	connectToMessage (DeleteCharacterReplyMessage::MessageType);
	connectToMessage (CuiLoadingManager::Messages::FullscreenLoadingDisabled);
	connectToMessage (Game::Messages::SCENE_CHANGED);

	{
		float f = 0.0f;
		if (m_objectViewer->GetPropertyFloat (Properties::DefaultViewerPitch, f))
			m_objectViewer->setCameraPitch (f);
		if (m_objectViewer->GetPropertyFloat (Properties::DefaultViewerYaw, f))
			m_objectViewer->setCameraYaw (f, true);
	}

	m_createButton->SetEnabled (true);
	m_objectViewer->setPaused  (false);

	{
		UIText* text;

		bool useExitText = !Game::getSinglePlayer() && (CuiLoginManager::getSessionIdKey () && !ConfigClientGame::getEnableAdminLogin());

		getCodeDataObject (TUIText,       text, "backTextOnPrev");
		text->SetVisible(!useExitText);

		getCodeDataObject (TUIText,       text, "exitTextOnPrev");
		text->SetVisible(useExitText);
	}


	//-- reconnect if needed
//	if (!GameNetwork::isConnectedToLoginServer ())
//		reconnectLoginServer (false);
	clearCharacterList ();
	refreshList        (false);

	m_table->SetEnabled        (true);
	m_table->SetFocus          ();

	setIsUpdating (true);

	CuiTransition::signalTransitionReady (CuiMediatorTypes::AvatarSelection);
}

//-----------------------------------------------------------------

void SwgCuiAvatarSelection::performDeactivate ()
{
	disconnectFromMessage (UnnamedMessages::GameConnectionOpened);
	disconnectFromMessage (DeleteCharacterReplyMessage::MessageType);
	disconnectFromMessage (CuiLoadingManager::Messages::FullscreenLoadingDisabled);
	disconnectFromMessage (Game::Messages::SCENE_CHANGED);

	m_autoConnected = true;

	m_callback->disconnect (*this, &SwgCuiAvatarSelection::onAvatarListChanged,      static_cast<CuiLoginManager::Messages::AvatarListChanged*>      (0));
	m_callback->disconnect (*this, &SwgCuiAvatarSelection::onClusterConnection,      static_cast<CuiLoginManager::Messages::ClusterConnection *>     (0));
	m_callback->disconnect (*this, &SwgCuiAvatarSelection::onClusterStatusChanged,   static_cast<CuiLoginManager::Messages::ClusterStatusChanged*>   (0));
	m_callback->disconnect (*this, &SwgCuiAvatarSelection::onDeleteAvatarConfirmation, static_cast<SwgCuiDeleteAvatarConfirmation::Message::DeleteAvatarConfirmation*> (0));

	setIsUpdating (false);

	clearCharacterList ();

	if (m_messageBox)
		m_messageBox->closeMessageBox ();
	if (m_messageBoxDeleteWait)
		m_messageBoxDeleteWait->closeMessageBox ();
	if (m_messageBoxLoginWait)
		m_messageBoxLoginWait->closeMessageBox ();

	m_messageBox           = 0;
	m_messageBoxDeleteWait = 0;
	m_messageBoxLoginWait  = 0;

	m_objectViewer->setPaused (true);

	disconnectAll();

	m_okButton->RemoveCallback      (this);
	m_cancelButton->RemoveCallback  (this);
	m_createButton->RemoveCallback  (this);
	m_deleteButton->RemoveCallback  (this);
	m_table->RemoveCallback         (this);

	m_objectViewer->clearObjects ();
}

//-----------------------------------------------------------------

void SwgCuiAvatarSelection::refreshList (bool updateSelection)
{
	const long oldRowSelected = m_table->GetLastSelectedRow ();
	m_table->SelectRow (-1);

	UITableModelDefault * const model = NON_NULL (safe_cast<UITableModelDefault *>(m_table->GetTableModel ()));
	model->ClearData ();

	typedef CuiLoginManager::AvatarInfoVector AvatarInfoVector;

	CuiLoginManager::AvatarInfoVector aiv;
	CuiLoginManager::getAvatarInfo  (aiv);

	AvatarInfoVector::const_iterator it;

	bool hasAvatarOnClosedServers = false;
	for (it = aiv.begin (); it != aiv.end (); ++it)
	{
		const CuiLoginManagerAvatarInfo & avatarInfo = *it;
		//if (isClosedServer(avatarInfo.clusterId))
		//{
		//	hasAvatarOnClosedServers = true;
		//	break;
		//}
	}

	m_hideClosed->SetVisible(hasAvatarOnClosedServers);
	
	for (it = aiv.begin (); it != aiv.end (); ++it)
	{
		const CuiLoginManagerAvatarInfo & avatarInfo = *it;
		addAvatar (avatarInfo);

		if (!isActive ())
			break;
	}

	m_table->Link ();
	
	//-- if autoconnecting, don't waste time updating the player model
	if (updateSelection || m_table->GetLastSelectedRow () < 0)
	{
		if (!autoConnectOk ())
		{
			if (oldRowSelected >= 0)
				m_table->SelectRow (std::min (oldRowSelected, static_cast<long>(model->GetRowCount () - 1L)));
			else
				m_table->SelectRow (0);
		}
	}
	
	//-- if no avatars exist, try to create one
	if (aiv.empty () && !m_autoConnected && GameNetwork::isConnectedToLoginServer ())
	{
		m_autoConnected = true;
		m_createButton->Press ();
	}

	else if (m_table->GetLastSelectedRow () < 0)
	{
		if (autoConnectOk () && ConfigClientGame::getAutoConnectToCentralServer ())
			m_createButton->Press ();
		else if (updateSelection)
			m_table->SelectRow (0);
	}

	CuiCachedAvatarManager::saveCharacterList ();

	m_avatarPopulateFirstTime = false;
}

//----------------------------------------------------------------------

//-- temporarily just add avatars in non-alphabetical order...

void SwgCuiAvatarSelection::addAvatar (const CuiLoginManagerAvatarInfo & avatarInfo)
{
	UITableModelDefault * const model = NON_NULL (safe_cast<UITableModelDefault *>(m_table->GetTableModel ()));

	{
		const int numRows = model->GetRowCount ();

		for (int i = 0; i < numRows; ++i)
		{
			UIData * const data = model->GetCellDataLogical (i, 0);
			NOT_NULL (data);

			std::string networkIdStr;
			long        clusterId = 0;

			if (data->GetPropertyNarrow  (Properties::AvatarNetworkId, networkIdStr) && data->GetPropertyLong    (Properties::ClusterId, clusterId))			
			{
				//-- avatar is already in the table
				if (avatarInfo.clusterId == static_cast<uint32>(clusterId) && avatarInfo.networkId == NetworkId (networkIdStr))
				{
					Unicode::String tooltipText = CreateTooltipText (avatarInfo);
					if (!tooltipText.empty ())
					{
						data->SetProperty (UITableModelDefault::DataProperties::LocalTooltip, tooltipText);
					}
					return;
				}
			}
		}
	}

	const std::string narrowName (Unicode::wideToNarrow (avatarInfo.name));
	const CuiLoginManagerClusterInfo * const clusterInfo = CuiLoginManager::findClusterInfo (avatarInfo.clusterId);

	WARNING (!clusterInfo, ("Unable to load cluster info for cluster %d, requested by avatar [%s], %s", avatarInfo.clusterId, narrowName.c_str (), avatarInfo.networkId.getValueString ().c_str ()));

	//if (m_hideClosed->IsChecked() && isClosedServer(avatarInfo.clusterId))
	//	return;

	Unicode::String avatarDisplayName = avatarInfo.name;
	if (avatarInfo.characterType == static_cast<int>(EnumerateCharacterId_Chardata::CT_jedi))
		avatarDisplayName += s_unlockedSlotCharacterSuffix;

	Unicode::String clusterDisplayName = clusterInfo ? Unicode::narrowToWide (clusterInfo->name) : Unicode::emptyString;
	Unicode::String planetDisplayName = avatarInfo.planetName.empty () ? Unicode::emptyString : StringId ("planet_n", avatarInfo.planetName).localize ();
	Unicode::String statusDisplayStr = Unicode::emptyString;

	if (clusterInfo)
	{
		if (clusterInfo->up)
		{
			if (clusterInfo->loading)
				statusDisplayStr = CuiStringIdsServer::server_loading.localize ();
			else if (clusterInfo->locked)
				statusDisplayStr = CuiStringIdsServer::server_locked.localize ();
			else if (clusterInfo->restricted)
				statusDisplayStr = CuiStringIdsServer::server_restricted.localize ();
			else if (clusterInfo->isFull)
				statusDisplayStr = CuiStringIdsServer::server_full.localize ();
			else
				statusDisplayStr = CuiStringIdsServer::server_online.localize ();
		}
		else
			statusDisplayStr = CuiStringIdsServer::server_offline.localize ();
	}
	
#if PRODUCTION != 1
	if (clusterInfo && !clusterInfo->branch.empty())
	{
		static const Unicode::String::value_type *s_colorRed     = L"\\#ff0000";
		static const Unicode::String::value_type *s_colorGreen   = L"\\#00ff00";
		static const Unicode::String::value_type *s_colorBlue    = L"\\#0000ff";
		static const Unicode::String::value_type *s_colorMagenta = L"\\#ff00ff";
		static const Unicode::String::value_type *s_colorYellow  = L"\\#ffff00";
		static const Unicode::String::value_type *s_colorCyan    = L"\\#00ffff";
		static const Unicode::String::value_type *s_colorWhite   = L"\\#ffffff";
		static const Unicode::String::value_type *s_colorBlack   = L"\\#000000";

		Unicode::String color = s_colorRed; // red is the default color

		if (clusterInfo->netVersionMatch == true)
		{
			if (clusterInfo->branch == Branch().getBranchName())
			{
				if (clusterInfo->version == (uint)atoi( ApplicationVersion::getPublicVersion()))
				{
					color = s_colorGreen;
				}
				else
				{
					color = s_colorMagenta;
				}
			}
			else
			{
				color = s_colorYellow;
			}
		}

		char buffer[64];

		//avatarDisplayName = color + avatarDisplayName;
		clusterDisplayName = color + clusterDisplayName + L" [" + Unicode::narrowToWide(clusterInfo->branch) + L"." + Unicode::narrowToWide(_itoa(clusterInfo->version, buffer, 10)) +  L"]";
		planetDisplayName = color + planetDisplayName;
		statusDisplayStr = color + statusDisplayStr;
	}
#endif

	UIData * const d = model->AppendCell (0, narrowName.c_str (), avatarDisplayName);
	d->SetPropertyNarrow  (Properties::AvatarNetworkId,                 avatarInfo.networkId.getValueString ());
	d->SetPropertyLong    (Properties::ClusterId,                       static_cast<long>(avatarInfo.clusterId));

	Unicode::String tooltipText = CreateTooltipText(avatarInfo);

	if(!tooltipText.empty())
	{
		d->SetProperty  (UITableModelDefault::DataProperties::LocalTooltip, tooltipText);
	}

	model->AppendCell (1, narrowName.c_str (), clusterDisplayName);

	model->AppendCell (2, narrowName.c_str (), planetDisplayName);

	model->AppendCell (3, narrowName.c_str (), statusDisplayStr);

	
	std::string avatarNameToUse;
	uint32        launcherClusterId  = ConfigClientGame::getLauncherClusterId  ();
	{
		const std::string & launcherAvatarName = ConfigClientGame::getLauncherAvatarName ();
		
		if (!launcherAvatarName.empty ())
		{
			avatarNameToUse = launcherAvatarName;
			const size_t splitpos = avatarNameToUse.find (" (");
			if (splitpos != std::string::npos)
				avatarNameToUse = avatarNameToUse.substr (0, splitpos);
		}
		else
		{
			avatarNameToUse   = ConfigClientGame::getAvatarName ();
			launcherClusterId = CuiLoginManager::findClusterId (ConfigClientGame::getCentralServerName ());
		}
	}
	
	if (!avatarNameToUse.empty () && m_table->GetLastSelectedRow () < 0)
	{
		if (!_stricmp (avatarNameToUse.c_str (), narrowName.c_str ()))
		{
			if (launcherClusterId == 0 || launcherClusterId == avatarInfo.clusterId)
			{
				if (autoConnectOk () || !s_autoSelectedAvatar)
					m_table->SelectRow (static_cast<long>(model->GetRowCount ()) - 1L);
				
				//-----------------------------------------------------------------
				//-- see if we can autoconnect with this avatar
				
				if (autoConnectOk ())
				{
					m_okButton->Press ();
					ConfigClientGame::setNextAutoConnectToGameServer (false);
				}
			}
		}

		s_autoSelectedAvatar = true;
	}
} //lint !e429 //d not a leak

//----------------------------------------------------------------------

bool SwgCuiAvatarSelection::autoConnectOk () const
{
	if (ConfigClientGame::getNextAutoConnectToGameServer ())
		return true;

	if (!m_autoConnected)
	{
		if (ConfigClientGame::getAutoConnectToGameServer ())
			return true;
		
		if (!ConfigClientGame::getLauncherAvatarName ().empty ())
			return true;
	}

	return false;
}

//-----------------------------------------------------------------

void SwgCuiAvatarSelection::OnButtonPressed(UIWidget *Context)
{
	if (Context == m_cancelButton)
	{
		if (Game::getSinglePlayer ())
		{
			CuiTransition::startTransition (CuiMediatorTypes::AvatarSelection, CuiMediatorTypes::SceneSelection);
		}
		else
		{
			if (CuiLoginManager::getSessionIdKey () && !ConfigClientGame::getEnableAdminLogin())
			{
				deactivate ();
				CuiManager::terminateIoWin ();
				return;
			}
			else
			{
				CuiTransition::startTransition(CuiMediatorTypes::AvatarSelection, CuiMediatorTypes::LoginScreen);
			}
		}
	}

	//----------------------------------------------------------------------

	else if (Context == m_okButton)
	{
		requestAvatarSelection ();
	}

	//----------------------------------------------------------------------

	else if (Context == m_createButton)
	{
		handleCreate ();
	}
	else if (Context == m_deleteButton)
	{
		requestAvatarDeletion ();
	}

} //lint !e818 //stfu noob

//-----------------------------------------------------------------

void SwgCuiAvatarSelection::OnGenericSelectionChanged (UIWidget * context)
{
	if (context == m_table)
	{
		//-- if autoconnecting, don't waste time updating the player model
		if (!autoConnectOk ())
			m_updateAvatar = 2;
//			updateAvatarSelection ();
	}
} //lint !e818 //stfu noob

//-----------------------------------------------------------------

void SwgCuiAvatarSelection::updateAvatarSelection ()
{
	m_objectViewer->clearObjects ();
	m_avatarNameText->Clear ();

	UITableModelDefault * const model = NON_NULL (safe_cast<UITableModelDefault *>(m_table->GetTableModel ()));

	const int row = m_table->GetLastSelectedRow ();
	Unicode::String currentlySelectedCharacter;

	const UIData * const cellData = model->GetCellDataVisual (row, 0);
	if (row >= 0 && cellData)
	{
		UIString selectionName;
		cellData->GetProperty (UITableModelDefault::DataProperties::Value, selectionName);
		WARNING (selectionName.empty (), ("Empty selection name"));

		m_avatarNameText->SetLocalText (selectionName);

		std::string networkIdStr;
		if (!cellData->GetPropertyNarrow (Properties::AvatarNetworkId, networkIdStr))
			WARNING (true, ("Can't get networkid"));
		const NetworkId networkId (networkIdStr);

		long l_clusterId = 0;
		if (!cellData->GetPropertyLong (Properties::ClusterId, l_clusterId))
			WARNING (true, ("Can't get clusterid"));

		const uint32 clusterId = static_cast<uint32>(l_clusterId);

		CreatureObject * const avatar = CuiLoginManager::getAvatarCreature (clusterId, networkId);

		if (!avatar)
		{
			WARNING (true, ("no avatar?"));

			CuiMessageBox::createInfoBox (CuiStringIdsServer::server_err_avatar_not_found.localize ());
			m_table->SelectRow (-1);
		}
		else
		{
			const CuiLoginManagerAvatarInfo * const avatarInfo = CuiLoginManager::findAvatarInfo (clusterId, avatar->getLocalizedName ());

			if (avatarInfo)
			{
				UIData * const planetCellData = model->GetCellDataVisual (row, 2);
				if (planetCellData)
				{
					if (avatarInfo->planetName.empty ())
						planetCellData->SetProperty (UITableModelDefault::DataProperties::Value, Unicode::emptyString);
					else
						planetCellData->SetProperty (UITableModelDefault::DataProperties::Value, StringId ("planet_n", avatarInfo->planetName).localize ());
				}

				UIData * const nameCellData = model->GetCellDataVisual (row, 0);
				if (nameCellData)
				{
					Unicode::String tooltipText = CreateTooltipText (*avatarInfo);
					if (!tooltipText.empty ())
					{
						nameCellData->SetProperty(UITableModelDefault::DataProperties::LocalTooltip, tooltipText);
					}
				}
			}
				
			m_objectViewer->addObject (*avatar);

			m_objectViewer->setCameraForceTarget     (true);
			m_objectViewer->recomputeZoom            ();
			m_objectViewer->setCameraForceTarget     (false);
	
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiAvatarSelection::update (float deltaTimeSecs)
{
	CuiMediator::update (deltaTimeSecs);

	if (m_updateAvatar > 0)
	{
		if (--m_updateAvatar == 0)
			updateAvatarSelection ();
	}

	if (m_dropFromCluster)
	{
		WARNING (true, ("SwgCuiAvatarSelection dropping cluster connection"));
		CuiLoginManager::disconnectFromCluster ();
		m_dropFromCluster = false;
		return;
	}

	if (m_connectingToGame && !m_waitForConnectionRetry)
	{
		static const float TIMEOUT_CONNECTING_TO_GAME = ConfigClientGame::getConnectionTimeout();
		m_connectionTimeout += deltaTimeSecs;

		if (m_connectionTimeout > TIMEOUT_CONNECTING_TO_GAME)
		{
			//-- this should trigger a callback to this class
			CuiLoadingManager::setFullscreenLoadingEnabled (false);
			CuiMessageBox::createInfoBox (CuiStringIdsServer::server_timeout_gameserver.localize ());
		}

		// SWG Source Addition 2021 - Aconite
		// Catch when a player has recently crashed and is trying shortly thereafter to login to
		// their character again but the connection server has refused to load the character because
		// it is still authoritative so we need to make the request for a second time to reset
		// everything since we can't use the back button or escape and our only options are to close
		// the client entirely or wait for the connection request to timeout
		if (m_connectionTimeout > 8 && !GameNetwork::isConnectedToConnectionServer() && CuiLoadingManager::isLoadingScreenVisible() && !m_hasAlreadyRetriedConnection)
		{
			m_waitForConnectionRetry = true;
			m_hasAlreadyRetriedConnection = true;
			m_connectionTimeout = 0;
			requestAvatarSelection();
		}
		return;
	}

	if (m_proceed)
	{
		UITableModelDefault * const model = NON_NULL (safe_cast<UITableModelDefault *>(m_table->GetTableModel ()));
		const int row = m_table->GetLastSelectedRow ();

		Unicode::String currentlySelectedCharacter;
		if (model->GetValueAtText (row, 0, currentlySelectedCharacter) && !currentlySelectedCharacter.empty())
		{
			std::string::size_type const pos = currentlySelectedCharacter.find(s_unlockedSlotCharacterSuffix);
			if (pos != std::string::npos)
				currentlySelectedCharacter = currentlySelectedCharacter.substr(0, pos);

			std::string const currentlySelectedCharacterName = Unicode::wideToNarrow (currentlySelectedCharacter);
			ConfigClientGame::setAvatarName        (currentlySelectedCharacterName);
			ConfigClientGame::setCentralServerName (CuiLoginManager::getConnectedClusterName ());
			LocalMachineOptionManager::save ();
			ConfigClientGame::setNextAutoConnectToGameServer (false);

			UIData * const d = model->GetCellDataVisual (row, 0);
			std::string networkIdStr;
			d->GetPropertyNarrow  (Properties::AvatarNetworkId, networkIdStr);
			const NetworkId avatarId (networkIdStr);

			const SelectCharacter s (avatarId);
			GameNetwork::send (s, true);

			//set the destination planet, so loading screen can contextualize
			if(m_selectedAvatar)
			{
				CuiLoadingManager::setPlanetName(m_selectedAvatar->planetName);
			}

			CuiLoadingManager::setFullscreenLoadingEnabled (true);
			CuiLoadingManager::setFullscreenBackButtonEnabled (true);
			CuiLoadingManager::setFullscreenLoadingPercent (-1);
			CuiLoadingManager::setFullscreenLoadingString  (CuiStringIdsServer::server_connecting_game.localize ());
			m_connectionTimeout      = 0.0f;
			m_connectingToGame       = true;

			CuiLoginManager::setAllPingsDisabled ();
			const uint32 clusterId = CuiLoginManager::getConnectedClusterId ();
			if (clusterId)
				CuiLoginManager::setPingEnabled (clusterId, true);

			GameNetwork::setAcceptSceneCommand (true);

			m_autoConnected = true;
		}
		else
		{
			CuiMessageBox::createInfoBox (CuiStringIdsServer::server_err_no_character_selected.localize ());
		}

		m_proceed = false;
		m_waitForConnectionRetry = false;
	}
}

//----------------------------------------------------------------------

void SwgCuiAvatarSelection::onClusterConnection       (bool b)
{
	if (m_messageBox)
		m_messageBox->closeMessageBox ();

	if (m_messageBoxDeleteWait)
		m_messageBoxDeleteWait->closeMessageBox ();

	if (b)
	{
		if (!m_waitingForConnection)
		{
			WARNING (true, ("SwgCuiAvatarSelection received unexpected cluster connection, dropping."));
			m_dropFromCluster = true;
		}
		else
		{
			m_waitingForConnection = false;
			const uint32 clusterId = CuiLoginManager::getConnectedClusterId ();

			if (clusterId != m_waitingForClusterId)
			{
				WARNING (true, ("SwgCuiAvatarSelection received cluster connection to the wrong cluster [%d], wanted [%d], dropping.", clusterId, m_waitingForClusterId));
				m_dropFromCluster = true;
			}
			else
				m_proceed = true;
		}
	}
	else
	{
		CuiMessageBox::createInfoBox (CuiStringIdsServer::server_cluster_login_failed.localize ());
		m_selectedAvatar->clear ();
		updateAvatarSelection ();
	}
}

//----------------------------------------------------------------------

void SwgCuiAvatarSelection::onSceneChanged (bool)
{
	if (Game::getScene ())
	{
		deactivate();
		CuiLoginManager::purgeCreatures ();
		GameNetwork::disconnectLoginServer ();
	}
}

//----------------------------------------------------------------------

/**
* We've been authenticated & connected
*/

void SwgCuiAvatarSelection::onAvatarListChanged (bool)
{
	if (m_waitingLogin)
	{
		if (m_messageBoxLoginWait)
			m_messageBoxLoginWait->closeMessageBox ();

		m_waitingLogin = false;
	}

	if (m_waitingLoginForDelete)
	{
		if (m_messageBoxLoginWait)
			m_messageBoxLoginWait->closeMessageBox ();

		m_waitingLoginForSelect = false;
		m_waitingLoginForDelete = false;
		m_waitingLoginForCreate = false;
		performDelete ();
	}
	else if (m_waitingLoginForSelect)
	{
		m_waitingLoginForSelect = false;
		m_waitingLoginForCreate = false;
		m_waitingLoginForDelete = false;
		requestAvatarSelection ();
	}
	else if (m_waitingLoginForCreate)
	{
		m_waitingLoginForSelect = false;
		m_waitingLoginForDelete = false;
		m_waitingLoginForCreate = false;
		handleCreate ();
	}
}

//-----------------------------------------------------------------------

void SwgCuiAvatarSelection::receiveMessage(const MessageDispatch::Emitter &, const MessageDispatch::MessageBase & message)
{
	if(message.isType(UnnamedMessages::GameConnectionOpened))
	{
		CuiLoadingManager::setFullscreenLoadingString  (CuiStringIdsServer::server_loading_scene.localize ());
		return;
	}

	else if (message.isType (DeleteCharacterReplyMessage::MessageType))
	{
		Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
		const DeleteCharacterReplyMessage delReply (ri);

		if (m_waitingDeletion)
		{
			if (m_messageBoxDeleteWait)
				m_messageBoxDeleteWait->closeMessageBox ();

			if (m_messageBoxLoginWait)
				m_messageBoxLoginWait->closeMessageBox ();

			if (delReply.getResultCode () == DeleteCharacterReplyMessage::rc_OK)
			{
				CuiMessageBox::createInfoBox (CuiStringIdsServer::server_avatar_deleted.localize ());

				CuiLoginManager::removeAvatarFromList (*m_deletingAvatar);
				refreshList (true);
				CuiCachedAvatarManager::saveCharacterList ();
			}
			else
				CuiMessageBox::createInfoBox (CuiStringIdsServer::server_avatar_deleted_failed.localize ());

			m_deleteButton->SetEnabled (true);

			m_deletingAvatar->clear ();
			m_waitingDeletion = false;
		}
	}

	//----------------------------------------------------------------------

	else if (message.isType (CuiLoadingManager::Messages::FullscreenLoadingDisabled))
	{
		GameNetwork::setAcceptSceneCommand (false);
		m_table->SetEnabled (true);
		m_table->SetFocus   ();
		m_connectingToGame = false;
		updateAvatarSelection ();
//		reconnectLoginServer (false);
	}

	//----------------------------------------------------------------------

	else if (message.isType (Game::Messages::SCENE_CHANGED))
	{
		onSceneChanged (true);
	}

	//----------------------------------------------------------------------
	
	const CuiMessageBox::BoxMessage * const abm = dynamic_cast<const CuiMessageBox::BoxMessage *>(&message);
	
	if (abm)
	{
		if (abm->getMessageBox () == m_messageBoxDeleteWait)
		{
			m_messageBoxDeleteWait = 0;
		}
		
		else if (abm->getMessageBox () == m_messageBoxLoginWait)
		{
			m_messageBoxLoginWait = 0;
			
			if (message.isType (CuiMessageBox::Messages::COMPLETED))
			{
				const CuiMessageBox::CompletedMessage * const cm = NON_NULL (dynamic_cast<const CuiMessageBox::CompletedMessage *>(abm));
				
				//-- user closed it
				if (cm->getButtonType () != CuiMessageBox::GBT_None)
				{
					m_waitingLoginForDelete = false;
					m_waitingLogin          = false;
					m_waitingLoginForSelect = false;
					m_waitingLoginForCreate = false;
					m_table->SetEnabled (true);
					m_table->SetFocus   ();
					m_connectingToGame = false;
				}
			}
		}

		else if (abm->getMessageBox () == m_messageBox)
		{
			if (message.isType (CuiMessageBox::Messages::CLOSED))
			{
				m_connectingToGame = false;
				m_table->SetEnabled (true);
				m_table->SetFocus   ();
				m_messageBox = 0;
				//			m_waitingDeletion = false;
			}
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiAvatarSelection::clearCharacterList ()
{
	UITableModelDefault * const model = NON_NULL (safe_cast<UITableModelDefault *>(m_table->GetTableModel ()));
	model->ClearData ();

	m_objectViewer->clearObjects ();
}

//----------------------------------------------------------------------

void SwgCuiAvatarSelection::requestAvatarSelection ()
{
	if (getCurrentlySelectedAvatar ())
	{
		ConfigClientGame::setNextAutoConnectToGameServer (false);
		m_autoConnected = true;

		const CuiLoginManagerClusterInfo * const clusterInfo = CuiLoginManager::findClusterInfo (m_selectedAvatar->clusterId);

		if (clusterInfo)
		{
			if (m_waitingLoginForCreate || m_waitingLoginForDelete)
				return;

			//-- keep waiting
			if (m_waitingLogin)
			{
				m_waitingLoginForSelect = true;
				return;
			}

			m_waitingLoginForSelect = false;

			const uint32 clusterId = CuiLoginManager::getConnectedClusterId ();
			if (clusterId != 0)
			{
				if (clusterId != clusterInfo->id)
				{
					WARNING (true, ("SwgCuiAvatarSelection::requestAvatarSelection already connected to [%d], wants [%d], dropping", clusterId, clusterInfo->id));
					CuiLoginManager::disconnectFromCluster ();
				}
				else
				{
					m_proceed = true;
					m_table->SetEnabled	(false);
					m_connectingToGame = false;
					return;
				}
			}
			
			if (!GameNetwork::isConnectedToLoginServer ())
			{ 
				reconnectLoginServer (false);
				m_waitingLoginForSelect = true;
				return;
			}
			
			CuiLoginManager::connectToCluster (*clusterInfo);

			if (m_messageBox)
				m_messageBox->closeMessageBox ();

			if (!m_hasAlreadyRetriedConnection) {
				m_messageBox = CuiMessageBox::createMessageBox(CuiStringIdsServer::server_connecting_central.localize());
				m_messageBox->setRunner(true);
				m_messageBox->connectToMessages(*this);
			}
			
			m_waitingForConnection = true;
			m_waitingForClusterId = clusterInfo->id;
		}
		else
			WARNING (true, ("no cluster"));
	}
}


//----------------------------------------------------------------------

void SwgCuiAvatarSelection::requestAvatarDeletion  ()
{
	if (getCurrentlySelectedAvatar (false))
	{
		if (m_messageBox)
			m_messageBox->closeMessageBox ();

		s_avatarToDelete = *m_selectedAvatar;

		NOT_NULL (m_selectedAvatar);

		const CuiLoginManager::ClusterInfo * const clusterInfo = CuiLoginManager::findClusterInfo (s_avatarToDelete.clusterId);

		if (!clusterInfo)
			return;
		
		m_connectingToGame = false;
		m_deleteAvatarConfirmationMediator->setAvatarInfo(s_avatarToDelete);
		m_deleteAvatarConfirmationMediator->activate();
	}
}

//----------------------------------------------------------------------

bool SwgCuiAvatarSelection::getCurrentlySelectedAvatar (bool checkCluster)
{
	UITableModelDefault * const model = NON_NULL (safe_cast<UITableModelDefault *>(m_table->GetTableModel ()));
	const int row = m_table->GetLastSelectedRow ();

	Unicode::String currentlySelectedCharacter;
	if (model->GetValueAtText (row, 0, currentlySelectedCharacter) && !currentlySelectedCharacter.empty())
	{
		std::string::size_type const pos = currentlySelectedCharacter.find(s_unlockedSlotCharacterSuffix);
		if (pos != std::string::npos)
			currentlySelectedCharacter = currentlySelectedCharacter.substr(0, pos);

		const UIData * const selectedData = model->GetCellDataVisual (row, 0);

		if (!selectedData)
		{
			CuiMessageBox::createInfoBox (CuiStringIdsServer::server_err_no_character_selected.localize ());
			return false;
		}
		
		long l_clusterId = 0;
		if (!selectedData->GetPropertyLong (Properties::ClusterId, l_clusterId))
			WARNING (true, ("No clusterid"));

		const uint32 clusterId = static_cast<uint32>(l_clusterId);

		if (checkCluster)
		{						
			const CuiLoginManagerClusterInfo * const clusterInfo = CuiLoginManager::findClusterInfo (clusterId);
			
			if (!clusterInfo)
				WARNING (true, ("no clusterinfo"));
			else
			{
				if (!clusterInfo->up)
				{
					CuiMessageBox::createInfoBox (CuiStringIdsServer::server_connection_unavailable.localize ());
					return false;
				}
				else if (clusterInfo->loading)
				{
					CuiMessageBox::createInfoBox (CuiStringIdsServer::server_connection_loading.localize ());
					return false;
				}
				else if (clusterInfo->locked)
				{
					CuiMessageBox::createInfoBox (CuiStringIdsServer::server_connection_locked.localize ());
					return false;
				}
				// Players should be allowed to login with existing characters
				// but not create new characters. The cluster list will still say 
				// restricted but players can login with existing characters
				/*
				else if (clusterInfo->restricted)
				{
					CuiMessageBox::createInfoBox (CuiStringIdsServer::server_connection_restricted.localize ());
					return false;
				}
				*/
				else if (clusterInfo->isFull && !ConfigClientGame::getAllowConnectWhenFull() && !autoConnectOk())
				{
					CuiMessageBox::createInfoBox (CuiStringIdsServer::server_cluster_full.localize ());
					return false;
				}
				else if (clusterInfo->getHost().empty () || clusterInfo->getPort() == 0)
				{
					CuiMessageBox::createInfoBox (CuiStringIdsServer::server_cluster_address_missing.localize ());
					return false;
				}
			}			
		}

		const CuiLoginManagerAvatarInfo * const avatarInfo = CuiLoginManager::findAvatarInfo (clusterId, currentlySelectedCharacter);
		
		if (!avatarInfo)
			WARNING (true, ("No avatar info"));
		else
		{
			*m_selectedAvatar  = *avatarInfo;
			return true;
		}

	}

	CuiMessageBox::createInfoBox (CuiStringIdsServer::server_err_no_character_selected.localize ());

	return false;
}

//----------------------------------------------------------------------

void SwgCuiAvatarSelection::performDelete ()
{
	m_waitingDeletion = false;
	
	if (s_avatarToDelete.clusterId == 0 || s_avatarToDelete.networkId == NetworkId::cms_invalid)
		CuiMessageBox::createInfoBox (CuiStringIdsServer::server_err_no_character_selected.localize ());
	else
	{
		if (!GameNetwork::isConnectedToLoginServer ())
			reconnectLoginServer (true);
		else
		{
			const DeleteCharacterMessage delMsg (s_avatarToDelete.clusterId, s_avatarToDelete.networkId);
			GameNetwork::sendToLoginServer (delMsg, true);
			
			if (m_messageBoxDeleteWait)
				m_messageBoxDeleteWait->closeMessageBox ();
			
			m_messageBoxDeleteWait = CuiMessageBox::createMessageBox (CuiStringIdsServer::server_wait_avatar_delete.localize ());
			m_messageBoxDeleteWait->setRunner (true);
			m_messageBoxDeleteWait->connectToMessages (*this);
			
			m_waitingDeletion = true;
			*m_deletingAvatar = *m_selectedAvatar;
			
			m_deleteButton->SetEnabled (false);

			s_avatarToDelete.clear ();
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiAvatarSelection::reconnectLoginServer (bool forDelete)
{
	m_waitingLoginForSelect = false;
	m_waitingLoginForCreate = false;
	
	GameNetwork::disconnectConnectionServer ();
	GameNetwork::disconnectLoginServer      ();
	
	if (m_messageBoxLoginWait)
		m_messageBoxLoginWait->closeMessageBox ();
	
	const char* const sessionId = CuiLoginManager::getSessionIdKey ();
	
	//-- station connection
	if (sessionId)
	{
		//-- @todo request new session key so we can login
		GameNetwork::setUserPassword    (sessionId);
		GameNetwork::connectLoginServer (ConfigClientGame::getLoginServerAddress(), ConfigClientGame::getLoginServerPort());
		
		m_messageBoxLoginWait = CuiMessageBox::createMessageBox (CuiStringIdsServer::server_connecting_login.localize ());
		m_messageBoxLoginWait->setRunner (true);
		m_messageBoxLoginWait->connectToMessages (*this);
		
	}
	
	//-- direct login server connection, no station api
	else
	{
		// hook up to the loginserver
		GameNetwork::connectLoginServer (ConfigClientGame::getLoginServerAddress(), ConfigClientGame::getLoginServerPort());
		
		m_messageBoxLoginWait = CuiMessageBox::createMessageBox (CuiStringIdsServer::server_connecting_login.localize ());
		m_messageBoxLoginWait->setRunner (true);
		m_messageBoxLoginWait->connectToMessages (*this);
	}

	m_waitingLoginForDelete = forDelete;
	m_waitingLogin          = !forDelete;
}

//----------------------------------------------------------------------

void SwgCuiAvatarSelection::onClusterStatusChanged (bool)
{
	refreshList (false);
}

//----------------------------------------------------------------------

void SwgCuiAvatarSelection::onDeleteAvatarConfirmation(CuiLoginManagerAvatarInfo const &info)
{
	// Delete this character!

	s_avatarToDelete = info;

	performDelete();
}

//----------------------------------------------------------------------

void SwgCuiAvatarSelection::handleCreate ()
{
	if (m_waitingLoginForSelect || m_waitingLoginForDelete)
		return;
	
	//-- keep waiting
	if (m_waitingLogin)
	{
		m_waitingLoginForCreate = true;
		return;
	}
	
	m_waitingLoginForCreate = false;
		
	if (!GameNetwork::isConnectedToLoginServer ())
	{ 
		reconnectLoginServer (false);
		m_waitingLoginForCreate = true;
		return;
	}

	CuiTransition::startTransition(CuiMediatorTypes::AvatarSelection, CuiMediatorTypes::ClusterSelection);
	//CuiTransition::startTransition(CuiMediatorTypes::AvatarSelection, CuiMediatorTypes::AvatarSimple);
}

void SwgCuiAvatarSelection::OnCheckboxSet( UIWidget *context )
{
	UNREF(context);
	clearCharacterList();
	refreshList(true);

	CuiPreferences::setHideCharactersOnClosedGalaxies(true);
}

void SwgCuiAvatarSelection::OnCheckboxUnset( UIWidget *context )
{
	UNREF(context);
	clearCharacterList();
	refreshList(true);

	CuiPreferences::setHideCharactersOnClosedGalaxies(false);
}

// ======================================================================
