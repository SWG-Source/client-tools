// ======================================================================
//
// SwgCuiSurvey.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiSurvey.h"

#include "sharedGame/Command.h"
#include "sharedGame/CommandTable.h"
#include "sharedGame/ResourceClassObject.h"
#include "sharedMath/Vector2d.h"
#include "sharedMessageDispatch/Emitter.h"

#include "clientGame/ClientCommandQueue.h"
#include "clientGame/ClientObject.h"
#include "clientGame/Game.h"
#include "clientGame/ResourceIconManager.h"
#include "clientGame/ResourceTypeManager.h"
#include "clientGame/ClientWaypointObject.h"
#include "clientGraphics/Graphics.h"
#include "clientTerrain/ClientProceduralTerrainAppearance.h"
#include "clientUserInterface/CuiIconManager.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiStringIds.h"
#include "clientUserInterface/CuiSurveyManager.h"
#include "clientUserInterface/CuiSystemMessageManager.h"
#include "clientUserInterface/CuiWidget3dObjectListViewer.h"
#include "clientUserInterface/CuiWidgetGroundRadar.h"

#include "swgSharedNetworkMessages/ResourceListForSurveyMessage.h"

#include "UIButton.h"
#include "UIData.h"
#include "UIDatasource.h"
#include "UIDataSourceContainer.h"
#include "UIImage.h"
#include "UIList.h"
#include "UIMessage.h"
#include "UIPage.h"
#include "UIText.h"
#include "UITreeView.h"

#include <algorithm>
#include <cstdio>

//-----------------------------------------------------------------------

namespace SwgCuiSurveyNamespace
{
	const float thresholdMin  = 0.1f;

	namespace DataProperties
	{
		namespace TypeTree
		{
			const UILowerString Text       = UILowerString ("Text");
			const UILowerString Selectable = UILowerString ("Selectable");
			const UILowerString ColorIndex = UILowerString ("ColorIndex");
		}
	}

	const float mapBuffer = 1.333333f;
}

using namespace SwgCuiSurveyNamespace;

//-----------------------------------------------------------------------

SwgCuiSurvey::SwgCuiSurvey  (UIPage & page)
: CuiMediator               ("SwgCuiSurvey", page),
  UIEventCallback           (),
  UINotification            (),
  MessageDispatch::Receiver (),
  m_resourceType            (0),
  m_surveyButton            (0),
  m_getCoreSampleButton     (0),
  m_lastMapUpdate           (0),
  m_mapDirty                (false),
  m_surveyTool              (),
  m_resourceName            (),
  m_mapBack                 (0),
  m_sampleIcon              (0),
  m_hereIcon                (0),
  m_rangeText               (0),
  m_mapWidth                (0),
  m_mapHeight               (0),
  m_minEfficiency           (0.0f),
  m_maxEfficiency           (0.0f),
  m_range                   (0),
  m_hasSurvey               (false),
  m_waypoint                (0),
  m_resourceTree            (0),
  m_resources               (),
  m_groundMap               (0),
  m_surveyLoc               (),
  m_resourceIcons           (),
  m_icons                   ()
{
	IGNORE_RETURN(setState    (MS_closeable));
	IGNORE_RETURN(setState    (MS_closeDeactivates));
	IGNORE_RETURN(removeState (MS_iconifiable));
	getCodeDataObject (TUIText,   m_resourceType,        "resourcetype");
	getCodeDataObject (TUIButton, m_surveyButton,        "buttonSurvey");
	getCodeDataObject (TUIButton, m_getCoreSampleButton, "buttonGetSample");	

	getCodeDataObject (TUITreeView,  m_resourceTree, "resourcetree");
	m_resourceTree->ClearData();

	UIWidget * viewerWidget = 0;
	getCodeDataObject (TUIWidget, viewerWidget,     "viewer");
	m_viewer = safe_cast<CuiWidget3dObjectListViewer *>(viewerWidget);

	m_viewer->SetDragable             (false);
	m_viewer->SetContextCapable       (false, false);
	m_viewer->setRotateSpeed          (0.0f);

	getCodeDataObject (TUIPage,  m_mapBack, "mapback");
	m_mapWidth  = m_mapBack->GetWidth();
	m_mapHeight = m_mapBack->GetHeight();

	getCodeDataObject (TUIPage,  m_sampleIcon, "sampleIcon");
	getCodeDataObject (TUIPage,  m_hereIcon,   "hereIcon");
	getCodeDataObject (TUIText,  m_rangeText,  "textRange"); 
	
	UIWidget *widget = 0;
	getCodeDataObject (TUIWidget, widget, "map");
	m_groundMap = NON_NULL (dynamic_cast<CuiWidgetGroundRadar *>(widget));
	m_groundMap->setClipToCircle(false);
	m_groundMap->setDrawBackground(false);

	m_rangeText->Clear();

	setIsUpdating(true);
}

