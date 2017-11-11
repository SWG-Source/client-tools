// ======================================================================
//
// EditableBasicAnimationAction.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_EditableBasicAnimationAction_H
#define INCLUDED_EditableBasicAnimationAction_H

// ======================================================================

#include "clientSkeletalAnimation/EditableAnimationAction.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/CrcLowerString.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/MemoryBlockManagerMacros.h"

class EditableAnimationStateHierarchyTemplate;
class Iff;
class MemoryBlockManager;
class XmlTreeNode;

// ======================================================================

class EditableBasicAnimationAction: public EditableAnimationAction
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITHOUT_INSTALL;

public:

	static void install();

public:

	EditableBasicAnimationAction(EditableAnimationStateHierarchyTemplate &hierarchyTemplate, Iff &iff, int versionNumber);
	EditableBasicAnimationAction(EditableAnimationStateHierarchyTemplate &hierarchyTemplate, XmlTreeNode const &treeNode, int versionNumber);
	EditableBasicAnimationAction(const CrcLowerString &actionName, EditableAnimationStateHierarchyTemplate &hierarchyTemplate);
	//virtual ~EditableBasicAnimationAction();
	
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// inherited interface
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	virtual const CrcLowerString &getName() const;
	virtual const CrcLowerString &getLogicalAnimationName(const AnimationEnvironment &animationEnvironment) const;
	virtual bool                  shouldHideHeldItem(const AnimationEnvironment &animationEnvironment) const;
	virtual bool                  shouldApplyAnimationAsAdd(const AnimationEnvironment &animationEnvironment) const;
	virtual void                  addReferencedLogicalAnimationNames(CrcLowerStringSet &set) const;

	virtual void                  setName(const CrcLowerString &actionName);

	virtual void                  write(Iff &iff) const;
	virtual void                  writeXml(IndentedFileWriter &writer) const;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// new functionality
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	void                          setLogicalAnimationName(const CrcLowerString &animationName);
	void                          setShouldHideHeldItem(bool shouldHide);
	void                          setShouldApplyAnimationAsAdd(bool applyAsAdd);

	// Intended only for EditableMovementAnimationAction and this class, didn't want to friend.
	void                          writeXmlActionCore(IndentedFileWriter &writer, char const *elementName) const;

private:

	static void                   remove();
	static std::string            getNewActionName();

private:

	void 		                      load_0000(Iff &iff);
	void 		                      load_0001(Iff &iff);
	void 		                      load_0002(Iff &iff);

	void                              loadXml_1(XmlTreeNode const &treeNode);

	// Disabled.
	EditableBasicAnimationAction();
	EditableBasicAnimationAction(const EditableBasicAnimationAction&);
	EditableBasicAnimationAction &operator =(const EditableBasicAnimationAction&);

private:

	static const std::string  cms_defaultLogicalAnimationName;
	static const std::string  cms_baseDefaultActionName;

	static int                ms_newActionCount;

private:

	EditableAnimationStateHierarchyTemplate &m_hierarchyTemplate;
	CrcLowerString                           m_actionName;
	CrcLowerString                           m_logicalAnimationName;
	bool                                     m_hideHeldItem;
	bool                                     m_applyAnimationAsAdd;

};

// ======================================================================

#endif
