// ============================================================================
//
// PlanetWatcher.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstPlanetWatcher.h"
#include "PlanetWatcher.h"
#include "PlanetWatcher.moc"

#include "AddFilterDialog.h"
#include "ClusterSelectionDialog.h"
#include "ColorTableItem.h"
#include "FilterManager.h"
#include "ObjectLocatorDialog.h"
#include "PlanetServerConnection.h"
#include "PlanetWatcherRenderer.h"
#include "PlanetWatcherUtility.h"
#include "ServerConnectionDialog.h"
#include "serverGame/AiMovementBase.h"
#include "sharedCompression/SetupSharedCompression.h"
#include "sharedDebug/PerformanceTimer.h"
#include "sharedDebug/SetupSharedDebug.h"
#include "sharedFile/SetupSharedFile.h"
#include "sharedFoundation/SetupSharedFoundation.h"
#include "sharedNetwork/SetupSharedNetwork.h"
#include "sharedNetwork/NetworkHandler.h"
#include "sharedNetwork/NetworkSetupData.h"
#include "sharedRandom/SetupSharedRandom.h"
#include "sharedRandom/Random.h"
#include "sharedThread/SetupSharedThread.h"
#include "sharedNetworkMessages/LoginClusterStatus.h"
#include "sharedNetworkMessages/SetupSharedNetworkMessages.h"

#include <algorithm>
#include <list>
#include <map>
#include <vector>

// ============================================================================
//
// PlanetWatcherNamespace
//
// ============================================================================

//-----------------------------------------------------------------------------
namespace PlanetWatcherNamespace
{
	typedef std::list<int> LastLoopTimes;

	PlanetWatcher::ServerProcesses s_serverProcesses;
	PlanetWatcher::ServerProcesses s_plotterServerProcesses;
	int           s_physicalFrame = 0;
	int           s_visualFrame = 0;
	int           s_anchoredFrame = 0;
	LastLoopTimes s_lastLoopTimes;
	int           s_framesToDisplay = 0;
	int           s_maxScrollFrames = 400;
	int           s_autoMaxMsCount = 100;
	bool          s_snapToCurrentFrame = true;
	QPoint        s_mouseClickPosition_g(0, 0);
	bool          s_showInformation = false;
	int           s_minFrameTime = 0;
	int           s_maxFrameTime = 0;
	unsigned int  s_killClockTime = 0;
	int           s_currentFilterRow = 0;

	void setPenColor(QPainter &painter, unsigned int const timeElapsed);
	void checkTableItem(QTable & table, int row, int column, bool checked);
	bool isTableItemChecked(QTable & table, int row, int column);

	const char * const s_allObjectsType = "All objects";
	const int     s_allObjectsTypeTag = 0;

	const int    s_levelMin = 1;
	const int    s_levelMax = 80;

	const int    s_numberOfServerDataRows = 4;
	LoginConnection *s_loginConnection = 0;

	std::string const cms_abt_unknownString = "Unknown";
	std::string const cms_abt_idleString = "Idle";
	std::string const cms_abt_loiterString = "Loiter";
	std::string const cms_abt_wanderString = "Wander";
	std::string const cms_abt_followString = "Follow";
	std::string const cms_abt_fleeString = "Flee";
	std::string const cms_abt_moveString = "Move";
	std::string const cms_abt_patrolString = "Patrol";
	std::string const cms_abt_faceString = "Face";
	std::string const cms_abt_swarmString = "Swarm";
	std::string const cms_abt_invalidString = "Invalid";

	std::vector<std::string> ms_allValidAiActivities;

	std::vector<std::string> const & getAllValidAiActivities()
	{
		return ms_allValidAiActivities;
	}
};

using namespace PlanetWatcherNamespace;

// ============================================================================
//
// PlanetWatcherNamespace
//
// ============================================================================

//-----------------------------------------------------------------------------
void PlanetWatcherNamespace::setPenColor(QPainter &painter, unsigned int const timeElapsed)
{
	if (timeElapsed == 0)
	{
		painter.setPen(QColor(0, 128, 0));
	}
	else if (timeElapsed == 1)
	{
		painter.setPen(QColor(0, 96, 0));
	}
	else if (timeElapsed == 2)
	{
		painter.setPen(QColor(0, 64, 0));
	}
	else if (timeElapsed == 3)
	{
		painter.setPen(QColor(0, 32, 0));
	}
	else
	{
		painter.setPen(QColor("black"));
	}
}

//-----------------------------------------------------------------------------
void PlanetWatcherNamespace::checkTableItem(QTable & table, int row, int column, bool checked)
{
	QTableItem * tableItem = table.item(row, column);
	if (tableItem && (tableItem->rtti() == 2))
		(dynamic_cast<QCheckTableItem *>(tableItem))->setChecked(checked);
}

//-----------------------------------------------------------------------------
bool PlanetWatcherNamespace::isTableItemChecked(QTable & table, int row, int column)
{
	QTableItem * tableItem = table.item(row, column);
	if (tableItem && (tableItem->rtti() == 2))
		return (dynamic_cast<QCheckTableItem *>(tableItem))->isChecked();

	return false;
}

// ============================================================================
//
// PlanetWatcher::GameServerInfo
//
// ============================================================================

class PlanetWatcher::GameServerInfo
{
public:

	GameServerInfo()
	 : m_ip()
	 , m_port()
	 , m_sceneId()
	{
	}

	QString m_ip;
	QString m_port;
	QString m_sceneId;
};

// ============================================================================
//
// PlanetWatcher::ProcessInfo
//
// ============================================================================

class PlanetWatcher::ProcessInfo
{
public:

	ProcessInfo()
	 : m_averageFrameTime(0.0f)
	 , m_frameInfo()
	 , m_firstVisualFrame(0)
	{
	}

	float                  m_averageFrameTime;
	int                    m_firstVisualFrame;
	std::vector<FrameInfo> m_frameInfo;
};

// ============================================================================
//
// PlanetWatcher::FrameInfo
//
// ============================================================================

class PlanetWatcher::FrameInfo
{
public:

	FrameInfo()
	 : m_frameTime(0)
	 , m_profileInfo()
	{
	}

	unsigned int m_clockTime;
	int          m_frameTime;
	QString      m_profileInfo;
};

// ============================================================================
//
// PlanetWatcher
//
// ============================================================================

QColor const PlanetWatcher::ms_selectedObjects(QColor(255, 255, 255));
QColor const PlanetWatcher::ms_mobileObjects(QColor(0, 255, 0));
QColor const PlanetWatcher::ms_mobileInterestRadiusObjects(QColor(255, 128, 255));
QColor const PlanetWatcher::ms_neverMovedNonCreatureObjects(QColor(226, 113, 0));
QColor const PlanetWatcher::ms_neverMovedCreatureObjects(QColor(28, 255, 250));

