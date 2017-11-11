// ============================================================================
//
// PlanetWatcherRenderer.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstPlanetWatcher.h"
#include "PlanetWatcherRenderer.h"
#include "PlanetWatcherRenderer.moc"
#include "PlanetWatcher.h"
#include "PlanetWatcherUtility.h"
#include "serverGame/ServerCreatureObjectTemplate.h"
#include "sharedRandom/Random.h"
#include "sharedMath/Vector2d.h"
#include <bitset>
#include <map>
#include <vector>

// ============================================================================
//
// PlanetWatcherRenderer::MiniMapObject
//
// ============================================================================

namespace PlanetWatcherRendererNamespace
{
	bool ms_colorGrid = true;

	//Qt doesn't define an orange, so make one
	QColor cms_orange(255, 165, 0);

	QColor lerpQColor (QColor & start, QColor & end, float f)
	{
		int startR = 0;
		int startG = 0;
		int startB = 0;
		int endR = 0;
		int endG = 0;
		int endB = 0;
		start.getRgb(&startR, &startG, &startB);
		end.getRgb(&endR, &endG, &endB);

		const float diff [3] = 
		{
			static_cast<float>(endR) - static_cast<float>(startR),
			static_cast<float>(endG) - static_cast<float>(startG),
			static_cast<float>(endB) - static_cast<float>(startB),
		};

		int const r = static_cast<unsigned char>(std::max (0.0f, std::min (255.0f, static_cast<float>(startR) + diff [0] * f)));
		int const g = static_cast<unsigned char>(std::max (0.0f, std::min (255.0f, static_cast<float>(startG) + diff [1] * f)));
		int const b = static_cast<unsigned char>(std::max (0.0f, std::min (255.0f, static_cast<float>(startB) + diff [2] * f)));
		return QColor(r, g, b);
	}
}

using namespace PlanetWatcherRendererNamespace;

// ============================================================================

PlanetWatcher const *PlanetWatcherRenderer::MiniMapObject::ms_planetWatcher = NULL;

//-----------------------------------------------------------------------------
PlanetWatcherRenderer::MiniMapObject::MiniMapObject(NetworkId const &objectId, int const gameServerId, float const x, float const z, QString const &label, const int objectTypeTag, int const level, bool const hibernating, uint32 const templateCrc, int const aiActivity, int const creationType)
 : m_objectId(objectId)
 , m_gameServerId(gameServerId)
 , m_interestRadius(0)
 , m_worldX()
 , m_worldZ()
 , m_label(label)
 , m_objectTypeTag(objectTypeTag)
 , m_isCreatureObject(false)
 , m_level(level)
 , m_hibernating(hibernating)
 , m_templateCrc(0)
 , m_templateCrcHexString()
 , m_aiActivity(aiActivity)
 , m_creationType(creationType)
{
	m_worldX.push_back(x);
	m_worldZ.push_back(z);

	m_worldX.push_back(x + 0.0f);
	m_worldZ.push_back(z - 0.001f);

	m_isCreatureObject = (m_objectTypeTag == ServerCreatureObjectTemplate::ServerCreatureObjectTemplate_tag);

	setTemplateCrc(templateCrc);
}

//-----------------------------------------------------------------------------
void PlanetWatcherRenderer::MiniMapObject::setPlanetWatcher(PlanetWatcher const *planetWatcher)
{
	ms_planetWatcher = planetWatcher;
}

//-----------------------------------------------------------------------------
bool PlanetWatcherRenderer::MiniMapObject::hasMoved() const
{
	return (m_worldX.size() > 2) || (m_worldZ.size() > 2);
}

//-----------------------------------------------------------------------------
void PlanetWatcherRenderer::MiniMapObject::move(float const x, float const z)
{
	// Only update if something changed

	if ((m_worldX.back() != x) && (m_worldZ.back() != z))
	{
		m_worldX.push_back(x);
		m_worldZ.push_back(z);
	}
}

//-----------------------------------------------------------------------------
float PlanetWatcherRenderer::MiniMapObject::getWorldX() const
{
	return m_worldX.back();
}

//-----------------------------------------------------------------------------
float PlanetWatcherRenderer::MiniMapObject::getWorldZ() const
{
	return m_worldZ.back();
}

//-----------------------------------------------------------------------------
uint32 PlanetWatcherRenderer::MiniMapObject::getGameServerId() const
{
	return m_gameServerId;
}

//-----------------------------------------------------------------------------
float PlanetWatcherRenderer::MiniMapObject::getDirectionX() const
{
	float result = 0.0f;

	if (m_worldX.size() <= 2)
	{
		result = m_worldX.front();
	}
	else
	{
		result = m_worldX.back() - m_worldX[m_worldX.size() - 2];
	}

	return result;
}

//-----------------------------------------------------------------------------
float PlanetWatcherRenderer::MiniMapObject::getDirectionZ() const
{
	float result = 0.0f;

	if (m_worldZ.size() <= 2)
	{
		result = m_worldZ.front();
	}
	else
	{
		result = -(m_worldZ.back() - m_worldZ[m_worldZ.size() - 2]);
	}

	return result;
}

//-----------------------------------------------------------------------------
float PlanetWatcherRenderer::MiniMapObject::getDrawRadius() const
{
	float result = 4.0f;

	if (m_interestRadius > 0)
	{
		result *= 4.0f;
	}
	else if (hasMoved())
	{
		result *= 3.0f;
	}

	return result;
}

//-----------------------------------------------------------------------------
QColor PlanetWatcherRenderer::MiniMapObject::getDrawColor() const
{
	QColor result;

	//level gradient owns all
	if(ms_planetWatcher->getUseLevelGradient())
	{
		if (m_level < -1)
		{
			char buffer[512];
			snprintf(buffer, sizeof(buffer)-1, "Level %d is < -1. FATALING!", m_level);
			buffer[sizeof(buffer)-1] = '\0';

			QMessageBox::warning(const_cast<PlanetWatcher*>(ms_planetWatcher), "Invalid Level", buffer, "OK");

			FATAL(true, ("Level cannot be < -1!"));
		}

		QColor startColor = Qt::black;
		QColor endColor   = Qt::black;
		float f = 0;

		if(m_level <= 10)
		{
			startColor = Qt::gray;
			endColor   = Qt::green;

			if (m_level < 0)
				f = 0.0f/10.0f;
			else
				f = m_level/10.0f;
		}
		else if (m_level <= 20)
		{
			startColor = Qt::green;
			endColor   = Qt::cyan;
			f = (m_level - 10.0f)/10.0f;
		}
		else if (m_level <= 30)
		{
			startColor = Qt::cyan;
			endColor   = Qt::blue;
			f = (m_level - 20.0f)/10.0f;
		}
		else if (m_level <= 40)
		{
			startColor = Qt::blue;
			endColor   = Qt::white;
			f = (m_level - 30.0f)/10.0f;
		}
		else if (m_level <= 50)
		{
			startColor = Qt::white;
			endColor   = Qt::yellow;
			f = (m_level - 40.0f)/10.0f;
		}
		else if (m_level <= 60)
		{
			startColor = Qt::yellow;
			endColor   = cms_orange;
			f = (m_level - 50.0f)/10.0f;
		}
		else if (m_level <= 70)
		{
			startColor = cms_orange;
			endColor   = Qt::red;
			f = (m_level - 60.0f)/10.0f;
		}
		else if (m_level <= 80)
		{
			startColor = Qt::red;
			endColor   = Qt::magenta;
			f = (m_level - 70.0f)/10.0f;
		}
		else //for 80 and above always magenta
		{
			startColor = Qt::magenta;
			endColor   = Qt::magenta;
			f = 1.0f;
		}

		result = lerpQColor(startColor, endColor, f);
	}
	else
	{
		if (ms_planetWatcher->getSelectedGameServerId() == m_gameServerId)
		{
			result = PlanetWatcher::ms_selectedObjects;
		}
		else
		{
			if (m_interestRadius > 0)
			{
				result = PlanetWatcher::ms_mobileInterestRadiusObjects;
			}
			else if (hasMoved())
			{
				result = PlanetWatcher::ms_mobileObjects;
			}
			else if (m_isCreatureObject)
			{
				result = PlanetWatcher::ms_neverMovedCreatureObjects;
			}
			else
			{
				result = PlanetWatcher::ms_neverMovedNonCreatureObjects;
			}
		}
	}
	return result;
}

