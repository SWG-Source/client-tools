// ======================================================================
//
// CuiDataDrivenPage.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiDataDrivenPage.h"

#include "UIBaseObject.h"
#include "UIButton.h"
#include "UIClock.h"
#include "UIData.h"
#include "UIDataSource.h"
#include "UIDataSourceContainer.h"
#include "UIImageStyle.h"
#include "UILoader.h"
#include "UIManager.h"
#include "UIMessage.h"
#include "UIPage.h"
#include "UIString.h"
#include "UnicodeUtils.h"
#include "UITable.h"
#include "UITableModel.h"
#include "UITableTypes.def"
#include "UIUtils.h"
#include "CuiDataDrivenPageListener.h"
#include "clientAudio/Audio.h"
#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "clientGame/ProsePackageManagerClient.h"
#include "clientUserInterface/CuiDataDrivenPageManager.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiStringIds.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "clientUserInterface/CuiSystemMessageManager.h"
#include "clientUserInterface/CuiWorkspace.h"
#include "clientUserInterface/CuiWorkspaceIcon.h"
#include "sharedObject/CachedNetworkId.h"
#include "sharedGame/SuiCommand.h"
#include "sharedGame/SuiEventSubscription.h"
#include "sharedGame/SuiEventType.h"
#include "sharedGame/SuiPageData.h"
#include "sharedGame/SuiWidgetProperty.h"
#include "sharedFoundation/Os.h"
#include "sharedNetworkMessages/SuiEventNotification.h"
#include <algorithm>
#include <map>
#include <set>
#include <stdio.h>
#include <vector>

// ======================================================================

/** SortCuiDataDrivePage's by their widgetNames
 */
namespace CuiDataDrivenPageNamespace
{
	void Export(UIPage & page);
	UIString formatText(UIString & input);
}

using namespace CuiDataDrivenPageNamespace;

bool CuiDataDrivenPage::SubscribedProperty::operator <(const SubscribedProperty& rhs) const
{
	if(uiObjectName != rhs.uiObjectName)
		return uiObjectName < rhs.uiObjectName;
	else
		return propertyName < rhs.propertyName;
}

// ======================================================================

CuiDataDrivenPage::CuiDataDrivenPage(const std::string & name, UIPage& page, int clientPageId) :
CuiMediator             (name.c_str(), page),
m_subscribedProperties  (new SubscribedPropertyVector),
m_clientPageId          (clientPageId),
m_ok                    (false),
m_mediators             (0),
m_initializedWithPageData(false),
m_windowNameFound       (false),
m_nextEventSubscriptionIndex (0),
m_subscribedEvents      (0),
m_isClosed              (false),
m_autosave              (true),
m_pageSoundId           (SoundId::getInvalid()),
m_exportButton          (NULL)
{	
	UIButton* okButton;
	UIButton* cancelButton;	
	
	okButton     = page.FindDefaultButton(false);
	cancelButton = page.FindCancelButton(false);

	//-- attempt to construct any necessary child mediators

	const UIData * const mediatorData = dynamic_cast<UIData *>(page.GetChild ("MediatorData"));
	if (mediatorData)
	{
		UIBaseObject::UIPropertyNameVector props;
		mediatorData->GetPropertyNames (props, false);
		
		for (UIBaseObject::UIPropertyNameVector::const_iterator it = props.begin (); it != props.end (); ++it)
		{
			const UILowerString & lname = *it;

			if (lname != UIBaseObject::PropertyName::Name)
			{
				std::string value;
				if (mediatorData->GetPropertyNarrow (lname, value))
				{
					UIPage * const mediatorPage  = dynamic_cast<UIPage *>(mediatorData->GetObjectFromPath (value.c_str (), TUIPage));
					if (mediatorPage)
					{
						CuiMediator * const mediator = CuiMediatorFactory::create (lname.c_str (), *mediatorPage);
						if (mediator)
						{
							mediator->fetch ();
							if (!m_mediators)
								m_mediators = new MediatorVector;

							m_mediators->push_back (mediator);
						}
						else
							WARNING (true, ("unable to create specified mediator: %s", lname.c_str ()));
					}
					else
						WARNING (true, ("specified mediator page '%s' not found", value.c_str ()));
				}
			}
		}
	}

	CuiWorkspaceIcon * const icon = new CuiWorkspaceIcon (this);
	icon->SetName ("CuiDataDrivenPage");
	icon->SetSize (UISize (32, 32));
	icon->SetBackgroundColor (UIColor (0, 0, 0, 50));
	icon->SetLocation (0, 64);

	if(okButton)
		registerMediatorObject (*okButton,     true);
	if(cancelButton)
		registerMediatorObject (*cancelButton, true);

	m_exportButton = dynamic_cast<UIButton*>(page.GetObjectFromPath("btnExport"));
	if(m_exportButton)
		registerMediatorObject(*m_exportButton, true);

	IGNORE_RETURN(setState (MS_closeable));
	setIcon (icon);

	m_listener = new CuiDataDrivenPageListener(this, okButton, cancelButton, getButtonClose());
	page.AddCallback(m_listener);

	m_subscribedEvents = new EventSubscriptionMap;
} //lint !e429 "icon" not freed or returned (maintained by other sysstem)

