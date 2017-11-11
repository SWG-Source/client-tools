// ======================================================================
//
// LogicalAnimationTableTemplate.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_LogicalAnimationTableTemplate_H
#define INCLUDED_LogicalAnimationTableTemplate_H

// ======================================================================

class AnimationEnvironment;
class AnimationStateHierarchyTemplate;
class AnimationStatePath;
class Appearance;
class ConstCharCrcLowerString;
class CrcLowerString;
class Iff;
class LogicalAnimationTableTemplateList;
class SkeletalAnimationTemplate;
class TransformAnimationController;
class TransformNameMap;
class XmlTreeDocument;
class XmlTreeNode;

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/MemoryBlockManagerMacros.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/PersistentCrcString.h"
#include <vector>


// ======================================================================

class LogicalAnimationTableTemplate
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITHOUT_INSTALL;

friend class LogicalAnimationTableTemplateList;

public:

	class AnimationEntry;

public:

	static void                            install();

public:

	void                                   fetch() const;
	void                                   release() const;
	int                                    getReferenceCount() const;

	const CrcString                       &getName() const;

	SkeletalAnimationTemplate const      *fetchConstAnimationTemplate(CrcString const &logicalAnimationName) const;

	const AnimationStateHierarchyTemplate *fetchConstHierarchyTemplate() const;
	TransformAnimationController          *createAnimationController(AnimationEnvironment &animationEnvironment, const TransformNameMap &transformNameMap, Appearance *ownerAppearance, int channel, AnimationStatePath const &initialPath) const;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// Tool-related 
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	explicit LogicalAnimationTableTemplate(const AnimationStateHierarchyTemplate &hierarchyTemplate);

	void                                   createUnspecifiedEntries();
	void                                   removeEntriesNotSpecifiedInAsh();

	int                                    getAnimationCount() const;
	const CrcLowerString                  &getLogicalAnimationName(int index) const;
	SkeletalAnimationTemplate             *fetchAnimationTemplateByIndex(int index);

	bool                                   hasLogicalAnimationName(const CrcLowerString &logicalAnimationName) const;

	void                                   setAnimationTemplate(const CrcLowerString &logicalAnimationName, const SkeletalAnimationTemplate *animationTemplate);

	void                                   write(Iff &iff) const;
	bool                                   writeXml(char const *filename) const;

	void                                   garbageCollect();

private:

	typedef stdvector<AnimationEntry*>::fwd  AnimationEntryVector;

	typedef stdvector<int>::fwd              IntVector;

private:

	static void  remove();

private:

	LogicalAnimationTableTemplate(const CrcString &name, Iff &iff);
	LogicalAnimationTableTemplate(const CrcString &name, XmlTreeDocument const &xmlTreeDocument);
	~LogicalAnimationTableTemplate();

	int   countWritableAnimationEntries() const;

	void  load_0000(Iff &iff);

	void  loadXml_1(XmlTreeNode const &treeNode);

	void  debugDump() const;

	std::string  getProperAshReferenceName() const;

	// disabled
	LogicalAnimationTableTemplate();
	LogicalAnimationTableTemplate(const LogicalAnimationTableTemplate&);
	LogicalAnimationTableTemplate &operator =(const LogicalAnimationTableTemplate&);

private:

	static const ConstCharCrcLowerString          cms_defaultEntryName;

private:

	AnimationStateHierarchyTemplate const *       m_hierarchyTemplate;
	AnimationEntryVector                          m_animationEntries;
	int                                           m_defaultAnimationEntryIndex;

	mutable int                                   m_referenceCount;
	PersistentCrcString const                     m_name;
};

// ======================================================================

inline void LogicalAnimationTableTemplate::fetch() const
{
	DEBUG_FATAL(m_referenceCount < 0, ("trying to revive an already-deleted reference."));
	++m_referenceCount;
}

// ----------------------------------------------------------------------

inline int LogicalAnimationTableTemplate::getReferenceCount() const
{
	return m_referenceCount;
}

// ----------------------------------------------------------------------

inline const CrcString &LogicalAnimationTableTemplate::getName() const
{
	return m_name;
}

// ======================================================================

#endif
