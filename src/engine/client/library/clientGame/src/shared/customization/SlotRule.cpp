// ======================================================================
//
// SlotRule.cpp
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/SlotRule.h"

#include "clientSkeletalAnimation/SkeletalAppearance2.h"

#include "clientGame/ContainerInterface.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"

#include "sharedDebug/InstallTimer.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/Tag.h"
#include "sharedObject/Object.h"
#include "sharedObject/SlottedContainer.h"
#include "sharedObject/SlotId.h"
#include "sharedObject/SlotIdManager.h"

#include <map>

// ======================================================================

//lint -e1511 // member hides non-virtual member.  Applies to install/remove/create functions.  It's okay.

// ======================================================================

namespace SlotRuleNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class SlotOccupiedRule: public SlotRule
	{
	public:

		static void install();
		static void remove();

	public:

		virtual ~SlotOccupiedRule();

		virtual bool test(SlottedContainer const &container, SlottedContainer const * alternateContainer, Object * owner) const;

	private:

		static SlotRule *create(Iff &iff);

	private:

		explicit SlotOccupiedRule(Iff &iff);
		void     load_0000(Iff &iff);

		// Disabled.
		SlotOccupiedRule();
		SlotOccupiedRule(SlotOccupiedRule const&); //lint -esym(754, SlotOccupiedRule::SlotOccupiedRule) // Not referenced. // Defensive hiding.
		SlotOccupiedRule &operator =(SlotOccupiedRule const&);

	private:

		SlotId  m_slotId;

	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class NegateRule: public SlotRule
	{
	public:

		static void install();
		static void remove();

	public:

		virtual ~NegateRule();

		virtual bool test(SlottedContainer const &container, SlottedContainer const * alternateContainer, Object * owner) const;

	private:

		static SlotRule *create(Iff &iff);

	private:

		explicit NegateRule(Iff &iff);
		void     load_0000(Iff &iff);

		// Disabled.
		NegateRule();
		NegateRule(NegateRule const&); //lint -esym(754, NegateRule::NegateRule) // Not referenced. // Defensive hiding.
		NegateRule &operator =(NegateRule const&);

	private:

		SlotRule *m_negatedRule;

	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class AndRule: public SlotRule
	{
	public:

		static void install();
		static void remove();

	public:

		virtual ~AndRule();

		virtual bool test(SlottedContainer const &container, SlottedContainer const * alternateContainer, Object * owner) const;

	private:

		static SlotRule *create(Iff &iff);

	private:

		explicit AndRule(Iff &iff);
		void     load_0000(Iff &iff);

		// Disabled.
		AndRule();
		AndRule(AndRule const&); //lint -esym(754, AndRule::AndRule) // Not referenced. // Defensive hiding.
		AndRule &operator =(AndRule const&);

	private:

		SlotRule *m_rule1;
		SlotRule *m_rule2;

	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class OrRule: public SlotRule
	{
	public:

		static void install();
		static void remove();

	public:

		virtual ~OrRule();

		virtual bool test(SlottedContainer const &container, SlottedContainer const * alternateContainer, Object * owner) const;

	private:

		static SlotRule *create(Iff &iff);

	private:

		explicit OrRule(Iff &iff);
		void     load_0000(Iff &iff);

		// Disabled.
		OrRule();
		OrRule(OrRule const&); //lint -esym(754, OrRule::OrRule) // Not referenced. // Defensive hiding.
		OrRule &operator =(OrRule const&);

	private:

		SlotRule *m_rule1;
		SlotRule *m_rule2;

	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	typedef SlotRule *(*CreateFunction)(Iff &iff);
	typedef std::map<Tag, CreateFunction>  CreateFunctionMap;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void  remove();

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	Tag const TAG_ANDT = TAG(A,N,D,T);
	Tag const TAG_NEGT = TAG(N,E,G,T);
	Tag const TAG_ORTS = TAG(O,R,T,S);
	Tag const TAG_SLOT = TAG(S,L,O,T);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool               s_installed;
	CreateFunctionMap  s_createFunctionMap;

}

using namespace SlotRuleNamespace;

// ======================================================================
// class SlotRuleNamespace::SlotOccupiedRule
// ======================================================================

void SlotRuleNamespace::SlotOccupiedRule::install()
{
	InstallTimer const installTimer("SlotRuleNamespace::SlotOccupiedRule");

	std::pair<CreateFunctionMap::iterator, bool> result = s_createFunctionMap.insert(CreateFunctionMap::value_type(TAG_SLOT, static_cast<CreateFunction>(create)));
	FATAL(!result.second, ("Failed to install SlotOccupiedRule."));
}

// ----------------------------------------------------------------------

void SlotRuleNamespace::SlotOccupiedRule::remove()
{
	CreateFunctionMap::iterator findIt = s_createFunctionMap.find(TAG_SLOT);
	if (findIt != s_createFunctionMap.end())
		s_createFunctionMap.erase(findIt);
}

// ----------------------------------------------------------------------

SlotRuleNamespace::SlotOccupiedRule::~SlotOccupiedRule()
{
}

// ----------------------------------------------------------------------

bool SlotRuleNamespace::SlotOccupiedRule::test(SlottedContainer const &container, SlottedContainer const * alternateContainer, Object * owner) const
{
	Container::ContainerErrorCode  errorCode;

	errorCode = Container::CEC_Success;

	if(alternateContainer && alternateContainer->hasSlot(m_slotId))
	{
		if(owner && alternateContainer->isSlotEmpty(m_slotId, errorCode) && !container.isSlotEmpty(m_slotId, errorCode))
		{
			// So we have an empty appearance inventory slot, and a currently occupied normal inventory slot.
			// Normally we'd want this slot to appear as occupied. HOWEVER, if we have an item that takes up
			// multiple slots (like robes), then we only want those slots to show up as occupied if they are indeed
			// being worn. Otherwise we're using customization data of an invisible object - which doesn't make much sense.
			CachedNetworkId obj = container.getObjectInSlot(m_slotId, errorCode);
			Object * itemObj = obj.getObject();
			SkeletalAppearance2 * ownerAppearance = owner->getAppearance() ? owner->getAppearance()->asSkeletalAppearance2() : 0;
			if(ownerAppearance)
			{
				return ownerAppearance->isWearing(itemObj); // This is the only real nastiness of this check. This is an O(n) check on our wearables (which are ~20 - 30 elements). Not horrible but ehh...
			}
		}

		return container.hasSlot(m_slotId) && (!container.isSlotEmpty(m_slotId, errorCode) || !alternateContainer->isSlotEmpty(m_slotId, errorCode) ) ;
	}

	return container.hasSlot(m_slotId) && !container.isSlotEmpty(m_slotId, errorCode);
}

// ======================================================================

SlotRule *SlotRuleNamespace::SlotOccupiedRule::create(Iff &iff)
{
	return new SlotOccupiedRule(iff);
}

// ======================================================================

SlotRuleNamespace::SlotOccupiedRule::SlotOccupiedRule(Iff &iff) :
	SlotRule(),
	m_slotId(SlotId::invalid)
{
	iff.enterForm(TAG_SLOT);

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
					InstallTimer const installTimer("SlotRuleNamespace::NegateRule");

					FATAL(true, ("Unsupported SlotOccupiedRule version [%s].", buffer));
				}
				break;
		}

	iff.exitForm(TAG_SLOT);
}

// ----------------------------------------------------------------------

void SlotRuleNamespace::SlotOccupiedRule::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);
		iff.enterChunk(TAG_INFO);

			// Read the slot name.
			char slotName[256];

			iff.read_string(slotName, sizeof(slotName) - 1);
			slotName[sizeof(slotName) - 1] = '\0';

			// Convert to SlotId.
			m_slotId = SlotIdManager::findSlotId(CrcLowerString(slotName));
			FATAL(m_slotId == SlotId::invalid, ("SlotOccupiedRule specified slot name [%s] unknown to the system.", slotName));

		iff.exitChunk(TAG_INFO);
	iff.exitForm(TAG_0000);
}