//-----------------------------------------------------------------

UIEventCallback* CuiDataDrivenPage::getCallbackObject()
{	
	return m_listener;
}

//-----------------------------------------------------------------

CuiDataDrivenPage::~CuiDataDrivenPage()
{
	stopPageSound();

	delete m_subscribedProperties;
	m_subscribedProperties = 0;

	//clean up the mediators
	if (m_mediators)
	{
		std::for_each (m_mediators->begin (), m_mediators->end (), ReleaserFunctor ());
		delete m_mediators;
		m_mediators = 0;
	}
	if(m_listener)
	{
		getPage().RemoveCallback(m_listener);
		delete m_listener;
		m_listener = NULL;
	}

	delete m_subscribedEvents;
	m_subscribedEvents = NULL;
}

//-----------------------------------------------------------------

/** Return the ID of the this page (a unique ID generated by the server)
 */
int CuiDataDrivenPage::getClientPageId() const
{
	return m_clientPageId;
}

//----------------------------------------------------------------------

void CuiDataDrivenPage::onSetProperty(std::string const & widgetPath, bool isThisPage, std::string const & propertyName, Unicode::String const & propertyValue)
{
	UNREF(widgetPath);
	UNREF(isThisPage);
	UNREF(propertyName);
	UNREF(propertyValue);

	return;
}

//-----------------------------------------------------------------

/** Handle activation
 */
void CuiDataDrivenPage::performActivate()
{
	CuiManager::requestPointer(true);

	m_ok = false;

	if (m_mediators)
	{
		for (MediatorVector::iterator it = m_mediators->begin (); it != m_mediators->end (); ++it)
			(*it)->activate ();
	}
}

//-----------------------------------------------------------------

/** Handle deactivation
 */
void CuiDataDrivenPage::performDeactivate()
{
	if (m_mediators)
	{
		for (MediatorVector::iterator it = m_mediators->begin (); it != m_mediators->end (); ++it)
			(*it)->deactivate ();
	}
	CuiManager::requestPointer(false);
}


//-----------------------------------------------------------------

/** Close this page
 */
bool CuiDataDrivenPage::close()
{		
	if(!isClosed())
	{
		setClosed(true);
		//Send CloseCancel notification up to the server
		onEvent(SuiEventType::SET_onClosedCancel, NULL);		
	}
	stopPageSound();
	CuiDataDrivenPageManager::removePage(this, true);
	deactivate();
	return false;
}

//-----------------------------------------------------------------

/** Track the given widget name, and return the value of it to the server when the dialog completes
 */
