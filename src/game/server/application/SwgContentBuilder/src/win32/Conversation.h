// ======================================================================
//
// Conversation.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_Conversation_H
#define INCLUDED_Conversation_H

// ======================================================================

#include <string>
#include <map>
#include <vector>

// ======================================================================

class Conversation
{
public:

	Conversation ();
	~Conversation ();

	void                clear ();

	std::string const & getString (std::string const & key) const;
	void                setString (std::string const & key, std::string const & value);

	typedef std::vector<std::pair<std::string, std::string> > StringList;
	void                getStringList (StringList & stringList);

private:

	typedef std::map<std::string, std::string> StringMap;
	StringMap m_stringMap;
};

// ======================================================================

#endif
