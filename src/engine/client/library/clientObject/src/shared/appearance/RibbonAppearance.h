//===================================================================
// 
// RibbonAppearance.h
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_RibbonAppearance_H
#define INCLUDED_RibbonAppearance_H

//===================================================================

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/MemoryBlockManagerMacros.h"
#include "sharedObject/Appearance.h"

class MemoryBlockManager;
class VectorArgb;

//===================================================================

class RibbonAppearance : public Appearance
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITHOUT_INSTALL;

public:

	typedef stdvector<Vector>::fwd PointList;

public:

	static void install ();

public:

	RibbonAppearance (const PointList& pointList, float width, const VectorArgb& color, bool closed = true);
	virtual ~RibbonAppearance ();

	virtual const Sphere& getSphere () const;
	virtual void          render () const;

	const PointList&      getPointList () const;
	void                  setPointList (const PointList& pointList);

public:

	class LocalShaderPrimitive;

private:

	static void remove ();

private:

	DPVS::Object* getDpvsObject () const;
	void          updateDpvsTestModel ();

private:

	RibbonAppearance ();
	RibbonAppearance (const RibbonAppearance&);
	RibbonAppearance& operator= (const RibbonAppearance&);

private:

	LocalShaderPrimitive* m_localShaderPrimitive;
	DPVS::Object*         m_dpvsObject;
};

//===================================================================

#endif

