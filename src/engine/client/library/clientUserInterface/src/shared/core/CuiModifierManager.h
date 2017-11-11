// ======================================================================
//
// CuiModifierManager.h
// copyright (c) 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CuiModifierManager_H
#define INCLUDED_CuiModifierManager_H

// ======================================================================

#include "StringId.h"

// ======================================================================

/** This class manages the data for the attribute and skill modifiers in the game.
    It tracks both a list of what modifiers have been applied to a character, as well
    as display information for all possible modifiers.
 */
class CuiModifierManager
{
public:
	typedef uint32 ModId;

	struct ModifierInfo
	{
		ModId   m_modId;
		StringId m_name;
		StringId m_description;
		std::string m_iconStyle;
		std::string m_internalName;
	};

	struct Messages
	{
		struct ModifiersChanged
		{
			typedef bool Payload;
		};
		struct ModifierAdded
		{
			typedef ModId Payload;
		};
		struct ModifierRemoved
		{
			typedef ModId Payload;
		};
		struct AllModifiersRemoved;
	};

	typedef stdmap<ModId, float>::fwd CurrentModifiers;
	typedef stdmap<ModId, ModifierInfo*>::fwd ModifiersInfo;
	typedef stdvector<ModId>::fwd ModIdList;

public:
	static void install ();
	static void remove  ();
	static void update  (float deltaTime);
	static void addModifier(ModId modId, float duration);
	static void removeModifier(ModId modId);
	static void removeAllModifiers();
//	static const CurrentModifiers * getCurrentModifierMap();
	static bool getModifierList(const ModId modId, ModIdList &modifierChildren);
	static const ModifierInfo * getModifierInfo(ModId modId);
//	static float getRemainingDuration(ModId modId);
	static ModId getModId(const std::string& internalName);

private:
	//disabled
	CuiModifierManager             ();
	~CuiModifierManager            ();
	CuiModifierManager             (const CuiModifierManager & rhs);
	CuiModifierManager & operator= (const CuiModifierManager & rhs);

private:
	static void loadData();

private:
	static CurrentModifiers * ms_currentModifierMap;
	static ModifiersInfo *    ms_modifiersInfo;
};

// ======================================================================

#endif
