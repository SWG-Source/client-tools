// ======================================================================
//
// SlotRuleManager.cpp
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/SlotRuleManager.h"

#include "clientGame/SlotCustomizationVariable.h"
#include "clientGame/SlotRule.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/SlotId.h"
#include "sharedObject/SlotIdManager.h"

#include <algorithm>
#include <vector>

// ======================================================================

namespace SlotRuleManagerNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class RuleBinding
	{
	public:

		RuleBinding(SlotId const &requiredSlot, char const *variableName, SlotRule *slotRule);
		~RuleBinding();

		SlotId const      &getRequiredSlot() const;
		std::string const &getVariableName() const;
		SlotRule *const    getSlotRule() const;

	private:

		// Disabled.
		RuleBinding();
	
	private:

		SlotId       m_requiredSlot;
		std::string  m_variableName;
		SlotRule    *m_slotRule;

	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	typedef std::vector<RuleBinding*>  RuleBindingVector;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void  remove();
	void  load(Iff &iff);
	void  load_0000(Iff &iff);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	Tag const TAG_BIND = TAG(B,I,N,D);
	Tag const TAG_SRMG = TAG(S,R,M,G);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool               s_installed;
	RuleBindingVector  s_ruleBindings;
}

using namespace SlotRuleManagerNamespace;

// ======================================================================
// class SlotRuleManagerNamespace::RuleBinding
// ======================================================================

SlotRuleManagerNamespace::RuleBinding::RuleBinding(SlotId const &requiredSlot, char const *variableName, SlotRule *slotRule) :
	m_requiredSlot(requiredSlot),
	m_variableName(variableName),
	m_slotRule(slotRule)
{
}

// ----------------------------------------------------------------------

SlotRuleManagerNamespace::RuleBinding::~RuleBinding()
{
	delete m_slotRule;
}

// ----------------------------------------------------------------------

inline SlotId const &SlotRuleManagerNamespace::RuleBinding::getRequiredSlot() const
{
	return m_requiredSlot;
}

// ----------------------------------------------------------------------

inline std::string const &SlotRuleManagerNamespace::RuleBinding::getVariableName() const
{
	return m_variableName;
}

// ----------------------------------------------------------------------

inline SlotRule *const SlotRuleManagerNamespace::RuleBinding::getSlotRule() const
{
	return m_slotRule;
}

// ======================================================================
// namespace SlotRuleManagerNamespace
// ======================================================================

void SlotRuleManagerNamespace::remove()
{
	DEBUG_FATAL(!s_installed, ("SlotRuleManager not installed."));
	s_installed = false;

	std::for_each(s_ruleBindings.begin(), s_ruleBindings.end(), PointerDeleter());
	RuleBindingVector().swap(s_ruleBindings);
}

// ----------------------------------------------------------------------

void SlotRuleManagerNamespace::load(Iff &iff)
{
	iff.enterForm(TAG_SRMG);

		Tag const version = iff.getCurrentName();
		switch (version)
		{
			case TAG_0000:
				load_0000(iff);
				break;

			default:
				{
					char buffer[5];
					ConvertTagToString(version, buffer);
					FATAL(true, ("SlotRuleManager: unsupported file version [%s].", buffer));
				}
		}

	iff.exitForm(TAG_SRMG);
}

// ----------------------------------------------------------------------

void SlotRuleManagerNamespace::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);

		//-- Load each rule.
		while (!iff.atEndOfForm())
		{
			iff.enterForm(TAG_BIND);

				iff.enterChunk(TAG_INFO);
					
					//-- Get the required slot for this rule application.
					char requiredSlotName[256];
					iff.read_string(requiredSlotName, sizeof(requiredSlotName) - 1);
					requiredSlotName[sizeof(requiredSlotName) - 1] = '\0';

					//-- Get the name of the variable that this rule will drive.
					char variableName[256];
					iff.read_string(variableName, sizeof(variableName) - 1);
					variableName[sizeof(variableName) - 1] = '\0';

					//-- Convert slot name to slot id.
					SlotId const slotId = SlotIdManager::findSlotId(CrcLowerString(requiredSlotName));
					FATAL(slotId == SlotId::invalid, ("SlotRuleManager: initialization file specified non-existing slot name [%s].", requiredSlotName));

				iff.exitChunk(TAG_INFO);

				//-- Load the slot rule.
				SlotRule *const rule = SlotRule::create(iff);
				NOT_NULL(rule);

				//-- Add the binding.
				s_ruleBindings.push_back(new RuleBinding(slotId, variableName, rule));

			iff.exitForm(TAG_BIND);
		}

	iff.exitForm(TAG_0000);
}

// ======================================================================
// class SlotRuleManager: PUBLIC STTATIC
// ======================================================================

void SlotRuleManager::install(char const *dataFilename)
{
	DEBUG_FATAL(s_installed, ("SlotRuleManager already installed."));

	Iff iff;

	bool const loadSuccess = iff.open(dataFilename, true);
	FATAL(!loadSuccess, ("SlotRuleManager: required initialization file [%s] not found.", dataFilename));

	load(iff);

	s_installed = true;
	ExitChain::add(remove, "SlotRuleManager");
}

// ----------------------------------------------------------------------

int SlotRuleManager::getRuleCount()
{
	DEBUG_FATAL(!s_installed, ("SlotRuleManager not installed."));
	return static_cast<int>(s_ruleBindings.size());
}

// ----------------------------------------------------------------------

SlotId const &SlotRuleManager::getSlotRequiredForRule(int ruleIndex)
{
	DEBUG_FATAL(!s_installed, ("SlotRuleManager not installed."));
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, ruleIndex, getRuleCount());

	RuleBinding const *const binding = s_ruleBindings[static_cast<RuleBindingVector::size_type>(ruleIndex)];
	NOT_NULL(binding);

	return binding->getRequiredSlot();
}

// ----------------------------------------------------------------------

void SlotRuleManager::addCustomizationVariableForRule(int ruleIndex, CustomizationData &customizationData)
{
	DEBUG_FATAL(!s_installed, ("SlotRuleManager not installed."));
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, ruleIndex, getRuleCount());

	RuleBinding const *const binding = s_ruleBindings[static_cast<RuleBindingVector::size_type>(ruleIndex)];
	NOT_NULL(binding);

	customizationData.addVariableTakeOwnership(binding->getVariableName(), new SlotCustomizationVariable(binding->getSlotRule()));
}

// ======================================================================
