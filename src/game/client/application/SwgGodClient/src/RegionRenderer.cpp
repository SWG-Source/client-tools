// ============================================================================
//
// RegionRenderer.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "SwgGodClient/FirstSwgGodClient.h"
#include "RegionRenderer.h"
#include "RegionRenderer.moc"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedRandom/Random.h"
#include "sharedMath/Vector2d.h"

#include <qpainter.h>
#include <qtimer.h>

#include <algorithm>

// ============================================================================
//
// RegionRenderer::Region
//
// ============================================================================



// ============================================================================
//
// RegionRenderer::ServerObject
//
// ============================================================================

class RegionRenderer::ServerObject
{
public:

	ServerObject(NetworkId const &objectId, int const gameServerId, float const x, float const z, QString const &label);

	float getWorldX() const;
	float getWorldZ() const;
	float getDirectionX() const;
	float getDirectionZ() const;

	void  move(float const x, float const z);
	bool  hasMoved() const;

	NetworkId m_objectId;
	uint32    m_gameServerId;
	int       m_interestRadius;
	QString   m_label;

private:

	std::vector<float> m_worldX;
	std::vector<float> m_worldZ;
};

//-----------------------------------------------------------------------------

RegionRenderer::ServerObject::ServerObject(NetworkId const &objectId, int const gameServerId, float const x, float const z, QString const &label)
 : m_objectId(objectId)
 , m_gameServerId(gameServerId)
 , m_interestRadius(0)
 , m_worldX()
 , m_worldZ()
 , m_label(label)
{
	m_worldX.push_back(x);
	m_worldZ.push_back(z);
}

//-----------------------------------------------------------------------------

bool RegionRenderer::ServerObject::hasMoved() const
{
	return (m_worldX.size() > 1) || (m_worldZ.size() > 1);
}

//-----------------------------------------------------------------------------

void RegionRenderer::ServerObject::move(float const x, float const z)
{
	// Only update if something changed

	if ((m_worldX.back() != x) && (m_worldZ.back() != z))
	{
		m_worldX.push_back(x);
		m_worldZ.push_back(z);
	}
}

//-----------------------------------------------------------------------------

float RegionRenderer::ServerObject::getWorldX() const
{
	return m_worldX.back();
}

//-----------------------------------------------------------------------------


float RegionRenderer::ServerObject::getWorldZ() const
{
	return m_worldZ.back();
}

//-----------------------------------------------------------------------------

float RegionRenderer::ServerObject::getDirectionX() const
{
	float result = 0.0f;

	if (m_worldX.size() > 1)
	{
		result = m_worldX.back() - m_worldX[m_worldX.size() - 2];
	}

	return result;
}

//-----------------------------------------------------------------------------

float RegionRenderer::ServerObject::getDirectionZ() const
{
	float result = 0.0f;

	if (m_worldZ.size() > 1)
	{
		result = -(m_worldZ.back() - m_worldZ[m_worldZ.size() - 2]);
	}

	return result;
}

// ============================================================================
//
// RegionRenderer::WorldCell
//
// ============================================================================

class RegionRenderer::WorldCell
{
public:

	WorldCell();

	uint32 m_gameServerIds;
};

//-----------------------------------------------------------------------------

RegionRenderer::WorldCell::WorldCell()
 : m_gameServerIds(0)
{
}

// ============================================================================
//
// RegionRenderer
//
// ============================================================================

float const RegionRenderer::ms_minZoom = 0.035f;
float const RegionRenderer::ms_maxZoom = 16.0f;

//-----------------------------------------------------------------------------

RegionRenderer::RegionRenderer(QWidget *parent, char const *name)
 : QFrame(parent, name)
 , m_objects()
 , m_worldCells()
 , m_screen()
 , m_serverColors()
 , m_timer(NULL)
 , m_leftButtonDown(false)
 , m_rightButtonDown(false)
 , m_zoom(ms_minZoom)
 , m_zoomRate(0.2f)
 , m_worldSizeX(0.0f)
 , m_worldSizeZ(0.0f)
 , m_cellCountX(1)
 , m_cellCountY(1)
 , m_cameraPositionX(0.5f)
 , m_cameraPositionY(0.5f)
 , m_destinationCameraPositionX(m_cameraPositionX)
 , m_destinationCameraPositionY(m_cameraPositionY)
 , m_mousePressPosition(0, 0)
 , m_mousePressZoom(1.0f)
 , m_mousePressCameraPositionX(0.0f)
 , m_mousePressCameraPositionY(0.0f)
 , m_gameServerIds()
 , m_gameServerCellCounts()
 , m_gameServerObjectCounts()
 , m_selectedCellPosition(0, 0)
 , m_font("Arial", 10, QFont::Bold),
// , m_gameServerConnected(false)
  m_filterOnPvp(false),
  m_filterOnBuildable(false),
  m_filterOnDifficulty(false),
  m_filterOnGeographical(false),
  m_filterOnMunicipal(false),
  m_filterOnMission(false),
  m_filterOnSpawnable(false)
{
	setMouseTracking(true);
	setBackgroundMode(Qt::NoBackground);

	m_timer = new QTimer(this, "RegionRendererTimer");
	m_timer->start(1000 / 24);

	connect(m_timer, SIGNAL(timeout()), this, SLOT(slotTimerTimeOut()));

	// Default initialization

	setWorld(16000.0f, 16000.0f, 160, 160);
}

//-----------------------------------------------------------------------------

RegionRenderer::~RegionRenderer()
{
	std::for_each(m_rectRegions.begin(),   m_rectRegions.end(),   PointerDeleterPairSecond());
	m_rectRegions.clear();
	std::for_each(m_circleRegions.begin(), m_circleRegions.end(), PointerDeleterPairSecond());
	m_circleRegions.clear();
}

//-----------------------------------------------------------------------------

void RegionRenderer::slotTimerTimeOut()
{
	// Move the camera closer to its destination

	float const scrollRate = 0.6f;

	m_cameraPositionX += (m_destinationCameraPositionX - m_cameraPositionX) * scrollRate;
	m_cameraPositionY += (m_destinationCameraPositionY - m_cameraPositionY) * scrollRate;

	float const distanceX = (m_destinationCameraPositionX - m_cameraPositionX);
	float const distanceY = (m_destinationCameraPositionY - m_cameraPositionY);
	float const distanceSquaredFromDestination = (distanceX * distanceX) + (distanceY * distanceY);
	float const snapDistance = 0.000005f;

	if (distanceSquaredFromDestination > 0.0f)
	{
		if (distanceSquaredFromDestination < (snapDistance * snapDistance))
		{
			m_cameraPositionX = m_destinationCameraPositionX;
			m_cameraPositionY = m_destinationCameraPositionY;
		}

		update();
	}
}

//-----------------------------------------------------------------------------

void RegionRenderer::updateObject(NetworkId const &objectId, uint32 const gameServerId, float const worldX, float const worldZ, int const interestRadius)
{
	NetworkIdToServerObjectMap::iterator iterObjects = m_objects.find(objectId);

	if (iterObjects != m_objects.end())
	{
		// Object already exists, update its information

		iterObjects->second.m_gameServerId = gameServerId;
		iterObjects->second.move(worldX, worldZ);
		iterObjects->second.m_interestRadius = interestRadius;
	}
	else
	{
		// New object, add it to the list

		m_objects.insert(std::make_pair(objectId, ServerObject(objectId, gameServerId, worldX, worldZ, "Name")));

		// Set the game server object count

		Uint32ToIntMap::iterator iterGameServerObjectCounts = m_gameServerObjectCounts.find(gameServerId);

		if (iterGameServerObjectCounts != m_gameServerObjectCounts.end())
		{
			++(iterGameServerObjectCounts->second);
		}
		else
		{
			// Create this game server's counter and add one

			m_gameServerObjectCounts.insert(std::make_pair(gameServerId, 1));
		}
	}
}

