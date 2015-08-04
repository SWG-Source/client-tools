// ======================================================================
//
// ProxySkeletalAnimationTemplate.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ProxySkeletalAnimationTemplate_H
#define INCLUDED_ProxySkeletalAnimationTemplate_H

// ======================================================================

#include "clientSkeletalAnimation/SkeletalAnimationTemplate.h"

class CrcLowerString;
class MemoryBlockManager;
class Iff;
class XmlTreeNode;

// ======================================================================

class ProxySkeletalAnimationTemplate: public SkeletalAnimationTemplate
{
public:

	// Public for MBM stuff but treat as private.
	class PriorityAnimationFactory;
	class MaskedPriorityAnimationFactory;
	class UniformPriorityAnimationFactory;

public:

	static void  install();

	static void *operator new(size_t size);
	static void  operator delete(void *data);

	static void  setUiFactory(SkeletalAnimationTemplateUiFactory *uiFactory);

public:

	virtual SkeletalAnimation                        *fetchSkeletalAnimation(AnimationEnvironment &animationEnvironment, const TransformNameMap &transformNameMap) const;
	virtual const SkeletalAnimationTemplateUiFactory *getConstUiFactory() const;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// Tools interface.
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	ProxySkeletalAnimationTemplate();

	const CrcLowerString &getTargetAnimationTemplateName() const;
	void                  setTargetAnimationTemplateName(const CrcLowerString &pathName);

	// -TRF- I don't like this write interface because is forces me to store priority group names
	// as strings rather than as converted priority int values.  That means I need to hang on to
	// the group names during runtime.  I must do this because each priority value may map to
	// multiple group names, so knowing just the value is not sufficient to be able to write out
	// the group.
	void                  setUniformPriority(const CrcLowerString &uniformTransformPriorityGroup, const CrcLowerString &locomotionPriorityGroup);
	bool                  hasUniformPriority() const;
	void                  getUniformPriority(CrcLowerString &uniformTransformPriorityGroup, CrcLowerString &locomotionPriorityGroup) const;

	void                  setMaskedPriority(const CrcLowerString &maskName, const CrcLowerString &inGroupTransformPriorityGroup, const CrcLowerString &outGroupTransformPriorityGroup, const CrcLowerString &locomotionPriorityGroup);
	bool                  hasMaskedPriority() const;
	void                  getMaskedPriority(CrcLowerString &maskName, CrcLowerString &inGroupTransformPriorityGroup, CrcLowerString &outGroupTransformPriorityGroup, CrcLowerString &locomotionPriorityGroup);

	void                  setHasNoAssignedPriority();
	bool                  hasNoAssignedPriority() const;

	virtual bool          supportsWriting() const;
	virtual void          write(Iff &iff) const;
	virtual void          writeXml(IndentedFileWriter &writer) const;

	virtual void          garbageCollect() const;

private:

	static void                       remove();
	static SkeletalAnimationTemplate *create(const CrcLowerString &name, Iff &iff);
	static SkeletalAnimationTemplate *xmlCreate(const CrcString &name, XmlTreeNode const &treeNode, int versionNumber);

private:

	ProxySkeletalAnimationTemplate(const CrcLowerString &name, Iff &iff);
	ProxySkeletalAnimationTemplate(const CrcString &name, XmlTreeNode const &treeNode, int versionNumber);
	virtual                         ~ProxySkeletalAnimationTemplate();

	const SkeletalAnimationTemplate *getTargetSkeletalAnimationTemplate() const;

	void                             load_0000(Iff &iff);
	void                             loadXml_1(XmlTreeNode const &treeNode, int versionNumber);

	// disabled
	ProxySkeletalAnimationTemplate(const ProxySkeletalAnimationTemplate&);
	ProxySkeletalAnimationTemplate &operator =(const ProxySkeletalAnimationTemplate&);

private:

	static bool                                ms_installed;
	static MemoryBlockManager   *ms_memoryBlockManager;
	static SkeletalAnimationTemplateUiFactory *ms_uiFactory;

private:

	CrcLowerString                          *m_targetSkeletalAnimationTemplateName;
	mutable const SkeletalAnimationTemplate *m_targetSkeletalAnimationTemplate;

	PriorityAnimationFactory                *m_priorityAnimationFactory;

};

// ======================================================================

#endif
