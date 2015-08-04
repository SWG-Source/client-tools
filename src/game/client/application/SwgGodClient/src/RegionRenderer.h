// ============================================================================
//
// RegionRenderer.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_RegionRenderer_H
#define INCLUDED_RegionRenderer_H

#include <qcanvas.h>
#include <map>
#include <vector>
#include <string>
#include "sharedFoundationTypes/FoundationTypes.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/NetworkId.h"

//-----------------------------------------------------------------------------

class RegionRenderer : public QFrame
{
	Q_OBJECT

public:

	class ServerObject;
	class WorldCell;

	class Region
	{
	public:
		Region(float const x, float const z, QString const &name) :
			m_worldX(x),
			m_worldZ(z),
			m_name(name),
			m_planet(),
			m_PvP(0),
			m_Municipal(0),
			m_Buildable(0),
			m_Geographical(0),
			m_MinDifficulty(0),
			m_MaxDifficulty(0),
			m_Spawnable(0),
			m_Mission(0) {}

		QString   m_planet;
		QString   m_name;
		float     m_worldX;
		float     m_worldZ;
		int       m_PvP;
		int       m_Municipal;
		int       m_Buildable;
		int       m_Geographical;
		int       m_MinDifficulty;
		int       m_MaxDifficulty;
		int       m_Spawnable;
		int       m_Mission;

		virtual ~Region() {}
	};

	class RegionCircle : public Region
	{
	public:
		RegionCircle(float const x, float const z, float radius, QString const &name)
								: Region(x, z, name),
								m_radius(radius) {}

		float m_radius;
	};

	class RegionRect : public Region
	{
	public:
		RegionRect(float const ll_x, float const ll_z, float ur_x, float ur_z, QString const &name)
								: Region(ll_x, ll_z, name),
									m_ur_worldX(ur_x),
									m_ur_worldZ(ur_z) {}

		float m_ur_worldX;
		float m_ur_worldZ;
	};

	typedef std::map<uint32, QColor>          Uint32ToColorMap;
	typedef std::vector<QColor>               ColorVector;
	typedef std::map<NetworkId, ServerObject> NetworkIdToServerObjectMap;
	typedef std::vector<WorldCell>            WorldCellVector;
	typedef std::vector<uint32>               Uint32Vector;
	typedef std::vector<NetworkId>            NetworkIdVector;
	typedef std::map<uint32, int>             Uint32ToIntMap;
	typedef std::map<std::string, RegionRect*>   RectRegionMap;
	typedef std::map<std::string, RegionCircle*> CircleRegionMap;

	RegionRenderer(QWidget *parent, char const *name);
	~RegionRenderer();

	void          setWorld(float const sizeX, float const sizeZ, int const cellCountX, int const cellCountZ);
	void          updateRectRegion(float const ll_worldX, float const ll_worldZ, float const ur_worldX, float const ur_worldZ, const std::string & name, const std::string & planet, int, bool, int, bool, int, int, int, int);
	void          updateCircleRegion(float const worldX, float const worldZ, int const radius, const std::string & name, const std::string & planet, int, bool, int, bool, int, int, int, int);
	void          removeRegion(NetworkId const &objectId);
	void          updateObject(NetworkId const &objectId, uint32 const gameServerId, float const worldX, float const worldZ, int const interestRadius);
	void          removeObject(NetworkId const &objectId);

	void          setCellSubscription(int const cellX, int const cellY, std::vector<uint32> const &gameServers, std::vector<int> const &subscriptionCounts);
	void          setCellGameServerId(int const cellX, int const cellY, uint32 const gameServerId);

	void          clearCellGameServersIds(int const cellX, int const cellY);
	QColor const &getGameServerColor(uint32 const gameServerId) const;
	void          setGameServerColor(uint32 const gameServerId, QColor const &color);

	void          removeGameServerObjects(uint32 const gameServerId);
	void          removeGameServerCells(uint32 const gameServerId);
//	void          setGameServerConnected(bool const connected);

	void          filterOnPvP(bool filter);
	void          filterOnBuildable(bool filter);
	void          filterOnSpawnable(bool filter);
	void          filterOnGeographical(bool filter);
	void          filterOnMission(bool filter);
	void          filterOnDifficulty(bool filter);
	void          filterOnMunicipal(bool filter);

	float  getZoom() const;
	int    getCellCountX() const;
	int    getCellCountZ() const;
	float  getCellSizeX() const;
	float  getCellSizeZ() const;
	float  getWorldSizeX() const;
	float  getWorldSizeZ() const;
	int    getGameServerCellCount(uint32 const gameServerId) const;
	int    getGameServerObjectCount(uint32 const gameServerId) const;
	int    getCellX(float const worldX) const;
	int    getCellY(float const worldZ) const;

