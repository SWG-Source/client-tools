//======================================================================
//
// CuiIconManagerObjectProperties.h
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiIconManagerObjectProperties_H
#define INCLUDED_CuiIconManagerObjectProperties_H

//======================================================================

class ClientObject;

//======================================================================

class CuiIconManagerObjectProperties
{
public:
	
	CuiIconManagerObjectProperties  ();
	explicit CuiIconManagerObjectProperties  (ClientObject const & obj);

	int  getDamageTaken() const;
	bool updateAndCompareFromObject (ClientObject const & obj);
	void updateFromObject           (ClientObject const & obj);

	bool operator== (CuiIconManagerObjectProperties const & rhs) const;

private:

	int m_damageTaken;
	int m_maxHitpoints;
	int m_condition;
	int m_count;
	int m_content;
	int m_contentMax;
};

//----------------------------------------------------------------------

inline int CuiIconManagerObjectProperties::getDamageTaken() const
{
	return m_damageTaken;
}

//----------------------------------------------------------------------

inline bool CuiIconManagerObjectProperties::operator== (CuiIconManagerObjectProperties const & rhs) const
{
	return 
		m_damageTaken  == rhs.m_damageTaken  &&
		m_maxHitpoints == rhs.m_maxHitpoints &&
		m_condition    == rhs.m_condition    &&
		m_count        == rhs.m_count        &&
		m_content      == rhs.m_content      &&
		m_contentMax   == rhs.m_contentMax;
}

//======================================================================

#endif
