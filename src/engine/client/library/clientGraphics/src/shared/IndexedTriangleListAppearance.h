// ======================================================================
//
// IndexedTriangleListAppearance.h
// asommers
//
// copyright 2001, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_IndexedTriangleListAppearance_H
#define INCLUDED_IndexedTriangleListAppearance_H

// ======================================================================

#include "sharedObject/Appearance.h"

class IndexedTriangleListShaderPrimitive;
class VectorArgb;

// ======================================================================

class IndexedTriangleListAppearance: public Appearance
{
public:

	IndexedTriangleListAppearance(const IndexedTriangleList *indexedTriangleList, const VectorArgb& color);
	virtual ~IndexedTriangleListAppearance();

	virtual void  render() const;

protected:

	DPVS::Object * getDpvsObject() const;

private:

	IndexedTriangleListAppearance();
	IndexedTriangleListAppearance(const IndexedTriangleListAppearance&);
	IndexedTriangleListAppearance &operator =(const IndexedTriangleListAppearance&);

private:

	IndexedTriangleListShaderPrimitive* m_indexedTriangleListShaderPrimitive;
	DPVS::Object*                       m_dpvsObject;
};

// ======================================================================

#endif
