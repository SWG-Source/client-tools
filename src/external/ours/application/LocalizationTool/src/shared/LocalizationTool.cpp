// LocalizationTool.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "LocalizationTool.h"
#include "LocalizationTool.moc"

#include "fileInterface/StdioFile.h"
#include "LocEditingWidget.h"
#include "LocExportWidget.h"
#include "LocImportWidget.h"
#include "LocListWidget.h"
#include "LocalizationData.h"
#include "LocalizedStringTableReaderWriter.h"
#include "MyMultiLineEdit.h"
#include <qapplication.h>
#include <qfiledialog.h>
#include <qfontdialog.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qsplitter.h>
#include <qwidget.h>
#include <qsettings.h>

#if WIN32
#include "Win32Registry.h"
#endif

#include <cassert>

//----------------------------------------------------------------------

//----------------------------------------------------------------------
//-- this is a SWG hack to work with our modified STLPORT

enum MemoryManagerNotALeak
{
	MM_notALeak
};

void * __cdecl operator new(size_t size, MemoryManagerNotALeak)
{
	return operator new(size);
}

//-----------------------------------------------------------------
//-- static constants for registry saving/loading
//-----------------------------------------------------------------

namespace
{
	//-----------------------------------------------------------------

	char const * const cms_version = "LocalizationTool 1.03";

	//-----------------------------------------------------------------

	const char * s_reg_root = "Software\\Sony Online Entertainment\\LocalizationTool";	

	//-----------------------------------------------------------------

	const char * s_reg_mru [2] =
	{
		"RecentFiles.Source",
		"RecentFiles.Translated"
	};	

	//-----------------------------------------------------------------

	const char * s_reg_mru_count [2] =
	{
		"RecentFiles.Source.Count",
		"RecentFiles.Translated.Count"
	};

	//----------------------------------------------------------------------

	const int MAX_MRU = 16;

	//-----------------------------------------------------------------
	const char * s_index_names [2] =
	{
		"Source",
		"Translated"
	};

	//-----------------------------------------------------------------

	const char * s_editing_font_faces [2] =
	{
		"Font.Source.face",
		"Font.Translated.face"
	};

	//-----------------------------------------------------------------

	const char * s_editing_font_sizes [2] =
	{
		"Font.Source.size",
		"Font.Translated.size"
	};
	
	//-----------------------------------------------------------------

	const char * s_list_font_face = "Font.List.face";
	const char * s_list_font_size = "Font.List.size";
	const char * s_window_width   = "Window.width";
	const char * s_window_height  = "Window.height";
	const char * s_window_x       = "Window.x";
	const char * s_window_y       = "Window.y";
}

//-----------------------------------------------------------------

LocalizationTool * LocalizationTool::ms_singleton;

//-----------------------------------------------------------------