//-----------------------------------------------------------------------------
PlanetWatcher::PlanetWatcher(QWidget *parent, char const *name)
 : BasePlanetWatcher(parent, name)
 , m_gameServers(NULL)
 , m_planetViewTimer(NULL)
 , m_barrierMetricsTimer(NULL)
 , m_selectedRow(0)
 , m_lastSelectedRow(-1)
 , m_selectedServerDataStartRow(-1)
 , m_lastSelectedServerDataStartRow(-1)
 , m_planetServerConnection(NULL)
 , m_filterMs(0)
 , m_warnMs(0)
 , m_objectTypeTotalCountMap()
 , m_objectTypeSelectedCountMap()
 , m_currentDrawObjectType()
 , m_showingServerData(false)
{
	ms_allValidAiActivities.clear();
	ms_allValidAiActivities.push_back(cms_abt_idleString);
	ms_allValidAiActivities.push_back(cms_abt_loiterString);
	ms_allValidAiActivities.push_back(cms_abt_wanderString);
	ms_allValidAiActivities.push_back(cms_abt_followString);
	ms_allValidAiActivities.push_back(cms_abt_fleeString);
	ms_allValidAiActivities.push_back(cms_abt_moveString);
	ms_allValidAiActivities.push_back(cms_abt_patrolString);
	ms_allValidAiActivities.push_back(cms_abt_faceString);
	ms_allValidAiActivities.push_back(cms_abt_swarmString);

	m_planetRenderer->setPlanetWatcher(this);

	m_gameServers = new GameServerMap;

	SetupSharedThread::install();
	SetupSharedDebug::install(4096);

	SetupSharedFoundation::Data data(SetupSharedFoundation::Data::D_mfc);
	data.useWindowHandle    = true;
	data.processMessagePump = false;
	data.windowHandle       = static_cast<HWND>(winId());
	data.configFile         = "planetwatcher.cfg";
	data.clockUsesSleep     = true;
	SetupSharedFoundation::install(data);

	SetupSharedCompression::install();
	SetupSharedFile::install(false);
	SetupSharedNetworkMessages::install();
	SetupSharedRandom::install(static_cast<uint32>(time(NULL)));

	NetworkHandler::install();
	SetupSharedNetwork::SetupData  networkSetupData;
	SetupSharedNetwork::getDefaultClientSetupData(networkSetupData);
	SetupSharedNetwork::install(networkSetupData);

	FilterManager::install();

	bool const success = PlanetWatcherUtility::install();
	if(!success)
	{
		std::string msg = "Cannot find [";
		msg            += PlanetWatcherUtility::getServerObjectTemplateCrcStringTableName();
		msg            += "], server object template names will not be converted from Crcs";
		QMessageBox::warning(this, "Missing File!", msg.c_str(), "OK");
	}

	FilterManager::setValidValues("AI Activity", ms_allValidAiActivities);
	FilterManager::setValidValues("Tag", PlanetWatcherUtility::getAllValidTagTypes());

	PlanetWatcherUtility::setOutputWindow(*m_outputTextEdit);

	QFont font("Courier", 8);
	m_outputTextEdit->setTextFormat(Qt::LogText);
	m_outputTextEdit->setMaxLogLines(-1);
	m_outputTextEdit->setFont(font);
	m_outputTextEdit->setReadOnly(true);

	m_propertiesListView->hide();

	// Setup the server table

	m_serverTable->setNumRows(1);
	m_serverTable->setItem(0, GSTC_color, new ColorTableItem(m_serverTable, "", QColor(128, 128, 128)));
	ColorTableItem *colorTableItem = dynamic_cast<ColorTableItem *>(m_serverTable->item(0, GSTC_color));
	if (colorTableItem != NULL)
	{
		colorTableItem->setCellColor(QColor(128, 128, 128));
	}

	m_serverTable->setText(0, GSTC_serverName, "Planet Server");
	m_serverTable->horizontalHeader()->setClickEnabled(false);
	m_serverTable->horizontalHeader()->setResizeEnabled(true);
	m_serverTable->horizontalHeader()->setMovingEnabled(true);
	m_serverTable->verticalHeader()->setClickEnabled(false);
	m_serverTable->verticalHeader()->setResizeEnabled(false);
	m_serverTable->verticalHeader()->setMovingEnabled(false);

	// Setup the legend table

	m_legendTable->horizontalHeader()->hide();
	m_legendTable->verticalHeader()->hide();
	m_legendTable->setTopMargin(0);
	m_legendTable->setLeftMargin(0);
	m_legendTable->setNumRows(0);

	m_legendTable->insertRows(m_legendTable->numRows());
	m_legendTable->setText(m_legendTable->numRows() - 1, LTC_description, "Total objects");

	m_legendTable->insertRows(m_legendTable->numRows());
	m_legendTable->setItem(m_legendTable->numRows() - 1, LTC_color, new ColorTableItem(m_legendTable, "", ms_mobileObjects));
	m_legendTable->setText(m_legendTable->numRows() - 1, LTC_description, "Objects: moved (probably creatures)");

	m_legendTable->insertRows(m_legendTable->numRows());
	m_legendTable->setItem(m_legendTable->numRows() - 1, LTC_color, new ColorTableItem(m_legendTable, "", ms_mobileInterestRadiusObjects));
	m_legendTable->setText(m_legendTable->numRows() - 1, LTC_description, "Objects: with interest radius (probably players)");

	m_legendTable->insertRows(m_legendTable->numRows());
	m_legendTable->setItem(m_legendTable->numRows() - 1, LTC_color, new ColorTableItem(m_legendTable, "", ms_neverMovedNonCreatureObjects));
	m_legendTable->setText(m_legendTable->numRows() - 1, LTC_description, "Objects: never moved non creature (buildings, etc)");

	m_legendTable->insertRows(m_legendTable->numRows());
	m_legendTable->setItem(m_legendTable->numRows() - 1, LTC_color, new ColorTableItem(m_legendTable, "", ms_neverMovedCreatureObjects));
	m_legendTable->setText(m_legendTable->numRows() - 1, LTC_description, "Objects: never moved creature");

	m_filtersTable->setColumnReadOnly(FTC_filterType, true);
	m_filtersTable->setColumnReadOnly(FTC_condition, true);
	m_filtersTable->setColumnReadOnly(FTC_value, true);

	reFitTable();

	// Initialize the object filter to show all objects
	m_currentDrawObjectType.insert(s_allObjectsTypeTag);

	// Planet view timer

	m_planetViewTimer = new QTimer(this, "PlanetViewTimer");
	m_planetViewTimer->start(1000 / 10); // Think about it when you jack this number
	connect(m_planetViewTimer, SIGNAL(timeout()), this, SLOT(slotPlanetViewTimerTimeOut()));

	// Barrier metrics timer

	m_barrierMetricsTimer = new QTimer(this, "BarrierMetricsTimer");
	m_barrierMetricsTimer->start(1000 / 10); // Think about it when you jack this number
	connect(m_barrierMetricsTimer, SIGNAL(timeout()), this, SLOT(slotBarrierMetricsTimerTimeOut()));

	// Connections

	connect(m_planetRenderer, SIGNAL(signalCellSelected()), this, SLOT(slotCellSelected()));
	connect(m_exitAction, SIGNAL(activated()), this, SLOT(slotExitActionActivated()));
	connect(m_simulationAction, SIGNAL(activated()), this, SLOT(slotSimulationActionActivated()));

	connect(m_connectAction, SIGNAL(activated()), this, SLOT(slotConnectActionActivated()));

	connect(m_loginConnectAction, SIGNAL(activated()), this, SLOT(slotLoginConnectActionActivated() ) );

	connect(m_disconnectAction, SIGNAL(activated()), this, SLOT(slotDisconnectActionActivated()));
	connect(m_serverTable, SIGNAL(clicked(int, int, int, const QPoint &)), SLOT(slotServerTableClicked(int, int, int, const QPoint &)));
	connect(m_processFrameScrollBar, SIGNAL(valueChanged(int)), SLOT(slotProcessFrameScrollBarValueChanged(int)));
	connect(m_snapToCurrentFramePushButton, SIGNAL(clicked()), this, SLOT(slotSnapToCurrentFramePushButtonClicked()));
	connect(m_stopFramePushButton, SIGNAL(clicked()), this, SLOT(slotStopFramePushButtonClicked()));
	connect(m_filterMsCheckBox, SIGNAL(clicked()), this, SLOT(slotFilterMsCheckBoxClicked()));
	connect(m_warnMsCheckBox, SIGNAL(clicked()), this, SLOT(slotWarnMsCheckBoxClicked()));
	connect(m_clearPushButton, SIGNAL(clicked()), this, SLOT(slotClearPushButtonClicked()));
	connect(m_locateObjectByIdAction, SIGNAL(activated()), this, SLOT(slotLocateObjectByIdActionActivated()));
	connect(m_dumpInfoPushButton, SIGNAL(clicked()), this, SLOT(slotDumpInfoPushButtonClicked()));
	connect(m_clearOutputTextWindowPushButton, SIGNAL(clicked()), this, SLOT(slotClearOutputTextWindowPushButtonClicked()));
	connect(m_refreshFilterPushButton, SIGNAL(clicked()), this, SLOT(slotRefreshFilterPushButtonClicked()));
	connect(m_colorGridCheckBox, SIGNAL(clicked()), this, SLOT(slotColorGridCheckBoxClicked()));
	connect(m_addFilterPushButton, SIGNAL(clicked()), this, SLOT(slotAddFilterPushButtonClicked()));
	connect(m_tagFilterTable, SIGNAL(valueChanged(int, int)), this, SLOT(slotTagFilterTableValueChanged(int, int)));
	connect(m_filtersTable, SIGNAL(contextMenuRequested(int, int, const QPoint &)), this, SLOT(slotFiltersTableContextMenuRequested(int, int, const QPoint &)));
	connect(m_filtersTable, SIGNAL(doubleClicked(int, int, int, const QPoint &)), this, SLOT(slotFiltersTableDoubleClicked(int, int, int, const QPoint &)));

	setServerLoopTime(0, 0);

	PlanetWatcherUtility::loadWidget(*this, 0, 0, 800, 600);

	QIntValidator *intValidator = new QIntValidator(this, "QIntValidator");
	m_filterMsLineEdit->setValidator(intValidator);
	m_warnMsLineEdit->setValidator(intValidator);

	m_filterMsLineEdit->setText(PlanetWatcherUtility::readEntry("filterMs", "200"));
	m_warnMsLineEdit->setText(PlanetWatcherUtility::readEntry("warnMs", "500"));

	m_filterMsCheckBox->setChecked(m_filterMsLineEdit->text().toInt() != 0);
	slotFilterMsCheckBoxClicked();

	m_warnMsCheckBox->setChecked(m_warnMsLineEdit->text().toInt() != 0);
	slotWarnMsCheckBoxClicked();

	m_colorGridCheckBox->setChecked(true);
	slotColorGridCheckBoxClicked();

	m_processFrameScrollBar->setMinValue(0);
	m_processFrameScrollBar->setMaxValue(0);

	QApplication::setGlobalMouseTracking(true);

	slotDisconnectActionActivated();
}

