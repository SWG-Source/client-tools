// ======================================================================
//
// SwgCuiClusterSelection.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"

#include "swgClientUserInterface/SwgCuiClusterSelection.h"

#include "UIButton.h"
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
#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "clientGame/LoginConnection.h"
#include "clientUserInterface/CuiLoginManager.h"
#include "clientUserInterface/CuiLoginManagerClusterInfo.h"
#include "clientUserInterface/CuiLoginManagerClusterPing.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiStringIdsServer.h"
#include "clientUserInterface/CuiTransition.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedFoundation/ApplicationVersion.h"
#include "sharedFoundation/Branch.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/Production.h"
#include "sharedMessageDispatch/Emitter.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/ClientCentralMessages.h"
#include "sharedRandom/Random.h"
#include "swgClientUserInterface/SwgCuiAvatarCreationHelper.h"
#include "swgClientUserInterface/SwgCuiAvatarSelection.h"
#include "swgClientUserInterface/SwgCuiMediatorTypes.h"
#include <algorithm>

// ======================================================================

namespace
{
	namespace Properties
	{
		const UILowerString ClusterId = UILowerString ("ClusterId");
	}

	namespace UnnamedMessages
	{
		const char * const ConnectionClosed                 = "ConnectionClosed";
	}

	enum Colummns
	{
		C_name,
		C_online,
		C_population,
		C_timeZoneDistance,
		C_numColumns
	};

	//----------------------------------------------------------------------

	static const UILowerString prop_oldValue       = UILowerString ("OldValue");

	//----------------------------------------------------------------------

	void markServerRecommended (UIData & data, bool recommended)
	{
		static Unicode::String tmpStr;
		static const Unicode::String recommendedString = Unicode::narrowToWide ("\\>128\\#pcontrast1  (recommended)\\#.");
		static const Unicode::String sortPrefixString  = Unicode::narrowToWide ("__");

		if (recommended)
		{
			//-- mark the server as recommended
			//-- the data only has the OldValue property if the server is already marked recommended

			if (!data.HasProperty (prop_oldValue))
			{
				if (data.GetProperty (UITableModelDefault::DataProperties::Value, tmpStr))
				{
					data.SetProperty (prop_oldValue,                              tmpStr);
					tmpStr += recommendedString;
					data.SetProperty (UITableModelDefault::DataProperties::Value, tmpStr);
					tmpStr = sortPrefixString + tmpStr;
					data.SetProperty (UITableModelDefault::DataProperties::SortValue, tmpStr);
				}
			}
		}
		else
		{
			//-- unmark the server as recommended
			//-- reset the server name from the OldValue property & remove the property

			if (data.GetProperty (prop_oldValue, tmpStr))
			{
				data.SetProperty (UITableModelDefault::DataProperties::Value, tmpStr);
				data.RemoveProperty (prop_oldValue);
				data.RemoveProperty (UITableModelDefault::DataProperties::SortValue);
			}
		}
	}

	//----------------------------------------------------------------------

	bool isSeverSameVersion(const CuiLoginManagerClusterInfo & clusterInfo)
	{
		return (!clusterInfo.branch.empty() && clusterInfo.netVersionMatch && clusterInfo.version == (uint)atoi(ApplicationVersion::getPublicVersion()));
	}

	//----------------------------------------------------------------------

	int getTimeZoneDistance(int remoteTimeZoneSeconds)
	{
		int myTimeZoneSeconds = Clock::getTimeZone();
		int distance = abs(myTimeZoneSeconds - remoteTimeZoneSeconds);
		if(distance > 12 * 60 * 60)
		{
			distance = 24 * 60 * 60 - distance;
		}
		
		if(distance > 0)
		{
			distance = distance / 60 / 60;
		}
		return distance;
	}

	//----------------------------------------------------------------------

