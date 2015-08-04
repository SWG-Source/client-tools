//===================================================================
//
// DetailAppearanceTemplate.h
//
// Copyright 2000-2002 Sony Online Interactive
// All Rights Reserved.
//
//===================================================================

#ifndef INCLUDED_DetailAppearanceTemplate_H
#define INCLUDED_DetailAppearanceTemplate_H

//===================================================================

#include "sharedObject/AppearanceTemplate.h"

#include <vector>
#include <string>

class Iff;
class IndexedTriangleList;

namespace DPVS
{
	class Model;
}

//===================================================================

class DetailAppearanceTemplate: public AppearanceTemplate
{
public:

	static void                install(bool viewer);

	static AppearanceTemplate *create(const char *newName, Iff *iff);

public:

	DetailAppearanceTemplate(const char *newName, Iff *iff);
	virtual ~DetailAppearanceTemplate(void);

	virtual void               preloadAssets() const;
	virtual Appearance        *createAppearance() const;

	int                        getDetailLevelCount(void) const;
	Appearance*                createAppearance(int detailLevel) const;
	real                       getNearDistance(int detailLevel) const;
	real                       getFarDistance(int detailLevel) const;
	const IndexedTriangleList *getRadarShape() const;
	const IndexedTriangleList *getTestShape() const;
	const IndexedTriangleList *getWriteShape() const;

	DPVS::Model               *fetchDpvsTestShape() const;
	void                       setDpvsTestShape(DPVS::Model *testShape);

	DPVS::Model               *getDpvsWriteShape() const;

	bool getUsePivotPoint()                 const { return m_usePivotPoint; }
	bool getDisableLodCrossFade()           const { return m_disableLodCrossFade; }

private:

	const char*                getAppearanceTemplateName (int detailLevel) const;
	const AppearanceTemplate*  getAppearanceTemplate (int detailLevel) const;

	void load(Iff& iff);
	bool loadEntries( Iff & iff );
	bool loadRadarShape(Iff& iff);
	bool loadTestShape( Iff & iff );
	bool loadWriteShape( Iff & iff );

	void fixNearFarDistance ( void );
	void computeBetterNearFarDistances ();

	void createDpvsTestShape (bool hasValidBoundingBox);

	void debugDump() const;

	// disabled
	DetailAppearanceTemplate(void);
	DetailAppearanceTemplate(const DetailAppearanceTemplate&);
	DetailAppearanceTemplate &operator =(const DetailAppearanceTemplate&);

public:
	// need this public so stl operations can access it

	struct Child
	{
		int id;
		char* appearanceTemplateName;
		const AppearanceTemplate* appearanceTemplate;
		real  nearDistance;
		real  farDistance;
	};

	void createAppearanceTemplate(int detailLevel) const;

private:

	static void remove ();

private:

	static bool ms_installed;

private:

	Child * findChild(int id);

	typedef stdvector<Child>::fwd ChildList;

	ChildList*  m_childList;

	IndexedTriangleList* m_radarShape;
	IndexedTriangleList* m_testShape;
	IndexedTriangleList* m_writeShape;
	DPVS::Model* m_dpvsTestShape;
	DPVS::Model* m_dpvsWriteShape;
	bool m_usePivotPoint;
	bool m_disableLodCrossFade;
};

//===================================================================

inline DPVS::Model* DetailAppearanceTemplate::getDpvsWriteShape() const
{
	return m_dpvsWriteShape;
}

//===================================================================

#endif
