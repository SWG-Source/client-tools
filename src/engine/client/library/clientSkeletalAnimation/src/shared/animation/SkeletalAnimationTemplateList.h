// ======================================================================
//
// SkeletalAnimationTemplateList.h
// Copyright 2001-2004 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_SkeletalAnimationTemplateList_H
#define INCLUDED_SkeletalAnimationTemplateList_H

// ======================================================================

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Tag.h"

class CrcLowerString;
class CrcString;
class Iff;
class LessPointerComparator;
class SkeletalAnimationTemplate;
class XmlTreeNode;

// ======================================================================

class SkeletalAnimationTemplateList
{
friend class SkeletalAnimationTemplate;

public:

	typedef SkeletalAnimationTemplate *(*CreateFunction)(const CrcLowerString &name, Iff &iff);
	typedef SkeletalAnimationTemplate *(*XmlCreateFunction)(const CrcString &name, XmlTreeNode const &treeNode, int versionNumber);

public:

	static void install();

	static bool registerCreateFunction(Tag tag, CreateFunction createFunction);
	static bool deregisterCreateFunction(Tag tag);

	static void registerXmlCreateFunction(char const *elementName, XmlCreateFunction createFunction);
	static void deregisterXmlCreateFunction(char const *elementName);

	static const SkeletalAnimationTemplate *fetch(const CrcString &name);
	static const SkeletalAnimationTemplate *fetch(Iff &iff);
	static const SkeletalAnimationTemplate *fetch(XmlTreeNode const &treeNode, int versionNumber);

private:

	static void remove();

	static void  stopTracking(const SkeletalAnimationTemplate *skeletalAnimationTemplate);

private:

	typedef stdmap<const CrcString*, SkeletalAnimationTemplate*, LessPointerComparator>::fwd  TemplateMap;
	typedef stdmap<Tag, CreateFunction>::fwd                                                       CreationFunctionMap;

private:

	static bool                       ms_installed;

	static CreationFunctionMap       *ms_creationFunctions;
	static TemplateMap               *ms_namedTemplates;

};

// ======================================================================

#endif
