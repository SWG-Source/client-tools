// ======================================================================
//
// SwgCuiCommandParserNet.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgCuiCommandParserNet_H
#define INCLUDED_SwgCuiCommandParserNet_H

#include "sharedCommandParser/CommandParser.h"

// ======================================================================
class SwgCuiCommandParserNet : public CommandParser
{
public:
	SwgCuiCommandParserNet ();
	virtual bool performParsing (const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node);
	
private:
	SwgCuiCommandParserNet (const SwgCuiCommandParserNet & rhs);
	SwgCuiCommandParserNet & operator= (const SwgCuiCommandParserNet & rhs);
	
	//-----------------------------------------------------------------

	class DisconnectParser : public CommandParser
	{
	public:
		DisconnectParser ();
		virtual bool performParsing (const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node);
	private:
		DisconnectParser (const DisconnectParser & rhs);
		DisconnectParser & operator= (const DisconnectParser & rhs);
	};

	//-----------------------------------------------------------------

	class ConnectParser : public CommandParser
	{
	public:
		ConnectParser ();
		virtual bool performParsing (const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node);
	private:
		ConnectParser (const ConnectParser & rhs);
		ConnectParser & operator= (const ConnectParser & rhs);
	};

	//-----------------------------------------------------------------

	class AddParser : public CommandParser
	{
	public:
		AddParser ();
		virtual bool performParsing (const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node);
	private:
		AddParser (const AddParser & rhs);
		AddParser & operator= (const AddParser & rhs);
	};

	//-----------------------------------------------------------------

	class SetParser : public CommandParser
	{
	public:
		SetParser ();
		virtual bool performParsing (const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node);
		
	private:
		SetParser (const SetParser & rhs);
		SetParser & operator= (const SetParser & rhs);
	};

	//-----------------------------------------------------------------

	class ShowParser : public CommandParser
	{
	public:
		ShowParser ();
		virtual bool performParsing (const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node);
		
	private:
		ShowParser (const ShowParser & rhs);
		ShowParser & operator= (const ShowParser & rhs);
	};
};

// ======================================================================

#endif
