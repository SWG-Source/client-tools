// ======================================================================
//
// LocalizationTool.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_LocalizationTool_H
#define INCLUDED_LocalizationTool_H

#include <qmainwindow.h>
#include <qsplitter.h>
#include "LocEditingWidget.h"
#include "LocListWidget.h"
#include "QPopupMenu.h"
#include <qlayout.h>
#include "DataChangeListener.h"

#include <deque>

class QApplication;
class PortableRegistry;

// ======================================================================
class LocalizationTool : public QMainWindow, public DataChangeListener
{
private:
	Q_OBJECT

public:

	enum FileId
	{
		SOURCE     = 0,
		TRANSLATED = 1
	};

	LocalizationTool (QApplication & app, int argc, char ** argv);
	~LocalizationTool ();

public slots:

	void doFileNewSource ();
	void doFileNewTranslated ();
	void doFileOpenSource ();
	void doFileOpenTranslated ();
	bool doFileCloseSource ();
	bool doFileCloseTranslated ();
	bool doFileSaveSource ();
	bool doFileSaveTranslated ();
	bool doFileSaveAsSource ();
	bool doFileSaveAsTranslated ();

	bool doFileImportSourceFile ();
	bool doFileExportSourceFile ();
	bool doFileImportTranslatedFile ();
	bool doFileExportTranslatedFile ();

	void doFileExit ();

	void doSaveRegistry ();

	void doOpenRecentFileSource (int id);
	void doOpenRecentFileTranslated (int id);
	
	void doPrepareRecentFileMenu ();
	void doPrepareFileMenu ();

	void doSourceFileReadonlyToggle ();

	void doViewFontSource ();
	void doViewFontTranslated ();
	void doViewFontList ();


	PortableRegistry * getRegistry ();

	static LocalizationTool * getInstance ();

	//----------------------------------------------------------------------

	virtual void dataChanged ();
	virtual void currentEditingChanged () {};
	virtual void readOnlyChanged () {};

protected:
	void closeEvent (QCloseEvent *);

private:

	                      LocalizationTool (const LocalizationTool & rhs);
	LocalizationTool &    operator=    (const LocalizationTool & rhs);

	void                  pushRecentFileName (const Unicode::NarrowString & name, int index);
	void                  doRefreshRecentFileMenu (int index);
	bool                  doLoadFile (const Unicode::NarrowString & name, int which);

	bool doFileOpen   (int index);
	bool doFileClose  (int index);
	bool doFileSave   (int index);
	bool doFileSaveAs (int index);

	void               registryWrite(const QString &id, const QString &value) const;
	void               registryRead(const QString &id, QString &value, QString const &defaultValue) const;
	const char * const getRegistryPath() const;
	void               loadWidget(QWidget &widget, const int defaultX, const int defaultY, const int defaultWidth, const int defaultHeight);
	void               saveWidget(const QWidget &widget);

private:

	struct FileInfo
	{
		Unicode::NarrowString m_fileName;
		Unicode::NarrowString m_locale;
		bool m_isValid;

		FileInfo() : m_fileName (), m_locale (), m_isValid (false) {}

		void  reset ()
		{
			m_fileName.erase ();
			m_locale.erase ();
			m_isValid    = false;
		}
	};

	QSplitter *           m_splitter;
	LocEditingWidget *    m_editingWidget;
	LocListWidget *       m_listWidget;

	QPopupMenu            m_fileMenu;
	QPopupMenu            m_editMenu;
	QPopupMenu            m_viewMenu;

	QPopupMenu            m_newMenu;
	QPopupMenu            m_openMenu;
	QPopupMenu            m_closeMenu;
	QPopupMenu            m_saveMenu;
	QPopupMenu            m_saveAsMenu;
	QPopupMenu            m_recentFileMenu;
	QPopupMenu            m_recentFileMenus [2];
	QPopupMenu            m_importMenu;
	QPopupMenu            m_exportMenu;

	QPopupMenu            m_fontMenu;

	int                   m_sourceFileReadonlyId;

	int                   m_fileNewTranslatedId;
	int                   m_fileOpenTranslatedId;

	int                   m_fileSaveId;
	int                   m_fileSaveSubIds [2];
	
	int                   m_fileSaveAsId;
	int                   m_fileSaveAsSubIds[2];

	int                   m_fileCloseId;
	int                   m_fileCloseSubIds [2];

	int                   m_recentFileId;
	int                   m_recentFileSubIds [2];

	int                   m_fileImportId;
	int                   m_importSourceId;
	int                   m_importTranslatedId;

	int                   m_fileExportId;
	int                   m_exportSourceId;
	int                   m_exportTranslatedId;

	QGridLayout *         m_gridLayout;

	FileInfo              m_fileInfo [2];

	QApplication &        m_app;

	typedef std::deque<Unicode::NarrowString> StringList_t;

	StringList_t          m_recentFiles [2];

	PortableRegistry *    m_registry;

	bool                  m_recentMenuDirty [2]; 

	static LocalizationTool * ms_singleton;
};

//-----------------------------------------------------------------
inline LocalizationTool * LocalizationTool::getInstance ()
{
	return ms_singleton;
}

//-----------------------------------------------------------------

inline PortableRegistry * LocalizationTool::getRegistry ()
{
	return m_registry;
}

// ======================================================================

#endif
