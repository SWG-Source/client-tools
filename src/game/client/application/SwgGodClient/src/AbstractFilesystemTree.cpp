// ======================================================================
//
// AbstractFilesystemTree.cpp
// copyright(c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgGodClient/FirstSwgGodClient.h"
#include "AbstractFilesystemTree.h"

#include <windows.h>

// ======================================================================
AbstractFilesystemTree::Node::Node()
: name(),
  type(Folder),
  children()
{
}
//-----------------------------------------------------------------
AbstractFilesystemTree::Node::Node(const Node & rhs)
: name(rhs.name),
  type(rhs.type),
  children()
{
	children.reserve(rhs.children.size());
	
	for(ConstIterator it = rhs.children.begin(); it != rhs.children.end(); ++it)
	{
		children.push_back((*it)->clone());
	}
}
//-----------------------------------------------------------------

AbstractFilesystemTree::Node::~Node()
{
	for(Iterator it = begin(); it != end(); ++it)
	{
		delete *it;
	}
}
//-----------------------------------------------------------------

AbstractFilesystemTree::AbstractFilesystemTree()
: m_rootNode(0)
{
}
//-----------------------------------------------------------------

AbstractFilesystemTree::AbstractFilesystemTree(const AbstractFilesystemTree & rhs)
: m_rootNode(rhs.m_rootNode ? rhs.m_rootNode->clone() : 0)
{
}

//-----------------------------------------------------------------
AbstractFilesystemTree::~AbstractFilesystemTree()
{
	delete m_rootNode;
}

// ======================================================================
