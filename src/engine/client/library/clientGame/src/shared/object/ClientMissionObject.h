// ClientMissionObject.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ClientMissionObject_H
#define	_INCLUDED_ClientMissionObject_H

//-----------------------------------------------------------------------

#include "clientGame/IntangibleObject.h"
#include "Archive/AutoDeltaVariableCallback.h"
#include "sharedUtility/Location.h"
#include "sharedGame/Waypoint.h"

#include "clientGame/ClientWaypointObject.h"

class ClientWaypointObject;
class SharedMissionObjectTemplate;
class Token;

//-----------------------------------------------------------------------

class ClientMissionObject : public IntangibleObject
{
public:
	struct Messages
	{
		struct EnteredMissionBag
		{
			typedef ClientMissionObject Payload;
		};
		struct LeftMissionBag
		{
			typedef ClientMissionObject Payload;
		};
	};

	ClientMissionObject(const SharedMissionObjectTemplate *);
	~ClientMissionObject();

	const int                getDifficulty      () const;
	const StringId &         getDescription     () const;
	const Location &         getEndLocation     () const;
	const Unicode::String &  getMissionCreator  () const;
	const int                getReward          () const;
	const int                getStatus          () const;
	const Location &         getStartLocation   () const;
	const uint32             getTargetAppearanceCrc() const;
	const std::string &      getTargetName      () const;
	const StringId &         getTitle           () const;
	const unsigned int       getType            () const;
	const std::string &      getMissionTypeString() const;
	ClientWaypointObject*    getLocalWaypoint   () const;
	void                     setLocalWaypoint   (ClientWaypointObject* wp);
	void                     movedFromMissionBag();
	bool                     isSpaceMission() const;
	bool                     isDuty() const;
	void                     updateName();

	void  containedByModified(const NetworkId & oldValue, const NetworkId & newValue, bool isLocal);

	virtual void getObjectInfo(stdmap<std::string, stdmap<std::string, Unicode::String>::fwd >::fwd & propertyMap) const;

public:
	static void              applyDeferredMissionObjectProcessing();

private:
	ClientMissionObject & operator = (const ClientMissionObject & rhs);
	ClientMissionObject(const ClientMissionObject & source);

	static void              addDeferredMissionObject(ClientMissionObject* mission, const NetworkId& newValue);

	struct Callbacks
	{
		template <typename U> struct DefaultCallback
		{
			void modified (ClientMissionObject & target, const U & old, const U & value, bool isLocal) const;
		};

		typedef DefaultCallback<int>             StatusChange;
		typedef DefaultCallback<unsigned int>    TypeChange;
		typedef DefaultCallback<Waypoint>        WaypointChange;
	};

private:
	static stdmap<Watcher<ClientMissionObject>, NetworkId>::fwd                                   ms_deferredMissionObjectMap;

private:
	friend Callbacks::StatusChange;
	friend Callbacks::TypeChange;

	Archive::AutoDeltaVariable<StringId>                                                          m_description;
	Archive::AutoDeltaVariable<int>                                                               m_difficulty;
	Archive::AutoDeltaVariable<Location>                                                          m_endLocation;
	Archive::AutoDeltaVariable<Unicode::String>                                                   m_missionCreator;
	Archive::AutoDeltaVariableCallback<unsigned int, Callbacks::TypeChange, ClientMissionObject>  m_missionType;
	Archive::AutoDeltaVariable<int>                                                               m_reward;
	Archive::AutoDeltaVariable<Location>                                                          m_startLocation;
	Archive::AutoDeltaVariable<uint32>                                                            m_targetAppearanceCrc;
	Archive::AutoDeltaVariable<StringId>                                                          m_title;
	Archive::AutoDeltaVariableCallback<int, Callbacks::StatusChange, ClientMissionObject>         m_status;
	Archive::AutoDeltaVariable<std::string>                                                       m_targetName;
	Archive::AutoDeltaVariableCallback<Waypoint, Callbacks::WaypointChange, ClientMissionObject>  m_waypoint;

	Watcher<ClientWaypointObject>                                                                 m_localWaypoint;
};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ClientMissionObject_H
