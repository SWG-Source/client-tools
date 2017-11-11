// ======================================================================
//
// GodClientData.cpp
// copyright(c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgGodClient/FirstSwgGodClient.h"
#include "GodClientData.h"

#include "sharedCollision/BoxExtent.h"
#include "sharedCollision/CollisionInfo.h"
#include "sharedCollision/CollideParameters.h"
#include "sharedCollision/Extent.h"
#include "sharedCollision/Floor.h"
#include "sharedCollision/FloorLocator.h"
#include "sharedGame/SharedBuildingObjectTemplate.h"
#include "sharedGame/SharedBuildoutAreaManager.h"
#include "sharedMath/Ray3d.h"
#include "sharedNetworkMessages/ConsoleChannelMessages.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/World.h"
#include "sharedTerrain/TerrainObject.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/PortalProperty.h"
#include "sharedObject/ContainedByProperty.h"

#include "clientGame/ClientWorld.h"
#include "clientGame/FreeCamera.h"
#include "clientGame/Game.h"
#include "clientGame/ClientWorld.h"
#include "clientGame/GroundScene.h"
#include "clientGraphics/Camera.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/RenderWorld.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientParticle/ParticleEffectAppearance.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientUserInterface/CuiManager.h"

#include "ActionHack.h"
#include "ActionsBuildoutArea.h"
#include "BrushData.h"
#include "BuildoutAreaSupport.h"
#include "ConfigGodClient.h"
#include "GodClientIoWin.h"
#include "MainFrame.h"
#include "ObjectTransformWindow.h"
#include "RegionBrowser.h"
#include "RegionRenderer.h"
#include "ServerCommander.h"
#include "ServerObjectData.h"
#include "SnapToGridSettings.h"
#include "UnicodeUtils.h"

#include <qapplication.h>
#include <qcheckbox.h>
#include <qinputdialog.h>
#include <qmessagebox.h>
#include <qspinbox.h>

#include <utility>
#include <algorithm>

// ======================================================================

#ifdef MESG
#undef MESG
#endif

#define MESG(msg) const char* const GodClientData::Messages::##msg = "GodClientData::" #msg;

MESG(SELECTION_CHANGED);        //lint !e19 useless declaration
MESG(SELECTED_OBJECTS_CHANGED); //lint !e19 useless declaration
MESG(CLIPBOARD_CHANGED);        //lint !e19 useless declaration
MESG(GHOSTS_CREATED);           //lint !e19 useless declaration
MESG(GHOSTS_KILLED);            //lint !e19 useless declaration
MESG(PALETTES_CHANGED);         //lint !e19 useless declaration

#undef MESG

                //                 world chunk  tile  floor portal cell
static float pathnodeRadii[6] = { 2.0f, 0.25f, 0.0f, 0.0f, 0.05f, 0.25f };

//-----------------------------------------------------------------

struct SelectedObjectDestroyer
{
	void operator()(GodClientData::SelectedObject* val) const
	{
		delete val;
	}
};

//-----------------------------------------------------------------

namespace
{
	typedef MessageDispatch::Message<std::pair<NetworkId, int> > ObjectMessage;
}

//-----------------------------------------------------------------

void drawLineSegment(const Vector &origin, float originSize, const Vector &dest, float destSize, const PackedArgb &color)
{
	Vector vertices[8];
	Vector delta  = dest-origin;
	delta.normalize();
	Vector right = Vector::unitY.cross(delta);
	if (!right.normalize())
	{
		right = Vector::unitZ.cross(delta);
	}
	Vector up = right.cross(delta);
	right = up.cross(delta); // this handedness might be wrong but it doesn't matter here
	vertices[0] = origin - right * originSize + up * originSize;
	vertices[1] = origin + right * originSize + up * originSize;
	vertices[2] = origin + right * originSize - up * originSize;
	vertices[3] = origin - right * originSize - up * originSize;

	vertices[4] = dest - right * destSize + up * destSize;
	vertices[5] = dest + right * destSize + up * destSize;
	vertices[6] = dest + right * destSize - up * destSize;
	vertices[7] = dest - right * destSize - up * destSize;

	Graphics::drawFrustum(vertices, color);
}

//-----------------------------------------------------------------

GodClientData::ClipboardObject::ClipboardObject () : 
	serverObjectTemplateName (),
	sharedObjectTemplateName (), 
	transform (),
	networkId () 
{
}

//-----------------------------------------------------------------

GodClientData::ClipboardObject::ClipboardObject (const Object& object) : 
	serverObjectTemplateName (),
	sharedObjectTemplateName (),
	transform (object.getTransform_o2w ()),
	networkId (object.getNetworkId ())
{
	const ClientObject* const clientObject = dynamic_cast<const ClientObject*>(&object);

	if (clientObject)
	{
		//get the server template name(the one we need to send the server to copy and paste)
		ServerObjectData::ObjectInfo const *serverObjectData = ServerObjectData::getInstance().getObjectInfo(networkId, false);
		if (!serverObjectData && object.getNetworkId() < NetworkId::cms_invalid)
		{
			BuildoutAreaSupport::populateServerObjectData(object.getNetworkId());
			serverObjectData = ServerObjectData::getInstance().getObjectInfo(networkId, false);
		}

		if (serverObjectData)
			serverObjectTemplateName = serverObjectData->serverTemplateName;

		sharedObjectTemplateName = clientObject->getObjectTemplateName ();
	}
}

//-----------------------------------------------------------------

Object* GodClientData::ClipboardObject::findInClientWorld() const
{
	return NetworkIdManager::getObjectById(networkId);
}

//-----------------------------------------------------------------

std::string const & GodClientData::ClipboardObject::getObjectTemplateName() const
{
	return Game::getSinglePlayer() ? sharedObjectTemplateName : serverObjectTemplateName;
}

//-----------------------------------------------------------------

GodClientData::SelectedObject::~SelectedObject()
{
	if(ghost)
	{
		ghost->kill();

		//-- Ensure the ghost gets an alter call this upcoming frame so it gets deleted properly.
		ghost->scheduleForAlter();
	}

	obj = 0;
	ghost = 0;
}

//-----------------------------------------------------------------

GodClientData::GodClientData()
: MessageDispatch::Emitter(),
  MessageDispatch::Receiver(),
  Singleton<GodClientData>(),
  m_selectedObjects(),
  m_clipboard(),
  m_palettes(),
  m_persistedSelections(),
  m_sphereTreeObjects(),
  m_triggerObjectSpheres(),
  m_outstandingRequests(),
  m_renderAIPaths(true),
  m_pivot(),
  m_lastPivotCameraPos(),
  m_createdCount(0),
  m_gs(0),
  m_ioWin(0),
  m_cursorPosition(0.0f, 0.0f),
  m_selectedPalette(""),
  m_snapToHorizontalGrid(false),
  m_snapToGridLinesPerHorizontalSide(50),
  m_snapToGridHorizontalSize(2.0),
  m_snapToGridHorizontalLineLength(m_snapToGridLinesPerHorizontalSide* m_snapToGridHorizontalSize),
  m_snapToGridHorizontalThreshold(9),
  m_snapToVerticalGrid(false),
  m_snapToGridLinesPerVerticalSide(50),
  m_snapToGridVerticalSize(2.0),
  m_snapToGridVerticalLineLength(m_snapToGridLinesPerVerticalSide* m_snapToGridVerticalSize),
  m_snapToGridVerticalThreshold(9),
  m_showAxesObjects(),
  m_pivotPoint(),
  m_objectCreationPending(false),
  m_toggleDropToTerrainOn(false),
  m_toggleAlignToTerrainOn(false)
{
	connectToMessage(World::Messages::OBJECT_REMOVED);
	connectToMessage(Game::Messages::SCENE_CHANGED);
	connectToMessage("ConGenericMessage");
	connectToMessage(AINodeInfo::MESSAGE_TYPE);
	connectToMessage(AIPathInfo::MESSAGE_TYPE);
	connectToMessage("GodClientSaveBuildoutArea");
}

//-----------------------------------------------------------------

GodClientData::~GodClientData()
{
	if(m_ioWin)
	{
		delete m_ioWin;
		m_ioWin = 0;
	}

	//don't use clearClipboard(), since it emits messages that we don't want to see the light of day (not from a destructor at least)
	for(ClipboardList_t::iterator it = m_clipboard.begin(); it != m_clipboard.end();)
	{
		ClipboardObject* const clipObj = *it;
		delete clipObj;
		it = m_clipboard.erase(it);
	}

	//don't use clearSelection(), since it emits messages that we don't want to see the light of day (not from a destructor at least)
	std::for_each(m_selectedObjects.begin(), m_selectedObjects.end(), SelectedObjectDestroyer());
	m_selectedObjects.clear();
}

//-----------------------------------------------------------------
void GodClientData::initialize()
{
	handleSceneChange();
}

//-----------------------------------------------------------------

void GodClientData::handleSceneChange()
{
	m_AINodes.clear();
	//get the new scene handle
 	m_gs = dynamic_cast<GroundScene*>(Game::getScene());

	if(m_ioWin)
		m_ioWin->close();
	else
		m_ioWin = new GodClientIoWin;

	m_ioWin->open();

	if (CuiManager::getInstalled ())
		CuiManager::raiseToTop();

	ActionsBuildoutArea::getInstance().actionRefresh->doActivate();

	if (Game::getSinglePlayer() && ConfigGodClient::getLoadServerObjects())
	{
		BuildoutAreaSupport::addServerOnlyObjectsToWorldSnapshot();
	}
}

//-----------------------------------------------------------------

