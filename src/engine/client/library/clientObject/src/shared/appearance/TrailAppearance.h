//===================================================================
// 
// TrailAppearance.h
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_TrailAppearance_H
#define INCLUDED_TrailAppearance_H

//===================================================================

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/MemoryBlockManagerMacros.h"
#include "sharedObject/Appearance.h"

class MemoryBlockManager;
class VectorArgb;

//===================================================================

class TrailAppearance : public Appearance
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITHOUT_INSTALL;

public:

	static void install ();

public:

	TrailAppearance (const Object* followObject, float width, const VectorArgb& color);
	virtual ~TrailAppearance ();

	virtual const Sphere& getSphere () const;
	virtual float         alter (float time);
	virtual void          render () const;
	virtual bool          isAlive () const;

public:

	class LocalShaderPrimitive;

private:

	static void remove ();

private:

	DPVS::Object* getDpvsObject () const;
	void          updateDpvsTestModel ();

private:

	TrailAppearance ();
	TrailAppearance (const TrailAppearance&);
	TrailAppearance& operator= (const TrailAppearance&);

private:

	LocalShaderPrimitive* m_localShaderPrimitive;
	DPVS::Object*         m_dpvsObject;
};

//===================================================================

#endif

