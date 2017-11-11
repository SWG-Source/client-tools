// ======================================================================
//
// TemplateListView.cpp
// copyright(c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgGodClient/FirstSwgGodClient.h"
#include "TemplateListView.h"
#include "TemplateListView.moc"

#include "AbstractFilesystemTree.h"
#include "ActionHack.h"
#include "ActionsObjectTemplate.h"
#include "ConfigGodClient.h"
#include "FileSystemTree.h"
#include "GodClientPerforce.h"
#include "IconLoader.h"
#include "ObjectTemplateData.h"

#include <qmessagebox.h>
#include <qdragobject.h>
#include <qpopupmenu.h>

//----------------------------------------------------------------------

TemplateListView::TemplateListView(QWidget* theParent, const char* theName)
: QListView(theParent, theName)
{
	IGNORE_RETURN(QListView::addColumn("Name"));
	QListView::setResizeMode(QListView::AllColumns);
	QListView::setRootIsDecorated(true);

	IGNORE_RETURN(connect(this, SIGNAL(contextMenuRequested(QListViewItem*, const QPoint&, int)), this, SLOT(onContextMenuRequested(QListViewItem*, const QPoint&, int))));
	IGNORE_RETURN(connect(this, SIGNAL(selectionChanged()), this, SLOT(onSelectionChanged())));

	const ActionsObjectTemplate* aot = &ActionsObjectTemplate::getInstance();

	//if someone triggers the "refresh object templates" action, refresh the list view
	IGNORE_RETURN(connect(aot->m_serverRefresh, SIGNAL(activated()), SLOT(onRefreshList())));
}

//----------------------------------------------------------------------

/**
 * If the user drags an object template item off this window, generate a message that lets other windows process this event
 *
 */
//QDragObject* TemplateListView::dragObject()
//{
//	return new QTextDrag(ObjectTemplateData::DragMessages::TEMPLATE_DRAGGED, this, "menu");
//}

// ======================================================================

/**
 * Fill (a part of) the template list view with the items from perforce, and use the given pixmaps to differentiate this part of the tree from the others
 *
 */
 void TemplateListView::populateTemplateTree(const char* name, const QPixmap* pix, const QPixmap* folderPix, QListView* parent, const AbstractFilesystemTree* afst) const
{
	if(afst)
	{
		const AbstractFilesystemTree::Node* const node = afst->getRootNode();
		
		if(node)
		{
			QListViewItem* const addedItem = name ? new QListViewItem(parent, name) : 0;
			if(addedItem)
			{
				if(folderPix)
					addedItem->setPixmap(0,*folderPix);

				addedItem->setSelectable(false);
				AbstractFilesystemTree::populateListItem(addedItem, node, true, folderPix, pix, true, true);
			}
			else
			{
				AbstractFilesystemTree::populateListItem(parent, node, true, folderPix, pix, true, true);
			}
		}
	}
}

//----------------------------------------------------------------------

/**
 * Take the full path of an item, and build a tree item out of it (one directory per branch)
 *
 */
const std::string TemplateListView::constructRelativePath(const QListViewItem* item, bool& isLeaf, bool& isNew, bool& isEdit) const
{
	std::string result;

	isLeaf = item->childCount() == 0;

	const QListViewItem* p = item;
	
	while(p)
	{
		const QString text = p->text(0);
		
		if(text == "[NEW]")
		{
			isNew = true;
			break;
		}
		else if(text == "[EDIT]")
		{
			isEdit = true;
			break;
		}
		
		if(!result.empty())
			result = std::string("/") + result;
		
		result = std::string(p->text(0)) + result;
		
		p = p->parent();
	}
	
	return result;
}

//----------------------------------------------------------------------

void TemplateListView::onSelectionChanged() const
{
	QListViewItem* item = selectedItem();
	
	bool isLeaf = false;
	bool isNew  = false;
	bool isEdit = false;
	const std::string path = item ? constructRelativePath(item, isLeaf, isNew, isEdit) : std::string("");
	ActionsObjectTemplate::getInstance().onServerObjectTemplatePathSelectionChanged(path, isLeaf, isNew, isEdit);
}

//----------------------------------------------------------------------

/**
 * Refresh the list view.  Ask perforce for all regular, edited, and new templates
 *
 */
void TemplateListView::onRefreshList()
{
	clear();
	
	setCursor(static_cast<int>(Qt::WaitCursor));

	const char* const templateIffPath = NON_NULL(ConfigGodClient::getData().templateServerIffPath);
	std::string result;
	AbstractFilesystemTree* afst = GodClientPerforce::getInstance().getFileTree(templateIffPath, "iff", result, GodClientPerforce::FileState_depot);

	if(!afst)
	{
		const std::string msg = "Unable to retrieve info from perforce:\n" + result;
		IGNORE_RETURN(QMessageBox::warning(this, "Warning", msg.c_str()));
		return;
	}

	{
		const QPixmap pix       = IL_PIXMAP(hi16_mime_document);
		const QPixmap folderPix = IL_PIXMAP(hi16_filesys_folder_blue);
		populateTemplateTree(0,&pix,&folderPix, this, afst);
	}

	delete afst;

	afst = GodClientPerforce::getInstance().getFileTree(templateIffPath, "iff", result, GodClientPerforce::FileState_add);
	
	{
		const QPixmap pix       = IL_PIXMAP(hi16_mime_document);
		const QPixmap folderPix = IL_PIXMAP(hi16_filesys_folder_red);
		populateTemplateTree("[NEW]",&pix,&folderPix, this, afst);
	}

	delete afst;

	afst = GodClientPerforce::getInstance().getFileTree(templateIffPath, "iff", result, GodClientPerforce::FileState_edit);
	
	{
		const QPixmap pix       = IL_PIXMAP(hi16_mime_document);
		const QPixmap folderPix = IL_PIXMAP(hi16_filesys_folder_violet);
		populateTemplateTree("[EDIT]",&pix,&folderPix, this, afst);
	}

	delete afst;

	unsetCursor();
}

//----------------------------------------------------------------------

/**
 * Show a right click menu, mimicing the "Menu-Script" menu
 */
void TemplateListView::onContextMenuRequested(QListViewItem* item, const QPoint& p, int)
{
	if(item == 0)
		return;

	//create a popup, its name is unimportant
	QPopupMenu* const m_pop = new QPopupMenu(this, "menu");

	ActionsObjectTemplate* aot =&ActionsObjectTemplate::getInstance();

	IGNORE_RETURN(aot->m_serverRefresh->addTo(m_pop));

	IGNORE_RETURN(m_pop->insertSeparator());

	IGNORE_RETURN(aot->m_serverCreate->addTo (m_pop));
	IGNORE_RETURN(aot->m_serverEdit->addTo   (m_pop));
	IGNORE_RETURN(aot->m_serverView->addTo   (m_pop));
	IGNORE_RETURN(aot->m_serverRevert->addTo (m_pop));
	IGNORE_RETURN(aot->m_serverSubmit->addTo (m_pop));

	IGNORE_RETURN(m_pop->insertSeparator());

	IGNORE_RETURN(aot->m_serverCompile->addTo(m_pop));
	IGNORE_RETURN(aot->m_serverReload->addTo(m_pop));
	IGNORE_RETURN(aot->m_serverReplace->addTo(m_pop));

	m_pop->popup(p);
} //lint !e818 item "could" be const, but Qt allows us to change it

// ======================================================================
