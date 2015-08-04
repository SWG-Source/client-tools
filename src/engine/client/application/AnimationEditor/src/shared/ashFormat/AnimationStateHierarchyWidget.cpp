// ======================================================================
//
// AnimationStateHierarchyWidget.cpp
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#include "AnimationEditor/FirstAnimationEditor.h"
#include "AnimationEditor/AnimationStateHierarchyWidget.h"

#include "AnimationEditor/ActionGroupRootListItem.h"
#include "AnimationEditor/AshFormatUiBuilder.h"
#include "AnimationEditor/DetailStateListItem.h"
#include "AnimationEditor/ListItemAction.h"
#include "clientSkeletalAnimation/EditableAnimationStateHierarchyTemplate.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/CrcLowerString.h"

#include <qapplication.h>
#include <qclipboard.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qtabwidget.h>

// ======================================================================

const std::string AnimationStateHierarchyWidget::cms_fileTypeShortName("ASH");
const std::string AnimationStateHierarchyWidget::cms_directorySeparatorCharacters("/\\");

// ======================================================================
/**
 * Constructor to use when a new state hierarchy should be created.
 */

AnimationStateHierarchyWidget::AnimationStateHierarchyWidget(QWidget* newParent, const char* newName, WFlags fl) :
	BaseAnimationStateHierarchyWidget(newParent, newName, fl),
	BasicFileModel(),
	m_hierarchy(new EditableAnimationStateHierarchyTemplate()),
	m_fileTitle(new std::string("unnamed")),
	m_pathName(new std::string()),
	m_listItemAction(0),
	m_actionGroupRootListItem(0)
{
	doCommonConstructionTasks();

	//-- Populate state hierarchy tree with first item.
	//lint -esym(429, newItem) // variable not freed or returned // Qt owns it.
	QListViewItem *const newItem = new DetailStateListItem(*this, m_hierarchy->getRootAnimationState(), m_stateHierarchyListView);
	UNREF(newItem);
}

// ----------------------------------------------------------------------
/**
 * Constructor to use with an already-existing state hierarchy.
 */

AnimationStateHierarchyWidget::AnimationStateHierarchyWidget(EditableAnimationStateHierarchyTemplate *hierarchy, const std::string &pathName, QWidget* newParent, const char* newName, WFlags fl) :
	BaseAnimationStateHierarchyWidget(newParent, newName, fl),
	BasicFileModel(),
	m_hierarchy(hierarchy),
	m_fileTitle(new std::string("")),
	m_pathName(new std::string(pathName)),
	m_listItemAction(0),
	m_actionGroupRootListItem(0)
{
	doCommonConstructionTasks();

	//-- Populate the UI.
	//lint -esym(429, newItem) // variable not freed or returned // Qt owns it.
	DetailStateListItem *const newItem = new DetailStateListItem(*this, m_hierarchy->getRootAnimationState(), m_stateHierarchyListView);
	UNREF(newItem);

	AshFormatUiBuilder::buildStateUi(*NON_NULL(m_hierarchy->getRootAnimationState()), *newItem);
}

// ----------------------------------------------------------------------

AnimationStateHierarchyWidget::~AnimationStateHierarchyWidget()
{
	delete m_listItemAction;

	delete m_pathName;
	delete m_fileTitle;

	//lint -esym(1740, AnimationStateHierarchyWidget::m_hierarchy) // not freed or zero'ed // reference count released
	m_hierarchy->release();

	m_actionGroupRootListItem = 0;
}

// ----------------------------------------------------------------------

void AnimationStateHierarchyWidget::setListItemAction(ListItemAction *action)
{
	//-- Test for assignment to self.
	if (action == m_listItemAction)
		return;

	//-- Delete existing action.
	if (m_listItemAction)
		delete m_listItemAction;

	//-- Assign new action.
	m_listItemAction = action;
}

// ----------------------------------------------------------------------

