// ======================================================================
//
// AbstractFilesystemTree.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_AbstractFilesystemTree_H
#define INCLUDED_AbstractFilesystemTree_H

// ======================================================================

/**
* AbstractFilesystemTree is a simple tree structure that represents a typical
* filesystem.  A node in the tree may be a folder or a file, and folder nodes
* may have any number of child nodes.  All file nodes are leaf nodes.
*
* AbstractFilesystemTree has a handy template method for populating a QListView.
*
*/

class AbstractFilesystemTree
{
public:

	//-----------------------------------------------------------------

	struct Node
	{
		typedef std::vector<Node *> NodeContainer;
		typedef NodeContainer::iterator Iterator;
		typedef NodeContainer::const_iterator ConstIterator;

		enum Type
		{
			Folder,
			File
		};

		std::string   name;
		Type          type;

		inline Iterator begin () { return children.begin (); }
		inline Iterator end () { return children.end (); }

		inline ConstIterator begin () const { return children.begin (); }
		inline ConstIterator end () const { return children.end (); }

		Node ();

		virtual void populateTree (const std::string & path) = 0;

		virtual ~Node () = 0;

		virtual Node * clone () const = 0;

	protected:
		Node (const Node & rhs);
		NodeContainer children;

	private:
		//disabled
		Node & operator= (const Node & rhs);
	};

	//-----------------------------------------------------------------

	AbstractFilesystemTree ();
	const Node * getRootNode () const;
	
	virtual void populateTree () = 0;
	virtual ~AbstractFilesystemTree () = 0;
	
private:
	//disabled
	AbstractFilesystemTree & operator= (const AbstractFilesystemTree & rhs);
	
protected:
	AbstractFilesystemTree (const AbstractFilesystemTree & rhs);
	Node *                    m_rootNode;

public:
		
	template <typename T> static void populateListItem (T * view, const AbstractFilesystemTree::Node * node,
		bool  selectableFolders,
		const QPixmap * folderPixmap,
		const QPixmap * filePixmap,
		bool stripDots, bool draggable)
	{
		for (AbstractFilesystemTree::Node::ConstIterator it = node->begin (); it != node->end (); ++it)
		{
			std::string name = (*it)->name;
			
			if ((*it)->type == AbstractFilesystemTree::Node::File)
			{
				if (stripDots)
				{
					const size_t dotpos = name.find_last_of ('.');
					
					if (dotpos != name.npos)
						name = name.substr (0, dotpos);
				}
			}
			
			QListViewItem * const ni = new QListViewItem (view, name.c_str ());
			
			if ((*it)->type == AbstractFilesystemTree::Node::Folder)
			{
				ni->setSelectable (selectableFolders);
				populateListItem (ni, *it, selectableFolders, folderPixmap, filePixmap, stripDots, draggable);
				if (folderPixmap)
					ni->setPixmap (0, *folderPixmap);
			}
			else
			{
				ni->setDragEnabled (draggable);
				if (filePixmap)
					ni->setPixmap (0, *filePixmap);
			}
		}
	}

};
//-----------------------------------------------------------------
inline const AbstractFilesystemTree::Node * AbstractFilesystemTree::getRootNode () const
{
	return m_rootNode;
}
// ======================================================================

#endif

