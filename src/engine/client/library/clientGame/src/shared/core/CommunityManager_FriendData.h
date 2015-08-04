// ============================================================================
// 
// CommunityManager_FriendData.h
// Copyright Sony Online Entertainment, Inc.
//
// ============================================================================

#ifndef INCLUDED_CommunityManager_FriendData_H
#define INCLUDED_CommunityManager_FriendData_H

#include "clientGame/CommunityManager.h"

//-----------------------------------------------------------------------------
class CommunityManager::FriendData
{
public:

	FriendData();

	void                   setName(Unicode::String const &name);
	Unicode::String const &getName() const;

	void                   setComment(Unicode::String const &comment);
	Unicode::String const &getComment() const;

	void                   setGroup(Unicode::String const &comment);
	Unicode::String const &getGroup() const;

	void                   setOnline(bool const online);
	bool                   isOnline() const;

	void                   setNotifyOnlineStatus(bool const online);
	bool                   isNotifyOnlineStatus() const;

private:

	Unicode::String m_name;
	Unicode::String m_comment;
	Unicode::String m_group;
	bool            m_online;
	bool            m_notifyOnlineStatus;
};

// ============================================================================

#endif // INCLUDED_CommunityManager_FriendData_H