void GodClientData::receiveMessage(const MessageDispatch::Emitter& , const MessageDispatch::MessageBase& message)
{
	if(message.isType(World::Messages::OBJECT_REMOVED))
	{
		const ObjectMessage * const om = dynamic_cast<const ObjectMessage *>(&message);
		
		ClientObject* const obj = om ? findSelectionByNetworkId(om->getValue ().first) : 0;

		if(obj)
			IGNORE_RETURN(removeSelection(obj));
	}
	if(message.isType(Game::Messages::SCENE_CHANGED))
	{
		handleSceneChange();
	}
	if(message.isType("ConGenericMessage"))
	{
		Archive::ReadIterator ri = dynamic_cast<const GameNetworkMessage&>(message).getByteStream().begin();
		ConGenericMessage cg(ri);
		const RequestIdSet_t::iterator it = m_outstandingRequests.find(cg.getMsgId());
		if(it == m_outstandingRequests.end())
			return;

		m_outstandingRequests.erase(it);

		const std::string msg = cg.getMsg();
		std::string line;
		std::vector<std::string> lines;
		size_t startpos = 0;
		size_t endpos;

		while(Unicode::getFirstToken(msg, startpos, endpos, line, "\n"))
		{
			lines.push_back(line);
			startpos = endpos + 1;
		}
		
		unsigned int numSpheres;
		int result = sscanf(lines[0].c_str(), "Sending %d object sphere tree nodes to client\n",&numSpheres);
		if(result > 0)
		{
			//the first line must list the number of spheres following, and the number of additional lines we read should be this big
			DEBUG_FATAL((result != 1) ||(numSpheres != lines.size()-1),("bad data sent in getObjectSpheres"));

			//clear out all the old spheres, build the new list
			m_sphereTreeObjects.clear();

			int sphereId;
			real x;
			real y;
			real z;
			real radius;
			char oid[256];
			for(unsigned int i = 1; i <= numSpheres; ++i)
			{
				result = sscanf(lines[i].c_str(), "Sphere id=[%d] origin=[%f, %f, %f] radius=[%f] OID=[%s]", &sphereId, &x, &y, &z, &radius, oid);
				SphereTreeObject sphereObj;
				sphereObj.sphere.location.set(x,y,z);
				sphereObj.sphere.radius = radius;
				sphereObj.networkId = NetworkId(oid);
				sphereObj.sphereId = sphereId;
				m_sphereTreeObjects.push_back(sphereObj);
			}
		}

		unsigned int numTriggers;
		result = sscanf(lines[0].c_str(), "Listing %d trigger volumes for object",&numTriggers);
		if(result > 0)
		{
			//the first line must list the number of triggers following, and the number of additional lines we read should be this big
			//subtract 2, one for the header line and one for the footer line
			if((result == 1) && (numTriggers == lines.size()-2))
			{
				//clear out all the old spheres, build the new list
				m_triggerObjectSpheres.clear();

				char name[1024];
				real radius;
				char oid[256];;
				for(unsigned int i = 1; i <= numTriggers; ++i)
				{
					result = sscanf(lines[i].c_str(), "trigger volume for object %s %s %f\n", oid, name, &radius);
					TriggerVolumeSphere sphereObj;
					sphereObj.sphere.radius = radius;
					sphereObj.networkId = NetworkId(oid);
					m_triggerObjectSpheres.push_back(sphereObj);
				}
			}
		}
	}
	if(message.isType(AINodeInfo::MESSAGE_TYPE))
	{
		Archive::ReadIterator ri = dynamic_cast<const GameNetworkMessage&>(message).getByteStream().begin();
		AINodeInfo ainiMessage(ri);
		AINode ain;
		ain.m_location = ainiMessage.getLocation();
		ain.m_parent   = ainiMessage.getParent();
		ain.m_children = ainiMessage.getChildren();
		ain.m_siblings = ainiMessage.getSiblings();
		ain.m_type     = ainiMessage.getType();
		ain.m_level    = ainiMessage.getLevel();
		int nodeType = ain.m_type;
		if (nodeType <0) nodeType = 0;
		else if (nodeType > 5) nodeType = 5;

		if (ain.m_type >= 0)
		{
			ain.m_location += Vector(0, pathnodeRadii[nodeType]+0.1f, 0); // elevate the nodes above the ground
			m_AINodes[ainiMessage.getNodeId()] = ain;
		}
		else
		{
			m_AINodes.erase(ainiMessage.getNodeId());
		}
	}
	if(message.isType(AIPathInfo::MESSAGE_TYPE))
	{
		Archive::ReadIterator ri = dynamic_cast<const GameNetworkMessage&>(message).getByteStream().begin();
		AIPathInfo aipiMessage(ri);
		AIPath aip;
		aip.m_nodes  = aipiMessage.getNodes();
		m_AIPaths[aipiMessage.getObjectId()] = aip;
	}
	if(message.isType("GodClientSaveBuildoutArea"))
	{
		Archive::ReadIterator ri = dynamic_cast<const GameNetworkMessage&>(message).getByteStream().begin();

		GenericValueTypeMessage<
			std::pair<
				std::pair<std::string, std::string>, // scene, area
				std::pair<std::vector<ServerBuildoutAreaRow>, std::vector<ClientBuildoutAreaRow> > // serverRows, clientRows
				>
			> const m(ri);
		std::string const &scene = m.getValue().first.first;
		std::string const &area  = m.getValue().first.second;
		std::vector<ServerBuildoutAreaRow> const &serverRows = m.getValue().second.first;
		std::vector<ClientBuildoutAreaRow> const &clientRows = m.getValue().second.second;
		
		if (scene == Game::getSceneId())
			BuildoutAreaSupport::saveBuildoutArea(area, serverRows, clientRows);		
	}
}

//-----------------------------------------------------------------

void GodClientData::draw()
{
	if(!m_gs)
		return;

	GameCamera* currentCam = NON_NULL(m_gs->getCurrentCamera());
	draw(*currentCam);
}

//-----------------------------------------------------------------

