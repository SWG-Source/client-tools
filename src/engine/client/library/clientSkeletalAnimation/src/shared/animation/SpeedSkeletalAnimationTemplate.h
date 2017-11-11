// ======================================================================
//
// SpeedSkeletalAnimationTemplate.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_SpeedSkeletalAnimationTemplate_H
#define INCLUDED_SpeedSkeletalAnimationTemplate_H

// ======================================================================

#include "clientSkeletalAnimation/SkeletalAnimationTemplate.h"

class CrcLowerString;
class Iff;
class XmlTreeNode;

// ======================================================================

class SpeedSkeletalAnimationTemplate: public SkeletalAnimationTemplate
{
public:

	static void                      install();

	static void                      setUiFactory(SkeletalAnimationTemplateUiFactory *uiFactory);

public:

	virtual SkeletalAnimation       *fetchSkeletalAnimation(AnimationEnvironment &animationEnvironment, const TransformNameMap &transformNameMap) const;

	int                              getSpeedChoiceCount() const;
	const SkeletalAnimationTemplate *fetchSpeedChoiceAnimationTemplate(int index) const;

	virtual void                     garbageCollect() const;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// Tool interface.
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	SpeedSkeletalAnimationTemplate();

	virtual bool                                      supportsWriting() const;
	virtual void                                      write(Iff &iff) const;
	virtual void                                      writeXml(IndentedFileWriter &writer) const;

	virtual const SkeletalAnimationTemplateUiFactory *getConstUiFactory() const;

	void                                              addSpeedChoiceAnimationTemplate(const SkeletalAnimationTemplate *newChoiceTemplate);
	void                                              deleteSpeedChoiceAnimationTemplate(const SkeletalAnimationTemplate *newChoiceTemplate);

private:

	typedef stdvector<const SkeletalAnimationTemplate*>::fwd  SkeletalAnimationTemplateVector;

private:

	static void                       remove();
	static SkeletalAnimationTemplate *create(const CrcLowerString &name, Iff &iff);
	static SkeletalAnimationTemplate *xmlCreate(const CrcString &name, XmlTreeNode const &treeNode, int versionNumber);

private:

	SpeedSkeletalAnimationTemplate(const CrcLowerString &name, Iff &iff);
	SpeedSkeletalAnimationTemplate(const CrcString &name, XmlTreeNode const &treeNode, int versionNumber);
	virtual ~SpeedSkeletalAnimationTemplate();

	int countWritableTemplates() const;

	void load_0000(Iff &iff);
	void loadXml_1(XmlTreeNode const &treeNode, int versionNumber);

	// disabled
	SpeedSkeletalAnimationTemplate(const SpeedSkeletalAnimationTemplate&);
	SpeedSkeletalAnimationTemplate &operator =(const SpeedSkeletalAnimationTemplate&);

private:

	static bool                                ms_installed;
	static SkeletalAnimationTemplateUiFactory *ms_uiFactory;

private:

	SkeletalAnimationTemplateVector *const m_choiceTemplates;

};

// ======================================================================

#endif