LocalizationTool::LocalizationTool (QApplication & app, int argc, char ** argv) :
QMainWindow (0, "main window"),
m_splitter (0),
m_editingWidget (0),
m_listWidget (0),
m_fileMenu (),
m_editMenu (),
m_viewMenu (),
m_newMenu (),
m_openMenu (),
m_closeMenu (),
m_saveMenu (),
m_saveAsMenu (),
m_recentFileMenu (),
m_importMenu (),
m_exportMenu (),
m_gridLayout (),
m_app (app),
m_registry (0)
{

	assert (!ms_singleton);

	ms_singleton = this;

	m_recentMenuDirty [0] = m_recentMenuDirty [1] = true;
	
	m_splitter = new QSplitter (0, "The Splitter");
	
	m_splitter->reparent (this, 0, QPoint (0,0));
	m_splitter->setOrientation (QSplitter::Vertical);
	m_splitter->setOpaqueResize (true);
	
	m_editingWidget = new LocEditingWidget (m_splitter);
	m_listWidget    = new LocListWidget (m_editingWidget, m_splitter);
	
	setCentralWidget (m_splitter);
	
	QObject::connect (m_editingWidget->EditSourceText, SIGNAL (textChanged ()), m_editingWidget, SLOT (onModified ()));
	QObject::connect (m_editingWidget->EditTranslatedText, SIGNAL (textChanged ()), m_editingWidget, SLOT (onModified ()));
	
	menuBar ()->setSeparator (QMenuBar::InWindowsStyle);
	
	connect (&m_fileMenu, SIGNAL (aboutToShow ()),  this, SLOT (doPrepareFileMenu ()));
	
	menuBar ()->insertItem ("&File", &m_fileMenu);
	menuBar ()->insertItem ("&Edit", &m_editMenu);
	menuBar ()->insertItem ("&View", &m_viewMenu);
	
	//-- File menu stuff
	{
		int tmp_id;
		
		tmp_id = m_fileMenu.insertItem  ("&New",  &m_newMenu);
		m_fileMenu.setWhatsThis (tmp_id, "Create a new Localized String file");
		
		{
			tmp_id = m_newMenu.insertItem   ("&Source File",     this, SLOT (doFileNewSource ()));
			m_newMenu.setWhatsThis (tmp_id, "Create a Localized String file used as the source for future translations into other locales.");
			
			tmp_id = m_fileNewTranslatedId = m_newMenu.insertItem   ("&Translated File", this, SLOT (doFileNewTranslated ()));
			m_newMenu.setWhatsThis (tmp_id, "Create a Localized String file to translate the currently loaded source file into a new locale.");
		}
		
		tmp_id = m_fileMenu.insertItem  ("&Open", &m_openMenu);
		m_fileMenu.setWhatsThis (tmp_id, "Load a new Localized String file from disk.");		
		{
			m_openMenu.insertItem  ("&Source File",     this, SLOT (doFileOpenSource ()));
			m_fileOpenTranslatedId = m_openMenu.insertItem  ("&Translated File", this, SLOT (doFileOpenTranslated ()));			
		}
		
		m_fileSaveId         = m_fileMenu.insertItem  ("&Save", &m_saveMenu);
		{
			m_fileSaveSubIds [0] = m_saveMenu.insertItem  ("&Source File",     this, SLOT (doFileSaveSource ()));
			m_fileSaveSubIds [1] = m_saveMenu.insertItem  ("&Translated File", this, SLOT (doFileSaveTranslated ()));
		}
		
		m_fileSaveAsId         = m_fileMenu.insertItem  ("Save &As", &m_saveAsMenu);
		{
			m_fileSaveAsSubIds [0] = m_saveAsMenu.insertItem  ("&Source File",     this, SLOT (doFileSaveAsSource ()));
			m_fileSaveAsSubIds [1] = m_saveAsMenu.insertItem  ("&Translated File", this, SLOT (doFileSaveAsTranslated ()));
		}
		
		m_fileCloseId         = m_fileMenu.insertItem  ("&Close", &m_closeMenu);
		{
			m_fileCloseSubIds [0] = m_closeMenu.insertItem ("&Source File",     this, SLOT (doFileCloseSource ()));
			m_fileCloseSubIds [1] = m_closeMenu.insertItem ("&Translated File", this, SLOT (doFileCloseTranslated ()));
		}
		
		m_fileMenu.insertSeparator ();
		
		m_recentFileId         = m_fileMenu.insertItem ("&Recent Files", &m_recentFileMenu);
		{
			{
				m_recentFileSubIds [0] = m_recentFileMenu.insertItem ("&Source",     &m_recentFileMenus [0]);
				
				connect (&m_recentFileMenu, SIGNAL (aboutToShow ()),  this, SLOT (doPrepareRecentFileMenu ()));
				
				connect (&m_recentFileMenus [0], SIGNAL (activated (int)), this, SLOT (doOpenRecentFileSource (int)));
				connect (&m_recentFileMenus [0], SIGNAL (aboutToShow ()),  this, SLOT (doPrepareRecentFileMenu ()));
			}
			
			{
				m_recentFileMenu.insertSeparator ();
				m_recentFileSubIds [1] = m_recentFileMenu.insertItem ("&Translated", &m_recentFileMenus [1]);
				
				connect (&m_recentFileMenus [1], SIGNAL (activated (int)), this, SLOT (doOpenRecentFileTranslated (int)));
				connect (&m_recentFileMenus [1], SIGNAL (aboutToShow ()),  this, SLOT (doPrepareRecentFileMenu ()));
			}
		}

		//-- The tool needs to be used to edit this data (we now need to save crcs, etc.), so we have disabled Import/Export.
		m_fileMenu.insertSeparator ();
		
		{	
			m_fileImportId = m_fileMenu.insertItem  ("&Import",  &m_importMenu);
			m_fileMenu.setWhatsThis (m_fileImportId, "Import localized information.");
			
			{
				m_importSourceId = m_importMenu.insertItem   ("&Source File",     this, SLOT (doFileImportSourceFile ()));
				m_importMenu.setWhatsThis (m_importSourceId, "Import into the source file.");
			}

#if 0			
			{
				m_importTranslatedId = m_importMenu.insertItem   ("&Translated File",     this, SLOT (doFileImportTranslatedFile ()));
				m_importMenu.setWhatsThis (m_importTranslatedId, "Import into the translated file.");
			}
#endif
		}

		{	
			m_fileExportId = m_fileMenu.insertItem  ("&Export",  &m_exportMenu);
			m_fileMenu.setWhatsThis (m_fileExportId, "Export localized information.");
			
			{
				m_exportSourceId = m_exportMenu.insertItem   ("&Source File",     this, SLOT (doFileExportSourceFile ()));
				m_exportMenu.setWhatsThis (m_exportSourceId, "Export into the source file, completely replacing it.");
			}
				
			{
				m_exportTranslatedId = m_exportMenu.insertItem   ("&Translated File",     this, SLOT (doFileExportTranslatedFile ()));
				m_exportMenu.setWhatsThis (m_exportTranslatedId, "Export into the translated file, completely replacing it.");
			}
		}

		m_fileMenu.insertSeparator ();
		
		m_fileMenu.insertItem  ("E&xit", this, SLOT (doFileExit ()));
	}
	
	
	//-- edit menu stuff
	
	{
		m_sourceFileReadonlyId = m_editMenu.insertItem ("&Source File Read Only", this, SLOT (doSourceFileReadonlyToggle ()));
//		m_editMenu.setItemChecked (m_sourceFileReadonlyId, true);
//		LocalizationData::getData ().setReadOnly (0,       true);
	}
	
	//-- view menu stuff
	
	{
		m_viewMenu.insertItem ("&Fonts",  &m_fontMenu);
		m_fontMenu.insertItem ("&Source Text",     this, SLOT (doViewFontSource ()));
		m_fontMenu.insertItem ("&Translated Text", this, SLOT (doViewFontTranslated ()));
		m_fontMenu.insertItem ("&List Text",       this, SLOT (doViewFontList ()));
	}
	
	//-- force status bar creation
	statusBar ();
	
	//--
	//-- load data from the registry
	//--
	
#if WIN32
	m_registry = new Win32Registry (s_reg_root);
#endif
	
	assert (m_registry);
	
	for (int index = 0; index < 2; ++index)
	{	
		int count = 0;
		if (m_registry->getIntValue (s_reg_mru_count [index], count))
		{
			count = std::min (MAX_MRU, count);

			for (int i = 0; i < count; ++i)
			{
				char buf [64];
				_snprintf (buf, sizeof (buf), "%s.%d", s_reg_mru [index], i);
				
				Unicode::NarrowString result;
				if (m_registry->getStringValue (buf, result))
				{
					m_recentFiles [index].push_back (result);
				}
			}
		}
	}
	
	//-----------------------------------------------------------------
	//-- load the font preferences
	
	Unicode::NarrowString face;
	int size = 0;
	
	if (m_registry->getStringValue (s_editing_font_faces [0], face) &&
		m_registry->getIntValue    (s_editing_font_sizes [0], size))
	{
		m_editingWidget->setSourceFont (QFont (face.c_str (), size));
	}
	
	if (m_registry->getStringValue (s_editing_font_faces [1], face) &&
		m_registry->getIntValue    (s_editing_font_sizes [1], size))
	{
		m_editingWidget->setTranslatedFont (QFont (face.c_str (), size));
	}
	
	if (m_registry->getStringValue (s_list_font_face, face) &&
		m_registry->getIntValue    (s_list_font_size, size))
	{
		m_listWidget->setListFont (QFont (face.c_str (), size));
	}
	
	//-----------------------------------------------------------------
	//-- load the window position/size preferences
	
	// TODO: make this work
	
	/*
	QRect geom = geometry ();
	geom.setWidth (800);
	geom.setHeight (600);
	
	  if (m_registry->getIntValue   (s_window_x, size))
	  geom.setX (size);
	  
		if (m_registry->getIntValue   (s_window_y, size))
		geom.setY (size);
		
		  if (m_registry->getIntValue   (s_window_width, size))
		  geom.setWidth (size);
		  
			if (m_registry->getIntValue   (s_window_height, size))
			geom.setHeight (size);
			
			  if (geom.x () < 0)
			  geom.setX (0);
			  
				if (geom.y () < 0)
				geom.setY (0);
				
				  const QSize dSize (QApplication::desktop ()->size ()); 
				  
					if (geom.right () >= dSize.width ())
					geom.setRight (dSize.width () - 1);
					
					  if (geom.bottom () >= dSize.height ())
					  geom.setBottom (dSize.height () - 1);
					  
						setGeometry (geom);
	*/
	//-----------------------------------------------------------------
	
	doPrepareRecentFileMenu ();
	
	m_editingWidget->hide ();
	m_listWidget->hide ();
	
	LocalizationData::getData ().addListener (this);
	
	if (argc > 1)
	{
		if (doLoadFile (argv [1], 0) && argc > 2)
			doLoadFile (argv[2], 1);
	}

	loadWidget(*this, 0, 0, 800, 600);

	setCaption(cms_version);
}

