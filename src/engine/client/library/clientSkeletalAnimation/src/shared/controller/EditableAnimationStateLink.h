// ======================================================================
//
// EditableAnimationStateLink.h
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_EditableAnimationStateLink_H
#define INCLUDED_EditableAnimationStateLink_H

// ======================================================================

#include "clientSkeletalAnimation/AnimationStateLink.h"

class AnimationStatePath;
class CrcLowerString;
class EditableAnimationStateHierarchyTemplate;
class Iff;
class IndentedFileWriter;
class XmlTreeNode;

// ======================================================================

class EditableAnimationStateLink: public AnimationStateLink
{
public:

	EditableAnimationStateLink(EditableAnimationStateHierarchyTemplate &hierarchyTemplate, Iff &iff, int versionNumber);
	EditableAnimationStateLink(EditableAnimationStateHierarchyTemplate &hierarchyTemplate, XmlTreeNode const &treeNode, int versionNumber);
	explicit EditableAnimationStateLink(EditableAnimationStateHierarchyTemplate &hierarchyTemplate);
	virtual ~EditableAnimationStateLink();

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// inherited interface
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	virtual const AnimationStatePath &getDestinationPath() const;
	virtual bool                      hasTransitionLogicalAnimation() const;
	virtual const CrcLowerString     &getTransitionLogicalAnimationName() const;
	virtual void                      addReferencedLogicalAnimationNames(CrcLowerStringSet &set) const;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// new functionality
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	void                  setDestinationPath(const AnimationStatePath &path);
	void                  setTransitionLogicalAnimationName(const CrcLowerString &name);

	void                  write(Iff &iff) const;
	void                  writeXml(IndentedFileWriter &writer) const;

private:

	void                  load_0000(Iff &iff);
	void                  load_0001(Iff &iff);
	void                  load_0002(Iff &iff);

	void                  loadXml_1(XmlTreeNode const &treeNode);

	// disabled
	EditableAnimationStateLink(const EditableAnimationStateLink&);
	EditableAnimationStateLink &operator =(const EditableAnimationStateLink&);

private:

	EditableAnimationStateHierarchyTemplate &      m_hierarchyTemplate;
	AnimationStatePath                      *const m_destinationPath;
	CrcLowerString                          *      m_transitionLogicalAnimationName;
	
};

// ======================================================================

#endif
