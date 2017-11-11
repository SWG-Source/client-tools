// ======================================================================
//
// BuildoutAreaListView.cpp
//
// Copyright 2005 Sony Online Entertainment
//
// ======================================================================

#include "SwgGodClient/FirstSwgGodClient.h"
#include "BuildoutAreaListView.h"
#include "BuildoutAreaListView.moc"

#include "ActionHack.h"
#include "ActionsBuildoutArea.h"
#include "BuildoutAreaSupport.h"
#include "IconLoader.h"
#include "ServerCommander.h"
#include "clientGame/Game.h"
#include "clientGame/WorldSnapshot.h"

#include "QDragObject.h"
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qfiledialog.h>

// ======================================================================

BuildoutAreaListView::BuildoutAreaListView(QWidget *theParent, char const *theName) :
	QListView(theParent, theName)
{
	IGNORE_RETURN(QListView::addColumn("Name"));
	IGNORE_RETURN(QListView::addColumn("Location"));
	IGNORE_RETURN(QListView::addColumn("Status"));

	QListView::setResizeMode(QListView::NoColumn);
	QListView::setRootIsDecorated(true);
	QListView::setColumnWidthMode(0, QListView::Maximum);

	IGNORE_RETURN(connect(this, SIGNAL(contextMenuRequested(QListViewItem*, const QPoint&, int)), this, SLOT(onContextMenuRequested(QListViewItem*, const QPoint&, int))));
	IGNORE_RETURN(connect(this, SIGNAL(selectionChanged()), this, SLOT(onSelectionChanged())));

	ActionsBuildoutArea * const ab = &ActionsBuildoutArea::getInstance();

	IGNORE_RETURN(connect(ab->actionRefresh, SIGNAL(activated()), SLOT(onRefresh())));
	IGNORE_RETURN(connect(ab->actionEdit, SIGNAL(activated()), SLOT(onEdit())));
	IGNORE_RETURN(connect(ab->actionSave, SIGNAL(activated()), SLOT(onSave())));
}

// ----------------------------------------------------------------------

void BuildoutAreaListView::onRefresh()
{
	clear();
	
	setCursor(static_cast<int>(Qt::WaitCursor));

	std::vector<std::string> areaNames;
	std::vector<std::string> areaLocations;
	std::vector<std::string> areaStatuses;

	std::string const &sceneName = Game::getSceneId();

	if (!sceneName.empty())
	{
		BuildoutAreaSupport::getBuildoutAreaList(sceneName, areaNames, areaLocations, areaStatuses);

		// update the listbox
		{
			for (unsigned int i = 0; i < areaNames.size(); ++i)
			{
				QListViewItem * const item = new QListViewItem(this, areaNames[i].c_str());
				item->setText(1, areaLocations[i].c_str());
				item->setText(2, areaStatuses[i].c_str());
			}
		}
	}

	unsetCursor();
}

// ----------------------------------------------------------------------

void BuildoutAreaListView::onEdit()
{
	if (Game::getSinglePlayer())
		return;

	QListViewItem * const item = selectedItem();
	if (item)
	{
		std::string const &areaName = item->text(0).ascii();
		std::string const &areaLocation = item->text(1).ascii();
		std::string const &areaStatus = item->text(2).ascii();
		UNREF(areaName);
		UNREF(areaLocation);
		UNREF(areaStatus);
		DEBUG_REPORT_LOG(true, ("onEdit: name=%s loc=%s status=%s\n", areaName.c_str(), areaLocation.c_str(), areaStatus.c_str()));
		IGNORE_RETURN(ServerCommander::getInstance().editBuildoutArea(areaName));

		WorldSnapshot::setExcludeArea(areaName.c_str());
		WorldSnapshot::load("");
		WorldSnapshot::load(Game::getSceneId().c_str());
	}
}

// ----------------------------------------------------------------------

void BuildoutAreaListView::onSave()
{
	QListViewItem * const item = selectedItem();
	if (item)
	{
		std::string const &areaName = item->text(0).ascii();
		std::string const &areaLocation = item->text(1).ascii();
		std::string const &areaStatus = item->text(2).ascii();
		UNREF(areaName);
		UNREF(areaLocation);
		UNREF(areaStatus);
		DEBUG_REPORT_LOG(true, ("onSave: name=%s loc=%s status=%s\n", areaName.c_str(), areaLocation.c_str(), areaStatus.c_str()));
		IGNORE_RETURN(ServerCommander::getInstance().saveBuildoutArea(areaName, areaLocation));
	} 
}

// ----------------------------------------------------------------------

void BuildoutAreaListView::onSelectionChanged() const
{
	QListViewItem * const item = selectedItem();
	ActionsBuildoutArea * const ab = &ActionsBuildoutArea::getInstance();

	if (!item)
	{
		ab->actionEdit->setEnabled(false);
		ab->actionSave->setEnabled(false);
	}
	else
	{
		ab->actionEdit->setEnabled(true);
		ab->actionSave->setEnabled(true);
	}
}

// ----------------------------------------------------------------------

void BuildoutAreaListView::onContextMenuRequested(QListViewItem* item, const QPoint& p, int)
{
	UNREF(item);
	QPopupMenu * const m_pop = new QPopupMenu(this, "menu");

	ActionsBuildoutArea * const ab = &ActionsBuildoutArea::getInstance();

	IGNORE_RETURN(ab->actionRefresh->addTo(m_pop));

	IGNORE_RETURN(m_pop->insertSeparator());

	if (!Game::getSinglePlayer())
		IGNORE_RETURN(ab->actionEdit->addTo(m_pop));
	IGNORE_RETURN(ab->actionSave->addTo(m_pop));

	m_pop->popup(p);
}

// ======================================================================
