//======================================================================
//
// UITreeView_DataNode.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "_precompile.h"
#include "UITreeView_DataNode.h"
#include "UIImageStyle.h"
#include <cassert>

//======================================================================

UITreeView::DataNode::DataNode (UIBaseObject * data, DataNode * theParent) :
text       (),
expanded   (false),
underline  (false),
icon       (0),
parent     (theParent),
children   (),
dataObject (data),
colorIndex (0),
selectable (true)
{
	if (parent)
		parent->children.push_back (this);
	
	if (dataObject)
	{
		dataObject->Attach (0);
		
		if (!dataObject->GetProperty (UITreeView::DataProperties::LocalText, text) || text.empty ())
			dataObject->GetProperty (UITreeView::DataProperties::Text, text);
		
		dataObject->GetPropertyBoolean (UITreeView::DataProperties::Underline, underline);	
		dataObject->GetPropertyBoolean (UITreeView::DataProperties::Expanded,  expanded);
		
		dataObject->GetPropertyInteger (UITreeView::DataProperties::ColorIndex,  colorIndex);
		dataObject->GetPropertyBoolean (UITreeView::DataProperties::Selectable,  selectable);

		std::string iconPath;
		if (dataObject->GetPropertyNarrow (UITreeView::DataProperties::Icon, iconPath))
		{
			if (icon)
				icon->Detach (0);
			
			icon = static_cast<UIImageStyle *>(dataObject->GetObjectFromPath (iconPath.c_str (), TUIImageStyle));
			if (icon)
				icon->Attach (0);
		}
	}
}

//----------------------------------------------------------------------

UITreeView::DataNode::~DataNode ()
{
	if (icon)
		icon->Detach (0);

	icon = 0;

	parent = 0;
	for (DataVector::iterator it = children.begin (); it != children.end (); ++it)
		delete *it;
	
	children.clear ();
	text.clear ();

	if (dataObject)
	{
		dataObject->Detach (0);
		dataObject = 0;
	}
}

//----------------------------------------------------------------------

int UITreeView::DataNode::countAllChildren ()
{
	int count = 0;
	for (DataVector::iterator it = children.begin (); it != children.end (); ++it)
		count += 1 + (*it)->countAllChildren ();
	
	return count;
}

//----------------------------------------------------------------------

int UITreeView::DataNode::countAllVisibleChildren ()
{
	int count = 0;
	for (DataVector::iterator it = children.begin (); it != children.end (); ++it)
	{
		++count;
		if ((*it)->expanded)
			count += (*it)->countAllVisibleChildren ();
	}
	
	return count;
}

//----------------------------------------------------------------------

int UITreeView::DataNode::countDepth ()
{
	return 1 + (parent ? parent->countDepth () : 0);
}

//----------------------------------------------------------------------

UITreeView::DataNode::Iterator::Iterator (DataNode * start, bool visibleOnly) :
m_nodes (),
m_visibleOnly (visibleOnly),
m_index (-1),
m_nextIndex (0)
{
	m_nodes.push_back (start);
}

//----------------------------------------------------------------------

void UITreeView::DataNode::Iterator::skip ()
{
	next ();
	--m_nextIndex;
	--m_index;
}

//----------------------------------------------------------------------

UITreeView::DataNode * UITreeView::DataNode::Iterator::next ()
{
	if (m_nodes.empty ())
		return 0;
	
	DataNode * cur = m_nodes.back ();
	
	assert (cur);
	
	m_nodes.pop_back ();
	
	m_index = m_nextIndex;
	
	++m_nextIndex;
	
	if (!m_visibleOnly || cur->expanded)
	{
		for (DataVector::reverse_iterator it = cur->children.rbegin (); it != cur->children.rend (); ++it)
			m_nodes.push_back (*it);
	}
	else
		m_nextIndex += cur->countAllChildren ();
	
	return cur;
}

//----------------------------------------------------------------------

UITreeView::DataNode * UITreeView::DataNode::Iterator::advance (int count)
{
	if (count <= 0)
		return 0;
	
	const int targetIndex = m_index + count;
	
	DataNode * node = next ();
	int lastIndex = m_index;
	
	for (int i = 1; i < count && node; ++i)
	{
		node = next ();
		if (node)
			lastIndex = m_index;
	}
	
	if (node && lastIndex == targetIndex)
		return node;
	
	return 0;
}

//----------------------------------------------------------------------

void UITreeView::DataNode::saveStateRecursive ()
{
	if (dataObject)
	{
		dataObject->SetPropertyBoolean (UITreeView::DataProperties::Expanded, expanded);
	}

	for (DataVector::iterator it = children.begin (); it != children.end (); ++it)
	{
		UITreeView::DataNode * const child = *it;
		child->saveStateRecursive ();
	}
}

//======================================================================