void CuiDataDrivenPage::subscribeToProperty(const std::string& uiobjectName, const std::string& propertyName)
{
	m_subscribedProperties->push_back(SubscribedProperty(uiobjectName, propertyName));
}

//-----------------------------------------------------------------

/** Return hte list of subscribed properties and their values
 */
CuiDataDrivenPage::SubscribedPropertyMap CuiDataDrivenPage::getSubscribedPropertyValues()
{
	const UIPage & p = getPage();
	std::map<SubscribedProperty, Unicode::String> result;
	for(std::vector<SubscribedProperty>::iterator i = m_subscribedProperties->begin(); i != m_subscribedProperties->end(); ++i)
	{
		const SubscribedProperty & prop = *i;

		const std::string object = prop.uiObjectName.c_str();
		UIBaseObject * uiobject = NULL;
		if(object.empty())
			uiobject = &(getPage());
		else
			uiobject = dynamic_cast<UIBaseObject *> (p.GetObjectFromPath (prop.uiObjectName.c_str()));
		if (uiobject)
		{
			Unicode::String s;
			if (uiobject->GetProperty(UILowerString (prop.propertyName), s))
				result[prop] = s;
		}
		else
			WARNING (true, ("Bad page name '%s' in CuiDataDrivenPage::getSubscribedPropertyValue", prop.uiObjectName.c_str ()));
		
	}
	return result;
}

//----------------------------------------------------------------------

void CuiDataDrivenPage::update (float deltaTimeSecs)
{
	CuiMediator::update (deltaTimeSecs);
}

//-----------------------------------------------------------------

void CuiDataDrivenPage::onEvent(int eventType, UIWidget const * widget)
{	
	//This function sends appropriate notifications to items listed in the event suscription map.
	//It also handles the special legacy cases of the SET_onClosedOK and SET_onClosedCancel events

	if (!isActive())
	{
		if ((eventType != SuiEventType::SET_onClosedOk) && (eventType != SuiEventType::SET_onClosedCancel))
			return;
	}

	if(m_exportButton && widget == m_exportButton)
		Export(getPage());

	std::string relativePath;
	if (widget)
		getPage().GetPathTo(relativePath, widget);
	SuiEventSubscription const targetEventSubscription(eventType, relativePath);
	EventSubscriptionMap::const_iterator const i =  m_subscribedEvents->find(targetEventSubscription);
	if (i != m_subscribedEvents->end())
	{
		EventSubscriptionData const & eventSubscriptionData  = i->second;
		SubscribedPropertyVector const & subscribedProperties = eventSubscriptionData.subscribedPropertyVector;
		
		SuiEventNotification eventNotification(getClientPageId(), eventSubscriptionData.eventSubscriptionIndex, subscribedProperties.size());
		SubscribedPropertyVector::const_iterator spi;
		for (spi = subscribedProperties.begin(); spi != subscribedProperties.end(); ++spi)
		{
			SubscribedProperty const & subscribedProperty = *spi;

			UIString value;
			UIBaseObject const * const uiObject = getPage().GetObjectFromPath (subscribedProperty.uiObjectName.c_str ());					
			if (uiObject && uiObject->GetProperty(UILowerString(subscribedProperty.propertyName), value))
			{
				eventNotification.addSubscribedProperty(value);
			}
		}		
		GameNetwork::send(eventNotification, true);
	}
	
}

//-----------------------------------------------------------------

