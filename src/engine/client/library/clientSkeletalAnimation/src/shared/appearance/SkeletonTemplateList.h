// ======================================================================
//
// SkeletonTemplateList.h
// Copyright 2001, 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_SkeletonTemplateList_H
#define INCLUDED_SkeletonTemplateList_H

// ==================================================================

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Tag.h"

class CrcString;
class Iff;
class LessPointerComparator;
class SkeletonTemplate;

// ==================================================================

class SkeletonTemplateList
{
friend class SkeletonTemplate;

public:

	typedef SkeletonTemplate *(*CreateFunction)(Iff &iff, CrcString const &filename);

public:

	static void  install();

	static void  registerCreateFunction(Tag tag, CreateFunction function);
	static void  deregisterCreateFunction(Tag tag);

	static const SkeletonTemplate *fetch(CrcString const &pathName);
	static const SkeletonTemplate *fetch(Iff &iff);

private:

	static void  remove();

	static void  stopTracking(const SkeletonTemplate &skeletonTemplate);

private:

	typedef stdmap<CrcString const*, SkeletonTemplate*, LessPointerComparator>::fwd       TemplateMap;
	typedef stdmap<Tag, CreateFunction>::fwd                                              CreateFunctionMap;

private:

	static bool               ms_installed;
	static CreateFunctionMap *ms_createFunctionMap;
	static TemplateMap       *ms_templateMap;

};

// ==================================================================

#endif