void GodClientData::draw(const Camera& camera)
{	
	UNREF(camera);

	Graphics::setStaticShader(ShaderTemplateList::get3dVertexColorZStaticShader());
	
	cleanSelectedObjectList();
	//draw extents around selections
	for(SelectedObjectList_t::const_iterator it = m_selectedObjects.begin(); it != m_selectedObjects.end(); ++it) 
	{
		const ClientObject* const obj = NON_NULL((*it)->obj.getPointer());
		{
			//some object might not have appearances (cells, base objects, etc.)
			const Appearance* appearance = obj->getAppearance();
			if(appearance)
			{
				const Extent* const extent = appearance->getSelectionExtent();

				if (extent)
				{
					Graphics::setObjectToWorldTransformAndScale(obj->getTransform_o2w(), obj->getScale());
					Graphics::drawExtent(extent, VectorArgb::solidRed);
				}
				else
				{
					WARNING(true, ("GodClientData::draw() appearance->getSelectionAppearance() failed for [%s]", appearance->getAppearanceTemplateName()));
				}
			}
		}

		const Object* const ghost =(*it)->ghost; 
		
		//draw ghosts' extents and axes (ghosts always get axes)
		if(ghost)
		{
			const Extent* const extent     = NON_NULL(ghost->getAppearance()->getSelectionExtent());
			Graphics::setObjectToWorldTransformAndScale(ghost->getTransform_o2w(), ghost->getScale());
			Graphics::drawExtent(extent, VectorArgb::solidCyan);

			Graphics::setObjectToWorldTransformAndScale(Transform::identity, Vector::xyz111);
			Graphics::drawLine(obj->getAppearanceSphereCenter_w(), ghost->getAppearanceSphereCenter_w(), VectorArgb::solidBlue);

			Graphics::setObjectToWorldTransformAndScale(ghost->getTransform_o2w(), ghost->getScale());
			const BoxExtent* const boxExtent = dynamic_cast<const BoxExtent* const>(extent);
			real frameSize = 1;
			if(boxExtent)
			{
				real h = boxExtent->getHeight();
				real w = boxExtent->getWidth();
				real l = boxExtent->getLength();
				frameSize = h > w ? h : w;
				frameSize = frameSize > l ? frameSize : l;
			}
			Graphics::drawFrame(frameSize*2);
		}
	}

	//draw the axes on the objects we care about
	for(ObjectWithAxes_t::iterator i = m_showAxesObjects.begin(); i != m_showAxesObjects.end(); ++i)
	{
		Graphics::setObjectToWorldTransformAndScale((*i)->getTransform_o2w(), (*i)->getScale());
		const Extent* const    extent    = NON_NULL((*i)->getAppearance()->getSelectionExtent());
		const BoxExtent* const boxExtent = dynamic_cast<const BoxExtent* const>(extent);
		real frameSize = 1;
		if(boxExtent)
		{
			real h = boxExtent->getHeight();
			real w = boxExtent->getWidth();
			real l = boxExtent->getLength();
			frameSize = h > w ? h : w;
			frameSize = frameSize > l ? frameSize : l;
		}
		Graphics::drawFrame(frameSize*2);
	}

	const FreeCamera* const freeCamera = dynamic_cast<const FreeCamera*>(&camera); //use dynamic_cast because we can handle the case where the camera is of a different type

	//draw the crossbars if in GodClient camera mode
	if(freeCamera)
	{
		Graphics::setObjectToWorldTransformAndScale(Transform::identity, Vector::xyz111);
		
		//draw large crossbars at camera pivot location
		const Vector   pivotPt  = freeCamera->getPivotPoint();
		const real     ylen     = CONST_REAL(10);
		const real     crosslen = CONST_REAL(1);
		const Vector pivot1(pivotPt.x, pivotPt.y - CONST_REAL(ylen), pivotPt.z);
		const Vector pivot2(pivotPt.x, pivotPt.y + CONST_REAL(ylen), pivotPt.z);
		Graphics::drawLine(pivot1, pivot2, VectorArgb::solidBlue);
		drawCrossbars(pivotPt, crosslen, VectorArgb::solidGreen);

		//draw small crossbars at mouse cursor intersection location
		const real     mouseCursorCrossLen = CONST_REAL(0.5);
		drawCrossbars(m_mouseCursorIntersection, mouseCursorCrossLen, VectorArgb::solidRed);

		m_pivotPoint = pivotPt;
	}

	for(SphereTreeObjectList_t::iterator itr = m_sphereTreeObjects.begin(); itr != m_sphereTreeObjects.end(); ++itr)
	{
		Graphics::drawSphere(itr->sphere.location, itr->sphere.radius, 8, 16);
	}

	for(TriggerSphereList_t::iterator iter = m_triggerObjectSpheres.begin(); iter != m_triggerObjectSpheres.end(); ++iter)
	{
		const Object* const obj = dynamic_cast<const ClientObject*>(NetworkIdManager::getObjectById (iter->networkId));
		if(obj)
		{
			const Vector& loc = obj->getPosition_w();
			Graphics::drawSphere(loc, iter->sphere.radius, 8, 16);
		}
	}

	Graphics::setObjectToWorldTransformAndScale(Transform::identity, Vector::xyz111);
	if (m_renderAIPaths)
	{
		PackedArgb nodeColor(255, 255, 255, 255);
		PackedArgb edgeColor(255, 255, 255, 128);
		//PackedArgb parentColor(64, 255, 255, 255);
		AINodeList_t::iterator iteratr;
		for(iteratr = m_AINodes.begin(); iteratr != m_AINodes.end(); ++iteratr)
		{
			int nodeType = iteratr->second.m_type;
			if (nodeType <0) nodeType = 0;
			else if (nodeType > 5) nodeType = 5;

			AINode &from = iteratr->second;
			if (pathnodeRadii[nodeType] > 0)
			{
				drawLineSegment(
					from.m_location, pathnodeRadii[nodeType], 
					from.m_location + Vector::unitY*pathnodeRadii[nodeType], pathnodeRadii[nodeType]*0.5,
					nodeColor);
			}
			/*
			if (from.m_parent)
			{
				AINodeList_t::iterator to = m_AINodes.find(from.m_parent);
				if (to != m_AINodes.end())
				{
					Graphics::drawLine(from.m_location, to->second.m_location, parentColor);
				}
			}
			*/
			std::vector<int>::iterator sibIter;
			for (sibIter = from.m_siblings.begin(); sibIter != from.m_siblings.end(); ++sibIter)
			{
				if (*sibIter)
				{
					AINodeList_t::iterator to = m_AINodes.find(*sibIter);
					if (to != m_AINodes.end() && iteratr->first < *sibIter)
					{
						Graphics::drawLine(from.m_location, to->second.m_location, edgeColor);
					}
				}
			}
		}
	}

	for(AIPathList_t::iterator iteratr2 = m_AIPaths.begin(); iteratr2 != m_AIPaths.end(); ++iteratr2)
	{
		std::vector<AIPathInfo_NodeInfo> &nodes = iteratr2->second.m_nodes;
		std::vector<AIPathInfo_NodeInfo>::iterator nodeIter, next;
		float scale = 1.0f;
		PackedArgb baseColor(255, 64, 64, 64);
		PackedArgb color(baseColor);
		for (nodeIter = nodes.begin(); nodeIter != nodes.end(); ++nodeIter)
		{
			AINodeList_t::iterator fromIter = m_AINodes.find(nodeIter->node);
			if (fromIter != m_AINodes.end())
			{
				AINode &from = fromIter->second;
				if (nodeIter->state & AIPathInfo_NodeInfo::kReported)
				{
					color.setG(255);
				}
				next =nodeIter;
				++next;
				if (next != nodes.end())
				{
					if (!(next->state & AIPathInfo_NodeInfo::kReported))
					{
						color.setG(64);
					}
				}

				PackedArgb spikeColor(255,0,0,0);
				PackedArgb studColor(255,0,0,0);
				bool drawSpike = false;
				bool drawStud = false;
				if (nodeIter->state & AIPathInfo_NodeInfo::kPassed)
				{
					spikeColor.setG(255);
					drawSpike = true;
				}
				if (nodeIter->state & AIPathInfo_NodeInfo::kTarget)
				{
					spikeColor.setR(255);
					drawSpike = true;
				}
				if (nodeIter->state & AIPathInfo_NodeInfo::kFacingTarget)
				{
					spikeColor.setB(255);
					drawSpike = true;
				}
				if (drawSpike)
				{
					drawLineSegment(from.m_location, 0.03f, from.m_location+Vector::unitY * 0.2f, 0.15f, spikeColor);
				}

				if (nodeIter->state & AIPathInfo_NodeInfo::kCanMove)
				{
					studColor.setR(255);
					drawStud = true;
				}
				if (nodeIter->state & AIPathInfo_NodeInfo::kInCone)
				{
					studColor.setB(255);
					drawStud = true;
				}
				if (drawStud)
				{
					drawLineSegment(from.m_location, 0.075f, from.m_location+Vector::unitY * 0.1f, 0.015f, studColor);
				}
				if (nodeIter->state & AIPathInfo_NodeInfo::kFinalTarget)
				{
					drawLineSegment(from.m_location, 0.3f, from.m_location+Vector::unitY * 0.3f, 0.06f, PackedArgb::solidWhite);
				}
				next =nodeIter;
				++next;
				if (next != nodes.end())
				{
					AINodeList_t::iterator to = m_AINodes.find(next->node);
					if (to != m_AINodes.end())
					{
						drawLineSegment(from.m_location, 0.05f*scale, to->second.m_location, 0.01f*scale, color);
					}
				}
			}
		}
	}

	if(m_snapToHorizontalGrid || m_snapToVerticalGrid)
	{
		//get the avatar's location, we base the grid around his position
		NOT_NULL(m_gs);
		const Object* player = m_gs->getPlayer();
		const Vector& loc = player->getPosition_w();


		////// HORIZONTAL GRID //////////////////////
		//find the closet "snap" point to the avatar - this becomes the grid center
		Vector snappedLoc = snapToGrid(loc);
		//move to a grid corner, start drawing there
		Vector horizontalStartingPoint(snappedLoc.x -(m_snapToGridLinesPerHorizontalSide* m_snapToGridHorizontalSize / 2), snappedLoc.y+1, snappedLoc.z -(m_snapToGridLinesPerHorizontalSide* m_snapToGridHorizontalSize / 2));

		//draw a set of lines along the x axis, use accumulation to minimize per/frame computations
		Vector currentLineBegin = horizontalStartingPoint;
		Vector currentLineEnd(horizontalStartingPoint.x, horizontalStartingPoint.y, horizontalStartingPoint.z+m_snapToGridHorizontalLineLength);

//		Graphics::setObjectToWorldTransformAndScale(Transform::identity, Vector::xyz111);
//		player->getParentCell()


		if(m_snapToHorizontalGrid)
		{
			for(int i = 0; i <= m_snapToGridLinesPerHorizontalSide; ++i)
			{
				Graphics::drawLine(currentLineBegin, currentLineEnd, VectorArgb::solidWhite);
				currentLineBegin.x += m_snapToGridHorizontalSize;
				currentLineEnd.x += m_snapToGridHorizontalSize;
			}

			//draw a set of lines along the z axis, use accumulation to minimize per/frame computations
			currentLineBegin = horizontalStartingPoint;
			currentLineEnd.x = horizontalStartingPoint.x+m_snapToGridHorizontalLineLength;
			currentLineEnd.z = horizontalStartingPoint.z;
			for(int j = 0; j <= m_snapToGridLinesPerHorizontalSide; ++j)
			{
				Graphics::drawLine(currentLineBegin, currentLineEnd, VectorArgb::solidWhite);
				currentLineBegin.z += m_snapToGridHorizontalSize;
				currentLineEnd.z += m_snapToGridHorizontalSize;
			}
		}

		if(m_snapToVerticalGrid)
		{
			////// VERTICAL GRID //////////////////////
			//find the closet "snap" point to the avatar - this becomes the grid center
			snappedLoc = snapToGrid(loc);
			//move to a grid corner, start drawing there
			Vector verticalStartingPoint(snappedLoc.x -(m_snapToGridLinesPerVerticalSide* m_snapToGridVerticalSize / 2), snappedLoc.y+1 -(m_snapToGridLinesPerVerticalSide* m_snapToGridVerticalSize / 2), snappedLoc.z);

			//draw a set of lines along the x axis, use accumulation to minimize per/frame computations
			currentLineBegin = verticalStartingPoint;
			currentLineEnd.x = verticalStartingPoint.x;
			currentLineEnd.y = verticalStartingPoint.y+m_snapToGridVerticalLineLength;
			currentLineEnd.z = verticalStartingPoint.z;
			for(int k = 0; k <= m_snapToGridLinesPerVerticalSide; ++k)
			{
				Graphics::drawLine(currentLineBegin, currentLineEnd, VectorArgb::solidWhite);
				currentLineBegin.x += m_snapToGridVerticalSize;
				currentLineEnd.x += m_snapToGridVerticalSize;
			}

			//draw a set of lines along the y axis, use accumulation to minimize per/frame computations
			currentLineBegin = verticalStartingPoint;
			currentLineEnd.x = verticalStartingPoint.x+m_snapToGridVerticalLineLength;
			currentLineEnd.y = verticalStartingPoint.y;
			for(int l = 0; l <= m_snapToGridLinesPerVerticalSide; ++l)
			{
				Graphics::drawLine(currentLineBegin, currentLineEnd, VectorArgb::solidWhite);
				currentLineBegin.y += m_snapToGridVerticalSize;
				currentLineEnd.y += m_snapToGridVerticalSize;
			}
		}
	}

	//display regions filtered regions if the browser is also open
	RegionBrowser* regionBrowser = MainFrame::getInstance().getRegionBrowser();
	if(regionBrowser && regionBrowser->isVisible())
	{
		std::map<std::string, RegionRenderer::Region*> visibleRegions = regionBrowser->getVisibleRegions();
		for(std::map<std::string, RegionRenderer::Region*>::iterator k = visibleRegions.begin(); k != visibleRegions.end(); ++k)
		{
			RegionRenderer::RegionRect*  rect    = dynamic_cast<RegionRenderer::RegionRect*>(k->second);
			RegionRenderer::RegionCircle* circle = dynamic_cast<RegionRenderer::RegionCircle*>(k->second);

			if(rect)
			{
				float x  = rect->m_worldX;
				float z  = rect->m_worldZ;
				float ux = rect->m_ur_worldX;
				float uz = rect->m_ur_worldZ;
				Graphics::drawBox(Vector(x, -1000, z), Vector(ux, 1000, uz), VectorArgb::solidGreen);
			}
			else if(circle)
			{
				float x  = circle->m_worldX;
				float z  = circle->m_worldZ;
				float radius = circle->m_radius;
				UNREF(radius);
				Graphics::drawCylinder(Vector(x, 0, z), radius, 100, 16, 16, 16, 16, VectorArgb::solidBlue);
			}
			else
			{
				DEBUG_FATAL(true, ("Region is neither a cirlce nor a rectagle"));
			}
		}
	}
}

//-----------------------------------------------------------------

void GodClientData::cursorScreenPositionChanged(int x, int y)
{
	m_cursorPosition.x = static_cast<real>(x);
	m_cursorPosition.y = static_cast<real>(y);
}

//-----------------------------------------------------------------

Vector2d GodClientData::getCursorScreenPosition() const
{
	return m_cursorPosition;
}

//-----------------------------------------------------------------

void GodClientData::setSelection(ClientObject* obj)
{
	NOT_NULL(obj);
	//don't manipulate objects without appearances (cells, planet objects, etc.)
	if(!obj->getAppearance())
		return;
	clearSelection();
	addSelection(obj);
} //lint !e429 // custodial sobj

//-----------------------------------------------------------------
/**
* Adding an existing selection moves it to the end of the list
*/
void GodClientData::addSelection(ClientObject* obj)
{
	//don't manipulate objects without appearances (cells, planet objects, etc.)
	NOT_NULL(obj);
	if(!obj->getAppearance())
		return;

	NOT_NULL(m_gs);

	const CellProperty* const playerCell = m_gs->getPlayer()->getParentCell();
	const CellProperty* const objectCell = obj->getParentCell();
	//don't allow selection of objects in a cell different from the player
	if (playerCell == objectCell)
	{
		IGNORE_RETURN(removeSelection(obj));
		m_selectedObjects.push_back(new SelectedObject(obj));
		emitMessage(MessageDispatch::MessageBase(Messages::SELECTION_CHANGED));
	}
}

//-----------------------------------------------------------------

bool GodClientData::removeSelection(const ClientObject* obj)
{
	bool found = false;
	for(SelectedObjectList_t::iterator it = m_selectedObjects.begin(); it != m_selectedObjects.end(); ++it) 
	{
		SelectedObject* selObj = NON_NULL(*it);
		if(selObj->obj == obj)
		{
			delete selObj;
			IGNORE_RETURN(m_selectedObjects.erase(it));
			found = true;
			break;
		}
	}

	disableShowObjectAxes(obj);

	if(found)
		emitMessage(MessageDispatch::MessageBase(Messages::SELECTION_CHANGED));

	return found;
}

//-----------------------------------------------------------------

void GodClientData::toggleSelection(ClientObject* obj)
{
	NOT_NULL(obj);
	//don't manipulate objects without appearances (cells, planet objects, etc.)
	if(!obj->getAppearance())
		return;
	if(!removeSelection(obj))
	{
		addSelection(obj);
	}
}

//-----------------------------------------------------------------

int GodClientData::getSelectionSize()
{
	return m_selectedObjects.size();
}

//-----------------------------------------------------------------

void GodClientData::storeSelection(int key)
{
	//build up a list of the currently selected client objects
	std::list<ClientObject*> selectionList;
	for(SelectedObjectList_t::iterator it = m_selectedObjects.begin(); it != m_selectedObjects.end(); ++it)
	{
		selectionList.push_back((*it)->obj);
	}
	//now push it out for later use
	m_persistedSelections[key] = selectionList;
}

//-----------------------------------------------------------------

