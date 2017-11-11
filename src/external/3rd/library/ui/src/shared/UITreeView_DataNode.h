//======================================================================
//
// UITreeView_DataNode.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_UITreeView_DataNode_H
#define INCLUDED_UITreeView_DataNode_H

//======================================================================

#include "UITreeView.h"
#include <list>
#include <vector>

class UIImageStyle;
class UIBaseObject;

//----------------------------------------------------------------------

class UITreeView::DataNode
{
public:
	typedef std::vector<DataNode *> DataVector;

	Unicode::String text;
	bool            expanded;
	bool            underline;
	UIImageStyle *  icon;
	DataNode *      parent;
	DataVector      children;
	int             colorIndex;
	bool            selectable;
	
	                     DataNode (UIBaseObject * data, DataNode * theParent);
	                    ~DataNode ();
	
	int                  countAllChildren ();
	int                  countAllVisibleChildren ();
	int                  countDepth ();
	void                 saveStateRecursive ();
	const UIBaseObject * getDataObject () const;
	
	//----------------------------------------------------------------------

	class Iterator
	{
	public:
		Iterator (DataNode * start, bool visibleOnly);
		
		void skip ();
		
		DataNode * next ();
		int getIndex () const { return m_index; }
		
		DataNode * advance (int count);
		
	private:
		std::list<DataNode *> m_nodes;
		bool                  m_visibleOnly;
		int                   m_index;
		int                   m_nextIndex;
	};

private:

	UIBaseObject *  dataObject;

	DataNode (const DataNode & rhs);
	DataNode & operator= (const DataNode & rhs);
};

//----------------------------------------------------------------------

inline const UIBaseObject * UITreeView::DataNode::getDataObject () const
{
	return dataObject;
}

//======================================================================

#endif