	std::map<std::string, Region*> getVisibleRegions();

	// Cell information

	QPoint getSelectedCellPosition() const;
	void   getCellWorldPosition(QPoint const &cellPosition, QPoint &topLeft, QPoint &bottomRight) const;
	void   getCellGameServerIds(QPoint const &cellPosition, Uint32Vector &gameServerIds) const;
	void   getCellObjects(QPoint const &cellPosition, NetworkIdVector &objectIds) const;

signals:

	void signalCellSelected();

public slots:

private slots:

	void slotTimerTimeOut();

protected:

	virtual void paintEvent(QPaintEvent *paintEvent);
	virtual void resizeEvent(QResizeEvent *resizeEvent);
	virtual void mousePressEvent(QMouseEvent *mouseEvent);
	virtual void mouseReleaseEvent(QMouseEvent *mouseEvent);
	virtual void mouseMoveEvent(QMouseEvent *mouseEvent);
	virtual void mouseDoubleClickEvent(QMouseEvent *mouseEvent);
	virtual void wheelEvent(QWheelEvent *wheelEvent);

private:

	void         setZoom(float const zoom);
	void         zoomIn();
	void         zoomOut();
	void         worldToPixel(float const worldX, float const worldZ, float &pixelX, float &pixelY) const;
	void         pixelToWorld(QPoint const &pixel, float &worldX, float &worldZ) const;
	void         drawGrid(QPainter &painter, QColor const &backGroundColor);
	void         drawGridLabels(QPainter &painter);

//	void         drawMiniMap(QPainter &painter, QPixmap const &pixMap, int const pixelX, int const pixelY, QString const &label);
	void         drawCursor(QPainter &painter);
//	void         drawObjects(QPainter &painter);
	void         drawRegions(QPainter &painter);
	void         drawCircle(QPainter &painter, float const worldX, float const worldZ, float const radius, QColor const &color);
	void         fillCircle(QPainter &painter, float const worldX, float const worldZ, float const radius, QColor const &color, bool const outline);
	void         fillTriangle(QPainter &painter, RegionRenderer::ServerObject const &serverObject, float const radius, QColor const &color, bool const outline);
	void         drawRect(QPainter &painter, float const ll_worldX, float const ll_worldZ,  float const ur_worldX, float const ur_worldZ, QColor const &color);
	void         fillRect(QPainter &painter, float const ll_worldX, float const ll_worldZ,  float const ur_worldX, float const ur_worldZ, QColor const &color, bool const outline);
	QColor       getColor(int const cellX, int const cellY, int index) const;
	void         getColors(int const cellX, int const cellY, ColorVector &colors) const;
	int          getCellServerCount(int const cellX, int const cellY) const;
	void         getScreenCells(int &startCellX, int &startCellY, int &endCellX, int &endCellY) const;
	QRect        getWorldCellScreenPosition(float const worldX, float const worldZ) const;
	unsigned int getCellIndex(int const cellX, int const cellY) const;
	int          getTotalGameServerCellCount() const;

	NetworkIdToServerObjectMap m_objects;
	WorldCellVector            m_worldCells;
	Uint32Vector               m_gameServerIds;
	QPixmap                    m_screen;
	QTimer *                   m_timer;
	Uint32ToIntMap             m_gameServerCellCounts;
	Uint32ToIntMap             m_gameServerObjectCounts;
	Uint32ToColorMap           m_serverColors;
	RectRegionMap              m_rectRegions;
	CircleRegionMap            m_circleRegions;
	float                      m_zoom;
	float const                m_zoomRate;
	float                      m_worldSizeX;
	float                      m_worldSizeZ;
	float                      m_cameraPositionX;
	float                      m_cameraPositionY;
	float                      m_destinationCameraPositionX;
	float                      m_destinationCameraPositionY;
	float                      m_mousePressZoom;
	float                      m_mousePressCameraPositionX;
	float                      m_mousePressCameraPositionY;
	int                        m_cellCountX;
	int                        m_cellCountY;
	QPoint                     m_mouseMovePosition;
	QPoint                     m_mousePressPosition;
	QPoint                     m_selectedCellPosition;
	bool                       m_leftButtonDown;
	bool                       m_rightButtonDown;
	QFont                      m_font;
	bool                       m_filterOnPvp;
	bool                       m_filterOnBuildable;
	bool                       m_filterOnDifficulty;
	bool                       m_filterOnGeographical;
	bool                       m_filterOnMunicipal;
	bool                       m_filterOnMission;
	bool                       m_filterOnSpawnable;

//	bool                       m_gameServerConnected;

	static float const   ms_minZoom;
	static float const   ms_maxZoom;

private:

	// Disabled

	RegionRenderer &operator =(RegionRenderer const &);
};

// ============================================================================

#endif // INCLUDED_RegionRenderer_H