//-----------------------------------------------------------------------------

void RegionRenderer::removeObject(NetworkId const &objectId)
{
	NetworkIdToServerObjectMap::iterator iterNetworkIdToServerObjectMap = m_objects.find(objectId);

	if (iterNetworkIdToServerObjectMap != m_objects.end())
	{
		// Decrease one item from the associated game server

		Uint32ToIntMap::iterator iterGameServerObjectCounts = m_gameServerObjectCounts.find(iterNetworkIdToServerObjectMap->second.m_gameServerId);

		if (iterGameServerObjectCounts != m_gameServerObjectCounts.end())
		{
			--(iterGameServerObjectCounts->second);
		}

		// Remove this object

		m_objects.erase(iterNetworkIdToServerObjectMap);
	}
}

//-----------------------------------------------------------------------------

void RegionRenderer::setWorld(float const sizeX, float const sizeY, int const cellCountX, int const cellCountY)
{
	if ((cellCountX > 0) && (cellCountY > 0))
	{
		// Reset all the data

		m_objects.clear();
		m_worldCells.clear();
		m_serverColors.clear();
		m_leftButtonDown = false;
		m_rightButtonDown = false;
		m_zoom = ms_minZoom;
		m_cameraPositionX = 0.5f;
		m_cameraPositionY = 0.5f;
		m_destinationCameraPositionX = m_cameraPositionX;
		m_destinationCameraPositionY = m_cameraPositionY;
		m_mousePressPosition = QPoint(0, 0);
		m_gameServerIds.clear();
		m_gameServerCellCounts.clear();
		m_gameServerObjectCounts.clear();
		m_selectedCellPosition = QPoint(0, 0);

		// Set the world dimensions

		m_worldSizeX = sizeX;
		m_worldSizeZ = sizeY;

		// Set the cell count

		m_cellCountX = cellCountX;
		m_cellCountY = cellCountY;

		m_worldCells.reserve(m_cellCountX * m_cellCountY);

		for (unsigned int i = 0; i < m_worldCells.capacity(); ++i)
		{
			m_worldCells[i].m_gameServerIds = 0;
		}
	}
}

//-----------------------------------------------------------------------------

void RegionRenderer::resizeEvent(QResizeEvent *)
{
	if ((m_screen.width() != contentsRect().width()) ||
		(m_screen.height() != contentsRect().height()))
	{
		m_screen.resize(contentsRect().width(), contentsRect().height());
	}
}

//-----------------------------------------------------------------------------

void RegionRenderer::paintEvent(QPaintEvent *)
{
	if (!isVisible())
	{
		return;
	}

	QPainter painter;

	if (painter.isActive())
	{
		painter.end();
	}

	if (painter.begin(&m_screen))
	{
		QColor backGroundColor("blue");
		// Clear the background
		painter.fillRect(m_screen.rect(), QBrush(backGroundColor));
		//draw the grid boxes over the background
		drawGrid(painter, backGroundColor);
		//draw the regions over the background
		drawRegions(painter);
		//draw the grid labels over the regions (since the often hang over the edge of the map)
		drawGridLabels(painter);
		//draw the cursor over the regions
//		drawCursor(painter);
		//draw the avatar object over the grid lines
//		drawObjects(painter);

		QFontMetrics fontMetrics(painter.font());

		bitBlt(this, contentsRect().left(), contentsRect().top(), &m_screen);

		painter.end();
	}
}

//-----------------------------------------------------------------------------

void RegionRenderer::updateRectRegion(float const ll_worldX, float const ll_worldZ, float const ur_worldX, float const ur_worldZ, const std::string & name, const std::string & planet, int PvP, bool buildable, int spawnable, bool municipal, int geographical, int minDifficulty, int maxDifficulty, int mission)
{
	RectRegionMap::iterator iterRegions = m_rectRegions.find(planet + "." + name);
	if (iterRegions != m_rectRegions.end())
	{
		// Region already exists, update its information
		iterRegions->second->m_planet = planet.c_str();
		iterRegions->second->m_name = name.c_str();
		iterRegions->second->m_worldX = ll_worldX;
		iterRegions->second->m_worldZ = ll_worldZ;
		iterRegions->second->m_ur_worldX = ur_worldX;
		iterRegions->second->m_ur_worldZ = ur_worldZ;
		iterRegions->second->m_PvP = PvP;
		iterRegions->second->m_Municipal = municipal;
		iterRegions->second->m_Buildable = buildable;
		iterRegions->second->m_Geographical = geographical;
		iterRegions->second->m_MinDifficulty = minDifficulty;
		iterRegions->second->m_MaxDifficulty = maxDifficulty;
		iterRegions->second->m_Spawnable = spawnable;
		iterRegions->second->m_Mission = mission;
	}
	else
	{
		m_rectRegions.insert(std::make_pair(planet + "." + name, new RegionRect(ll_worldX, ll_worldZ, ur_worldX, ur_worldZ, "")));
	}
}

//-----------------------------------------------------------------------------

void RegionRenderer::updateCircleRegion(float const worldX, float const worldZ, int const radius, const std::string & name, const std::string & planet, int PvP, bool buildable, int spawnable, bool municipal, int geographical, int minDifficulty, int maxDifficulty, int mission)
{
	CircleRegionMap::iterator iterRegions = m_circleRegions.find(planet + "." + name);
	if (iterRegions != m_circleRegions.end())
	{
		// Region already exists, update its information
		iterRegions->second->m_planet = planet.c_str();
		iterRegions->second->m_name = name.c_str();
		iterRegions->second->m_worldX = worldX;
		iterRegions->second->m_worldZ = worldZ;
		iterRegions->second->m_radius = radius;
		iterRegions->second->m_PvP = PvP;
		iterRegions->second->m_Municipal = municipal;
		iterRegions->second->m_Buildable = buildable;
		iterRegions->second->m_Geographical = geographical;
		iterRegions->second->m_MinDifficulty = minDifficulty;
		iterRegions->second->m_MaxDifficulty = maxDifficulty;
		iterRegions->second->m_Spawnable = spawnable;
		iterRegions->second->m_Mission = mission;
	}
	else
	{
		m_circleRegions.insert(std::make_pair(planet + "." + name, new RegionCircle(worldX, worldZ, radius, "")));
	}
}

//-----------------------------------------------------------------------------

