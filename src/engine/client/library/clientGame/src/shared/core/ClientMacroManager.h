//======================================================================
//
// ClientMacroManager.h
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ClientMacroManager_H
#define INCLUDED_ClientMacroManager_H

//======================================================================

class Iff;
class InputMap;

//----------------------------------------------------------------------

class ClientMacroManager
{
public:

	struct Messages
	{
		struct Changed  { typedef bool        Payload; };
		struct Reset    { typedef bool        Payload; };
		struct Added    { typedef std::string Payload; };
		struct Removed  { typedef std::string Payload; };
		struct Modified { typedef std::string Payload; };
	};

	struct Data
	{
		int         number;
		std::string name;
		Unicode::UTF8String  userDefinedName;
		Unicode::UTF8String  commandString;
		std::string icon;
		std::string color;
	};

	typedef stdvector<Data>::fwd MacroDataVector;

	static const MacroDataVector &      getMacroDataVector ();
	static const Data *                 findMacroData      (const std::string & name);
	static const Data *                 createNewMacroData (const Unicode::UTF8String & userDefinedName, const Unicode::UTF8String & commandString, const std::string & icon, bool notify = true);
	static const bool                   modifyMacroData    (const std::string & name, const Unicode::UTF8String & userDefinedName, const Unicode::UTF8String & commandString, const std::string & icon, bool notify = true, bool pushToInputMap = true);
	static bool                         eraseMacro         (const std::string & name, bool notify = true);
	static bool                         save               ();
	static bool                         load               ();
	static bool                         executeMacroByUserDefinedName (const std::string & userDefinedName, bool insertPause = true);

	static const std::string &          getForcePauseCommand ();

	static void                         synchronizeWithInputMap   (class InputMap * inputmap);
	static const stdvector<std::string>::fwd & getDisallowedCommands();

private:
	static Data *                       findMacroDataInternal      (const std::string & name);
	static Data *                       createNewMacroDataInternal (int number);
	static Data *                       findMacroDataByUserDefinedNameInternal (const Unicode::UTF8String & userDefinedName);
	static bool                         saveText                   ();
	static bool                         loadText                   ();
	static bool                         saveIff                    ();
	static bool                         loadIff                    ();
	static bool                         loadIff_0000               (Iff& iff);

private:
	static stdvector<std::string>::fwd         ms_disallowedMacroCommands;


private:
	static std::string                 ms_forcePauseCommand;
};

//======================================================================

#endif