void GodClientData::restoreSelection(int key)
{
	//get back the previously built selection list(or an empty list if it hasn't been used)
	std::list<ClientObject*> selectedList = m_persistedSelections[key];
	//leave the selection as-is if the requested group is empty
	if(selectedList.empty())
		return;
	clearSelection();
	for(std::list<ClientObject*>::iterator it = selectedList.begin(); it != selectedList.end(); ++it)
	{
		//repopulate the selection
		addSelection(*it);
	}
	emitMessage(MessageDispatch::MessageBase(Messages::SELECTION_CHANGED));
}

//-----------------------------------------------------------------
void GodClientData::setObjectCreationPending(bool val)
{
	m_objectCreationPending = val;
}

//-----------------------------------------------------------------
bool GodClientData::getObjectCreationPending()
{
	return m_objectCreationPending;
}


//-----------------------------------------------------------------

bool GodClientData::pasteLocationKnown() const
{
	return m_pasteLocationKnown;
}

//-----------------------------------------------------------------

void GodClientData::setPasteLocation(const Vector& vec)
{
	m_pasteLocationKnown = true;
	m_pasteLocation = vec;
}

//-----------------------------------------------------------------

Vector GodClientData::absorbPasteLocation()
{
	m_pasteLocationKnown = false;
	Vector result = m_pasteLocation;
	m_pasteLocation = Vector::zero;
	return result;
}

//-----------------------------------------------------------------

/** Given a new a translation vector in world space, move the objects by that amount
 */
void GodClientData::translateGhosts(const Vector& v, bool alongGround)
{
	if(m_selectedObjects.empty())
		return;

	m_createdCount = 0;

	int len = m_selectedObjects.size(); 
	UNREF(len);

	for(SelectedObjectList_t::iterator it = m_selectedObjects.begin(); it != m_selectedObjects.end(); ++it) 
	{
		SelectedObject* const selObj = NON_NULL(*it);
		if(selObj->ghost == 0)
			selObj->ghost = NON_NULL(createGhost(*selObj->obj));

		const Vector objLoc = selObj->obj->getPosition_p();
		const Vector ghostLoc = selObj->ghost->getPosition_p();

		//find the diff to the closest snap point(if snap to grid is off, this trivials to a 0 diff)
		Vector snappedLoc  = snapToGrid(ghostLoc);
		Vector snappedDiff = snappedLoc - ghostLoc;

		real oldHeightDiff = objLoc.y - getObjectDropCollisionHeight(*selObj->ghost);

		if(m_snapToHorizontalGrid || m_snapToVerticalGrid)
		{
			if(m_snapToHorizontalGrid)
			{
				if(v.x > m_snapToGridHorizontalThreshold / (10 * m_snapToGridHorizontalSize))
					snappedDiff.x += m_snapToGridHorizontalSize;
				else if(v.x < -m_snapToGridHorizontalThreshold / (10 * m_snapToGridHorizontalSize))
					snappedDiff.x -= m_snapToGridHorizontalSize;

				if(v.z > m_snapToGridHorizontalThreshold / (10 * m_snapToGridHorizontalSize))
					snappedDiff.z += m_snapToGridHorizontalSize;
				else if(v.z < -m_snapToGridHorizontalThreshold / (10 * m_snapToGridHorizontalSize))
					snappedDiff.z -= m_snapToGridHorizontalSize;
			}
			if(m_snapToVerticalGrid)
			{
				if(v.y > m_snapToGridVerticalThreshold / (10 * m_snapToGridVerticalSize))
					snappedDiff.y += m_snapToGridVerticalSize;
				else if(v.y < -m_snapToGridVerticalThreshold / (10 * m_snapToGridVerticalSize))
					snappedDiff.y -= m_snapToGridVerticalSize;
			}
			selObj->ghost->move_p(snappedDiff);
		}
		else
			selObj->ghost->move_p(v);

		if(alongGround)
			dropObjectToTerrain(*selObj->ghost, oldHeightDiff);
	}

	if(m_createdCount)
		emitMessage(MessageDispatch::MessageBase(Messages::GHOSTS_CREATED));
}

//-----------------------------------------------------------------

void GodClientData::scaleGhosts(const Vector& v)
{	
	if(m_selectedObjects.empty())
		return;
	
	m_createdCount = 0;

	for(SelectedObjectList_t::iterator it = m_selectedObjects.begin(); it != m_selectedObjects.end(); ++it) 
	{
		SelectedObject* const selObj = NON_NULL(*it);
		if(selObj->ghost == 0)
			selObj->ghost = NON_NULL(createGhost(*selObj->obj));
		selObj->ghost->setScale(selObj->ghost->getScale() + v); 
	}
}

//-----------------------------------------------------------------

void GodClientData::rotateGhosts(const real v, RotationType type, RotationPivotType pivotType)
{
	UNREF(type);
	
	if(m_selectedObjects.empty())
		return;
	
	m_createdCount = 0;

	//-- find the rotational centerpoint, if any
	bool useCenter = false;
	Vector centerPoint;
	if(pivotType == RotatePivot_selectionCenter)
	{
		IGNORE_RETURN(calculateSelectionCenter(centerPoint, true));
		useCenter = true;
	}
	else if(pivotType == RotatePivot_lastSelection)
	{
		SelectedObject* const pivSelObj = NON_NULL(m_selectedObjects.back());
		if(pivSelObj->ghost == 0)
			pivSelObj->ghost = NON_NULL(createGhost(*pivSelObj->obj));		
		centerPoint = pivSelObj->ghost->getAppearanceSphereCenter_w();
		useCenter = true;
	}
	
	for(SelectedObjectList_t::iterator it = m_selectedObjects.begin(); it != m_selectedObjects.end(); ++it) 
	{
		SelectedObject* const selObj = NON_NULL(*it);
		if(selObj->ghost == 0)
			selObj->ghost = NON_NULL(createGhost(*selObj->obj));
		
		Vector translate;

		if(useCenter)
		{
			translate = selObj->ghost->rotateTranslate_w2o(centerPoint);
			selObj->ghost->move_o(translate);
		}

		switch(type)
		{
		case Rotate_yaw:
			selObj->ghost->yaw_o(v);
			break;
		case Rotate_pitch:
			selObj->ghost->pitch_o(v);
			break;
		case Rotate_roll:
			selObj->ghost->roll_o(v);
			break;
		case Rotate_none:
		default:
			break;
		}

		if(useCenter)
		{
			selObj->ghost->move_o(-translate);
		}
	}

	if(m_createdCount)
		emitMessage(MessageDispatch::MessageBase(Messages::GHOSTS_CREATED));
}

//-----------------------------------------------------------------

void GodClientData::dropGhostsToTerrain()
{
	//@todo interiors fix for interiors

	if( m_selectedObjects.empty())
		return;
		
	m_createdCount = 0;

	for(SelectedObjectList_t::iterator it = m_selectedObjects.begin(); it != m_selectedObjects.end(); ++it) 
	{
		SelectedObject* const selObj = NON_NULL(*it);
		if(selObj->ghost == 0)
			selObj->ghost = NON_NULL(createGhost(*selObj->obj));
		dropObjectToTerrain(*selObj->ghost, CONST_REAL(0));
	}
	
	if(m_createdCount)
		emitMessage(MessageDispatch::MessageBase(Messages::GHOSTS_CREATED));
}

//-----------------------------------------------------------------

void GodClientData::resetGhostsRotations()
{
	if(m_selectedObjects.empty())
		return;
	
	m_createdCount = 0;
	
	for(SelectedObjectList_t::iterator it = m_selectedObjects.begin(); it != m_selectedObjects.end(); ++it) 
	{
		SelectedObject* const selObj = NON_NULL(*it);
		if(selObj->ghost == 0)
			selObj->ghost = NON_NULL(createGhost(*selObj->obj));
		selObj->ghost->resetRotate_o2p();
	}
		
	if(m_createdCount)
		emitMessage(MessageDispatch::MessageBase(Messages::GHOSTS_CREATED));
}

//-----------------------------------------------------------------

void GodClientData::alignGhostsToTerrain()
{
	//@todo interiors fix for interiors

	if(m_selectedObjects.empty())
		return;
	
	m_createdCount = 0;
	
	for(SelectedObjectList_t::iterator it = m_selectedObjects.begin(); it != m_selectedObjects.end(); ++it) 
	{
		SelectedObject* const selObj = NON_NULL(*it);
		if(selObj->ghost == 0)
			selObj->ghost = NON_NULL(createGhost(*selObj->obj));
		alignObjectToTerrain(*selObj->ghost);
	}
		
	if(m_createdCount)
		emitMessage(MessageDispatch::MessageBase(Messages::GHOSTS_CREATED));
}

//-----------------------------------------------------------------

void GodClientData::getSelection(GodClientData::ObjectList_t& objectList)
{
	objectList.clear();
	cleanSelectedObjectList();
	for(SelectedObjectList_t::iterator it = m_selectedObjects.begin(); it != m_selectedObjects.end(); ++it) 
	{
		ClientObject* const obj = NON_NULL((*it)->obj.getPointer()); 
		objectList.push_back(obj);
	}
}

//-----------------------------------------------------------------

void GodClientData::getClipboard(GodClientData::ClipboardList_t& clip) const
{
	clip = m_clipboard;
}

//-----------------------------------------------------------------

void GodClientData::setCurrentBrush(const GodClientData::ClipboardList_t& b)
{
	m_currentBrush = b;
}

//-----------------------------------------------------------------

void GodClientData::getCurrentBrush(GodClientData::ClipboardList_t& b) const
{
	b = m_currentBrush;
}

//-----------------------------------------------------------------

void GodClientData::replaceSelection(const GodClientData::ObjectList_t& objectList)
{
	clearSelection();
	for(ObjectList_t::const_iterator it = objectList.begin(); it != objectList.end(); ++it) 
		addSelection(*it);
}

//-----------------------------------------------------------------

ClientObject* GodClientData::findSelectionByNetworkId(const NetworkId& id)
{
	cleanSelectedObjectList();
	SelectedObject* const selObj = findSelectedObjectByNetworkId(id);
	return selObj ? selObj->obj.getPointer() : 0;
}

//-----------------------------------------------------------------

GodClientData::SelectedObject* GodClientData::findSelectedObjectByNetworkId(const NetworkId& id)
{
	for(SelectedObjectList_t::iterator it = m_selectedObjects.begin(); it != m_selectedObjects.end(); ++it) 
	{
		if((*it)->obj->getNetworkId() == NetworkId(id))
			return(*it);
	}
	return 0;
}

//-----------------------------------------------------------------

real GodClientData::getObjectDropCollisionHeight(const Object& obj) const
{
	bool         objectCollision = false;
	Vector       objectCollisionPoint;
	const Vector end             = obj.getPosition_w() -(Vector::unitY * 10000.0f);
	const Vector start           = obj.getPosition_w() +(Vector::unitY* obj.getAppearanceSphereRadius());
	ClientWorld::CollisionInfoVector results;
	CellProperty const * const avatarCell = m_gs->getPlayer ()->getParentCell ();			
				
	if(ClientWorld::collide(avatarCell, start, end, CollideParameters::cms_default,results,ClientWorld::CF_allCamera))
	{
		ClientWorld::CollisionInfoVector::iterator iiEnd = results.end();
		
		for (ClientWorld::CollisionInfoVector::iterator ii = results.begin(); ii != iiEnd;  ++ii)
		{
			CollisionInfo & collisionInfo = *ii;
			Object * const objectToTest = const_cast<Object*>(collisionInfo.getObject());
			NOT_NULL(objectToTest);

			// don't collide with the original object or the ghost (myself)
			bool isSelection, isGhost;
			getIsSelectionOrGhost(*objectToTest,isSelection,isGhost);
			if (isSelection || isGhost) 
			{	
				continue;
			}
			
			const Vector collisionInfoPoint = collisionInfo.getPoint();

			// only look for collision below me
			if(!objectCollision || collisionInfoPoint.y > objectCollisionPoint.y) 
			{
				objectCollisionPoint = collisionInfoPoint;
				objectCollision      = true;
			}
		}
	}
	
	if(objectCollision)
	{
		return objectCollisionPoint.y;
	}

	return CONST_REAL(0);
}

