// ======================================================================
//
// ShadowBlobObject.h
// asommers
// 
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_ShadowBlobObject_H
#define INCLUDED_ShadowBlobObject_H

// ======================================================================

#include "sharedObject/Object.h"

// ======================================================================

namespace DPVS
{
	class RegionOfInfluence;
};

// ======================================================================

class ShadowBlobObject : public Object
{
public:

	explicit ShadowBlobObject (float radius);
	virtual ~ShadowBlobObject ();

	virtual void addToWorld ();
	virtual void removeFromWorld ();
	virtual void setRegionOfInfluenceEnabled (bool enabled) const;

	float getRadius () const;

	bool isVisible() const;
	void setVisible(bool visible);

private:

	ShadowBlobObject ();
	ShadowBlobObject (const ShadowBlobObject&);
	ShadowBlobObject& operator= (const ShadowBlobObject&);

private:

	float const m_radius;
	bool m_visible;
	DPVS::RegionOfInfluence* m_dpvsRegionOfInfluence;
};

// ======================================================================

#endif
