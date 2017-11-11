// ======================================================================
//
// LodSkeletonTemplate.h
// Copyright 2002, 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_LodSkeletonTemplate_H
#define INCLUDED_LodSkeletonTemplate_H

// ======================================================================

#include "clientSkeletalAnimation/SkeletonTemplate.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/MemoryBlockManagerMacros.h"

#include <vector>

class BasicSkeletonTemplate;
class Iff;
class MemoryBlockManager;

// ======================================================================

class LodSkeletonTemplate: public SkeletonTemplate
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITHOUT_INSTALL;

public:

	static void install(bool allowLod0Skipping);

public:

	virtual int                          getDetailCount() const;
	virtual const BasicSkeletonTemplate *fetchBasicSkeletonTemplate(int detailIndex) const;

private:

	typedef stdvector<const BasicSkeletonTemplate*>::fwd  BasicSkeletonTemplateVector;

private:

	static void              remove();
	static SkeletonTemplate *create(Iff &iff, CrcString const &filename);

private:

	LodSkeletonTemplate(Iff &iff, CrcString const &filename);
	virtual ~LodSkeletonTemplate();

	void     load_0000(Iff &iff);

	// Disabled.
	LodSkeletonTemplate();
	LodSkeletonTemplate(const LodSkeletonTemplate&);
	LodSkeletonTemplate &operator =(const LodSkeletonTemplate&);

private:

	static bool  ms_installed;
	static bool  ms_skipL0;

private:

	BasicSkeletonTemplateVector  m_skeletonTemplates;

};

// ======================================================================

#endif
