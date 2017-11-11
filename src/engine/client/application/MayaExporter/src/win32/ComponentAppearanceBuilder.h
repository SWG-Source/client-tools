// ======================================================================
//
// ComponentAppearanceBuilder.h
// Portions Copyright 1999, Bootprint Entertainment, Inc.
// Portions Copyright 2000 - 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ComponentAppearanceBuilder_H
#define INCLUDED_ComponentAppearanceBuilder_H

// ======================================================================

class Vector;

#include "MeshBuilder.h"

#include "MayaHierarchy.h"

#include <string>
#include <vector>

class IndexedTriangleList;

// ======================================================================

class ComponentAppearanceBuilder : public MeshBuilder
{
public:

	ComponentAppearanceBuilder(MObject const & rootMayaObject, Vector const &minVector, Vector const &maxVector);
	virtual ~ComponentAppearanceBuilder(void);

	bool addPart( char const * partName, Transform transform );
	void addRadarShape (const IndexedTriangleList* radarShape);

	bool hasFrames ( void ) const;

	bool write ( Iff & iff );

protected:

	void writeRadarShape (Iff& iff) const;

	class Part
	{
	public:

		Part( char const * name, Transform transform) : m_name(name), m_transform(transform) {}

		std::string m_name;
		Transform m_transform;
	};

	typedef std::vector<Part> PartList;

	PartList m_parts;

	IndexedTriangleList* m_radarShape;

private:

	ComponentAppearanceBuilder();
	ComponentAppearanceBuilder(ComponentAppearanceBuilder const &);
	ComponentAppearanceBuilder & operator=(ComponentAppearanceBuilder const &);
};

// ======================================================================

#endif // #ifndef INCLUDED_ComponentAppearanceBuilder_H