bool AnimationStateHierarchyWidget::save()
{
	//-- Get a pathname for the file if the user hasn't provided one already.
	if (m_pathName->empty())
	{
		const QString qPathName = QFileDialog::getSaveFileName(QString::null, "ASH Files (*.ash)", this, "save file dialog", "Choose a save name");
		if (qPathName == QString::null)
		{
			// user aborted
			return false;
		}

		*m_pathName = static_cast<const char*>(qPathName);
	}

	if (m_pathName->empty())
	{
		// User must have punted from name choosing operation.
		WARNING(true, ("save(): skipping save, no pathname was specified."));
		return false;
	}

	//-- Convert TreeFile-relative path to full directory path.
	char writePathName[2048];
	if (!TreeFile::getPathName(m_pathName->c_str(), writePathName, sizeof(writePathName) - 1))
	{
		// The conversion from the treefile pathname to the full drive pathname failed, so try to use the treefile pathname.
		// Perhaps it was not TreeFile-relative to begin with.
		strcpy(writePathName, m_pathName->c_str());
		WARNING(true, ("unexpected: m_pathName [%s] should be TreeFile relative but couldn't be converted to full pathname.", m_pathName->c_str()));
	}

	//-- Figure out the iff filename based on the presence of the .xml extension.
	std::string  basePathName(writePathName);
	std::string  iffWritePathName(basePathName);

	std::string::size_type  xmlExtensionStartPosition = basePathName.find(".xml");
	if (static_cast<int>(xmlExtensionStartPosition) != static_cast<int>(std::string::npos))
	{
		// chop off the .xml extension.
		iffWritePathName.resize(xmlExtensionStartPosition);

		// convert the first ref to dsrc, if any, to data.
		std::string::size_type const dsrcStartPos = iffWritePathName.find("dsrc");
		if (static_cast<int>(dsrcStartPos) != static_cast<int>(std::string::npos))
			iffWritePathName.replace(dsrcStartPos, 4, "data");
	}

	//-- Save the hierarchy.
	// Save it in the original binary format.
	// @todo remove this once we only want to write in XML.
	const bool saveResult = m_hierarchy->write(iffWritePathName.c_str());
	if (!saveResult)
	{
		char buffer[512];
		snprintf(buffer, sizeof(buffer), "failed to write IFF-based ASH file to path [%s].", iffWritePathName.c_str());
		buffer[sizeof(buffer) - 1] = '\0';

		WARNING(true, (buffer));
		QMessageBox::warning(this, "IFF Save Failure", buffer, QMessageBox::Ok, QMessageBox::NoButton);
	}

	// Save it in XML.
	std::string xmlWritePathName(iffWritePathName);
	xmlWritePathName.append(".xml");

	// Replace /data/ with /dsrc/ if config file option indicates this.
	if (ConfigFile::getKeyBool("AnimationEditor", "saveXmlInDsrc", true))
	{
		std::string::size_type const dataStartPos = xmlWritePathName.find("data");
		if (static_cast<int>(dataStartPos) != static_cast<int>(std::string::npos))
		{
			// Replace data with dsrc
			xmlWritePathName.replace(dataStartPos, 4, "dsrc");
		}
		else
		{
			// Couldn't find data in the path.
			WARNING(true, ("[AnimationEditor] saveXmlInDsrc is set to true but no 'data' existed in the original XML write name [%s], using original path name.", xmlWritePathName.c_str()));
		}
	}
	DEBUG_REPORT_LOG(true, ("Will save XML ASH to location [%s].\n", xmlWritePathName.c_str()));

	const bool xmlSaveResult = m_hierarchy->writeXml(xmlWritePathName.c_str());
	if (!xmlSaveResult)
	{
		char buffer[512];
		snprintf(buffer, sizeof(buffer), "failed to write XML-based ASH file to path [%s].", xmlWritePathName.c_str());
		buffer[sizeof(buffer) - 1] = '\0';

		WARNING(true, (buffer));
		QMessageBox::warning(this, "XML Save Failure", buffer, QMessageBox::Ok, QMessageBox::NoButton);
	}

	//-- Update modified status.
	if (saveResult && xmlSaveResult)
		setModifiedState(false);
	

	//-- Return result.
	return saveResult;
}

// ----------------------------------------------------------------------

const std::string &AnimationStateHierarchyWidget::getFileTypeShortName() const
{
	return cms_fileTypeShortName;
}

// ----------------------------------------------------------------------

CrcString const &AnimationStateHierarchyWidget::getFilePath() const
{
	if (m_hierarchy)
		return m_hierarchy->getName();
	else
		return CrcLowerString::empty;
}

// ----------------------------------------------------------------------

void AnimationStateHierarchyWidget::setFileTitle(const std::string &shortName)
{
	*m_fileTitle = shortName;
}

// ----------------------------------------------------------------------