void RegionRenderer::drawRegions(QPainter &painter)
{
	float const regionZoom = 0.05f;
	float ll_pixelX;
	float ll_pixelY;
	float ur_pixelX;
	float ur_pixelY;
	QFontMetrics fontMetrics(painter.font());

	for(RectRegionMap::iterator i1 = m_rectRegions.begin(); i1 != m_rectRegions.end(); ++i1)
	{
		int r = 0;
		int g = 0;
		int b = 0;

		bool black = false;

	//filter on each requested type
		if(i1->second->m_Mission == 0)
		{
			if(m_filterOnMission)
				black = true;
		}
		else
			r += 128;
		if(i1->second->m_Buildable == 0)
		{
			if(m_filterOnBuildable)
				black = true;
		}
		else
			r += 127;
		if(i1->second->m_Spawnable == 0)
		{
			if(m_filterOnSpawnable)
				black = true;
		}
		else
			g += 85;
		
		if(i1->second->m_PvP == 0)
		{
			if(m_filterOnPvp)
				black = true;
		}
		else
			g += 85;

		if(i1->second->m_Geographical == 0)
		{
			if(m_filterOnGeographical)
				black = true;
		}
		else
			g += 85;

		if(i1->second->m_MinDifficulty == 0)
		{
			if(m_filterOnDifficulty)
				black = true;
		}
		else
			b += 128;

		if(i1->second->m_Municipal == 0)
		{
			if(m_filterOnMunicipal)
				black = true;
		}
		else
			b += 127;

		if ( black )
		{
			r = g = b = 0;
		}

		fillRect(painter, i1->second->m_worldX, i1->second->m_worldZ, i1->second->m_ur_worldX, i1->second->m_ur_worldZ, QColor(r, g, b), true);
		if (m_zoom > regionZoom)
		{
			worldToPixel(i1->second->m_worldX,    i1->second->m_worldZ,    ll_pixelX, ll_pixelY);
			worldToPixel(i1->second->m_ur_worldX, i1->second->m_ur_worldZ, ur_pixelX, ur_pixelY);
			QString text(i1->second->m_planet + ": " + i1->second->m_name);
			const int x1 = (ur_pixelX - ll_pixelX) / 2;
			const int x2 = ll_pixelX + x1;
			const int x3 = fontMetrics.width(text) / 2;
			const int x = x2 - x3;
			const int y1 = (ur_pixelY - ll_pixelY) / 2;
			const int y = ll_pixelY + y1;
			painter.setPen(QColor("black"));
			painter.drawText(x + 1, y + 1, text);
			painter.setPen(QColor("white"));
			painter.drawText(x, y, text);
		}

	}

	for(CircleRegionMap::iterator i2 = m_circleRegions.begin(); i2 != m_circleRegions.end(); ++i2)
	{
		int r = 0;
		int g = 0;
		int b = 0;

		bool black = false;

		//filter on each requested type
		if(i2->second->m_Mission == 0)
		{
			if(m_filterOnMission)
				black = true;
		}
		else
			r += 128;
		if(i2->second->m_Buildable == 0)
		{
			if(m_filterOnBuildable)
				black = true;
		}
		else
			r += 127;
		if(i2->second->m_Spawnable == 0)
		{
			if(m_filterOnSpawnable)
				black = true;
		}
		else
			g += 85;
		
		if(i2->second->m_PvP == 0)
		{
			if(m_filterOnPvp)
				black = true;
		}
		else
			g += 85;

		if(i2->second->m_Geographical == 0)
		{
			if(m_filterOnGeographical)
				black = true;
		}
		else
			g += 85;

		if(i2->second->m_MinDifficulty == 0)
		{
			if(m_filterOnDifficulty)
				black = true;
		}
		else
			b += 128;

		if(i2->second->m_Municipal == 0)
		{
			if(m_filterOnMunicipal)
				black = true;
		}
		else
			b += 127;


		if ( black )
		{
			r = g = b = 0;
		}



		fillCircle(painter, i2->second->m_worldX, i2->second->m_worldZ, i2->second->m_radius, QColor(r, g, b), true);
		if (m_zoom > regionZoom)
		{
			worldToPixel(i2->second->m_worldX,    i2->second->m_worldZ,    ll_pixelX, ll_pixelY);

			QString text(i2->second->m_planet + ": " + i2->second->m_name);
			int const x = ll_pixelX - fontMetrics.width(text) / 2;
			int const y = ll_pixelY;

			painter.setPen(QColor("black"));
			painter.drawText(x + 1, y + 1, text);
			painter.setPen(QColor("white"));
			painter.drawText(x, y, text);
		}


	}
}


//-----------------------------------------------------------------------------

std::map<std::string, RegionRenderer::Region*> RegionRenderer::getVisibleRegions()
{
	std::map<std::string, RegionRenderer::Region*> visibleRegions;

	for(RectRegionMap::iterator i1 = m_rectRegions.begin(); i1 != m_rectRegions.end(); ++i1)
	{
		//filter on each requested type
		if(i1->second->m_Mission == 0)
		{
			if(m_filterOnMission)
				continue;
		}

		if(i1->second->m_Buildable == 0)
		{
			if(m_filterOnBuildable)
				continue;
		}

		if(i1->second->m_Spawnable == 0)
		{
			if(m_filterOnSpawnable)
				continue;
		}
		
		if(i1->second->m_PvP == 0)
		{
			if(m_filterOnPvp)
				continue;
		}

		if(i1->second->m_Geographical == 0)
		{
			if(m_filterOnGeographical)
				continue;
		}

		if(i1->second->m_MinDifficulty == 0)
		{
			if(m_filterOnDifficulty)
				continue;
		}

		if(i1->second->m_Municipal == 0)
		{
			if(m_filterOnMunicipal)
				continue;
		}

		visibleRegions[i1->first] = i1->second;
	}


	for(CircleRegionMap::iterator i2 = m_circleRegions.begin(); i2 != m_circleRegions.end(); ++i2)
	{
		//filter on each requested type
		if(i2->second->m_Mission == 0)
		{
			if(m_filterOnMission)
				continue;
		}

		if(i2->second->m_Buildable == 0)
		{
			if(m_filterOnBuildable)
				continue;
		}

		if(i2->second->m_Spawnable == 0)
		{
			if(m_filterOnSpawnable)
				continue;
		}
		
		if(i2->second->m_PvP == 0)
		{
			if(m_filterOnPvp)
				continue;
		}

		if(i2->second->m_Geographical == 0)
		{
			if(m_filterOnGeographical)
				continue;
		}

		if(i2->second->m_MinDifficulty == 0)
		{
			if(m_filterOnDifficulty)
				continue;
		}

		if(i2->second->m_Municipal == 0)
		{
			if(m_filterOnMunicipal)
				continue;
		}

		visibleRegions[i2->first] = i2->second;
	}

	return visibleRegions;
}

//-----------------------------------------------------------------------------

void RegionRenderer::mousePressEvent(QMouseEvent *mouseEvent)
{
	m_mousePressPosition = mouseEvent->globalPos();
	m_mousePressZoom = getZoom();
	m_mousePressCameraPositionX = m_cameraPositionX;
	m_mousePressCameraPositionY = m_cameraPositionY;

	switch (mouseEvent->button())
	{
		case Qt::LeftButton:
			{
				m_leftButtonDown = true;
			}
			break;
		case Qt::RightButton:
			{
				m_rightButtonDown = true;
			}
			break;
		case Qt::MidButton:
			{
			}
			break;
		case Qt::NoButton:
			{
			}
			break;
	}

	update();
}

//-----------------------------------------------------------------------------

void RegionRenderer::mouseReleaseEvent(QMouseEvent *mouseEvent)
{
	switch (mouseEvent->button())
	{
		case Qt::LeftButton:
			{
				m_leftButtonDown = false;
			}
			break;
		case Qt::RightButton:
			{
				m_rightButtonDown = false;
			}
			break;
		case Qt::MidButton:
			{
			}
			break;
		case Qt::NoButton:
			{
			}
			break;
	}

	update();
}

//-----------------------------------------------------------------------------

void RegionRenderer::setZoom(float const zoom)
{
	if (zoom < ms_minZoom)
	{
		m_zoom = ms_minZoom;
	}
	else if (zoom > ms_maxZoom)
	{
		m_zoom = ms_maxZoom;
	}
	else
	{
		m_zoom = zoom;
	}

	update();
}

//-----------------------------------------------------------------------------

float RegionRenderer::getZoom() const
{
	return m_zoom;
}

