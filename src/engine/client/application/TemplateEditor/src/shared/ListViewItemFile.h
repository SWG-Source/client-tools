// ============================================================================
//
// ListViewItemFile.h
//
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_ListViewItemFile_H
#define INCLUDED_ListViewItemFile_H

//-----------------------------------------------------------------------------
class ListViewItemFile : public QListViewItem
{
public:

	ListViewItemFile(QListView *parent);
	virtual ~ListViewItemFile();

	QString const &getPath();
	void           setPath(QString const &path);

private:

	QString m_path;
};

// ============================================================================

#endif // INCLUDED_ListViewItemFile_H