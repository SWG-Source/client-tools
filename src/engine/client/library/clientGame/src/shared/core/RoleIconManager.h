// ======================================================================
//
// RoleIconManager.h
// copyright (c) 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_RoleIconManager_H
#define INCLUDED_RoleIconManager_H

// ======================================================================

class UIImage;
class UIImageStyle;
class Buff;
class CreatureObject;

// ----------------------------------------------------------------------

class RoleIconManager
{
public:
	static void install();
	static void remove();

	static void cycleToNextQualifyingIcon(bool const includeWorkingSkill);
	static std::string const & getRoleIconName(int choice);
	static int getPetRoleIconChoice();
	static bool doesQualifyForIcon(CreatureObject const * const creature, int const curChoice, bool const includeWorkingSkill);

protected:	
	static int getNextPossibleIconChoice(int curChoice);
};


// ======================================================================

#endif
