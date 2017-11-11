// ======================================================================
//
// LogicalAnimationTableWidget.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "AnimationEditor/FirstAnimationEditor.h"
#include "AnimationEditor/LogicalAnimationTableWidget.h"
#include "LogicalAnimationTableWidget.moc"

#include "AnimationEditor/EditorListView.h"
#include "AnimationEditor/FolderListItem.h"
#include "AnimationEditor/LatEntryFolderListItem.h"
#include "AnimationEditor/LatFormatUiBuilder.h"
#include "AnimationEditor/LogicalAnimationListItem.h"
#include "clientSkeletalAnimation/AnimationStateHierarchyTemplate.h"
#include "clientSkeletalAnimation/LogicalAnimationTableTemplate.h"
#include "sharedFile/Iff.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/CrcLowerString.h"

#include <qclipboard.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qstring.h>
#include <set>
#include <string>

// ======================================================================

namespace LogicalAnimationTableWidgetNamespace
{
	typedef std::set<CrcLowerString>  CrcLowerStringSet;
}

using namespace LogicalAnimationTableWidgetNamespace;

// ======================================================================

const std::string LogicalAnimationTableWidget::cms_directorySeparatorCharacters("\\/");
const std::string LogicalAnimationTableWidget::cms_fileTypeShortName("LAT");
const std::string LogicalAnimationTableWidget::cms_newFileTitleBase("new");
const QString     LogicalAnimationTableWidget::cms_templateNameSortKey("1:");
const QString     LogicalAnimationTableWidget::cms_logicalAnimationFolderSortKey("2:");

int LogicalAnimationTableWidget::ms_newFileCount;

// ======================================================================
// class LogicalAnimationTableWidget: public member functions
// ======================================================================

LogicalAnimationTableWidget::LogicalAnimationTableWidget(QWidget *newParent, LogicalAnimationTableTemplate &latTemplate, const std::string &pathName) :
	BaseLogicalAnimationTableWidget(newParent),
	BasicFileModel(),
	m_latTemplate(latTemplate),
	m_pathName(new std::string(pathName)),
	m_fileTitle(pathName.empty() ? new std::string(getNewFileTitleName()) : new std::string("")),
	m_templateNameListItem(0),
	m_logicalAnimationFolder(0)
{
	//-- Fetch local reference.
	m_latTemplate.fetch();

	//-- Delete all entries initially filled in by Qt.
	IGNORE_RETURN(m_listView->addColumn(trUtf8("Logical Animation Mapping")));

	{
		QListViewItem *item = new QListViewItem(m_listView, 0);
		item->setText(0, trUtf8("New Item"));
	} //lint !e429 // Custodial pointer 'item' has not been freed or returned // QT owns this item.

	m_listView->setLineWidth(2);
	m_listView->setShowSortIndicator(false);

	m_listView->clear();

	//-- Decorate root entries in tree view.
	m_listView->setRootIsDecorated(true);

	//-- Create Ui.
	// Add ash template ui element.
	m_templateNameListItem = new ListItem(m_listView, &cms_templateNameSortKey);
	m_templateNameListItem->setText(0, getTemplateNameItemText().c_str());

	// Build the Ui.
	LatFormatUiBuilder::buildUi(m_latTemplate, *this);
	m_listView->show();

	//-- Setup popup menu handler.
	bool result = connect(m_listView, SIGNAL(rightButtonClicked(QListViewItem *, const QPoint &, int)), this, SLOT(doListItemPopupMenu(QListViewItem *, const QPoint &, int)));
	DEBUG_FATAL(!result, ("failed to setup list view right click handler."));

	//-- Connect list view double click to the double click handler.  This will call the item's double click function.
	result = connect(m_listView, SIGNAL(doubleClicked(QListViewItem *)), this, SLOT(doDoubleClick(QListViewItem *)));
	DEBUG_FATAL(!result, ("failed to setup LAT list view double click handler."));

	UNREF(result);
}

// ----------------------------------------------------------------------

