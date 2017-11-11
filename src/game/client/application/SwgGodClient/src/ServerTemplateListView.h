// ======================================================================
//
// ServerTemplateListView.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ServerTemplateListView_H
#define INCLUDED_ServerTemplateListView_H

// ======================================================================

#include <qlistview.h>
#include <string>

//-----------------------------------------------------------------

class QPixmap;
class QListView;
class AbstractFilesystemTree;

//-----------------------------------------------------------------
/**
* ServerTemplateListView is simply the view for object templates.  It supports dragging
* template items.  The GameWidget supports dropping template items.
*
* @todo: integrate with an Object Template editor
*/
class ServerTemplateListView : public QListView
{
	Q_OBJECT; //lint !e1516 !e19 !e1924 !e1762 various deficiencies in the Qt macro

public:
	ServerTemplateListView(QWidget* parent, const char* name);

public slots:
	void onRefreshList();

protected:
	QDragObject* dragObject();

protected slots:
	void onContextMenuRequested(QListViewItem* item, const QPoint& pos, int col);

private:
	void populateTemplateTree(const char* name, const QPixmap* pix, const QPixmap* folderPix, QListView* parent, const AbstractFilesystemTree* afst) const;
	const std::string constructRelativePath(const QListViewItem* item, bool& isLeaf, bool& isNew, bool& isEdit) const;

private slots:
	void onSelectionChanged() const;

private:
	//disabled
	ServerTemplateListView();
	ServerTemplateListView(const ServerTemplateListView & rhs);
	ServerTemplateListView& operator= (const ServerTemplateListView& rhs);
};

// ======================================================================

#endif
