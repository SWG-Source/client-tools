// ======================================================================
//
// SwgCuiSurvey.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef SwgCuiSurvey_H
#define SwgCuiSurvey_H

//-----------------------------------------------------------------

#include "sharedMessageDispatch/Receiver.h"

#include "clientUserInterface/CuiMediator.h"

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/NetworkId.h"
#include "swgSharedNetworkMessages/SurveyMessage.h"

#include "UITypes.h"
#include "UIEventCallback.h"
#include "UINotification.h"

//-----------------------------------------------------------------

class ClientObject;
class ClientProceduralTerrainAppearance;
class CuiWidget3dObjectListViewer;
class CuiWidgetGroundRadar;
class ResourceListForSurveyMessage;
class SurveyMessage;
class Shader;
class UIDataSource;
class UIDataSourceContainer;
class UIList;
class UITable;
class UIText;
class UIWidget;
class UITreeView;
class Vector;
class Waypoint;

//-----------------------------------------------------------------

namespace MessageDispatch
{
	class Callback;
}

//-----------------------------------------------------------------

class SwgCuiSurvey : public CuiMediator, public UIEventCallback, public UINotification, public MessageDispatch::Receiver
{
public:
	static SwgCuiSurvey*          createInto            (UIPage & parent);

public:
	explicit                      SwgCuiSurvey          (UIPage & page);
	void                          Notify                (UINotificationServer *notifyingObject, UIBaseObject *contextObject, Code notificationCode);
	void                          receiveMessage        (const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message);
	void                          setResourceData       (const ResourceListForSurveyMessage& message);
	void                          setSurveyData         (const SurveyMessage& message);
	void                          updateMap             (const ClientProceduralTerrainAppearance & cmtat);
	NetworkId                     getResourceAtPosition (int index);
	CuiWidget3dObjectListViewer * getResourceIconAtPosition       (int index);
	bool                          hasSurvey                       () const;
	int                           getNumResources                 () const;
	bool                          OnMessage                       (UIWidget *Context, const UIMessage & msg);
	void                          update                          (float deltaTimeSecs);

protected:
	void                       performActivate       ();
	void                       performDeactivate     ();
	bool                       close                 ();
	void                       OnButtonPressed           (UIWidget *context);
	void                       OnGenericSelectionChanged (UIWidget * context);

private:
	                          ~SwgCuiSurvey          ();
	                           SwgCuiSurvey          (const SwgCuiSurvey &);
	SwgCuiSurvey&              operator=             (const SwgCuiSurvey &);

	void                       clearPoints           ();
	Vector                     convertUIPointToMapSpace(const UIPoint& uiLoc) const;
	UIPoint                    convertMapSpacePointToUIPoint(const Vector & worldLoc) const;
	void                       addSurveyIcon(const Vector& loc, float efficiency);
	int                        getRange(const stdvector<SurveyMessage::DataItem>::fwd& data) const;
	void                       setHere();
	Vector                     getDataCenterPoint(const stdvector<SurveyMessage::DataItem>::fwd& data) const;
	std::pair<float, float>    getDataEfficiencyRange(const stdvector<SurveyMessage::DataItem>::fwd& data) const;

private:
	UIText*                    m_resourceType;
	UIButton*                  m_surveyButton;
	UIButton*                  m_getCoreSampleButton;
	int                        m_lastMapUpdate;
	bool                       m_mapDirty;
	NetworkId                  m_surveyTool;
	Unicode::String            m_resourceName;
	UIPage*                    m_mapBack;
	UIPage*                    m_sampleIcon;
	UIPage*                    m_hereIcon;
	UIText*                    m_rangeText;
	UIScalar                   m_mapWidth;
	UIScalar                   m_mapHeight;
	float                      m_minEfficiency;
	float                      m_maxEfficiency;
	int                        m_range;
	bool                       m_hasSurvey;
	Waypoint*                  m_waypoint;
	UITreeView*                m_resourceTree;
	CuiWidget3dObjectListViewer* m_viewer;

	stdvector<NetworkId>::fwd                      m_resources;
	CuiWidgetGroundRadar*                          m_groundMap;
	Vector                                         m_surveyLoc;
	stdvector<CuiWidget3dObjectListViewer*>::fwd   m_resourceIcons;
	stdvector<UIPage*>::fwd                        m_icons;
	stdmap<Unicode::String, UIDataSourceContainer*>::fwd m_resourceTreeMapping;
//	stdmap<NetworkId, CuiWidget3dObjectListViewer*>::fwd m_iconMap;
};

//-----------------------------------------------------------------

#endif