	int getAdjustedTimeZoneForCluster(std::string const & clusterName, int clusterTimeZone)
	{
		// the time zone we get for the cluster is based on the time zone that is set
		// on the box that the CentralServer for that cluster is running on; for live
		// services (sdk, abl, ams, and jpe clusters), this is set to Pacific Time
		// (GMT - 8:00) even though the abl (GMT - 5:00), ams (GMT + 1:00), and
		// jpe (GMT + 9:00) clusters are not physically located in the Pacific Time Zone;
		// in order to accurately calculate the distance from the client PC to the cluster,
		// we must use the actual time zone where the clusters are physically located;
		// to do this, we'll just define the actual time zone for the live clusters
		// here, and use that information in calculating the time zone distance
		static std::map<std::string, int> sTimeZoneAdjustmentInfo;

		if (sTimeZoneAdjustmentInfo.empty())
		{
			// this information could be put into a config file or a data table,
			// but in reality, the information will rarely, if ever, change, so
			// we save the hassle and just hard-code it here.

			// sdk Pacific (GMT - 8:00)
			static const int pacificTimeZone = -8 * 60 * 60;
			sTimeZoneAdjustmentInfo[std::string("Bria")] = pacificTimeZone;
			sTimeZoneAdjustmentInfo[std::string("Ahazi")] = pacificTimeZone;
			sTimeZoneAdjustmentInfo[std::string("Corbantis")] = pacificTimeZone;
			sTimeZoneAdjustmentInfo[std::string("Flurry")] = pacificTimeZone;
			sTimeZoneAdjustmentInfo[std::string("Intrepid")] = pacificTimeZone;
			sTimeZoneAdjustmentInfo[std::string("Kettemoor")] = pacificTimeZone;
			sTimeZoneAdjustmentInfo[std::string("Naritus")] = pacificTimeZone;
			sTimeZoneAdjustmentInfo[std::string("Scylla")] = pacificTimeZone;
			sTimeZoneAdjustmentInfo[std::string("Sunrunner")] = pacificTimeZone;
			sTimeZoneAdjustmentInfo[std::string("Valcyn")] = pacificTimeZone;
			sTimeZoneAdjustmentInfo[std::string("Tempest")] = pacificTimeZone;

			// abl Eastern (GMT - 5:00)
			static const int easternTimeZone = -5 * 60 * 60;
			sTimeZoneAdjustmentInfo[std::string("Bloodfin")] = easternTimeZone;
			sTimeZoneAdjustmentInfo[std::string("Chilastra")] = easternTimeZone;
			sTimeZoneAdjustmentInfo[std::string("Eclipse")] = easternTimeZone;
			sTimeZoneAdjustmentInfo[std::string("Gorath")] = easternTimeZone;
			sTimeZoneAdjustmentInfo[std::string("Kauri")] = easternTimeZone;
			sTimeZoneAdjustmentInfo[std::string("Lowca")] = easternTimeZone;
			sTimeZoneAdjustmentInfo[std::string("Radiant")] = easternTimeZone;
			sTimeZoneAdjustmentInfo[std::string("Starsider")] = easternTimeZone;
			sTimeZoneAdjustmentInfo[std::string("Tarquinas")] = easternTimeZone;
			sTimeZoneAdjustmentInfo[std::string("Wanderhome")] = easternTimeZone;
			sTimeZoneAdjustmentInfo[std::string("Shadowfire")] = easternTimeZone;

			// ams Amsterdam (GMT + 1:00)
			static const int amsterdamTimeZone = +1 * 60 * 60;
			sTimeZoneAdjustmentInfo[std::string("Europe-Chimaera")] = amsterdamTimeZone;
			sTimeZoneAdjustmentInfo[std::string("Europe-FarStar")] = amsterdamTimeZone;
			sTimeZoneAdjustmentInfo[std::string("Europe-Infinity")] = amsterdamTimeZone;

			// jpe Japan (GMT + 9:00)
			static const int japanTimeZone = +9 * 60 * 60;
			sTimeZoneAdjustmentInfo[std::string("Japan-Katana")] = japanTimeZone;
			sTimeZoneAdjustmentInfo[std::string("Japan-Harla")] = japanTimeZone;
		}

		std::map<std::string, int>::const_iterator iter = sTimeZoneAdjustmentInfo.find(clusterName);
		if (iter != sTimeZoneAdjustmentInfo.end())
			return iter->second;
		else
			return clusterTimeZone;
	}

	static const Unicode::String::value_type *s_colorRed     = L"\\#ff0000";
	static const Unicode::String::value_type *s_colorGreen   = L"\\#00ff00";
	static const Unicode::String::value_type *s_colorBlue    = L"\\#0000ff";
	static const Unicode::String::value_type *s_colorMagenta = L"\\#ff00ff";
	static const Unicode::String::value_type *s_colorYellow  = L"\\#ffff00";
	static const Unicode::String::value_type *s_colorCyan    = L"\\#00ffff";
	static const Unicode::String::value_type *s_colorWhite   = L"\\#ffffff";
	static const Unicode::String::value_type *s_colorBlack   = L"\\#000000";
}

