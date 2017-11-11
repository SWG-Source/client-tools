// =====================================================================
//
// ActionsScript.cpp
// copyright(c) 2001 Sony Online Entertainment
//
// =====================================================================

#include "SwgGodClient/FirstSwgGodClient.h"
#include "ActionsScript.h"
#include "ActionsScript.moc"

#include "clientGame/Game.h"
#include "clientGame/GroundScene.h"
#include "fileInterface/StdioFile.h"

#include "ActionHack.h"
#include "ConfigGodClient.h"
#include "GodClientData.h"
#include "GodClientPerforce.h"
#include "IconLoader.h"
#include "MainFrame.h"
#include "ServerCommander.h"
#include "Unicode.h"

#include <qmessagebox.h>
#include <process.h>
#include <qfiledialog.h>

// =====================================================================

const char * const ActionsScript::DragMessages::SCRIPT_DRAGGED = "ScriptListView::DragMessages::SCRIPT_DRAGGED";

//----------------------------------------------------------------------
ActionsScript::ActionsScript() :
	QObject(),
	Singleton<ActionsScript>(),
	create(0),
	view(0),
	edit(0),
	revert(0),
	refresh(0),
	submit(0),
	compile(0),
	serverReload(0),
	removeScript(0),
	removeObjvar(0),
	setObjvar(0),
	m_selectedPath(),
	m_isFile(false),
	m_isNew(false),
	m_isEdit(false)
{
	QWidget * const p = &MainFrame::getInstance();

	create    = new ActionHack("New Script",                  IL_PIXMAP(hi16_mime_document2),  "&New",     0, p, "script_new");
	IGNORE_RETURN(connect(create, SIGNAL(activated()), this, SLOT(onNew())));

	view      = new ActionHack("View Script in Editor",       IL_PIXMAP(hi16_mime_document),   "&View",    0, p, "script_view");
	IGNORE_RETURN(connect(view, SIGNAL(activated()), this, SLOT(onView())));

	edit      = new ActionHack("Edit Script from Perforce",   IL_PIXMAP(hi16_action_edit),     "&Edit",    0, p, "script_edit");
	IGNORE_RETURN(connect(edit, SIGNAL(activated()), this, SLOT(onEdit())));

	revert    = new ActionHack("Revert Script from Perforce", IL_PIXMAP(hi16_action_revert),   "Re&vert",  0, p, "script_revert");
	IGNORE_RETURN(connect(revert, SIGNAL(activated()), this, SLOT(onRevert())));

	submit    = new ActionHack("Submit Script to Perforce",   IL_PIXMAP(hi16_action_finish),   "&Submit",  0, p, "script_submit");
	IGNORE_RETURN(connect(submit, SIGNAL(activated()), this, SLOT(onSubmit())));

	compile   = new ActionHack("Compile Script",              IL_PIXMAP(hi16_action_gear),     "&Compile", 0, p, "script_compile");
	IGNORE_RETURN(connect(compile, SIGNAL(activated()), this, SLOT(onCompile())));

	refresh   = new ActionHack("Refresh Script List",         IL_PIXMAP(hi16_action_reload),   "&Refresh", 0, p, "script_refresh");

	serverReload  = new ActionHack("Reload Script on Server", IL_PIXMAP(hi16_filesys_network), "&Server Reload",  0, p, "script_server_reload");
	IGNORE_RETURN(connect(serverReload, SIGNAL(activated()), this, SLOT(onServerReload())));

	removeScript = new ActionHack("Detach this script from the currently selected object.", IL_PIXMAP(hi16_action_editdelete), "Detach", 0, p, "detach_script");
	removeObjvar = new ActionHack("Remove this objvar from the currently selected object.", IL_PIXMAP(hi16_action_editdelete), "Remove Objvar", 0, p, "remove_objvar");
	setObjvar = new ActionHack("Set an objvar on the currently selected object.", IL_PIXMAP(hi16_action_editdelete), "Set Objvar", 0, p, "set_objvar");

	view->setEnabled        (false);
	edit->setEnabled        (false);
	revert->setEnabled      (false);
	serverReload->setEnabled(false);
	submit->setEnabled      (false);
	compile->setEnabled     (false);
}