//-----------------------------------------------------------------------------
PlanetWatcher::~PlanetWatcher()
{
	delete m_gameServers;
	m_gameServers = NULL;

	PlanetWatcherUtility::saveWidget(*this);

	if (m_planetServerConnection)
	{
		m_planetServerConnection = 0;
	}

	PlanetWatcherUtility::writeEntry("filterMs", QString::number(m_filterMs));
	PlanetWatcherUtility::writeEntry("warnMs", QString::number(m_warnMs));

	// Kickoff the exit chain

	FilterManager::remove();

	NetworkHandler::remove();
	SetupSharedFoundation::remove();
	SetupSharedThread::remove();
}

//-----------------------------------------------------------------------------
void PlanetWatcher::slotBarrierMetricsTimerTimeOut()
{
	if (m_barrierInfoFrame->isVisible())
	{
		drawBarrierMetrics();
	}
}

//-----------------------------------------------------------------------------
void PlanetWatcher::slotPlanetViewTimerTimeOut()
{
	// Set the zoom

	QString zoom;
	zoom.sprintf("Zoom: %f", m_planetRenderer->getZoom());
	statusBar()->message(zoom);

	// Set the game servers object and cell count

	int planetCellCount = 0;
	int planetObjectCount = 0;
	int const rowCount = m_serverTable->numRows();
	
	for (int i = 1; i < rowCount; ++i)
	{
		if(isAServerRow(i))
		{
			uint32 const gameServerId = static_cast<uint32>(m_serverTable->text(i, GSTC_serverName).toInt());

			// Update the cell and object counts

			int const gameServerCellCount = m_planetRenderer->getGameServerCellCount(gameServerId);
			int const gameServerObjectCount = m_planetRenderer->getGameServerObjectCount(gameServerId);

			m_serverTable->setText(i, GSTC_cellCount, QString::number(gameServerCellCount));
			m_serverTable->setText(i, GSTC_objectCount, QString::number(gameServerObjectCount));
		
			planetCellCount += gameServerCellCount;
			planetObjectCount += gameServerObjectCount;

			// Update the color

			ColorTableItem *colorTableItem = dynamic_cast<ColorTableItem *>(m_serverTable->item(i, GSTC_color));

			if (colorTableItem != NULL)
			{
				colorTableItem->setCellColor(getGameServerColor(gameServerId));
			}
		}
	}
	
	// Legend information
	int total, selected, moved, interestRadius, immobileCreature, immobileNonCreature, selectedMoved, selectedInterestRadius, selectedImmobileCreature, selectedImmobileNonCreature;
	m_planetRenderer->getObjectCount(m_objectTypeTotalCountMap, total, m_objectTypeSelectedCountMap, selected, moved, interestRadius, immobileCreature, immobileNonCreature, selectedMoved, selectedInterestRadius, selectedImmobileCreature, selectedImmobileNonCreature);

	m_legendTable->setText(0, LTC_count, QString::number(total));
	m_legendTable->setText(1, LTC_count, QString::number(moved));
	m_legendTable->setText(2, LTC_count, QString::number(interestRadius));
	m_legendTable->setText(3, LTC_count, QString::number(immobileNonCreature));
	m_legendTable->setText(4, LTC_count, QString::number(immobileCreature));

	if(m_selectedServerDataStartRow != -1 && m_showingServerData)
	{
		m_serverTable->setText(m_selectedServerDataStartRow, LTC_description, "Moved (probably creatures)");
		m_serverTable->setText(m_selectedServerDataStartRow, 3, QString::number(selectedMoved));
		m_serverTable->setText(m_selectedServerDataStartRow + 1, LTC_description, "With interest radius (probably players)");
		m_serverTable->setText(m_selectedServerDataStartRow + 1, 3, QString::number(selectedInterestRadius));
		m_serverTable->setText(m_selectedServerDataStartRow + 2, LTC_description, "Never moved non-creature (buildings, etc)");
		m_serverTable->setText(m_selectedServerDataStartRow + 2, 3, QString::number(selectedImmobileNonCreature));
		m_serverTable->setText(m_selectedServerDataStartRow + 3, LTC_description, "Never moved creature");
		m_serverTable->setText(m_selectedServerDataStartRow + 3, 3, QString::number(selectedImmobileCreature));
	}

	// Mouse world position

	QPoint mousePosition(m_planetRenderer->getMousePosition());
	float worldX;
	float worldZ;
	m_planetRenderer->pixelToWorld(mousePosition, worldX, worldZ);

	QString position;
	position.sprintf("(%d, 0, %d)", static_cast<int>(worldX), static_cast<int>(worldZ));
	m_mouseWorldPositionTextLabel->setText(position);

	// Set the planet server object and cell count

	m_serverTable->setText(0, GSTC_cellCount, QString::number(planetCellCount));
	m_serverTable->setText(0, GSTC_objectCount, QString::number(planetObjectCount));

	//// Remove any game servers that do not own any cells anymore
	//
	//GameServerMap::const_iterator iterGameServers = m_gameServers->begin();
	//
	//for (; iterGameServers != m_gameServers->end(); ++iterGameServers)
	//{
	//	uint32 const gameServerId = iterGameServers->first;
	//
	//	if (m_planetRenderer->getGameServerCellCount(gameServerId) <= 0)
	//	{
	//		QString text;
	//		text.sprintf("Game server %d was removed because it owns no cells.", gameServerId);
	//		PlanetWatcherUtility::report(text);
	//
	//		removeGameServer(gameServerId);
	//
	//		break;
	//	}
	//}

	NetworkHandler::update();
	NetworkHandler::dispatch();

	if (m_planetServerConnection && m_planetServerConnection->gotUpdates())
	{
		m_planetServerConnection->clearUpdatesFlag();
		update();
	}
}

//-----------------------------------------------------------------------------
void PlanetWatcher::slotCellSelected()
{
	QPoint cellPosition(m_planetRenderer->getSelectedCellPosition());

	QPoint topLeft;
	QPoint bottomRight;
	m_planetRenderer->getCellWorldPosition(cellPosition, topLeft, bottomRight);

	PlanetWatcherRenderer::Uint32Vector gameServerIds;
	m_planetRenderer->getCellGameServerIds(cellPosition, gameServerIds);

	PlanetWatcherRenderer::NetworkIdVector objectIds;
	m_planetRenderer->getCellObjects(cellPosition, objectIds);
}

//-----------------------------------------------------------------------------
void PlanetWatcher::resizeEvent(QResizeEvent *)
{
	//int const contentsWidth = width();
	//int const contentsHeight = height();
	//
	//m_planetServerCanvasView->setSize(contentsWidth, contentsHeight);
	//m_canvas->setZoom((contentsWidth < contentsHeight) ? (static_cast<float>(contentsWidth) / static_cast<float>(m_canvas->getWorldSizeX())) : (static_cast<float>(contentsHeight) / static_cast<float>(m_canvas->getWorldSizeY())));
}

//-----------------------------------------------------------------------------
void PlanetWatcher::addGameServer(uint32 const gameServerId, QString const &ip, QString const &port, QString const &sceneId)
{
	// Make sure the game server has not already been added to the list

	GameServerMap::const_iterator iterGameServers = m_gameServers->find(gameServerId);
	
	if (iterGameServers == m_gameServers->end())
	{
		GameServerInfo gameServerInfo;

		gameServerInfo.m_ip = ip;
		gameServerInfo.m_port = port;
		gameServerInfo.m_sceneId = sceneId;

		m_gameServers->insert(std::make_pair(gameServerId, gameServerInfo));

		int const row = m_serverTable->numRows();

		m_serverTable->insertRows(row);
	
		// Color

		QColor color("black");

		switch (m_gameServers->size())
		{
			case 1: { color = QColor("dark blue"); } break;
			case 2: { color = QColor("dark red"); } break;
			case 3: { color = QColor("dark green"); } break;
			case 4: { color = QColor(128, 0, 128); } break;
			case 5: { color = QColor(128, 128, 0); } break;
			case 6: { color = QColor(0, 128, 128); } break;
			case 7: { color = QColor(128, 128, 64); } break;
			case 8: { color = QColor(128, 64, 128); } break;
			case 9: { color = QColor(64, 128, 128); } break;
			default: {} break;
		}

		m_planetRenderer->setGameServerColor(gameServerId, color);
		m_serverTable->setItem(row, GSTC_color, new ColorTableItem(m_serverTable, "", m_planetRenderer->getGameServerColor(gameServerId)));

		// Server name

		QString gameServerLabel;
		gameServerLabel.sprintf("%d", gameServerId);

		m_serverTable->setText(row, GSTC_serverName, gameServerLabel);
		m_serverTable->setText(row, GSTC_IP, ip);
		m_serverTable->setText(row, GSTC_port, port);

		// Planet

		m_serverTable->setText(row, GSTC_planet, sceneId);

		ColorTableItem *colorTableItem = dynamic_cast<ColorTableItem *>(m_serverTable->item(row, GSTC_color));

		if (colorTableItem != NULL)
		{
			colorTableItem->setCellColor(getGameServerColor(gameServerId));
		}

		reFitTable();

		m_serverTable->horizontalHeader()->adjustHeaderSize();
		m_serverTable->verticalHeader()->adjustHeaderSize();
	}
}

