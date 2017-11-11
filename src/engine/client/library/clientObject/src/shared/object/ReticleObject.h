// ======================================================================
//
// ReticleObject.h
// asommers
// 
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_ReticleObject_H
#define INCLUDED_ReticleObject_H

// ======================================================================

#include "sharedObject/Object.h"

// ======================================================================

namespace DPVS
{
	class RegionOfInfluence;
};

// ======================================================================

class ReticleObject : public Object
{
public:

	explicit ReticleObject (float radius);
	virtual ~ReticleObject ();

	virtual void addToWorld ();
	virtual void removeFromWorld ();
	virtual void setRegionOfInfluenceEnabled (bool enabled) const;

	float getRadius () const;
	void setRadius (float r);

	bool isVisible() const;
	void setVisible(bool visible);

private:

	ReticleObject ();
	ReticleObject (const ReticleObject&);
	ReticleObject& operator= (const ReticleObject&);

private:

	float m_radius;
	bool m_visible;
	DPVS::RegionOfInfluence* m_dpvsRegionOfInfluence;
};

// ======================================================================

#endif
