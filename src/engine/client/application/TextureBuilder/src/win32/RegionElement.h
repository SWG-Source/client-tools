// ======================================================================
//
// RegionElement.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef INCLUDED_RegionElement_h
#define INCLUDED_RegionElement_h

// ======================================================================

#include "Element.h"

// ======================================================================

class RegionElement: public Element
{
public:

	static bool isPersistedNext(Iff &iff);

public:

	explicit RegionElement(Iff &iff);
	RegionElement(const std::string &regionName, float x0, float y0, float x1, float y1);
	virtual ~RegionElement();

	virtual std::string   getLabel() const;
	virtual unsigned int  getTypeIndex() const;

	virtual bool          writeForWorkspace(Iff &iff) const;

	const std::string    &getRegionName() const;

	float                 getX0() const;
	float                 getY0() const;
	float                 getX1() const;
	float                 getY1() const;

	void                  setX0(float x0);
	void                  setY0(float y0);
	void                  setX1(float x1);
	void                  setY1(float y1);

private:

	void                  load_0000(Iff &iff);

private:

	std::string *m_regionName;

	float        m_x0;
	float        m_y0;
	float        m_x1;
	float        m_y1;

private:
	// disabled
	RegionElement();
	RegionElement(const RegionElement&);
	RegionElement &operator =(const RegionElement&);
};

// ======================================================================

inline const std::string &RegionElement::getRegionName() const
{
	return *NON_NULL(m_regionName);
}

// ----------------------------------------------------------------------

inline float RegionElement::getX0() const
{
	return m_x0;
}

// ----------------------------------------------------------------------

inline float RegionElement::getY0() const
{
	return m_y0;
}

// ----------------------------------------------------------------------

inline float RegionElement::getX1() const
{
	return m_x1;
}

// ----------------------------------------------------------------------

inline float RegionElement::getY1() const
{
	return m_y1;
}

// ----------------------------------------------------------------------

inline void RegionElement::setX0(float x0)
{
	m_x0 = x0;
}

// ----------------------------------------------------------------------

inline void RegionElement::setY0(float y0)
{ //lint !e578 // y0 hides bogus global math.h y0
	m_y0 = y0;
}

// ----------------------------------------------------------------------

inline void RegionElement::setX1(float x1)
{
	m_x1 = x1;
}

// ----------------------------------------------------------------------

inline void RegionElement::setY1(float y1)
{ //lint !e578 // y0 hides bogus global math.h y1
	m_y1 = y1;
}

// ======================================================================

#endif
