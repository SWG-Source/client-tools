// ======================================================================
//
// DepotListViewItem.cpp
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "DepotListViewItem.h"

#include "ApplicationWindow.h"
#include "DirsClientUser.h"
#include "FstatClientUser.h"
#include "Pixmaps.h"

// ======================================================================

DepotListViewItem::DepotListViewItem(ApplicationWindow &applicationWindow, bool file, QListView *parent, QString name)
: QListViewItem(parent),
	m_applicationWindow(applicationWindow),
	m_fullName(name),
	m_syncName(name),
	m_file(file),
	m_cached(false),
	m_count(0)
{
	if (!m_file)
	{
		setExpandable(true);
		m_syncName += QString("/...");
	}
	setText(0, m_fullName.section( '/', -1 ));
}

// ----------------------------------------------------------------------

DepotListViewItem::DepotListViewItem(ApplicationWindow &applicationWindow, bool file, QListViewItem *parent, QString name)
: QListViewItem(parent),
	m_applicationWindow(applicationWindow),
	m_fullName(name),
	m_syncName(name),
	m_file(file),
	m_cached(false),
	m_count(0)
{
	if (!m_file)
	{
		setExpandable(true);
		m_syncName += QString("/...");
	}
	setText(0, m_fullName.section( '/', -1 ));
}

// ----------------------------------------------------------------------

void DepotListViewItem::setOpen(bool o)
{
	if (o && !m_cached)
	{
		char buffer[256];
		sprintf(buffer, "%s/*", m_fullName.latin1());

		{
			std::vector<const char *> argv;
			argv.push_back("dirs");
			argv.push_back("-C");
			argv.push_back(buffer);
			DirsClientUser clientUser(*this);
			m_applicationWindow.p4(false, argv, clientUser);	
		}

		{
			std::vector<const char *> argv;
			argv.push_back("fstat");
			argv.push_back("-C");
			argv.push_back(buffer);
			FstatClientUser clientUser(*this);
			m_applicationWindow.p4(false, argv, clientUser);	
		}

		m_cached = true;
		if (m_count == 0)
		{
			setExpandable(false);
			return;
		}
	}

	QListViewItem::setOpen(o);
}

// ----------------------------------------------------------------------

void DepotListViewItem::addDirectory(QString directoryName)
{
	++m_count;
	DepotListViewItem *item = new DepotListViewItem(m_applicationWindow, false, this, directoryName);
	item->setPixmap(0, *Pixmaps::folderIcon);
}

// ----------------------------------------------------------------------

void DepotListViewItem::addFile(QString directoryName, bool binary)
{
	++m_count;
	DepotListViewItem *item = new DepotListViewItem(m_applicationWindow, true, this, directoryName);
	if (binary)
		item->setPixmap(0, *Pixmaps::binaryFileIcon);
	else
		item->setPixmap(0, *Pixmaps::textFileIcon);
}

// ----------------------------------------------------------------------

const char *DepotListViewItem::getSyncName() const
{
	return m_syncName.latin1();
}

// ======================================================================