//-----------------------------------------------------------------------------

void RegionRenderer::zoomIn()
{
	float zoom = getZoom() * m_zoomRate;
	float const zoomMax = 16.0f;

	if (zoom > zoomMax)
	{
		zoom = zoomMax;
	}

	setZoom(zoom);
}

//-----------------------------------------------------------------------------

void RegionRenderer::zoomOut()
{
	float zoom = getZoom() / m_zoomRate;
	float const zoomMin = 0.0f;
	
	if (zoom < zoomMin)
	{
		zoom = zoomMin;
	}

	setZoom(zoom);
}

//-----------------------------------------------------------------------------

void RegionRenderer::mouseDoubleClickEvent(QMouseEvent *mouseEvent)
{
	if (mouseEvent->button() == Qt::RightButton)
	{
		// Get the change in movement

		QPoint centerOfScreen(contentsRect().size().width() / 2, contentsRect().size().height() / 2);
		QPoint const mouseDelta(centerOfScreen - mouseEvent->pos());

		float cameraPositionDeltaX = static_cast<float>(mouseDelta.x()) / (m_worldSizeX * m_zoom);
		float cameraPositionDeltaY = static_cast<float>(mouseDelta.y()) / (m_worldSizeZ * m_zoom);

		m_destinationCameraPositionX = m_mousePressCameraPositionX - cameraPositionDeltaX;
		m_destinationCameraPositionY = m_mousePressCameraPositionY - cameraPositionDeltaY;

		// Save the cell that was clicked

		float const pixelsPerCellX = static_cast<float>(m_worldSizeX) / static_cast<float>(m_cellCountX);
		float const pixelsPerCellY = static_cast<float>(m_worldSizeZ) / static_cast<float>(m_cellCountY);

		int const cellX = static_cast<int>(floor((m_destinationCameraPositionX * m_worldSizeX) / pixelsPerCellX));
		int const cellY = static_cast<int>(ceil((m_destinationCameraPositionY * m_worldSizeZ) / pixelsPerCellY));

		m_selectedCellPosition = QPoint(cellX, cellY);

		emit signalCellSelected();
	}
}

//-----------------------------------------------------------------------------

void RegionRenderer::mouseMoveEvent(QMouseEvent *mouseEvent)
{
	// Save the mouse position

	m_mouseMovePosition = mouseEvent->pos();

	// Get the change in movement

	QPoint const mouseDelta(mouseEvent->globalPos() - m_mousePressPosition);

	if (m_rightButtonDown)
	{
		// Pan the view

		float cameraPositionDeltaX = static_cast<float>(mouseDelta.x()) / (m_worldSizeX * m_zoom);
		float cameraPositionDeltaY = static_cast<float>(mouseDelta.y()) / (m_worldSizeZ * m_zoom);

		m_cameraPositionX = m_mousePressCameraPositionX - cameraPositionDeltaX;
		m_cameraPositionY = m_mousePressCameraPositionY - cameraPositionDeltaY;
		m_destinationCameraPositionX = m_cameraPositionX;
		m_destinationCameraPositionY = m_cameraPositionY;
	}
	else if (m_leftButtonDown)
	{
	}

	// Needed for cursor redraw when the mouse moves

	update();
}

//-----------------------------------------------------------------------------

void RegionRenderer::drawCircle(QPainter &painter, float const worldX, float const worldZ, float const radius, QColor const &color)
{
	float pixelX;
	float pixelY;

	worldToPixel(worldX, worldZ, pixelX, pixelY);

	QPoint const pixel(static_cast<int>(pixelX), static_cast<int>(pixelY));
	int const width = static_cast<int>(radius * 0.5f * m_zoom);
	int const height = width;
	
	int const minDiameter = 3;
	int const drawWidth = (width < minDiameter) ? minDiameter : width;
	int const drawHeight = (height < minDiameter) ? minDiameter : height;
	
	painter.setPen(color);
	painter.drawEllipse(pixel.x() - (drawWidth / 2), pixel.y() - (drawHeight / 2), drawWidth, drawHeight);
}

//-----------------------------------------------------------------------------

void RegionRenderer::fillCircle(QPainter &painter, float const worldX, float const worldZ, float const radius, QColor const &color, bool const outline)
{
	float pixelX;
	float pixelY;

	worldToPixel(worldX, worldZ, pixelX, pixelY);

	QPoint const pixel(static_cast<int>(pixelX), static_cast<int>(pixelY));
	int const width = static_cast<int>(radius * 0.5f * m_zoom);
	int const height = width;
	
	int const minDiameter = 3;
	int const drawWidth = (width < minDiameter) ? minDiameter : width;
	int const drawHeight = (height < minDiameter) ? minDiameter : height;
	
	painter.setBrush(QBrush(color));
	if (outline)
	{
		painter.setPen("black");
	}
	else
	{
		painter.setPen(color);
	}
	painter.drawEllipse(pixel.x() - (drawWidth / 2), pixel.y() - (drawHeight / 2), drawWidth, drawHeight);
}

//-----------------------------------------------------------------------------

void RegionRenderer::drawRect(QPainter &painter, float const ll_worldX, float const ll_worldZ,  float const ur_worldX, float const ur_worldZ, QColor const &color)
{
	float ll_pixelX;
	float ll_pixelY;
	float ur_pixelX;
	float ur_pixelY;

	worldToPixel(ll_worldX, ll_worldZ, ll_pixelX, ll_pixelY);
	worldToPixel(ur_worldX, ur_worldZ, ur_pixelX, ur_pixelY);

	const float width = ur_pixelX - ll_pixelX;
	const float height = ur_pixelY - ll_pixelY;

	if (width < 0)
		return;

	if (height < 0)
		return;

	painter.setPen(color);
	painter.drawRect(ll_pixelX, ll_pixelY, width, -height);
}

//-----------------------------------------------------------------------------

void RegionRenderer::fillRect(QPainter &painter, float const ll_worldX, float const ll_worldZ,  float const ur_worldX, float const ur_worldZ, QColor const &color, bool const outline)
{
	float ll_pixelX;
	float ll_pixelY;
	float ur_pixelX;
	float ur_pixelY;

	worldToPixel(ll_worldX, ll_worldZ, ll_pixelX, ll_pixelY);
	worldToPixel(ur_worldX, ur_worldZ, ur_pixelX, ur_pixelY);

	const float width = abs(ur_pixelX - ll_pixelX);
	const float height = abs(ur_pixelY - ll_pixelY);

	painter.setBrush(QBrush(color));
	if (outline)
	{
		painter.setPen("black");
	}
	else
	{
		painter.setPen(color);
	}
	painter.drawRect(ll_pixelX, ll_pixelY, width, -height);
}

//-----------------------------------------------------------------------------

