// ======================================================================
//
// LodMeshGeneratorTemplate.h
// Copyright 2002, 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_LodMeshGeneratorTemplate_H
#define INCLUDED_LodMeshGeneratorTemplate_H

// ======================================================================

#include "clientSkeletalAnimation/MeshGeneratorTemplate.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/MemoryBlockManagerMacros.h"

#include <vector>

class BasicMeshGeneratorTemplate;
class CrcLowerString;
class Iff;
class MemoryBlockManager;

// ======================================================================

class LodMeshGeneratorTemplate: public MeshGeneratorTemplate
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITHOUT_INSTALL;

public:

	static void install(bool allowLod0Skipping);

public:

	virtual int                               getDetailCount() const;
	virtual const BasicMeshGeneratorTemplate *fetchBasicMeshGeneratorTemplate(int detailIndex) const;
	virtual void                              preloadAssets() const;
	virtual void                              garbageCollect() const;

private:

	typedef stdvector<const BasicMeshGeneratorTemplate*>::fwd  BasicMeshGeneratorTemplateVector;
	typedef stdvector<CrcLowerString*>::fwd                    CrcStringVector;

private:

	static void                   remove();
	static MeshGeneratorTemplate *create(Iff *iff, CrcString const &filename);

private:

	LodMeshGeneratorTemplate(Iff &iff, CrcString const &filename);
	virtual ~LodMeshGeneratorTemplate();

	void     load_0000(Iff &iff);

	// Disabled.
	LodMeshGeneratorTemplate();
	LodMeshGeneratorTemplate(const LodMeshGeneratorTemplate&);
	LodMeshGeneratorTemplate &operator =(const LodMeshGeneratorTemplate&);

private:

	static bool  ms_installed;
	static bool  ms_skipL0;

private:

	CrcStringVector                           m_pathNames;
	BasicMeshGeneratorTemplateVector mutable  m_meshGeneratorTemplates;

};

// ======================================================================

#endif