void CuiDataDrivenPage::processPageData(SuiPageData const & pageData)
{
	//May be called to initialize, or update an existing page.  The first call initializes, after that it's updates
	bool isInitialize = false;
	if (m_initializedWithPageData)
	{
		isInitialize = false;
	}
	else
	{
		isInitialize = true;
		m_initializedWithPageData = true;
	}

	bool setProperties = false;
	
	SuiPageData::SuiCommandVector const &commands = pageData.getCommands();
	SuiPageData::SuiCommandVector::const_iterator i;
	for (i = commands.begin(); i != commands.end(); ++i)
	{
		if (!isInitialize && (i->getType() == SuiCommand::SCT_subscribeToEvent))
			continue;
		if (i->getType() == SuiCommand::SCT_setProperty)
			setProperties = true;
		executeCommand(*i);
	}
	
	
	if (m_windowNameFound && m_autosave)
	{
		setSettingsAutoSizeLocation (true, true);
	}
	else
	{
		DEBUG_WARNING(m_autosave, ("Could not find name for DataDrivenPage, cannot load and save location/size"));
	}
	
	if (setProperties)
		handleMediatorPropertiesChanged();
	
	setAssociatedObjectId  (pageData.getAssociatedObjectId());
	setAssociatedLocation  (pageData.getAssociatedLocation());
	setMaxRangeFromObject  (pageData.getMaxRangeFromObject());

	return;
}

//-----------------------------------------------------------------

void CuiDataDrivenPage::expandTokens(std::string & input)
{
	size_t nextPercent = input.find('%');
	if(nextPercent == std::string::npos)
		return;
	std::string result;
	result.append(input, 0, nextPercent);
	Unicode::String value;
	while(nextPercent != std::string::npos)
	{
		const size_t endPercent = input.find('%', nextPercent + 1);
		if(endPercent == std::string::npos)
		{
			DEBUG_WARNING(true, ("Invalid string for parsing [%s]", input.c_str()));
			return;
		}
		value.clear();
		getCodeDataString(input.substr(nextPercent + 1, endPercent - 1).c_str(), value);
		result.append(Unicode::wideToNarrow(value));
		nextPercent = input.find('%', endPercent + 1);
		result.append(input, endPercent + 1, nextPercent);
	}
	input = result;
	return;
}

//-----------------------------------------------------------------

void CuiDataDrivenPage::expandTokens(Unicode::String & input)
{
	size_t nextPercent = input.find('%');
	if(nextPercent == Unicode::String::npos)
		return;
	Unicode::String result;
	result.append(input, 0, nextPercent);
	Unicode::String value;
	while(nextPercent != Unicode::String::npos)
	{
		const size_t endPercent = input.find('%', nextPercent + 1);
		if(endPercent == Unicode::String::npos)
		{
			DEBUG_WARNING(true, ("Invalid string for parsing [%s]", input.c_str()));
			return;
		}
		value.clear();

		// treat consecutive %% as the % literal
		if ((nextPercent + 1) == endPercent)
			value = Unicode::narrowToWide("%");
		else
			getCodeDataString(Unicode::wideToNarrow(input.substr(nextPercent + 1, endPercent)).c_str(), value);

		result.append(value);
		nextPercent = input.find('%', endPercent + 1);

		if (nextPercent == Unicode::String::npos)
			result.append(input, endPercent + 1, (input.size() - endPercent - 1));
		else
			result.append(input, endPercent + 1, (nextPercent - endPercent - 1));
	}
	input = result;
	return;
}

//-----------------------------------------------------------------