//----------------------------------------------------------------------

ActionsScript::~ActionsScript()
{
	create = 0;
	view = 0;
	edit = 0;
	revert = 0;
	refresh = 0;
	submit = 0;
	compile = 0;
	serverReload = 0;
	removeScript = 0;
	removeObjvar = 0;
	setObjvar = 0;
}

//----------------------------------------------------------------------

/**
 * Slot to handle updating the global script or script path selection.
 */
void ActionsScript::onScriptPathSelectionChanged(const std::string & path, bool isFile, bool isNew, bool isEdit)
{
	m_selectedPath = path;
	m_isFile = isFile;
	m_isNew  = isNew;
	m_isEdit = isEdit;

	view->setEnabled        (!m_selectedPath.empty() && isFile);
	edit->setEnabled        (view->isEnabled());
	serverReload->setEnabled(view->isEnabled());

	revert->setEnabled      (view->isEnabled() &&(isEdit || isNew));
	submit->setEnabled      (view->isEnabled() &&(isEdit || isNew));
	compile->setEnabled     (false);//revert->isEnabled());
}

//----------------------------------------------------------------------

/**
 * Open an arbitrary file in the user's editor.  Editor is set in [GocClient] editor
 * 
 */
void ActionsScript::doEditFile(const std::string & filename) const
{
	AbstractFile * fl = new StdioFile(filename.c_str(), "r");

	if(!fl->isOpen())
	{
		delete fl;
		fl = new StdioFile(filename.c_str(), "w");

		if(!fl->isOpen())
		{
			const std::string err = std::string("Can't create new file: [") + filename + "].";
			IGNORE_RETURN(QMessageBox::warning(&MainFrame::getInstance(), "Warning", err.c_str()));
			delete fl;
			return;
		}

		delete fl;
	}

	const char * editor = NON_NULL(ConfigGodClient::getData().scriptEditor);

	std::string arg0 = editor;

	const size_t slashpos = arg0.find_last_of("\\/");
	if(slashpos != arg0.npos && slashpos <(arg0.size() - 1)) //lint !e737 implicit promotion, bug in STL with size_t and std::string::npos being of different signage
		arg0 = arg0.substr(slashpos + 1);

	const int retval = _spawnlp(_P_NOWAIT, editor, arg0.c_str(), filename.c_str(), 0);

	if(retval == -1)
	{
		const std::string err = std::string("Unable to execute scriptEditor [") + editor + "]: " + strerror(errno);
		IGNORE_RETURN(QMessageBox::warning(&MainFrame::getInstance(), "Warning", err.c_str()));
		return;
	}
}

//----------------------------------------------------------------------

/**
 * Revert the selected script.  Calls p4 revert on both the class and script file
 */
void ActionsScript::onRevert()
{
	if(m_selectedPath.empty() || !m_isFile || !(m_isEdit || m_isNew))
		return;
	
	{
		const std::string classPath = ActionsScript::convertToClasspath(m_selectedPath);
		const std::string msg = std::string("Really revert script: [") + classPath + "]?";
		const int retval = QMessageBox::warning(&MainFrame::getInstance(), "Warning", msg.c_str(), QMessageBox::Yes, QMessageBox::No);
		
		if(retval == QMessageBox::No)
			return;
	}
	
	GodClientPerforce::StringVector sv;
	sv.reserve(2);
	sv.push_back(getScriptFilePath());
	sv.push_back(getClassFilePath());

	std::string result;
	
	if(!GodClientPerforce::getInstance().revertFiles(sv, false, result))
	{
		const std::string msg = "Could not revert class files: " + sv [0] + ", " + sv [1] + "\n" + result;
		IGNORE_RETURN(QMessageBox::warning(&MainFrame::getInstance(), "Warning", msg.c_str()));
	}

	refresh->doActivate();
}

//----------------------------------------------------------------------

/**
 * Opens the editor on the selected .script file
 *
 */
void ActionsScript::onView() const
{
	if(m_selectedPath.empty() || !m_isFile)
		return;

	std::string dummy;
	std::string local;
	
	const std::string path = getScriptFilePath();

	std::string result;

	//-- map the desired script classpath to a local source directory
	if(!GodClientPerforce::getInstance().getFileMapping(path, dummy, dummy, local, result))
	{
		const std::string msg = "Can't 'where' directory.  User error?  Perhaps.\n" + result;
		IGNORE_RETURN(QMessageBox::warning(&MainFrame::getInstance(), "Warning", msg.c_str()));
		return;
	}

	doEditFile(local);
}

