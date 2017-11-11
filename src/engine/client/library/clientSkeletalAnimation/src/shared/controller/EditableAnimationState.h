// ======================================================================
//
// EditableAnimationState.h
// Copyright 2002-2004 Sony Online Entertainment, Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_EditableAnimationState_H
#define INCLUDED_EditableAnimationState_H

// ======================================================================

#include "clientSkeletalAnimation/AnimationState.h"
#include "clientSkeletalAnimation/AnimationStatePath.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/PersistentCrcString.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/MemoryBlockManagerMacros.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Tag.h"

#include <vector>

class CrcString;
class EditableAnimationAction;
class EditableAnimationStateHierarchyTemplate;
class EditableAnimationStateLink;
class Iff;
class IndentedFileWriter;
class MemoryBlockManager;
class XmlTreeNode;

// ======================================================================

class EditableAnimationState: public AnimationState
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

public:

	EditableAnimationState(EditableAnimationStateHierarchyTemplate &hierarchyTemplate, Iff &iff, int versionNumber);
	EditableAnimationState(EditableAnimationStateHierarchyTemplate &hierarchyTemplate, XmlTreeNode const &treeNode, int versionNumber);
	EditableAnimationState(const AnimationStateNameId &stateNameId, EditableAnimationStateHierarchyTemplate &hierarchyTemplate);
	virtual ~EditableAnimationState();

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// inherited interface
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	virtual const AnimationStatePath     &getStatePath() const;
	virtual const AnimationStateNameId    getStateNameId() const;

	virtual const AnimationState         *getConstParentState() const;

	virtual int                           getChildStateCount() const;
	virtual const AnimationState         &getConstChildState(int index) const;
	virtual bool                          lookupChildStateIndex(const AnimationStateNameId &childNameId, int &index) const;

	virtual CrcString const              &getLogicalAnimationName() const;

	virtual int                           getLinkCount() const;
	virtual const AnimationStateLink     &getConstLink(int index) const;

	virtual int                           getActionCount() const;
	virtual const AnimationAction        &getConstAction(int index) const;

	virtual int                           getGrantedActionGroupCount() const;
	virtual int                           getGrantedActionGroupIndex(int index) const;

	virtual int                           getDeniedActionGroupCount() const;
	virtual int                           getDeniedActionGroupIndex(int index) const;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// new functionality
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	EditableAnimationStateHierarchyTemplate &getHierarchyTemplate();

	void                                  setStateNameId(const AnimationStateNameId &nameId);
	CrcString const                      &getStateName() const;

	EditableAnimationState               *getParentState();
	void                                  setParentState(EditableAnimationState *parentState);

	EditableAnimationState               &getChildState(int index);
	void                                  addChildState(EditableAnimationState *child);
	void                                  removeChildState(EditableAnimationState *child);
	void                                  removeAllChildStates();

	void                                  setLogicalAnimationName(CrcString const &name);

	EditableAnimationStateLink           &getLink(int index);
	void                                  addLink(EditableAnimationStateLink *link);
	void                                  removeLink(EditableAnimationStateLink *link);

	EditableAnimationAction              &getAction(int index);
	void                                  addAction(EditableAnimationAction *action);
	void                                  removeAction(EditableAnimationAction *action);

	void                                  addGrantedActionGroupIndex(int index);
	void                                  deleteGrantedActionGroupIndex(int index);

	void                                  addDeniedActionGroupIndex(int index);
	void                                  deleteDeniedActionGroupIndex(int index);

	virtual void                          addReferencedLogicalAnimationNames(CrcLowerStringSet &set) const;

	void                                  write(Iff &iff) const;
	void                                  writeXml(IndentedFileWriter &writer) const;

private:

	typedef stdvector<int>::fwd                          IntVector;
	typedef stdvector<EditableAnimationAction*>::fwd     EditableAnimationActionVector;
	typedef stdvector<EditableAnimationStateLink*>::fwd  EditableAnimationStateLinkVector;
	typedef stdvector<EditableAnimationState*>::fwd      EditableAnimationStateVector;
	
	struct LessNameComparator;

private:

	void buildStatePath(AnimationStatePath &path) const;
	void writeActionGroup(Iff &iff, const IntVector &actionGroupNames, const Tag &destTag) const;
	void load_0000(Iff &iff, int versionNumber);
	void load_0001(Iff &iff, int versionNumber);
	void load_0002(Iff &iff, int versionNumber);

	void loadXml_1(XmlTreeNode const &treeNode);

	void writeXmlActions(IndentedFileWriter &writer) const;
	void writeXmlLinks(IndentedFileWriter &writer) const;
	void writeXmlChildStates(IndentedFileWriter &writer) const;

	// disabled
	EditableAnimationState();
	EditableAnimationState(const EditableAnimationState&);
	EditableAnimationState &operator =(const EditableAnimationState&);

private:

	EditableAnimationStateHierarchyTemplate &m_hierarchyTemplate;

	AnimationStateNameId                    *m_stateNameId;
	mutable AnimationStatePath               m_statePath;

	EditableAnimationState                  *m_parentState;
	EditableAnimationStateVector             m_childStates;

	PersistentCrcString                      m_logicalAnimationName;

	EditableAnimationStateLinkVector         m_stateLinks;

	EditableAnimationActionVector           *m_actions;
	IntVector                               *m_grantedActionGroupIndices;
	IntVector                               *m_deniedActionGroupIndices;

};

// ======================================================================

inline EditableAnimationStateHierarchyTemplate &EditableAnimationState::getHierarchyTemplate()
{
	return m_hierarchyTemplate;
}

// ======================================================================

#endif
