// ======================================================================
//
// StringFilesystemTree.h
// copyright(c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_StringFilesystemTree_H
#define INCLUDED_StringFilesystemTree_H

// ======================================================================

#include "AbstractFilesystemTree.h"

//-----------------------------------------------------------------

/**
 * StringFilesystemTree is a filesystem tree that is built up by repeatedly
 * adding path strings to it.  It is useful for building a filesystem tree
 * from a series of lines of output from something like 'p4 files'.
 */
class StringFilesystemTree : public AbstractFilesystemTree
{
public:
	struct StringFilesystemNode : public AbstractFilesystemTree::Node
	{
		StringFilesystemNode();
		virtual Node* clone() const;
		virtual void populateTree(const std::string& path);
		
	private:
		//disabled
		virtual ~StringFilesystemNode();
		StringFilesystemNode(const StringFilesystemNode& rhs);
		StringFilesystemNode& operator=(const StringFilesystemNode& rhs);
	};

	StringFilesystemTree();
	virtual ~StringFilesystemTree();
	virtual void populateTree();
	void         addFile(const std::string& path);

private:
	//disabled
	StringFilesystemTree(const StringFilesystemTree& rhs);
	StringFilesystemTree& operator=(const StringFilesystemTree& rhs);
};
// ======================================================================

#endif

