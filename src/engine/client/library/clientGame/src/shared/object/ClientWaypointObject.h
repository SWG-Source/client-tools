// ClientWaypointObject.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ClientWaypointObject_H
#define	_INCLUDED_ClientWaypointObject_H

//-----------------------------------------------------------------------

#include "clientGame/IntangibleObject.h"
#include "Archive/AutoDeltaVariableCallback.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/NetworkIdArchive.h"
#include "sharedMathArchive/VectorArchive.h"

class SharedWaypointObjectTemplate;
class StringId;
class Waypoint;
class WaypointDataBase;
class VectorArgb;
class Timer;

//-----------------------------------------------------------------------

class ClientWaypointObject : public IntangibleObject
{
public:

	struct Messages
	{
		struct ActivatedChanged
		{
			typedef ClientWaypointObject Payload;
		};

		struct LocationChanged
		{
			typedef ClientWaypointObject Payload;
		};

		struct ColorChanged
		{
			typedef ClientWaypointObject Payload;
		};
	};

	typedef stdvector<Watcher <ClientWaypointObject> >::fwd WaypointVector;
	typedef stdvector<ConstWatcher <ClientWaypointObject> >::fwd ConstWaypointVector;

	                                         ClientWaypointObject             (const SharedWaypointObjectTemplate *);
	                                        ~ClientWaypointObject             ();

	const NetworkId &                        getCell              () const;
	const Vector &                           getLocation          () const;
	const std::string                        getPlanetName        () const;
	const Unicode::String &                  getRegionName        () const;
	const std::string &                      getColor             () const;
	const VectorArgb &                       getColorArgb         () const;

	ClientWaypointObject &                   operator = (const Waypoint & rhs);

	bool                                     setWaypointActive    (bool b);

	bool                                     isWaypointActive     () const;
	bool                                     isWaypointVisible    () const;

	static ClientWaypointObject *            createClientWaypoint (const Unicode::String & name, const std::string planetName, const Vector & location, const std::string& color, bool active, const char * appearance = NULL, bool isPoi = false, bool isGroupWaypoint = false);
	static ClientWaypointObject *            createClientWaypoint (const ClientObject &target);
	static ClientWaypointObject *            createClientWaypoint (const Waypoint & wd, bool isGroupWaypoint = false);

	static void                              requestWaypoint      (const Unicode::String & name, const Vector & pos_w);
	static void                              requestWaypoint      (const ClientObject & target);
	static void                              requestWaypoint      (const Unicode::String & name, const std::string & planet, const Vector & pos_w, uint8 color);
	static void                              requestWaypoint      (const WaypointDataBase & wd);

	static const WaypointVector &            getActiveWaypoints   ();
	static const ConstWaypointVector &       getRegularWaypoints  ();
	static const ConstWaypointVector &       getPoiWaypoints      ();
	static const ConstWaypointVector &       getGroupWaypoints    ();

	static void                              checkWaypoints       (float const deltaTime);

	void                                     setDescription       (const StringId & desc);
	const StringId &                         getDescription       () const;

	void                                     setVisible           (const bool isVisible);
	void                                     setEntrance		  (const bool isEntrance);

	float                                    alter                (float time);

	bool                                     equals               (const WaypointDataBase & wd) const;
	bool                                     isPoi                () const;
	bool                                     isGroupWaypoint      () const;
	bool                                     isEntranceUsed       () const;
	bool                                     isEntrance           () const;
	bool                                     isSmallWaypoint      () const;

	const ClientWaypointObject *             getWorldObject       () const;

	Vector const getWorldWaypointLocation_w() const;
	ClientWaypointObject * getWorldObject();

	static void setSingleWaypointMode(bool enable, ClientObject * selectedObject, bool forceUpdate = false);

	void resetUpdateTimer();
	Vector const & getRelativeWaypointPosition() const;

	void updateCustomAppearance();
	
	bool computeBuildoutLocation_w(Vector & location_w) const;

private:
	ClientWaypointObject &                   operator = (const ClientWaypointObject & rhs);
	ClientWaypointObject(const ClientWaypointObject & source);

	void                                     removeWorldObject    ();
	void                                     addWorldObject       ();

	//----------------------------------------------------------------------
	//-- AutoDeltaVariable Callbacks

	struct Callbacks
	{
		template <typename U, typename T> struct DefaultCallback
		{
			void modified (ClientWaypointObject & target, const U & old, const U & value, bool isLocal) const;
		};

		struct Dummy;
		struct ColorChangeDummy;
		struct CellChangeDummy;

		typedef DefaultCallback<bool,        Dummy>              ActiveChange;
		typedef DefaultCallback<Vector,      Dummy>              LocationChange;
		typedef DefaultCallback<std::string, Dummy>              PlanetChange;
		typedef DefaultCallback<std::string, ColorChangeDummy>   ColorChange;
		typedef DefaultCallback<NetworkId,   CellChangeDummy>    CellChange;
	};

	friend Callbacks::ActiveChange;
	friend Callbacks::LocationChange;
	friend Callbacks::PlanetChange;
	friend Callbacks::ColorChange;

	Archive::AutoDeltaVariableCallback<bool, Callbacks::ActiveChange, ClientWaypointObject>         m_waypointActive;
	Archive::AutoDeltaVariableCallback<NetworkId, Callbacks::CellChange, ClientWaypointObject>      m_cell;
	Archive::AutoDeltaVariableCallback<Vector, Callbacks::LocationChange, ClientWaypointObject>     m_location;
	Archive::AutoDeltaVariableCallback<std::string, Callbacks::PlanetChange, ClientWaypointObject>  m_planetName;
	Archive::AutoDeltaVariable<Unicode::String>                                                     m_regionName;
	Archive::AutoDeltaVariable<bool>                                                                m_waypointVisible;
	Archive::AutoDeltaVariableCallback<std::string, Callbacks::ColorChange, ClientWaypointObject>   m_waypointColor;
	Watcher<ClientWaypointObject>                                                                   m_worldObject;

	std::string                                                                                     m_appearance;
	StringId                                                                                        m_description;
	bool m_isPoi : 1,
	     m_isGroupWaypoint : 1,
		m_isBuildoutWaypoint : 1;

	Vector m_relativeWaypointPosition;
};

//----------------------------------------------------------------------

inline bool ClientWaypointObject::isWaypointActive     () const
{
	return m_waypointActive.get();
}

//----------------------------------------------------------------------

inline const std::string & ClientWaypointObject::getColor () const
{
	return m_waypointColor.get ();
}

//----------------------------------------------------------------------

inline const ClientWaypointObject * ClientWaypointObject::getWorldObject       () const
{
	return m_worldObject.getPointer ();
}

//----------------------------------------------------------------------

inline ClientWaypointObject * ClientWaypointObject::getWorldObject()
{
	return m_worldObject.getPointer();
}

//----------------------------------------------------------------------

inline bool ClientWaypointObject::isPoi () const
{
	return m_isPoi;
}

//-----------------------------------------------------------------------

inline bool ClientWaypointObject::isGroupWaypoint () const
{
	return m_isGroupWaypoint;
}

//----------------------------------------------------------------------

inline Vector const ClientWaypointObject::getWorldWaypointLocation_w() const
{
	if (m_worldObject.getPointer())
		return m_worldObject->getPosition_w();
	else
		return getLocation();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ClientWaypointObject_H