void CuiDataDrivenPage::executeCommand(SuiCommand const & command)
{
	std::string widgetPath = command.getTargetWidget();
	expandTokens(widgetPath);
	//@todo Here is where we should tokenize
	switch(command.getType())
	{
	case SuiCommand::SCT_clearDataSource:
		{
			UIDataSource * const dataSource = static_cast<UIDataSource *>(getPage().GetObjectFromPath( widgetPath.c_str(), TUIDataSource ));
			if(dataSource)
				dataSource->Clear();
		}
		break;
	case SuiCommand::SCT_clearDataSourceContainer:
		{
			UIDataSourceContainer * const dataSourceContainer = static_cast<UIDataSourceContainer *>(getPage().GetObjectFromPath( widgetPath.c_str(), TUIDataSourceContainer ));
			if(dataSourceContainer)
				dataSourceContainer->Clear();
		}
		break;
	case SuiCommand::SCT_addChildWidget:
		{			
			UILoader loader;
			UIWidget * const parentWidget = static_cast<UIWidget *>(getPage().GetObjectFromPath( widgetPath.c_str(), TUIWidget ));
			std::string widgetType;
			std::string widgetName;
			command.getAddChildWidget(widgetType, widgetName);
			expandTokens(widgetType);
			expandTokens(widgetName);
	
			UIBaseObject* const newWidget = loader.CreateObject(widgetType.c_str());
			if(newWidget)
			{			
				newWidget->SetName(widgetName.c_str());
				if (parentWidget)
					IGNORE_RETURN(parentWidget->AddChild (newWidget));
				else
					WARNING (true, ("No such parent widget: [%s]", widgetPath.c_str ()));
			}
			else
				WARNING(true, ("SuiCommand::SCT_addChildWidget invalid widget type [%s]", widgetType.c_str()));
		}
		break;
	case SuiCommand::SCT_setProperty:
		{
			std::string propertyName;
			Unicode::String propertyValue;
			command.getSetProperty(propertyName, propertyValue);

			bool wasProsePackage = expandProsePackage(propertyValue);
			expandTokens(propertyName);
			if(!wasProsePackage)
				expandTokens(propertyValue);
			if( (_stricmp(widgetPath.c_str(), CuiDataDrivenPageNamespace::titleWidgetPath) == 0) && (_stricmp(propertyName.c_str(),CuiDataDrivenPageNamespace::titleProperty) == 0) )
			{				
				std::string windowName = Unicode::wideToNarrow(propertyValue);
				expandTokens(windowName);
				setMediatorDebugName(windowName);
				m_windowNameFound = true;
			}

			bool isThisPage = _stricmp(widgetPath.c_str(), CuiDataDrivenPageNamespace::thisObject) == 0 ||
				_stricmp(widgetPath.c_str(), getPage().GetName().c_str()) == 0;

			//-- check for autosave
			if (isThisPage && _stricmp(propertyName.c_str(), CuiDataDrivenPageNamespace::autosaveProperty) == 0)
			{
				std::string autosave = Unicode::wideToNarrow(propertyValue);
				m_autosave = !(autosave[0] == 'F' || autosave[0] == 'f' || autosave == "0");
			}

			//-- check for sound
			if (isThisPage && _stricmp(propertyName.c_str(), CuiDataDrivenPageNamespace::soundProperty) == 0)
			{
				std::string soundFile = Unicode::wideToNarrow(propertyValue);
				if (soundFile.empty())
					stopPageSound();
				else
					startPageSound(soundFile.c_str());
			}

			UIBaseObject * const targetWidget = getPage().GetObjectFromPath( widgetPath.c_str());
			if(targetWidget)
				targetWidget->SetProperty(UILowerString (propertyName), propertyValue);	

			onSetProperty(widgetPath, isThisPage, propertyName, propertyValue);
		}
		break;
	case SuiCommand::SCT_addDataItem:
		{
			UIDataSource * const dataSource = static_cast<UIDataSource *>(getPage().GetObjectFromPath( widgetPath.c_str(), TUIDataSource ));
			std::string dataItemName;
			Unicode::String dataItemValue;
			command.getAddDataItem(dataItemName, dataItemValue);
			expandTokens(dataItemName);
			expandTokens(dataItemValue);
			if (dataSource)
			{
				UIData* const d = new UIData;
				IGNORE_RETURN(d->SetProperty(UILowerString (dataItemName), dataItemValue));
				IGNORE_RETURN(dataSource->AddChild(d));
			}
			else
				WARNING (true, ("Bad data source name in addDataItem: [%s]", widgetPath.c_str ()));	
		}
		break;
	case SuiCommand::SCT_addDataSourceContainer:
		{
			UIDataSourceContainer * const parent = static_cast<UIDataSourceContainer *>(getPage().GetObjectFromPath( widgetPath.c_str(), TUIDataSourceContainer ));
			std::string dataSourceContainerName;
			Unicode::String dataSourceContainerValue;
			command.getAddDataSourceContainer(dataSourceContainerName, dataSourceContainerValue);
			expandTokens(dataSourceContainerName);
			expandTokens(dataSourceContainerValue);
			if (parent)
			{
				UIDataSourceContainer* const d = new UIDataSourceContainer;
				IGNORE_RETURN(d->SetProperty(UILowerString (dataSourceContainerName), dataSourceContainerValue));
				IGNORE_RETURN(parent->AddChild(d));
			}
			else
				WARNING (true, ("Bad data source name in addDataSourceContainer: [%s]", widgetPath.c_str ()));	
		}
		break;
	case SuiCommand::SCT_addDataSource:
		{
			UIDataSource * const parent = static_cast<UIDataSource *>(getPage().GetObjectFromPath( widgetPath.c_str(), TUIDataSourceContainer ));
			std::string dataSourceName;
			Unicode::String dataSourceValue;
			command.getAddDataSource(dataSourceName, dataSourceValue);
			expandTokens(dataSourceName);
			expandTokens(dataSourceValue);
			if (parent)
			{
				UIDataSource* const d = new UIDataSource;
				IGNORE_RETURN(d->SetProperty(UILowerString (dataSourceName), dataSourceValue));
				IGNORE_RETURN(parent->AddChild(d));
			}
			else
				WARNING (true, ("Bad data source name in addDataSource: [%s]", widgetPath.c_str ()));	
		}
		break;
	case SuiCommand::SCT_subscribeToEvent:
		{
			UIWidget * const targetWidget = static_cast<UIWidget *>(getPage().GetObjectFromPath( widgetPath.c_str(), TUIWidget ));
			int eventType = 0;
			std::string callback;
			command.getSubscribeToEventCallback(eventType, callback);
			if(targetWidget)
			{
				if(!isRegisteredMediatorObject(*targetWidget))
					registerMediatorObject(*targetWidget, true);
			}
			SuiCommand::WidgetPropertyVector widgetProperties;
			command.getPropertySubscriptionsForEvent(widgetProperties);
			SuiCommand::WidgetPropertyVector::iterator widgetPropertiesIterator;

			SubscribedPropertyVector subscribedPropertyVector;
			subscribedPropertyVector.reserve(widgetProperties.size());
			
			for(widgetPropertiesIterator = widgetProperties.begin(); widgetPropertiesIterator != widgetProperties.end(); ++widgetPropertiesIterator)
			{
				SuiWidgetProperty &widgetProperty = *widgetPropertiesIterator;
				expandTokens(widgetProperty.widgetName);
				expandTokens(widgetProperty.propertyName);
				SubscribedProperty subscribedProperty(widgetProperty.widgetName, widgetProperty.propertyName);
				subscribedPropertyVector.push_back(subscribedProperty);
			}								
			SuiEventSubscription const eventSubscription(eventType, widgetPath);
			EventSubscriptionData eventSubscriptionData;
			eventSubscriptionData.eventSubscriptionIndex = m_nextEventSubscriptionIndex;
			eventSubscriptionData.subscribedPropertyVector = subscribedPropertyVector;
			m_subscribedEvents->insert(std::make_pair(eventSubscription, eventSubscriptionData));
			m_nextEventSubscriptionIndex++;
		}
		break;
	case SuiCommand::SCT_none:
	default:
		WARNING_DEBUG_FATAL(true, ("Invalid SuiCommand::Type found: %d", command.getType()));
		break;
	}
	return;
}

