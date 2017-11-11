// ============================================================================
//
// PlanetWatcherRenderer.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_PlanetWatcherRenderer_H
#define INCLUDED_PlanetWatcherRenderer_H

// These files are included here because PlanetWatcherRenderer is a custom Qt
// widget and you can't define precompiled headers for Qt generated files.

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedFoundation/NetworkId.h"
#include <qpixmap.h>

#include <map>
#include <vector>
#include <string>

class PlanetWatcher;

//-----------------------------------------------------------------------------
class PlanetWatcherRenderer : public QFrame
{
	Q_OBJECT

public:

	//inner class
	class MiniMapObject
	{
	public:
		MiniMapObject(NetworkId const &objectId, int const gameServerId, float const x, float const z, QString const &label, int objectTypeTag, int level, bool hibernating, uint32 templateCrc, int aiAiActivity, int creationType);

		float  getWorldX() const;
		float  getWorldZ() const;
		float  getDirectionX() const;
		float  getDirectionZ() const;
		float  getDrawRadius() const;
		QColor getDrawColor() const;
		int    getMinDiameter() const;
		int    getObjectTypeTag() const;
		int    getLevel() const;
		bool   getHibernating() const;
		uint32 getTemplateCrc() const;
		std::string const & getTemplateCrcHexString() const;
		void   setTemplateCrc(uint32 templateCrc);
		int    getAiActivity() const;
		std::string const & getAiActivityString() const;
		int    getCreationType() const;
		void   move(float const x, float const z);
		bool   hasMoved() const;
		uint32 getGameServerId() const;

		static void setPlanetWatcher(PlanetWatcher const *planetWatcher);

		NetworkId m_objectId;
		uint32    m_gameServerId;
		int       m_interestRadius;
		QString   m_label;
		int       m_objectTypeTag;
		bool      m_isCreatureObject;
		int       m_level;
		bool      m_hibernating;
		int       m_aiActivity;
		int       m_creationType;

	private:
		static PlanetWatcher const *ms_planetWatcher;
		uint32    m_templateCrc;
		std::string m_templateCrcHexString;
		std::vector<float> m_worldX;
		std::vector<float> m_worldZ;
	};

	class WorldCell;

	typedef stdmap<uint32, QColor>::fwd           Uint32ToColorMap;
	typedef stdvector<QColor>::fwd                ColorVector;
	typedef stdmap<NetworkId, MiniMapObject>::fwd NetworkIdToServerObjectMap;
	typedef stdvector<WorldCell>::fwd             WorldCellVector;
	typedef stdvector<uint32>::fwd                Uint32Vector;
	typedef stdvector<NetworkId>::fwd             NetworkIdVector;
	typedef stdmap<uint32, int>::fwd              Uint32ToIntMap;
	typedef stdvector<int>::fwd                   IntVector;

	PlanetWatcherRenderer(QWidget *parent, char const *name);
	~PlanetWatcherRenderer();

	void          setPlanetWatcher(PlanetWatcher const *planetWatcher);
	void          setWorld(float const sizeX, float const sizeZ, int const cellCountX, int const cellCountZ);
	void          updateObject(NetworkId const &objectId, uint32 gameServerId, float worldX, float worldZ, int interestRadius, int objectTypeTag, int level, bool hibernating, uint32 templateCrc, int aiActivity, int creationType);
	void          removeObject(NetworkId const &objectId);

	void          setCellSubscription(int const cellX, int const cellY, Uint32Vector const &gameServers, IntVector const &subscriptionCounts);
	void          setCellGameServerId(int const cellX, int const cellY, uint32 const gameServerId);

	void          clearCellGameServersIds(int const cellX, int const cellY);
	QColor const &getGameServerColor(uint32 const gameServerId) const;
	void          setGameServerColor(uint32 const gameServerId, QColor const &color);

	void          removeGameServerObjects(uint32 const gameServerId);
	void          removeGameServerCells(uint32 const gameServerId);
	void          setGameServerConnected(bool const connected);

	void          setColorGrid(bool colorGrid);
	void          setShowLevelGradient(bool showLevelGradient);

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

	void   getObjectCount(std::map<int, int> & objectTypeTotalCountMap, int & total, std::map<int, int> & objectTypeSelectedCountMap, int & selected, int & moved, int & interestRadius, int & immobileCreature, int & immobileNonCreature, int & selectedMoved, int & selectedInterestRadius, int & selectedImmobileCreature, int & selectedImmobileNonCreature) const;

