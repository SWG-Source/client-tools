// ======================================================================
//
// PriorityBlendAnimationTemplate.h
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_PriorityBlendAnimationTemplate_H
#define INCLUDED_PriorityBlendAnimationTemplate_H

// ======================================================================

class AnimationEnvironment;
class CrcLowerString;
class Iff;
class SkeletalAnimation;
class TransformNameMap;
class XmlTreeNode;

#include "clientSkeletalAnimation/SkeletalAnimationTemplate.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/MemoryBlockManagerMacros.h"

// ======================================================================

class PriorityBlendAnimationTemplate: public SkeletalAnimationTemplate
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITHOUT_INSTALL;

public:

	enum
	{
		MAX_ANIMATION_COUNT = 2
	};

public:

	static void install();
	static void setUiFactory(SkeletalAnimationTemplateUiFactory *uiFactory);

public:

	virtual SkeletalAnimation *fetchSkeletalAnimation(AnimationEnvironment &animationEnvironment, const TransformNameMap &transformNameMap) const;
	int                        getPrimaryComponentAnimationIndex() const;

	// Editor interface: runtime users should not need to call this.
	PriorityBlendAnimationTemplate();
	virtual bool                     supportsWriting() const;
	virtual void                     write(Iff &iff) const;
	virtual void                     writeXml(IndentedFileWriter &writer) const;

	SkeletalAnimationTemplate const *fetchComponentAnimationTemplate(int componentAnimationIndex) const;
	void                             setComponentAnimationTemplate(int componentAnimationIndex, SkeletalAnimationTemplate const *componentAnimationTemplate);
	void                             setPrimaryComponentAnimationIndex(int index);

	virtual SkeletalAnimationTemplateUiFactory const *getConstUiFactory() const;

private:

	static void remove();
	static SkeletalAnimationTemplate *create(const CrcLowerString &name, Iff &iff);
	static SkeletalAnimationTemplate *xmlCreate(const CrcString &name, XmlTreeNode const &treeNode, int versionNumber);

private:

	PriorityBlendAnimationTemplate(CrcLowerString const &name, Iff &iff);
	PriorityBlendAnimationTemplate(CrcString const &name, XmlTreeNode const &treeNode, int versionNumber);
	virtual ~PriorityBlendAnimationTemplate();
	
	void               load_0000(Iff &iff);
	void               loadXml_1(XmlTreeNode const &treeNode, int versionNumber);
	SkeletalAnimation *fetchNewComponentAnimation(int index, AnimationEnvironment &animationEnvironment, const TransformNameMap &transformNameMap) const;

	PriorityBlendAnimationTemplate(PriorityBlendAnimationTemplate const&);
	PriorityBlendAnimationTemplate &operator =(PriorityBlendAnimationTemplate const&);

private:

	int                              m_primaryComponentAnimationIndex;
	SkeletalAnimationTemplate const *m_componentAnimationTemplateArray[MAX_ANIMATION_COUNT];

};

// ======================================================================

#endif