//----------------------------------------------------------------------

SwgCuiClusterSelection::SwgCuiClusterSelection (UIPage & page) :
CuiMediator              ("SwgCuiClusterSelection", page),
UIEventCallback          (),
MessageDispatch::Receiver(),
m_okButton               (0),
m_cancelButton           (0),
m_changeGalaxy(0),
m_main(0),
m_main2(0),
m_recommendedGalaxy(0),
m_messageBox             (0),
m_autoConnected          (false),
m_table                  (0),
m_model                  (0),
m_sampleBar              (0),
m_performancePages       (new PageVector),
m_proceed                (false),
m_callback               (new MessageDispatch::Callback),
m_waitingForConnection   (false),
m_dropFromCluster        (false),
m_waitingForClusterId    (0),
m_hasRecommended         (false)
{

	static bool s_installed = false;

	if (!s_installed)
		s_installed = true;

	getCodeDataObject (TUIButton,     m_okButton,          "buttonNext");
	getCodeDataObject (TUIButton,     m_cancelButton,      "buttonPrev");
	getCodeDataObject (TUIButton,     m_changeGalaxy,      "changeGalaxy");
	getCodeDataObject (TUIPage,       m_main,              "main");
	getCodeDataObject (TUIPage,       m_main2,             "main2");
	getCodeDataObject (TUIText,       m_recommendedGalaxy, "recommendedGalaxy");

	getCodeDataObject (TUITable,      m_table,        "table");
	m_model = NON_NULL (safe_cast<UITableModelDefault *>(m_table->GetTableModel ()));

	m_model->Attach (0);

	DEBUG_FATAL (m_model->GetColumnCount () != C_numColumns, ("bad cols"));

	m_table->SetVisible (true);

	getCodeDataObject (TUIPage,      m_sampleBar,        "sampleBar");
	m_sampleBar->SetVisible (false);

	registerMediatorObject(*m_changeGalaxy, true);
}

//-----------------------------------------------------------------

SwgCuiClusterSelection::~SwgCuiClusterSelection ()
{
	delete m_callback;
	m_callback = 0;

	clearPerformancePages ();

	delete m_performancePages;
	m_performancePages = 0;

	m_okButton       = 0;
	m_cancelButton   = 0;
	m_changeGalaxy = 0;
	m_main = 0;
	m_main2 = 0;
	m_recommendedGalaxy = 0;
	m_messageBox     = 0;
	m_sampleBar      = 0;
	m_table          = 0;
	m_model->Detach (0);
	m_model          = 0;
}

//-----------------------------------------------------------------

void SwgCuiClusterSelection::performActivate ()
{
	SwgCuiAvatarCreationHelper::setCreatingJedi (false);

	m_timeElapsed = 0.0f;
	m_hasRecommended = false;

	m_waitingForConnection   = false;
	m_dropFromCluster        = false;
	m_waitingForClusterId    = 0;

	m_callback->connect (*this, &SwgCuiClusterSelection::onClusterConnection,      static_cast<CuiLoginManager::Messages::ClusterConnection *>     (0));
	m_callback->connect (*this, &SwgCuiClusterSelection::onClusterStatusChanged,   static_cast<CuiLoginManager::Messages::ClusterStatusChanged*>   (0));

	CuiLoginManager::disconnectFromCluster ();

	connectToMessage (UnnamedMessages::ConnectionClosed);

	setPointerInputActive  (true);
	setKeyboardInputActive (true);
	setInputToggleActive   (false);

	m_okButton->AddCallback      (this);
	m_cancelButton->AddCallback  (this);
	m_table->AddCallback         (this);

	refreshList ();

	m_model->sortOnColumn (C_timeZoneDistance,      UITableModel::SD_up);
	m_model->sortOnColumn (C_population,  UITableModel::SD_up);
	m_model->sortOnColumn (C_online,      UITableModel::SD_up);

	m_model->clearSortingHistory ();

	m_main->SetVisible(false);
	m_main2->SetVisible(true);

	setIsUpdating (true);

	CuiTransition::signalTransitionReady (CuiMediatorTypes::ClusterSelection);
}

//-----------------------------------------------------------------