// ======================================================================
// class SlotRuleNamespace::NegateRule
// ======================================================================

void SlotRuleNamespace::NegateRule::install()
{
	InstallTimer const installTimer("SlotRuleNamespace::NegateRule");

	std::pair<CreateFunctionMap::iterator, bool> result = s_createFunctionMap.insert(CreateFunctionMap::value_type(TAG_NEGT, static_cast<CreateFunction>(create)));
	FATAL(!result.second, ("Failed to install NegateRule."));
}

// ----------------------------------------------------------------------

void SlotRuleNamespace::NegateRule::remove()
{
	CreateFunctionMap::iterator findIt = s_createFunctionMap.find(TAG_NEGT);
	if (findIt != s_createFunctionMap.end())
		s_createFunctionMap.erase(findIt);
}

// ----------------------------------------------------------------------

SlotRuleNamespace::NegateRule::~NegateRule()
{
	delete m_negatedRule;
}

// ----------------------------------------------------------------------

bool SlotRuleNamespace::NegateRule::test(SlottedContainer const &container, SlottedContainer const * alternateContainer, Object * owner) const
{
	NOT_NULL(m_negatedRule);
	return !(m_negatedRule->test(container, alternateContainer, owner));
}

// ======================================================================

SlotRule *SlotRuleNamespace::NegateRule::create(Iff &iff)
{
	return new NegateRule(iff);
}