//----------------------------------------------------------------------

/**
 * p4 edits the selected .script and .class file, and opens the editor on the .script file
 *
 */
void ActionsScript::onEdit()
{
	if(m_selectedPath.empty() || !m_isFile)
		return;

	std::string dummy;
	std::string local;
			
	GodClientPerforce::StringVector sv;
	sv.reserve(2);
	sv.push_back(getScriptFilePath());
	sv.push_back(getClassFilePath());

	std::string result;
	
	if(!GodClientPerforce::getInstance().editFiles(sv, result))
	{
		const std::string msg = "Could not open file(s) for perforce edit.  View Will be read-only.\n" + result;
		IGNORE_RETURN(QMessageBox::warning(&MainFrame::getInstance(), "Warning", msg.c_str()));
	}

	//-- map the desired script classpath to a local source directory
	if(!GodClientPerforce::getInstance().getFileMapping(sv [0], dummy, dummy, local, result))
	{
		const std::string msg = "Can't 'where' directory.  User error?  Perhaps.\n" + result;
		IGNORE_RETURN(QMessageBox::warning(&MainFrame::getInstance(), "Warning", msg.c_str()));
		return;
	}

	refresh->doActivate();
	doEditFile(local);
}

//----------------------------------------------------------------------

/**
 * Prompts the user to select a new .script file.  Adds the resulting .script and .class files to perforce.
 * Creates the .script file and opens the editor on it.
 *
 */
void ActionsScript::onNew()
{
	std::string actualPath = m_selectedPath;

	//-- trim the filename from the path

	if(!actualPath.empty() && m_isFile)
	{
		const size_t slashpos = actualPath.rfind('/');

		if(slashpos == actualPath.npos) //lint !e737 implicit promotion, bug in STL with size_t and std::string::npos being of different signage
			actualPath.clear();
		else
			actualPath = actualPath.substr(0, slashpos);
	}

	
	const char * const srcPath = NON_NULL(ConfigGodClient::getData().scriptSourcePath);
	std::string path = GodClientPerforce::concatenateSubpath(srcPath, actualPath);

	std::string dummy;
	std::string local;

	std::string result;
	
	//-- map the desired script classpath to a local source directory
	if(!GodClientPerforce::getInstance().getFileMapping(path, dummy, dummy, local, result))
	{
		const std::string msg = "Can't 'where' directory.  User error?  Perhaps.\n" + result;
		IGNORE_RETURN(QMessageBox::warning(&MainFrame::getInstance(), "Warning", msg.c_str()));
		return;
	}

	const QString qselection = QFileDialog::getSaveFileName(local.c_str(), "*.script", &MainFrame::getInstance());
	
	if(qselection == QString::null)
		return;

	std::string selection = qselection.latin1();
	if(selection.size() < 7 || selection.substr(selection.size() - 7) != ".script")
		selection += ".script";

	UNREF(selection);

	//-- discover the depot path to the .class file
	size_t dsrc_pos = selection.find("\\dsrc\\");
	if(dsrc_pos == selection.npos) //lint !e737 implicit promotion, bug in STL with size_t and std::string::npos being of different signage
		dsrc_pos = selection.find("/dsrc/");

	if(dsrc_pos == selection.npos) //lint !e737 implicit promotion, bug in STL with size_t and std::string::npos being of different signage
	{
		IGNORE_RETURN(QMessageBox::warning(&MainFrame::getInstance(), "Warning", "Can't translate script path to class path."));
		return;
	}

	std::string compiledLocalPath = selection;
	IGNORE_RETURN(compiledLocalPath.replace(dsrc_pos + 1, 4, "data"));
	IGNORE_RETURN(compiledLocalPath.replace(compiledLocalPath.size() - 6, 6, "class"));

	GodClientPerforce::StringVector sv;
	sv.reserve(2);
	sv.push_back(selection);
	sv.push_back(compiledLocalPath);

	if(!GodClientPerforce::getInstance().addFiles(sv, result))
	{
		const std::string msg = "Unable to add new files to perforce." + result;
		IGNORE_RETURN(QMessageBox::warning(&MainFrame::getInstance(), "Warning", msg.c_str()));
		return;
	}

	refresh->doActivate();
	doEditFile(selection);
}

