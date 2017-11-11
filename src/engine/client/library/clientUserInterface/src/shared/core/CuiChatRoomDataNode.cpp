//======================================================================
//
// CuiChatRoomDataNode.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiChatRoomDataNode.h"

#include "sharedFoundation/PointerDeleter.h"
#include "UnicodeUtils.h"

#include <algorithm>

//======================================================================

namespace
{
	bool setAvatarInList (const CuiChatAvatarId & avatarId, bool b, CuiChatRoomDataNode::AvatarVector & av)
	{
		const CuiChatRoomDataNode::AvatarVector::iterator it = std::find (av.begin (), av.end (), avatarId);
		
		if (b)
		{
			if (it == av.end ())
			{
				av.push_back (avatarId);
				std::sort (av.begin (), av.end ());
			}
			else
				return false;
		}
		else
		{
			if (it != av.end ())
				av.erase (it);
			else
				return false;
		}
		
		return true;
	}

	//----------------------------------------------------------------------

	bool findAvatarInList (const CuiChatAvatarId & id, CuiChatAvatarId & avatar, const CuiChatRoomDataNode::AvatarVector & av)
	{
		for (CuiChatRoomDataNode::AvatarVector::const_iterator it = av.begin (); it != av.end (); ++it)
		{
			const CuiChatAvatarId & avatarInList = *it;

			if (id == avatarInList)
			{
				avatar = avatarInList;
				return true;
			}

			if (id.chatId.gameCode.empty ())
			{
				if (!_stricmp (id.chatId.name.c_str (), avatarInList.chatId.name.c_str ()))
				{
					if (id.chatId.cluster.empty () || !_stricmp (id.chatId.cluster.c_str (), avatarInList.chatId.cluster.c_str ()))
					{
						avatar = avatarInList;
						return true;
					}
				}
			}
		}

		return false;
	}


	//----------------------------------------------------------------------

	void setAvatarList (const CuiChatRoomDataNode::PlainAvatarVector & pav, CuiChatRoomDataNode::AvatarVector & av)
	{
		av.clear ();
		av.reserve (pav.size ());
		
		for (CuiChatRoomDataNode::PlainAvatarVector::const_iterator it = pav.begin (); it != pav.end (); ++it)
		{
			const ChatAvatarId & avatar = *it;
			av.push_back (CuiChatAvatarId (avatar, NetworkId::cms_invalid));
		}
		
		std::sort (av.begin (), av.end ());
	}
}

//----------------------------------------------------------------------

class CuiChatRoomDataNode::NodeSortComparator
{
public:
	bool operator() (const CuiChatRoomDataNode * left, const CuiChatRoomDataNode * right) const
	{
		NOT_NULL (left);
		NOT_NULL (right);

		return left->lowerName < right->lowerName;
	}
};

//----------------------------------------------------------------------

CuiChatRoomDataNode::CuiChatRoomDataNode () :
name       (),
data       (),
parent     (0),
children   (),
members    (),
invitees   (),
moderators (),
banned     (),
lowerName  ()
{

}

//----------------------------------------------------------------------

CuiChatRoomDataNode::CuiChatRoomDataNode (CuiChatRoomDataNode * _parent, const ChatRoomData & _data) :
name       (),
data       (_data),
parent     (_parent),
children   (),
members    (),
invitees   (),
moderators (),
banned     (),
lowerName  ()
{
	getBaseName (_data.path, name);
	lowerName = Unicode::toLower (name);

	if (_parent)
	{
		if (!name.empty () && _parent->getChild (name))
			DEBUG_FATAL (true, ("Can't add existing room."));
		else
		{
			_parent->children.push_back (this);
			std::sort (_parent->children.begin (), _parent->children.end (), NodeSortComparator ());
		}
	}
}

//----------------------------------------------------------------------

CuiChatRoomDataNode::~CuiChatRoomDataNode ()
{
	purge ();
}

//----------------------------------------------------------------------

void CuiChatRoomDataNode::purge ()
{
	std::for_each (children.begin (), children.end (), PointerDeleter ());
	children.clear ();
}

//----------------------------------------------------------------------

