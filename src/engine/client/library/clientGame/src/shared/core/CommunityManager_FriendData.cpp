// ============================================================================
// 
// CommunityManager_FriendData.cpp
// Copyright Sony Online Entertainment, Inc.
//
// ============================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/CommunityManager_FriendData.h"

// ============================================================================
//
//  CommunityManager::FriendData
//
// ============================================================================

//-----------------------------------------------------------------------------
CommunityManager::FriendData::FriendData()
 : m_name()
 , m_group()
 , m_comment()
 , m_online(false)
 , m_notifyOnlineStatus(true)
{
}

//-----------------------------------------------------------------------------
void CommunityManager::FriendData::setName(Unicode::String const &name)
{
	m_name = name;
}

//-----------------------------------------------------------------------------
Unicode::String const &CommunityManager::FriendData::getName() const
{
	return m_name;
}

//-----------------------------------------------------------------------------
void CommunityManager::FriendData::setComment(Unicode::String const &comment)
{
	m_comment = comment;
}

//-----------------------------------------------------------------------------
Unicode::String const &CommunityManager::FriendData::getComment() const
{
	return m_comment;
}

//-----------------------------------------------------------------------------
void CommunityManager::FriendData::setGroup(Unicode::String const &group)
{
	m_group = group;
}

//-----------------------------------------------------------------------------
Unicode::String const &CommunityManager::FriendData::getGroup() const
{
	return m_group;
}

//-----------------------------------------------------------------------------
void CommunityManager::FriendData::setOnline(bool const online)
{
	m_online = online;
}

//-----------------------------------------------------------------------------
bool CommunityManager::FriendData::isOnline() const
{
	return m_online;
}

//-----------------------------------------------------------------------------
void CommunityManager::FriendData::setNotifyOnlineStatus(bool const notify)
{
	m_notifyOnlineStatus = notify;
}

//-----------------------------------------------------------------------------
bool CommunityManager::FriendData::isNotifyOnlineStatus() const
{
	return m_notifyOnlineStatus;
}

// ============================================================================