const std::string AnimationStateHierarchyWidget::getFileTitle() const
{
	//-- use the file title if the pathname is not set; otherwise, use
	//   the file name portion of the path name.
	if (m_pathName->empty())
		return *m_fileTitle;
	else
	{
		std::string::size_type directoryEndPosition = m_pathName->find_last_of(cms_directorySeparatorCharacters);
		if (static_cast<int>(directoryEndPosition) == static_cast<int>(std::string::npos))
		{
			// no directory separator in the pathname, return the whole thing
			return *m_pathName;
		}
		else
		{
			// directory separator present, strip it out
			return std::string(*m_pathName, directoryEndPosition + 1);
		}
	}
}

// ----------------------------------------------------------------------

QWidget *AnimationStateHierarchyWidget::getWidget()
{
	return this;
}

// ----------------------------------------------------------------------

void AnimationStateHierarchyWidget::expandAllChildrenForSelectedItem()
{
	if (!m_tabWidget)
		return;

	QListView *listView = 0;

	const int currentPageIndex = m_tabWidget->currentPageIndex();
	if (currentPageIndex == 0)
		listView = m_stateHierarchyListView;
	else
		listView = m_actionGroupListView;

	ListItem *const listItem = dynamic_cast<ListItem*>(listView->selectedItem());
	if (listItem)
		listItem->expandItemAndChildren();
}

// ----------------------------------------------------------------------

void AnimationStateHierarchyWidget::collapseAllChildrenForSelectedItem()
{
	if (!m_tabWidget)
		return;

	QListView *listView = 0;

	const int currentPageIndex = m_tabWidget->currentPageIndex();
	if (currentPageIndex == 0)
		listView = m_stateHierarchyListView;
	else
		listView = m_actionGroupListView;

	ListItem *const listItem = dynamic_cast<ListItem*>(listView->selectedItem());
	if (listItem)
		listItem->collapseItemAndChildren();
}


void AnimationStateHierarchyWidget::selectMatchingItem(const char* string, bool onlyReplaceableTextItems)
{
	if(!m_tabWidget || !m_stateHierarchyListView || !m_actionGroupListView)
	{
		return;
	}

	QListView *listView = 0;

	const int currentPageIndex = m_tabWidget->currentPageIndex();
	if (currentPageIndex == 0)
	{
		listView = m_stateHierarchyListView;
	}
	else
	{
		listView = m_actionGroupListView;
	}

	ListItem *const selectedItem = dynamic_cast<ListItem*>(listView->selectedItem());
	ListItem *const firstChild = dynamic_cast<ListItem*>(listView->firstChild());
	if(!firstChild)
	{
		QMessageBox::warning(this, "AnimationStateHierarchyWidget::selectMatchingItem", 
			"FirstChild not found - cancelled", QMessageBox::Ok, QMessageBox::NoButton);
		return;
	}

	ListItem * listItem = firstChild;
	while(listItem != NULL)
	{
		// ignore is set until the selected item is passed up in the treewalk
		bool ignore = (selectedItem) ? true : false;
		ListItem* foundMatchingListItem = listItem->findItemMatch(string,selectedItem,ignore,onlyReplaceableTextItems);
		if(foundMatchingListItem && foundMatchingListItem != selectedItem)
		{
			listView->setSelected(foundMatchingListItem,true);
			listView->ensureItemVisible(foundMatchingListItem);
			break;
		}
		listItem = dynamic_cast<ListItem*>(listItem->nextSibling());
	}

}

// ----------------------------------------------------------------------

void AnimationStateHierarchyWidget::replaceSelectedText(const char* substringToReplace, const char* newText)
{
	if(!m_tabWidget || !m_stateHierarchyListView || !m_actionGroupListView)
	{
		return;
	}

	QListView *listView = 0;

	const int currentPageIndex = m_tabWidget->currentPageIndex();
	if (currentPageIndex == 0)
	{
		listView = m_stateHierarchyListView;
	}
	else
	{
		listView = m_actionGroupListView;
	}

	ListItem* const selectedItem = dynamic_cast<ListItem*>(listView->selectedItem());
	if(!selectedItem)
	{
		QMessageBox::warning(this, "Replace error", "Nothing selected", QMessageBox::Ok, QMessageBox::NoButton);
		return;
	}

	// JU_TODO: debug
#if 1
	DEBUG_REPORT_LOG(true,("AnimationStateHierarchyWidget::replaceSelectedText selectedItem type is %d\n",selectedItem->getType()));
#endif	
	// JU_TODO: end debug

	if(selectedItem->canReplaceText())
	{
		selectedItem->replaceText(substringToReplace,newText);
	}
}

