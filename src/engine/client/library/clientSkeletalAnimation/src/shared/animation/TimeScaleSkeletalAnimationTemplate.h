// ======================================================================
//
// TimeScaleSkeletalAnimationTemplate.h
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_TimeScaleSkeletalAnimationTemplate_H
#define INCLUDED_TimeScaleSkeletalAnimationTemplate_H

// ======================================================================

class CrcLowerString;
class MemoryBlockManager;
class XmlTreeNode;

#include "clientSkeletalAnimation/SkeletalAnimationTemplate.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/MemoryBlockManagerMacros.h"

// ======================================================================

class TimeScaleSkeletalAnimationTemplate: public SkeletalAnimationTemplate
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITHOUT_INSTALL;

public:

	static void install();
	static void setUiFactory(SkeletalAnimationTemplateUiFactory *uiFactory);

public:

	virtual SkeletalAnimation                        *fetchSkeletalAnimation(AnimationEnvironment &animationEnvironment, const TransformNameMap &transformNameMap) const;

	virtual bool                                      supportsWriting() const;
	virtual void                                      write(Iff &iff) const;
	virtual void                                      writeXml(IndentedFileWriter &writer) const;

	virtual const SkeletalAnimationTemplateUiFactory *getConstUiFactory() const;

	void                                              setTimeScale(float timeScale);
	float                                             getTimeScale() const;
	float                                             getOneOverTimeScale() const;

	void                                              setBaseAnimationTemplate(SkeletalAnimationTemplate const *animationTemplate);
	SkeletalAnimationTemplate const                  *fetchBaseAnimationTemplate() const;

	virtual void                                      garbageCollect() const;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// Tools interface --- don't use these functions in a game
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	TimeScaleSkeletalAnimationTemplate();

protected:

	virtual ~TimeScaleSkeletalAnimationTemplate();

private:

	static void                       remove();
	static SkeletalAnimationTemplate *create(const CrcLowerString &name, Iff &iff);
	static SkeletalAnimationTemplate *xmlCreate(const CrcString &name, XmlTreeNode const &treeNode, int versionNumber);

private:

	TimeScaleSkeletalAnimationTemplate(CrcLowerString const &name, Iff &iff);
	TimeScaleSkeletalAnimationTemplate(CrcString const &name, XmlTreeNode const &treeNode, int versionNumber);

	void load_0000(Iff &iff);
	void loadXml_1(XmlTreeNode const &treeNode, int versionNumber);

	// Disabled.
	TimeScaleSkeletalAnimationTemplate(const TimeScaleSkeletalAnimationTemplate&);
	TimeScaleSkeletalAnimationTemplate &operator =(const TimeScaleSkeletalAnimationTemplate&);

private:

	float                            m_timeScale;
	float                            m_oneOverTimeScale;
	SkeletalAnimationTemplate const *m_baseAnimationTemplate;

};

// ======================================================================

#endif
