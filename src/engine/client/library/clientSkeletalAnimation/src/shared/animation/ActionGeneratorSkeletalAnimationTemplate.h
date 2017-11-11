// ======================================================================
//
// ActionGeneratorSkeletalAnimationTemplate.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ActionGeneratorSkeletalAnimationTemplate_H
#define INCLUDED_ActionGeneratorSkeletalAnimationTemplate_H

// ======================================================================

class CrcLowerString;
class XmlTreeNode;

#include "clientSkeletalAnimation/SkeletalAnimationTemplate.h"

// ======================================================================

class ActionGeneratorSkeletalAnimationTemplate: public SkeletalAnimationTemplate
{
public:

	static void install();
	static void setUiFactory(SkeletalAnimationTemplateUiFactory *uiFactory);

	static bool shouldLogInfo();

public:

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// Inherited interface.
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	virtual SkeletalAnimation       *fetchSkeletalAnimation(AnimationEnvironment &animationEnvironment, const TransformNameMap &transformNameMap) const;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// Class-specific animation interface.
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	float                            getElapsedTimeUntilNextAction() const;
	const CrcLowerString            &randomSelectActionName() const;

	const SkeletalAnimationTemplate *fetchLoopingAnimationTemplate() const;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// Tool interface.
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	ActionGeneratorSkeletalAnimationTemplate();

	virtual bool                     supportsWriting() const;
	virtual void                     write(Iff &iff) const;
	virtual void                     writeXml(IndentedFileWriter &writer) const;

	void                             setMinActionElapsedTime(float value);
	void                             setMaxActionElapsedTime(float value);

	float                            getMinActionElapsedTime() const;
	float                            getMaxActionElapsedTime() const;

	void                             setLoopingSkeletalAnimationTemplate(const SkeletalAnimationTemplate *loopingTemplate);

	int                              getActionCount() const;
	const CrcLowerString            &getActionName(int index) const;

	void                             addNewAction(const CrcLowerString &actionName, float relativeWeight);
	void                             removeAction(const CrcLowerString &actionName);

	float                            getRelativeWeight(const CrcLowerString &actionName) const;
	void                             setRelativeWeight(const CrcLowerString &actionName, float relativeWeight);

	void                             changeActionName(const CrcLowerString &oldActionName, const CrcLowerString &newActionName);

	virtual const SkeletalAnimationTemplateUiFactory *getConstUiFactory() const;

	virtual void                     garbageCollect() const;

private:

	class ActionData;
	typedef stdvector<ActionData*>::fwd  ActionDataVector;

private:

	static void                       remove();
	static SkeletalAnimationTemplate *create(const CrcLowerString &name, Iff &iff);
	static SkeletalAnimationTemplate *xmlCreate(const CrcString &name, XmlTreeNode const &treeNode, int versionNumber);

private:

	ActionGeneratorSkeletalAnimationTemplate(const CrcString &name, Iff &iff);
	ActionGeneratorSkeletalAnimationTemplate(const CrcString &name, XmlTreeNode const &xmlTreeNode, int versionNumber);
	virtual ~ActionGeneratorSkeletalAnimationTemplate();

	void              load_0000(Iff &iff);

	void              loadXml_1(XmlTreeNode const &treeNode, int versionNumber);

	void              computePercentileData();

	ActionData       *findActionData(const CrcLowerString &actionName);
	const ActionData *findActionData(const CrcLowerString &actionName) const;

	// Disabled.
	ActionGeneratorSkeletalAnimationTemplate(const ActionGeneratorSkeletalAnimationTemplate&);
	ActionGeneratorSkeletalAnimationTemplate &operator =(const ActionGeneratorSkeletalAnimationTemplate&);

private:

	static bool                                ms_installed;
	static bool                                ms_logInfo;
	static SkeletalAnimationTemplateUiFactory *ms_uiFactory;

private:

	float                                   m_minActionElapsedTime;
	float                                   m_maxActionElapsedTime;

	ActionDataVector                 *const m_actionDataVector;

	const SkeletalAnimationTemplate  *      m_loopingAnimationTemplate;

};

// ======================================================================

inline bool ActionGeneratorSkeletalAnimationTemplate::shouldLogInfo()
{
	return ms_logInfo;
}

// ======================================================================

inline void ActionGeneratorSkeletalAnimationTemplate::setMinActionElapsedTime(float value)
{
	m_minActionElapsedTime = value;
}

// ----------------------------------------------------------------------

inline void ActionGeneratorSkeletalAnimationTemplate::setMaxActionElapsedTime(float value)
{
	m_maxActionElapsedTime = value;
}

// ----------------------------------------------------------------------

inline float ActionGeneratorSkeletalAnimationTemplate::getMinActionElapsedTime() const
{
	return m_minActionElapsedTime;
}

// ----------------------------------------------------------------------

inline float ActionGeneratorSkeletalAnimationTemplate::getMaxActionElapsedTime() const
{
	return m_maxActionElapsedTime;
}

// ======================================================================

#endif
