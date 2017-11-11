// ============================================================================
//
// ListViewItemFile.cpp
//
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstTemplateEditor.h"
#include "ListViewItemFile.h"

//-----------------------------------------------------------------------------
ListViewItemFile::ListViewItemFile(QListView *parent)
 : QListViewItem(parent)
{
}

//-----------------------------------------------------------------------------
ListViewItemFile::~ListViewItemFile()
{
}

//-----------------------------------------------------------------------------
QString const &ListViewItemFile::getPath()
{
	return m_path;
}

//-----------------------------------------------------------------------------
void ListViewItemFile::setPath(QString const &path)
{
	m_path = path;

	QFileInfo fileInfo(m_path);
	setText(1, fileInfo.fileName());
}

// ============================================================================
