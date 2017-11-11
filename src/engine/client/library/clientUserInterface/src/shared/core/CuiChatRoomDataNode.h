//======================================================================
//
// CuiChatRoomDataNode.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiChatRoomDataNode_H
#define INCLUDED_CuiChatRoomDataNode_H

#include "sharedNetworkMessages/ChatRoomData.h"
#include "clientUserInterface/CuiChatAvatarId.h"
#include <vector>

//======================================================================

class CuiChatRoomDataNode
{
public:
	typedef std::vector<CuiChatRoomDataNode *> NodeVector;
	typedef std::vector<CuiChatAvatarId>       AvatarVector;
	typedef std::vector<ChatAvatarId>          PlainAvatarVector;
	
	std::string           name;

	ChatRoomData          data;
	CuiChatRoomDataNode * parent;
	
	                            CuiChatRoomDataNode ();
	                           ~CuiChatRoomDataNode ();

	                            CuiChatRoomDataNode (CuiChatRoomDataNode * parent, const ChatRoomData & data);

	CuiChatRoomDataNode *       getOrCreateNode     (const std::string & path);

	CuiChatRoomDataNode *       getChild            (const std::string & path);
	const CuiChatRoomDataNode * getChild            (const std::string & path) const;

	CuiChatRoomDataNode *       insertData          (const ChatRoomData & data);

	bool                        setMember           (const CuiChatAvatarId & avatarId, bool b);
	bool                        setInvitee          (const CuiChatAvatarId & avatarId, bool b);
	bool                        setModerator        (const CuiChatAvatarId & avatarId, bool b);
	bool                        setBanned           (const CuiChatAvatarId & avatarId, bool b);

	const std::string &         getFullPath         () const;

	bool                        removeNode          (CuiChatRoomDataNode & node);

	static void                 getParentString     (const std::string & path, std::string & parentString);
	static void                 getBaseName         (const std::string & path, std::string & baseName);
	static void                 getRootName         (const std::string & path, std::string & rootname, std::string & subpath);

	const NodeVector &          getChildren   () const;
	const AvatarVector &        getMembers    () const;
	const AvatarVector &        getInvitees   () const;
	const AvatarVector &        getModerators () const;
	const AvatarVector &        getBanned     () const;

	bool                        isMember      (const std::string & shortName) const;
	bool                        isInvitee     (const std::string & shortName) const;
	bool                        isModerator   (const std::string & shortName) const;
	bool                        isBanned      (const std::string & shortName) const;

	void                        setMembers    (const PlainAvatarVector & pav);
	void                        setInvitees   (const PlainAvatarVector & pav);
	void                        setModerators (const PlainAvatarVector & pav);
	void                        setBanned     (const PlainAvatarVector & pav);

	const std::string &         getLowerName () const;

	void                        invalidateRoom ();

	enum AvatarListType
	{
		ALT_members,
		ALT_invitees,
		ALT_moderators,
		ALT_banned
	};

	bool                       findAvatarInRoom    (const std::string & name, CuiChatAvatarId & avatar, AvatarListType searchFirst) const;

	void purge ();

private:

	NodeVector            children;
	AvatarVector          members;
	AvatarVector          invitees;
	AvatarVector          moderators;
	AvatarVector          banned;

	class NodeSortComparator;
	friend class NodeSortComparator;
	std::string lowerName;

};

//----------------------------------------------------------------------

inline const CuiChatRoomDataNode::NodeVector &  CuiChatRoomDataNode::getChildren   () const
{
	return children;
}

//----------------------------------------------------------------------

inline const CuiChatRoomDataNode::AvatarVector & CuiChatRoomDataNode::getMembers    () const
{
	return members;
}

//----------------------------------------------------------------------

inline const CuiChatRoomDataNode::AvatarVector & CuiChatRoomDataNode::getInvitees   () const
{
	return invitees;
}

//----------------------------------------------------------------------

inline const CuiChatRoomDataNode::AvatarVector & CuiChatRoomDataNode::getModerators () const
{
	return moderators;
}

//----------------------------------------------------------------------

inline const CuiChatRoomDataNode::AvatarVector & CuiChatRoomDataNode::getBanned () const
{
	return banned;
}

//----------------------------------------------------------------------

inline const std::string & CuiChatRoomDataNode::getLowerName () const
{
	return lowerName;
}

//======================================================================

#endif
