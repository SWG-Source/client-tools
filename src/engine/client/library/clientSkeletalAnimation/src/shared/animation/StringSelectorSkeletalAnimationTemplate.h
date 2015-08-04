// ======================================================================
//
// StringSelectorSkeletalAnimationTemplate.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_StringSelectorSkeletalAnimationTemplate_H
#define INCLUDED_StringSelectorSkeletalAnimationTemplate_H

// ======================================================================

#include "clientSkeletalAnimation/SkeletalAnimationTemplate.h"

class AnimationEnvironment;
class CrcString;
class Iff;
class SkeletalAnimationTemplate;
class XmlTreeNode;

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/CrcLowerString.h"

// ======================================================================

class StringSelectorSkeletalAnimationTemplate: public SkeletalAnimationTemplate
{
public:

	static void install();

	static void setUiFactory(SkeletalAnimationTemplateUiFactory *uiFactory);

public:

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// Inherited interface.
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	virtual SkeletalAnimation       *fetchSkeletalAnimation(AnimationEnvironment &animationEnvironment, const TransformNameMap &transformNameMap) const;
	virtual void                     garbageCollect() const;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// Engine interface.
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	const CrcLowerString            &getEnvironmentVariableName() const;

	// deprecated
	const SkeletalAnimationTemplate *fetchConstAnimationTemplateForValue(const CrcLowerString &value) const;

	int                              getSelectionAnimationTemplateCount() const;
	int                              getSelectionIndexForValue(const CrcLowerString &value) const;
	const SkeletalAnimationTemplate *fetchSelectionAnimationTemplate(int index) const;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// Tool interface.
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	StringSelectorSkeletalAnimationTemplate();

	void                             setEnvironmentVariableName(const CrcLowerString &variableName);

	int                              getSelectionValueCount(const SkeletalAnimationTemplate *selectionTemplate) const;
	const CrcLowerString            &getSelectionValue(const SkeletalAnimationTemplate *selectionTemplate, int index) const;

	bool                             hasValue(const CrcLowerString &value) const;
	void                             addSelectionAnimationTemplateMapping(const SkeletalAnimationTemplate *selectionTemplate, const CrcLowerString &value);
	void                             replaceSelectionAnimationTemplateMapping(const SkeletalAnimationTemplate *selectionTemplate, const CrcLowerString &oldValue, const CrcLowerString &newValue);
	void                             deleteSelectionAnimationTemplateMapping(const SkeletalAnimationTemplate *selectionTemplate, const CrcLowerString &value);

	void                             addSelectionAnimationTemplate(const SkeletalAnimationTemplate *selectionTemplate);
	void                             replaceSelectionAnimationTemplate(const SkeletalAnimationTemplate *oldTemplate, const SkeletalAnimationTemplate *newTemplate);
	void                             deleteSelectionAnimationTemplate(const SkeletalAnimationTemplate *selectionTemplate);

	const SkeletalAnimationTemplate *getDefaultSelectionAnimationTemplate() const;
	void                             setDefaultSelectionAnimationTemplate(const SkeletalAnimationTemplate *selectionTemplate);

	virtual bool                     supportsWriting() const;
	virtual void                     write(Iff &iff) const;
	virtual void                     writeXml(IndentedFileWriter &writer) const;

	virtual const SkeletalAnimationTemplateUiFactory *getConstUiFactory() const;

private:

	typedef stdvector<const SkeletalAnimationTemplate*>::fwd               TemplateVector;
	typedef stdmap<CrcLowerString, const SkeletalAnimationTemplate*>::fwd  ValueMap;

private:

	static void                       remove();
	static SkeletalAnimationTemplate *create(const CrcLowerString &name, Iff &iff);
	static SkeletalAnimationTemplate *xmlCreate(const CrcString &name, XmlTreeNode const &treeNode, int versionNumber);

private:

	StringSelectorSkeletalAnimationTemplate(const CrcLowerString &name, Iff &iff);
	StringSelectorSkeletalAnimationTemplate(const CrcString &name, XmlTreeNode const &treeNode, int versionNumber);
	virtual ~StringSelectorSkeletalAnimationTemplate();

	void load_0000(Iff &iff);
	void loadXml_1(XmlTreeNode const &treeNode, int versionNumber);

	void addSelectionTemplateIfNotPresent(const SkeletalAnimationTemplate *selectionTemplate);
	void removeSelectionTemplateIfUnreferenced(const SkeletalAnimationTemplate *selectionTemplate);

	// Disabled.
	StringSelectorSkeletalAnimationTemplate(const StringSelectorSkeletalAnimationTemplate&);
	StringSelectorSkeletalAnimationTemplate &operator =(const StringSelectorSkeletalAnimationTemplate&);

private:

	static bool                                ms_installed;
	static SkeletalAnimationTemplateUiFactory *ms_uiFactory;

private:

	CrcLowerString                   m_environmentVariableName;
	TemplateVector                  *m_templates;
	ValueMap                        *m_valueMap;
	const SkeletalAnimationTemplate *m_defaultTemplate;

};

// ======================================================================

inline const CrcLowerString &StringSelectorSkeletalAnimationTemplate::getEnvironmentVariableName() const
{
	return m_environmentVariableName;
}

// ----------------------------------------------------------------------

inline const SkeletalAnimationTemplate *StringSelectorSkeletalAnimationTemplate::getDefaultSelectionAnimationTemplate() const
{
	return m_defaultTemplate;
}

// ======================================================================

#endif
