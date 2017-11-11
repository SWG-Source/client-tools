// ======================================================================
//
// SlotCustomizationVariable.h
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_SlotCustomizationVariable_H
#define INCLUDED_SlotCustomizationVariable_H

// ======================================================================

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/MemoryBlockManagerMacros.h"
#include "sharedObject/RangedIntCustomizationVariable.h"

class MemoryBlockManager;
class SlotRule;

// ======================================================================

class SlotCustomizationVariable: public RangedIntCustomizationVariable
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

public:

	explicit SlotCustomizationVariable(SlotRule const *slotRule);
	virtual ~SlotCustomizationVariable();

	virtual bool         doesVariablePersist() const;

	virtual int          getValue() const;
	virtual bool         setValue(int value);

	virtual void         getRange(int &minRangeInclusive, int &maxRangeExclusive) const;

	virtual void         alter(CustomizationData &owner);

	virtual void         writeObjectTemplateExportString(const std::string &variablePathName, ObjectTemplateCustomizationDataWriter &writer) const;

#ifdef _DEBUG
	virtual std::string  debugToString() const;
#endif

private:

	// Disabled.
	SlotCustomizationVariable(SlotCustomizationVariable const&);
	SlotCustomizationVariable &operator =(SlotCustomizationVariable const&);

private:

	SlotRule const *m_slotRule;
	bool            m_ruleState;

};

// ======================================================================

#endif