void AnimationStateHierarchyWidget::replaceAllText(const char* substringToReplace, const char* newText)
{
	if(!m_tabWidget || !m_stateHierarchyListView || !m_actionGroupListView)
	{
		return;
	}

	QListView *listView = 0;

	const int currentPageIndex = m_tabWidget->currentPageIndex();
	if (currentPageIndex == 0)
	{
		listView = m_stateHierarchyListView;
	}
	else
	{
		listView = m_actionGroupListView;
	}
	
	ListItem *const firstChild = dynamic_cast<ListItem*>(listView->firstChild());
	
	if(!firstChild)
	{
		QMessageBox::warning(this, "AnimationStateHierarchyWidget::replaceAllText", 
			"FirstChild not found - cancelled", QMessageBox::Ok, QMessageBox::NoButton);
		return;
	}

	ListItem * listItem = firstChild;
	
	while(listItem != NULL)
	{
		ListItem *const selectedItem = dynamic_cast<ListItem*>(listView->selectedItem());
		bool ignore = false;
		ListItem* foundMatchingListItem = listItem->findItemMatch(substringToReplace,selectedItem,ignore,true);
		if(foundMatchingListItem)
		{
			listView->setSelected(foundMatchingListItem,true);
			listView->ensureItemVisible(foundMatchingListItem);
			foundMatchingListItem->replaceText(substringToReplace, newText);
		}
		else
		{
			listItem = dynamic_cast<ListItem*>(listItem->nextSibling());
		}
	}
}

// ----------------------------------------------------------------------

void AnimationStateHierarchyWidget::doListItemPopupMenu(QListViewItem *item, const QPoint &location, int /* column */)
{
	//-- Check if the item is a ListItem-derived instance.
	ListItem *const listItem = dynamic_cast<ListItem*>(item);
	if (!listItem)
		return;

	//-- Check if the item has a popup menu.
	if (!listItem->supportsPopupMenu())
		return;

	//-- Execute the popup menu.
	QPopupMenu *menu = listItem->createPopupMenu();
	if (!menu)
		return;

	//-- Handle the popup menu synchronously.  I need to do this
	//   so I can delete the menu when done.
	IGNORE_RETURN(menu->exec(location));

	delete menu;
}

// ----------------------------------------------------------------------

void AnimationStateHierarchyWidget::doUpdateListItemVisuals(QListViewItem *item)
{
	//-- Check if the item is a ListItem-derived instance.
	ListItem *const listItem = dynamic_cast<ListItem*>(item);
	if (!listItem)
		return;

	//-- Update the list item visuals.
	listItem->updateVisuals();
}

// ----------------------------------------------------------------------

void AnimationStateHierarchyWidget::doDoubleClick(QListViewItem *item)
{
	//-- Check if the item is a ListItem-derived instance.
	ListItem *const listItem = dynamic_cast<ListItem*>(item);
	if (!listItem)
		return;

	//-- Undo the collapse/open.
	item->setOpen(!item->isOpen());

	//-- Update the list item visuals.
	listItem->doDoubleClick();
}

// ----------------------------------------------------------------------

void AnimationStateHierarchyWidget::handleListItemSelectionChange(QListViewItem *targetItem)
{
	if (m_listItemAction && targetItem)
	{
		//-- Check if the pending list item action will accept the selected target.
		if (m_listItemAction->isTargetAcceptable(*targetItem))
		{
			//-- Perform the action.
			m_listItemAction->doAction(*targetItem);

			//-- Delete the action --- they're all one shot.
			delete m_listItemAction;
			m_listItemAction = 0;
		}
	}
}

void AnimationStateHierarchyWidget::keyPressEvent(QKeyEvent *keypressEvent)
{
	if (!keypressEvent)
		return;

	if (!m_tabWidget)
		return;

	QListView *listView = 0;

	const int currentPageIndex = m_tabWidget->currentPageIndex();
	if (currentPageIndex == 0)
		listView = m_stateHierarchyListView;
	else
		listView = m_actionGroupListView;

	if (keypressEvent->state() ==  QObject::ControlButton && keypressEvent->key() == static_cast<int>(QObject::Key_C))
	{
		ListItem *const listItem = dynamic_cast<ListItem*>(listView->selectedItem());
		if (listItem)
		{
			QClipboard *cb = QApplication::clipboard();
			cb->setText( listItem->text(0),QClipboard::Clipboard );
		}
		return;
	}
	else if (keypressEvent->state() ==  QObject::ControlButton && keypressEvent->key() == static_cast<int>(QObject::Key_V))
	{
		ListItem *const listItem = dynamic_cast<ListItem*>(listView->selectedItem());
		if (listItem)
		{
			if(listItem->canReplaceText())
			{
				QClipboard *cb = QApplication::clipboard();
				QString text = cb->text(QClipboard::Clipboard);
				if ( !text.isNull() )
				{
					listItem->replaceText(NULL,text.ascii());
				}
			}
			else
			{
				QMessageBox::warning(this, "paste error", "This item cannot be pasted into", QMessageBox::Ok, QMessageBox::NoButton);
			}	
		}
		return;
	}

	//-- We didn't handle it, let parent class try.`x
	BaseAnimationStateHierarchyWidget::keyPressEvent(keypressEvent);
}

