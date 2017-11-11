// ======================================================================
//
// TransformMaskList.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_TransformMaskList_H
#define INCLUDED_TransformMaskList_H

// ======================================================================

class CrcLowerString;
class LessPointerComparator;
class TransformMask;

// ======================================================================

class TransformMaskList
{
friend class TransformMask;

public:

	static void  install();

	static const TransformMask *fetch(const CrcLowerString &pathName);

private:

	typedef stdmap<const CrcLowerString*, TransformMask*, LessPointerComparator>::fwd  NamedTemplateMap;

private:

	static void  remove();
	static void  stopTracking(const TransformMask &mask);

private:

	static bool              ms_installed;
	static NamedTemplateMap *ms_templates;
	static TransformMask    *ms_defaultTemplate;

};

// ======================================================================

#endif
