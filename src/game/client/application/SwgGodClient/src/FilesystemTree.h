// ======================================================================
//
// FilesystemTree.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_FilesystemTree_H
#define INCLUDED_FilesystemTree_H

// ======================================================================
#include "AbstractFilesystemTree.h"
//-----------------------------------------------------------------

/**
* FilesystemTree is an implementation of AbstractFilesystemTree that represents
* a real filesystem.  Currently supports only Win32 filesystems. 
*/

class FilesystemTree : public AbstractFilesystemTree
{
public:
	
	//-----------------------------------------------------------------
	
	struct FilesystemNode : public AbstractFilesystemTree::Node
	{
	public:
		FilesystemNode();
		virtual ~FilesystemNode();
		virtual void populateTree(const std::string & path);
		virtual Node* clone() const;

		void setFilter(const std::string& filter);
		
	private:
		//disabled
		FilesystemNode(const FilesystemNode& rhs);
		FilesystemNode& operator=(const FilesystemNode& rhs);

		std::string m_filter;
	};
	
	//-----------------------------------------------------------------
	
	FilesystemTree();
	virtual ~FilesystemTree();
	virtual void populateTree();	
	void setRootPath(const std::string& path);
	void setFilter(const std::string& filter);
	
	
private:
	//disabled
	FilesystemTree(const FilesystemTree& rhs);	
	FilesystemTree & operator=(const FilesystemTree& rhs);

private:
	std::string               m_rootPath;
	std::string               m_filter;
};
// ======================================================================

#endif

