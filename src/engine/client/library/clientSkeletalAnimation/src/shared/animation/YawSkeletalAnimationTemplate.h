// ======================================================================
//
// YawSkeletalAnimationTemplate.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_YawSkeletalAnimationTemplate_H
#define INCLUDED_YawSkeletalAnimationTemplate_H

// ======================================================================

#include "clientSkeletalAnimation/SkeletalAnimationTemplate.h"

class CrcLowerString;
class Iff;
class XmlTreeNode;

// ======================================================================
/**
 * A SkeletalAnimationTemplate that supports playing a specific animation
 * when yawing clockwise, a different animation when yawing counterclockwise, 
 * and a different animation when not yawing.
 *
 * This animation type is used to support creatures turning in a more visually appealing
 * manner.  Without this animation type, creatures (or players) turning
 * simply rotate in place with nothing other than the idle played on them.
 * With this animation type, during a rotate left or rotate right, a
 * feet-shuffling animation will be played.  When not rotating, the
 * non-rotating animation will be played.
 *
 * Expected usage: this type of animation is added under the zero speed
 * entry for a speed selector.  The turn clockwise plays something that
 * looks like the user is rotating right.  The animation does not actually
 * contain master node rotation --- the creature controller already handles
 * rotation via code.  The turn counterclockwise plays something different
 * that makes the creature look like it is turning left.  The non-rotation
 * animation plays the normal idle.
 *
 * Initially I was going to restrict this animation type to use the
 * rotations only when the velocity was zero.  However, the proper placement
 * for this animation type is at the zero speed on a speed selector, so
 * there is no need to force a zero-speed velocity check.  In addition,
 * this node type becomes more flexible as it allows non-zero-speed
 * animations to be played for rotationing left and right while moving.
 * That might prove to be completely useless, but at least we have the
 * flexibility to do so if we have a use for it.
 */

class YawSkeletalAnimationTemplate: public SkeletalAnimationTemplate
{
public:

	static void                      install();
	static void                      setUiFactory(SkeletalAnimationTemplateUiFactory *uiFactory);

public:

	virtual SkeletalAnimation       *fetchSkeletalAnimation(AnimationEnvironment &animationEnvironment, const TransformNameMap &transformNameMap) const;
	virtual void                     garbageCollect() const;

	const SkeletalAnimationTemplate *fetchYawLeftAnimationTemplate() const;
	const SkeletalAnimationTemplate *fetchYawRightAnimationTemplate() const;
	const SkeletalAnimationTemplate *fetchNoYawAnimationTemplate() const;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// Tool interface.
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	YawSkeletalAnimationTemplate();

	virtual bool                                      supportsWriting() const;
	virtual void                                      write(Iff &iff) const;
	virtual void                                      writeXml(IndentedFileWriter &writer) const;

	virtual const SkeletalAnimationTemplateUiFactory *getConstUiFactory() const;

	void                                              setYawLeftAnimationTemplate(const SkeletalAnimationTemplate *animationTemplate);
	void                                              setYawRightAnimationTemplate(const SkeletalAnimationTemplate *animationTemplate);
	void                                              setNoYawAnimationTemplate(const SkeletalAnimationTemplate *animationTemplate);

private:

	typedef stdvector<const SkeletalAnimationTemplate*>::fwd  SkeletalAnimationTemplateVector;

private:

	static void                       remove();
	static SkeletalAnimationTemplate *create(const CrcLowerString &name, Iff &iff);
	static SkeletalAnimationTemplate *xmlCreate(const CrcString &name, XmlTreeNode const &treeNode, int versionNumber);

private:

	YawSkeletalAnimationTemplate(const CrcLowerString &name, Iff &iff);
	YawSkeletalAnimationTemplate(const CrcString &name, XmlTreeNode const &treeNode, int versionNumber);
	virtual ~YawSkeletalAnimationTemplate();

	void load_0000(Iff &iff);
	void loadXml_1(XmlTreeNode const &treeNode, int versionNumber);

	// Disabled.
	YawSkeletalAnimationTemplate(const YawSkeletalAnimationTemplate&);
	YawSkeletalAnimationTemplate &operator =(const YawSkeletalAnimationTemplate&);

private:

	static bool                                ms_installed;
	static SkeletalAnimationTemplateUiFactory *ms_uiFactory;

private:

	const SkeletalAnimationTemplate *m_yawLeftAnimationTemplate;
	const SkeletalAnimationTemplate *m_yawRightAnimationTemplate;
	const SkeletalAnimationTemplate *m_noYawAnimationTemplate;

};

// ======================================================================

#endif