//-----------------------------------------------------------------------

SwgCuiSurvey::~SwgCuiSurvey ()
{
	m_groundMap = 0;
	m_mapBack   = 0;
	m_resourceTree = 0;
	m_waypoint = 0;

	std::for_each(m_resourceIcons.begin(), m_resourceIcons.end(), UIBaseObject::DetachFunctor(0));
	m_resourceIcons.clear();
}

//-----------------------------------------------------------------------

void SwgCuiSurvey::receiveMessage(const MessageDispatch::Emitter &, const MessageDispatch::MessageBase &)
{
}

//-----------------------------------------------------------------------

void SwgCuiSurvey::performActivate ()
{
	CuiManager::requestPointer    (true);
	m_surveyButton->AddCallback        (this);
	m_getCoreSampleButton->AddCallback (this);
	m_mapBack->AddCallback             (this);

	m_resourceTree->AddCallback        (this);
}

//-----------------------------------------------------------------------

void SwgCuiSurvey::performDeactivate ()
{
	m_resourceTree->RemoveCallback  (this);
	m_mapBack->RemoveCallback       (this);

	CuiManager::requestPointer (false);
	m_surveyButton->RemoveCallback  (this);
	m_getCoreSampleButton->RemoveCallback  (this);
}

//-----------------------------------------------------------------------

bool SwgCuiSurvey::close ()
{
	deactivate ();
	return true;
}

//-----------------------------------------------------------------------

void SwgCuiSurvey::update (float deltaTimeSecs)
{
	CuiMediator::update (deltaTimeSecs);

	if (hasSurvey() && Game::getPlayer())
	{
		setHere();
	}
}

//-----------------------------------------------------------------------

void SwgCuiSurvey::OnButtonPressed(UIWidget *context)
{
	if (context == m_surveyButton)
	{
		if(m_resourceName.empty())
		{
			CuiMessageBox::createInfoBox (CuiStringIds::survey_select_resource.localize ());
			return;
		}
		else
		{
			clearPoints();
			CuiSurveyManager::setDefaultResourceForTool(m_surveyTool, m_resourceName);
			IGNORE_RETURN(ClientCommandQueue::enqueueCommand("requestSurvey", m_surveyTool, m_resourceName));
		}
	}

	if (context == m_getCoreSampleButton)
	{
		if(m_resourceName.empty())
		{
			CuiMessageBox::createInfoBox (CuiStringIds::survey_select_resource.localize ());
			return;
		}
		else
		{
			CuiSurveyManager::setDefaultResourceForTool(m_surveyTool, m_resourceName);
			IGNORE_RETURN(ClientCommandQueue::enqueueCommand("requestCoreSample", m_surveyTool, m_resourceName));
		}
	}
}

//-----------------------------------------------------------------------