LogicalAnimationTableWidget::~LogicalAnimationTableWidget()
{
	//-- Zero Qt-owned elements.
	//lint -esym(423, LogicalAnimationTableWidget::m_templateNameListItem) // creating memory leak.  No, owned and delete by Qt.
	m_templateNameListItem = 0;

	//lint -esym(423, LogicalAnimationTableWidget::m_logicalAnimationFolder) // creating memory leak.  No, owned and deleted by Qt.
	m_logicalAnimationFolder = 0;

	delete m_fileTitle;
	delete m_pathName;

	//-- Release local reference.
	m_latTemplate.release();
}

// ----------------------------------------------------------------------

void LogicalAnimationTableWidget::keyPressEvent(QKeyEvent *keypressEvent)
{

	if (!keypressEvent)
		return;

	if (keypressEvent->state() ==  QObject::ControlButton && keypressEvent->key() == static_cast<int>(QObject::Key_C))
	{
		ListItem *const listItem = dynamic_cast<ListItem*>(m_listView->selectedItem());
		if (listItem)
		{
			QClipboard *cb = QApplication::clipboard();
			cb->setText( listItem->text(0),QClipboard::Clipboard );
		}
		return;
	}
	else if (keypressEvent->state() ==  QObject::ControlButton && keypressEvent->key() == static_cast<int>(QObject::Key_V))
	{
		ListItem *const listItem = dynamic_cast<ListItem*>(m_listView->selectedItem());
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
	BaseLogicalAnimationTableWidget::keyPressEvent(keypressEvent);
}

// ----------------------------------------------------------------------

bool LogicalAnimationTableWidget::save()
{
	//-- Get a pathname for the file if the user hasn't provided one already.
	if (m_pathName->empty())
	{
		const QString qPathName = QFileDialog::getSaveFileName(QString::null, "LAT Files (*.lat)", this, "save file dialog", "Choose a save name");
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

	//-- Save the file.
	// Save as IFF.
	Iff iff(8 * 1024 * 1024);
	m_latTemplate.write(iff);

	const bool saveResult = iff.write(iffWritePathName.c_str(), true);
	if (!saveResult)
	{
		char buffer[512];
		snprintf(buffer, sizeof(buffer), "failed to write IFF-based LAT file to path [%s].", iffWritePathName.c_str());
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
	}
	DEBUG_REPORT_LOG(true, ("Will save XML LAT to location [%s].\n", xmlWritePathName.c_str()));

	const bool xmlSaveResult = m_latTemplate.writeXml(xmlWritePathName.c_str());
	if (!xmlSaveResult)
	{
		char buffer[512];
		snprintf(buffer, sizeof(buffer), "failed to write XML-based LAT file to path [%s].", xmlWritePathName.c_str());
		buffer[sizeof(buffer) - 1] = '\0';

		WARNING(true, (buffer));
		QMessageBox::warning(this, "XML Save Failure", buffer, QMessageBox::Ok, QMessageBox::NoButton);
	}

	//-- Update modified status.
	if (saveResult)
		setModifiedState(false);

	//-- Return result.
	return saveResult;
}

// ----------------------------------------------------------------------

const std::string &LogicalAnimationTableWidget::getFileTypeShortName() const
{
	return cms_fileTypeShortName;
}

// ----------------------------------------------------------------------

CrcString const &LogicalAnimationTableWidget::getFilePath() const
{
	return m_latTemplate.getName();
}

// ----------------------------------------------------------------------

void LogicalAnimationTableWidget::setFileTitle(const std::string &shortName)
{
	*m_fileTitle = shortName;
}

// ----------------------------------------------------------------------

const std::string LogicalAnimationTableWidget::getFileTitle() const
{
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

QWidget *LogicalAnimationTableWidget::getWidget()
{
	return this;
}

// ----------------------------------------------------------------------

void LogicalAnimationTableWidget::expandAllChildrenForSelectedItem()
{
	if (!m_listView)
		return;

	ListItem *const listItem = dynamic_cast<ListItem*>(m_listView->selectedItem());
	if (listItem)
		listItem->expandItemAndChildren();
}

// ----------------------------------------------------------------------

void LogicalAnimationTableWidget::collapseAllChildrenForSelectedItem()
{
	if (!m_listView)
		return;

	ListItem *const listItem = dynamic_cast<ListItem*>(m_listView->selectedItem());
	if (listItem)
		listItem->collapseItemAndChildren();
}

// ----------------------------------------------------------------------

void LogicalAnimationTableWidget::selectMatchingItem(const char* string, bool onlyReplaceableTextItems)
{
	if(!m_listView)
	{
		return;
	}
	
	ListItem *const selectedItem = dynamic_cast<ListItem*>(m_listView->selectedItem());
	ListItem *const firstChild = dynamic_cast<ListItem*>(m_listView->firstChild());
	
	if(!firstChild)
	{
		QMessageBox::warning(this, "LogicalAnimationTableWidget::selectMatchingItem", 
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
			m_listView->setSelected(foundMatchingListItem,true);
			m_listView->ensureItemVisible(foundMatchingListItem);
			break;
		}
		listItem = dynamic_cast<ListItem*>(listItem->nextSibling());
	}
}

// ----------------------------------------------------------------------

void LogicalAnimationTableWidget::replaceSelectedText(const char* substringToReplace, const char* newText)
{
	if(!m_listView)
	{
		return;
	}

	ListItem* const selectedItem = dynamic_cast<ListItem*>(m_listView->selectedItem());
	if(!selectedItem)
	{
		QMessageBox::warning(this, "Replace error", "Nothing selected", QMessageBox::Ok, QMessageBox::NoButton);
		return;
	}

	// JU_TODO: debug
#if 1
	DEBUG_REPORT_LOG(true,("LogicalAnimationTableWidget::replaceSelectedText selectedItem type is %d\n",selectedItem->getType()));
#endif	
	// JU_TODO: end debug

	if(selectedItem->canReplaceText())
	{
		selectedItem->replaceText(substringToReplace,newText);
	}
}

void LogicalAnimationTableWidget::replaceAllText(const char* substringToReplace, const char* newText)
{
	if(!m_listView)
	{
		return;
	}
	
	ListItem *const firstChild = dynamic_cast<ListItem*>(m_listView->firstChild());
	
	if(!firstChild)
	{
		QMessageBox::warning(this, "LogicalAnimationTableWidget::replaceAllText", 
			"FirstChild not found - cancelled", QMessageBox::Ok, QMessageBox::NoButton);
		return;
	}

	ListItem * listItem = firstChild;
	
	while(listItem != NULL)
	{
		ListItem *const selectedItem = dynamic_cast<ListItem*>(m_listView->selectedItem());
		bool ignore = false;
		ListItem* foundMatchingListItem = listItem->findItemMatch(substringToReplace,selectedItem,ignore,true);
		if(foundMatchingListItem)
		{
			m_listView->setSelected(foundMatchingListItem,true);
			m_listView->ensureItemVisible(foundMatchingListItem);
			foundMatchingListItem->replaceText(substringToReplace, newText);
		}
		else
		{
			listItem = dynamic_cast<ListItem*>(listItem->nextSibling());
		}
	}

}

// ----------------------------------------------------------------------

LogicalAnimationListItem *LogicalAnimationTableWidget::addLogicalAnimationNameUi(const CrcLowerString &logicalAnimationName)
{
	// @todo check to make sure this named child doesn't already exist.
	return new LogicalAnimationListItem(m_logicalAnimationFolder, m_latTemplate, logicalAnimationName, *this);
}

// ----------------------------------------------------------------------

void LogicalAnimationTableWidget::clearLogicalAnimationUi()
{
	//-- I don't see a removeAllChildren()/clear() call, so it's easier for me to delete this node then to walk the children.
	delete m_logicalAnimationFolder;

	// Re-add logical animation name folder ui element.
	//m_logicalAnimationFolder = new FolderListItem(m_listView, cms_logicalAnimationFolderText, FolderListItem::FC_green, &cms_logicalAnimationFolderSortKey);
	m_logicalAnimationFolder = new LatEntryFolderListItem(m_listView, *this, &cms_logicalAnimationFolderSortKey);
	m_logicalAnimationFolder->expandItemAndParents();
}

// ----------------------------------------------------------------------

void LogicalAnimationTableWidget::setModifiedState(bool newModifiedState)
{
	if (newModifiedState)
	{
		//-- Force LAT to rebuild .ash -> .lat table.
		// m_latTemplate.handleHierarchyChange();
	}

	//-- Get BasicFileModel to handle this.
	BasicFileModel::setModifiedState(newModifiedState);
}

// ======================================================================
// class LogicalAnimationTableWidget: public slot member functions
// ======================================================================

void LogicalAnimationTableWidget::doListItemPopupMenu(QListViewItem *item, const QPoint &location, int /* column */)
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
} //lint !e1762 // function could be made const // I'm bound to the slot definition. @todo check if slot definition in .uic file could be made const.

// ----------------------------------------------------------------------

void LogicalAnimationTableWidget::doDoubleClick(QListViewItem *item)
{
	//-- Check if the item is a ListItem-derived instance.
	ListItem *const listItem = dynamic_cast<ListItem*>(item);
	if (!listItem)
		return;

	//-- Undo the collapse/open.
	item->setOpen(!item->isOpen());

	//-- Update the list item visuals.
	listItem->doDoubleClick();
} //lint !e1762 // could be made const function // maybe so, but this is a slot and I don't yet know if Qt can deal with that.

// ----------------------------------------------------------------------

void LogicalAnimationTableWidget::addNewLatEntries()
{
#if 1
	//-- Collect set of unique logical animation names used by the animation state hierarchy.

	// Get the ash template.
	const AnimationStateHierarchyTemplate *const ashTemplate = m_latTemplate.fetchConstHierarchyTemplate();
	if (!ashTemplate)
		return;

	// Get the referenced logical animation names.
	CrcLowerStringSet  animationNameSet;
	ashTemplate->addReferencedLogicalAnimationNames(animationNameSet);
	
	const CrcLowerStringSet::iterator endIt = animationNameSet.end();
	for (CrcLowerStringSet::iterator it = animationNameSet.begin(); it != endIt; ++it)
	{
		// Get logical animation name.
		const CrcLowerString &logicalAnimationName = *it;

		// Add empty entry for logical animation name.
		if (!m_latTemplate.hasLogicalAnimationName(logicalAnimationName))
			IGNORE_RETURN(addLogicalAnimationNameUi(logicalAnimationName));
	}

	//-- Release local reference.
	ashTemplate->release();
#else
	// @todo this probably needs to pass in a set of CrcLowerString names to be
	//       filled by the ASH template.
	//
	//       This should not blow away the LAT UI.  Instead, it should only add
	//       newer entries to the UI.


	//-- Delete the logical animation UI.
	clearLogicalAnimationUi();

	//-- Rebuild the LAT data.
	m_latTemplate.createUnspecifiedEntries();
	//m_latTemplate.handleHierarchyChange();

	//-- Recreate the logical animation UI.
	LatFormatUiBuilder::buildUiForAllAnimations(m_latTemplate, *this);
#endif
}

// ----------------------------------------------------------------------

void LogicalAnimationTableWidget::deleteStaleLatEntries()
{
	m_latTemplate.removeEntriesNotSpecifiedInAsh();
	setModifiedState(true);

	//-- Delete the logical animation UI.
	clearLogicalAnimationUi();

	//-- Rebuild the LAT data.
	m_latTemplate.createUnspecifiedEntries();
	//m_latTemplate.handleHierarchyChange();

	//-- Recreate the logical animation UI.
	LatFormatUiBuilder::buildUi(m_latTemplate, *this);
}

// ======================================================================
// class LogicalAnimationTableWidget: private static member functions
// ======================================================================

std::string LogicalAnimationTableWidget::getNewFileTitleName()
{
	//-- Keep track of # new files during this run.
	++ms_newFileCount;

	//-- Convert file count to string.
	char countAsString[32];
	sprintf(countAsString, "%d", ms_newFileCount);
	
	return cms_newFileTitleBase + countAsString;
}

// ======================================================================
// class LogicalAnimationTableWidget: private member functions
// ======================================================================

std::string LogicalAnimationTableWidget::getTemplateNameItemText() const
{
	std::string itemText("Template Name: ");

	const AnimationStateHierarchyTemplate *ashTemplate     = m_latTemplate.fetchConstHierarchyTemplate();
	if (ashTemplate)
	{
		IGNORE_RETURN(itemText.append(ashTemplate->getName().getString()));
		ashTemplate->release();
	}

	return itemText;
}

// ======================================================================
