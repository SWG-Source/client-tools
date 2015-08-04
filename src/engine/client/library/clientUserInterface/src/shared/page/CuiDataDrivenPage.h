// ======================================================================
//
// CuiDataDrivenPage.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CuiDataDrivenPage_H
#define INCLUDED_CuiDataDrivenPage_H

#include "UIEventCallback.h"
#include "clientAudio/SoundId.h"
#include "clientUserInterface/CuiMediator.h"

#include <vector>

class CuiDataDrivenPageListener;
class SubscribedProperty;
class SuiCommand;
class SuiEventSubscription;
class SuiPageData;
class UIButton;
class UIPage;
// ======================================================================

namespace CuiDataDrivenPageNamespace
{
	const char * const titleWidgetPath = "bg.caption.lbltitle";
	const char * const titleProperty   = "text";
	const char * const autosaveProperty = "autosave";
	const char * const soundProperty = "sound";
	const char * const thisObject = "this";
}

// ======================================================================

/**
 *  CuiDataDrivenPage just keeps a scene render around and also paints the background.
 *  This class is used by the server scripting system to instantiate some simple "fill-in-data" UI's.
 *  The CuiDataDrivenPageManager handles the creation and building of these pages.
 */
class CuiDataDrivenPage : 
public CuiMediator
{
public:
	struct SubscribedProperty
	{
		std::string uiObjectName;
		std::string propertyName;
		SubscribedProperty(std::string uin, std::string pn) : uiObjectName(uin), propertyName(pn) {}
		bool operator <(const SubscribedProperty& rhs) const;
	};

	typedef stdmap<SubscribedProperty, Unicode::String>::fwd SubscribedPropertyMap;
	typedef stdmap<std::string, int>::fwd                    SubscribedDataSourceMap;

public:
	                           CuiDataDrivenPage     (const std::string & name, UIPage & thePage, int clientPageId);
	virtual                    ~CuiDataDrivenPage    ();
	virtual bool               close                 ();
	void                       subscribeToProperty   (const std::string& uiObjectName, const std::string& propertyName);

	SubscribedPropertyMap      getSubscribedPropertyValues();

	virtual void               update                     (float deltaTimeSecs);

	int                        getClientPageId            () const;

	void                       onEvent               (int eventType, UIWidget const * widget);
	void                       processPageData       (SuiPageData const & pageData);

	void handleMediatorPropertiesChanged();

	void                       setClosed             (const bool isClosed);
	const bool                 isClosed              () const;

protected:
	virtual void               onSetProperty         (std::string const & widgetPath, bool isThisPage, std::string const & propertyName, Unicode::String const & propertyValue);
	virtual void               performActivate       ();
	virtual void               performDeactivate     ();
	virtual UIEventCallback*   getCallbackObject     ();
	void startPageSound(char const * const soundFile);
	void stopPageSound();

private:
	//disabled
	CuiDataDrivenPage                  ();
	CuiDataDrivenPage                  (const CuiDataDrivenPage& rhs);
	CuiDataDrivenPage& operator=       (const CuiDataDrivenPage& rhs);

private:
	typedef stdvector<SubscribedProperty>::fwd                             SubscribedPropertyVector;
	typedef stdvector<CuiMediator *>::fwd                                  MediatorVector;
	

	struct EventSubscriptionData
	{
		int                                eventSubscriptionIndex;   //Index of this subscription in the stream of added subscriptions
		stdvector<SubscribedProperty>::fwd subscribedPropertyVector;
	};

	typedef stdmap<SuiEventSubscription, EventSubscriptionData >::fwd EventSubscriptionMap;


	SubscribedPropertyVector *           m_subscribedProperties;
	int                                  m_clientPageId;
	bool                                 m_ok;

	MediatorVector*                      m_mediators;

	EventSubscriptionMap*                m_subscribedEvents;

	CuiDataDrivenPageListener*           m_listener;
	
	bool                                 m_initializedWithPageData;

	bool								 m_windowNameFound;

	int									 m_nextEventSubscriptionIndex;

	void                        executeCommand      (SuiCommand const & command);

	void                        expandTokens        (std::string & input);
	void                        expandTokens        (Unicode::String & input);
	bool                        expandProsePackage  (Unicode::String & input);

	bool                                 m_isClosed;

	bool m_autosave;
	SoundId m_pageSoundId;

	UIButton *							 m_exportButton;
};

//----------------------------------------------------------------------

inline void CuiDataDrivenPage::setClosed(const bool isClosed)
{
	m_isClosed = isClosed;
}

//----------------------------------------------------------------------

inline const bool CuiDataDrivenPage::isClosed(void) const
{
	return m_isClosed;
}

// ======================================================================

#endif
