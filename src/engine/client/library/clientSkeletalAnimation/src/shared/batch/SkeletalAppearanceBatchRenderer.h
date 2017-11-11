// ======================================================================
//
// SkeletalAppearanceBatchRenderer.h
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_SkeletalAppearanceBatchRenderer_H
#define INCLUDED_SkeletalAppearanceBatchRenderer_H

// ======================================================================

class SkeletalAppearance2;

// ======================================================================

class SkeletalAppearanceBatchRenderer
{
public:

	virtual ~SkeletalAppearanceBatchRenderer();

	virtual void submit(SkeletalAppearance2 const &appearance) = 0;

protected:

	SkeletalAppearanceBatchRenderer();

private:

	// Disabled.
	SkeletalAppearanceBatchRenderer(SkeletalAppearanceBatchRenderer const &);
	SkeletalAppearanceBatchRenderer &operator =(SkeletalAppearanceBatchRenderer const&);

};

// ======================================================================

#endif