//-----------------------------------------------------------------------------
void PlanetWatcher::reFitTable()
{
	{
		for (int i = 0; i < m_serverTable->numCols(); ++i)
		{
			m_serverTable->setColumnWidth(i, 0);
			m_serverTable->adjustColumn(i);
		}
	}

	{
		for (int i = 0; i < m_legendTable->numCols(); ++i)
		{
			m_legendTable->setColumnWidth(i, 0);
			m_legendTable->adjustColumn(i);
		}
	}

	{
		for (int i = 0; i < m_tagFilterTable->numCols(); ++i)
		{
			m_tagFilterTable->setColumnWidth(i, 0);
			m_tagFilterTable->adjustColumn(i);
		}
	}

	{
		for (int i = 0; i < m_filtersTable->numCols(); ++i)
		{
			m_filtersTable->setColumnWidth(i, 0);
			m_filtersTable->adjustColumn(i);
		}
	}
}

//-----------------------------------------------------------------------------
void PlanetWatcher::setWorld(float const sizeX, float const sizeZ, int const cellCountX, int const cellCountZ)
{
	m_planetRenderer->setWorld(sizeX, sizeZ, cellCountX, cellCountZ);
}

//-----------------------------------------------------------------------------
void PlanetWatcher::setCellSubscription(float const worldX, float const worldZ, Uint32Vector const &gameServers, IntVector const &subscriptionCounts)
{
	float const newWorldX = worldX + m_planetRenderer->getWorldSizeX() / 2.0f;
	float const newWorldZ = worldZ + m_planetRenderer->getWorldSizeZ() / 2.0f;

	m_planetRenderer->setCellSubscription(m_planetRenderer->getCellX(newWorldX), m_planetRenderer->getCellY(newWorldZ), gameServers, subscriptionCounts);
}

//-----------------------------------------------------------------------------
void PlanetWatcher::setGameServerColor(uint32 const gameServerId, QColor const &color)
{
	m_planetRenderer->setGameServerColor(gameServerId, color);
}

//-----------------------------------------------------------------------------
void PlanetWatcher::updateObject(NetworkId const &objectId, uint32 const gameServerId, float const worldX, float const worldZ, int const interestRadius, int objectTypeTag, int const level, bool const hibernating, uint32 const templateCrc, int const aiActivity, int const creationType)
{
	float const newWorldX = worldX;
	float const newWorldZ = worldZ;

	m_planetRenderer->updateObject(objectId, gameServerId, newWorldX, newWorldZ, interestRadius, objectTypeTag, level, hibernating, templateCrc, aiActivity, creationType);
}

//-----------------------------------------------------------------------------
void PlanetWatcher::removeObject(NetworkId const &objectId)
{
	m_planetRenderer->removeObject(objectId);
}

//-----------------------------------------------------------------------------
void PlanetWatcher::clearCellGameServersIds(float const worldX, float const worldZ)
{
	float const newWorldX = worldX + m_planetRenderer->getWorldSizeX() / 2.0f;
	float const newWorldZ = worldZ + m_planetRenderer->getWorldSizeZ() / 2.0f;

	m_planetRenderer->clearCellGameServersIds(m_planetRenderer->getCellX(newWorldX), m_planetRenderer->getCellY(newWorldZ));
}

//-----------------------------------------------------------------------------
QColor const &PlanetWatcher::getGameServerColor(uint32 const gameServerId) const
{
	return m_planetRenderer->getGameServerColor(gameServerId);
}

//-----------------------------------------------------------------------------
void PlanetWatcher::slotExitActionActivated()
{
	close();
}

//-----------------------------------------------------------------------------
void PlanetWatcher::slotSimulationActionActivated()
{
	int const simulatedGameServerCount = 5;

	initializeTestGameServers(simulatedGameServerCount);

	m_planetRenderer->update();
}