void SwgCuiClusterSelection::performDeactivate ()
{
	setIsUpdating (false);

	if (m_messageBox)
		m_messageBox->closeMessageBox ();
	m_messageBox = 0;

	m_callback->disconnect (*this, &SwgCuiClusterSelection::onClusterConnection,      static_cast<CuiLoginManager::Messages::ClusterConnection *>     (0));
	m_callback->disconnect (*this, &SwgCuiClusterSelection::onClusterStatusChanged,   static_cast<CuiLoginManager::Messages::ClusterStatusChanged*>   (0));

	clearPerformancePages ();

	disconnectAll();

	m_okButton->RemoveCallback      (this);
	m_cancelButton->RemoveCallback  (this);
	m_table->RemoveCallback         (this);

	CuiLoginManager::setAllPingsDisabled ();
}

//-----------------------------------------------------------------

void SwgCuiClusterSelection::refreshList ()
{
	if (Game::getSinglePlayer ())
		CuiLoginManager::simulateClusters ();

	clearPerformancePages ();
	m_model->ClearData ();

	CuiLoginManager::ClusterInfoVector civ;
	CuiLoginManager::getClusterInfo (civ);

	const std::string clientBranch(Branch().getBranchName());

	for (CuiLoginManager::ClusterInfoVector::const_iterator it = civ.begin (); it != civ.end (); ++it)
	{
		const CuiLoginManagerClusterInfo & clusterInfo = *it;

#if PRODUCTION == 1
		if (clusterInfo.disableCharacterCreation)
			continue;
#endif

		Unicode::String name(Unicode::narrowToWide(clusterInfo.name));

#if PRODUCTION != 1
		Unicode::String color = s_colorRed; // red is the default color

		if (!clusterInfo.branch.empty())
		{
			if (clusterInfo.netVersionMatch == true)
			{
				if (clusterInfo.branch == clientBranch)
				{
					if (clusterInfo.version == (uint)atoi(ApplicationVersion::getPublicVersion()))
					{
						name = s_colorGreen + name;
						color = s_colorGreen;
					}
					else
					{
						name = s_colorMagenta + name;
						color = s_colorMagenta;
					}
				}
				else
				{
					color = s_colorYellow;
				}
			}

			char buffer[64];
			name = color + name + L" [" + Unicode::narrowToWide(clusterInfo.branch) + L"." + Unicode::narrowToWide(_itoa(clusterInfo.version, buffer, 10)) +  L"]";
		}
#endif

		UIData * const dataCells [C_numColumns] =
		{
			m_model->AppendCell (C_name,        clusterInfo.name.c_str (), name),
			m_model->AppendCell (C_timeZoneDistance, 0,                    Unicode::emptyString),
			m_model->AppendCell (C_online,      0,                         Unicode::emptyString),
			m_model->AppendCell (C_population,  0,                         Unicode::emptyString)
		};

		dataCells [C_name]->SetPropertyLong (Properties::ClusterId, clusterInfo.id);

		//-----------------------------------------------------------------
		//-- see if we can autoconnect to this one

		if (!m_autoConnected && ConfigClientGame::getAutoConnectToCentralServer ())
		{
			const std::string & defaultServer = ConfigClientGame::getCentralServerName ();
			if (!_stricmp (defaultServer.c_str (), clusterInfo.name.c_str ()))
			{
				m_autoConnected = true;
				m_table->SelectRow (static_cast<long>(m_model->GetRowCount ()) - 1L);
				m_okButton->Press ();
				break;
			}
		}
	} //lint !e429 //data

	updateServerStatus ();

	//TODO smart updating of selected row if nothing selected (to handle clusters going up/down, autoselect based on population, etc.)
}

//----------------------------------------------------------------------