void RegionRenderer::fillTriangle(QPainter &painter, RegionRenderer::ServerObject const &serverObject, float const radius, QColor const &color, bool const outline)
{
	float pixelX;
	float pixelY;

	Vector2d forwardVector(0.0f, -1.0f);

	if (serverObject.hasMoved())
	{
		forwardVector = Vector2d(serverObject.getDirectionX(), serverObject.getDirectionZ());
		forwardVector.normalize();
	}

	Vector2d sideVector(forwardVector / 2.0f);
	sideVector.rotate(PI / 2.0f);

	worldToPixel(serverObject.getWorldX(), serverObject.getWorldZ(), pixelX, pixelY);

	QPoint point(static_cast<int>(pixelX), static_cast<int>(pixelY));
	int const width = static_cast<int>(radius * 0.5f * m_zoom);
	int const height = width;
	
	int const minDiameter = 5;
	int const drawWidth = (width < minDiameter) ? minDiameter : width;
	int const drawHeight = (height < minDiameter) ? minDiameter : height;

	int const x1 = point.x() + static_cast<int>(forwardVector.x * static_cast<float>(drawHeight));
	int const y1 = point.y() + static_cast<int>(forwardVector.y * static_cast<float>(drawHeight));
	int const x2 = point.x() + static_cast<int>(sideVector.x * static_cast<float>(drawWidth) + forwardVector.x * static_cast<float>(-drawHeight));
	int const y2 = point.y() + static_cast<int>(sideVector.y * static_cast<float>(drawWidth) + forwardVector.y * static_cast<float>(-drawHeight));
	int const x3 = point.x() + static_cast<int>(sideVector.x * static_cast<float>(-drawWidth) + forwardVector.x * static_cast<float>(-drawHeight));
	int const y3 = point.y() + static_cast<int>(sideVector.y * static_cast<float>(-drawWidth) + forwardVector.y * static_cast<float>(-drawHeight));
	
	QPointArray pointArray(3);
	pointArray.setPoint(0, x1, y1);
	pointArray.setPoint(1, x3, y3);
	pointArray.setPoint(2, x2, y2);
	//pointArray.setPoint(0, point.x() - 10, point.y() - 10);
	//pointArray.setPoint(1, point.x() + 10, point.y() - 10);
	//pointArray.setPoint(2, point.x(), point.y() + 10);
	painter.setBrush(QBrush(color));
	if (outline)
	{
		painter.setPen("black");
	}
	else
	{
		painter.setPen(color);
	}
	painter.drawPolygon(pointArray);
}

//-----------------------------------------------------------------------------

void RegionRenderer::worldToPixel(float const worldX, float const worldY, float &pixelX, float &pixelY) const
{
	float const cameraCenterPixelX = ((m_worldSizeX / 2.0f) - (m_worldSizeX * m_cameraPositionX)) * m_zoom;
	float const cameraCenterPixelY = ((m_worldSizeZ / 2.0f) - (m_worldSizeZ * m_cameraPositionY)) * m_zoom;

	pixelX = static_cast<float>((contentsRect().width() / 2) + cameraCenterPixelX) + (worldX * m_zoom);
	pixelY = static_cast<float>((contentsRect().height() / 2) + cameraCenterPixelY) - (worldY * m_zoom);
}

//-----------------------------------------------------------------------------

void RegionRenderer::pixelToWorld(QPoint const &pixel, float &worldX, float &worldZ) const
{
	float left;
	float top;
	float right;
	float bottom;

	worldToPixel(-m_worldSizeX / 2.0f, m_worldSizeZ / 2.0f, left, top);
	worldToPixel(+m_worldSizeX / 2.0f, -m_worldSizeZ / 2.0f, right, bottom);
	
	float percentX = (static_cast<float>(pixel.x()) - left) / static_cast<float>(right - left);
	float percentY = (static_cast<float>(pixel.y()) - top) / static_cast<float>(bottom - top);
	
	worldX = -(m_worldSizeX / 2.0f) + (m_worldSizeX * percentX);
	worldZ = (m_worldSizeZ / 2.0f) - (m_worldSizeZ * percentY);
}

//-----------------------------------------------------------------------------

int RegionRenderer::getCellServerCount(int const cellX, int const cellY) const
{
	int const serverBits = m_worldCells[getCellIndex(cellX, cellY)].m_gameServerIds;
	int count = 0;

	if (serverBits & (1 << 0)) { ++count; }
	if (serverBits & (1 << 1)) { ++count; }
	if (serverBits & (1 << 2)) { ++count; }
	if (serverBits & (1 << 3)) { ++count; }
	if (serverBits & (1 << 4)) { ++count; }
	if (serverBits & (1 << 5)) { ++count; }
	if (serverBits & (1 << 6)) { ++count; }
	if (serverBits & (1 << 7)) { ++count; }
	if (serverBits & (1 << 8)) { ++count; }
	if (serverBits & (1 << 9)) { ++count; }
	if (serverBits & (1 << 10)) { ++count; }
	if (serverBits & (1 << 11)) { ++count; }
	if (serverBits & (1 << 12)) { ++count; }
	if (serverBits & (1 << 13)) { ++count; }
	if (serverBits & (1 << 14)) { ++count; }
	if (serverBits & (1 << 15)) { ++count; }
	if (serverBits & (1 << 16)) { ++count; }
	if (serverBits & (1 << 17)) { ++count; }
	if (serverBits & (1 << 18)) { ++count; }
	if (serverBits & (1 << 19)) { ++count; }
	if (serverBits & (1 << 20)) { ++count; }
	if (serverBits & (1 << 21)) { ++count; }
	if (serverBits & (1 << 22)) { ++count; }
	if (serverBits & (1 << 23)) { ++count; }
	if (serverBits & (1 << 24)) { ++count; }
	if (serverBits & (1 << 25)) { ++count; }
	if (serverBits & (1 << 26)) { ++count; }
	if (serverBits & (1 << 27)) { ++count; }
	if (serverBits & (1 << 28)) { ++count; }
	if (serverBits & (1 << 29)) { ++count; }
	if (serverBits & (1 << 30)) { ++count; }
	if (serverBits & (1 << 31)) { ++count; }

	return count;
}

//-----------------------------------------------------------------------------

void RegionRenderer::getColors(int const cellX, int const cellY, ColorVector &colors) const
{
	colors.clear();

	int const colorCount = getCellServerCount(cellX, cellY);

	if (colorCount <= 0)
	{
		// No colors assigned

		colors.push_back(QColor(128, 128, 128));
	}
	else
	{
		// Colors assigned

		for (int i = 0; i < colorCount; ++i)
		{
			colors.push_back(getColor(cellX, cellY, i));
		}
	}
}

//-----------------------------------------------------------------------------

QColor RegionRenderer::getColor(int const cellX, int const cellY, int const index) const
{
	QColor result;

	// ROYGBIV

	unsigned int colorIndex = 0;
	int foundColorCount = 0;
	int const cellIndex = getCellIndex(cellX, cellY);
	int const gameServerIds = m_worldCells[cellIndex].m_gameServerIds;

	if (gameServerIds <= 0)
	{
		// This cell is not assigned a game server

		result = QColor(128, 128, 128);
	}
	else
	{
		for (int i = 0; i < 32; ++i)
		{
			if (gameServerIds & (1 << i))
			{
				if (foundColorCount == index)
				{
					colorIndex = i;
					break;
				}

				++foundColorCount;
			}
		}

#ifdef _DEBUG
		unsigned int const gameServerIdsCount = m_gameServerIds.size();
		DEBUG_FATAL((colorIndex >= gameServerIdsCount), ("out of bounds"));
#endif // _DEBUG

		result = getGameServerColor(m_gameServerIds[colorIndex]);
	}

	return result;
}

//-----------------------------------------------------------------------------

