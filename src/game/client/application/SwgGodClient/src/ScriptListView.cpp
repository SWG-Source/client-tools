// ======================================================================
//
// ScriptListView.cpp
// copyright(c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgGodClient/FirstSwgGodClient.h"
#include "ScriptListView.h"
#include "ScriptListView.moc"

#include "ActionHack.h"
#include "ActionsScript.h"
#include "AbstractFilesystemTree.h"
#include "ConfigGodClient.h"
#include "FilesystemTree.h"
#include "GodClientPerforce.h"
#include "IconLoader.h"
#include "fileInterface/StdioFile.h"

#include "QDragObject.h"
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qfiledialog.h>

//-----------------------------------------------------------------

namespace
{
	void poulateScriptTree(const char* name, const QPixmap* pix, const QPixmap* folderPix, QListView* parent, const AbstractFilesystemTree* afst)
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
	
	//-----------------------------------------------------------------

	const std::string constructRelativePath(const QListViewItem* item, bool& isLeaf, bool& isNew, bool& isEdit)
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

	//-----------------------------------------------------------------

}

ScriptListView::ScriptListView(QWidget* theParent, const char* theName)
: QListView(theParent, theName)
{
	IGNORE_RETURN(QListView::addColumn("Name"));
	QListView::setResizeMode(QListView::NoColumn);
	QListView::setRootIsDecorated(true);
	QListView::setColumnWidthMode(0, QListView::Maximum);

	IGNORE_RETURN(connect(this, SIGNAL(contextMenuRequested(QListViewItem*, const QPoint&, int)), this, SLOT(onContextMenuRequested(QListViewItem*, const QPoint&, int))));
	IGNORE_RETURN(connect(this, SIGNAL(selectionChanged()), this, SLOT(onSelectionChanged())));
	
	ActionsScript* const as =&ActionsScript::getInstance();

	IGNORE_RETURN(connect(as->refresh, SIGNAL(activated()), SLOT(onRefreshList())));
}

//-----------------------------------------------------------------

QDragObject* ScriptListView::dragObject()
{
	return new QTextDrag(ActionsScript::DragMessages::SCRIPT_DRAGGED, this, "menu");
}

//-----------------------------------------------------------------

void ScriptListView::onSelectionChanged() const
{
	QListViewItem* item = selectedItem();
	
	bool isLeaf = false;
	bool isNew = false;
	bool isEdit = false;
	const std::string path = item ? constructRelativePath(item, isLeaf, isNew, isEdit) : std::string("");
	ActionsScript::getInstance().onScriptPathSelectionChanged(path, isLeaf, isNew, isEdit);
}

//-----------------------------------------------------------------

void ScriptListView::onRefreshList()
{
	clear();
	
	setCursor(static_cast<int>(Qt::WaitCursor));

	if ( ConfigGodClient::getConnectToPerforce() )
	{

		const char* const classPath = NON_NULL(ConfigGodClient::getData().scriptClassPath);
		std::string result;
		AbstractFilesystemTree* afst = GodClientPerforce::getInstance().getFileTree(classPath, "class", result, GodClientPerforce::FileState_depot);

		if(!afst)
		{
			const std::string msg = "Unable to retrieve info from perforce:\n" + result;
			IGNORE_RETURN(QMessageBox::warning(this, "Warning", msg.c_str()));
			return;
		}

		{
			const QPixmap pix       = IL_PIXMAP(hi16_mime_document);
			const QPixmap folderPix = IL_PIXMAP(hi16_filesys_folder_green);
			poulateScriptTree(0,&pix,&folderPix, this, afst);
		}

		delete afst;

		afst = GodClientPerforce::getInstance().getFileTree(classPath, "class", result, GodClientPerforce::FileState_add);
		
		{
			const QPixmap pix       = IL_PIXMAP(hi16_mime_document);
			const QPixmap folderPix = IL_PIXMAP(hi16_filesys_folder_red);
			poulateScriptTree("[NEW]",&pix,&folderPix, this, afst);
		}

		delete afst;

		afst = GodClientPerforce::getInstance().getFileTree(classPath, "class", result, GodClientPerforce::FileState_edit);
		
		{
			const QPixmap pix       = IL_PIXMAP(hi16_mime_document);
			const QPixmap folderPix = IL_PIXMAP(hi16_filesys_folder_violet);
			poulateScriptTree("[EDIT]",&pix,&folderPix, this, afst);
		}

	}
	else
	{

		FilesystemTree* fst = new FilesystemTree();

		std::string path = ConfigGodClient::getData().localServerDataPath;
		path += "/script";

		fst->setRootPath(path);
		fst->setFilter("*.class");
		fst->populateTree();

		{
			const QPixmap pix       = IL_PIXMAP(hi16_mime_document);
			const QPixmap folderPix = IL_PIXMAP(hi16_filesys_folder_green);
			poulateScriptTree(0,&pix,&folderPix, this, fst);
		}

		delete fst;
	}

	unsetCursor();
}

//-----------------------------------------------------------------

/**
 * Show a right click menu, mimicing the "Menu-Script" menu
 */
void ScriptListView::onContextMenuRequested(QListViewItem* item, const QPoint& p, int)
{
	if(item == 0)
		return;

	QPopupMenu* const m_pop = new QPopupMenu(this, "menu");

	ActionsScript* as =&ActionsScript::getInstance();

	IGNORE_RETURN(as->refresh->addTo(m_pop));

	IGNORE_RETURN(m_pop->insertSeparator());

	IGNORE_RETURN(as->create->addTo (m_pop));
	IGNORE_RETURN(as->edit->addTo   (m_pop));
	IGNORE_RETURN(as->view->addTo   (m_pop));
	IGNORE_RETURN(as->revert->addTo (m_pop));
	IGNORE_RETURN(as->submit->addTo (m_pop));

	IGNORE_RETURN(m_pop->insertSeparator());

	IGNORE_RETURN(as->compile->addTo(m_pop));
	IGNORE_RETURN(as->serverReload->addTo(m_pop));

	m_pop->popup(p);
} //lint !e818 item "could" be const, but Qt allows us to change it

// ======================================================================