void SwgCuiClusterSelection::updateServerStatus ()
{
	const bool oldHasRecommended = m_hasRecommended;
	m_hasRecommended = true;

	m_table->SetVisible (m_hasRecommended);

	m_model->clearSortingHistory ();

	const UITableModel::SortStateVector ssv = m_model->getSortStateVector ();

	CuiLoginManager::ClusterInfoVector civ;
	CuiLoginManager::getClusterInfo (civ);

	m_table->SetIgnoreDataChanges (true);

	{
		int row = 0;
		Unicode::String statusStr;

		for (CuiLoginManager::ClusterInfoVector::const_iterator it = civ.begin (); it != civ.end (); ++it)
		{
			const CuiLoginManagerClusterInfo & clusterInfo = *it;

#if PRODUCTION == 1
			if (clusterInfo.disableCharacterCreation)
				continue;
#endif

			const int visualRow = m_model->GetVisualDataRowIndex (row);
			UIData * const dataNameCell = NON_NULL (m_model->GetCellDataLogical (row, C_name));

			if (dataNameCell)
			{
				long cellId = 0;
				if (!dataNameCell->GetPropertyLong (Properties::ClusterId, cellId) || cellId != static_cast<int>(clusterInfo.id))
					DEBUG_FATAL (true, ("bad table cell"));

				Unicode::String statusSortStr;

				//-- cluster online status
				if (clusterInfo.up)
				{
					if (clusterInfo.loading)
					{
						statusStr     = CuiStringIdsServer::server_loading.localize ();
						statusSortStr.push_back ('2');
					}
					else if (clusterInfo.locked)
					{
						statusStr     = CuiStringIdsServer::server_locked.localize ();
						statusSortStr.push_back ('3');
					}
					else if (clusterInfo.isFull)
					{
						statusStr     = CuiStringIdsServer::server_cluster_full.localize ();
						statusSortStr.push_back ('4');
					}
					else if (clusterInfo.restricted)
					{
						statusStr     = CuiStringIdsServer::server_restricted.localize ();
						statusSortStr.push_back ('5');
					}
					else
					{
						statusStr = CuiStringIdsServer::server_online.localize ();
						if (clusterInfo.notRecommended)
							statusSortStr.push_back ('1');
						else
							statusSortStr.push_back ('0');
					}
				}
				else
				{
					statusStr = CuiStringIdsServer::server_offline.localize ();
					statusSortStr.push_back ('6');
				}

				m_model->SetValueAtText     (visualRow, C_online, statusStr);
				m_model->SetSortKeyAtString (visualRow, C_online, statusSortStr);

				m_model->SetValueAtInteger     (visualRow, C_timeZoneDistance, getTimeZoneDistance(getAdjustedTimeZoneForCluster(clusterInfo.name, clusterInfo.timeZone)));

				char buf[64];
				Unicode::String populationSortString;
				if (clusterInfo.up)
				{
					Unicode::String populationString;

					if ((clusterInfo.populationStatus == LoginClusterStatus::ClusterData::PS_full) || clusterInfo.isFull)
						populationString = CuiStringIdsServer::server_cluster_table_population_full.localize();
					else if (clusterInfo.populationStatus == LoginClusterStatus::ClusterData::PS_extremely_heavy)
						populationString = CuiStringIdsServer::server_cluster_table_population_extremely_heavy.localize();
					else if (clusterInfo.populationStatus == LoginClusterStatus::ClusterData::PS_very_heavy)
						populationString = CuiStringIdsServer::server_cluster_table_population_very_heavy.localize();
					else if (clusterInfo.populationStatus == LoginClusterStatus::ClusterData::PS_heavy)
						populationString = CuiStringIdsServer::server_cluster_table_population_heavy.localize();
					else if (clusterInfo.populationStatus == LoginClusterStatus::ClusterData::PS_medium)
						populationString = CuiStringIdsServer::server_cluster_table_population_medium.localize();
					else if (clusterInfo.populationStatus == LoginClusterStatus::ClusterData::PS_light)
						populationString = CuiStringIdsServer::server_cluster_table_population_light.localize();
					else if (clusterInfo.populationStatus == LoginClusterStatus::ClusterData::PS_very_light)
						populationString = CuiStringIdsServer::server_cluster_table_population_very_light.localize();

					// if LoginServer sent actual cluster population, display it
					if (clusterInfo.population >= 0)
					{
						snprintf(buf, sizeof(buf)-1, " (%d)", clusterInfo.population);
						buf[sizeof(buf)-1] = '\0';
						populationString += Unicode::narrowToWide(buf);
					}

					snprintf(buf, sizeof(buf)-1, "%02d", (((clusterInfo.populationStatus == LoginClusterStatus::ClusterData::PS_full) || clusterInfo.isFull) ? static_cast<int>(LoginClusterStatus::ClusterData::PS_full) : static_cast<int>(clusterInfo.populationStatus)));
					buf[sizeof(buf)-1] = '\0';
					populationSortString = Unicode::narrowToWide(buf);

#if PRODUCTION == 0
					if (clusterInfo.disableCharacterCreation)
					{
						populationString += Unicode::narrowToWide(" (Character Creation Disabled)");
					}
#endif

					m_model->SetValueAtText  (visualRow, C_population, populationString);
				}
				else
				{
					m_model->SetValueAtText  (visualRow, C_population, Unicode::emptyString);

					snprintf(buf, sizeof(buf)-1, "%02d", (static_cast<int>(LoginClusterStatus::ClusterData::PS_HIGHEST) + 1));
					buf[sizeof(buf)-1] = '\0';
					populationSortString = Unicode::narrowToWide(buf);
				}

				m_model->SetSortKeyAtString (visualRow, C_population, populationSortString);
			}

			++row;
		}
	}

	//-- restore the previous sort order unless we just marked the recommended servers

	if (oldHasRecommended || !m_hasRecommended)
	{
		m_table->SetIgnoreSortingChanges (true);
		m_model->setSortStateVector (ssv);
		m_table->SetIgnoreSortingChanges (false);
	}
	else if (m_hasRecommended)
	{
		//-- resort the list the first time we recommend servers
		m_model->sortOnColumn (C_timeZoneDistance,  UITableModel::SD_up);
		m_model->sortOnColumn (C_population,  UITableModel::SD_up);
		m_model->sortOnColumn (C_online,      UITableModel::SD_up);

		const int numRows = m_model->GetRowCount ();
		int numRecommended = 0;

		unsigned int minimumPopulation = INT_MAX;
		std::string smallestPopulationName;
		int smallestPopulationRow = -1;
		UIData * smallestPopulationRowData = 0;

		static const int max_recommended = 5;
		for (int i = 0; i < numRows; ++i)
		{
			UIData * const data = m_model->GetCellDataVisual (i, C_name);
			NOT_NULL (data);
			if (data)
			{
				bool isRecommended = false;

				if (numRecommended < max_recommended)
				{
					long clusterId = 0L;
					data->GetPropertyLong (Properties::ClusterId, clusterId);
					const CuiLoginManagerClusterInfo * const clusterInfo = CuiLoginManager::findClusterInfo (static_cast<uint32>(clusterId));

					if (clusterInfo
#if PRODUCTION != 1
								// in non-prod mode, only clusters with same version are applicable
							&& isSeverSameVersion(*clusterInfo)
#endif
						)
					{
						if (clusterInfo->up && !clusterInfo->notRecommended && !clusterInfo->loading 
							&& !clusterInfo->locked && !clusterInfo->restricted && !clusterInfo->isFull)
						{
							if (static_cast<unsigned int>(clusterInfo->populationStatus) < minimumPopulation)
							{
								minimumPopulation = static_cast<unsigned int>(clusterInfo->populationStatus);
								smallestPopulationName = clusterInfo->name;
								smallestPopulationRowData = data;
								smallestPopulationRow = i;
							}

							if (static_cast<int>(clusterInfo->populationStatus) <= static_cast<int>(LoginClusterStatus::ClusterData::PS_medium))
							{
								//-- randomly recommend 1/2 of qualified servers so that players won't always pile into the same one
								if (Random::random (0, 1))
								{
									isRecommended = true;
								}
							}
						}

						if ((i == 0) && (smallestPopulationRowData == 0))
						{
							minimumPopulation = static_cast<unsigned int>(clusterInfo->populationStatus);
							smallestPopulationName = clusterInfo->name;
							smallestPopulationRowData = data;
							smallestPopulationRow = i;
						}
					}
				}

				if (isRecommended)
				{
					++numRecommended;
				}

				markServerRecommended(*data, isRecommended);
			}
		}

		//-- first time we recommend servers, select the server with the least population
		if (!oldHasRecommended)
		{
			if (smallestPopulationRowData != 0)
			{
				markServerRecommended(*smallestPopulationRowData, true);
			}

			smallestPopulationRow = std::max (0, smallestPopulationRow);
			m_table->SelectRow(smallestPopulationRow);
			Unicode::String name(Unicode::narrowToWide(smallestPopulationName));
			m_recommendedGalaxy->SetLocalText(name);
		}
	}

	m_table->SetIgnoreDataChanges (false);
}

