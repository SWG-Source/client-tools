//======================================================================
//
// SwgCuiChatWindow_ChannelId.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiChatWindow_ChannelId_H
#define INCLUDED_SwgCuiChatWindow_ChannelId_H

//======================================================================

#include "swgClientUserInterface/SwgCuiChatWindow.h"

//----------------------------------------------------------------------

struct SwgCuiChatWindow::ChannelId
{
	ChannelType  type;
	
	bool operator< (const ChannelId & rhs) const
	{
		return type < rhs.type || (type == rhs.type && lowerName < rhs.lowerName);
	} 
	
	bool operator== (const ChannelId & rhs) const
	{
		return type == rhs.type && lowerName == rhs.lowerName;
	} 

	bool operator!= (const ChannelId & rhs) const
	{
		return !(*this == rhs);
	} 
	
	                        ChannelId ();
	explicit                ChannelId (ChannelType _type);
	                        ChannelId (ChannelType _type, const std::string & _name);
							ChannelId (ChannelType _type, const std::string & _name, bool _isPublic);
	
	void                    setName        (const std::string & _name);
	bool                    getIsPublic    () const;
	const std::string &     getName        () const;
	const Unicode::String & getDisplayName () const;

	void                updateDefaultName ();
	void                updateDisplayName ();
	
private:

	std::string         lowerName;
	std::string         name; //name is only valid for CT_chatRoom type ids
	Unicode::String     displayName;
	bool                isPublic;
};

//----------------------------------------------------------------------

inline const std::string & SwgCuiChatWindow::ChannelId::getName () const
{
	return name;
}

//----------------------------------------------------------------------

inline const Unicode::String & SwgCuiChatWindow::ChannelId::getDisplayName () const
{
	return displayName;
}

//======================================================================

#endif
