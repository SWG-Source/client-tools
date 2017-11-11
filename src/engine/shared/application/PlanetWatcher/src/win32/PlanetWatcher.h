// ============================================================================
//
// PlanetWatcher.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_PlanetWatcher_H
#define INCLUDED_PlanetWatcher_H

#include "BasePlanetWatcher.h"

#include "PlanetWatcherRenderer.h"

#include <map>
#include <set>

class NetworkId;
class PlanetServerConnection;

//-----------------------------------------------------------------------------
class PlanetWatcher : public BasePlanetWatcher
{
	Q_OBJECT

public:

	class GameServerInfo;
	class ProcessInfo;
	class FrameInfo;

	typedef stdmap<int, ProcessInfo>::fwd       ServerProcesses;
	typedef stdmap<uint32, GameServerInfo>::fwd GameServerMap;
	typedef stdvector<uint32>::fwd              Uint32Vector;
	typedef stdvector<int>::fwd                 IntVector;

	PlanetWatcher(QWidget *parent, char const *name);
	virtual ~PlanetWatcher();

	void          update();
	void          setWorld(float const sizeX, float const sizeZ, int const cellCountX, int const cellCountZ);
	void          setGameServerColor(uint32 const gameServerId, QColor const &color);
	//void          setCellGameServerId(float const worldX, float const worldZ, uint32 const gameServerId);
	void          setCellSubscription(float const worldX, float const worldZ, Uint32Vector const &gameServers, IntVector const &subscriptionCounts);

	void          addGameServer(uint32 const gameServerId, QString const &ip, QString const &port, QString const &sceneId);
	void          removeGameServer(uint32 const gameServerId);
	void          removeAllGameServers();

	void          updateObject(NetworkId const &objectId, uint32 gameServerId, float worldX, float worldZ, int interestRadius, int objectTypeTag, int level, bool hibernating, uint32 templateCrc, int aiActivity, int creationType);
	void          removeObject(NetworkId const &objectId);

	void          clearCellGameServersIds(float const worldX, float const worldZ);

	void          connectionSuccess();
	void          connectionLost();

	QColor const &getGameServerColor(uint32 const gameServerId) const;
	uint32        getSelectedGameServerId() const;

	void          setServerLoopTime(int const currentMilliSeconds, int const lastMilliSeconds);

	void          setServerFrameInfo(unsigned int processId, unsigned long frameTime, std::string const &profilerData);

	bool          isDrawAllObjects() const;
	bool          isDrawObjectType(int objectTypeTag) const;

	bool          passesFilters(PlanetWatcherRenderer::MiniMapObject const & object) const;

	std::string const & getAiActivityString(int aiActivity) const;

	bool          getUseLevelGradient() const;

	static QColor const ms_selectedObjects;
	static QColor const ms_mobileObjects;
	static QColor const ms_mobileInterestRadiusObjects;
	static QColor const ms_neverMovedNonCreatureObjects;
	static QColor const ms_neverMovedCreatureObjects;

public slots:

private slots:

	void slotPlanetViewTimerTimeOut();
	void slotBarrierMetricsTimerTimeOut();
	void slotCellSelected();
	void slotExitActionActivated();
	void slotSimulationActionActivated();
	void slotConnectActionActivated();
	void slotLoginConnectActionActivated();
	void slotDisconnectActionActivated();
	void slotServerTableClicked(int, int, int, const QPoint &);
	void slotProcessFrameScrollBarValueChanged(int);
	void slotSnapToCurrentFramePushButtonClicked();
	void slotStopFramePushButtonClicked();
	void slotFilterMsCheckBoxClicked();
	void slotWarnMsCheckBoxClicked();
	void slotClearPushButtonClicked();
	void slotLocateObjectByIdActionActivated();
	void slotDumpInfoPushButtonClicked();
	void slotClearOutputTextWindowPushButtonClicked();
	void slotRefreshFilterPushButtonClicked();
	void slotColorGridCheckBoxClicked();
	void slotAddFilterPushButtonClicked();
	void slotTagFilterTableValueChanged(int row, int col);
	void slotFiltersTableContextMenuRequested(int row, int col, const QPoint & pos);
	void slotFiltersTableDoubleClicked(int row, int col, int button, const QPoint & pos);
	void slotDeleteFilter();
	void slotEditFilter();

signals:

protected:

	virtual void mouseDoubleClickEvent(QMouseEvent *mouseEvent);
	virtual void resizeEvent(QResizeEvent *resizeEvent);

private:

	void initializeTestGameServers(int const serverCount);
	void reFitTable();
	void drawBarrierMetrics();
	void drawFrameInfo(QFrame &frame, int const warnMs) const;
	void setServerFrameInfo(ServerProcesses &serverProcesses, unsigned int processId, unsigned long frameTime, std::string const &profilerData);
	void reportFrameInfo(FrameInfo const &frameInfo, int const processId) const;
	bool isAServerRow(int row) const;

	GameServerMap *         m_gameServers;
	QTimer *                m_planetViewTimer;
	QTimer *                m_barrierMetricsTimer;
	int                     m_selectedRow;
	int                     m_lastSelectedRow;
	int                     m_selectedServerDataStartRow;
	int                     m_lastSelectedServerDataStartRow;
	PlanetServerConnection *m_planetServerConnection;
	int                     m_filterMs;
	int                     m_warnMs;
	std::map<int, int>      m_objectTypeTotalCountMap;
	std::map<int, int>      m_objectTypeSelectedCountMap;
	std::set<int>           m_currentDrawObjectType;
	bool                    m_showingServerData;

	enum GameServerTableColumns
	{
		GSTC_color,
		GSTC_serverName,
		GSTC_cellCount,
		GSTC_objectCount,
		GSTC_IP,
		GSTC_port,
		GSTC_planet
	};

	enum LegendTableColumns
	{
		LTC_color,
		LTC_count,
		LTC_description
	};

	enum TagFilterTableColumns
	{
		TFTC_objectType,
		TFTC_objectCount,
		TFTC_filterSetting
	};

	enum FilterTableColumns
	{
		FTC_filterType,
		FTC_condition,
		FTC_value
	};
};

// ============================================================================

#endif // INCLUDED_PlanetWatcher_H