// ======================================================================
// class AnimationStateHierarchyWidget: private member functions
// ======================================================================

void AnimationStateHierarchyWidget::doCommonConstructionTasks()
{
	//-- Fetch a reference to the new hierarchy.
	NOT_NULL(m_hierarchy);
	m_hierarchy->fetch();

	//-- Clear list views.
	m_stateHierarchyListView->clear();
	m_actionGroupListView->clear();

	//-- Set root decoration for list views.
	m_stateHierarchyListView->setRootIsDecorated(true);
	m_actionGroupListView->setRootIsDecorated(true);

	//-- Create root action group node.
	m_actionGroupRootListItem = new ActionGroupRootListItem(m_actionGroupListView, *m_hierarchy, *this);

	//-- Connect list view right click signals to the popup handler slot.
	bool  result;

	result = connect(m_stateHierarchyListView, SIGNAL(rightButtonClicked(QListViewItem *, const QPoint &, int)), this, SLOT(doListItemPopupMenu(QListViewItem *, const QPoint &, int)));
	DEBUG_FATAL(!result, ("failed to setup state hierarchy list view right click handler."));

	result = connect(m_actionGroupListView, SIGNAL(rightButtonClicked(QListViewItem *, const QPoint &, int)), this, SLOT(doListItemPopupMenu(QListViewItem *, const QPoint &, int)));
	DEBUG_FATAL(!result, ("failed to setup action group list view right click handler."));

	//-- Connect list view expand/collapse to ListItem::updateVisuals().
	result = connect(m_stateHierarchyListView, SIGNAL(expanded(QListViewItem*)), this, SLOT(doUpdateListItemVisuals(QListViewItem *)));
	DEBUG_FATAL(!result, ("failed to setup state hierarchy list view expanded connection."));

	result = connect(m_stateHierarchyListView, SIGNAL(collapsed(QListViewItem*)), this, SLOT(doUpdateListItemVisuals(QListViewItem *)));
	DEBUG_FATAL(!result, ("failed to setup state hierarchy list view collapsed connection."));

	result = connect(m_actionGroupListView, SIGNAL(expanded(QListViewItem*)), this, SLOT(doUpdateListItemVisuals(QListViewItem *)));
	DEBUG_FATAL(!result, ("failed to setup action group list view expanded connection."));

	result = connect(m_actionGroupListView, SIGNAL(collapsed(QListViewItem*)), this, SLOT(doUpdateListItemVisuals(QListViewItem *)));
	DEBUG_FATAL(!result, ("failed to setup action group list view collapsed connection."));

	//-- Connect list view ListItem selection to selection handler.  This is used by
	//   the ListItem action mechanism.
	result = connect(m_stateHierarchyListView, SIGNAL(selectionChanged(QListViewItem *)), this, SLOT(handleListItemSelectionChange(QListViewItem *)));
	DEBUG_FATAL(!result, ("failed to setup state hierarchy list view ListItem selected connection."));

	result = connect(m_actionGroupListView, SIGNAL(selectionChanged(QListViewItem *)), this, SLOT(handleListItemSelectionChange(QListViewItem *)));
	DEBUG_FATAL(!result, ("failed to setup action group view ListItem selected connection."));

	//-- Connect list view double click to the double click handler.  This will call the item's double click function.
	result = connect(m_stateHierarchyListView, SIGNAL(doubleClicked(QListViewItem *)), this, SLOT(doDoubleClick(QListViewItem *)));
	DEBUG_FATAL(!result, ("failed to setup state hierarchy list view double click handler."));

	result = connect(m_actionGroupListView, SIGNAL(doubleClicked(QListViewItem *)), this, SLOT(doDoubleClick(QListViewItem *)));
	DEBUG_FATAL(!result, ("failed to setup action group list view double click handler."));
}

// ======================================================================