void RegionRenderer::drawGrid(QPainter &painter, QColor const &backGroundColor)
{
	UNREF(backGroundColor);
//	float const gridLabelZoom = 0.1f;
	float const gridZoom = 0.2f;

	// Draw the grid lines

	float const worldCellSizeX = static_cast<float>(m_worldSizeX) / static_cast<float>(m_cellCountX);
	float const worldCellSizeY = static_cast<float>(m_worldSizeZ) / static_cast<float>(m_cellCountY);

	int startCellX;
	int startCellY;
	int endCellX;
	int endCellY;
	
	getScreenCells(startCellX, startCellY, endCellX, endCellY);
	
	ColorVector colors;
	colors.reserve(16);

	for (int x = startCellX; x < endCellX; ++x)
	{
		for (int y = startCellY; y < endCellY; ++y)
		{
			// Get the cell
	
			float const worldX = -m_worldSizeX / 2.0f + static_cast<float>(x) * worldCellSizeX;
			float const worldZ = m_worldSizeZ / 2.0f + -static_cast<float>(y) * worldCellSizeY;
	
			QRect cellScreenRect(getWorldCellScreenPosition(worldX, worldZ));
	
			// Get the color
	
			unsigned int const colorCount = getCellServerCount(x, y);
	
			if (colorCount > 1)
			{
				if (m_zoom > gridZoom)
				{
					// Multi color cell
	
					getColors(x, y, colors);
	
					for (unsigned int i = 0; i < colorCount; ++i)
					{
						// Draw the cell
	
						QColor color(colors[i]);
						QBrush brush(color);
	
						float const subWidth = static_cast<float>(cellScreenRect.width()) / static_cast<float>(colorCount);
						int const cellX = cellScreenRect.topLeft().x();
						int const cellY = cellScreenRect.topLeft().y();
	
						QPoint a(cellX + static_cast<int>(subWidth * static_cast<float>(i)), cellY);
						QPoint b(cellX + static_cast<int>(subWidth * static_cast<float>(i + 1)), cellY + cellScreenRect.height() - 1);
	
						QRect sharedCellRect(a, b);
						painter.fillRect(sharedCellRect, brush);
					}
				}
				else
				{
					// White cell for shared cells
	
					QBrush brush(QColor("white"));
					painter.fillRect(cellScreenRect, brush);
				}
			}
			else
			{
				// Single color cell
	
				QColor color(getColor(x, y, 0));
	
				// Draw the cell
	
				QBrush brush(color);
				painter.fillRect(cellScreenRect, brush);
			}
		}
	}

	if (m_zoom > gridZoom)
	{
		int const fontSize = (m_zoom < 0.3f) ? 8 : 12;
		m_font.setPointSize(fontSize);
		painter.setFont(m_font);
		QFontMetrics fontMetrics(m_font);

		// X Axis
		
		for (int x = startCellX; x <= endCellX; ++x)
		{
			float const worldX = -m_worldSizeX / 2.0f + worldCellSizeX * static_cast<float>(x);
   			
   			float leftPixel;
   			float topPixel;
   			float rightPixel;
   			float bottomPixel;
   
			worldToPixel(worldX, m_worldSizeZ / 2.0f, leftPixel, topPixel);
			worldToPixel(worldX, -m_worldSizeZ / 2.0f, rightPixel, bottomPixel);

			QPoint a(static_cast<int>(leftPixel), static_cast<int>(topPixel));
			QPoint b(static_cast<int>(rightPixel), static_cast<int>(bottomPixel));
			
			painter.setPen(QColor("black"));
			painter.drawLine(a, b);
		}
		
		// Y Axis
		
		for (int y = startCellY; y <= endCellY; ++y)
		{
			float const worldY = m_worldSizeZ / 2.0f - worldCellSizeY * static_cast<float>(y);
   			
   			float leftPixel;
   			float topPixel;
   			float rightPixel;
   			float bottomPixel;
   
			worldToPixel(-m_worldSizeX / 2.0f, worldY, leftPixel, topPixel);
			worldToPixel(m_worldSizeX / 2.0f, worldY, rightPixel, bottomPixel);

			QPoint a(static_cast<int>(leftPixel), static_cast<int>(topPixel));
			QPoint b(static_cast<int>(rightPixel), static_cast<int>(bottomPixel));

			painter.setPen(QColor("black"));
			painter.drawLine(a, b);
		}
	}
}


//-----------------------------------------------------------------------------

void RegionRenderer::drawGridLabels(QPainter &painter)
{
//	UNREF(backGroundColor);
	float const gridLabelZoom = 0.1f;
	float const gridZoom = 0.2f;

	// Draw the grid lines

	float const worldCellSizeX = static_cast<float>(m_worldSizeX) / static_cast<float>(m_cellCountX);
	float const worldCellSizeY = static_cast<float>(m_worldSizeZ) / static_cast<float>(m_cellCountY);

	int startCellX;
	int startCellY;
	int endCellX;
	int endCellY;
	
	getScreenCells(startCellX, startCellY, endCellX, endCellY);
	
	ColorVector colors;
	colors.reserve(16);
	int x = 0;
	int y = 0;

	if (m_zoom > gridZoom)
	{
		int const fontSize = (m_zoom < 0.3f) ? 8 : 12;
		m_font.setPointSize(fontSize);
		painter.setFont(m_font);
		QFontMetrics fontMetrics(m_font);

		// X grid label

		for (x = startCellX; x <= endCellX; ++x)
		{
			float const worldX = -m_worldSizeX / 2.0f + worldCellSizeX * static_cast<float>(x);
   			
   			float leftPixel;
   			float topPixel;
   			float rightPixel;
   			float bottomPixel;
   
			worldToPixel(worldX, m_worldSizeZ / 2.0f, leftPixel, topPixel);
			worldToPixel(worldX, -m_worldSizeZ / 2.0f, rightPixel, bottomPixel);

			// Draw the cell x location

			if ((x < endCellX) && (m_zoom > gridLabelZoom))
			{
				int const textX = leftPixel + 2;
				int const textY = fontMetrics.height();

				painter.setPen("black");
				painter.drawText(textX, textY, QString::number((-m_cellCountX / 2 + x) * 100));
				painter.setPen("white");
				painter.drawText(textX - 1, textY - 1, QString::number((-m_cellCountX / 2 + x) * 100));
			}
		}

		// Y grid label
		
		for (y = startCellY; y <= endCellY; ++y)
		{
			float const worldY = m_worldSizeZ / 2.0f - worldCellSizeY * static_cast<float>(y);
   			
   			float leftPixel;
   			float topPixel;
   			float rightPixel;
   			float bottomPixel;
   
			worldToPixel(-m_worldSizeX / 2.0f, worldY, leftPixel, topPixel);
			worldToPixel(m_worldSizeX / 2.0f, worldY, rightPixel, bottomPixel);

			// Draw the cell y location

			if ((y < endCellY) && (m_zoom > gridLabelZoom))
			{
				int const textX = 2;
				int const textY = topPixel + fontMetrics.height();

				painter.setPen("black");
				painter.drawText(textX, textY, QString::number((m_cellCountY / 2 - y) * 100));
				painter.setPen("white");
				painter.drawText(textX - 1, textY - 1, QString::number((m_cellCountY / 2 - y) * 100));
			}
		}
	}
}

//-----------------------------------------------------------------------------

void RegionRenderer::setCellGameServerId(int const cellX, int const cellY, uint32 const gameServerId)
{
	if ((cellX >= 0) &&
	    (cellX < m_cellCountX) &&
	    (cellY >= 0) &&
	    (cellY < m_cellCountY))
	{
		// See if this server id is already stored

		Uint32Vector::const_iterator iterGameServerIds = m_gameServerIds.begin();

		bool serverIdAlreadyUsed = false;
		uint32 serverIdIndex = 0;

		for (; iterGameServerIds != m_gameServerIds.end(); ++iterGameServerIds)
		{
			if (*iterGameServerIds == gameServerId)
			{
				serverIdAlreadyUsed = true;
				break;
			}

			++serverIdIndex;
		}

		// See if we need to add the server id

		if (!serverIdAlreadyUsed)
		{
			m_gameServerIds.push_back(gameServerId);
		}

		FATAL((m_gameServerIds.size() > 32), ("RegionRenderer::setCellServerId() - Too many unique server ids (> 32), rethink the implementation."));

		// Set the bit corresponding to the server id list

		m_worldCells[getCellIndex(cellX, cellY)].m_gameServerIds |= (1 << serverIdIndex);
	}

	// Set the game server cell count

	Uint32ToIntMap::iterator iterGameServerCellCounts = m_gameServerCellCounts.find(gameServerId);

	if (iterGameServerCellCounts != m_gameServerCellCounts.end())
	{
		// Add to the cell count for this game server

		++(iterGameServerCellCounts->second);
	}
	else
	{
		// Create this game server count and add one

		m_gameServerCellCounts.insert(std::make_pair(gameServerId, 1));
	}
}

