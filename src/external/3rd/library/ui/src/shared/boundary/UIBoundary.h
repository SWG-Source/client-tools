// ======================================================================
//
// UIBoundary.h
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_UIBoundary_H
#define INCLUDED_UIBoundary_H

// ======================================================================

struct UIPoint;
typedef UIPoint UISize;

//----------------------------------------------------------------------

class UIBoundary
{
public:
	
	UIBoundary (const std::string & name);
	virtual bool       hitTest      (const UIPoint & pt) const = 0;
	virtual void       onSizeChange (const UISize & prevSize, const UISize & curSize) = 0;
	virtual           ~UIBoundary () = 0;

	const std::string & getName      () const;
	const std::string & getLowerName () const;

private:
	UIBoundary (const UIBoundary & rhs);
	UIBoundary & operator= (const UIBoundary & rhs);

	std::string m_name;
	std::string m_lowerName;
};

//----------------------------------------------------------------------

inline const std::string & UIBoundary::getName () const
{
	return m_name;
}

//----------------------------------------------------------------------

inline const std::string & UIBoundary::getLowerName () const
{
	return m_lowerName;
}

// ======================================================================

#endif