//-----------------------------------------------------------------

void SwgCuiClusterSelection::OnButtonPressed(UIWidget *context)
{
	if (context == m_okButton)
	{
		const int lastSelectedRow = m_table->GetLastSelectedRow ();
		const UIData * const selectedData = m_model->GetCellDataVisual      (lastSelectedRow, C_name);

		if (!selectedData)
		{
			CuiMessageBox::createInfoBox (CuiStringIdsServer::server_err_no_cluster_selected.localize ());
			return;
		}

		long l_clusterId;
		if (!selectedData->GetPropertyLong (Properties::ClusterId, l_clusterId))
			WARNING (true, ("No clusterid"));

		const uint32 clusterId = static_cast<uint32>(l_clusterId);

		const CuiLoginManagerClusterInfo * const clusterInfo = CuiLoginManager::findClusterInfo (clusterId);

		if (clusterInfo)
		{
			if (!clusterInfo->up)
			{
				CuiMessageBox::createInfoBox (CuiStringIdsServer::server_connection_unavailable.localize ());
			}
			else if (clusterInfo->loading)
			{
				CuiMessageBox::createInfoBox (CuiStringIdsServer::server_connection_loading.localize ());
			}
			else if (clusterInfo->locked)
			{
				CuiMessageBox::createInfoBox (CuiStringIdsServer::server_connection_locked.localize ());
			}
			else if (clusterInfo->restricted)
			{
				CuiMessageBox::createInfoBox (CuiStringIdsServer::server_connection_restricted.localize ());
			}
			else if (clusterInfo->isFull && !ConfigClientGame::getAllowConnectWhenFull())
			{
				CuiMessageBox::createInfoBox (CuiStringIdsServer::server_cluster_full.localize ());
			}
			else if (clusterInfo->getHost().empty () || clusterInfo->getPort() == 0)
			{
				CuiMessageBox::createInfoBox (CuiStringIdsServer::server_cluster_address_missing.localize ());
			}
			else
			{
				const uint32 clusterId = CuiLoginManager::getConnectedClusterId ();
				if (clusterId != 0)
				{
					if (clusterId != clusterInfo->id)
					{
						WARNING (true, ("SwgCuiClusterSelection::requestAvatarSelection already connected to [%d], wants [%d], dropping", clusterId, clusterInfo->id));
						CuiLoginManager::disconnectFromCluster ();
					}
					else
					{
						m_proceed = true;
						return;
					}
				}

				m_waitingForConnection = true;
				m_waitingForClusterId = clusterInfo->id;

				CuiLoginManager::connectToCluster (*clusterInfo);

				if (m_messageBox)
					m_messageBox->closeMessageBox ();

				m_messageBox = CuiMessageBox::createWaitingBox (CuiStringIdsServer::server_connecting_central.localize ());
				m_messageBox->setRunner (true);
				m_messageBox->connectToMessages (*this);
			}

		}
		else
			WARNING (true, ("no clusterinfo"));

	}
	else if (context == m_cancelButton)
	{
		if (CuiLoginManager::getAvatarCount () == 0)
		{
			CuiTransition::startTransition (CuiMediatorTypes::ClusterSelection, CuiMediatorTypes::AvatarSelection);
		}
		else
		{
			GameNetwork::disconnectConnectionServer ();
			CuiTransition::startTransition (CuiMediatorTypes::ClusterSelection, CuiMediatorTypes::AvatarSelection);
		}
	}
	else if (context == m_changeGalaxy)
	{
		m_main->SetVisible(true);
		m_main2->SetVisible(false);
	}
} //lint !e818 //stfu noob

