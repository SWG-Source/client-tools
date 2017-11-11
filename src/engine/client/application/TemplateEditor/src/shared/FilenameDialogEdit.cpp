// ============================================================================
//
// FilenameDialogEdit.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstTemplateEditor.h"
#include "FilenameDialogEdit.h"
#include "FilenameDialogEdit.moc"

#include "sharedFile/TreeFile.h"
#include "TableItemBase.h"
#include "TemplateEditorUtility.h"
#include "WorkingDirectories.h"

// ============================================================================
//
// FilenameDialogEdit
//
// ============================================================================

//-----------------------------------------------------------------------------
FilenameDialogEdit::FilenameDialogEdit(QWidget *parentWidget, const char *name, TableItemBase &parentTableItemBase)
 : BaseDialogEdit(parentWidget, name, parentTableItemBase)
 , m_filter()
 , m_fileInfo()
{
	m_filter = "All file types (*.*)";

	// Allow editing of the filename directly

	setTextLineEditReadOnly(false);

	//m_fileInfo = QFileInfo(WorkingDirectories::getCurrentDirectory(), QString::null);

	// Instead of letting the base class do the dialog managing, I'm going to steal the focus from them
	// and do it myself. That way, I get to have more control over the QFileDialog because I want to
	// take advantage of some of its static member features. As such, in this class I never call setInputDialog(...), 
	// which all other BaseDialogEdits do. - Dherman
	// See openFile()

	connect(getOpenDialogButton(), SIGNAL(clicked()), SLOT(openFile()));
}

//-----------------------------------------------------------------------------
void FilenameDialogEdit::setPath(QString const &path)
{
	m_fileInfo.setFile(path);

	setText(path);
}

//-----------------------------------------------------------------------------
QString FilenameDialogEdit::getPath()
{
	return m_fileInfo.filePath();
}

//-----------------------------------------------------------------------------
void FilenameDialogEdit::setFilter(QString const &filter)
{
	m_filter = filter;
}

//-----------------------------------------------------------------------------
void FilenameDialogEdit::openFile()
{
	// Get the parameter name
	
	QString parameterName(getTableItemBase().getTemplateTableRow().getParameterName());
	
	// Find the last used path
	
	QSettings settings;
	settings.insertSearchPath(QSettings::Windows, TemplateEditorUtility::getSearchPath());
	QString previousUsedPath(settings.readEntry(parameterName, QDir::root().absPath()));

	QString path(QFileDialog::getOpenFileName(previousUsedPath, m_filter, this, 0, "Open a file"));

	if(!path.isNull())
	{
		// Save the last used path

		settings.writeEntry(parameterName, path);

		// Find the tree file relative path

		std::string resultFileName;

		bool result = TreeFile::stripTreeFileSearchPathFromFile(path.latin1(), resultFileName);

		if (result)
		{
			setPath(resultFileName.c_str());
		}
		else
		{
			setPath(path);
		}
	}
}

//-----------------------------------------------------------------------------
QString FilenameDialogEdit::getTextFromDialog() const
{
	QString result;

	// Current value

	result.sprintf("\"%s\"", getLineEdit()->text().latin1());

	// Weighted percent

	if (isWeighted())
	{
		result += ":" + QString::number(getWeight());
	}

	return result;
}

// ============================================================================
