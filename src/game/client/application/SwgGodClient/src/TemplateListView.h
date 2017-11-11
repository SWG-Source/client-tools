// ======================================================================
//
// TemplateListView.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_TemplateListView_H
#define INCLUDED_TemplateListView_H

// ======================================================================
#include <qlistview.h>

//-----------------------------------------------------------------

class QPixmap;
class QListView;
class AbstractFilesystemTree;

//-----------------------------------------------------------------
/**
* TemplateListView is simply the view for object templates.  It supports dragging
* template items.  The GameWidget supports dropping template items.
*
* @todo: integrate with an Object Template editor
*/
class TemplateListView : public QListView
{
	Q_OBJECT; //lint !e1516 !e19 !e1924 !e1762 various deficiencies in the Qt macro

public:
	TemplateListView(QWidget* parent, const char* name);

public slots:
	void onRefreshList();

//protected:
//	QDragObject* dragObject();

protected slots:
	void onContextMenuRequested(QListViewItem* item, const QPoint& pos, int col);

private:
	void populateTemplateTree(const char* name, const QPixmap* pix, const QPixmap* folderPix, QListView* parent, const AbstractFilesystemTree* afst) const;
	const std::string constructRelativePath(const QListViewItem* item, bool& isLeaf, bool& isNew, bool& isEdit) const;

private slots:
	void onSelectionChanged() const;

private:
	//disabled
	TemplateListView();
	TemplateListView(const TemplateListView & rhs);
	TemplateListView& operator= (const TemplateListView& rhs);
};

// ======================================================================

#endif