//-----------------------------------------------------------------------------
int PlanetWatcherRenderer::MiniMapObject::getMinDiameter() const
{
	int result = 3;

	if (hasMoved())
	{
		result = 5;
	}

	return result;
}

//-----------------------------------------------------------------------

int PlanetWatcherRenderer::MiniMapObject::getObjectTypeTag() const
{
	return m_objectTypeTag;
}

//-----------------------------------------------------------------------

int PlanetWatcherRenderer::MiniMapObject::getLevel() const
{
	return m_level;
}

//-----------------------------------------------------------------------

bool PlanetWatcherRenderer::MiniMapObject::getHibernating() const
{
	return m_hibernating;
}

//-----------------------------------------------------------------------

uint32 PlanetWatcherRenderer::MiniMapObject::getTemplateCrc() const
{
	return m_templateCrc;
}

//-----------------------------------------------------------------------

std::string const & PlanetWatcherRenderer::MiniMapObject::getTemplateCrcHexString() const
{
	return m_templateCrcHexString;
}

//-----------------------------------------------------------------------

void PlanetWatcherRenderer::MiniMapObject::setTemplateCrc(uint32 templateCrc)
{
	if (m_templateCrc != templateCrc)
	{
		char buffer[32];
		snprintf(buffer, sizeof(buffer)-1, "%x", templateCrc);
		buffer[sizeof(buffer)-1] = '\0';

		m_templateCrcHexString = buffer;
	}

	m_templateCrc = templateCrc;
}

//-----------------------------------------------------------------------

int PlanetWatcherRenderer::MiniMapObject::getAiActivity() const
{
	return m_aiActivity;
}

//-----------------------------------------------------------------------

std::string const & PlanetWatcherRenderer::MiniMapObject::getAiActivityString() const
{
	return ms_planetWatcher->getAiActivityString(m_aiActivity);
}

//-----------------------------------------------------------------------

int PlanetWatcherRenderer::MiniMapObject::getCreationType() const
{
	return m_creationType;
}

// ============================================================================
//
// PlanetWatcherRenderer::WorldCell
//
// ============================================================================

//-----------------------------------------------------------------------------
class PlanetWatcherRenderer::WorldCell
{
public:

	WorldCell();

	std::bitset<64> m_gameServerIds;
};

//-----------------------------------------------------------------------------
PlanetWatcherRenderer::WorldCell::WorldCell()
 : m_gameServerIds()
{
	m_gameServerIds.reset();
}

// ============================================================================
//
// PlanetWatcherRenderer
//
// ============================================================================

float const PlanetWatcherRenderer::ms_minZoom = 0.035f;
float const PlanetWatcherRenderer::ms_maxZoom = 16.0f;

//-----------------------------------------------------------------------------
PlanetWatcherRenderer::PlanetWatcherRenderer(QWidget *parent, char const *name)
 : QFrame(parent, name)
 , m_objects(NULL)
 , m_gameServerObjectCounts(NULL)
 , m_serverColors(NULL)
 , m_worldCells(NULL)
 , m_gameServerIds(NULL)
 , m_screen()
 , m_objectMiniMap()
 , m_gameServerMiniMap()
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
 , m_mousePressPositionLocal(0, 0)
 , m_mousePressZoom(1.0f)
 , m_mousePressCameraPositionX(0.0f)
 , m_mousePressCameraPositionY(0.0f)
 , m_selectedCellPosition(0, 0)
 , m_font("Arial", 10, QFont::Bold)
 , m_gameServerConnected(false)
 , m_selectedObject(NetworkId::cms_invalid)
 , m_trackSelected(false)
 , m_labelDynamics(true)
 , m_labelStatics(true)
 , m_planetWatcher(NULL)
{
	m_objects = new NetworkIdToServerObjectMap;
	m_gameServerObjectCounts = new Uint32ToIntMap;
	m_serverColors = new Uint32ToColorMap;
	m_worldCells = new WorldCellVector;
	m_gameServerIds = new Uint32Vector;

	setMouseTracking(true);
	setBackgroundMode(Qt::NoBackground);

	m_timer = new QTimer(this, "PlanetWatcherRendererTimer");
	m_timer->start(1000 / 24);

	connect(m_timer, SIGNAL(timeout()), this, SLOT(slotTimerTimeOut()));

	// Default initialization

	setWorld(16000.0f, 16000.0f, 160, 160);
}

//-----------------------------------------------------------------------------
PlanetWatcherRenderer::~PlanetWatcherRenderer()
{
	delete m_objects;
	m_objects = NULL;

	delete m_gameServerObjectCounts;
	m_gameServerObjectCounts = NULL;

	delete m_serverColors;
	m_serverColors = NULL;

	delete m_worldCells;
	m_worldCells = NULL;

	delete m_gameServerIds;
	m_gameServerIds = NULL;
}

//-----------------------------------------------------------------------------
void PlanetWatcherRenderer::slotTimerTimeOut()
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

	if (m_trackSelected)
	{
		NetworkIdToServerObjectMap::iterator iterObjects = m_objects->find(m_selectedObject);

		if (iterObjects != m_objects->end())
		{
			MiniMapObject const &miniMapObject = iterObjects->second;

			m_destinationCameraPositionX = (m_worldSizeX / 2.0f + miniMapObject.getWorldX()) / m_worldSizeX;
			m_destinationCameraPositionY = (m_worldSizeZ / 2.0f - miniMapObject.getWorldZ()) / m_worldSizeZ;
		}
	}

	//if (m_leftButtonDown)
	//{
	//	zoomIn();
	//}
	//else if (m_rightButtonDown)
	//{
	//	zoomOut();
	//}
}

//-----------------------------------------------------------------------------
void PlanetWatcherRenderer::updateObject(NetworkId const &objectId, uint32 const gameServerId, float const worldX, float const worldZ, int const interestRadius, int objectTypeTag, int const level, bool const hibernating, uint32 const templateCrc, int const aiActivity, int const creationType)
{
	NetworkIdToServerObjectMap::iterator iterObjects = m_objects->find(objectId);

	if (iterObjects != m_objects->end())
	{
		// Remove the previous object position from the minimap

		QPainter painter;
		painter.begin(&m_objectMiniMap);
		painter.setPen(QColor(128, 128, 128));
		int const previousCellX = getCellCountX() / 2 + static_cast<int>(floor(iterObjects->second.getWorldX() / getCellSizeX()));
		int const previousCellY = getCellCountX() / 2 + static_cast<int>(floor(iterObjects->second.getWorldZ() / getCellSizeX()));
		painter.drawPoint(previousCellX, previousCellY);
		painter.end();

		// Object already exists, update its information

		iterObjects->second.m_gameServerId = gameServerId;
		iterObjects->second.move(worldX, worldZ);
		iterObjects->second.m_interestRadius = interestRadius;
		iterObjects->second.m_level = level;
		iterObjects->second.m_hibernating = hibernating;
		iterObjects->second.setTemplateCrc(templateCrc);
		iterObjects->second.m_aiActivity   = aiActivity;
		iterObjects->second.m_creationType = creationType;
	}
	else
	{
		// New object, add it to the list

		m_objects->insert(std::make_pair(objectId, MiniMapObject(objectId, gameServerId, worldX, worldZ, "Name", objectTypeTag, level, hibernating, templateCrc, aiActivity, creationType)));

		// Set the game server object count

		Uint32ToIntMap::iterator iterGameServerObjectCounts = m_gameServerObjectCounts->find(gameServerId);

		if (iterGameServerObjectCounts != m_gameServerObjectCounts->end())
		{
			++(iterGameServerObjectCounts->second);
		}
		else
		{
			// Create this game server's counter and add one

			m_gameServerObjectCounts->insert(std::make_pair(gameServerId, 1));
		}
	}

	// Update the new object position to the minimap

	QPainter painter;
	painter.begin(&m_objectMiniMap);
	painter.setPen(QColor("white"));
	int const cellX = getCellCountX() / 2 + static_cast<int>(floor(worldX / getCellSizeX()));
	int const cellY = getCellCountZ() / 2 - static_cast<int>(floor(worldZ / getCellSizeZ()));
	painter.drawPoint(cellX, cellY);
	painter.end();
}