//----------------------------------------------------------------------

void CuiDataDrivenPage::handleMediatorPropertiesChanged()
{
	if (m_mediators != NULL)
	{
		for (MediatorVector::iterator it = m_mediators->begin(); it != m_mediators->end(); ++it)
		{
			CuiMediator * const mediator = NON_NULL(*it);
			mediator->handleMediatorPropertiesChanged();
		}
	}
}

//----------------------------------------------------------------------

bool CuiDataDrivenPage::expandProsePackage(Unicode::String &input)
{
	size_t nullPos = input.find(Unicode::unicode_char_t('\0'));
	if(nullPos == input.npos)
		return false;

	Unicode::String normalText = input.substr(0, nullPos);
	Unicode::String oob = input.substr(nullPos + 1);

	if(!oob.empty())
	{
		Unicode::String result;
		ProsePackageManagerClient::appendAllProsePackages(oob, result);
		
		normalText += result;
		input = normalText;
		return true;
	}
	input = normalText;
	return false;
}

//-----------------------------------------------------------------

void CuiDataDrivenPage::startPageSound(char const * const soundFile)
{
	if (m_pageSoundId.isValid())
		Audio::stopSound(m_pageSoundId);
	m_pageSoundId = Audio::playSound(soundFile, NULL);
}

//-----------------------------------------------------------------