void SwgCuiSurvey::OnGenericSelectionChanged (UIWidget * context)
{
	if(context == m_resourceTree)
	{
		long val = m_resourceTree->GetLastSelectedRow();
		if(val != -1)
		{
			UIDataSourceContainer * dsc = m_resourceTree->GetDataSourceContainerAtRow(val);
			if(dsc)
			{
				CuiSurveyManager::setDefaultResourceForTool(m_surveyTool, Unicode::narrowToWide(dsc->GetName()));
				m_resourceName = Unicode::narrowToWide(dsc->GetName());
				NetworkId res = ResourceTypeManager::findTypeByName(Unicode::narrowToWide(dsc->GetName()));
				std::string parentClass;
				ResourceTypeManager::findTypeParent(res, parentClass);
				if(!parentClass.empty())
				{
					Object* obj = ResourceIconManager::getObjectForClass(parentClass);
					m_viewer->setObject(obj);
					m_viewer->setViewDirty             (true);
					m_viewer->setCameraForceTarget     (true);
					m_viewer->recomputeZoom            ();
					m_viewer->setCameraForceTarget     (false);
					m_viewer->setViewDirty             (true);
				}
				else
					m_viewer->setObject(NULL);
			}
		}
	}
}

//-----------------------------------------------------------------------

bool SwgCuiSurvey::OnMessage(UIWidget *context, const UIMessage & msg)
{
	if (msg.Type == UIMessage::LeftMouseDoubleClick)
	{
		if(context == m_resourceTree)
			m_surveyButton->Press();
	}

	return true;
}

//-----------------------------------------------------------------------

void SwgCuiSurvey::Notify(UINotificationServer *, UIBaseObject *, Code)
{
}

//-----------------------------------------------------------------------

SwgCuiSurvey* SwgCuiSurvey::createInto (UIPage & parent)
{
	UIPage * const dupe = NON_NULL (UIPage::DuplicateInto (parent, "/Res.Surveying"));
	return new SwgCuiSurvey (*dupe);
}

//-----------------------------------------------------------------------

void SwgCuiSurvey::setResourceData (const ResourceListForSurveyMessage& message)
{
	m_resources.clear();

	std::for_each(m_resourceIcons.begin(), m_resourceIcons.end(), UIBaseObject::DetachFunctor(0));
	m_resourceIcons.clear();

	m_resourceTreeMapping.clear();

	m_resourceType->SetText(Unicode::narrowToWide(message.getType()));

	UIDataSourceContainer * const dsc = m_resourceTree->GetDataSourceContainer ();
	NOT_NULL (dsc);

	dsc->Attach (0);
	m_resourceTree->SetDataSourceContainer (0);
	dsc->Clear ();

	const std::vector<ResourceListForSurveyMessage::DataItem>& resources = message.getData();
	m_surveyTool = message.getSurveyToolId();
	CuiSurveyManager::setCurrentTool(m_surveyTool);

	m_resourceName = CuiSurveyManager::getDefaultResourceForTool(m_surveyTool);
	
	UIDataSourceContainer* lastResource = NULL;
	for(std::vector<ResourceListForSurveyMessage::DataItem>::const_iterator i = resources.begin(); i != resources.end(); ++i)
	{
		ResourceListForSurveyMessage::DataItem d = (*i);
		ResourceTypeManager::setTypeInfo(d.resourceId, Unicode::narrowToWide(d.resourceName), d.parentClassName);

		m_resources.push_back(d.resourceId);

		//find all the parents for the given resource
		std::vector<Unicode::String> resourceParents;
		resourceParents.push_back(Unicode::narrowToWide(d.resourceName));
		ResourceClassObject* o = ResourceTypeManager::findTypeParent(d.resourceId);
		while(o)
		{
			//don't show the root node of the resource tree
			if(o->getResourceClassName() == "resource")
				break;

			resourceParents.push_back(o->getFriendlyName().localize());
			o = o->getParent();
		}

		//now build all the tree items for the given resource
		UIDataSourceContainer * currentParent = dsc;
		while(!resourceParents.empty())
		{
			Unicode::String currentRes = resourceParents.back();
			resourceParents.pop_back();

			//see if the node already exists, use it if it does
			std::map<Unicode::String, UIDataSourceContainer*>::iterator i = m_resourceTreeMapping.find(currentRes);
			if(i != m_resourceTreeMapping.end())
			{
				currentParent = i->second;
				continue;
			}
			else
			{
				//create a new tree node
				UIDataSourceContainer * dsc_child = new UIDataSourceContainer;
				currentParent->AddChild          (dsc_child);
				//set the new item properties
				dsc_child->SetName     (Unicode::wideToNarrow(currentRes));
				dsc_child->SetProperty (DataProperties::TypeTree::Text, currentRes);
				dsc_child->SetProperty (DataProperties::TypeTree::Selectable, Unicode::narrowToWide("true"));
				dsc_child->SetProperty (DataProperties::TypeTree::ColorIndex, Unicode::narrowToWide("1"));
				//set parent properties (non-leaves are grey and non-selectable)
				currentParent->SetProperty (DataProperties::TypeTree::Selectable, Unicode::narrowToWide("false"));
				currentParent->SetProperty (DataProperties::TypeTree::ColorIndex, Unicode::narrowToWide("2"));
				//update loop values
				currentParent = dsc_child;
				m_resourceTreeMapping[currentRes] = dsc_child;

				if(currentRes == m_resourceName)
					lastResource = dsc_child;
			}
		}
	}
	m_resourceTree->SetDataSourceContainer (dsc);
	dsc->Detach (0);

	m_resourceTree->SetAllRowsExpanded(true);

	m_resourceTree->RemoveRowSelection(m_resourceTree->GetLastSelectedRow());

	OnGenericSelectionChanged(m_resourceTree);

	//select the last used resource
	if(lastResource)
	{
		int row = 0;
		if (m_resourceTree->FindDataNodeByDataSource (*lastResource, row))
		{
			m_resourceTree->SelectRow  (row);
			m_resourceTree->ScrollToRow(row);
		}
	}
}