//-----------------------------------------------------------------

void SwgCuiClusterSelection::OnGenericSelectionChanged (UIWidget *context)
{
	if (context == m_table)
	{
	}
}

//----------------------------------------------------------------------

void SwgCuiClusterSelection::update (float deltaTimeSecs)
{
	CuiMediator::update (deltaTimeSecs);

	m_timeElapsed += deltaTimeSecs;

	if (m_dropFromCluster)
	{
		WARNING (true, ("SwgCuiClusterSelection dropping cluster connection"));
		CuiLoginManager::disconnectFromCluster ();
		m_dropFromCluster = false;
		return;
	}

	if (m_proceed)
	{
		const uint32 clusterId = CuiLoginManager::getConnectedClusterId ();
		const CuiLoginManagerClusterInfo * const clusterInfo = CuiLoginManager::findClusterInfo (clusterId);

		if (!clusterInfo)
			WARNING (true, ("no cluster [%d]", clusterId));
		else if (!CuiLoginManager::canCreateCharacter ())
		{
			CuiMessageBox::createInfoBox (CuiStringIdsServer::server_character_creation_max_chars.localize ());
			m_dropFromCluster = true;
			m_proceed = false;
			return;
		}

		CuiTransition::startTransition(CuiMediatorTypes::ClusterSelection, CuiMediatorTypes::AvatarSimple);
		//CuiTransition::startTransition(CuiMediatorTypes::ClusterSelection, CuiMediatorTypes::AvatarCreation);

		GameNetwork::disconnectLoginServer ();

		if (clusterId)
			CuiLoginManager::setPingEnabled (clusterId, true);

		m_proceed = false;
	}
	else
	{
		static float next_update = 0.0f;
		static const float update_threshold = 2.0f;
		const float curTime = Game::getElapsedTime ();

		if (curTime > next_update)
		{
			updateServerStatus ();
			next_update = curTime + update_threshold;

		}
	}
}

