// ======================================================================
//
// DepotListViewItem.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_DepotListViewItem_H
#define INCLUDED_DepotListViewItem_H

// ======================================================================

class ApplicationWindow;
#include <qlistview.h>

// ======================================================================

class DepotListViewItem : public QListViewItem
{
public:

	DepotListViewItem(ApplicationWindow &applicationWindow, bool file, QListView *parent, QString name);
	DepotListViewItem(ApplicationWindow &applicationWindow, bool file, QListViewItem *parent, QString name);
	virtual void setOpen(bool o);

	const char *getSyncName() const;
	void addDirectory(QString directoryName);
	void addFile(QString directoryName, bool binary);

public slots:

	void sync();
	void edit();

private:

	DepotListViewItem();
	DepotListViewItem(const DepotListViewItem &);
	DepotListViewItem &operator =(const DepotListViewItem &);

private:

	ApplicationWindow &m_applicationWindow;
	QString            m_fullName;
	QString            m_syncName;
	bool               m_file;
	bool               m_cached;
	int                m_count;
};

// ======================================================================

#endif
