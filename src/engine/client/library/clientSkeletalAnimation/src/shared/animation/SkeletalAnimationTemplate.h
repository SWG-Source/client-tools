// ======================================================================
//
// SkeletalAnimationTemplate.h
// Copyright 2001, 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_SkeletalAnimationTemplate_H
#define INCLUDED_SkeletalAnimationTemplate_H

// ======================================================================

class AnimationEnvironment;
class Iff;
class IndentedFileWriter;
class SkeletalAnimation;
class SkeletalAnimationTemplateUiFactory;
class TransformNameMap;

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/PersistentCrcString.h"

// ======================================================================

class SkeletalAnimationTemplate
{
friend class SkeletalAnimationTemplateList;

public:

	static void  install();

public:

	const CrcString           &getName() const;

	void                       fetch() const;
	void                       release() const;

	virtual SkeletalAnimation *fetchSkeletalAnimation(AnimationEnvironment &animationEnvironment, const TransformNameMap &transformNameMap) const = 0;

	virtual bool               supportsWriting() const;
	virtual void               write(Iff &iff) const;
	virtual void               writeXml(IndentedFileWriter &writer) const;

	virtual const SkeletalAnimationTemplateUiFactory *getConstUiFactory() const;

	virtual void               garbageCollect() const;

protected:

	explicit SkeletalAnimationTemplate(const CrcString &name);
	virtual ~SkeletalAnimationTemplate() = 0;

private:

	// disabled
	SkeletalAnimationTemplate();
	SkeletalAnimationTemplate(const SkeletalAnimationTemplate&);
	SkeletalAnimationTemplate &operator =(const SkeletalAnimationTemplate&);

private:

	PersistentCrcString const  m_name;
	mutable int                m_referenceCount;

};

// ======================================================================

inline const CrcString &SkeletalAnimationTemplate::getName() const
{
	return m_name;
}

// ----------------------------------------------------------------------

inline void SkeletalAnimationTemplate::fetch() const
{
	++m_referenceCount;
}

// ======================================================================

#endif