//-----------------------------------------------------------------------------
void PlanetWatcherRenderer::removeObject(NetworkId const &objectId)
{
	NetworkIdToServerObjectMap::iterator iterNetworkIdToServerObjectMap = m_objects->find(objectId);

	if (iterNetworkIdToServerObjectMap != m_objects->end())
	{
		// Decrease one item from the associated game server

		Uint32ToIntMap::iterator iterGameServerObjectCounts = m_gameServerObjectCounts->find(iterNetworkIdToServerObjectMap->second.m_gameServerId);

		if (iterGameServerObjectCounts != m_gameServerObjectCounts->end())
		{
			--(iterGameServerObjectCounts->second);
		}

		// Remove this object

		m_objects->erase(iterNetworkIdToServerObjectMap);
	}
}

//-----------------------------------------------------------------------------
void PlanetWatcherRenderer::setWorld(float const sizeX, float const sizeY, int const cellCountX, int const cellCountY)
{
	if ((cellCountX > 0) && (cellCountY > 0))
	{
		// Reset all the data

		m_objects->clear();
		m_worldCells->clear();
		m_serverColors->clear();
		m_leftButtonDown = false;
		m_rightButtonDown = false;
		m_zoom = ms_minZoom;
		m_cameraPositionX = 0.5f;
		m_cameraPositionY = 0.5f;
		m_destinationCameraPositionX = m_cameraPositionX;
		m_destinationCameraPositionY = m_cameraPositionY;
		m_mousePressPositionLocal = QPoint(0, 0);
		m_gameServerIds->clear();
		m_gameServerObjectCounts->clear();
		m_selectedCellPosition = QPoint(0, 0);

		// Set the world dimensions

		m_worldSizeX = sizeX;
		m_worldSizeZ = sizeY;

		// Set the cell count

		m_cellCountX = cellCountX;
		m_cellCountY = cellCountY;

		m_worldCells->reserve(m_cellCountX * m_cellCountY);

		for (unsigned int i = 0; i < m_worldCells->capacity(); ++i)
		{
			m_worldCells->push_back(WorldCell());
		}

		m_gameServerMiniMap.resize(cellCountX, cellCountY);
		m_objectMiniMap.resize(cellCountX, cellCountY);

		QPainter painter;
		painter.begin(&m_gameServerMiniMap);
		painter.fillRect(painter.window(), QBrush(QColor(128, 128, 128)));
		painter.end();

		painter.begin(&m_objectMiniMap);
		painter.fillRect(painter.window(), QBrush(QColor(128, 128, 128)));
		painter.end();
	}
}

//-----------------------------------------------------------------------------
void PlanetWatcherRenderer::resizeEvent(QResizeEvent *)
{
	if ((m_screen.width() != contentsRect().width()) ||
		(m_screen.height() != contentsRect().height()))
	{
		m_screen.resize(contentsRect().width(), contentsRect().height());
	}
}

//-----------------------------------------------------------------------------
void PlanetWatcherRenderer::paintEvent(QPaintEvent *)
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
		drawGrid(painter, backGroundColor);
		drawCursor(painter);
		drawObjects(painter);
		drawMiniMap(painter, m_gameServerMiniMap, painter.window().size().width() - m_gameServerMiniMap.width() - 2, 0, "Game Servers");
		drawMiniMap(painter, m_objectMiniMap, painter.window().size().width() - m_objectMiniMap.width() * 2 - 3, 0, "Objects");

		// Draw the connection status

		QFontMetrics fontMetrics(painter.font());

		if (!m_gameServerConnected)
		{
			QString text("No planet server connected");
			int const x = m_screen.width() / 2 - fontMetrics.width(text) / 2;
			int const y = m_screen.height() / 2;

			painter.setPen(QColor("black"));
			painter.drawText(x + 1, y + 1, text);
			painter.setPen(QColor("white"));
			painter.drawText(x, y, text);
		}
		else if (getTotalGameServerCellCount() <= 0)
		{
			QString text("Connection established but no cells owned by a game server");
			int const x = m_screen.width() / 2 - fontMetrics.width(text) / 2;
			int const y = m_screen.height() / 2;

			painter.setPen(QColor("black"));
			painter.drawText(x + 1, y + 1, text);
			painter.setPen(QColor("white"));
			painter.drawText(x, y, text);
		}

		bitBlt(this, contentsRect().left(), contentsRect().top(), &m_screen);

		painter.end();
	}
}

//-----------------------------------------------------------------------------
void PlanetWatcherRenderer::drawObjects(QPainter &painter)
{
	// Do we draw all objects?
	const bool drawAllObjects = m_planetWatcher->isDrawAllObjects();

	// Draw all the objects

	float worldX1;
	float worldZ1;
	float worldX2;
	float worldZ2;

	pixelToWorld(QPoint(0, 0), worldX1, worldZ1);
	pixelToWorld(QPoint(contentsRect().width(), contentsRect().height()), worldX2, worldZ2);

	static NetworkIdVector interestRadiusObjectList;
	interestRadiusObjectList.clear();

	// Draw the non-interest radius objects first

	NetworkIdToServerObjectMap::const_iterator iterObjects = m_objects->begin();

	for (; iterObjects != m_objects->end(); ++iterObjects)
	{
		MiniMapObject const &miniMapObject = iterObjects->second;

		if (!drawAllObjects && !m_planetWatcher->isDrawObjectType(miniMapObject.m_objectTypeTag))
			continue;

		if (!m_planetWatcher->passesFilters(miniMapObject))
			continue;

		if ((miniMapObject.getWorldX() >= worldX1) &&
			(miniMapObject.getWorldX() <= worldX2) &&
			(miniMapObject.getWorldZ() <= worldZ1) &&
			(miniMapObject.getWorldZ() >= worldZ2))
		{
			if ((miniMapObject.m_interestRadius > 0) || miniMapObject.hasMoved())
			{
				interestRadiusObjectList.push_back(iterObjects->first);
			}
			else
			{
				if (!miniMapObject.hasMoved() &&
				    (miniMapObject.m_objectId != m_selectedObject) &&
				    (m_zoom < 0.1f))
				{
					float pixelX;
					float pixelY;

					worldToPixel(miniMapObject.getDirectionX(), miniMapObject.getDirectionZ(), pixelX, pixelY);

					QPoint const pixel(static_cast<int>(pixelX), static_cast<int>(pixelY));

					painter.setPen(miniMapObject.getDrawColor());
					painter.drawPoint(pixelX, pixelY);
				}
				else
				{
					bool const outline = (m_zoom > 4.0f);

					fillCircle(painter, miniMapObject, outline);

					if (m_labelStatics)
					{
					}
				}
			}
		}
	}

	// Draw the interest radius object second

	NetworkIdVector::const_iterator iterInterestRadiusObjectList = interestRadiusObjectList.begin();

	for (; iterInterestRadiusObjectList != interestRadiusObjectList.end(); ++iterInterestRadiusObjectList)
	{
		iterObjects = m_objects->find(*iterInterestRadiusObjectList);

		if (iterObjects != m_objects->end())
		{
			MiniMapObject const &miniMapObject = iterObjects->second;

			if ((drawAllObjects || m_planetWatcher->isDrawObjectType(miniMapObject.m_objectTypeTag)) && (m_planetWatcher->passesFilters(miniMapObject)))
			{
				fillTriangle(painter, miniMapObject, true);

				if (m_labelDynamics)
				{
				}
			}
		}
	}

	// Draw the highlighted object

	iterObjects = m_objects->find(m_selectedObject);

	if (iterObjects != m_objects->end())
	{
		MiniMapObject const &miniMapObject = iterObjects->second;

		if ((drawAllObjects || m_planetWatcher->isDrawObjectType(miniMapObject.m_objectTypeTag)) && (m_planetWatcher->passesFilters(miniMapObject)))
			drawRotatingSquare(painter, miniMapObject);
	}
}