//-----------------------------------------------------------------------

void SwgCuiSurvey::setSurveyData (const SurveyMessage& message)
{
	clearPoints();

	const std::vector<SurveyMessage::DataItem>& data = message.getData();
	int baseRange = getRange(data);
	m_range      = static_cast<int>(baseRange * mapBuffer);
	m_surveyLoc = getDataCenterPoint(data);

	std::pair<float, float> efficiencyRange = getDataEfficiencyRange(data);
	m_minEfficiency = efficiencyRange.first;
	m_maxEfficiency = efficiencyRange.second;

	DEBUG_FATAL(m_minEfficiency > m_maxEfficiency, ("Survey data is bad, minEfficiency %f is > than maxEfficiency %f", m_minEfficiency, m_maxEfficiency));

	char buf[64];
	sprintf(buf, "%dm", baseRange);
	m_rangeText->SetText(Unicode::narrowToWide(buf));
	
	bool foundSomePoint = false;
	
	for(std::vector<SurveyMessage::DataItem>::const_iterator i = data.begin(); i != data.end(); ++i)
	{
		if(i->m_efficiency > thresholdMin)
		{
			addSurveyIcon(i->m_location, i->m_efficiency);
			foundSomePoint = true;
		}
	}
	//if no area has enough efficiency to display, let the user know
	if(!foundSomePoint)
	{
		CuiSystemMessageManager::sendFakeSystemMessage (CuiStringIds::survey_nothingfound.localize());
	}
	m_hasSurvey = true;
	m_mapDirty = true;
}

//-----------------------------------------------------------------

int SwgCuiSurvey::getRange(const std::vector<SurveyMessage::DataItem>& data) const
{
	std::vector<SurveyMessage::DataItem>::const_iterator i = data.begin();

	float minX = i->m_location.x;
	float maxX = i->m_location.x;
	float minY = i->m_location.z;
	float maxY = i->m_location.z;
	
	for(; i != data.end(); ++i)
	{
		if(i->m_location.x < minX)
			minX = i->m_location.x;
		if(i->m_location.x > maxX)
			maxX = i->m_location.x;
		if(i->m_location.z < minY)
			minY = i->m_location.z;
		if(i->m_location.z > maxY)
			maxY = i->m_location.z;
	}

	int xRange = static_cast<int>(maxX - minX);
	int yRange = static_cast<int>(maxY - minY);

	if(xRange > yRange)
		return xRange;
	else
		return yRange;
}

//-----------------------------------------------------------------

