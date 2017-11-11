// ======================================================================
//
// AnimationStateHierarchyTemplateList.h
// Copyright 2002-2004 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_AnimationStateHierarchyTemplateList_H
#define INCLUDED_AnimationStateHierarchyTemplateList_H

// ======================================================================

class AnimationStateHierarchyTemplate;
class CrcString;
class Iff;
class XmlTreeDocument;

// ======================================================================

class AnimationStateHierarchyTemplateList
{
friend class AnimationStateHierarchyTemplate;

public:

	typedef AnimationStateHierarchyTemplate *(*CreateFunction)(const CrcString &name, Iff &iff);
	typedef AnimationStateHierarchyTemplate *(*XmlCreateFunction)(const CrcString &name, XmlTreeDocument const &xmlTreeDocument);

public:

	static void install();
	static void setCreateFunction(CreateFunction createFunction);
	static void setXmlCreateFunction(XmlCreateFunction xmlCreateFunction);

	static const AnimationStateHierarchyTemplate *fetch(const CrcString &pathName);

private:

	static void  remove();
	static void  stopTracking(const AnimationStateHierarchyTemplate &latTemplate);

private:

	// disabled
	AnimationStateHierarchyTemplateList();

private:

	static bool            ms_installed;

};

// ======================================================================

#endif
