// ======================================================================
//
// DirectionSkeletalAnimationTemplate.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_DirectionSkeletalAnimationTemplate_H
#define INCLUDED_DirectionSkeletalAnimationTemplate_H

// ======================================================================

#include "clientSkeletalAnimation/SkeletalAnimationTemplate.h"

class CrcLowerString;
class Iff;
class MemoryBlockManager;
class Vector;
class XmlTreeNode;

// ======================================================================

class DirectionSkeletalAnimationTemplate: public SkeletalAnimationTemplate
{
public:

	enum Magnitude
	{
		M_zero,
		M_positive,
		M_negative
	};

public:

	static void                      install();
	
	static void                     *operator new(size_t size);
	static void                      operator delete(void *data);

	static void                      setUiFactory(SkeletalAnimationTemplateUiFactory *uiFactory);

	static int                       encodeDirectionInInt(Magnitude xMagnitude, Magnitude zMagnitude);
	static void                      decodeDirectionFromInt(int encodedDirection, Magnitude &xMagnitude, Magnitude &zMagnitude);

public:

	virtual SkeletalAnimation       *fetchSkeletalAnimation(AnimationEnvironment &animationEnvironment, const TransformNameMap &transformNameMap) const;

	void                             getClosestAvailableEncodedDirections(float targetYaw, int &index1, int &index2, float &blendFactor) const;
#if 0
	int                              getClosestAvailableEncodedDirection(const Vector &directionVector) const;
#endif
	const SkeletalAnimationTemplate *fetchDirectionalAnimationTemplate(int encodedDirection) const;

	uint32                           getSupportedLocomotionDirections() const;

	virtual void                     garbageCollect() const;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// Tool interface.
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	DirectionSkeletalAnimationTemplate();

	virtual bool                     supportsWriting() const;
	virtual void                     write(Iff &iff) const;
	virtual void                     writeXml(IndentedFileWriter &writer) const;

	virtual const SkeletalAnimationTemplateUiFactory *getConstUiFactory() const;

	void                             setDirectionalAnimationTemplate(Magnitude xMagnitude, Magnitude zMagnitude, const SkeletalAnimationTemplate *animationTemplate);
	uint32							 getNumDirectionTemplates() const;

private:

	static void                       remove();
	static SkeletalAnimationTemplate *create(const CrcLowerString &name, Iff &iff);
	static SkeletalAnimationTemplate *xmlCreate(const CrcString &name, XmlTreeNode const &treeNode, int versionNumber);

private:

	typedef stdvector<const SkeletalAnimationTemplate*>::fwd  TemplateVector;

private:

	DirectionSkeletalAnimationTemplate(const CrcLowerString &name, Iff &iff);
	DirectionSkeletalAnimationTemplate(const CrcString &name, XmlTreeNode const &treeNode, int versionNumber);
	virtual ~DirectionSkeletalAnimationTemplate();

	int      getWritableDirectionalTemplateCount() const;
	void     load_0000(Iff &iff);
	void     loadXml_1(XmlTreeNode const &treeNode, int versionNumber);

	void     calculateSupportedLocomotionDirections();

	// disabled
	DirectionSkeletalAnimationTemplate(const DirectionSkeletalAnimationTemplate&);
	DirectionSkeletalAnimationTemplate &operator =(const DirectionSkeletalAnimationTemplate&);

private:

	static const uint32                        cms_supportedLocomotionDirectionMap[16];

	static bool                                ms_installed;
	static MemoryBlockManager   *ms_memoryBlockManager;
	static SkeletalAnimationTemplateUiFactory *ms_uiFactory;

private:

	TemplateVector                  *m_directionTemplates;
	uint32                           m_supportedLocomotionDirections;

};

// ======================================================================
// class DirectionSkeletalAnimationTemplate: private member functions
// ======================================================================

inline int DirectionSkeletalAnimationTemplate::encodeDirectionInInt(Magnitude xMagnitude, Magnitude zMagnitude)
{
	return (static_cast<int>(zMagnitude) << 2) | static_cast<int>(xMagnitude);
}

// ----------------------------------------------------------------------

inline void DirectionSkeletalAnimationTemplate::decodeDirectionFromInt(int encodedDirection, Magnitude &xMagnitude, Magnitude &zMagnitude)
{
	xMagnitude = static_cast<Magnitude>(encodedDirection & 0x03);
	zMagnitude = static_cast<Magnitude>((encodedDirection >> 2) & 0x03);
}

// ----------------------------------------------------------------------

inline uint32 DirectionSkeletalAnimationTemplate::getSupportedLocomotionDirections() const
{
	return m_supportedLocomotionDirections;
}

// ======================================================================

#endif