void SwgCuiSurvey::addSurveyIcon(const Vector& loc, float efficiency)
{
	//duplicate our sample icon
	UIPage * const dupe   = safe_cast<UIPage *>(m_sampleIcon->DuplicateObject ());
	dupe->SetVisible(true);
	
	//find the UI point on the map on which to center this icon
	UIPoint pt = convertMapSpacePointToUIPoint(loc);
	
	//scale the icon to match efficency
	const int size = dupe->GetSize().x;
	UIText* text = NON_NULL(dynamic_cast<UIText *>(dupe->GetChild("text")));
	int intEfficiency = static_cast<int>(floor(efficiency*100));
	if(intEfficiency > 100)
		intEfficiency = 100;
	if(intEfficiency < 0)
		intEfficiency = 0;
	char buf[256];
	_itoa(intEfficiency, buf, 10);
	std::string intText = buf;
	intText += "%";
	text->SetText(Unicode::narrowToWide(intText.c_str()));

	//center the icon around the point, based on the new size
	const int newX = pt.x - size/2;
	const int newY = pt.y - size/2;
	dupe->SetLocation(UIPoint(newX, newY));

	DEBUG_FATAL(efficiency < m_minEfficiency, ("data point efficiency is less than min efficiency"));
	DEBUG_FATAL(efficiency > m_maxEfficiency, ("data point efficiency is greater than max efficiency"));

	//0 to 1 value is (eff - min) / (the range)
	const float rangeValue = (efficiency - m_minEfficiency) / (m_maxEfficiency - m_minEfficiency);

	UIColor c = UIColor::lerp (UIColor::cyan, UIColor::red, rangeValue);
	text->SetTextColor(c);

	//add it to the UI
	IGNORE_RETURN (m_mapBack->AddChild (dupe));
	dupe->Link();

	//add it to our maintenence list
	m_icons.push_back(dupe);
}

//-----------------------------------------------------------------

void SwgCuiSurvey::clearPoints()
{
	for(std::vector<UIPage*>::iterator i = m_icons.begin(); i != m_icons.end(); ++i)
	{
		(*i)->SetVisible(false);
	}
	
	m_icons.clear();
}

//-----------------------------------------------------------------

void  SwgCuiSurvey::updateMap (const ClientProceduralTerrainAppearance & cmtat)
{
	if(m_mapDirty)
	{
		DEBUG_FATAL(m_range == 0, ("Range is 0"));
		const float range = static_cast<float>(m_range);

		if (m_groundMap->updateRadarShader (m_surveyLoc, range, &cmtat, true))
			m_lastMapUpdate = Graphics::getFrameNumber ();

		char buf[32];
		IGNORE_RETURN(_snprintf (buf, 32,  "%1.0f m", range));

		m_mapDirty = false;
	}
}

//-----------------------------------------------------------------

NetworkId SwgCuiSurvey::getResourceAtPosition (int index)
{
	return m_resources[static_cast<unsigned int>(index)];
}

//-----------------------------------------------------------------

CuiWidget3dObjectListViewer * SwgCuiSurvey::getResourceIconAtPosition (int index)
{
	return m_resourceIcons[static_cast<unsigned int>(index)];
}

//-----------------------------------------------------------------

int SwgCuiSurvey::getNumResources () const
{
	return static_cast<int>(m_resources.size());
}

//-----------------------------------------------------------------

bool SwgCuiSurvey::hasSurvey() const
{
	return m_hasSurvey;
}

//-----------------------------------------------------------------

Vector SwgCuiSurvey::getDataCenterPoint(const std::vector<SurveyMessage::DataItem>& data) const
{
	float xSum = 0;
	float zSum = 0;
	float pointCount = 0;
	for(std::vector<SurveyMessage::DataItem>::const_iterator i = data.begin(); i != data.end(); ++i)
	{
		xSum += i->m_location.x;
		zSum += i->m_location.z;
		++pointCount;
	}

	const float xAvg = xSum / pointCount;
	const float zAvg = zSum / pointCount;
	return Vector(xAvg, 0.0f, zAvg);
}

//-----------------------------------------------------------------

