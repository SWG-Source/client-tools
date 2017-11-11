// ======================================================================
//
// EditableAnimationStateHierarchyTemplate.h
// Copyright 2002-2004 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_EditableAnimationStateHierarchyTemplate_H
#define INCLUDED_EditableAnimationStateHierarchyTemplate_H

// ======================================================================

#include "clientSkeletalAnimation/AnimationStateHierarchyTemplate.h"

class EditableAnimationActionGroup;
class EditableAnimationState;
class Iff;
class XmlTreeDocument;
class XmlTreeNode;

// ======================================================================

class EditableAnimationStateHierarchyTemplate: public AnimationStateHierarchyTemplate
{
public:

	static void install();

public:

	EditableAnimationStateHierarchyTemplate();

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// inherited interface
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	virtual const AnimationState       *getConstRootAnimationState() const;

	virtual int                         getAnimationActionGroupCount() const;
	virtual const AnimationActionGroup &getConstAnimationActionGroup(int index) const;
	virtual bool                        lookupAnimationActionGroupIndex(const CrcLowerString &groupName, int &index) const;

	virtual int                         getLogicalAnimationCount() const;
	virtual const CrcLowerString       &getLogicalAnimationName(int index) const;
	virtual bool                        lookupLogicalAnimationIndex(const CrcLowerString &animationName, int &index) const;

	virtual void                        addReferencedLogicalAnimationNames(CrcLowerStringSet &set) const;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// new functionality
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	EditableAnimationState             *getRootAnimationState();

	EditableAnimationActionGroup       &getAnimationActionGroup(int index);
	void                                addAnimationActionGroup(EditableAnimationActionGroup *group);
	void                                deleteAnimationActionGroup(const CrcLowerString &groupName);

	bool                                write(const std::string &outputPathName);
	bool                                writeXml(char const *outputPathName) const;

private:

	typedef stdvector<EditableAnimationActionGroup*>::fwd  EditableAnimationActionGroupVector;
	typedef stdvector<CrcLowerString>::fwd                 CrcLowerStringVector;

private:

	static AnimationStateHierarchyTemplate *create(const CrcString &pathName, Iff &iff);
	static AnimationStateHierarchyTemplate *xmlCreate(CrcString const &pathName, XmlTreeDocument const &xmlTreeDocument);

private:

	EditableAnimationStateHierarchyTemplate(const CrcString &name, Iff &iff);
	EditableAnimationStateHierarchyTemplate(const CrcString &name, XmlTreeDocument const &xmlTreeDocument);
	virtual ~EditableAnimationStateHierarchyTemplate();

	void    load_0000(Iff &iff);
	void    load_0001(Iff &iff);
	void    load_0002(Iff &iff);

	void    loadXml_1(XmlTreeNode const &treeNode);

	// disabled
	EditableAnimationStateHierarchyTemplate(const EditableAnimationStateHierarchyTemplate&);
	EditableAnimationStateHierarchyTemplate &operator =(const EditableAnimationStateHierarchyTemplate&);

private:

	static const int  cms_maxIffSize;

private:

	EditableAnimationState             *       m_rootState;
	EditableAnimationActionGroupVector *const  m_actionGroups;
	CrcLowerStringVector               *       m_logicalAnimationNames;

};

// ======================================================================

inline EditableAnimationState *EditableAnimationStateHierarchyTemplate::getRootAnimationState()
{
	return m_rootState;
}

// ======================================================================

#endif