//----------------------------------------------------------------------

void SwgCuiClusterSelection::onClusterConnection       (bool b)
{
	if (m_messageBox)
		m_messageBox->closeMessageBox ();

	if (b)
	{

		if (!m_waitingForConnection)
		{
			WARNING (true, ("SwgCuiClusterSelection received unexpected cluster connection, dropping."));
			m_dropFromCluster = true;
		}
		else
		{
			m_waitingForConnection = false;
			const uint32 clusterId = CuiLoginManager::getConnectedClusterId ();

			if (clusterId != m_waitingForClusterId)
			{
				WARNING (true, ("SwgCuiClusterSelection received cluster connection to the wrong cluster [%d], wanted [%d], dropping.", clusterId, m_waitingForClusterId));
				m_dropFromCluster = true;
			}
			else
			{
				const bool canCreateJedi             = CuiLoginManager::canCreateJedi ();
				const bool canCreateRegularCharacter = CuiLoginManager::canCreateRegularCharacter ();

				SwgCuiAvatarCreationHelper::setCreatingJedi (canCreateJedi && !canCreateRegularCharacter);

				m_proceed = true;
			}
		}
	}
	else
		CuiMessageBox::createInfoBox (CuiStringIdsServer::server_cluster_login_failed.localize ());
}

//-----------------------------------------------------------------------

void SwgCuiClusterSelection::receiveMessage (const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message)
{
	if (message.isType (UnnamedMessages::ConnectionClosed))
	{
		if (!CuiTransition::isTransitioning ())
		{
			// todo need a better way to determine if we should clear the cluster servers, since we no longer have a central connection
			if (dynamic_cast<const LoginConnection*>(&source))
			{
				CuiTransition::startTransition (CuiMediatorTypes::ClusterSelection, CuiMediatorTypes::AvatarSelection);
			}
			return;
		}
	}

	//----------------------------------------------------------------------

	const CuiMessageBox::BoxMessage * const abm = dynamic_cast<const CuiMessageBox::BoxMessage *>(&message);

	if (abm)
	{
		if (message.isType (CuiMessageBox::Messages::CLOSED))
		{
			if (abm->getMessageBox () == m_messageBox)
				m_messageBox = 0;
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiClusterSelection::clearPerformancePages ()
{
	for (PageVector::iterator it = m_performancePages->begin (); it != m_performancePages->end (); ++it)
	{
		UIPage * const page = NON_NULL (*it);
		getPage ().RemoveChild (page);
		page->Detach (0);
	}

	m_performancePages->clear ();

	//-- todo: reset the table model's pointers to widgets
}

//----------------------------------------------------------------------

void SwgCuiClusterSelection::onClusterStatusChanged (bool)
{
	refreshList ();
}

//----------------------------------------------------------------------

bool SwgCuiClusterSelection::OnMessage (UIWidget * context, const UIMessage & msg)
{
	if (context == m_table)
	{
		if (msg.Type == UIMessage::LeftMouseDoubleClick)
		{

			const int lastSelectedRow = m_table->GetLastSelectedRow ();
			const UIData * const selectedData = m_model->GetCellDataVisual      (lastSelectedRow, C_name);
			if (selectedData)
				m_okButton->Press ();
			return false;
		}
	}

	return true;
}

// ======================================================================
