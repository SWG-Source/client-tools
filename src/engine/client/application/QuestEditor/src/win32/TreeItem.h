// ======================================================================
//
// TreeItem.h
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_TreeItem_H
#define INCLUDED_TreeItem_H

// ----------------------------------------------------------------------

#include <qlistview.h>
#include <qdom.h>

// ----------------------------------------------------------------------

class TreeItem : public QListViewItem
{
public:
	TreeItem(QListView * parentTreeItem, QListViewItem * afterTreeItem);
	TreeItem(QListViewItem * parentTreeItem, QListViewItem * afterTreeItem);
	virtual ~TreeItem();

	void setNode(QDomNode const & node);
	QDomNode getNode() const;

	void refresh();

protected:

private:
	QDomNode m_node;

private: //-- disabled
	TreeItem();
	TreeItem(TreeItem const &);
	TreeItem & operator=(TreeItem const &);
};

// ----------------------------------------------------------------------

inline QDomNode TreeItem::getNode() const
{
	return m_node;
}

// ======================================================================

#endif // INCLUDED_TreeItem_H