//-----------------------------------------------------------------

LocalizationTool::~LocalizationTool ()
{
	assert (ms_singleton);
	ms_singleton = 0;

	LocalizationData::getData ().removeListener (this);

	saveWidget(*this);
}

//-----------------------------------------------------------------

void LocalizationTool::doPrepareRecentFileMenu ()
{
	if (m_recentMenuDirty [0])
		doRefreshRecentFileMenu (0);
	
	m_recentFileMenu.setItemEnabled (m_recentFileSubIds [0], m_recentFiles [0].size () != 0);
	
	if (m_recentMenuDirty [1])
		doRefreshRecentFileMenu (1);
	
	m_recentFileMenu.setItemEnabled (m_recentFileSubIds [1], m_fileInfo [0].m_isValid && m_recentFiles [1].size () != 0);
	
}

//-----------------------------------------------------------------

void LocalizationTool::doPrepareFileMenu ()
{
	m_saveMenu.setItemEnabled   (m_fileSaveSubIds   [0], m_fileInfo [0].m_isValid && LocalizationData::getData ().isModified (0));
	m_saveAsMenu.setItemEnabled (m_fileSaveAsSubIds [0], true);
	m_closeMenu.setItemEnabled  (m_fileCloseSubIds  [0], true);
	
	m_newMenu.setItemEnabled    (m_fileNewTranslatedId,  m_fileInfo [0].m_isValid);
	m_openMenu.setItemEnabled   (m_fileOpenTranslatedId, m_fileInfo [0].m_isValid);
	
	m_saveMenu.setItemEnabled   (m_fileSaveSubIds   [1], m_fileInfo [1].m_isValid && LocalizationData::getData ().isModified (1));
	m_saveAsMenu.setItemEnabled (m_fileSaveAsSubIds [1], m_fileInfo [1].m_isValid);
	m_closeMenu.setItemEnabled  (m_fileCloseSubIds  [1], m_fileInfo [1].m_isValid);	
	
	m_fileMenu.setItemEnabled   (m_fileSaveId,   m_fileInfo [0].m_isValid && (m_saveMenu.isItemEnabled   (m_fileSaveSubIds   [0]) || m_saveMenu.isItemEnabled   (m_fileSaveSubIds   [1])));
	m_fileMenu.setItemEnabled   (m_fileSaveAsId, m_fileInfo [0].m_isValid && (m_saveAsMenu.isItemEnabled (m_fileSaveAsSubIds [0]) || m_saveAsMenu.isItemEnabled (m_fileSaveAsSubIds [1])));
	m_fileMenu.setItemEnabled   (m_fileCloseId,  m_fileInfo [0].m_isValid && (m_closeMenu.isItemEnabled  (m_fileCloseSubIds  [0]) || m_closeMenu.isItemEnabled  (m_fileCloseSubIds  [1])));
	
	m_fileMenu.setItemEnabled   (m_recentFileId, m_recentFiles [0].size () != 0 || m_recentFiles [1].size () != 0);

	bool importEnabled [2];

	m_importMenu.setItemEnabled (m_importSourceId,      (importEnabled [0] = !LocalizationData::getData ().getReadOnly (0)));
	m_importMenu.setItemEnabled (m_importTranslatedId,  (importEnabled [1] = m_fileInfo [0].m_isValid && !LocalizationData::getData ().getReadOnly (1)));

	bool exportEnabled [2];

	m_exportMenu.setItemEnabled (m_exportSourceId,      (exportEnabled [0] = m_fileInfo [0].m_isValid));
	m_exportMenu.setItemEnabled (m_exportTranslatedId,  (exportEnabled [1] = m_fileInfo [1].m_isValid));

	m_fileMenu.setItemEnabled (m_fileImportId, importEnabled [0] || importEnabled [1]);
	m_fileMenu.setItemEnabled (m_fileExportId, exportEnabled [0] || exportEnabled [1]);
}