CuiChatRoomDataNode * CuiChatRoomDataNode::insertData (const ChatRoomData & data)
{
	typedef std::vector<std::string> StringVector;
	StringVector sv;

	std::string parentPath;
	getParentString (data.path, parentPath);
	
	CuiChatRoomDataNode * const parentNode = getOrCreateNode (parentPath);
	NOT_NULL (parentNode);

	std::string basename;
	getBaseName (data.path, basename);
	
	CuiChatRoomDataNode * child = parentNode->getChild (basename);
	
	if (child)
	{
		child->data      = data;
		child->name      = basename;
		child->lowerName = Unicode::toLower (basename);
	}
	else
	{
		child = new CuiChatRoomDataNode (parentNode, data);
	}

	return child;
}

//----------------------------------------------------------------------

/**
* trimmed
*/

void CuiChatRoomDataNode::getParentString (const std::string & path, std::string & parentString)
{
	const size_t last_dot = path.rfind ('.');
	if (last_dot != path.npos && last_dot > 0)
		parentString = path.substr (0, last_dot);
	else
		parentString.clear ();
}

//----------------------------------------------------------------------

CuiChatRoomDataNode * CuiChatRoomDataNode::getOrCreateNode (const std::string & path)
{
	if (path == data.path)
		return this;

	std::string parentPath;
	getParentString (path, parentPath);

	CuiChatRoomDataNode * const parentNode = getOrCreateNode (parentPath);

	NOT_NULL (parentNode);

	std::string basename;
	getBaseName (path, basename);
	
	CuiChatRoomDataNode * child = parentNode->getChild (basename);
	
	if (!child)
	{
		child = new CuiChatRoomDataNode (parentNode, ChatRoomData ());
		child->data.path = path;
		child->name      = basename;
		child->lowerName = Unicode::toLower (basename);
	}

	return child;
}

//----------------------------------------------------------------------

void CuiChatRoomDataNode::getBaseName (const std::string & path, std::string & baseName)
{
	const size_t last_dot = path.rfind ('.');
	if (last_dot != path.npos)
		baseName = path.substr (last_dot + 1);
	else
		baseName = path;
}

//----------------------------------------------------------------------

void CuiChatRoomDataNode::getRootName     (const std::string & path, std::string & rootname, std::string & subpath)
{
	const size_t first_dot = path.find ('.');
	rootname = path.substr (0, first_dot);

	if (first_dot != path.npos)
		subpath = path.substr (first_dot + 1);
	else
		subpath.clear ();
}

//----------------------------------------------------------------------

CuiChatRoomDataNode * CuiChatRoomDataNode::getChild (const std::string & path)
{
	if (path.empty ())
		return this;

	std::string rootname;
	std::string subpath;

	getRootName (path, rootname, subpath);

	const std::string lowerRootname = Unicode::toLower (rootname);

	for (NodeVector::iterator it = children.begin (); it != children.end (); ++it)
	{
		CuiChatRoomDataNode * const node = *it;

		if (node->lowerName == lowerRootname)
			return node->getChild (subpath);
	}
	
	return 0;
}

//----------------------------------------------------------------------

const CuiChatRoomDataNode * CuiChatRoomDataNode::getChild (const std::string & path) const
{
	return const_cast<CuiChatRoomDataNode *>(this)->getChild (path);
}

//----------------------------------------------------------------------

const std::string & CuiChatRoomDataNode::getFullPath         () const
{
	return data.path;
}

//----------------------------------------------------------------------

bool CuiChatRoomDataNode::removeNode          (CuiChatRoomDataNode & node)
{
	const NodeVector::iterator it = std::find (children.begin (), children.end (), &node);

	if (it != children.end ())
	{
		children.erase (it);
		node.parent = 0;
		return true;
	}

	WARNING (true, ("removeNode invalid child"));
	return false;
}

//----------------------------------------------------------------------

bool CuiChatRoomDataNode::setMember           (const CuiChatAvatarId & avatarId, bool b)
{
	return setAvatarInList (avatarId, b, members);
}

//----------------------------------------------------------------------