std::pair<float, float> SwgCuiSurvey::getDataEfficiencyRange(const stdvector<SurveyMessage::DataItem>::fwd& data) const
{
	std::vector<SurveyMessage::DataItem>::const_iterator i = data.begin();

	float minEfficiency = i->m_efficiency;
	float maxEfficiency = i->m_efficiency;
	
	for(; i != data.end(); ++i)
	{
		if(i->m_efficiency < minEfficiency)
			minEfficiency = i->m_efficiency;
		if(i->m_efficiency > maxEfficiency)
			maxEfficiency = i->m_efficiency;
	}

	return std::make_pair(minEfficiency, maxEfficiency);
}

//-----------------------------------------------------------------

void SwgCuiSurvey::setHere()
{
	Object* player  = Game::getPlayer();
	Vector location = player->getPosition_w();
	UIPoint uiloc = convertMapSpacePointToUIPoint(location);

	//scale the icon to match efficency
	//center the icon around the point, based on the new size
	int newX = uiloc.x - m_hereIcon->GetSize().x;
	int newY = uiloc.y - m_hereIcon->GetSize().y;

	const UIPoint & mapSize  = m_groundMap->GetSize();
	const UIPoint & mapLoc   = m_groundMap->GetLocation();
	const UIPoint & iconSize = m_hereIcon->GetSize();
	if(newX < mapLoc.x)
		newX = mapLoc.x;
	else if(newX > mapLoc.x + mapSize.x - iconSize.x)
		newX = mapLoc.x + mapSize.x - iconSize.x;
	if(newY < mapLoc.y)
		newY = mapLoc.y;
	else if(newY > mapLoc.y + mapSize.y - iconSize.y)
		newY = mapLoc.y + mapSize.y - iconSize.y;

	m_hereIcon->SetLocation(UIPoint(newX, newY));

	//rotate icon to match player orientation
	const float radians = player->getObjectFrameK_w().theta();

	float rotation;
	if(radians >= 0)
		rotation = radians / (2 * PI);
	else
		rotation = 1 - (-(radians / (2 * PI)));

	char buf[256];
	IGNORE_RETURN (_gcvt(static_cast<double>(rotation), 3, buf));
	IGNORE_RETURN(m_hereIcon->SetProperty(UIWidget::PropertyName::Rotation, Unicode::narrowToWide(buf)));

	m_hereIcon->SetVisible(true);
}

//-----------------------------------------------------------------

Vector SwgCuiSurvey::convertUIPointToMapSpace(const UIPoint& uiLoc) const
{
	DEBUG_FATAL(m_range == 0, ("The range is 0"));
	const Vector2d upperLeftMapSpace((m_surveyLoc.x - m_range / 2.0f),
	                                 (m_surveyLoc.z + m_range / 2.0f));

	const float xProportion = static_cast<float>(uiLoc.x) / static_cast<float>(m_mapWidth);
	const float xMapSpace   = upperLeftMapSpace.x + (m_range * xProportion);
	const float yProportion = static_cast<float>(uiLoc.y) / static_cast<float>(m_mapHeight);
	const float yMapSpace   = upperLeftMapSpace.y - (m_range * yProportion);

	return Vector(xMapSpace, 0.0f, yMapSpace);
}

//-----------------------------------------------------------------

UIPoint SwgCuiSurvey::convertMapSpacePointToUIPoint(const Vector & worldLoc) const
{
	DEBUG_FATAL(m_range == 0, ("The range is 0"));
	const Vector2d upperLeftMapSpace((m_surveyLoc.x - m_range / 2.0f),
	                                 (m_surveyLoc.z + m_range / 2.0f));

	const float xProportion = static_cast<float>(worldLoc.x - upperLeftMapSpace.x) / static_cast<float>(m_range);
	const int   xUISpace    = static_cast<int>(m_mapWidth * xProportion);
	const float yProportion = -(static_cast<float>(worldLoc.z - upperLeftMapSpace.y) / static_cast<float>(m_range));
	const int   yUISpace    = static_cast<int>(m_mapHeight * yProportion);

	return UIPoint(xUISpace, yUISpace);
}

// ======================================================================
