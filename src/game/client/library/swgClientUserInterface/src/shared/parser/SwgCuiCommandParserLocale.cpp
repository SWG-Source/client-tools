// ======================================================================
//
// SwgCuiCommandParserLocale.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiCommandParserLocale.h"

#include "LocalizationManager.h"
#include "LocalizedStringTable.h"
#include "LocalizedString.h"

#include <algorithm>

#include <ctime>
#include <cstdio>

// ======================================================================

static const CommandParser::CmdInfo cmds[] =
{
	 {"reset",     1, "<locale>",           "Change the current locale."},
	 {"", 0, "", ""} // this must be last
};

//-----------------------------------------------------------------

SwgCuiCommandParserLocale::SwgCuiCommandParserLocale  ():
CommandParser ("locale", 0, "...", "Manipulate Locale.", 0)
{
	createDelegateCommands (cmds);
	IGNORE_RETURN (addSubCommand (new StringsParser ())); //lint !e1524 // new in ctor w no explicit dtor
}

//-----------------------------------------------------------------

bool SwgCuiCommandParserLocale::performParsing (const NetworkId & , const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node)
{
	UNREF (node);
	UNREF (originalCommand);

	//-----------------------------------------------------------------

	if (isAbbrev (argv [0], "reset"))
	{
		LocalizationManager::getManager ().setLocaleName (Unicode::wideToNarrow ( argv[1] ).c_str ());
		result += Unicode::narrowToWide ("Locale reset.\n");
		return true;
	}
	return false;
}

//-----------------------------------------------------------------
// AddParser
//-----------------------------------------------------------------

SwgCuiCommandParserLocale::StringsParser::StringsParser () :
CommandParser ("strings", 0, "[table name]", "Manipulate Localized strings.", 0)
{

	
} //lint !e429 custodial pointer setParser

//-----------------------------------------------------------------

bool SwgCuiCommandParserLocale::StringsParser::performParsing (const NetworkId &, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node)
{
	UNREF (node);
	UNREF (originalCommand);

	// list info about localizationmanager
	if (argv.size () < 2)
	{
		result += Unicode::narrowToWide ("LocalizationManager info:\n"
			"Table name                    num entries   \n"
			"--------------------------------------------\n");
		
		const LocalizationManager::StringTableMap_t & tableMap = LocalizationManager::getManager ().getTableMap ();
		
		for (LocalizationManager::StringTableMap_t::const_iterator iter = tableMap.begin (); iter != tableMap.end (); ++iter)
		{
			const LocalizationManager::TimedStringTable & tst = (*iter).second;
			const LocalizedStringTable & table                = *tst.second;			
			const Unicode::NarrowString & narrowName          = table.getName ();
			
			const Unicode::String & name = Unicode::narrowToWide (narrowName);
			
			result += name;
			result.append (std::max (size_t (0), 30 - name.length ()), ' ');
			char buf[32];
			_snprintf (buf, 31, "%d\n", table.getMap ().size ());
			const Unicode::String numStr (Unicode::narrowToWide (buf));
			result.append (numStr);
		}
	}
	// list info about localizationmanager
	else
	{
		Unicode::NarrowString tableName(Unicode::wideToNarrow (argv [1]));
		
		const LocalizedStringTable * table = LocalizationManager::getManager ().fetchStringTable (tableName);
		
		if (table != 0)
		{
			const LocalizedStringTable::NameMap_t & nameMap	= table->getNameMap ();
			
			for (LocalizedStringTable::NameMap_t::const_iterator iter = nameMap.begin (); iter != nameMap.end (); ++iter)
			{
				char buf[128];
				
				IGNORE_RETURN (_snprintf (buf, 127, "%3d %-20s ", (*iter).second, (*iter).first.c_str ()));
				result += Unicode::narrowToWide (buf);
				
				const LocalizedString * locstr = table->getLocalizedString ((*iter).second);
				
				if (locstr != 0)
				{
					LocalizedString::crc_type const crc = locstr->getCrc();
					
					char crcBuf[128];
					IGNORE_RETURN (_snprintf (crcBuf, sizeof(crcBuf) - 1, " %x ", crc));

					result += Unicode::narrowToWide (crcBuf);

					result += locstr->getString ();
				}
				else
				{
					result += Unicode::narrowToWide ("NO LOCALIZED STRING FOUND!! ERROR!!\n");
				}
				IGNORE_RETURN (result.append (1, '\n'));
			}
			
			LocalizationManager::getManager ().releaseStringTable (table);
		}
		
		else
		{
			result += Unicode::narrowToWide ("No such localized string table: ") + argv[1];
			IGNORE_RETURN (result.append (1, '\n'));
		}
	}
	
	return true;
}
//-----------------------------------------------------------------