bool CuiChatRoomDataNode::setInvitee          (const CuiChatAvatarId & avatarId, bool b)
{
	return setAvatarInList (avatarId, b, invitees);
}

//----------------------------------------------------------------------

bool CuiChatRoomDataNode::setModerator        (const CuiChatAvatarId & avatarId, bool b)
{
	return setAvatarInList (avatarId, b, moderators);
}

//----------------------------------------------------------------------

bool CuiChatRoomDataNode::setBanned (const CuiChatAvatarId & avatarId, bool b)
{
	return setAvatarInList (avatarId, b, banned);
}

//----------------------------------------------------------------------

bool CuiChatRoomDataNode::findAvatarInRoom    (const std::string & name, CuiChatAvatarId & avatar, AvatarListType searchFirst) const
{
	const ChatAvatarId avatarId (name);
	const CuiChatAvatarId id = CuiChatAvatarId (avatarId);

	const AvatarVector * lists [3] = { 0, 0, 0};
	
	switch (searchFirst)
	{
	case ALT_members:
		{
			lists [0] = &members;
			lists [1] = 0;
			lists [2] = 0;
		}
		break;
	case ALT_invitees:
		{
			lists [0] = &invitees;
			lists [1] = 0;
			lists [2] = 0;
		}
		break;
		
	case ALT_moderators:
		{
			lists [0] = &moderators;
			lists [1] = 0;
			lists [2] = 0;
		}
		break;

	case ALT_banned:
		{
			lists [0] = &banned;
			lists [1] = 0;
			lists [2] = 0;
		}
		break;
	}

	for (int i = 0; i < 3; ++i)
	{
		if (!lists [i])
			continue;

		if (findAvatarInList (id, avatar, *(lists [i])))
		{
			return true;
		}
	}

	return false;
}

//----------------------------------------------------------------------

void CuiChatRoomDataNode::setMembers    (const PlainAvatarVector & pav)
{
	setAvatarList (pav, members);
}

//----------------------------------------------------------------------

void CuiChatRoomDataNode::setInvitees   (const PlainAvatarVector & pav)
{
	setAvatarList (pav, invitees);
}

//----------------------------------------------------------------------

void CuiChatRoomDataNode::setModerators (const PlainAvatarVector & pav)
{
	setAvatarList (pav, moderators);
}

//----------------------------------------------------------------------

void CuiChatRoomDataNode::setBanned (const PlainAvatarVector & pav)
{
	setAvatarList (pav, banned);
}

//----------------------------------------------------------------------

void CuiChatRoomDataNode::invalidateRoom ()
{
	data.id = 0;
	members.clear         ();
	invitees.clear        ();
	moderators.clear      ();
	banned.clear          ();
	data.owner.clear      ();
	data.creator.clear    ();
	data.title.clear      ();
	data.moderators.clear ();
	data.invitees.clear   ();
}

//----------------------------------------------------------------------

bool CuiChatRoomDataNode::isMember      (const std::string & shortName) const
{
	const CuiChatAvatarId id (ChatAvatarId (shortName), NetworkId::cms_invalid);
	CuiChatAvatarId dummy;
	return findAvatarInList (id, dummy, members);
}

//----------------------------------------------------------------------

bool CuiChatRoomDataNode::isInvitee     (const std::string & shortName) const
{
	const CuiChatAvatarId id (ChatAvatarId (shortName), NetworkId::cms_invalid);
	CuiChatAvatarId dummy;
	return findAvatarInList (id, dummy, invitees);
}

//----------------------------------------------------------------------

bool CuiChatRoomDataNode::isModerator   (const std::string & shortName) const
{
	const CuiChatAvatarId id (ChatAvatarId (shortName), NetworkId::cms_invalid);
	CuiChatAvatarId dummy;
	return findAvatarInList (id, dummy, moderators);
}

//----------------------------------------------------------------------

bool CuiChatRoomDataNode::isBanned (const std::string & shortName) const
{
	const CuiChatAvatarId id (ChatAvatarId (shortName), NetworkId::cms_invalid);
	CuiChatAvatarId dummy;
	return findAvatarInList (id, dummy, banned);
}

//======================================================================
