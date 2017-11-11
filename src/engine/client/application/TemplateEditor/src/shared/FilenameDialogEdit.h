// ============================================================================
//
// FilenameDialogEdit.h
//
// Dialog that lets user select a filename from a file dialog.
//
// Note that Qt file dialogs work in a strange way -- if you create one
// using 'new QFileDialog', then you will get a generic Qt file dialog. However,
// if you use Qt's static FileDialog function, it will open up the file browser
// that is native for whatever platform which it is running on.
//
// Most BaseDialogEdits are expected to create an instance of their dialog of
// choice and pass it on to their parent, but in this case, I had to break that
// trend so that I could take advantage of QFileDialog's strange nature. - Dherman
//
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_FilenameDialogEdit_H
#define INCLUDED_FilenameDialogEdit_H

#include "BaseDialogEdit.h"

class TableItemBase;

//-----------------------------------------------------------------------------
class FilenameDialogEdit : public BaseDialogEdit
{
	Q_OBJECT

public:

	FilenameDialogEdit(QWidget *parentWidget, const char *name, TableItemBase &parentTableItemBase);

	QString getPath();
	void    setPath(QString const &path);
	void    setFilter(QString const &filter);

private slots:

	void openFile();

protected:

	virtual QString getTextFromDialog() const;

private:

	QString   m_filter;
	QFileInfo m_fileInfo;

private:

	// Disabled

	FilenameDialogEdit &operator =(FilenameDialogEdit const &);
};

// ============================================================================

#endif // INCLUDED_FilenameDialogEdit_H
