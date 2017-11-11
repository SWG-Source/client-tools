// ======================================================================
//
// SlotRuleManager.h
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_SlotRuleManager_H
#define INCLUDED_SlotRuleManager_H

// ======================================================================

class CustomizationData;
class SlotId;

// ======================================================================

class SlotRuleManager
{
public:

	static void          install(char const *dataFilename);

	static int           getRuleCount();

	static SlotId const &getSlotRequiredForRule(int ruleIndex);
	static void          addCustomizationVariableForRule(int ruleIndex, CustomizationData &customizationData);

};

// ======================================================================

#endif