//-----------------------------------------------------------------

void GodClientData::dropObjectToTerrain(Object& obj, real height) const
{
	Vector v = obj.getPosition_w();
	v.y = getObjectDropCollisionHeight(obj) + height;
	obj.setPosition_w(v);
}

//-----------------------------------------------------------------

void GodClientData::alignObjectToTerrain(Object& obj) const
{
	//@todo interiors fix for interiors
	const TerrainObject* const terrain = TerrainObject::getConstInstance ();
	if(!terrain)
		return;

	Vector worldStart = obj.getPosition_w();
	Vector worldEnd = worldStart;

	worldStart.y = CONST_REAL(10000);
	worldEnd.y   = -CONST_REAL(10000);

	CollisionInfo result;
	if(terrain->collide(worldStart, worldEnd, result))
	{
		Transform t = obj.getTransform_o2w();
		t.resetRotate_l2p();
		t.yaw_l(obj.getObjectFrameK_w().theta());
		
		Vector k = t.getLocalFrameK_p();
		
		Vector i = result.getNormal().cross(k);
		if(!i.normalize())
			DEBUG_FATAL(true,("couldn't normalize vector"));
		
		k = i.cross(result.getNormal());
		if(!k.normalize())
			DEBUG_FATAL(true,("couldn't normalize vector"));
		
		t.setLocalFrameIJK_p(i, result.getNormal(), k);
		t.reorthonormalize();
		
		obj.setTransform_o2p(t);
	}
}

//-----------------------------------------------------------------

bool GodClientData::findIntersection_p (const int screenX, const int screenY, const CellProperty*& cellProperty, Vector& intersection_p)
{
	//-- if groundscene or the player do not exist
	if (!m_gs || !m_gs->getPlayer ())
		return false;

	//-- if the camera does not exist
	const Camera* const camera = m_gs->getCurrentCamera ();
	if (!camera)
		return false;

	//-- get a ray from the current camera position to through the <screenX, screenY>
	const Vector start_p (camera->getPosition_p ());
	const Vector end_p   (start_p + 8192.f * camera->rotate_o2p (camera->reverseProjectInScreenSpace (screenX, screenY)));

	const Object* const player = m_gs->getPlayer ();
	NOT_NULL (player);
	cellProperty = m_gs->getPlayer ()->getParentCell ();
	NOT_NULL (cellProperty);

	if (cellProperty != CellProperty::getWorldCellProperty())
	{
		//-- if we're in an interior, do a floor collision
		const Floor* const floor = cellProperty->getFloor();
		if (floor)
		{
			FloorLocator tempLoc;
			Vector direction_p = end_p - start_p;
			direction_p.normalize ();
			Ray3d testRay(start_p, direction_p);
			if (const_cast<Floor*>(floor)->intersectClosest(testRay, tempLoc))
			{
				intersection_p = tempLoc.getPosition_p();
				return true;
			}
		}
	}
	else
	{
		//we're in the world cell, do terrain collision
		const TerrainObject* const terrain = TerrainObject::getConstInstance ();
		if (terrain)
		{
			CollisionInfo info;
			if (terrain->collide(start_p, end_p, info))
			{
				intersection_p = info.getPoint();
				return true;
			}
		}
	}

	return false;
}

//-----------------------------------------------------------------

void GodClientData::clearSelection()
{
	std::for_each(m_selectedObjects.begin(), m_selectedObjects.end(), SelectedObjectDestroyer());
	m_selectedObjects.clear();

	emitMessage(MessageDispatch::MessageBase(Messages::SELECTION_CHANGED));
	emitMessage(MessageDispatch::MessageBase(Messages::GHOSTS_KILLED));
}

//-----------------------------------------------------------------

void GodClientData::clearClipboard()
{
	for(ClipboardList_t::iterator it = m_clipboard.begin(); it != m_clipboard.end();)
	{
		ClipboardObject* const clipObj = *it;
		delete clipObj;
		it = m_clipboard.erase(it);
	}
	emitMessage(MessageDispatch::MessageBase(Messages::CLIPBOARD_CHANGED));
}

//-----------------------------------------------------------------

void GodClientData::copySelection()
{
	cleanSelectedObjectList();
	//check that all objects have the required data to copy them(since it's a non-synchronous operation)
	{
		for(SelectedObjectList_t::iterator it = m_selectedObjects.begin(); it != m_selectedObjects.end(); ++it)
		{
			ClientObject* const obj = NON_NULL((*it)->obj.getPointer());
			if(!Game::getSinglePlayer())
			{
				const ServerObjectData::ObjectInfo* serverObjectData = ServerObjectData::getInstance().getObjectInfo(obj->getNetworkId(), false);
				if(!serverObjectData)
				{
					QApplication::beep();
					return;
				}
			}
		}
	}
	
	clearClipboard();

	//don't copy the player
	{
		const Object* const player = m_gs ? m_gs->getPlayer() : 0;
		for(SelectedObjectList_t::iterator it = m_selectedObjects.begin(); it != m_selectedObjects.end(); ++it)
		{    
			ClientObject* const obj = NON_NULL((*it)->obj.getPointer());
			if(obj == player)
				continue;
			else
				m_clipboard.push_back(new ClipboardObject(*obj));
		}
	}
	
	emitMessage(MessageDispatch::MessageBase(Messages::CLIPBOARD_CHANGED));
}

//-----------------------------------------------------------------

bool GodClientData::getSelectionEmpty() const
{
	return m_selectedObjects.empty();
}

//-----------------------------------------------------------------

bool GodClientData::getClipboardEmpty() const
{
	return m_clipboard.empty();
}

//-----------------------------------------------------------------

void GodClientData::synchronizeSelectionWithGhosts()
{
	if(m_selectedObjects.empty())
		return;

	if(isToggleDropToTerrainOn())
	{
		dropGhostsToTerrain();
	}
	if(isToggleAlignToTerrainOn())
	{
		alignGhostsToTerrain();
	}

	bool ghosts_killed = false;

	cleanSelectedObjectList();

	bool allDone = false;
	while (!allDone)
	{
		bool iteratorsInvalidated = false;
		for (SelectedObjectList_t::iterator it = m_selectedObjects.begin(); it != m_selectedObjects.end(); ++it)
		{
			SelectedObject* const selObj = NON_NULL(*it);

			if(selObj->ghost)
			{
				ClientObject* const obj = NON_NULL(selObj->obj.getPointer());

				ghosts_killed = true;

				if (obj->getTransform_o2p() != selObj->ghost->getTransform_o2p())
				{
					NetworkId objId = obj->getNetworkId();
					
					ServerCommander::getInstance().setObjectTransform(obj, selObj->ghost->getTransform_o2p(), true);
					
					if (NetworkIdManager::getObjectById(objId) != obj)
					{
						iteratorsInvalidated = true;
						break;
					}
				}

				selObj->ghost->kill();

				//-- Ensure the ghost gets an alter call this upcoming frame so it gets deleted properly.
				selObj->ghost->scheduleForAlter();

				selObj->ghost = 0;
			}
		}
		if (!iteratorsInvalidated)
			allDone = true;
	}

	emitMessage(MessageDispatch::MessageBase(Messages::SELECTED_OBJECTS_CHANGED));

	if(ghosts_killed)
		emitMessage(MessageDispatch::MessageBase(Messages::GHOSTS_KILLED));
}

//-----------------------------------------------------------------

void GodClientData::unlockSelectedObjects()
{
	if (m_selectedObjects.empty())
		return;

	cleanSelectedObjectList();

	for (SelectedObjectList_t::iterator it = m_selectedObjects.begin(); it != m_selectedObjects.end(); ++it)
	{
		SelectedObject* const selObj = NON_NULL(*it);
		ClientObject* const obj = NON_NULL(selObj->obj.getPointer());
		if (obj && obj->getNetworkId() < NetworkId::cms_invalid)
			BuildoutAreaSupport::unlock(*obj);
	}
}

//-----------------------------------------------------------------

int GodClientData::killGhosts()
{
	bool numKilled = 0;

	for(SelectedObjectList_t::iterator it = m_selectedObjects.begin(); it != m_selectedObjects.end(); ++it) 
	{
		SelectedObject* const selObj = NON_NULL(*it);

		if(selObj->ghost)
		{
			selObj->ghost->kill();

			//-- Ensure the ghost gets an alter call this upcoming frame so it gets deleted properly.
			selObj->ghost->scheduleForAlter();

			selObj->ghost = 0;
			++numKilled;
		}
	}
	emitMessage(MessageDispatch::MessageBase(Messages::GHOSTS_KILLED));

	return numKilled;
}

//-----------------------------------------------------------------

void GodClientData::createGhosts()
{
	for(SelectedObjectList_t::iterator it = m_selectedObjects.begin(); it != m_selectedObjects.end(); ++it) 
	{
		SelectedObject* const selObj = NON_NULL(*it);

		if(selObj->ghost)
		{
			selObj->ghost->kill();

			//-- Ensure the ghost gets an alter call this upcoming frame so it gets deleted properly.
			selObj->ghost->scheduleForAlter();

			selObj->ghost = 0;
		}

		selObj->ghost = NON_NULL(createGhost(*selObj->obj));
	}
}

//-----------------------------------------------------------------

/**
 * Create a ghost.  If it is a particle system make sure to pause it if the source object is paused
 */
Object* GodClientData::createGhost(ClientObject& obj)
{
	Object* const ghost = new Object();
	ghost->setParentCell(obj.getParentCell());

	// -TRF- remove the skeletal appearance checking once rebuildMesh/draw
	//       issues are resolved.
	Appearance* const          baseAppearance     = obj.getAppearance()->getAppearanceTemplate()->createAppearance();

	// if we've got a skeletal appearance, we need to call rebuildMesh()
	SkeletalAppearance2*const skeletalAppearance = dynamic_cast<SkeletalAppearance2*>(baseAppearance); // this is allowed to fail.
	if(skeletalAppearance)
			IGNORE_RETURN(skeletalAppearance->rebuildIfDirtyAndAvailable());

	// set ghost object appearance
	ghost->setAppearance(baseAppearance);

	//add an object notification - so it actually renders
	ghost->addNotification(ClientWorld::getIntangibleNotification());
	RenderWorld::addObjectNotifications (*ghost);

	ghost->setTransform_o2p(obj.getTransform_o2p());
	ClientWorld::queueObject(ghost);
	++m_createdCount;

	ParticleEffectAppearance* originalParticleAppearance = ParticleEffectAppearance::asParticleEffectAppearance(obj.getAppearance());
	if(originalParticleAppearance)
	{
		if(originalParticleAppearance->isPaused())
		{
			ParticleEffectAppearance* particleAppearance = ParticleEffectAppearance::asParticleEffectAppearance(baseAppearance);
			if(particleAppearance)
			{
				particleAppearance->setPaused(true);
			}
		}
	}

	return ghost;
}