//-----------------------------------------------------------------

void LocalizationTool::doRefreshRecentFileMenu (int index)
{
	int i = 0;
	
	m_recentFileMenus[index].clear ();
	
	for (StringList_t::const_iterator iter = m_recentFiles [index].begin (); iter != m_recentFiles [index].end (); ++iter, ++i)
	{					
		char buf [1024];
		
		_snprintf (buf, sizeof (buf), "&%d  %s", i+1, (*iter).c_str ());
		
		m_recentFileMenus[index].insertItem (buf, i);
	}
	
	m_recentMenuDirty [0] = false;
}

//-----------------------------------------------------------------

void LocalizationTool::doFileNewSource ()
{
	//-- first attempt to close the file
	//-- note- this implicitly causes the translated file (if any) to be closed
	if (m_fileInfo [0].m_isValid && !doFileCloseSource ())
		return;
	
	//-- clear just to be sure... the close routine should handle it for us though.
	m_editingWidget->cancelEditing ();
	LocalizationData::getData ().clear ();
	
	m_fileInfo [0].reset ();
	m_fileInfo [0].m_isValid    = true;
	LocalizationData::getData ().clearModified (0);
	
	m_editingWidget->show ();
	m_listWidget->show ();
}

//-----------------------------------------------------------------

void LocalizationTool::doFileNewTranslated ()
{
	//-- first attempt to close the file
	if (m_fileInfo [1].m_isValid && !doFileCloseTranslated ())
		return;
	
	m_editingWidget->cancelEditing ();
	
	LocalizationData::getData ().setHasTranslatedData (false);
	LocalizationData::getData ().setHasTranslatedData (true);
	
	m_editingWidget->resetValues ();
	
	m_fileInfo [1].reset ();
	m_fileInfo [1].m_isValid    = true;
	LocalizationData::getData ().clearModified (1);
}

//-----------------------------------------------------------------

