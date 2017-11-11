//===================================================================
// 
// RibbonTrailAppearance.h
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_RibbonTrailAppearance_H
#define INCLUDED_RibbonTrailAppearance_H

//===================================================================

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/MemoryBlockManagerMacros.h"
#include "sharedObject/Appearance.h"

class MemoryBlockManager;
class VectorArgb;

//===================================================================

class RibbonTrailAppearance : public Appearance
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITHOUT_INSTALL;

public:

	static void install ();

public:

	RibbonTrailAppearance (const Object* followObject, float width, const VectorArgb& color);
	virtual ~RibbonTrailAppearance ();

	virtual const Sphere& getSphere () const;
	virtual float         alter (float time);
	virtual void          render () const;
	virtual bool          isAlive () const;

	void kill ();

public:

	class LocalShaderPrimitive;

private:

	static void remove ();

private:

	DPVS::Object* getDpvsObject () const;
	void          updateDpvsTestModel ();

private:

	RibbonTrailAppearance ();
	RibbonTrailAppearance (const RibbonTrailAppearance&);
	RibbonTrailAppearance& operator= (const RibbonTrailAppearance&);

private:

	LocalShaderPrimitive* m_localShaderPrimitive;
	DPVS::Object*         m_dpvsObject;
};

//===================================================================

#endif