//-----------------------------------------------------------------

/**Determine the world-space location of the center of the selection
 */
bool GodClientData::calculateSelectionCenter(Vector& center, bool ghosts) const
{
	if(m_selectedObjects.empty())
		return false;

	center = Vector::zero;

	for(SelectedObjectList_t::const_iterator it = m_selectedObjects.begin(); it != m_selectedObjects.end(); ++it) 
	{
		const SelectedObject* const selObj = NON_NULL(*it);

		if(ghosts && selObj->ghost)
			center += selObj->ghost->getAppearanceSphereCenter_w();
		else
			center += selObj->obj->getAppearanceSphereCenter_w();
	}

	center /= static_cast<real>(m_selectedObjects.size());
	return true;
}

//-----------------------------------------------------------------

bool GodClientData::calculateClipboardCenter(const ClipboardList_t& clip, Vector& center) const
{
	if(clip.empty())
		return false;
	center = Vector::zero;
	for(ClipboardList_t::const_iterator it = clip.begin(); it != clip.end(); ++it) 
	{
		const ClipboardObject* const clipObj = NON_NULL(*it);
		center += clipObj->transform.getPosition_p();
	}
	center /= static_cast<real>(clip.size());
	return true;
}

//-----------------------------------------------------------------

bool GodClientData::calculateClipboardBottom(const ClipboardList_t& clip, float& bottom) const
{
	if(clip.empty())
		return false;
	bottom = 0;
	bool first = true;
	for(ClipboardList_t::const_iterator it = clip.begin(); it != clip.end(); ++it) 
	{
		const ClipboardObject* const clipObj = NON_NULL(*it);
		if(first)
		{
			bottom = clipObj->transform.getPosition_p().y;
			first = false;
		}
		else
		{
			if(clipObj->transform.getPosition_p().y < bottom)
				bottom = clipObj->transform.getPosition_p().y;
		}
	}
	return true;
}

//-----------------------------------------------------------------

bool GodClientData::getSelectionExtent(bool ghosts, BoxExtent& extent) const
{
	extent.setMin(Vector::maxXYZ);
	extent.setMax(Vector::negativeMaxXYZ);

	int count = 0;

	for(SelectedObjectList_t::const_iterator it = m_selectedObjects.begin(); it != m_selectedObjects.end(); ++it) 
	{
		const SelectedObject* const selObj = NON_NULL(*it);

		const Object* obj = ghosts ? selObj->ghost : selObj->obj;

		if(!obj)
			continue;
		
		++count;

		const real radius = obj->getAppearanceSphereRadius();

		Vector ijk [3] =
		{
			obj->getObjectFrameI_w(),
			obj->getObjectFrameJ_w(),
			obj->getObjectFrameK_w()
		};

		IGNORE_RETURN(ijk [0].normalize());
		IGNORE_RETURN(ijk [1].normalize());
		IGNORE_RETURN(ijk [2].normalize());

		ijk [0]*= radius;
		ijk [1]*= radius;
		ijk [2]*= radius;

		extent.updateMinAndMax(obj->getAppearanceSphereCenter_w() + ijk [0]);
		extent.updateMinAndMax(obj->getAppearanceSphereCenter_w() - ijk [0]);
		extent.updateMinAndMax(obj->getAppearanceSphereCenter_w() + ijk [1]);
		extent.updateMinAndMax(obj->getAppearanceSphereCenter_w() - ijk [1]);
		extent.updateMinAndMax(obj->getAppearanceSphereCenter_w() + ijk [2]);
		extent.updateMinAndMax(obj->getAppearanceSphereCenter_w() - ijk [2]);
	}

	if(count == 0)
		return false;

	extent.calculateCenterAndRadius();
	return true;
}

//-----------------------------------------------------------------


/*
 * Returns true on selection, false on ghost
 *
 */
bool GodClientData::getIsSelectionOrGhost(const Object& obj, bool& isSelection, bool& isGhost) const
{
	isSelection = false;
	isGhost = false;

	for(SelectedObjectList_t::const_iterator it = m_selectedObjects.begin(); it != m_selectedObjects.end(); ++it) 
	{
		const SelectedObject* const selObj = NON_NULL(*it);

		if(&obj == selObj->obj)
		{
			isSelection = true;
			return true;
		}
		if(&obj == selObj->ghost)
		{
			isGhost = true;
			return false;
		}
	}

	return false;
}

//-----------------------------------------------------------------

int GodClientData::countGhosts()
{
	int ghosts = 0;
	for(SelectedObjectList_t::const_iterator it = m_selectedObjects.begin(); it != m_selectedObjects.end(); ++it) 
	{	
		const SelectedObject* const selObj = NON_NULL(*it);
		if(selObj->ghost)
			++ghosts;
	}

	return ghosts;
}

//-----------------------------------------------------------------

bool GodClientData::moveGhostsFollowingObject(const NetworkId& oid, const Vector& pt, bool alongGround)
{
	SelectedObject* const selObj = findSelectedObjectByNetworkId(oid);

	if(!selObj)
		return false;
	
	const Vector leaderPos = selObj->ghost ? selObj->ghost->getPosition_w() : selObj->obj->getPosition_w();
	const Vector diff = pt - leaderPos;

	translateGhosts(diff, alongGround);
	return true;
}

//-----------------------------------------------------------------

void GodClientData::translateSelection(real x, real y, bool alongGround)
{
	if(!m_gs)
		return;

	m_gs->activateGodClientCamera();
	FreeCamera* const cam = NON_NULL(m_gs->getGodClientCamera());

	Vector loc;

	Vector center;

	const real pitch = cam->getPitch();
	if(pitch > -PI_OVER_4 && pitch < PI_OVER_4)
		loc = cam->rotate_o2p(Vector(x, CONST_REAL(0), -y));
	else
		loc = cam->rotate_o2p(Vector(x, -y, CONST_REAL(0)));

	if(!calculateSelectionCenter(center, true))
		return;

	real const accurateDistance = std::max(CONST_REAL(0.01), center.magnitudeBetween(cam->getPosition_w()));

	//scale the movement by the distance between camera and object
	if (accurateDistance > CONST_REAL(10))
		//-- use the standard god client adjustment then the camera is a normal or far distance from the object
		loc *= sqrt(accurateDistance);
	else
	{
		//-- do finer distance adjustment when the camera is close to the center of the translated object
		real const scaleFactor = accurateDistance / CONST_REAL(10);
		loc *= sqrt(CONST_REAL(10)) * scaleFactor * scaleFactor * scaleFactor;
	}

	loc.y = 0;

	translateGhosts(loc, alongGround);
}

//-----------------------------------------------------------------

void GodClientData::translateSelectionY(real dy)
{
	if(!m_gs)
		return;

	m_gs->activateGodClientCamera();
	FreeCamera* const cam = NON_NULL(m_gs->getGodClientCamera());

	Vector center;
	if(!calculateSelectionCenter(center, true))
		return;

	real dist  = std::max(CONST_REAL(0.01), center.magnitudeBetween(cam->getPosition_w()));

	if(dist > CONST_REAL(10))
	{
		dist = sqrt(dist);
	}
	else
	{
		real const scaleFactor = dist/CONST_REAL(10);
		dist = sqrt(CONST_REAL(10)) * scaleFactor * scaleFactor * scaleFactor;
	}

	translateGhosts(Vector(CONST_REAL(0), dy* dist, CONST_REAL(0)), false);
}


//-----------------------------------------------------------------

void GodClientData::scaleSelection(real dx, real dy)
{
	if(!m_gs)
		return;

	m_gs->activateGodClientCamera();
	FreeCamera* const cam = NON_NULL(m_gs->getGodClientCamera());

	Vector v;

	const real pitch = cam->getPitch();
	if(pitch > -PI_OVER_4 && pitch < PI_OVER_4)
		v = cam->rotate_o2w(Vector(dx, CONST_REAL(0), -dy));
	else
		v = cam->rotate_o2w(Vector(dx, -dy, CONST_REAL(0)));

	Vector center;
	if(!calculateSelectionCenter(center, true))
		return;

	const real dist  = std::max(CONST_REAL(10), center.magnitudeBetween(cam->getPosition_w()));

		v*= sqrt(dist);

	v.y = 0;

	scaleGhosts(v);
}

//-----------------------------------------------------------------

void GodClientData::scaleSelectionY(real dy)
{
	if(!m_gs)
		return;

	m_gs->activateGodClientCamera();
	FreeCamera* const cam = NON_NULL(m_gs->getGodClientCamera());

	Vector center;
	if(!calculateSelectionCenter(center, true))
		return;

	const real dist  = sqrt(std::max(CONST_REAL(10), center.magnitudeBetween(cam->getPosition_w())));

	scaleGhosts(Vector(CONST_REAL(0), dy* dist, CONST_REAL(0)));
}

//-----------------------------------------------------------------

void GodClientData::saveCurrentSelectionAsBrush(const std::string& brushName)
{
	ClipboardList_t newBrush;
	const Object* const player = m_gs ? m_gs->getPlayer() : 0;

	cleanSelectedObjectList();
	//check that all objects have the required data to copy them(since it's a non-synchronous operation)
	for(SelectedObjectList_t::iterator it = m_selectedObjects.begin(); it != m_selectedObjects.end(); ++it) 
	{
		ClientObject* const obj = NON_NULL((*it)->obj.getPointer()); 
		if(!Game::getSinglePlayer())
		{
			const ServerObjectData::ObjectInfo* serverObjectData = ServerObjectData::getInstance().getObjectInfo(obj->getNetworkId(), false);
			if(!serverObjectData)
			{
				QApplication::beep();
				return;
			}
		}
	}

	for(SelectedObjectList_t::iterator itr = m_selectedObjects.begin(); itr != m_selectedObjects.end(); ++itr) 
	{
		ClientObject* const obj = NON_NULL((*itr)->obj.getPointer()); 
		//don't store the player in a brush
		if(obj == player)
			continue;
		else
		{
			newBrush.push_back(new ClipboardObject(*obj));
		}
	}
	BrushData::getInstance().addBrush(brushName, newBrush);
}

//-----------------------------------------------------------------

void GodClientData::addPalette(const std::string& name)
{
	Palette_t palette = m_palettes[name];
	if(!palette.empty())
		return;
	m_palettes[name] = Palette_t();
	emitMessage(MessageDispatch::MessageBase(Messages::PALETTES_CHANGED));
}

//-----------------------------------------------------------------

void GodClientData::deletePalette(const std::string& name)
{
	IGNORE_RETURN(m_palettes.erase(name));
	emitMessage(MessageDispatch::MessageBase(Messages::PALETTES_CHANGED));
}

//-----------------------------------------------------------------

void GodClientData::addTemplateToPalette(const std::string& templateName)
{
	Palette_t palette = m_palettes[getSelectedPalette()];

	//add item to palette
	palette.push_back(templateName);
	m_palettes[getSelectedPalette()] = palette;
	emitMessage(MessageDispatch::MessageBase(Messages::PALETTES_CHANGED));
}

