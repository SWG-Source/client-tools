// ======================================================================
//
// ComponentAppearanceTemplate.h
// Copyright 2000 - 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ComponentAppearanceTemplate_H
#define INCLUDED_ComponentAppearanceTemplate_H

//===================================================================

#include "sharedMath/Transform.h"
#include "sharedObject/AppearanceTemplate.h"

class IndexedTriangleList;

namespace DPVS
{
	class Model;
};

//===================================================================

class ComponentAppearanceTemplate : public AppearanceTemplate
{
public:

	static void                install ();
	static AppearanceTemplate* create (const char* filename, Iff* iff);

public:

	ComponentAppearanceTemplate (const char* filename, Iff* iff);
	virtual ~ComponentAppearanceTemplate ();

	virtual Appearance*        createAppearance () const;

	virtual void               preloadAssets() const;

	int                        getNumberOfComponents () const;
	const AppearanceTemplate*  getAppearanceTemplate (int componentIndex) const;
	const Transform&           getTransform (int componentIndex) const;

	DPVS::Model               *fetchDpvsTestShape() const;
	void                       setDpvsTestShape(DPVS::Model *testShape);

	const IndexedTriangleList* getRadarShape () const;

private:

	struct Component
	{
		const AppearanceTemplate * appearanceTemplate;
		Transform                  transform;
	};

private:

	static void remove ();

private:

	ComponentAppearanceTemplate ();
	ComponentAppearanceTemplate (const ComponentAppearanceTemplate&);
	ComponentAppearanceTemplate& operator= (const ComponentAppearanceTemplate&);

	const Component& getComponent (int componentIndex) const;

	void load ( Iff& iff );

private:

	void load_0001 ( Iff & iff );
	void load_0002 ( Iff & iff );
	void load_0003 ( Iff & iff );
	void load_0004 ( Iff & iff );
	void load_0005 (Iff& iff);

	void loadParts      ( Iff & iff );
	void loadParts_old  ( Iff & iff );

	void loadRadarShape (Iff& iff);

	void createTestShapeFromTemplate ();

private:

	static bool    ms_installed;

private:

	typedef stdvector<Component>::fwd ComponentList;
	ComponentList* m_componentList;
	DPVS::Model*   m_dpvsTestShape;

	IndexedTriangleList* m_radarShape;
};

//===================================================================

#endif