bool LocalizationTool::doLoadFile (const Unicode::NarrowString & rawName, int index)
{

	std::string fixedFilename (rawName);

	const int len = static_cast<int>(fixedFilename.size ());
	for (int i = 0; i <  len; ++i)
	{
		if (fixedFilename [i] == '\\')
			fixedFilename [i] = '/';
	}

	assert (index == 0 || index == 1);	
	
	StdioFileFactory fileFactory;
	LocalizedStringTableRW * table = LocalizedStringTableRW::loadRW (fileFactory, fixedFilename);
	
	if (table == 0)
	{
		QMessageBox::warning (this, "Error", "An error occured while loading the file.");
		return false;
	}
	

	const int cur_edit_id = LocalizationData::getData ().getCurrentEditId ();
	
	// todo: make this return bool
	m_editingWidget->cancelEditing ();
	
	if (index == 0)
		LocalizationData::getData ().clear ();
	else
		LocalizationData::getData ().setHasTranslatedData (true);
	
	LocalizationData::getData ().clearModified (index);
	
	bool retval = false;

	if (!LocalizationData::getData ().loadFromStringTable (*table, index))
	{
		QMessageBox::warning (this, "Error", "An error occured while loading the data.");
		
		//-- clean up any bogus translated data that may have occured
		if (index == 1)
			LocalizationData::getData ().setHasTranslatedData (false);
	}
	
	else
	{		
		if (index == 1)
		{
			if (cur_edit_id)
			{
				const LocalizedStringPair * sp = LocalizationData::getData ().getEntryById (cur_edit_id);
				
				if (sp)
					m_editingWidget->startEditingString (*sp);
			}
		}
		
		m_fileInfo [index].m_fileName   = fixedFilename;
		m_fileInfo [index].m_isValid    = true;
		
		pushRecentFileName (fixedFilename, index);
		
		//-- force editing widget to reveal it's translation editing widgets
		m_editingWidget->resetValues ();
		
		m_editingWidget->show ();
		m_listWidget->show ();

		dataChanged ();
	}

	bool hadInvalidCrc = false;
	LocalizedStringTableRW::Map_t tableMap = table->getMap();
	LocalizedStringTableRW::Map_t::iterator tableMapIterator;
	for(tableMapIterator = tableMap.begin(); tableMapIterator != tableMap.end(); ++tableMapIterator)
	{
		LocalizedString *stringPtr = (*tableMapIterator).second;

		bool isInvalidCrc = false;

		//-- check source file
		if (!stringPtr->getString().empty() && stringPtr->getCrc() == LocalizedString::nullCrc)
			isInvalidCrc = true;

		//-- check translated file
		if (index == 1 && stringPtr->getSourceCrc() == LocalizedString::nullCrc)
			isInvalidCrc = true;
		
		if (isInvalidCrc)
		{
			hadInvalidCrc = true;
			stringPtr->buildCrc();
			const unsigned long src_id = stringPtr->getId ();			
			const Unicode::NarrowString * const name = table->getNameById (src_id);						
			const LocalizedStringPair sp(*name, *stringPtr, 0);			
			LocalizationData::getData().modifyEntryNoUpdate(sp);			
		}
	}
	if (hadInvalidCrc)
	{
		LocalizationData::getData().notifyListeners();
		QMessageBox::warning (this, "Warning", "Some entries in this file had invalid Crcs.  They have been updated and marked as changed.  Please save this file before exiting.");
	}

	delete table;
	return retval;
}

//-----------------------------------------------------------------

bool LocalizationTool::doFileOpen   (int index)
{
	assert (index == 0 || index == 1);
	
	//-- first attempt to close the file
	if (m_fileInfo [index].m_isValid && !doFileCloseSource ())
		return false;
	
	bool result = false;
	QString initialDirectory;
	registryRead("last_file", initialDirectory, QDir::currentDirPath());
	QString resultFileName(QFileDialog::getOpenFileName(initialDirectory, "String Files (*.stf)", this, "QFileDialog", "Choose a string file to open"));

	// Make sure the file is valid

	QFileInfo readableTest(resultFileName);

	if (readableTest.isReadable())
	{
		// Save the last valid file location

		registryWrite("last_file", readableTest.dirPath());

		const Unicode::NarrowString filename (resultFileName.latin1 ());

		result = doLoadFile (filename, index);
	}

	
	return result;
}

//-----------------------------------------------------------------

void LocalizationTool::doFileOpenSource ()
{
	doFileOpen (0);
}

//-----------------------------------------------------------------

void LocalizationTool::doFileOpenTranslated ()
{
	doFileOpen (1);
}

//-----------------------------------------------------------------

bool LocalizationTool::doFileClose  (int index)
{
	assert (index == 0 || index == 1);
	
	if (!m_fileInfo [index].m_isValid)
		return true;
	
	//-- if modified prompt for save
	if (LocalizationData::getData ().isModified (index))
	{
		//-- loop until the user fixes the situation
		for ( ;; )
		{
			char buf1 [64];
			char buf2 [128];
			
			_snprintf (buf1, sizeof (buf1),  "%s File Modified.", s_index_names [index]);
			_snprintf (buf2, sizeof (buf2), "%s File has been modified.  Save Changes?", s_index_names [index]);
			
			int result = QMessageBox::warning (this, buf1, buf2, "&Save", "&Don't Save", "&Cancel", 0, 2);
			
			// cancel
			if (result == 2)
			{
				return false;
			}
			// save
			else if (result == 0)
			{
				if (!doFileSave (index))
				{
					continue;
				}
			}
			break;
		}
	}
	
	m_fileInfo [index].reset ();
	
	return true;
}

//-----------------------------------------------------------------

bool LocalizationTool::doFileCloseSource ()
{
	//-- if a translated file is open, close it
	if (m_fileInfo [1].m_isValid && !doFileCloseTranslated ())
		return false;
	
	if (doFileClose (0))
	{
		//-- clear the data
		m_editingWidget->cancelEditing ();
		LocalizationData::getData ().clear ();
		
		m_editingWidget->hide ();
		m_listWidget->hide ();
		
		return true;
	}
	
	return false;
}