//-----------------------------------------------------------------

void GodClientData::setSelectedPalette(const std::string& name)
{
	m_selectedPalette = name;
}

//-----------------------------------------------------------------

size_t GodClientData::getNumPalettes() const
{
	return m_palettes.size();
}

//-----------------------------------------------------------------

GodClientData::Palette_t GodClientData::getPalette(int index, std::string& name)
{
	PaletteMap_t::iterator it = m_palettes.begin();
	for(int i = 0; it != m_palettes.end() && i < index; ++i, ++it)
		;
	name = it->first;
	return it->second;
}

//-----------------------------------------------------------------

const std::string& GodClientData::getSelectedPalette() const
{
	return m_selectedPalette;
}

//-----------------------------------------------------------------

void GodClientData::getSphereTreeSnapshot()
{
	unsigned int reqnum = ServerCommander::getInstance().getSphereTreeSnapshot();
	IGNORE_RETURN(m_outstandingRequests.insert(reqnum));
}

//-----------------------------------------------------------------

/**
* Get the trigger volumes for the currently selected objects
*/
void GodClientData::showTriggerVolumes()
{
	m_triggerObjectSpheres.clear();
	cleanSelectedObjectList();
	for(SelectedObjectList_t::iterator it = m_selectedObjects.begin(); it != m_selectedObjects.end(); ++it) 
	{
		ClientObject* const obj = NON_NULL((*it)->obj.getPointer()); 
		unsigned int reqnum = ServerCommander::getInstance().getTriggerVolumes(obj->getNetworkId());
		IGNORE_RETURN(m_outstandingRequests.insert(reqnum));
	}
}

//-----------------------------------------------------------------

/**
*/
void GodClientData::clearSpheres()
{
	m_triggerObjectSpheres.clear();
	m_sphereTreeObjects.clear();
}

//-----------------------------------------------------------------

Vector GodClientData::snapToGrid(const Vector& originalLoc) const
{
	real newX = originalLoc.x;
	real newY = originalLoc.y;
	real newZ = originalLoc.z;

	if(m_snapToHorizontalGrid)
	{
		newX = floor(originalLoc.x / m_snapToGridHorizontalSize)* m_snapToGridHorizontalSize;
		newZ = floor(originalLoc.z / m_snapToGridHorizontalSize)* m_snapToGridHorizontalSize;
	}

	if(m_snapToVerticalGrid)
	{
		newX = floor(originalLoc.x / m_snapToGridVerticalSize)* m_snapToGridVerticalSize;
		newY = floor(originalLoc.y / m_snapToGridVerticalSize)* m_snapToGridVerticalSize;
	}
	return Vector(newX, newY, newZ);
}

//-----------------------------------------------------------------

/**
 * Show dialog box that allows the user to view and change object transform properties
 */
void GodClientData::setTransformDlg()
{
	ObjectTransformWindow* transformDlg = new ObjectTransformWindow(&MainFrame::getInstance(), "Object Transform", true);

	if(m_selectedObjects.empty())
	{
		IGNORE_RETURN(QMessageBox::warning(&MainFrame::getInstance(), "Warning", "No objects selected, can't transform"));
		return;
	}

	bool xVaries       = false;
	bool yVaries       = false;
	bool zVaries       = false;
	bool xAssigned     = false;
	bool yAssigned     = false;
	bool zAssigned     = false;
	bool yawVaries     = false;
	bool pitchVaries   = false;
	bool rollVaries    = false;
	bool yawAssigned   = false;
	bool pitchAssigned = false;
	bool rollAssigned  = false;

	real originalX     = 0.0;
	real originalY     = 0.0;
	real originalZ     = 0.0;
	real originalYaw   = 0.0;
	real originalPitch = 0.0;
	real originalRoll  = 0.0;

	//iterate over all selected objects, determine if they share any axes(in which case we fill those values in the dlg, if they vary we leave them blank)
	for(SelectedObjectList_t::iterator it = m_selectedObjects.begin(); it != m_selectedObjects.end(); ++it) 
	{
		SelectedObject*     selObj = NON_NULL(*it);
		const Vector           pos = selObj->obj->getPosition_p();

		if(pos.x != originalX) //lint !e777 test floats for equality, we want to know if the value hasn't changed yet, and is *exactly* 0.0f
			if(!xAssigned && !xVaries)
				originalX = pos.x;
			else
				xVaries = true;
		xAssigned = true;

		if(pos.y != originalY) //lint !e777 test floats for equality, we want to know if the value hasn't changed yet, and is *exactly* 0.0f
			if(!yAssigned && !yVaries)
				originalY = pos.y;
			else
				yVaries = true;
		yAssigned = true;

		if(pos.z != originalZ) //lint !e777 test floats for equality, we want to know if the value hasn't changed yet, and is *exactly* 0.0f
			if(!zAssigned && !zVaries)
				originalZ = pos.z;
			else
				zVaries = true;
		zAssigned = true;

		if(selObj->obj->getObjectFrameK_w().phi() != originalYaw) //lint !e777 test floats for equality, we want to know if the value hasn't changed yet, and is *exactly* 0.0f
			if(!yawAssigned && !yawVaries)
				originalYaw = selObj->obj->getObjectFrameK_w().phi();
			else
				yawVaries = true;
		yawAssigned = true;

		if(selObj->obj->getObjectFrameK_w().theta() != originalPitch) //lint !e777 test floats for equality, we want to know if the value hasn't changed yet, and is *exactly* 0.0f
			if(!pitchAssigned && !pitchVaries)
				originalPitch = selObj->obj->getObjectFrameK_w().theta();
			else
				pitchVaries = true;
		pitchAssigned = true;

		if(selObj->obj->getObjectFrameJ_w().phi() != originalRoll) //lint !e777 test floats for equality, we want to know if the value hasn't changed yet, and is *exactly* 0.0f
			if(!rollAssigned && !rollVaries)
				originalRoll = selObj->obj->getObjectFrameJ_w().phi();
			else
				rollVaries = true;
		rollAssigned = true;
	}

	//set the position values into the dlg, only show values that all selected items have in common
	QString xText;
	IGNORE_RETURN(xText.setNum(originalX));
	QString yText;
	IGNORE_RETURN(yText.setNum(originalY));
	QString zText;
	IGNORE_RETURN(zText.setNum(originalZ));
	if(xAssigned && !xVaries)
		transformDlg->m_LineEditX->setText(xText);
	if(yAssigned && !yVaries)
		transformDlg->m_LineEditY->setText(yText);
	if(zAssigned && !zVaries)
		transformDlg->m_LineEditZ->setText(zText);

	//set the rotation values into the dlg, only show values that all selected items have in common
	if(yawAssigned && !yawVaries)
		transformDlg->m_yawEdit->setValue(static_cast<int>(originalYaw));
	if(pitchAssigned && !pitchVaries)
		transformDlg->m_pitchEdit->setValue(static_cast<int>(originalPitch));
	if(rollAssigned && !rollVaries)
		transformDlg->m_rollEdit->setValue(static_cast<int>(originalRoll));

	//run the dialog box, only commit changes if they hit OK
	int result = transformDlg->exec();
	if(result != QDialog::Accepted)
		return;

	//grab the new loc
	xText = transformDlg->m_LineEditX->text();
	yText = transformDlg->m_LineEditY->text();
	zText = transformDlg->m_LineEditZ->text();
	real xValue = xText.toFloat();
	real yValue = yText.toFloat();
	real zValue = zText.toFloat();

	//grab the new rotation
	const QString yawText   = transformDlg->m_pitchEdit->text();
	real newYawDegrees   = transformDlg->m_yawEdit->value();
	const QString pitchText = transformDlg->m_pitchEdit->text();
	real newPitchDegrees = transformDlg->m_pitchEdit->value();
	const QString rollText  = transformDlg->m_pitchEdit->text();
	real newRollDegrees  = transformDlg->m_rollEdit->value();
	real newYawRadians   = convertDegreesToRadians(static_cast<real>(newYawDegrees));
	real newPitchRadians = convertDegreesToRadians(static_cast<real>(newPitchDegrees));
	real newRollRadians  = convertDegreesToRadians(static_cast<real>(newRollDegrees));

	//set the new values
	if(xText != "")
		setSelectionX(xValue);
	if(yText != "")
		setSelectionY(yValue);
	if(zText != "")
		setSelectionZ(zValue);
	if(yawText != "")
		setSelectionYaw(newYawRadians);
	if(pitchText != "")
		setSelectionPitch(newPitchRadians);
	if(rollText != "")
		setSelectionRoll(newRollRadians);
}

//-----------------------------------------------------------------

/**
 * Show dialog box that allows the user to view and change snap to grid properties
 */
void GodClientData::snapToGridDlg()
{
	SnapToGridSettings* snapToGridWindow = new SnapToGridSettings(&MainFrame::getInstance(), "Snap to Grid Settings");
	
	//set the current grid values into the dlg
	snapToGridWindow->m_snapToHorizontalGrid->setChecked(m_snapToHorizontalGrid);
	snapToGridWindow->m_snapToVerticalGrid->setChecked(m_snapToVerticalGrid);
	snapToGridWindow->m_horizontalSensitivitySpinBox->setValue(m_snapToGridHorizontalThreshold);
	snapToGridWindow->m_verticalSensitivitySpinBox->setValue(m_snapToGridVerticalThreshold);
	QString text;
	IGNORE_RETURN(text.setNum(m_snapToGridHorizontalSize));
	snapToGridWindow->m_horizontalGridSize->setText(text);
	IGNORE_RETURN(text.setNum(m_snapToGridVerticalSize));
	snapToGridWindow->m_verticalGridSize->setText(text);
	IGNORE_RETURN(text.setNum(m_snapToGridLinesPerHorizontalSide));
	snapToGridWindow->m_horizontalGridSegments->setText(text);
	IGNORE_RETURN(text.setNum(m_snapToGridLinesPerVerticalSide));
	snapToGridWindow->m_verticalGridSegments->setText(text);

	//run the dialog box, only commit changes if they hit OK
	int result = snapToGridWindow->exec();
	if(result != QDialog::Accepted)
		return;

	//get the new values from the dlg, push into the game system
	m_snapToHorizontalGrid             = snapToGridWindow->m_snapToHorizontalGrid->isChecked();
	m_snapToVerticalGrid               = snapToGridWindow->m_snapToVerticalGrid->isChecked();
	m_snapToGridHorizontalThreshold    = snapToGridWindow->m_horizontalSensitivitySpinBox->value();
	m_snapToGridVerticalThreshold      = snapToGridWindow->m_verticalSensitivitySpinBox->value();
	m_snapToGridHorizontalSize         = snapToGridWindow->m_horizontalGridSize->text().toFloat();
	m_snapToGridVerticalSize           = snapToGridWindow->m_verticalGridSize->text().toFloat();
	m_snapToGridLinesPerHorizontalSide = snapToGridWindow->m_horizontalGridSegments->text().toInt();
	m_snapToGridLinesPerVerticalSide   = snapToGridWindow->m_verticalGridSegments->text().toInt();

	//precompute for speed
	m_snapToGridHorizontalLineLength   = m_snapToGridLinesPerHorizontalSide * m_snapToGridHorizontalSize;
	m_snapToGridVerticalLineLength     = m_snapToGridLinesPerVerticalSide   * m_snapToGridVerticalSize;
}

