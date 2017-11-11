// ======================================================================
//
// FullGeometrySkeletalAppearanceBatchRenderer.h
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_FullGeometrySkeletalAppearanceBatchRenderer_H
#define INCLUDED_FullGeometrySkeletalAppearanceBatchRenderer_H

// ======================================================================

#include "clientSkeletalAnimation/SkeletalAppearanceBatchRenderer.h"

// ======================================================================

class FullGeometrySkeletalAppearanceBatchRenderer: public SkeletalAppearanceBatchRenderer
{
public:

	static void install();
	static FullGeometrySkeletalAppearanceBatchRenderer *getInstance();

public:

	virtual void submit(SkeletalAppearance2 const &appearance);

private:

	FullGeometrySkeletalAppearanceBatchRenderer();

	// Disabled.
	FullGeometrySkeletalAppearanceBatchRenderer(FullGeometrySkeletalAppearanceBatchRenderer const&);
	FullGeometrySkeletalAppearanceBatchRenderer &operator =(FullGeometrySkeletalAppearanceBatchRenderer const&);

};

// ======================================================================

#endif