//-----------------------------------------------------------------------------
void PlanetWatcherRenderer::mousePressEvent(QMouseEvent *mouseEvent)
{
	setFocus();

	m_mousePressPositionLocal = mouseEvent->globalPos();
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
void PlanetWatcherRenderer::mouseReleaseEvent(QMouseEvent *mouseEvent)
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
void PlanetWatcherRenderer::setZoom(float const zoom)
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
float PlanetWatcherRenderer::getZoom() const
{
	return m_zoom;
}

//-----------------------------------------------------------------------------
void PlanetWatcherRenderer::zoomIn()
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
void PlanetWatcherRenderer::zoomOut()
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
void PlanetWatcherRenderer::mouseDoubleClickEvent(QMouseEvent *mouseEvent)
{
	switch (mouseEvent->button())
	{
		case Qt::RightButton:
			{
				moveCameraToMouseScreenPosition(mouseEvent->pos());
			}
			break;
		case Qt::LeftButton:
			{
				// Save the cell that was clicked

				float const pixelsPerCellX = static_cast<float>(m_worldSizeX) / static_cast<float>(m_cellCountX);
				float const pixelsPerCellY = static_cast<float>(m_worldSizeZ) / static_cast<float>(m_cellCountY);

				int const cellX = static_cast<int>(floor((m_destinationCameraPositionX * m_worldSizeX) / pixelsPerCellX));
				int const cellY = static_cast<int>(ceil((m_destinationCameraPositionY * m_worldSizeZ) / pixelsPerCellY));

				m_selectedCellPosition = QPoint(cellX, cellY);

				emit signalCellSelected();

				// Save the selected object

				NetworkIdToServerObjectMap::const_iterator iterObjects = m_objects->find(m_highlightedObject);

				if ((iterObjects != m_objects->end()) && getObjectPixelRect(iterObjects->second).contains(m_mouseMovePositionLocal) && (m_planetWatcher->isDrawAllObjects() || m_planetWatcher->isDrawObjectType(iterObjects->second.m_objectTypeTag)) && m_planetWatcher->passesFilters(iterObjects->second))
				{
					m_selectedObject = iterObjects->first;
					m_trackSelected = true;

					QString text;
					text.sprintf("Selected %s.", PlanetWatcherUtility::dumpObjectInfo(iterObjects->second).c_str());
					PlanetWatcherUtility::report(text);
				}
				else
				{
					m_selectedObject = NetworkId::cms_invalid;

					moveCameraToMouseScreenPosition(mouseEvent->pos());
				}
			}
			break;
		default:
			{
			}
			break;
	}
}

//-----------------------------------------------------------------------------
void PlanetWatcherRenderer::moveCameraToMouseScreenPosition(QPoint const &position)
{
	// Get the change in movement

	QPoint centerOfScreen(contentsRect().size().width() / 2, contentsRect().size().height() / 2);
	QPoint const mouseDelta(centerOfScreen - position);

	float cameraPositionDeltaX = static_cast<float>(mouseDelta.x()) / (m_worldSizeX * m_zoom);
	float cameraPositionDeltaY = static_cast<float>(mouseDelta.y()) / (m_worldSizeZ * m_zoom);

	m_destinationCameraPositionX = m_mousePressCameraPositionX - cameraPositionDeltaX;
	m_destinationCameraPositionY = m_mousePressCameraPositionY - cameraPositionDeltaY;
}

//-----------------------------------------------------------------------------
void PlanetWatcherRenderer::mouseMoveEvent(QMouseEvent *mouseEvent)
{
	mouseEvent->ignore();

	// Save the mouse position

	m_mouseMovePositionLocal = mouseEvent->pos();

	// Get the change in movement

	QPoint const mouseDelta(mouseEvent->globalPos() - m_mousePressPositionLocal);

	if (m_rightButtonDown)
	{
		// Pan the view

		float cameraPositionDeltaX = static_cast<float>(mouseDelta.x()) / (m_worldSizeX * m_zoom);
		float cameraPositionDeltaY = static_cast<float>(mouseDelta.y()) / (m_worldSizeZ * m_zoom);

		m_cameraPositionX = m_mousePressCameraPositionX - cameraPositionDeltaX;
		m_cameraPositionY = m_mousePressCameraPositionY - cameraPositionDeltaY;
		m_destinationCameraPositionX = m_cameraPositionX;
		m_destinationCameraPositionY = m_cameraPositionY;

		m_trackSelected = false;
	}
	else if (m_leftButtonDown)
	{
	}

	// Find the highlighted object

	NetworkIdToServerObjectMap::const_iterator iterObjects = m_objects->begin();

	for (; iterObjects != m_objects->end(); ++iterObjects)
	{
		QRect objectRect(getObjectPixelRect(iterObjects->second));

		if (objectRect.contains(m_mouseMovePositionLocal) && (m_planetWatcher->isDrawAllObjects() || m_planetWatcher->isDrawObjectType(iterObjects->second.m_objectTypeTag)) && (m_planetWatcher->passesFilters(iterObjects->second)))
		{
			m_highlightedObject = iterObjects->first;
			break;
		}
	}

	// Needed for cursor redraw when the mouse moves

	update();
}

//-----------------------------------------------------------------------------
void PlanetWatcherRenderer::drawCircle(QPainter &painter, float const worldX, float const worldZ, float const radius, QColor const &color)
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
void PlanetWatcherRenderer::fillCircle(QPainter &painter, PlanetWatcherRenderer::MiniMapObject const &miniMapObject, bool const outline) const
{
	float pixelX;
	float pixelY;

	worldToPixel(miniMapObject.getDirectionX(), miniMapObject.getDirectionZ(), pixelX, pixelY);

	QPoint const pixel(static_cast<int>(pixelX), static_cast<int>(pixelY));
	int const radius = getObjectPixelRadius(miniMapObject);
	
	painter.setBrush(QBrush(miniMapObject.getDrawColor()));

	if (miniMapObject.m_objectId == m_selectedObject)
	{
		painter.setPen("pink");
		painter.setBrush(QBrush("pink"));
	}
	else
	{
		if (outline)
		{
			painter.setPen("black");
		}
		else
		{
			painter.setPen(miniMapObject.getDrawColor());
		}
		painter.setBrush(QBrush(miniMapObject.getDrawColor()));
	}

	painter.drawEllipse(pixel.x() - (radius / 2), pixel.y() - (radius / 2), radius, radius);
}

//-----------------------------------------------------------------------------
void PlanetWatcherRenderer::fillTriangle(QPainter &painter, PlanetWatcherRenderer::MiniMapObject const &miniMapObject, bool const outline) const
{
	float pixelX;
	float pixelY;

	Vector2d forwardVector(0.0f, -1.0f);

	if (miniMapObject.hasMoved())
	{
		forwardVector = Vector2d(miniMapObject.getDirectionX(), miniMapObject.getDirectionZ());
		forwardVector.normalize();
	}

	Vector2d sideVector(forwardVector / 2.0f);
	sideVector.rotate(PI / 2.0f);

	worldToPixel(miniMapObject.getWorldX(), miniMapObject.getWorldZ(), pixelX, pixelY);

	QPoint point(static_cast<int>(pixelX), static_cast<int>(pixelY));
	int const radius = getObjectPixelRadius(miniMapObject);
	int const x1 = point.x() + static_cast<int>(forwardVector.x * static_cast<float>(radius));
	int const y1 = point.y() + static_cast<int>(forwardVector.y * static_cast<float>(radius));
	int const x2 = point.x() + static_cast<int>(sideVector.x * static_cast<float>(radius) + forwardVector.x * static_cast<float>(-radius));
	int const y2 = point.y() + static_cast<int>(sideVector.y * static_cast<float>(radius) + forwardVector.y * static_cast<float>(-radius));
	int const x3 = point.x() + static_cast<int>(sideVector.x * static_cast<float>(-radius) + forwardVector.x * static_cast<float>(-radius));
	int const y3 = point.y() + static_cast<int>(sideVector.y * static_cast<float>(-radius) + forwardVector.y * static_cast<float>(-radius));
	
	QPointArray pointArray(3);
	pointArray.setPoint(0, x1, y1);
	pointArray.setPoint(1, x3, y3);
	pointArray.setPoint(2, x2, y2);

	if (miniMapObject.m_objectId == m_selectedObject)
	{
		painter.setPen("pink");
		painter.setBrush(QBrush("pink"));
	}
	else
	{
		if ((m_zoom > 2.0f) && outline)
		{
			painter.setPen("black");
		}
		else
		{
			painter.setPen(miniMapObject.getDrawColor());
		}
		painter.setBrush(QBrush(miniMapObject.getDrawColor()));
	}

	painter.drawPolygon(pointArray);
}

//-----------------------------------------------------------------------------
void PlanetWatcherRenderer::drawRotatingSquare(QPainter &painter, PlanetWatcherRenderer::MiniMapObject const &miniMapObject) const
{
	static float radian = 0;
	static int lastMillisecond = 0;
	static int accumulatedMillisecond = 0;

	accumulatedMillisecond += QTime::currentTime().msec();

	int deltaTime = accumulatedMillisecond - lastMillisecond;
	radian = static_cast<float>(deltaTime) / 1000.0f * PI * 2.0f;
	lastMillisecond = accumulatedMillisecond;

	float pixelX;
	float pixelY;

	Vector2d forwardVector(0.0f, -1.0f);
	forwardVector.rotate(radian);

	Vector2d sideVector(forwardVector);
	sideVector.rotate(PI / 2.0f);

	worldToPixel(miniMapObject.getWorldX(), miniMapObject.getWorldZ(), pixelX, pixelY);

	QPoint point(static_cast<int>(pixelX), static_cast<int>(pixelY));
	int const radius = getObjectPixelRadius(miniMapObject) * 2;
	int const x1 = point.x() + static_cast<int>(forwardVector.x * static_cast<float>(+radius));
	int const y1 = point.y() + static_cast<int>(forwardVector.y * static_cast<float>(+radius));
	int const x2 = point.x() + static_cast<int>(sideVector.x * static_cast<float>(+radius));
	int const y2 = point.y() + static_cast<int>(sideVector.y * static_cast<float>(+radius));
	int const x3 = point.x() + static_cast<int>(forwardVector.x * static_cast<float>(-radius));
	int const y3 = point.y() + static_cast<int>(forwardVector.y * static_cast<float>(-radius));
	int const x4 = point.x() + static_cast<int>(sideVector.x * static_cast<float>(-radius));
	int const y4 = point.y() + static_cast<int>(sideVector.y * static_cast<float>(-radius));
	
	QPointArray pointArray(4);
	pointArray.setPoint(0, x1, y1);
	pointArray.setPoint(1, x2, y2);
	pointArray.setPoint(2, x3, y3);
	pointArray.setPoint(3, x4, y4);
	painter.setBrush(QBrush(QColor("pink"), Qt::NoBrush));
	painter.setPen(QColor("pink"));
	painter.drawPolygon(pointArray);
}

//-----------------------------------------------------------------------------
void PlanetWatcherRenderer::worldToPixel(float const worldX, float const worldY, float &pixelX, float &pixelY) const
{
	float const cameraCenterPixelX = ((m_worldSizeX / 2.0f) - (m_worldSizeX * m_cameraPositionX)) * m_zoom;
	float const cameraCenterPixelY = ((m_worldSizeZ / 2.0f) - (m_worldSizeZ * m_cameraPositionY)) * m_zoom;

	pixelX = static_cast<float>((contentsRect().width() / 2) + cameraCenterPixelX) + (worldX * m_zoom);
	pixelY = static_cast<float>((contentsRect().height() / 2) + cameraCenterPixelY) - (worldY * m_zoom);
}

//-----------------------------------------------------------------------------
void PlanetWatcherRenderer::pixelToWorld(QPoint const &pixel, float &worldX, float &worldZ) const
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
int PlanetWatcherRenderer::getCellServerCount(int const cellX, int const cellY) const
{
	NOT_NULL(m_worldCells);

	int count = 0;

	if (validCell(cellX, cellY))
	{
		unsigned int const cellIndex = getCellIndex(cellX, cellY);
		unsigned int const gameServerIdsSize = (*m_worldCells)[cellIndex].m_gameServerIds.size();

		for (unsigned int i = 0; i < gameServerIdsSize; ++i)
		{
			if ((*m_worldCells)[cellIndex].m_gameServerIds.test(i))
			{
				++count;
			}
		}
	}

	return count;
}

//-----------------------------------------------------------------------------
void PlanetWatcherRenderer::getColors(int const cellX, int const cellY, ColorVector &colors) const
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
QColor PlanetWatcherRenderer::getColor(int const cellX, int const cellY, int const index) const
{
	NOT_NULL(m_gameServerIds);
	NOT_NULL(m_worldCells);

	QColor result(128, 128, 128);

	// ROYGBIV

	if (!validCell(cellX, cellY))
	{
		return result;
	}

	unsigned int const cellIndex = getCellIndex(cellX, cellY);

	if ((*m_worldCells)[cellIndex].m_gameServerIds.none())
	{
		// This cell is not assigned a game server
	
		result = result;
	}
	else
	{
		unsigned int colorIndex = 0;
		int foundColorCount = 0;

		for (unsigned int i = 0; i < (*m_worldCells)[cellIndex].m_gameServerIds.size(); ++i)
		{
			if ((*m_worldCells)[cellIndex].m_gameServerIds.test(i))
			{
				if (foundColorCount == index)
				{
					colorIndex = i;
					break;
				}
	
				++foundColorCount;
			}
		}
	
		unsigned int const gameServerIdsCount = m_gameServerIds->size();
		UNREF(gameServerIdsCount);
		DEBUG_FATAL((colorIndex >= gameServerIdsCount), ("out of bounds"));
	
		uint32 const gameServerId = (*m_gameServerIds)[colorIndex];

		result = getGameServerColor(gameServerId);
	}

	return result;
}

//-----------------------------------------------------------------------------
void PlanetWatcherRenderer::drawGrid(QPainter &painter, QColor const &backGroundColor)
{
	float const gridLabelZoom = 0.2f;
	float const gridZoom = 0.2f;

	// Clear the background

	//white background if no server coloring
	if(ms_colorGrid)
		painter.fillRect(m_screen.rect(), QBrush(backGroundColor));
	else
		painter.fillRect(m_screen.rect(), QBrush(QColor(128, 128, 128)));

	// Draw the grid lines

	float const worldCellSizeX = static_cast<float>(m_worldSizeX) / static_cast<float>(m_cellCountX);
	float const worldCellSizeZ = static_cast<float>(m_worldSizeZ) / static_cast<float>(m_cellCountY);

	int startCellX;
	int startCellY;
	int endCellX;
	int endCellY;
	
	getScreenCells(startCellX, startCellY, endCellX, endCellY);
	
	static ColorVector colors;
	colors.reserve(16);

	if(ms_colorGrid)
	{
		for (int x = startCellX; x < endCellX; ++x)
		{
			for (int y = startCellY; y < endCellY; ++y)
			{
				// Get the cell
		
				float const worldX = -m_worldSizeX / 2.0f + static_cast<float>(x) * worldCellSizeX;
				float const worldZ = m_worldSizeZ / 2.0f + -static_cast<float>(y) * worldCellSizeZ + worldCellSizeZ;
		
				QRect cellScreenRect(getWorldCellScreenPosition(worldX, worldZ));
		
				// Get the color
		
				unsigned int const colorCount = getCellServerCount(x, y);
		
				if (colorCount > 1)
				{
					//if (m_zoom > gridZoom)
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
					//else
					//{
					//	// White cell for shared cells
					//
					//	QBrush brush(QColor("white"));
					//	painter.fillRect(cellScreenRect, brush);
					//}
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
	}

	if (m_zoom > gridZoom)
	{
		int const fontSize = (m_zoom < 0.3f) ? 8 : 12;
		m_font.setPointSize(fontSize);
		painter.setFont(m_font);
		QFontMetrics fontMetrics(m_font);

		// X Axis
		int x;
		for (x = startCellX; x <= endCellX; ++x)
		{
			float const worldX = -m_worldSizeX / 2.0f + worldCellSizeX * static_cast<float>(x);
   			
   			float leftPixel;
   			float topPixel;
   			float rightPixel;
   			float bottomPixel;
   
			worldToPixel(worldX, m_worldSizeZ / 2.0f + worldCellSizeZ, leftPixel, topPixel);
			worldToPixel(worldX, -m_worldSizeZ / 2.0f + worldCellSizeZ, rightPixel, bottomPixel);

			QPoint a(static_cast<int>(leftPixel), static_cast<int>(topPixel));
			QPoint b(static_cast<int>(rightPixel), static_cast<int>(bottomPixel));
			
			painter.setPen(QColor("black"));
			painter.drawLine(a, b);
		}
		
		// Y Axis
		int y;
		for (y = startCellY; y <= endCellY; ++y)
		{
			float const worldY = m_worldSizeZ / 2.0f - worldCellSizeZ * static_cast<float>(y) + worldCellSizeZ;
   			
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
				painter.drawText(textX, textY, QString::number(-m_cellCountX / 2 + x));
				painter.setPen("white");
				painter.drawText(textX - 1, textY - 1, QString::number(-m_cellCountX / 2 + x));
			}
		}

		// Y grid label
		
		for (y = startCellY; y <= endCellY; ++y)
		{
			float const worldY = m_worldSizeZ / 2.0f - worldCellSizeZ * static_cast<float>(y);
   			
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
				painter.drawText(textX, textY, QString::number(m_cellCountY / 2 - y));
				painter.setPen("white");
				painter.drawText(textX - 1, textY - 1, QString::number(m_cellCountY / 2 - y));
			}
		}
	}
}

//-----------------------------------------------------------------------------
void PlanetWatcherRenderer::drawMiniMap(QPainter &painter, QPixmap const &pixMap, int const pixelX, int const pixelY, QString const &label)
{
	// Draw the black trim

	painter.setPen(QColor("black"));
	painter.drawRect(QRect(pixelX, pixelY, pixMap.width() + 2, pixMap.height() + 2));

	// Draw the mini map

	painter.drawPixmap(QPoint(pixelX + 1, pixelY + 1), pixMap);

	// Draw the viewable area of the map

	QPoint a(0, 0);
	QPoint b(painter.window().width(), painter.window().height());
	float worldX1;
	float worldZ1;
	float worldX2;
	float worldZ2;

	pixelToWorld(a, worldX1, worldZ1);
	pixelToWorld(b, worldX2, worldZ2);

	float const worldCellSizeX = m_worldSizeX / static_cast<float>(m_cellCountX);
	float const worldCellSizeZ = m_worldSizeZ / static_cast<float>(m_cellCountY);
	
	int x1 = static_cast<int>(((m_worldSizeX / 2.0f) + worldX1) / worldCellSizeX);
	int z1 = static_cast<int>(((m_worldSizeZ / 2.0f) - worldZ1) / worldCellSizeZ);
	int x2 = static_cast<int>(((m_worldSizeX / 2.0f) + worldX2) / worldCellSizeX);
	int z2 = static_cast<int>(((m_worldSizeZ / 2.0f) - worldZ2) / worldCellSizeZ);

	// Clamp

	if (x1 < 0) { x1 = 0; }
	if (x1 > m_cellCountX) { x1 = m_cellCountX; }
	
	if (z1 < 0) { z1 = 0; }
	if (z1 > m_cellCountY) { z1 = m_cellCountY; }
	
	if (x2 < 0) { x2 = 0; }
	if (x2 > m_cellCountX) { x2 = m_cellCountX; }
	
	if (z2 < 0) { z2 = 0; }
	if (z2 > m_cellCountY) { z2 = m_cellCountY; }

	x1 += pixelX + 1;
	z1 += 1;
	x2 += pixelX + 1;
	z2 += 1;

	int width = x2 - x1;
	int height = z2 - z1;

	if (width < 2) { width = 2; }
	if (height < 2) { height = 2; }

	painter.setBrush(Qt::NoBrush);
	painter.setPen(QColor("white"));
	painter.drawRect(x1, z1, width, height);

	painter.setPen(QColor("black"));
	painter.drawRect(x1 - 1, z1 - 1, width + 2, height + 2);
	painter.drawRect(x1 + 1, z1 + 1, width - 2, height - 2);

	int const fontSize = 10;
	QFont arial("Arial", fontSize, QFont::Bold);
	painter.setFont(arial);
	QFontMetrics fontMetrics(arial);
	int const textX = pixelX;
	int const textY = pixelY + pixMap.height() + fontMetrics.height();
	painter.setPen(QColor("black"));
	painter.drawText(textX, textY, label);
	painter.setPen(QColor("white"));
	painter.drawText(textX - 1, textY - 1, label);

	// Draw some crosshairs to show the viewable area

	painter.setPen(QColor("white"));

	// West

	painter.drawLine(pixelX, z1 + height / 2, x1, z1 + height / 2);

	// East

	painter.drawLine(x1 + width, z1 + height / 2, pixelX + pixMap.width(), z1 + height / 2);

	// North

	painter.drawLine(x1 + width / 2, pixelY, x1 + width / 2, z1);

	// South

	painter.drawLine(x1 + width / 2, z1 + height, x1 + width / 2, pixelY + pixMap.height());
}

//-----------------------------------------------------------------------------
void PlanetWatcherRenderer::setCellGameServerId(int const cellX, int const cellY, uint32 const gameServerId)
{
	NOT_NULL(m_worldCells);

	if (validCell(cellX, cellY))
	{
		// See if this server id is already stored

		Uint32Vector::const_iterator iterGameServerIds = m_gameServerIds->begin();

		bool serverIdAlreadyUsed = false;
		uint32 serverIdIndex = 0;

		for (; iterGameServerIds != m_gameServerIds->end(); ++iterGameServerIds)
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
			m_gameServerIds->push_back(gameServerId);
		}

		FATAL((m_gameServerIds->size() > 32), ("PlanetWatcherRenderer::setCellServerId() - Too many unique server ids (> 32), rethink the implementation."));

		// Set the bit corresponding to the server id list

		unsigned int const cellIndex = getCellIndex(cellX, cellY);

		(*m_worldCells)[cellIndex].m_gameServerIds.set(serverIdIndex);

		// Update the minimap with the new color

		QPainter painter;
		painter.begin(&m_gameServerMiniMap);
		QColor color;

		if (getCellServerCount(cellX, cellY) > 1)
		{
			color = QColor("white");
		}
		else
		{
			color = getColor(cellX, cellY, 0);
		}

		painter.setPen(color);
		painter.drawPoint(cellX, cellY);
		painter.end();
	}
}

//-----------------------------------------------------------------------------
void PlanetWatcherRenderer::setCellSubscription(int const cellX, int const cellY, std::vector<uint32> const &gameServers, std::vector<int> const &subscriptionCounts)
{
	// Clear the current servers associated with this cell

	clearCellGameServersIds(cellX, cellY);

	UNREF(subscriptionCounts);

	// Add all the servers associated with this cell

	std::vector<uint32>::const_iterator iterGameServers = gameServers.begin();

	for (; iterGameServers != gameServers.end(); ++iterGameServers)
	{
		uint32 const gameServerId = (*iterGameServers);

		setCellGameServerId(cellX, cellY, gameServerId);
	}
}
//-----------------------------------------------------------------------------
void PlanetWatcherRenderer::setColorGrid(bool const colorGrid)
{
	ms_colorGrid = colorGrid;
}

//-----------------------------------------------------------------------------
void PlanetWatcherRenderer::getScreenCells(int &startCellX, int &startCellY, int &endCellX, int &endCellY) const
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
int PlanetWatcherRenderer::getCellCountX() const
{
	return m_cellCountX;
}

//-----------------------------------------------------------------------------
int PlanetWatcherRenderer::getCellCountZ() const
{
	return m_cellCountY;
}

//-----------------------------------------------------------------------------
void PlanetWatcherRenderer::drawCursor(QPainter &painter)
{
	// Draw the outline of the current square the mouse is over
	
	float worldX;
	float worldZ;

	pixelToWorld(m_mouseMovePositionLocal, worldX, worldZ);
	
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
	painter.drawLine(QPoint(m_mouseMovePositionLocal.x(), startY), QPoint(m_mouseMovePositionLocal.x(), endY));

	int const startX = startY;
	int const endX = endY;
	painter.drawLine(QPoint(startX, m_mouseMovePositionLocal.y()), QPoint(endX, m_mouseMovePositionLocal.y()));
}

//-----------------------------------------------------------------------------
QRect PlanetWatcherRenderer::getWorldCellScreenPosition(float const worldX, float const worldZ) const
{
	float const worldCellSizeX = static_cast<float>(m_worldSizeX) / static_cast<float>(m_cellCountX);
	float const worldCellSizeZ = static_cast<float>(m_worldSizeZ) / static_cast<float>(m_cellCountY);
	float const worldX1 = floor(worldX / worldCellSizeX) * worldCellSizeX;
	float const worldY1 = ceil(worldZ / worldCellSizeZ) * worldCellSizeZ;
	float const worldX2 = worldX1 + worldCellSizeX;
	float const worldY2 = worldY1 - worldCellSizeZ;

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
int PlanetWatcherRenderer::getGameServerCellCount(uint32 const gameServerId) const
{
	UNREF(m_worldCells);

	int result = 0;
	int gameServerBitIndex = getGameServerBitIndex(gameServerId);

	if (gameServerBitIndex >= 0)
	{
		for (unsigned int i = 0; i < m_worldCells->size(); ++i)
		{
			if ((*m_worldCells)[i].m_gameServerIds.test(gameServerBitIndex))
			{
				++result;
			}
		}
	}

	return result;
}

//-----------------------------------------------------------------------------
int PlanetWatcherRenderer::getGameServerObjectCount(uint32 const gameServerId) const
{
	int result = 0;
	Uint32ToIntMap::const_iterator iterGameServerObjectCounts = m_gameServerObjectCounts->find(gameServerId);

	if (iterGameServerObjectCounts != m_gameServerObjectCounts->end())
	{
		result = iterGameServerObjectCounts->second;
	}

	return result;
}

//-----------------------------------------------------------------------------
QPoint PlanetWatcherRenderer::getSelectedCellPosition() const
{
	return m_selectedCellPosition;
}

//-----------------------------------------------------------------------------
void PlanetWatcherRenderer::getCellWorldPosition(QPoint const &cellPosition, QPoint &topLeft, QPoint &bottomRight) const
{
	UNREF(cellPosition);
	UNREF(topLeft);
	UNREF(bottomRight);
}

//-----------------------------------------------------------------------------
void PlanetWatcherRenderer::getCellGameServerIds(QPoint const &cellPosition, Uint32Vector &gameServerIds) const
{
	UNREF(cellPosition);
	UNREF(gameServerIds);
}

//-----------------------------------------------------------------------------
void PlanetWatcherRenderer::getCellObjects(QPoint const &cellPosition, NetworkIdVector &objectIds) const
{
	UNREF(cellPosition);
	UNREF(objectIds);
}

//-----------------------------------------------------------------------------
void PlanetWatcherRenderer::clearCellGameServersIds(int const cellX, int const cellY)
{
	NOT_NULL(m_worldCells);

	// Make sure the cells are in range

	if (!validCell(cellX, cellY))
	{
		return;
	}

	// Clear out this game server from this cell

	unsigned int const cellIndex = getCellIndex(cellX, cellY);

	(*m_worldCells)[cellIndex].m_gameServerIds.reset();

	// Clear the cell from the game server minimap

	QPainter painter;
	painter.begin(&m_gameServerMiniMap);
	QColor color(128, 128, 128);
	painter.setPen(color);
	painter.drawPoint(cellX, cellY);
	painter.end();
}

//-----------------------------------------------------------------------------
unsigned int PlanetWatcherRenderer::getCellIndex(int const cellX, int const cellY) const
{
	unsigned int result = 0;

	if (validCell(cellX, cellY))
	{
		result = static_cast<unsigned int>(cellX + cellY * m_cellCountX);
	}
	else
	{
		DEBUG_FATAL(true, ("PlanetWatcherRenderer::getCellIndex() - Invalid cell index"));
	}

	return result;
}

//-----------------------------------------------------------------------------
void PlanetWatcherRenderer::wheelEvent(QWheelEvent *wheelEvent)
{
	wheelEvent->accept();

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
QColor const &PlanetWatcherRenderer::getGameServerColor(uint32 const gameServerId) const
{
	NOT_NULL(m_serverColors);
	
	static QColor defaultColor("black");
	QColor &result = defaultColor;
	Uint32ToColorMap::const_iterator iterServerColors = m_serverColors->find(gameServerId);

	if (iterServerColors != m_serverColors->end())
	{
		result = iterServerColors->second;
	}
	else
	{
		// Dump the current game server colors

		Uint32ToColorMap::const_iterator iterServerColors = m_serverColors->begin();

		for (; iterServerColors != m_serverColors->end(); ++iterServerColors)
		{
			DEBUG_WARNING(true, ("PlanetWatcherRenderer::getGameServerColor() - game server ids %d\n", iterServerColors->first));
		}

		DEBUG_FATAL(true, ("Invalid game server id"));
	}

	return result;
}

//-----------------------------------------------------------------------------
void PlanetWatcherRenderer::setGameServerColor(uint32 const gameServerId, QColor const &color)
{
	Uint32ToColorMap::iterator iterServerColors = m_serverColors->find(gameServerId);

	if (iterServerColors != m_serverColors->end())
	{
		iterServerColors->second = color;
	}
	else
	{
		m_serverColors->insert(std::make_pair(gameServerId, color));
	}
}

//-----------------------------------------------------------------------------
int PlanetWatcherRenderer::getCellX(float const worldX) const
{
	int result = 0;

	if (m_cellCountX > 0)
	{
		result = static_cast<int>(floor(worldX / getCellSizeX()));
	}

	return result;
}

//-----------------------------------------------------------------------------
int PlanetWatcherRenderer::getCellY(float const worldZ) const
{
	int result = 0;

	if (m_cellCountY > 0)
	{
		result = static_cast<int>(floor((m_worldSizeZ - worldZ) / getCellSizeZ()));
	}

	return result;
}

//-----------------------------------------------------------------------------
float PlanetWatcherRenderer::getCellSizeX() const
{
	float result = 0.0f;
	
	if (m_cellCountX > 0)
	{
		result = static_cast<float>(m_worldSizeX) / static_cast<float>(m_cellCountX);
	}

	return result;
}

//-----------------------------------------------------------------------------
float PlanetWatcherRenderer::getCellSizeZ() const
{
	float result = 0.0f;
	
	if (m_cellCountY > 0)
	{
		result = static_cast<float>(m_worldSizeZ) / static_cast<float>(m_cellCountY);
	}

	return result;
}

//-----------------------------------------------------------------------------
float PlanetWatcherRenderer::getWorldSizeX() const
{
	return m_worldSizeX;
}

//-----------------------------------------------------------------------------
float PlanetWatcherRenderer::getWorldSizeZ() const
{
	return m_worldSizeZ;
}

//-----------------------------------------------------------------------------
int PlanetWatcherRenderer::getTotalGameServerCellCount() const
{
	NOT_NULL(m_gameServerIds);

	int result = 0;

	// Decrement the cell count for the game servers at the cell

	for (unsigned int i = 0; i < m_gameServerIds->size(); ++i)
	{
		unsigned int const gameServerIdsCount = m_gameServerIds->size();
		UNREF(gameServerIdsCount);
		DEBUG_FATAL((i >= gameServerIdsCount), ("out of bounds"));

		uint32 gameServerId = (*m_gameServerIds)[i];

		result += getGameServerCellCount(gameServerId);
	}

	return result;
}

//-----------------------------------------------------------------------------
void PlanetWatcherRenderer::removeGameServerObjects(uint32 const gameServerId)
{
	NetworkIdVector removeList;
	NetworkIdToServerObjectMap::iterator iterObjects = m_objects->begin();

	for (; iterObjects != m_objects->end(); ++iterObjects)
	{
		PlanetWatcherRenderer::MiniMapObject const &miniMapObject = iterObjects->second;

		if (miniMapObject.m_gameServerId == gameServerId)
		{
			removeList.push_back(iterObjects->first);
		}
	}

	NetworkIdVector::const_iterator iterRemoveList = removeList.begin();

	for (; iterRemoveList != removeList.end(); ++iterRemoveList)
	{
		m_objects->erase(*iterRemoveList);
	}
}

//-----------------------------------------------------------------------------
void PlanetWatcherRenderer::removeGameServerCells(uint32 const gameServerId)
{
	UNREF(gameServerId);

}

//-----------------------------------------------------------------------------
void PlanetWatcherRenderer::setGameServerConnected(bool const connected)
{
	m_gameServerConnected = connected;
}

//-----------------------------------------------------------------------------
QRect PlanetWatcherRenderer::getObjectPixelRect(MiniMapObject const &miniMapObject) const
{
	float pixelX;
	float pixelY;

	worldToPixel(miniMapObject.getWorldX(), miniMapObject.getWorldZ(), pixelX, pixelY);

	int const pixelRadius = getObjectPixelRadius(miniMapObject);

	QPoint topLeft(pixelX - pixelRadius, pixelY - pixelRadius);
	QPoint bottomRight(pixelX + pixelRadius, pixelY + pixelRadius);

	return QRect(topLeft, bottomRight);
}

//-----------------------------------------------------------------------------
int PlanetWatcherRenderer::getObjectPixelRadius(MiniMapObject const &miniMapObject) const
{
	int result = static_cast<int>(miniMapObject.getDrawRadius() * 0.5f * m_zoom);

	result = (result < miniMapObject.getMinDiameter()) ? miniMapObject.getMinDiameter() : result;

	return result;
}

//-----------------------------------------------------------------------------
int PlanetWatcherRenderer::getGameServerBitIndex(uint32 const gameServerId) const
{
	int result = -1;
	bool found = false;
	uint32 serverIdIndex = 0;
	Uint32Vector::const_iterator iterGameServerIds = m_gameServerIds->begin();

	for (; iterGameServerIds != m_gameServerIds->end(); ++iterGameServerIds)
	{
		uint32 id = (*iterGameServerIds);

		if (id == gameServerId)
		{
			found = true;
			break;
		}

		++serverIdIndex;
	}

	if (found)
	{	
		result = serverIdIndex;
	}

	return result;
}

//-----------------------------------------------------------------------------
void PlanetWatcherRenderer::setPlanetWatcher(PlanetWatcher const *planetWatcher)
{
	m_planetWatcher = planetWatcher;

	PlanetWatcherRenderer::MiniMapObject::setPlanetWatcher(planetWatcher);
}

//-----------------------------------------------------------------------------
void PlanetWatcherRenderer::getObjectCount(std::map<int, int> & objectTypeTotalCountMap, int & total, std::map<int, int> & objectTypeSelectedCountMap, int & selected, int & moved, int & interestRadius, int & immobileCreature, int & immobileNonCreature, int & selectedMoved, int & selectedInterestRadius, int & selectedImmobileCreature, int & selectedImmobileNonCreature) const
{
	objectTypeTotalCountMap.clear();
	total = 0;
	objectTypeSelectedCountMap.clear();
	selected = 0;
	moved = 0;
	interestRadius = 0;
	immobileCreature = 0;
	immobileNonCreature = 0;
	selectedMoved = 0;
	selectedInterestRadius = 0;
	selectedImmobileCreature = 0;
	selectedImmobileNonCreature = 0;

	std::map<int, int>::iterator iterFind;
	uint32 const selectedGameServerId = m_planetWatcher->getSelectedGameServerId();

	for (NetworkIdToServerObjectMap::const_iterator iterObjects = m_objects->begin(); iterObjects != m_objects->end(); ++iterObjects)
	{
		iterFind = objectTypeTotalCountMap.find(iterObjects->second.m_objectTypeTag);
		if (iterFind != objectTypeTotalCountMap.end())
			(iterFind->second)++;
		else
			objectTypeTotalCountMap[iterObjects->second.m_objectTypeTag] = 1;

		++total;

		if (iterObjects->second.m_gameServerId == selectedGameServerId)
		{
			iterFind = objectTypeSelectedCountMap.find(iterObjects->second.m_objectTypeTag);
			if (iterFind != objectTypeSelectedCountMap.end())
				(iterFind->second)++;
			else
				objectTypeSelectedCountMap[iterObjects->second.m_objectTypeTag] = 1;

			++selected;

			if (iterObjects->second.m_interestRadius > 0)
			{
				++interestRadius;
				++selectedInterestRadius;
			}

			if (iterObjects->second.hasMoved())
			{
				++moved;
				++selectedMoved;
			}
			else if (iterObjects->second.m_isCreatureObject)
			{
				++immobileCreature;
				++selectedImmobileCreature;
			}
			else
			{
				++immobileNonCreature;
				++selectedImmobileNonCreature;
			}
		}
		else
		{
			if (iterObjects->second.m_interestRadius > 0)
				++interestRadius;

			if (iterObjects->second.hasMoved())
				++moved;
			else if (iterObjects->second.m_isCreatureObject)
				++immobileCreature;
			else
				++immobileNonCreature;
		}
	}
}

//-----------------------------------------------------------------------------
QPoint const &PlanetWatcherRenderer::getMousePosition() const
{
	return m_mouseMovePositionLocal;
}

//-----------------------------------------------------------------------------
bool PlanetWatcherRenderer::validCell(int const cellX, int const cellY) const
{
	bool result = false;

	if ((cellX >= 0) &&
	    (cellX < m_cellCountX) &&
	    (cellY >= 0) &&
	    (cellY < m_cellCountY))
	{
		result = true;
	}

	return result;
}

//-----------------------------------------------------------------------------
PlanetWatcherRenderer::MiniMapObject * PlanetWatcherRenderer::locateObject(NetworkId const &objectId)
{
	NetworkIdToServerObjectMap::iterator iterObjects = m_objects->find(objectId);
	
	if (iterObjects != m_objects->end())
	{
		m_selectedObject = objectId;
		m_trackSelected = true;
		return &(iterObjects->second);
	}

	return NULL;
}

//-----------------------------------------------------------------------------
void PlanetWatcherRenderer::dumpObjectInfo() const
{
	// Do we dump all objects info?
	const bool drawAllObjects = m_planetWatcher->isDrawAllObjects();

	QString text;
	int count = 0;

	NetworkIdToServerObjectMap::const_iterator iterObjects = m_objects->begin();
	for (; iterObjects != m_objects->end(); ++iterObjects)
	{
		MiniMapObject const &miniMapObject = iterObjects->second;

		if (!drawAllObjects && !m_planetWatcher->isDrawObjectType(miniMapObject.m_objectTypeTag))
			continue;

		if (!m_planetWatcher->passesFilters(miniMapObject))
			continue;

		text.sprintf("%s.", PlanetWatcherUtility::dumpObjectInfo(iterObjects->second).c_str());
		PlanetWatcherUtility::report(text);
		++count;
	}

	text.sprintf("%d objects", count);
	PlanetWatcherUtility::report(text);
}

//-----------------------------------------------------------------------------
std::vector<uint32> const & PlanetWatcherRenderer::getGameServerIds() const
{
	return *m_gameServerIds;
}


// ============================================================================