// ======================================================================

SlotRuleNamespace::NegateRule::NegateRule(Iff &iff) :
	SlotRule(),
	m_negatedRule(0)
{
	iff.enterForm(TAG_NEGT);

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
					FATAL(true, ("Unsupported NegateRule version [%s].", buffer));
				}
				break;
		}

	iff.exitForm(TAG_NEGT);
}

// ----------------------------------------------------------------------

void SlotRuleNamespace::NegateRule::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);

		m_negatedRule = SlotRule::create(iff);
		FATAL(!m_negatedRule, ("NegateRule specified null negated rule."));

	iff.exitForm(TAG_0000);
}

// ======================================================================
// class SlotRuleNamespace::AndRule
// ======================================================================

void SlotRuleNamespace::AndRule::install()
{
	InstallTimer const installTimer("SlotRuleNamespace::AndRule");

	std::pair<CreateFunctionMap::iterator, bool> result = s_createFunctionMap.insert(CreateFunctionMap::value_type(TAG_ANDT, static_cast<CreateFunction>(create)));
	FATAL(!result.second, ("Failed to install AndRule."));
}

// ----------------------------------------------------------------------

void SlotRuleNamespace::AndRule::remove()
{
	CreateFunctionMap::iterator findIt = s_createFunctionMap.find(TAG_ANDT);
	if (findIt != s_createFunctionMap.end())
		s_createFunctionMap.erase(findIt);
}

// ----------------------------------------------------------------------

SlotRuleNamespace::AndRule::~AndRule()
{
	delete m_rule1;
	delete m_rule2;
}

// ----------------------------------------------------------------------

bool SlotRuleNamespace::AndRule::test(SlottedContainer const &container, SlottedContainer const * alternateContainer, Object * owner) const
{
	NOT_NULL(m_rule1);
	NOT_NULL(m_rule2);

	return m_rule1->test(container, alternateContainer, owner) && m_rule2->test(container, alternateContainer, owner);
}

// ======================================================================

SlotRule *SlotRuleNamespace::AndRule::create(Iff &iff)
{
	return new AndRule(iff);
}

// ======================================================================

SlotRuleNamespace::AndRule::AndRule(Iff &iff) :
	SlotRule(),
	m_rule1(0),
	m_rule2(0)
{
	iff.enterForm(TAG_ANDT);

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
					FATAL(true, ("Unsupported AndRule version [%s].", buffer));
				}
				break;
		}

	iff.exitForm(TAG_ANDT);
}

// ----------------------------------------------------------------------

void SlotRuleNamespace::AndRule::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);

		m_rule1 = SlotRule::create(iff);
		FATAL(!m_rule1, ("AndRule specified null rule for first rule."));

		m_rule2 = SlotRule::create(iff);
		FATAL(!m_rule2, ("AndRule specified null rule for second rule."));

	iff.exitForm(TAG_0000);
}

