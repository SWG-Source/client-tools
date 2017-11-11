// ======================================================================
//
// StringFilesystemTree.cpp
// copyright(c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgGodClient/FirstSwgGodClient.h"
#include "StringFilesystemTree.h"

#include "UnicodeUtils.h"

//-----------------------------------------------------------------

namespace
{
}

// ======================================================================

StringFilesystemTree::StringFilesystemNode::StringFilesystemNode()
: Node()
{
}

//-----------------------------------------------------------------

StringFilesystemTree::StringFilesystemNode::StringFilesystemNode(const StringFilesystemNode& rhs)
: Node(rhs)
{
}

//-----------------------------------------------------------------

StringFilesystemTree::StringFilesystemNode::~StringFilesystemNode()
{
}

//-----------------------------------------------------------------

AbstractFilesystemTree::Node* StringFilesystemTree::StringFilesystemNode::clone() const
{
	return new StringFilesystemNode(*this);
}

//-----------------------------------------------------------------

StringFilesystemTree::StringFilesystemTree()
: AbstractFilesystemTree()
{
}

//-----------------------------------------------------------------

StringFilesystemTree::StringFilesystemTree(const StringFilesystemTree& rhs)
: AbstractFilesystemTree(rhs)
{
}

//-----------------------------------------------------------------

StringFilesystemTree::~StringFilesystemTree()
{
}

//-----------------------------------------------------------------

void StringFilesystemTree::populateTree()
{
	if(m_rootNode)
		delete m_rootNode;

	m_rootNode = 0;
}

//-----------------------------------------------------------------

void StringFilesystemTree::addFile(const std::string& path)
{
	if(m_rootNode == 0)
	{
		m_rootNode = new StringFilesystemNode;
		m_rootNode->name = "";
		m_rootNode->type = Node::Folder;
	}
	
	m_rootNode->populateTree(path);
}

//-----------------------------------------------------------------

void StringFilesystemTree::StringFilesystemNode::populateTree(const std::string& path)
{
	
	if(path.empty())
		return;

	{
		static const char* const dirsep = "\\/";

		size_t start = path.find_first_not_of(dirsep);

		std::string subname;
		size_t endpos;

		if(!Unicode::getFirstToken(path, start, endpos, subname, dirsep))
			return;

		if(endpos == path.npos) //lint !e737 implicit promotion, bug in STL with size_t and std::string::npos being of different signage
		{
			//-- file: leaf node
			StringFilesystemNode* newNode = new StringFilesystemNode;

			newNode->name = path.substr(start);
			newNode->type = File;

			children.push_back(newNode);
		}
		else
		{
			Node* subfolder = 0;
			for(Iterator it = begin(); it != end(); ++it)
			{
				if((*it)->name == subname)
				{
					subfolder =*it;
					break;
				}
			}
			
			if(!subfolder)
			{
				subfolder = new StringFilesystemNode;
				
				subfolder->name = subname;
				subfolder->type = Folder;
				
				children.push_back(subfolder);
			}

			subfolder->populateTree(path.substr(endpos + 1));
		}
	}
}

// ======================================================================
