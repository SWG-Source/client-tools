// \addtodoc

#ifndef INC_NXN_EVENTTARGET_H
#define INC_NXN_EVENTTARGET_H

/*	\class		CNxNEventTarget NxNEventTarget.h
 *
 *  \brief		This class provides an interface for receiving events from the alienbrain/medializer
 *				kernel. Just implement this interface (using the provided macros), to be able
 *				to receive and process events. 
 *
 *  \author		Axel Pfeuffer
 *
 *  \version	1.00
 *
 *  \date		2000
 *
 *	\mod
 *		[ap]-10-27-2000 file created.
 *	\endmod
 */

//---------------------------------------------------------------------------
//	helper macros for declaring and implementing an event
//---------------------------------------------------------------------------
/*
// declares an eventmap in for a class object
#define NXN_DECLARE_EVENTMAP(ClassName)				public:	\
														virtual bool BaseDispatch(CNxNEventMsg& eEvent);\
														virtual bool Dispatch(CNxNEventMsg& eEvent);

// for the implementation of the event map (start of event map)
#define NXN_BEGIN_EVENTMAP(ClassName, BaseClassName)	bool ClassName::BaseDispatch(CNxNEventMsg& eEvent)\
														{\
															return BaseClassName::Dispatch(eEvent);\
														}\
														bool ClassName::Dispatch(CNxNEventMsg& eEvent)\
														{\

// all events are routed to this method, too!
#define	NXN_EVENTS_ALL(EventMethod)							(EventMethod)(eEvent);

// for event methods
#define	NXN_EVENT(EventName, EventMethod)					if (eEvent.GetProperty(_STR("EventID")) == EventName) {\
																return (EventMethod)(eEvent);\
															} else 

#define NXN_END_EVENTMAP()									{ return BaseDispatch(eEvent); }\
														}
*/

// declares an eventmap in for a class object
#define NXN_DECLARE_EVENTMAP(ClassName)				public:	\
														virtual bool BaseDispatch(CNxNEventMsg& eEvent);\
														virtual bool Dispatch(CNxNEventMsg& eEvent);\
														virtual sNXN_EVENTMAP_ENTRY* GetEventMap() const;\
													private:\
														static sNXN_EVENTMAP_ENTRY* _m_pEventMap;\
														static sNXN_EVENTMAP_ENTRY _m_aEventEntries[];

// for the implementation of the event map (start of event map)
#define NXN_BEGIN_EVENTMAP(ClassName, BaseClassName)	sNXN_EVENTMAP_ENTRY* ClassName::GetEventMap() const\
														{\
															return ClassName::_m_pEventMap;\
														}\
														bool ClassName::BaseDispatch(CNxNEventMsg& eEvent)\
														{\
															return BaseClassName::Dispatch(eEvent);\
														}\
														bool ClassName::Dispatch(CNxNEventMsg& eEvent)\
														{\
															int nIndex = 0;\
															bool bDispatched = false;\
															while (1) {\
																EventMethodPtr pfnEventMethod = _m_aEventEntries[nIndex].pfnEventMethod;\
																if (pfnEventMethod == 0) {\
																	break;\
																}\
																if (eEvent.GetProperty(_STR("EventID")) == _m_aEventEntries[nIndex].szEvent) {\
																	if (!(this->*pfnEventMethod)(eEvent)) {\
																		return false;\
																	}\
																	else {\
																		bDispatched = true;\
																	}\
																}\
																nIndex++;\
															}\
															if (bDispatched) {\
																return true;\
															}\
															else {\
																return BaseDispatch(eEvent);\
															}\
														}\
														sNXN_EVENTMAP_ENTRY* ClassName::_m_pEventMap = &ClassName::_m_aEventEntries[0];\
														sNXN_EVENTMAP_ENTRY ClassName::_m_aEventEntries[] = {

// for event methods
#define	NXN_EVENT(EventName, EventMethod)					{ EventName, -1, (EventMethodPtr)(bool (CNxNEventTarget::*)(CNxNEventMsg&))&EventMethod }, 	


#define NXN_END_EVENTMAP()									{ "", -2, (EventMethodPtr)0 }\
														};


// all events are routed to this method, too!
//#define	NXN_EVENTS_ALL(EventMethod)							(EventMethod)(eEvent);


// declares an event method in the class interface
#define	NXN_EVENTMETHOD(EventMethodName)				bool EventMethodName(CNxNEventMsg& eEvent);

// for the implementation method in the implementation
#define NXN_EVENTMETHODIMP(EventMethodName)				bool EventMethodName(CNxNEventMsg& eEvent)

//	static const EVENTMAP_ENTRY _aEventEntries[];

//---------------------------------------------------------------------------
//	event map typedef's and event map structure
//---------------------------------------------------------------------------
// typedef for event method pointer
typedef bool (CNxNEventTarget::*EventMethodPtr)(CNxNEventMsg&); 

// structure for one event map entry
struct sNXN_EVENTMAP_ENTRY {
	const char* szEvent; // name of thge event as a string
	long	lEventID; // unique id of the event
	EventMethodPtr pfnEventMethod; // pointer to the event processing method
};

//---------------------------------------------------------------------------
//	CNxNEventTarget class interface
//---------------------------------------------------------------------------
class NXNINTEGRATORSDK_API CNxNEventTarget {
	public:
		//---------------------------------------------------------------------------
		//	construction/destruction
		//---------------------------------------------------------------------------
		CNxNEventTarget(CNxNIntegrator* pIntegrator);
		CNxNEventTarget(CNxNEventManager* pEventManager);
		virtual ~CNxNEventTarget();

		//---------------------------------------------------------------------------
		//	registration and unregistration methods
		//---------------------------------------------------------------------------
		bool Register();
		bool Unregister();
		inline bool IsRegistered() const { return m_bRegistered; };

		//---------------------------------------------------------------------------
		//	pure virtual dispatch method (implemented by the macros)
		//---------------------------------------------------------------------------
		virtual bool Dispatch(CNxNEventMsg& pEvent) = 0 { return true; };

		//---------------------------------------------------------------------------
		//	direct access to the event manager's object
		//---------------------------------------------------------------------------
		inline CNxNEventManager* GetEventManager() const { return m_pEventManager; };

		virtual sNXN_EVENTMAP_ENTRY* GetEventMap() const { return NULL; };
	private:
		CNxNEventManager*	m_pEventManager;
		bool				m_bRegistered;
};

#endif // INC_NXN_EVENTTARGET_H
