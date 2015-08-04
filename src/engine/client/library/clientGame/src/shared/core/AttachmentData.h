//======================================================================
//
// AttachmentData.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_AttachmentData_H
#define INCLUDED_AttachmentData_H

//======================================================================

class ClientObject;
class OutOfBandBase;
class WaypointDataBase;

//----------------------------------------------------------------------

class AttachmentData
{
public:

	enum Type
	{
		T_none,
		T_waypoint,
		T_numTypes
	};

public:
	                 AttachmentData ();
	explicit         AttachmentData (const Unicode::String & oob);
	explicit         AttachmentData (const ClientObject & oob);
	                ~AttachmentData ();
	                 AttachmentData (const AttachmentData & rhs);
	AttachmentData & operator=      (const AttachmentData & rhs);

	Type                    getType         () const;
	const WaypointDataBase *getWaypointData () const;
	bool                    set             (const Unicode::String & oob);
	bool                    set             (const ClientObject & obj);
	bool                    set             (const OutOfBandBase & base);
	void                    clear           ();
	bool                    isValid         () const;
	void                    packToOob       (Unicode::String & oob, int position) const;

private:

	Type                    m_type;
	const WaypointDataBase *m_waypoint;
	bool                    m_valid;
};

//----------------------------------------------------------------------

inline AttachmentData::Type  AttachmentData::getType         () const
{
	return m_type;
}

//----------------------------------------------------------------------

inline const WaypointDataBase * AttachmentData::getWaypointData () const
{
	return m_waypoint;
}

//----------------------------------------------------------------------

inline bool AttachmentData::isValid () const
{
	return m_valid;
}

//======================================================================

#endif
