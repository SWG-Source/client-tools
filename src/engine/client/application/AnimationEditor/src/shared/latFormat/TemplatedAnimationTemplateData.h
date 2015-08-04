// ======================================================================
//
// TemplatedAnimationTemplateData.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

// ======================================================================

#include "AnimationEditor/SkeletalAnimationTemplateFactory.h"
#include <string>

// ======================================================================

template<typename T>
class TemplatedAnimationTemplateData: public SkeletalAnimationTemplateFactory::AnimationTemplateData
{
public:

	TemplatedAnimationTemplateData(const std::string &name);

	virtual const std::string         &getFriendlyClassName() const;
	virtual SkeletalAnimationTemplate *createSkeletalAnimationTemplate() const;

private:

	// disabled
	TemplatedAnimationTemplateData();
	TemplatedAnimationTemplateData(const TemplatedAnimationTemplateData&);
	TemplatedAnimationTemplateData &operator =(const TemplatedAnimationTemplateData&);

private:

	const std::string  m_name;

};

// ======================================================================

template<typename T>
inline TemplatedAnimationTemplateData<T>::TemplatedAnimationTemplateData(const std::string &name) :
	AnimationTemplateData(),
	m_name(name)
{
}

// ----------------------------------------------------------------------

template<typename T>
inline const std::string &TemplatedAnimationTemplateData<T>::getFriendlyClassName() const
{
	return m_name;
}

// ----------------------------------------------------------------------

template<typename T>
inline SkeletalAnimationTemplate *TemplatedAnimationTemplateData<T>::createSkeletalAnimationTemplate() const
{
	return new T();
}

// ======================================================================