//-----------------------------------------------------------------

bool LocalizationTool::doFileCloseTranslated ()
{
	if (!m_fileInfo [1].m_isValid)
		return true;
	
	if (doFileClose (1))
	{
		LocalizationData::getData ().setHasTranslatedData (false);
		m_editingWidget->resetValues ();
		return true;
	}
	
	return false;
}

//-----------------------------------------------------------------

bool LocalizationTool::doFileSave   (int index)
{
	assert (m_fileInfo [index].m_isValid);
	
	if (!m_fileInfo [index].m_isValid)
		return false;
	
	//-- this occurs when a 'New File' is first saved
	if (m_fileInfo [index].m_fileName.empty ())
	{
		return doFileSaveAs (index);
	}
	
	//-- todo: whats up with this name?  need locale name
	LocalizedStringTableRW table ("balls");
	
	if (!LocalizationData::getData ().populateStringTable (table, index))
	{
		QMessageBox::warning (this, "Error", "A data error occured.");
		return false;
	}
	
	StdioFileFactory fileFactory;
	if (!table.writeRW (fileFactory, m_fileInfo [index].m_fileName))
	{
		QMessageBox::warning (this, "Error", "An error occured while saving the file.");
		return false;
	}
	
	LocalizationData::getData ().clearModified (index);
	
	return true;
}

//-----------------------------------------------------------------

bool LocalizationTool::doFileSaveSource ()
{
	return doFileSave (0);
}

//-----------------------------------------------------------------

bool LocalizationTool::doFileSaveTranslated ()
{
	return doFileSave (1);
}

//-----------------------------------------------------------------

bool LocalizationTool::doFileSaveAs (int index)
{
	bool result = false;
	QString initialDirectory;
	registryRead("last_file", initialDirectory, QDir::currentDirPath());
	QString resultFileName(QFileDialog::getSaveFileName(initialDirectory, "String Files (*.stf)", this, "QFileDialog", "Choose a string file to save"));

	if (!resultFileName.isEmpty())
	{
		// Save the last valid file location

		registryWrite("last_file", QFileInfo(resultFileName).dirPath());

		std::string filename(resultFileName.latin1());

		//-- need to tack on the .stf extension
		if (filename.length () > 4 && _stricmp (filename.substr (filename.length () - 4).c_str (), ".stf"))
		{
			filename += ".stf";
		}

		//-- test the selected filename for existance
		FILE * fl = fopen (filename.c_str (), "rb");

		bool saveFile = true;

		if (fl != 0)
		{
			fclose (fl);
			
			int result = QMessageBox::warning (this, "Overwrite file?", "The specified file already exists.\nOverwrite?", "Overwrite", "Don't Overwrite", "Cancel", 0, 2);
			
			if (result == 2 ||  //-- cancel
				result == 1)    //-- don't overwrite
			{
				saveFile = false;
			}
		}
		
		if (saveFile)
		{
			m_fileInfo [index].m_fileName = filename;
			pushRecentFileName (filename, index);
			
			result = doFileSave (index);
		}
	}

	return result;
}

//-----------------------------------------------------------------

bool LocalizationTool::doFileSaveAsSource ()
{
	return doFileSaveAs (0);
}

//-----------------------------------------------------------------

bool LocalizationTool::doFileSaveAsTranslated ()
{
	return doFileSaveAs (1);
}

//-----------------------------------------------------------------

void LocalizationTool::doFileExit ()
{
	if (m_fileInfo [1].m_isValid && !doFileCloseTranslated ())
		return;
	
	if (m_fileInfo [0].m_isValid && !doFileCloseSource ())
		return;
	
	doSaveRegistry ();
	m_app.quit ();
}

//-----------------------------------------------------------------

void LocalizationTool::doSaveRegistry ()
{
	//-- save the recent file lists
	
	assert (m_registry);
	
	size_t index;
	
	for (index = 0; index < 2; ++index)
	{
		const int count = std::min (MAX_MRU, static_cast<int>(m_recentFiles [index].size ()));

		if (!m_registry->putIntValue (s_reg_mru_count [index], count))
		{
			// todo: emit some sort of error ? gui interaction is unavailable at this point
			break;
		}
		
		int i = 0;
		
		for (StringList_t::const_iterator iter = m_recentFiles [index].begin (); iter != m_recentFiles [index].end () && i < count; ++iter, ++i)
		{		
			char buf [64];
			_snprintf (buf, sizeof (buf), "%s.%d", s_reg_mru [index], i);
			
			if (!m_registry->putStringValue (buf, *iter))
			{
				// todo: emit some sort of error ? gui interaction is unavailable at this point
				break;
			}
		}
	}
	
	//-----------------------------------------------------------------
	//-- save the font preferences
	
	const QFont eFonts [2] =
	{
		m_editingWidget->getSourceFont (),
			m_editingWidget->getTranslatedFont ()
	};
	
	for (index = 0; index < 2; ++index)
	{
		m_registry->putStringValue (s_editing_font_faces [index], eFonts [index].family ().latin1 ());
		const int ps = eFonts [index].pointSize ();
		m_registry->putIntValue    (s_editing_font_sizes [index], ps);
	}
	
	const QFont lFont (m_listWidget->getListFont ());
	m_registry->putStringValue (s_list_font_face, lFont.family ().latin1 ());
	m_registry->putIntValue    (s_list_font_size, lFont.pointSize ());
	
	//-----------------------------------------------------------------
	//-- save the window position/size preferences
	
	m_registry->putIntValue   (s_window_x, x ());
	m_registry->putIntValue   (s_window_y, y ());
	m_registry->putIntValue   (s_window_width, width ());
	m_registry->putIntValue   (s_window_height, height ());
	
}

