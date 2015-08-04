// ======================================================================
//
// SlotCustomizationVariable.cpp
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/SlotCustomizationVariable.h"

#include "clientGame/ContainerInterface.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/SlotRule.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedObject/CustomizationData.h"


// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(SlotCustomizationVariable, true, 0, 0, 0);

// ======================================================================
// class SlotCustomizationVariable: PUBLIC
// ======================================================================

SlotCustomizationVariable::SlotCustomizationVariable(SlotRule const *slotRule) :
	RangedIntCustomizationVariable(),
	m_slotRule(slotRule),
	m_ruleState(false)
{
	NOT_NULL(slotRule);
}

// ----------------------------------------------------------------------

SlotCustomizationVariable::~SlotCustomizationVariable()
{
	m_slotRule = 0;
}

// ----------------------------------------------------------------------

bool SlotCustomizationVariable::doesVariablePersist() const
{
	return false;
}

// ----------------------------------------------------------------------

int SlotCustomizationVariable::getValue() const
{
	// If the rule is true, set variable on full blast; otherwise, set to nothing.
	return m_ruleState ? 255 : 0;
}

// ----------------------------------------------------------------------

bool SlotCustomizationVariable::setValue(int value)
{
	DEBUG_WARNING(true, ("ignoring setValue() call on procedurally generated SlotCustomizationVariable."));
	UNREF(value);
	return false;
}

// ----------------------------------------------------------------------

void SlotCustomizationVariable::getRange(int &minRangeInclusive, int &maxRangeExclusive) const
{
	minRangeInclusive = 0;
	maxRangeExclusive = 256;
}

// ----------------------------------------------------------------------

void SlotCustomizationVariable::alter(CustomizationData &owner)
{
	//-- Evaluate the rule.
	Object &ownerObject = owner.getOwnerObject();
	SlottedContainer *const slottedContainer = ContainerInterface::getSlottedContainer(ownerObject);
	if (!slottedContainer)
	{
		DEBUG_WARNING(true, ("SlotCustomizationVariable: could not evaluate varaiable because owner object does not have a slotted container."));
		return;
	}
	
	SlottedContainer * alternateContainer = NULL;
	CreatureObject * creatureOwner = dynamic_cast<CreatureObject *>(&ownerObject);
	if(creatureOwner && creatureOwner->getAppearanceInventoryObject())
		alternateContainer = ContainerInterface::getSlottedContainer(*creatureOwner->getAppearanceInventoryObject());

	bool const oldState = m_ruleState;

	NOT_NULL(m_slotRule);
	m_ruleState = m_slotRule->test(*slottedContainer, alternateContainer, creatureOwner);

	// If they differ, signal that the variable changed so associated
	// appearance data can update itself appropriately.
	if ((oldState && !m_ruleState) || (!oldState && m_ruleState))
		signalVariableModified();
}

// ----------------------------------------------------------------------

void SlotCustomizationVariable::writeObjectTemplateExportString(const std::string & /* variablePathName */, ObjectTemplateCustomizationDataWriter & /* writer */) const
{
	WARNING(true, ("SlotCustomizationVariable::writeObjectTemplateExportString() called on procedural variable that does not persist."));
}

// ----------------------------------------------------------------------

#ifdef _DEBUG

std::string SlotCustomizationVariable::debugToString() const
{
	char buffer[256];
	IGNORE_RETURN(snprintf(buffer, sizeof(buffer) - 1, "<SlotCustomizationVariable> [0, %d, 256)", m_ruleState ? 255 : 0));
	buffer[sizeof(buffer) - 1] = '\0';

	return buffer;
}

#endif

// ======================================================================