//-----------------------------------------------------------------------------

void RegionRenderer::setCellSubscription(int const cellX, int const cellY, std::vector<uint32> const &gameServers, std::vector<int> const &subscriptionCounts)
{
	UNREF(subscriptionCounts);

	std::vector<uint32>::const_iterator iterGameServers = gameServers.begin();

	for (; iterGameServers != gameServers.end(); ++iterGameServers)
	{
		uint32 const gameServerId = *iterGameServers;

		setCellGameServerId(cellX, cellY, gameServerId);
	}
}

//-----------------------------------------------------------------------------

void RegionRenderer::getScreenCells(int &startCellX, int &startCellY, int &endCellX, int &endCellY) const
{
	float worldX;
	float worldZ;
	
	float const cellSizeX = m_worldSizeX / m_cellCountX;
	float const cellSizeY = m_worldSizeZ / m_cellCountY;
	
	// Start X
	
	pixelToWorld(QPoint(0, 0), worldX, worldZ);
	
	startCellX = static_cast<int>((m_worldSizeX / 2.0f) + worldX) / cellSizeX;
	startCellX -= 1;
	
	if (startCellX < 0)
	{
		startCellX = 0;
	}
	
	if (startCellX > m_cellCountX)
	{
		startCellX = m_cellCountX;
	}
	
	// Start Y
	
	startCellY = static_cast<int>((m_worldSizeZ / 2.0f) - worldZ) / cellSizeY;
	startCellY -= 1;
	
	if (startCellY < 0)
	{
		startCellY = 0;
	}
	
	if (startCellY > m_cellCountY)
	{
		startCellY = m_cellCountY;
	}
	
	// End X
	
	pixelToWorld(QPoint(contentsRect().width(), contentsRect().height()), worldX, worldZ);
	
	endCellX = static_cast<int>((m_worldSizeX / 2.0f) + worldX) / cellSizeX;
	endCellX += 1;
	
	if (endCellX < 0)
	{
		endCellX = 0;
	}
	
	if (endCellX > m_cellCountX)
	{
		endCellX = m_cellCountX;
	}
	
	// End Y
	
	endCellY = static_cast<int>((m_worldSizeZ / 2.0f) - worldZ) / cellSizeY;
	endCellY += 2;
	
	if (endCellY < 0)
	{
		endCellY = 0;
	}
	
	if (endCellY > m_cellCountY)
	{
		endCellY = m_cellCountY;
	}
}

//-----------------------------------------------------------------------------

int RegionRenderer::getCellCountX() const
{
	return m_cellCountX;
}

//-----------------------------------------------------------------------------

int RegionRenderer::getCellCountZ() const
{
	return m_cellCountY;
}

//-----------------------------------------------------------------------------

void RegionRenderer::drawCursor(QPainter &painter)
{
	// Draw the outline of the current square the mouse is over
	
	float worldX;
	float worldZ;

	pixelToWorld(m_mouseMovePosition, worldX, worldZ);
	
	QRect cellScreenPosition(getWorldCellScreenPosition(worldX, worldZ));
	QPoint topLeft(cellScreenPosition.topLeft());
	QPoint bottomRight(cellScreenPosition.bottomRight());

	painter.setPen(QColor("white"));
	painter.drawRect(QRect(QPoint(topLeft.x() + 1, topLeft.y() + 1), QPoint(bottomRight.x() + 0, bottomRight.y() + 0)));
	painter.setPen(QColor("black"));
	painter.drawRect(QRect(QPoint(topLeft.x() + 0, topLeft.y() + 0), QPoint(bottomRight.x() + 1, bottomRight.y() + 1)));
	painter.drawRect(QRect(QPoint(topLeft.x() + 2, topLeft.y() + 2), QPoint(bottomRight.x() - 1, bottomRight.y() - 1)));

	// Draw some whiskers to help locate the cell coordinates

	QFontMetrics fontMetrics(m_font);
	painter.setPen(QColor("white"));
	int const startY = static_cast<int>(static_cast<float>(fontMetrics.height()) * 1.2f);
	int const endY = static_cast<int>(static_cast<float>(fontMetrics.height()) * 2.5f);
	painter.drawLine(QPoint(m_mouseMovePosition.x(), startY), QPoint(m_mouseMovePosition.x(), endY));

	int const startX = startY;
	int const endX = endY;
	painter.drawLine(QPoint(startX, m_mouseMovePosition.y()), QPoint(endX, m_mouseMovePosition.y()));
}

//-----------------------------------------------------------------------------

QRect RegionRenderer::getWorldCellScreenPosition(float const worldX, float const worldZ) const
{
	float const worldCellSizeX = static_cast<float>(m_worldSizeX) / static_cast<float>(m_cellCountX);
	float const worldCellSizeY = static_cast<float>(m_worldSizeZ) / static_cast<float>(m_cellCountY);
	float const worldX1 = floor(worldX / worldCellSizeX) * worldCellSizeX;
	float const worldY1 = ceil(worldZ / worldCellSizeY) * worldCellSizeY;
	float const worldX2 = worldX1 + worldCellSizeX;
	float const worldY2 = worldY1 - worldCellSizeY;

	float leftPixel;
	float topPixel;
	float rightPixel;
	float bottomPixel;

	worldToPixel(worldX1, worldY1, leftPixel, topPixel);
	worldToPixel(worldX2, worldY2, rightPixel, bottomPixel);

	QPoint cellPixelTopLeft(QPoint(static_cast<int>(leftPixel), static_cast<int>(topPixel)));
	QPoint cellPixelBottomRight(QPoint(static_cast<int>(rightPixel), static_cast<int>(bottomPixel)));
	int width = cellPixelBottomRight.x() - cellPixelTopLeft.x();
	int height = cellPixelBottomRight.y() - cellPixelTopLeft.y();

	return QRect(cellPixelTopLeft.x(), cellPixelTopLeft.y(), width, height);
}

//-----------------------------------------------------------------------------

int RegionRenderer::getGameServerCellCount(uint32 const gameServerId) const
{
	int result = 0;
	Uint32ToIntMap::const_iterator iterGameServerCellCounts = m_gameServerCellCounts.find(gameServerId);

	if (iterGameServerCellCounts != m_gameServerCellCounts.end())
	{
		result = iterGameServerCellCounts->second;
	}

	return result;
}

//-----------------------------------------------------------------------------

int RegionRenderer::getGameServerObjectCount(uint32 const gameServerId) const
{
	int result = 0;
	Uint32ToIntMap::const_iterator iterGameServerObjectCounts = m_gameServerObjectCounts.find(gameServerId);

	if (iterGameServerObjectCounts != m_gameServerObjectCounts.end())
	{
		result = iterGameServerObjectCounts->second;
	}

	return result;
}

//-----------------------------------------------------------------------------