//-----------------------------------------------------------------

void LocalizationTool::pushRecentFileName (const Unicode::NarrowString & name, int index)
{
	assert (index == 0 || index == 1);
	
	//-- copied to prevent the case where our string reference references an element of the recent file list
	Unicode::NarrowString nameCopy = name;
	
	StringList_t::iterator iter = std::find (m_recentFiles [index].begin (), m_recentFiles [index].end (), name);
	if (iter != m_recentFiles [index].end ())
		m_recentFiles [index].erase (iter);
	
	m_recentFiles [index].push_front (nameCopy);
	m_recentMenuDirty [index] = true;
}

//-----------------------------------------------------------------

void LocalizationTool::doOpenRecentFileSource (int id)
{
	//-- first attempt to close the file
	if (m_fileInfo [0].m_isValid && !doFileCloseSource ())
		return;
	
	doLoadFile (m_recentFiles [0][id], 0);
}

//-----------------------------------------------------------------

void LocalizationTool::doOpenRecentFileTranslated (int id)
{
	//-- first attempt to close the file
	if (m_fileInfo [1].m_isValid && !doFileCloseTranslated ())
		return;
	
	doLoadFile (m_recentFiles [1][id], 1);
}

//-----------------------------------------------------------------

void LocalizationTool::doSourceFileReadonlyToggle ()
{
	m_editMenu.setItemChecked (m_sourceFileReadonlyId, !m_editMenu.isItemChecked (m_sourceFileReadonlyId));
	LocalizationData::getData ().setReadOnly (0, m_editMenu.isItemChecked (m_sourceFileReadonlyId));
}

//-----------------------------------------------------------------

void LocalizationTool::doViewFontSource ()
{
	const QFont oldFont (m_editingWidget->getSourceFont ());
	
	bool ok;
	const QFont font (QFontDialog::getFont (&ok, oldFont, this, 0));
	
	if (ok)
	{
		m_editingWidget->setSourceFont (font);
	}
}

//-----------------------------------------------------------------

void LocalizationTool::doViewFontTranslated ()
{
	const QFont oldFont (m_editingWidget->getTranslatedFont ());
	
	bool ok;
	const QFont font (QFontDialog::getFont (&ok, oldFont, this, 0));
	
	if (ok)
	{
		m_editingWidget->setTranslatedFont (font);
	}
}

//-----------------------------------------------------------------

void LocalizationTool::doViewFontList ()
{
	const QFont oldFont (m_listWidget->getListFont ());
	
	bool ok;
	const QFont font (QFontDialog::getFont (&ok, oldFont, this, 0));
	
	if (ok)
	{
		m_listWidget->setListFont (font);
	}

}

//-----------------------------------------------------------------

void LocalizationTool::closeEvent (QCloseEvent * closeEvent)
{
	if (!doFileCloseSource ())
		closeEvent->ignore ();
	else
	{
		closeEvent->accept ();
		doSaveRegistry ();
	}
}

//----------------------------------------------------------------------

bool LocalizationTool::doFileImportSourceFile ()
{
	if (!m_fileInfo [0].m_isValid)
		doFileNewSource ();

	LocImportWidget::showDialog (this, 0);
	return true;
}

//----------------------------------------------------------------------

bool LocalizationTool::doFileExportSourceFile ()
{
	if (!m_fileInfo [0].m_isValid)
		return false;

	LocExportWidget::showDialog (this, 0);
	return true;
}

//----------------------------------------------------------------------

bool LocalizationTool::doFileImportTranslatedFile ()
{
	if (!m_fileInfo [1].m_isValid)
		doFileNewTranslated ();

	LocImportWidget::showDialog (this, 1);
	return true;
}

//----------------------------------------------------------------------

bool LocalizationTool::doFileExportTranslatedFile ()
{
	if (!m_fileInfo [1].m_isValid)
		return false;

	LocExportWidget::showDialog (this, 1);
	return true;
}

//----------------------------------------------------------------------