//----------------------------------------------------------------------

/**
 * Requests the server to relaod the script.
 *
 */
void ActionsScript::onServerReload() const
{
	if(m_selectedPath.empty() || !m_isFile)
		return;

	const std::string classPath = ActionsScript::convertToClasspath(m_selectedPath);
	const std::string msg = std::string("Really server-reload script: [") + classPath + "]?";
	const int retval = QMessageBox::warning(&MainFrame::getInstance(), "Warning", msg.c_str(), QMessageBox::Yes, QMessageBox::No);

	if(retval == QMessageBox::No)
		return;

	IGNORE_RETURN(ServerCommander::getInstance().scriptReload(classPath));
}

//----------------------------------------------------------------------

/**
 * Submit the selected script to perforce, reverting unchanged first if possible.
 *
 */
void ActionsScript::onSubmit()
{
	if(m_selectedPath.empty() || !m_isFile || !(m_isEdit || m_isNew))
		return;

	const std::string classPath = ActionsScript::convertToClasspath(m_selectedPath);
	const std::string msg = std::string("Really submit script: [") + classPath + "]?";
	const int retval = QMessageBox::warning(&MainFrame::getInstance(), "Warning", msg.c_str(), QMessageBox::Yes, QMessageBox::No);

	if(retval == QMessageBox::No)
		return;

	GodClientPerforce::StringVector sv;
	sv.reserve(2);
	sv.push_back(getScriptFilePath());
	sv.push_back(getClassFilePath());

	std::string result;

	IGNORE_RETURN(GodClientPerforce::getInstance().revertFiles(sv, true, result));

	if(!GodClientPerforce::getInstance().submitFiles(sv, result))
	{
		const std::string msg = "Could not submit file(s) to perforce!  Beanhead.\n" + result;
		IGNORE_RETURN(QMessageBox::warning(&MainFrame::getInstance(), "Warning", msg.c_str()));
	}

	refresh->doActivate();
}

//----------------------------------------------------------------------

/**
 * Compile the selected script..
 * @todo implement script compile process and popup a text output window for compiler output
 */
void ActionsScript::onCompile() const
{
	if(m_selectedPath.empty() || !m_isFile)
		return;

	//-- python exe/win32/script_prep.pyc ${scriptname}
}

//----------------------------------------------------------------------

/**
 * Simply convert all / to .
 * Does not strip .class or .script suffix
 */
const std::string ActionsScript::convertToClasspath(const std::string & path)
{
	//-- convert the filesystem path to java package-class notation
	std::string classPath = path;
	size_t slashPos = classPath.find('/', 0);

	while(slashPos != classPath.npos) //lint !e737 implicit promotion, bug in STL with size_t and std::string::npos being of different signage
	{
		classPath [slashPos] = '.';
		slashPos = classPath.find('/', slashPos + 1);
	}

	return classPath;
}

//----------------------------------------------------------------------

const std::string ActionsScript::getScriptFilePath() const
{
	if(m_selectedPath.empty() || !m_isFile)
		return "";
	
	const char * const srcPath = NON_NULL(ConfigGodClient::getData().scriptSourcePath);
	return GodClientPerforce::concatenateSubpath(srcPath, m_selectedPath) + ".script"; 
}

//----------------------------------------------------------------------

const std::string ActionsScript::getClassFilePath() const
{
	if(m_selectedPath.empty() || !m_isFile)
		return "";
	
	const char * const classPath = NON_NULL(ConfigGodClient::getData().scriptClassPath);
	return GodClientPerforce::concatenateSubpath(classPath, m_selectedPath) + ".class";
}

//----------------------------------------------------------------------

bool ActionsScript::getSelectedScript(std::string & path) const
{
	if(m_selectedPath.empty() || !m_isFile)
		return false;

	path = convertToClasspath(m_selectedPath);
	return true;
}

//----------------------------------------------------------------------