	MiniMapObject * locateObject(NetworkId const &objectId);

	void   dumpObjectInfo() const;

	stdvector<uint32>::fwd const & getGameServerIds() const;

	// Cell information

	QPoint getSelectedCellPosition() const;
	void   getCellWorldPosition(QPoint const &cellPosition, QPoint &topLeft, QPoint &bottomRight) const;
	void   getCellGameServerIds(QPoint const &cellPosition, Uint32Vector &gameServerIds) const;
	void   getCellObjects(QPoint const &cellPosition, NetworkIdVector &objectIds) const;

	QPoint const &getMousePosition() const;
	void          pixelToWorld(QPoint const &pixel, float &worldX, float &worldZ) const;

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
	void         drawGrid(QPainter &painter, QColor const &backGroundColor);
	void         drawMiniMap(QPainter &painter, QPixmap const &pixMap, int const pixelX, int const pixelY, QString const &label);
	void         drawCursor(QPainter &painter);
	void         drawObjects(QPainter &painter);
	void         drawCircle(QPainter &painter, float const worldX, float const worldZ, float const radius, QColor const &color);
	void         fillCircle(QPainter &painter, PlanetWatcherRenderer::MiniMapObject const &miniMapObject, bool const outline) const;
	void         fillTriangle(QPainter &painter, PlanetWatcherRenderer::MiniMapObject const &miniMapObject, bool const outline) const;
	void         drawRotatingSquare(QPainter &painter, PlanetWatcherRenderer::MiniMapObject const &miniMapObject) const;
	QColor       getColor(int const cellX, int const cellY, int index) const;
	void         getColors(int const cellX, int const cellY, ColorVector &colors) const;
	int          getCellServerCount(int const cellX, int const cellY) const;
	void         getScreenCells(int &startCellX, int &startCellY, int &endCellX, int &endCellY) const;
	QRect        getWorldCellScreenPosition(float const worldX, float const worldZ) const;
	unsigned int getCellIndex(int const cellX, int const cellY) const;
	int          getTotalGameServerCellCount() const;
	QRect        getObjectPixelRect(MiniMapObject const &miniMapObject) const;
	int          getObjectPixelRadius(MiniMapObject const &miniMapObject) const;
	int          getGameServerBitIndex(uint32 const gameServerId) const;
	void         moveCameraToMouseScreenPosition(QPoint const &position);
	bool         validCell(int const cellX, int const cellY) const;

	NetworkIdToServerObjectMap *m_objects;
	Uint32ToIntMap             *m_gameServerObjectCounts;
	Uint32ToColorMap           *m_serverColors;
	WorldCellVector            *m_worldCells;
	Uint32Vector               *m_gameServerIds;
	QPixmap                     m_screen;
	QPixmap                     m_objectMiniMap;
	QPixmap                     m_gameServerMiniMap;
	QTimer *                    m_timer;
	float                       m_zoom;
	float const                 m_zoomRate;
	float                       m_worldSizeX;
	float                       m_worldSizeZ;
	float                       m_cameraPositionX;
	float                       m_cameraPositionY;
	float                       m_destinationCameraPositionX;
	float                       m_destinationCameraPositionY;
	float                       m_mousePressZoom;
	float                       m_mousePressCameraPositionX;
	float                       m_mousePressCameraPositionY;
	int                         m_cellCountX;
	int                         m_cellCountY;
	QPoint                      m_mouseMovePositionLocal;
	QPoint                      m_mousePressPositionLocal;
	QPoint                      m_selectedCellPosition;
	bool                        m_leftButtonDown;
	bool                        m_rightButtonDown;
	QFont                       m_font;
	bool                        m_gameServerConnected;
	NetworkId                   m_selectedObject;
	NetworkId                   m_highlightedObject;
	bool                        m_trackSelected;
	bool                        m_labelDynamics;
	bool                        m_labelStatics;
	PlanetWatcher const *       m_planetWatcher;

	static float const          ms_minZoom;
	static float const          ms_maxZoom;

private:

	// Disabled

	PlanetWatcherRenderer &operator =(PlanetWatcherRenderer const &);
};

// ============================================================================

#endif // INCLUDED_PlanetWatcherRenderer_H