//-----------------------------------------------------------------

/**
 * Move all selected objects to the new X value
 */
void GodClientData::setSelectionX(real newX)
{
	m_createdCount = 0;
	for(SelectedObjectList_t::iterator itr = m_selectedObjects.begin(); itr != m_selectedObjects.end(); ++itr) 
	{
		SelectedObject* selObj = NON_NULL(*itr);

		//create the ghost if needed
		if(!selObj->ghost)
			selObj->ghost = NON_NULL(createGhost(*selObj->obj));

		const Vector originalPos = selObj->ghost->getPosition_p();
	
		//move ghost to new position
		selObj->ghost->setPosition_p(Vector(newX, originalPos.y, originalPos.z));
	}

	//if we created any new ghosts, emit the appropriate messages
	if(m_createdCount)
		emitMessage(MessageDispatch::MessageBase(Messages::GHOSTS_CREATED));

	emitMessage(MessageDispatch::MessageBase(GodClientData::Messages::SELECTED_OBJECTS_CHANGED));
}

//-----------------------------------------------------------------

/**
 * Move all selected objects to the new Y value
 */
void GodClientData::setSelectionY(real newY)
{
	m_createdCount = 0;
	for(SelectedObjectList_t::iterator itr = m_selectedObjects.begin(); itr != m_selectedObjects.end(); ++itr) 
	{
		SelectedObject* selObj = NON_NULL(*itr);

		//create the ghost if needed
		if(!selObj->ghost)
			selObj->ghost = NON_NULL(createGhost(*selObj->obj));

		const Vector originalPos = selObj->ghost->getPosition_p();

		//move ghost to new position
		selObj->ghost->setPosition_p(Vector(originalPos.x, newY, originalPos.z));
	}

	//if we created any new ghosts, emit the appropriate messages
	if(m_createdCount)
		emitMessage(MessageDispatch::MessageBase(Messages::GHOSTS_CREATED));

	emitMessage(MessageDispatch::MessageBase(GodClientData::Messages::SELECTED_OBJECTS_CHANGED));
}

//-----------------------------------------------------------------

/**
 * Move all selected objects to the new Z value
 */
void GodClientData::setSelectionZ(real newZ)
{
	m_createdCount = 0;
	for(SelectedObjectList_t::iterator itr = m_selectedObjects.begin(); itr != m_selectedObjects.end(); ++itr) 
	{
		SelectedObject* selObj = NON_NULL(*itr);

		//create the ghost if needed
		if(!selObj->ghost)
			selObj->ghost = NON_NULL(createGhost(*selObj->obj));

		const Vector originalPos = selObj->ghost->getPosition_p();
	
		//move ghost to new position
		selObj->ghost->setPosition_p(Vector(originalPos.x, originalPos.y, newZ));
	}

	//if we created any new ghosts, emit the appropriate messages
	if(m_createdCount)
		emitMessage(MessageDispatch::MessageBase(Messages::GHOSTS_CREATED));

	emitMessage(MessageDispatch::MessageBase(GodClientData::Messages::SELECTED_OBJECTS_CHANGED));
}

//-----------------------------------------------------------------

/**
 * Rotate all selected objects to the new yaw value
 */
void GodClientData::setSelectionYaw(real newYaw)
{
	m_createdCount = 0;
	for(SelectedObjectList_t::iterator itr = m_selectedObjects.begin(); itr != m_selectedObjects.end(); ++itr) 
	{
		SelectedObject* selObj = NON_NULL(*itr);

		//create the ghost if needed
		if(!selObj->ghost)
			selObj->ghost = NON_NULL(createGhost(*selObj->obj));

		//set the yaw
		selObj->ghost->yaw_o(newYaw);
	}

	//if we created any new ghosts, emit the appropriate messages
	if(m_createdCount)
		emitMessage(MessageDispatch::MessageBase(Messages::GHOSTS_CREATED));

	emitMessage(MessageDispatch::MessageBase(GodClientData::Messages::SELECTED_OBJECTS_CHANGED));
}

//-----------------------------------------------------------------

/**
 * Rotate all selected objects to the new pitch value
 */
void GodClientData::setSelectionPitch(real newPitch)
{
	m_createdCount = 0;
	for(SelectedObjectList_t::iterator itr = m_selectedObjects.begin(); itr != m_selectedObjects.end(); ++itr) 
	{
		SelectedObject* selObj = NON_NULL(*itr);

		//create the ghost if needed
		if(!selObj->ghost)
			selObj->ghost = NON_NULL(createGhost(*selObj->obj));

		//set the pitch
		selObj->ghost->pitch_o(newPitch);
	}

	//if we created any new ghosts, emit the appropriate messages
	if(m_createdCount)
		emitMessage(MessageDispatch::MessageBase(Messages::GHOSTS_CREATED));

	emitMessage(MessageDispatch::MessageBase(GodClientData::Messages::SELECTED_OBJECTS_CHANGED));
}

//-----------------------------------------------------------------

/**
 * Rotate all selected objects to the new pitch value
 */
void GodClientData::setSelectionRoll(real newRoll)
{
	m_createdCount = 0;
	for(SelectedObjectList_t::iterator itr = m_selectedObjects.begin(); itr != m_selectedObjects.end(); ++itr) 
	{
		SelectedObject* selObj = NON_NULL(*itr);

		//create the ghost if needed
		if(!selObj->ghost)
			selObj->ghost = NON_NULL(createGhost(*selObj->obj));

		//set the roll
		selObj->ghost->roll_o(newRoll);
	}

	//if we created any new ghosts, emit the appropriate messages
	if(m_createdCount)
		emitMessage(MessageDispatch::MessageBase(Messages::GHOSTS_CREATED));

	emitMessage(MessageDispatch::MessageBase(GodClientData::Messages::SELECTED_OBJECTS_CHANGED));
}

//-----------------------------------------------------------------

void GodClientData::enableShowObjectAxes(const ClientObject* const obj)
{
		m_showAxesObjects.push_back(obj);
}

//-----------------------------------------------------------------

void GodClientData::disableShowObjectAxes(const ClientObject* const obj)
{
	m_showAxesObjects.remove(obj);
}

//-----------------------------------------------------------------

/**
 * Turn on/off an x/y/z RGB frame for all selected objects
 */
void GodClientData::toggleShowObjectAxes(const ClientObject* const obj)
{
	DEBUG_FATAL(!obj, ("toggleShowObjectAxes received a NULL for obj"));
	ObjectWithAxes_t::iterator i = std::find(m_showAxesObjects.begin(), m_showAxesObjects.end(), obj);
	//if it isn't in the list, add it
	if(i == m_showAxesObjects.end())
	{
		enableShowObjectAxes(obj);
	}
	//else remove it from the list
	else
	{
		disableShowObjectAxes(obj);
	}
}

//-----------------------------------------------------------------

void GodClientData::pauseParticleSystem(ParticleEffectAppearance* particleAppearance) const
{
	particleAppearance->setPaused(true);
}

//-----------------------------------------------------------------

void GodClientData::unpauseParticleSystem(ParticleEffectAppearance* particleAppearance) const
{
	particleAppearance->setPaused(false);
}

//-----------------------------------------------------------------

/**
 * Toggle the pause state of any selected particle systems
 */
void GodClientData::togglePauseParticleSystem(ClientObject* const obj)
{
	DEBUG_FATAL(!obj, ("toggleShowObjectAxes received a NULL for obj"));

	ParticleEffectAppearance* particleAppearance = ParticleEffectAppearance::asParticleEffectAppearance(obj->getAppearance());

	if(!particleAppearance->isPaused())
	{
		pauseParticleSystem(particleAppearance);
	}
	else
	{
		unpauseParticleSystem(particleAppearance);
	}
}

//-----------------------------------------------------------------

/**
 * Search all selected objects for a particle system
 */
bool GodClientData::isParticleSystemSelected()
{
	for(SelectedObjectList_t::iterator i = m_selectedObjects.begin(); i != m_selectedObjects.end(); ++i)
	{
		const ParticleEffectAppearance* const app = dynamic_cast<const ParticleEffectAppearance* const>((*i)->obj->getAppearance());
		if(app)
			return true;
	}
	return false;
}

//-------------------------------------------------------------

bool GodClientData::isBuildingSelected(const Object** result) const
{
	if (m_selectedObjects.size () == 1)
	{
		const Object* const object = m_selectedObjects.front ()->obj;
		if (object && object->getObjectTemplate () && object->getObjectTemplate ()->getId () == SharedBuildingObjectTemplate::SharedBuildingObjectTemplate_tag)
		{
			if (result)
				*result = object;

			return true;
		}
	}

	return false;
}

//-------------------------------------------------------------

/**
 * Remove any NULL items from the list (since they're watchers, they could go NULL at any time)
 */
void GodClientData::cleanSelectedObjectList()
{
	bool allDone = false;
	while(!allDone)
	{
		bool iteratorsInvalidated = false;
		for(SelectedObjectList_t::iterator i = m_selectedObjects.begin(); i != m_selectedObjects.end(); ++i)
		{
			if((*i)->obj.getPointer() == NULL)
			{
				SelectedObject * const s = *i;
				m_selectedObjects.remove(s);
				delete s;
				iteratorsInvalidated = true;
				break;
			}
		}
		if(!iteratorsInvalidated)
			allDone = true;
	}
}

//-------------------------------------------------------------

void GodClientData::setMouseCursorIntersection(const Vector& point)
{
	m_mouseCursorIntersection = point;
}

//-------------------------------------------------------------

void GodClientData::drawCrossbars(const Vector& point, float barSize, const VectorArgb& color)
{
	const Vector crossBars [4] =
	{
		Vector(point.x - barSize, point.y, point.z),
		Vector(point.x + barSize, point.y, point.z),
		Vector(point.x,           point.y, point.z - barSize),
		Vector(point.x,           point.y, point.z + barSize)
	};

	Graphics::drawLine(crossBars [0], crossBars [1], color);
	Graphics::drawLine(crossBars [2], crossBars [3], color);
}

//-------------------------------------------------------------

void GodClientData::setRenderAIPaths(bool i_render)
{
	m_renderAIPaths = i_render;
	if (!i_render)
	{
		m_AINodes.clear();
	}
}

//-------------------------------------------------------------

void GodClientData::ignoreTargetPath(const NetworkId &object)
{
	m_AIPaths.erase(object);
}

//-------------------------------------------------------------

const Vector &   GodClientData::getCameraPivotPoint ()
{
	return m_pivotPoint;
}

//-------------------------------------------------------------

void GodClientData::toggleDropToTerrain()
{
	m_toggleDropToTerrainOn = !m_toggleDropToTerrainOn;
}

//-------------------------------------------------------------

bool GodClientData::isToggleDropToTerrainOn() const
{
	return m_toggleDropToTerrainOn;
}

//-------------------------------------------------------------

void GodClientData::toggleAlignToTerrain()
{
	m_toggleAlignToTerrainOn = !m_toggleAlignToTerrainOn;
}

//-------------------------------------------------------------

bool GodClientData::isToggleAlignToTerrainOn() const
{
	return m_toggleAlignToTerrainOn;
}

//-------------------------------------------------------------