// ======================================================================
// class SlotRuleNamespace::OrRule
// ======================================================================

void SlotRuleNamespace::OrRule::install()
{
	InstallTimer const installTimer("SlotRuleNamespace::OrRule");

	std::pair<CreateFunctionMap::iterator, bool> result = s_createFunctionMap.insert(CreateFunctionMap::value_type(TAG_ORTS, static_cast<CreateFunction>(create)));
	FATAL(!result.second, ("Failed to install OrRule."));
}

// ----------------------------------------------------------------------

void SlotRuleNamespace::OrRule::remove()
{
	CreateFunctionMap::iterator findIt = s_createFunctionMap.find(TAG_ORTS);
	if (findIt != s_createFunctionMap.end())
		s_createFunctionMap.erase(findIt);
}

// ----------------------------------------------------------------------

SlotRuleNamespace::OrRule::~OrRule()
{
	delete m_rule1;
	delete m_rule2;
}

// ----------------------------------------------------------------------

bool SlotRuleNamespace::OrRule::test(SlottedContainer const &container, SlottedContainer const * alternateContainer, Object * owner) const
{
	NOT_NULL(m_rule1);
	NOT_NULL(m_rule2);

	return m_rule1->test(container, alternateContainer, owner) || m_rule2->test(container, alternateContainer, owner);
}

// ======================================================================

SlotRule *SlotRuleNamespace::OrRule::create(Iff &iff)
{
	return new OrRule(iff);
}

// ======================================================================

SlotRuleNamespace::OrRule::OrRule(Iff &iff) :
	SlotRule(),
	m_rule1(0),
	m_rule2(0)
{
	iff.enterForm(TAG_ORTS);

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
					FATAL(true, ("Unsupported OrRule version [%s].", buffer));
				}
				break;
		}

	iff.exitForm(TAG_ORTS);
}

// ----------------------------------------------------------------------

void SlotRuleNamespace::OrRule::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);

		m_rule1 = SlotRule::create(iff);
		FATAL(!m_rule1, ("OrRule specified null slot rule for first rule."));

		m_rule2 = SlotRule::create(iff);
		FATAL(!m_rule2, ("OrRule specified null slot rule for second rule."));

	iff.exitForm(TAG_0000);
}

// ======================================================================
// namespace SlotRuleNamespace
// ======================================================================

void SlotRuleNamespace::remove()
{
	DEBUG_FATAL(!s_installed, ("SlotRule not installed."));
	s_installed = false;

	OrRule::remove();
	AndRule::remove();
	NegateRule::remove();
	SlotOccupiedRule::remove();
}

// ======================================================================
// class SlotRule: PUBLIC STATIC
// ======================================================================

void SlotRule::install()
{
	InstallTimer const installTimer("SlotRule::install");

	DEBUG_FATAL(s_installed, ("SlotRule already installed."));

	SlotOccupiedRule::install();
	NegateRule::install();
	AndRule::install();
	OrRule::install();

	s_installed = true;
	ExitChain::add(remove, "SlotRule");
}

// ----------------------------------------------------------------------

SlotRule *SlotRule::create(Iff &iff)
{
	Tag const name = iff.getCurrentName();
	CreateFunctionMap::iterator findIt = s_createFunctionMap.find(name);
	if (findIt == s_createFunctionMap.end())
	{
		char buffer[5];
		ConvertTagToString(name, buffer);
		FATAL(true, ("Data error: cannot load unregistered slot rule [%s].", buffer));
	}

	NOT_NULL(findIt->second);
	CreateFunction const createFunction = findIt->second;

	NOT_NULL(createFunction);
	return (*createFunction)(iff);
}

// ======================================================================
// class SlotRule: PUBLIC
// ======================================================================

SlotRule::~SlotRule()
{
}

// ======================================================================
// class SlotRule: PROTECTED
// ======================================================================

SlotRule::SlotRule()
{
}

// ======================================================================