QPoint RegionRenderer::getSelectedCellPosition() const
{
	return m_selectedCellPosition;
}

//-----------------------------------------------------------------------------

void RegionRenderer::getCellWorldPosition(QPoint const &cellPosition, QPoint &topLeft, QPoint &bottomRight) const
{
	UNREF(cellPosition);
	UNREF(topLeft);
	UNREF(bottomRight);
}

//-----------------------------------------------------------------------------

void RegionRenderer::getCellGameServerIds(QPoint const &cellPosition, Uint32Vector &gameServerIds) const
{
	UNREF(cellPosition);
	UNREF(gameServerIds);
}

//-----------------------------------------------------------------------------

void RegionRenderer::getCellObjects(QPoint const &cellPosition, NetworkIdVector &objectIds) const
{
	UNREF(cellPosition);
	UNREF(objectIds);
}

//-----------------------------------------------------------------------------

void RegionRenderer::clearCellGameServersIds(int const cellX, int const cellY)
{
	uint32 gameServerIds = m_worldCells[getCellIndex(cellX, cellY)].m_gameServerIds;

	if (gameServerIds > 0)
	{
		for (int i = 0; i < 32; ++i)
		{
			if (gameServerIds & (1 << i))
			{
				uint32 gameServerId = m_gameServerIds[i];

				Uint32ToIntMap::iterator iterGameServerCellCounts = m_gameServerCellCounts.find(gameServerId);

				if (iterGameServerCellCounts != m_gameServerCellCounts.end())
				{
					DEBUG_FATAL((iterGameServerCellCounts->second <= 0), ("The game server cell count should be at least 1."));

					--(iterGameServerCellCounts->second);

					// If there are no more cells owned by this game server, then remove it from the count list

					int count = iterGameServerCellCounts->second;

					if (count <= 0)
					{
						m_gameServerCellCounts.erase(gameServerId);
					}
				}

				// Clear out this game server from this cell

				m_worldCells[getCellIndex(cellX, cellY)].m_gameServerIds &= ~(1 << i);
			}
		}
	}
}

//-----------------------------------------------------------------------------

unsigned int RegionRenderer::getCellIndex(int const cellX, int const cellY) const
{
	return static_cast<unsigned int>(cellX + cellY * m_cellCountX);
}

//-----------------------------------------------------------------------------

void RegionRenderer::wheelEvent(QWheelEvent *wheelEvent)
{
	float const wheelDelta = static_cast<float>(wheelEvent->delta()) / 120.0f;
	float zoomRate = m_zoomRate;
	
	if (m_zoom < 0.75)
	{
		zoomRate /= 4.0f;
	}
	else if (m_zoom > 2.0f)
	{
		zoomRate *= 4.0f;
	}
	
	float const zoom = getZoom() + zoomRate * wheelDelta;

	setZoom(zoom);
}

//-----------------------------------------------------------------------------

QColor const &RegionRenderer::getGameServerColor(uint32 const gameServerId) const
{
	static QColor defaultColor("black");
	QColor &result = defaultColor;
	Uint32ToColorMap::const_iterator iterServerColors = m_serverColors.find(gameServerId);

	if (iterServerColors != m_serverColors.end())
	{
		result = iterServerColors->second;
	}
	else
	{
		DEBUG_FATAL(true, ("Unable to find game server color"));
	}

	return result;
}

//-----------------------------------------------------------------------------

void RegionRenderer::setGameServerColor(uint32 const gameServerId, QColor const &color)
{
	Uint32ToColorMap::iterator iterServerColors = m_serverColors.find(gameServerId);

	if (iterServerColors != m_serverColors.end())
	{
		iterServerColors->second = color;
	}
	else
	{
		m_serverColors.insert(std::make_pair(gameServerId, color));
	}
}

//-----------------------------------------------------------------------------

int RegionRenderer::getCellX(float const worldX) const
{
	int result = 0;

	if (m_cellCountX > 0)
	{
		result = static_cast<int>(floor(worldX / getCellSizeX()));
	}

	return result;
}

//-----------------------------------------------------------------------------

int RegionRenderer::getCellY(float const worldZ) const
{
	int result = 0;

	if (m_cellCountY > 0)
	{
		result = static_cast<int>(floor((m_worldSizeZ - worldZ) / getCellSizeZ()));
	}

	return result;
}

//-----------------------------------------------------------------------------

float RegionRenderer::getCellSizeX() const
{
	float result = 0.0f;
	
	if (m_cellCountX > 0)
	{
		result = static_cast<float>(m_worldSizeX) / static_cast<float>(m_cellCountX);
	}

	return result;
}

//-----------------------------------------------------------------------------

float RegionRenderer::getCellSizeZ() const
{
	float result = 0.0f;
	
	if (m_cellCountY > 0)
	{
		result = static_cast<float>(m_worldSizeZ) / static_cast<float>(m_cellCountY);
	}

	return result;
}

//-----------------------------------------------------------------------------

float RegionRenderer::getWorldSizeX() const
{
	return m_worldSizeX;
}

//-----------------------------------------------------------------------------

float RegionRenderer::getWorldSizeZ() const
{
	return m_worldSizeZ;
}

//-----------------------------------------------------------------------------

int RegionRenderer::getTotalGameServerCellCount() const
{
	int result = 0;

	// Decrement the cell count for the game servers at the cell

	for (unsigned int i = 0; i < m_gameServerIds.size(); ++i)
	{
		uint32 gameServerId = m_gameServerIds[i];

		result += getGameServerCellCount(gameServerId);
	}

	return result;
}

//-----------------------------------------------------------------------------

void RegionRenderer::removeGameServerObjects(uint32 const gameServerId)
{
	NetworkIdVector removeList;
	NetworkIdToServerObjectMap::iterator iterObjects = m_objects.begin();

	for (; iterObjects != m_objects.end(); ++iterObjects)
	{
		RegionRenderer::ServerObject const &serverObject = iterObjects->second;

		if (serverObject.m_gameServerId == gameServerId)
		{
			removeList.push_back(iterObjects->first);
		}
	}

	NetworkIdVector::const_iterator iterRemoveList = removeList.begin();

	for (; iterRemoveList != removeList.end(); ++iterRemoveList)
	{
		m_objects.erase(*iterRemoveList);
	}

	update();
}

//-----------------------------------------------------------------------------

void RegionRenderer::removeGameServerCells(uint32 const gameServerId)
{
	UNREF(gameServerId);

//m_worldCells

	update();
}

//-----------------------------------------------------------------------------

void RegionRenderer::filterOnPvP(bool filter)
{
	m_filterOnPvp = filter;
	repaint();
}

//-----------------------------------------------------------------------------

void RegionRenderer::filterOnBuildable(bool filter)
{
	m_filterOnBuildable = filter;
	repaint();
}

//-----------------------------------------------------------------------------

void RegionRenderer::filterOnSpawnable(bool filter)
{
	m_filterOnSpawnable = filter;
	repaint();
}

//-----------------------------------------------------------------------------

void RegionRenderer::filterOnGeographical(bool filter)
{
	m_filterOnGeographical = filter;
	repaint();
}

//-----------------------------------------------------------------------------

void RegionRenderer::filterOnMission(bool filter)
{
	m_filterOnMission = filter;
	repaint();
}

//-----------------------------------------------------------------------------

void RegionRenderer::filterOnDifficulty(bool filter)
{
	m_filterOnDifficulty = filter;
	repaint();
}

//-----------------------------------------------------------------------------

void RegionRenderer::filterOnMunicipal(bool filter)
{
	m_filterOnMunicipal = filter;
	repaint();
}

// ============================================================================