//-----------------------------------------------------------------------------
void PlanetWatcher::initializeTestGameServers(int const serverCount)
{
	// Set the world size

	float const worldSizeX = 16000.0f;
	float const worldSizeZ = 16000.0f;
	float const cellWidthX = 100.0f;
	float const cellWidthZ = 100.0f;
	int const cellCountX = worldSizeX / cellWidthX;
	int const cellCountZ = worldSizeZ / cellWidthZ;
	
	setWorld(worldSizeX, worldSizeZ, cellCountX, cellCountZ);

	// Add some game servers

	typedef std::vector<int> IntVector;
	IntVector gameServerIds;
	int i;

	for (i = 0; i < serverCount; ++i)
	{
		int const gameServerId = rand() % 99999 + 1;

		addGameServer(gameServerId, "0.0.0.0", "0", "NA");
		gameServerIds.push_back(gameServerId);
	}

	// Add some objects

	for (i = 0; i < 10000; ++i)
	{
		float const worldX = -worldSizeX / 2.0f + Random::randomReal() * worldSizeX;
		float const worldZ = -worldSizeZ / 2.0f + Random::randomReal() * worldSizeZ;
		int   const level  =   Random::random(s_levelMin, s_levelMax);
		unsigned int const gameServerIndex = rand() % gameServerIds.size();
		uint32 const gameServerId = static_cast<uint32>(gameServerIds[gameServerIndex]);

		updateObject(NetworkId(static_cast<int64>(i)), gameServerId, worldX, worldZ, Random::random(0, 10), PlanetWatcherUtility::getRandomObjectType(), level, (Random::random(0, 1) == 1), 0, Random::random(0, 6), Random::random(0, 7));
	}

	// Simulate the server grid cells
	
	int const serverCellCount = 1000;
	
	for (i = 0; i < serverCellCount; ++i)
	{
		float const worldX = static_cast<int>(-m_planetRenderer->getWorldSizeX() / 2.0f + static_cast<float>(m_planetRenderer->getWorldSizeX()) * Random::randomReal());
		float const worldZ = static_cast<int>(-m_planetRenderer->getWorldSizeZ() / 2.0f + static_cast<float>(m_planetRenderer->getWorldSizeZ()) * Random::randomReal());
		unsigned int const gameServerIndex = rand() % gameServerIds.size();
		uint32 const gameServerId = static_cast<uint32>(gameServerIds[gameServerIndex]);
	
		m_planetRenderer->setCellGameServerId(worldX, worldZ, gameServerId);
	}

	// Simulate some influence radius server cells (basically circles)

	int const influenceRadiusServerCellCount = 20;
	int const cellInfluenceRadius = 20;

	for (i = 0; i < influenceRadiusServerCellCount; ++i)
	{
		unsigned int const gameServerIndex = rand() % gameServerIds.size();
		uint32 const gameServerId = static_cast<uint32>(gameServerIds[gameServerIndex]);
		float const circleWorldX = static_cast<int>(static_cast<float>(m_planetRenderer->getWorldSizeX()) * Random::randomReal());
		float const circleWorldZ = static_cast<int>(static_cast<float>(m_planetRenderer->getWorldSizeZ()) * Random::randomReal());

		for (int x = 0; x < cellInfluenceRadius * 2; ++x)
		{
			for (int y = 0; y < cellInfluenceRadius * 2; ++y)
			{
				int const deltaX = x - cellInfluenceRadius;
				int const deltaY = y - cellInfluenceRadius;
				float const distance = deltaX * deltaX + deltaY * deltaY;

				if (distance < (cellInfluenceRadius * cellInfluenceRadius))
				{
					float const worldX = -m_planetRenderer->getWorldSizeX() / 2.0f + circleWorldX - (static_cast<float>(cellInfluenceRadius) * m_planetRenderer->getCellSizeX()) + static_cast<float>(x) * m_planetRenderer->getCellSizeX();
					float const worldZ = -m_planetRenderer->getWorldSizeZ() / 2.0f + circleWorldZ - (static_cast<float>(cellInfluenceRadius) * m_planetRenderer->getCellSizeZ()) + static_cast<float>(y) * m_planetRenderer->getCellSizeZ();

					m_planetRenderer->setCellGameServerId(worldX, worldZ, gameServerId);
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
void PlanetWatcher::removeGameServer(uint32 const gameServerId)
{
	GameServerMap::const_iterator iterGameServers = m_gameServers->find(gameServerId);

#ifdef _DEBUG
	int const startRowCount = m_serverTable->numRows();
	UNREF(startRowCount);
#endif // _DEBUG
	
	if (iterGameServers != m_gameServers->end())
	{
		for (int i = 0; i < m_serverTable->numRows(); ++i)
		{
			QString serverId(m_serverTable->item(i, GSTC_serverName)->text());

			if (serverId == QString::number(gameServerId))
			{
				// We found a match, remove it from the table

				m_serverTable->removeRow(i);

#ifdef _DEBUG
				int const endRowCount = m_serverTable->numRows();
				UNREF(endRowCount);
#endif // _DEBUG

				break;
			}
		}

		m_gameServers->erase(iterGameServers->first);
	}
	else
	{
		QString text;
		text.sprintf("Trying to remove game server %d which does not appear to be in the list.", gameServerId);
		PlanetWatcherUtility::report(text);
	}

	m_planetRenderer->removeGameServerObjects(gameServerId);
	m_planetRenderer->removeGameServerCells(gameServerId);
}

//-----------------------------------------------------------------------------
void PlanetWatcher::removeAllGameServers()
{
	while (!m_gameServers->empty())
	{
		uint32 const gameServerId = m_gameServers->begin()->first;

		removeGameServer(gameServerId);
	}

	m_serverTable->setNumRows(1);
}

//-----------------------------------------------------------------------------
void PlanetWatcher::connectionSuccess()
{
	PlanetWatcherUtility::report("Connection Successful");
	m_planetRenderer->setGameServerConnected(true);
}

//-----------------------------------------------------------------------------
void PlanetWatcher::connectionLost()
{
	QMessageBox::warning(this, "Connection Error", "Connection to Planet Server lost", "OK");

	PlanetWatcherUtility::report("Connection Lost");
	m_planetRenderer->setGameServerConnected(false);
	m_planetServerConnection = NULL;
}

//-----------------------------------------------------------------------------
void PlanetWatcher::slotDisconnectActionActivated()
{
	if (m_planetServerConnection)
	{
		m_planetServerConnection->disconnect();
	}

	m_planetServerConnection = NULL;

	PlanetWatcherUtility::report("Disconnected from the Planet Server");
	m_planetRenderer->setGameServerConnected(false);

	setCaption("Planet Watcher - disconnected");
}

//-----------------------------------------------------------------------------

void PlanetWatcher::slotLoginConnectActionActivated()
{
	ClusterSelectionDialog dlg( this, "ClusterSelectionDialog" );

	// load the username from disk

	
	int result = dlg.exec();

	if ( result == QDialog::Accepted )
	{

		removeAllGameServers();
		slotDisconnectActionActivated();

		int cluster = dlg.getSelectedCluster();

		try
		{
			const LoginClusterStatus_ClusterData & clusterData = dlg.getClusterData( cluster );
			
			setWorld( 16000.f, 16000.f, 160, 160 );

			m_planetServerConnection = new PlanetServerConnection(
				clusterData.m_connectionServerAddress.c_str(),
				60002,
				this );

			m_serverTable->setText(0, GSTC_serverName, "Planet Server");
			m_serverTable->setText(0, GSTC_IP, clusterData.m_connectionServerAddress.c_str());
			m_serverTable->setText(0, GSTC_port, QString().sprintf( "%d", clusterData.m_connectionServerPort ) );

			setServerLoopTime(0, 0);


			reFitTable();

		}

		catch ( const std::bad_exception &e )
		{
			PlanetWatcherUtility::report( "exception caught: %s", e.what() );
			return;
		}

	}


}

//-----------------------------------------------------------------------------

void PlanetWatcher::slotConnectActionActivated()
{
	ServerConnectionDialog serverConnectionDialog(this, "ServerConnectionDialog");
	
	int const result = serverConnectionDialog.exec();

	if (result == QDialog::Accepted)
	{
		// Clear any previous data

		removeAllGameServers();

		slotDisconnectActionActivated();

		// Set the world size and cells

		float const sizeX = 16000.0f;
		float const sizeZ = 16000.0f;
		int const cellCountX = 160;
		int const cellCountZ = 160;

		setWorld(sizeX, sizeZ, cellCountX, cellCountZ);

		QString server(serverConnectionDialog.getServer());
		QString port(serverConnectionDialog.getPort());
		QString text;
		text.sprintf("Attempting to connect to %s:%s", server.latin1(), port.latin1());
		PlanetWatcherUtility::report(text);

		m_planetServerConnection = new PlanetServerConnection(server.latin1(), port.toInt(), this);

		m_serverTable->setText(0, GSTC_serverName, "Planet Server");
		m_serverTable->setText(0, GSTC_IP, serverConnectionDialog.getServer());
		m_serverTable->setText(0, GSTC_port, serverConnectionDialog.getPort());

		setServerLoopTime(0, 0);

		QString caption;
		caption.sprintf("Planet Watcher - %s:%s", serverConnectionDialog.getServer().latin1(), serverConnectionDialog.getPort().latin1());
		setCaption(caption);

		reFitTable();
	}
}

//-----------------------------------------------------------------------------
void PlanetWatcher::slotServerTableClicked(int row, int /*col*/, int /*button*/, const QPoint &/*mousePosition*/)
{
	m_lastSelectedRow = m_selectedRow;
	m_selectedRow = row;

	if(isAServerRow(row))
	{
		m_lastSelectedServerDataStartRow = m_selectedServerDataStartRow;
		m_selectedServerDataStartRow = m_selectedRow + 1;

		if(m_showingServerData)
		{
			if(m_lastSelectedServerDataStartRow != -1)
			{
				//remove old selected items
				for(int i = m_lastSelectedServerDataStartRow; i < m_lastSelectedServerDataStartRow + s_numberOfServerDataRows; ++i)
					m_serverTable->removeRow(m_lastSelectedServerDataStartRow);

				reFitTable();
				m_serverTable->horizontalHeader()->adjustHeaderSize();
				m_serverTable->verticalHeader()->adjustHeaderSize();

			}
			m_showingServerData = false;
		}

		if(m_lastSelectedRow != m_selectedRow)
		{
			int row = m_selectedServerDataStartRow;

			//add new selected items
			m_serverTable->insertRows(row);
			m_serverTable->setText(row, LTC_description, "Moved (probably creatures)");
			++row;

			m_serverTable->insertRows(row);
			m_serverTable->setText(row, LTC_description, "With interest radius (probably players)");
			++row;

			m_serverTable->insertRows(row);
			m_serverTable->setText(row, LTC_description, "Never moved non-creature (buildings, etc)");
			++row;

			m_serverTable->insertRows(row);
			m_serverTable->setText(row, LTC_description, "Never moved creature");
			++row;

			reFitTable();
			m_serverTable->horizontalHeader()->adjustHeaderSize();
			m_serverTable->verticalHeader()->adjustHeaderSize();

			m_showingServerData = true;
		}
	}
	m_planetRenderer->update();
}

//-----------------------------------------------------------------------------
bool PlanetWatcher::isAServerRow(int const row) const
{
	//not a server row if not a valid table row (and 0 is the header, not a server)
	if(row <= 0 || row >= m_serverTable->numRows())
		return false;
	
	//if valid table row, but nothing selected, then ALL rows are for server
	if(m_selectedServerDataStartRow == -1 || !m_showingServerData)
		return true;

	//otherwise, it is NOT a server row if it falls in the range of rows used for the currently selected server's data
	if(row >= m_selectedServerDataStartRow && row <= (m_selectedServerDataStartRow + s_numberOfServerDataRows - 1))
		return false;

	return true;
}

//-----------------------------------------------------------------------------
uint32 PlanetWatcher::getSelectedGameServerId() const
{
	int result = -1;

	if (m_selectedRow >= 0)
	{
		int rowToUse = m_selectedRow;
		if(!isAServerRow(m_selectedRow))
		{
			rowToUse = m_selectedServerDataStartRow - 1;
		}
		result = m_serverTable->text(rowToUse, GSTC_serverName).toUInt();
	}

	return result;
}

//-----------------------------------------------------------------------------
void PlanetWatcher::setServerLoopTime(int const currentMilliSeconds, int const lastMilliSeconds)
{
	char text[256];
	sprintf(text, "%d ms", lastMilliSeconds);
	m_lastLoopTimeTextLabel->setText(text);

	sprintf(text, "%d ms", currentMilliSeconds);
	m_currentLoopTimeTextLabel->setText(text);
}

//-----------------------------------------------------------------------------
void PlanetWatcher::update()
{
	static int updateCount = 0;
	static PerformanceTimer timer;
	timer.stop();
	//float const elapsedTime = timer.getElapsedTime();
	timer.start();

	m_planetRenderer->update();

	//QString text;
	//text.sprintf("Update (%d) - deltaTime: %.3f", updateCount, elapsedTime);
	//PlanetWatcherUtility::report(text);

	++updateCount;
}

//-----------------------------------------------------------------------------

void PlanetWatcher::slotProcessFrameScrollBarValueChanged(int value)
{
	//int const maxValue = m_processFrameScrollBar->maxValue();

	s_visualFrame = value + s_framesToDisplay;

	if (m_processFrameScrollBar->value() < m_processFrameScrollBar->maxValue())
	{
		slotStopFramePushButtonClicked();
	}
}

//-----------------------------------------------------------------------------
void PlanetWatcher::drawBarrierMetrics()
{
	// Get the warn ms

	if (m_warnMsCheckBox->isChecked())
	{
		m_warnMs = std::max(0, m_warnMsLineEdit->text().toInt());
	}
	else
	{
		m_warnMs = 0;
	}

	// Do we snap to the most current frame?

	if (s_snapToCurrentFrame)
	{
		m_processFrameScrollBar->setMaxValue((s_physicalFrame - s_framesToDisplay) > 0 ? (s_physicalFrame - s_framesToDisplay) : 0);
		m_processFrameScrollBar->setValue(m_processFrameScrollBar->maxValue());
		s_anchoredFrame = s_physicalFrame;
		s_visualFrame = s_anchoredFrame;
	}

	// Draw the barrier information

	drawFrameInfo(*m_barrierInfoFrame, m_warnMs);
	s_showInformation = false;
}

//-----------------------------------------------------------------------------
void PlanetWatcher::slotSnapToCurrentFramePushButtonClicked()
{
	s_snapToCurrentFrame = true;
	m_stopTextLabel->setText("");
}

//-----------------------------------------------------------------------------
void PlanetWatcher::slotStopFramePushButtonClicked()
{
	s_snapToCurrentFrame = false;
	m_stopTextLabel->setText("Stopped");
}

//-----------------------------------------------------------------------------
void PlanetWatcher::mouseDoubleClickEvent(QMouseEvent *mouseEvent)
{
	if (mouseEvent != NULL)
	{
		s_mouseClickPosition_g = mouseEvent->globalPos();
		mouseEvent->accept();
		s_showInformation = true;
	}
}

//-----------------------------------------------------------------------------
void PlanetWatcher::setServerFrameInfo(unsigned int processId, unsigned long frameTime, std::string const &profilerData)
{
	// Get the filter ms

	if (m_filterMsCheckBox->isChecked())
	{
		m_filterMs = std::max(0, m_filterMsLineEdit->text().toInt());
	}
	else
	{
		m_filterMs = 0;
	}

	// Throw out frame times we are not interested in

	if (frameTime >= static_cast<unsigned long>(m_filterMs))
	{
		setServerFrameInfo(s_serverProcesses, processId, frameTime, profilerData);
	}

	// Save all the info for the plotter

	setServerFrameInfo(s_plotterServerProcesses, processId, frameTime, profilerData);
}

//-----------------------------------------------------------------------------
bool PlanetWatcher::isDrawAllObjects() const
{
	return isDrawObjectType(s_allObjectsTypeTag);
}

//-----------------------------------------------------------------------------
bool PlanetWatcher::isDrawObjectType(int objectTypeTag) const
{
	std::set<int>::iterator iterFind = m_currentDrawObjectType.find(objectTypeTag);
	return (iterFind != m_currentDrawObjectType.end());
}

//-----------------------------------------------------------------------------
class KillFrameInfo
{
public:

	KillFrameInfo() {}

	bool operator() (PlanetWatcher::FrameInfo &frameInfo) const
	{
		bool result = false;

		if (frameInfo.m_clockTime < s_killClockTime)
		{
			result = true;
		}

		return result;
	}
};

//-----------------------------------------------------------------------------
void PlanetWatcher::setServerFrameInfo(ServerProcesses &serverProcesses, unsigned int processId, unsigned long frameTime, std::string const &profilerData)
{
	time_t clockTime;
	time(&clockTime);

	FrameInfo frameInfo;
	frameInfo.m_clockTime = clockTime;
	frameInfo.m_frameTime = static_cast<int>(frameTime);
	frameInfo.m_profileInfo = profilerData.c_str();

	ServerProcesses::iterator iterServerProcesses = serverProcesses.find(processId);

	if (iterServerProcesses == serverProcesses.end())
	{
		// This is a new process so add it to the list

		ProcessInfo processInfo;
		processInfo.m_firstVisualFrame = 0;
		processInfo.m_frameInfo.push_back(frameInfo);
		processInfo.m_averageFrameTime = frameTime;

		serverProcesses.insert(std::make_pair(static_cast<int>(processId), processInfo));
	}
	else
	{
		// This process already exists so just append the new data

		if (s_snapToCurrentFrame)
		{
			iterServerProcesses->second.m_firstVisualFrame = iterServerProcesses->second.m_frameInfo.size();
		}

		iterServerProcesses->second.m_frameInfo.push_back(frameInfo);
		iterServerProcesses->second.m_averageFrameTime = iterServerProcesses->second.m_averageFrameTime * 0.98f + static_cast<float>(frameTime) * 0.02f;

		// Trim the frame list so it doesn't grow forever

		unsigned int const maxListSize = 10000;

		if (s_snapToCurrentFrame ||
		    (iterServerProcesses->second.m_frameInfo.size() > maxListSize * 5))
		{
			if (iterServerProcesses->second.m_frameInfo.size() > maxListSize)
			{
				std::vector<FrameInfo> &frameInfoVector = iterServerProcesses->second.m_frameInfo;

				int index = 0;
				std::vector<FrameInfo>::iterator iterFrameInfoVector = frameInfoVector.begin();

				for (; iterFrameInfoVector != frameInfoVector.end(); ++iterFrameInfoVector)
				{
					if (index > maxListSize / 2)
					{
						s_killClockTime = iterFrameInfoVector->m_clockTime;
						break;
					}

					++index;
				}

				std::vector<FrameInfo>::iterator iterFrameInfo = std::remove_if(frameInfoVector.begin(), frameInfoVector.end(), KillFrameInfo());
				IGNORE_RETURN(frameInfoVector.erase(iterFrameInfo, frameInfoVector.end()));

				slotSnapToCurrentFramePushButtonClicked();
			}
		}
	}

	// Set the physical frame number based on the process with the most frames logged

	if (&serverProcesses == &s_serverProcesses)
	{
		ServerProcesses::iterator iterServerProcesses = serverProcesses.begin();

		s_physicalFrame = 0;

		for (; iterServerProcesses != serverProcesses.end(); ++iterServerProcesses)
		{
			s_physicalFrame = std::max(s_physicalFrame, static_cast<int>(iterServerProcesses->second.m_frameInfo.size()));
		}
	}
}

//-----------------------------------------------------------------------------
void PlanetWatcher::drawFrameInfo(QFrame &frame, int const warnMs) const
{
	if (!frame.isVisible())
	{
		return;
	}

	static QPixmap m_pixMap;

	QPainter painter;
	QFontMetrics fm(fontMetrics());

	if ((m_pixMap.width() != frame.width()) ||
	    (m_pixMap.height() != frame.height()))
	{
		m_pixMap.resize(frame.width(), frame.height());
	}

	painter.begin(&m_pixMap);

	QRect drawArea(QRect(0, 0, painter.viewport().width(), painter.viewport().height()));

	QFont font("Courier", 8);
	painter.setFont(font);
	painter.setViewport(drawArea);
	painter.setWindow(drawArea);
	painter.fillRect(m_pixMap.rect(), QBrush(QColor(backgroundColor())));
	painter.setPen(QColor("grey"));
	painter.drawRect(m_pixMap.rect());

	int const width = 42;
	int const height = 16;
	int const plotterHeight = height * 2;

	// Top server line divider

	painter.setPen(QColor("black"));
	painter.drawLine(0, height + 1, frame.width(), height + 1);

	// Top average frame time line divider

	painter.setPen(QColor("black"));
	painter.drawLine(0, height * 2, frame.width(), height * 2);

	// Bottom current frame divider

	painter.setPen(QColor("black"));
	painter.drawLine(0, frame.height() - height - plotterHeight + 1, frame.width(), frame.height() - height - plotterHeight + 1);

	// Bottom plotter divider

	painter.setPen(QColor("black"));
	painter.drawLine(0, frame.height() - plotterHeight + 1, frame.width(), frame.height() - plotterHeight + 1);

	if (s_serverProcesses.size() == 0)
	{
		painter.setPen(QColor("black"));
		painter.drawText(m_pixMap.rect(), Qt::AlignHCenter | Qt::AlignVCenter, "Awaiting frame data...");
	}
	else
	{
		ServerProcesses::iterator iterServerProcesses = s_serverProcesses.begin();
		ServerProcesses::iterator iterPlotterServerProcesses = s_plotterServerProcesses.begin();

		QRect processIdRect(0, 0, width, height);

		int newMinFrameTime = s_maxFrameTime;
		int newMaxFrameTime = s_minFrameTime;

		for (; iterServerProcesses != s_serverProcesses.end(); ++iterServerProcesses, ++iterPlotterServerProcesses)
		{
			int const frameCount = iterServerProcesses->second.m_frameInfo.size();
			int const processId = static_cast<int>(iterServerProcesses->first);

			QString processIdString;
			processIdString.sprintf("S%d", processId);
			painter.drawText(processIdRect, Qt::AlignHCenter | Qt::AlignBottom, processIdString);

			QRect frameTimeRect(processIdRect.left(), processIdRect.top(), processIdRect.width() - 3, processIdRect.height());

			// Show the average frame time

			frameTimeRect.moveBy(0, height);
			QString averageFrameTimeString;
			averageFrameTimeString.sprintf("%d", static_cast<int>(iterPlotterServerProcesses->second.m_averageFrameTime));
			painter.setPen(QColor(0, 0, 128));
			painter.drawText(frameTimeRect, Qt::AlignRight | Qt::AlignVCenter, averageFrameTimeString);

			s_framesToDisplay = (static_cast<int>(frame.height() - (height * 3 + plotterHeight)) / height);

			frameTimeRect.moveBy(0, height * s_framesToDisplay);

			for (int currentFrameToDisplay = 0; currentFrameToDisplay < s_framesToDisplay; ++currentFrameToDisplay)
			{	
				int const firstVisualFrame = iterServerProcesses->second.m_firstVisualFrame;
				int const stoppedOffset = (frameCount - firstVisualFrame);
				int const scrolledOffset = (s_anchoredFrame - s_visualFrame);

				int const currentFrame = (frameCount - 1) - currentFrameToDisplay - stoppedOffset - scrolledOffset;

				if (currentFrame < 0)
				{
					break;
				}

				FrameInfo const &currentFrameInfo = iterServerProcesses->second.m_frameInfo[currentFrame];

				// Highlight this frame time if the mouse is over it

				QRect highlightRect(processIdRect.left(), frameTimeRect.top(), processIdRect.width(), processIdRect.height());
				QRect globalRect(frame.mapToGlobal(highlightRect.topLeft()), frame.mapToGlobal(highlightRect.bottomRight()));

				if (globalRect.contains(s_mouseClickPosition_g) && s_showInformation)
				{
					painter.fillRect(highlightRect, QBrush(QColor("orange")));

					reportFrameInfo(currentFrameInfo, iterServerProcesses->first);
				}

				// Draw the frame time

				int const frameTime = currentFrameInfo.m_frameTime;

				if ((warnMs != 0) &&
				    (frameTime > warnMs))
				{
					painter.setPen(QColor("red"));
				}
				else
				{
					time_t clockTime;
					time(&clockTime);

					setPenColor(painter, clockTime - currentFrameInfo.m_clockTime);
				}

				QString frameTimeString;
				frameTimeString.sprintf("%d", frameTime);
				painter.drawText(frameTimeRect, Qt::AlignRight | Qt::AlignVCenter, frameTimeString);

				frameTimeRect.moveBy(0, -height);
			}

			if (frameCount > 0)
			{
				// Draw the current frame time

				FrameInfo const &mostRecentFrameInfo = iterServerProcesses->second.m_frameInfo[frameCount - 1];

				if ((warnMs != 0) &&
				    (mostRecentFrameInfo.m_frameTime > warnMs))
				{
					painter.setPen(QColor("red"));
				}
				else
				{
					time_t clockTime;
					time(&clockTime);

					setPenColor(painter, clockTime - mostRecentFrameInfo.m_clockTime);
				}

				QString frameTimeString;
				frameTimeString.sprintf("%d", mostRecentFrameInfo.m_frameTime);
				
				QRect mostRecentFrameRect(processIdRect.left(), frame.height() - height - plotterHeight, width - 3, height);
				painter.drawText(mostRecentFrameRect, Qt::AlignRight | Qt::AlignVCenter, frameTimeString);

				QRect globalRect(frame.mapToGlobal(mostRecentFrameRect.topLeft()), frame.mapToGlobal(mostRecentFrameRect.bottomRight()));
				if (globalRect.contains(s_mouseClickPosition_g) && s_showInformation)
				{
					painter.fillRect(mostRecentFrameRect, QBrush(QColor("orange")));

					reportFrameInfo(mostRecentFrameInfo, iterServerProcesses->first);
				}
			}

			if (iterPlotterServerProcesses->second.m_frameInfo.size() > 0)
			{
				int const left = processIdRect.left();
				int const right = processIdRect.right();
				int x1 = right;
				int y1 = -1;
				int x2 = -1;
				int y2 = -1;

				std::vector<FrameInfo>::reverse_iterator iterProcessInfo = iterPlotterServerProcesses->second.m_frameInfo.rbegin();
				int const frameTimeDifference = (s_maxFrameTime - s_minFrameTime);

				for (; iterProcessInfo != iterPlotterServerProcesses->second.m_frameInfo.rend(); ++iterProcessInfo)
				{
					int const frameTime = iterProcessInfo->m_frameTime;

					newMinFrameTime = std::min(newMinFrameTime, frameTime);
					newMaxFrameTime = std::max(newMaxFrameTime, frameTime);

					if (frameTimeDifference > 0)
					{
						float const percent = 1.0f - (static_cast<float>(frameTime - s_minFrameTime) / static_cast<float>(frameTimeDifference));

						y1 = frame.height() - (plotterHeight - 2) + static_cast<float>(plotterHeight - 3) * percent;
					}

					if (x1 <= left)
					{
						break;
					}

					if (y2 != -1)
					{
						// Draw the plotter info
						
						if ((warnMs != 0) &&
							(frameTime > warnMs))
						{
							painter.setPen(QColor("red"));
						}
						else
						{
							painter.setPen(QColor("black"));
						}
						
						painter.drawLine(x1, y1, x2, y2);
					}

					--x1;

					x2 = x1;
					y2 = y1;
				}
			}

			painter.setPen(QColor("black"));
			painter.drawLine(processIdRect.right(), 0, processIdRect.right(), frame.height());

			processIdRect.moveBy(width, 0);
		}

		s_minFrameTime = newMinFrameTime;
		s_maxFrameTime = newMaxFrameTime;
	}

	// Draw the buffer to the widget

	bitBlt(&frame, 0, 0, &m_pixMap);

	painter.end();
}

//-----------------------------------------------------------------------------
void PlanetWatcher::slotFilterMsCheckBoxClicked()
{
	m_filterMsLineEdit->setEnabled(m_filterMsCheckBox->isChecked());
}

//-----------------------------------------------------------------------------
void PlanetWatcher::slotWarnMsCheckBoxClicked()
{
	m_warnMsLineEdit->setEnabled(m_warnMsCheckBox->isChecked());
}

//-----------------------------------------------------------------------------
void PlanetWatcher::slotClearPushButtonClicked()
{
	s_serverProcesses.clear();
	s_plotterServerProcesses.clear();
	slotSnapToCurrentFramePushButtonClicked();
	s_physicalFrame = 0;
	s_visualFrame = 0;
	s_anchoredFrame = 0;
}

//-----------------------------------------------------------------------------
void PlanetWatcher::slotLocateObjectByIdActionActivated()
{
	ObjectLocatorDialog objectLocatorDialog(this, "ObjectLocatorDialog");
	
	int const result = objectLocatorDialog.exec();

	if (result == QDialog::Accepted)
	{
		const char * objectIds = objectLocatorDialog.getObjectId();
		char * copyObjectIds = new char[strlen(objectIds)+1];
		strcpy(copyObjectIds, objectIds);

		QString text;

		const char * strToken = strtok(copyObjectIds, " ");
		while (strToken)
		{
			const std::string objectIdStr(strToken);
			NetworkId const objectId(objectIdStr);

			PlanetWatcherRenderer::MiniMapObject * object = m_planetRenderer->locateObject(objectId);

			if (object)
			{
				text.sprintf("Found %s.", PlanetWatcherUtility::dumpObjectInfo(*object).c_str());
			}
			else
			{
				text.sprintf("Unable to find object %s.", objectId.getValueString().c_str());
			}

			PlanetWatcherUtility::report(text);

			strToken = strtok(NULL, " ");
		}

		delete [] copyObjectIds;
	}
}

//-----------------------------------------------------------------------------
void PlanetWatcher::slotDumpInfoPushButtonClicked()
{
	m_planetRenderer->dumpObjectInfo();
}

//-----------------------------------------------------------------------------
void PlanetWatcher::slotClearOutputTextWindowPushButtonClicked()
{
	PlanetWatcherUtility::clearOutputTextWindow();
}

//-----------------------------------------------------------------------------
void PlanetWatcher::slotRefreshFilterPushButtonClicked()
{
	int indexRowTotal, total;

	m_tagFilterTable->setNumRows(0);
	m_tagFilterTable->setColumnReadOnly(TFTC_objectType, true);
	m_tagFilterTable->setColumnReadOnly(TFTC_objectCount, true);
	m_tagFilterTable->setColumnReadOnly(TFTC_filterSetting, false);

	if (!m_objectTypeTotalCountMap.empty())
	{
		m_tagFilterTable->insertRows(m_tagFilterTable->numRows());
		indexRowTotal = m_tagFilterTable->numRows() - 1;
		m_tagFilterTable->setText(indexRowTotal, TFTC_objectType, s_allObjectsType);
		m_tagFilterTable->setItem(indexRowTotal, TFTC_filterSetting, new QCheckTableItem(m_tagFilterTable, "Show"));

		if (isDrawObjectType(s_allObjectsTypeTag))
			checkTableItem(*m_tagFilterTable, indexRowTotal, TFTC_filterSetting, true);

		total = 0;
		for (std::map<int, int>::const_iterator iter = m_objectTypeTotalCountMap.begin(); iter != m_objectTypeTotalCountMap.end(); ++iter)
		{
			m_tagFilterTable->insertRows(m_tagFilterTable->numRows());
			m_tagFilterTable->setText(m_tagFilterTable->numRows() - 1, TFTC_objectType, PlanetWatcherUtility::getObjectTypeName(iter->first));
			m_tagFilterTable->setText(m_tagFilterTable->numRows() - 1, TFTC_objectCount, QString::number(iter->second));
			m_tagFilterTable->setItem(m_tagFilterTable->numRows() - 1, TFTC_filterSetting, new QCheckTableItem(m_tagFilterTable, "Show"));

			if (isDrawObjectType(iter->first))
				checkTableItem(*m_tagFilterTable, m_tagFilterTable->numRows() - 1, TFTC_filterSetting, true);

			total += iter->second;
		}

		m_tagFilterTable->setText(indexRowTotal, TFTC_objectCount, QString::number(total));
	}

	if (!m_objectTypeSelectedCountMap.empty())
	{
		// blank row
		m_tagFilterTable->insertRows(m_tagFilterTable->numRows());
		m_tagFilterTable->setRowReadOnly(m_tagFilterTable->numRows() - 1, true);

		m_tagFilterTable->insertRows(m_tagFilterTable->numRows());
		indexRowTotal = m_tagFilterTable->numRows() - 1;
		m_tagFilterTable->setText(indexRowTotal, TFTC_objectType, "Selected game server objects");
		m_tagFilterTable->setRowReadOnly(indexRowTotal, true);

		total = 0;
		for (std::map<int, int>::const_iterator iter = m_objectTypeSelectedCountMap.begin(); iter != m_objectTypeSelectedCountMap.end(); ++iter)
		{
			m_tagFilterTable->insertRows(m_tagFilterTable->numRows());
			m_tagFilterTable->setText(m_tagFilterTable->numRows() - 1, TFTC_objectType, PlanetWatcherUtility::getObjectTypeName(iter->first));
			m_tagFilterTable->setText(m_tagFilterTable->numRows() - 1, TFTC_objectCount, QString::number(iter->second));
			m_tagFilterTable->setRowReadOnly(m_tagFilterTable->numRows() - 1, true);

			total += iter->second;
		}

		m_tagFilterTable->setText(indexRowTotal, TFTC_objectCount, QString::number(total));
	}

	reFitTable();
}

//-----------------------------------------------------------------------------
void PlanetWatcher::slotTagFilterTableValueChanged(int row, int col)
{
	UNREF(row);
	UNREF(col);
	if (m_tagFilterTable->numRows() == 0)
		return;

	m_currentDrawObjectType.clear();

	QString objectType;
	QString text;
	int objectTypeTag;
	for (int i = 0; i < m_tagFilterTable->numRows(); ++i)
	{
		if (isTableItemChecked(*m_tagFilterTable, i, TFTC_filterSetting))
		{
			objectType = m_tagFilterTable->text(i, TFTC_objectType);
			if (objectType != QString::null)
			{
				text.sprintf("Adding object type (%s) to filter", objectType.ascii());

				if (strcmp(objectType.ascii(), s_allObjectsType) == 0)
				{
					m_currentDrawObjectType.insert(s_allObjectsTypeTag);
					PlanetWatcherUtility::report(text);
				}
				else if (PlanetWatcherUtility::getObjectTypeTagFromName(objectType.ascii(), objectTypeTag))
				{
					m_currentDrawObjectType.insert(objectTypeTag);
					PlanetWatcherUtility::report(text);
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------

void PlanetWatcher::slotFiltersTableContextMenuRequested(int row, int col, const QPoint & pos)
{
	UNREF(row);
	UNREF(col);
	s_currentFilterRow = row;
	QPopupMenu popupMenu(this);
	popupMenu.insertItem("&Edit Filter", this, SLOT(slotEditFilter()));
	popupMenu.insertItem("&Delete Filter", this, SLOT(slotDeleteFilter()));
	popupMenu.exec(pos);
}

//-----------------------------------------------------------------------------

void PlanetWatcher::slotFiltersTableDoubleClicked(int row, int col, int button, const QPoint & pos)
{
	UNREF(col);
	UNREF(button);
	UNREF(pos);
	s_currentFilterRow = row;
	slotEditFilter();
}

//-----------------------------------------------------------------------------

void PlanetWatcher::slotDeleteFilter()
{
	m_filtersTable->removeRow(s_currentFilterRow);
}

//-----------------------------------------------------------------------------

void PlanetWatcher::slotEditFilter()
{
	AddFilterDialog addFilterDialog(this, "AddFilterDialog");
	addFilterDialog.setEditing(m_filtersTable->text(s_currentFilterRow, FTC_filterType).latin1(), m_filtersTable->text(s_currentFilterRow, FTC_condition).latin1(), m_filtersTable->text(s_currentFilterRow, FTC_value).latin1());
	int const result = addFilterDialog.exec();

	if (result == QDialog::Accepted)
	{
		m_filtersTable->setText(s_currentFilterRow, FTC_filterType, addFilterDialog.m_selectedFilterType.c_str());
		m_filtersTable->setText(s_currentFilterRow, FTC_condition, addFilterDialog.m_selectedCondition.c_str());
		m_filtersTable->setText(s_currentFilterRow, FTC_value, addFilterDialog.m_selectedValue.c_str());
		reFitTable();
		m_filtersTable->horizontalHeader()->adjustHeaderSize();
		m_filtersTable->verticalHeader()->adjustHeaderSize();
	}
}

//-----------------------------------------------------------------------------
void PlanetWatcher::slotColorGridCheckBoxClicked()
{
	m_planetRenderer->setColorGrid(m_colorGridCheckBox->isChecked());
}

//-----------------------------------------------------------------------------
void PlanetWatcher::slotAddFilterPushButtonClicked()
{
	AddFilterDialog addFilterDialog(this, "AddFilterDialog");

	int const result = addFilterDialog.exec();

	if (result == QDialog::Accepted)
	{
		int const newRowNumber = m_filtersTable->numRows();
		m_filtersTable->insertRows(newRowNumber);
		m_filtersTable->setText(newRowNumber, FTC_filterType, addFilterDialog.m_selectedFilterType.c_str());
		m_filtersTable->setText(newRowNumber, FTC_condition, addFilterDialog.m_selectedCondition.c_str());
		m_filtersTable->setText(newRowNumber, FTC_value, addFilterDialog.m_selectedValue.c_str());
		reFitTable();
		m_filtersTable->horizontalHeader()->adjustHeaderSize();
		m_filtersTable->verticalHeader()->adjustHeaderSize();
	}
}

//-----------------------------------------------------------------------------
void PlanetWatcher::reportFrameInfo(FrameInfo const &frameInfo, int const processId) const
{
	QString text;
	text.sprintf("// -- BEGIN ---- (%d) -----------------------------------", processId);
	PlanetWatcherUtility::report(text);
	text.sprintf("processId: %d", processId);
	PlanetWatcherUtility::report(text, false);

	text.sprintf("frameTime: %d", frameInfo.m_frameTime);
	PlanetWatcherUtility::report(text, false);
	
	if (frameInfo.m_profileInfo.isEmpty())
	{
		PlanetWatcherUtility::report("no profile info\n", false);
	}
	else
	{
		PlanetWatcherUtility::report(frameInfo.m_profileInfo, false);
	}
	text.sprintf("// -- END ------ (%d) -----------------------------------\n", processId);
	PlanetWatcherUtility::report(text);
}

//-----------------------------------------------------------------------------

bool PlanetWatcher::passesFilters(PlanetWatcherRenderer::MiniMapObject const & object) const
{
	if(m_filtersTable->numRows() == 0)
		return true;

	bool passedAnyFilter = false;
	for(int i = 0; i < m_filtersTable->numRows(); ++i)
	{
		if(FilterManager::passesFilter(m_filtersTable->text(i, FTC_filterType).latin1(), m_filtersTable->text(i, FTC_condition).latin1(), m_filtersTable->text(i, FTC_value).latin1(), object))
		{
			if(m_matchTypeComboBox->currentText() == "Any")
				return true;
			passedAnyFilter = true;
		}
		else
		{
			if(m_matchTypeComboBox->currentText() == "All")
				return false;
		}
	}

	return passedAnyFilter;
}

//-----------------------------------------------------------------------------

std::string const & PlanetWatcher::getAiActivityString(int const aiActivity) const
{
	switch(aiActivity)
	{
		case(-1):
		{
			return cms_abt_unknownString;
		}
		case(static_cast<int>(AMT_idle)):
		{
			return cms_abt_idleString;
		}
		case(static_cast<int>(AMT_loiter)):
		{
			return cms_abt_loiterString;
		}
		case(static_cast<int>(AMT_wander)):
		{
			return cms_abt_wanderString;
		}
		case(static_cast<int>(AMT_follow)):
		{
			return cms_abt_followString;
		}
		case(static_cast<int>(AMT_flee)):
		{
			return cms_abt_fleeString;
		}
		case(static_cast<int>(AMT_move)):
		{
			return cms_abt_moveString;
		}
		case(static_cast<int>(AMT_patrol)):
		{
			return cms_abt_patrolString;
		}
		case(static_cast<int>(AMT_face)):
		{
			return cms_abt_faceString;
		}
		case(static_cast<int>(AMT_swarm)):
		{
			return cms_abt_swarmString;
		}
		case(static_cast<int>(AMT_invalid)):
		{
			return cms_abt_invalidString;
		}
		default:
		{
			char buffer[512];
			snprintf(buffer, sizeof(buffer)-1, "Unknown AI state %d. FATALING!", aiActivity);
			buffer[sizeof(buffer)-1] = '\0';

			QMessageBox::warning(const_cast<PlanetWatcher*>(this), "Unknown AI State", buffer, "OK");

			FATAL(true, ("Undefined action type!"));
			return cms_abt_unknownString;
		}
	}
}

//-----------------------------------------------------------------------------
bool PlanetWatcher::getUseLevelGradient() const
{
	return m_levelGradientCheckBox->isChecked();
}

// ============================================================================
