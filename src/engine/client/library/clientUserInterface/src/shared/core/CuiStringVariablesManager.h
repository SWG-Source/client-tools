//======================================================================
//
// CuiStringVariablesManager.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiStringVariablesManager_H
#define INCLUDED_CuiStringVariablesManager_H

class CuiStringVariablesData;
class StringId;

//======================================================================

class CuiStringVariablesManager
{
public:
	static void             process (const Unicode::String & encoded, const CuiStringVariablesData & data, Unicode::String & resultStr);
	static void             process (const StringId & stringId,       const CuiStringVariablesData & data, Unicode::String & resultStr);
	static void             process (const StringId & stringId,       const std::string & source, const std::string & target, Unicode::String & resultStr);
	static void             process (const StringId & stringId,       const Unicode::String & source, const Unicode::String & target, const Unicode::String & other, Unicode::String & resultStr);
	static void             process (const StringId & stringId,       const Unicode::String & source, const Unicode::String & target, const Unicode::String & other, int digitInteger, float digitFloat, Unicode::String & resultStr);

	enum VariableObject
	{
		VO_user,
		VO_target,
		VO_other,
	};

	static bool             stringHasCodeForObject (const Unicode::String & encoded, VariableObject participantCode);
};

//======================================================================

#endif