void CuiDataDrivenPage::stopPageSound()
{
	if (m_pageSoundId.isValid())
	{
		Audio::stopSound(m_pageSoundId);
		m_pageSoundId.invalidate();
	}
}

void CuiDataDrivenPageNamespace::Export(UIPage & page)
{
	FILE * newFile = NULL;

	UIBaseObject * const targetWidget = page.GetObjectFromPath( "bg.caption.lblTitle");
	UIString name = Unicode::narrowToWide("TableNameMissing");
	if(targetWidget)
	{
		UIString localText;
		if(targetWidget->GetProperty(UILowerString ("LocalText"), localText))
			name = formatText(localText);
	}
	
	char filename[128];
	memset(filename, 0, 128);
	
	Os::createDirectories("export/");

	sprintf(filename, "./export/%s.html", Unicode::wideToNarrow(name).c_str());

	newFile = fopen(filename, "r");
	while(newFile)
	{
		// File already exists, append a value till we find one that doesn't.
		fclose(newFile);
		static int currentAppend = 0;
		++currentAppend;
		memset(filename, 0, 128);
		sprintf(filename, "./export/%s%d.html", Unicode::wideToNarrow(name).c_str(), currentAppend);
		newFile = fopen(filename, "r");
	}

	if(newFile)
		fclose(newFile);

	newFile = fopen(filename, "wt");
	if(newFile)
	{
		char header[256];
		memset(header, 0, 256);
		sprintf(header, "<html>\n<head>\n<title>%s</title>\n</head>\n<body><center>\n", Unicode::wideToNarrow(name).c_str());
		fwrite(header, 1, strlen(header), newFile);

		UITable * table = dynamic_cast<UITable*>(page.GetObjectFromPath("comp.TablePage.table"));

		if(table && table->GetTableModel())
		{
			UITableModel * tableInfo = table->GetTableModel();

			int totalColumns = tableInfo->GetColumnCount();
			int totalRowCount = tableInfo->GetRowCount();
			
			const char * tableBorder = "<table border =\"1\">\n";
			const char * tableClosingTag = "</table>\n";
			const char * trTag = "<tr>\n";
			const char * closingTrTag = "</tr>\n";
			const char * color1 = "FFFFFF";
			const char * color2 = "CCCCCC";
			bool  colorChoice = true;

			fwrite(tableBorder, 1, strlen(tableBorder), newFile);
			fwrite(trTag, 1, strlen(trTag), newFile);

			for(int i = 0; i < totalColumns; ++i)
			{
				char headerBuffer[128];
				memset(headerBuffer, 0, 128);
				UIString columnName;
				tableInfo->GetLocalizedColumnName(i, columnName);
				sprintf(headerBuffer, "<th>%s</th>\n", Unicode::wideToNarrow(formatText(columnName)).c_str());

				fwrite(headerBuffer, 1, strlen(headerBuffer), newFile);

			}

			fwrite(closingTrTag, 1, strlen(closingTrTag), newFile);
			
			for(int i = 0; i < totalRowCount; ++i)
			{
				fwrite(trTag, 1, strlen(trTag), newFile);
				for(int j = 0; j < totalColumns; ++j)
				{
					

					UITableTypes::CellType type = tableInfo->GetColumnCellType(j);
					char rowTextBuffer[256];
					Unicode::String formatColumn;
					switch(type)
					{
					case UITableTypes::CT_image:
						{
							formatColumn = Unicode::narrowToWide("<i>Image</i>");

							UIImageStyle * value = NULL;
							if (tableInfo->GetValueAtImage(i, j, value) && (value != NULL))
							{
								formatColumn += Unicode::narrowToWide("=");
								formatColumn += Unicode::narrowToWide(value->GetName());
							}
						}
						break;
					case UITableTypes::CT_widget:
						{
							formatColumn = Unicode::narrowToWide("<i>Image</i>");
						}
						break;
					case UITableTypes::CT_bool:
						{
							bool value;
							formatColumn.clear();
							if(tableInfo->GetValueAtBool(i, j, value))
							{
								UIUtils::FormatBoolean(formatColumn, value);
							}
						}
						break;
					case UITableTypes::CT_integer:
					case UITableTypes::CT_percent:
					case UITableTypes::CT_delimitedInteger:
						{
							int value;
							if (tableInfo->GetValueAtInteger (i, j, value))
								UIUtils::FormatLong (formatColumn, value);

							if(type == UITableTypes::CT_percent)
								formatColumn.append(1, '%');

							if(type == UITableTypes::CT_delimitedInteger)
								formatColumn = UIUtils::FormatDelimitedInteger(formatColumn);
						}
						break;
					case UITableTypes::CT_float:
						{
							float f;
							if (tableInfo->GetValueAtFloat (i, j, f))
								UIUtils::FormatFloat (formatColumn, f);
						}
						break;
					case UITableTypes::CT_text:
					default:
						{
							tableInfo->GetValueAtText (i, j, formatColumn);
							formatColumn = formatText(formatColumn);
						}
						break;

					}

					memset(rowTextBuffer, 0, 256);
					sprintf(rowTextBuffer, "<td bgcolor=#%s>%s</td>\n", colorChoice ? color1 : color2, Unicode::wideToNarrow(formatColumn).c_str());
					fwrite(rowTextBuffer, 1, strlen(rowTextBuffer), newFile);

				}
				fwrite(closingTrTag, 1, strlen(closingTrTag), newFile);
				colorChoice = !colorChoice;
			}

			fwrite(tableClosingTag, 1, strlen(tableClosingTag), newFile);

		}
		
		char footer[32];
		memset(footer, 0, 32);
		sprintf(footer, "</center></body>\n</html>");
		fwrite(footer, 1, strlen(footer), newFile);

		char outputMessage[128];
		memset(outputMessage, 0, 128);
		sprintf(outputMessage, "Exported to %s", filename);

		CuiSystemMessageManager::sendFakeSystemMessage(Unicode::narrowToWide(outputMessage));
	}

	fclose(newFile);
}

Unicode::String CuiDataDrivenPageNamespace::formatText(Unicode::String & input)
{
	std::string inputString = Unicode::wideToNarrow(input);

	if(inputString[0] == '@')
	{
		// String ID
		std::string::size_type splitPos = inputString.find(':');

		if(splitPos == std::string::npos)
			return input;

		std::string cat = inputString.substr(1, splitPos - 1);
		std::string index = inputString.substr(splitPos + 1);

		StringId myStringID(cat, index);

		return myStringID.localize();
	}
	else if (inputString[0] == '\\' && inputString[1] == '#')
	{
		// Color code.

		std::string colorHex = inputString.substr(2, 6);
		std::string textString = inputString.substr(8, inputString.size() - 8);
		char buffer[256];
		memset(buffer, 0, 256);
		sprintf(buffer,"<font color=#%s>%s</font>", colorHex.c_str(), textString.c_str());
		
		return Unicode::narrowToWide(buffer);
	}

	return input;

}
// ======================================================================