void LocalizationTool::dataChanged ()
{
	char buf [1024];
 
	static bool last_valid [2] =
	{
		false,
		false
	};

	static std::string last_filenames [2] =
	{
		"",
		""
	};

	static bool last_modified [2] =
	{
		false,
		false,
	};

	static bool last_hasTranslated = false;

	std::string filenames [2] = 
	{
		m_fileInfo [0].m_fileName,
		m_fileInfo [1].m_fileName
	};

	const bool modified [2] =
	{
		LocalizationData::getData ().isModified (0),
		LocalizationData::getData ().isModified (1),
	};
	
	const bool hasTranslated = LocalizationData::getData ().getHasTranslatedData ();

	if (filenames [0]            == last_filenames [0] &&
		filenames [1]            == last_filenames [1] &&
		modified  [0]            == last_modified  [0] &&
		modified  [1]            == last_modified  [1] &&
		hasTranslated            == last_hasTranslated &&
		m_fileInfo [0].m_isValid == last_valid [0] &&
		m_fileInfo [1].m_isValid == last_valid [1])
		return;
		
	if (m_fileInfo [0].m_isValid)
	{
		static const int max_filename_truncation_size = 40;
		static const std::string elipsis = "...";
		
		if (filenames [0].size () > max_filename_truncation_size)
		{
			filenames [0] = elipsis + filenames [0].substr (filenames [0].size () - max_filename_truncation_size);
		}
		
		if (m_fileInfo [1].m_isValid && hasTranslated)
		{
			if (filenames [1].size () > max_filename_truncation_size)
				filenames [1] = elipsis + filenames [1].substr (filenames [1].size () - max_filename_truncation_size);
			
			_snprintf (buf, sizeof (buf), "[%s]%s -> [%s]%s", filenames [0].c_str (), modified [0] ? "*" : "", filenames [1].c_str (), modified [1] ? "*" : "");
		}
		else
			_snprintf (buf, sizeof (buf), "[%s]%s", filenames [0].c_str (), modified [0] ? "*" : "");
	}
	else
		strcpy (buf, cms_version);

	setCaption (buf);

	last_hasTranslated = hasTranslated;

	last_filenames [0] = filenames [0];
	last_filenames [1] = filenames [1];

	last_modified [0]  = modified [0];
	last_modified [1]  = modified [1];

	last_valid [0]     = m_fileInfo [0].m_isValid;
	last_valid [1]     = m_fileInfo [1].m_isValid;
}

//-----------------------------------------------------------------

void LocalizationTool::registryWrite(const QString &id, const QString &value) const
{
	QSettings settings;
	settings.insertSearchPath(QSettings::Windows, getRegistryPath());
	settings.writeEntry(id, value);
}

//-----------------------------------------------------------------

void LocalizationTool::registryRead(const QString &id, QString &value, QString const &defaultValue) const
{
	QSettings settings;
	settings.insertSearchPath(QSettings::Windows, getRegistryPath());
	bool result;
	value = settings.readEntry(id, defaultValue, &result);
}

//-----------------------------------------------------------------

const char * const LocalizationTool::getRegistryPath() const
{
	return "SOE/SWG/LocalizationTool";
}

//-----------------------------------------------------------------------------

void LocalizationTool::loadWidget(QWidget &widget, const int defaultX, const int defaultY, const int defaultWidth, const int defaultHeight)
{
	QSettings settings;
	settings.insertSearchPath(QSettings::Windows, getRegistryPath());

	char text[256];
	sprintf(text, "%s_PositionX", widget.name());
	int x = settings.readNumEntry(text, defaultX);
	x = (x > 10000) ? defaultX : x;

	sprintf(text, "%s_PositionY", widget.name());
	int y = settings.readNumEntry(text, defaultY);
	y = (y > 10000) ? defaultY : y;

	sprintf(text, "%s_Width", widget.name());
	int w = settings.readNumEntry(text, defaultWidth);
	w = (w > 10000) ? defaultWidth : w;

	sprintf(text, "%s_Height", widget.name());
	int h = settings.readNumEntry(text, defaultHeight);
	h = (h > 10000) ? defaultHeight : h;

	widget.move(x, y);
	widget.resize(w, h);
	widget.show();
}

//-----------------------------------------------------------------------------

void LocalizationTool::saveWidget(const QWidget &widget)
{
	QSettings settings;
	settings.insertSearchPath(QSettings::Windows, getRegistryPath());

	const QWidget *parent = (widget.parentWidget() == NULL) ? &widget : widget.parentWidget();

	const int x = parent->pos().x();
	const int y = parent->pos().y();
	const int w = widget.width();
	const int h = widget.height();

	char text[256];
	sprintf(text, "%s_PositionX", widget.name());
	settings.writeEntry(text, x);

	sprintf(text, "%s_PositionY", widget.name());
	settings.writeEntry(text, y);

	sprintf(text, "%s_Width", widget.name());
	settings.writeEntry(text, w);

	sprintf(text, "%s_Height", widget.name());
	settings.writeEntry(text, h);
}

//-----------------------------------------------------------------

int main (int argc, char ** argv)
{
	
	LocalizationData::install ();
	
	QApplication app (argc, argv);
	LocalizationTool tool(app, argc, argv);
	
	app.setMainWidget (&tool);
	
	QObject::connect (&app, SIGNAL (aboutToQuit ()), &tool, SLOT (doSaveRegistry ()));

	return app.exec ();

}

//-----------------------------------------------------------------

void *operator new[](size_t size)
{
	return operator new(size);
}

//-----------------------------------------------------------------

void operator delete[](void *pointer)
{
	operator delete(pointer);
}

//-----------------------------------------------------------------
