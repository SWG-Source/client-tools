// ======================================================================
//
// ActionsObjectTemplate.cpp
// copyright(c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgGodClient/FirstSwgGodClient.h"
#include "ActionsObjectTemplate.h"
#include "ActionsObjectTemplate.moc"

#include "clientGame/Game.h"
#include "clientGame/GroundScene.h"
#include "fileInterface/StdioFile.h"

#include "ActionHack.h"
#include "ConfigGodClient.h"
#include "FavoritesWindow.h"
#include "GodClientData.h"
#include "GodClientPerforce.h"
#include "IconLoader.h"
#include "MainFrame.h"
#include "ServerCommander.h"
#include "Unicode.h"

#include <qmessagebox.h>
#include <process.h>
#include <qfiledialog.h>

// ======================================================================

const char * const ActionsObjectTemplate::DragMessages::ObjectTemplate_SERVER_TEMPLATE_DRAGGED = "ObjectTemplateListView::DragMessages::ObjectTemplate_SERVER_TEMPLATE_DRAGGED";

const char* const ActionsObjectTemplate::ms_objectTemplateSourceExtension = ".tpf";
const char* const ActionsObjectTemplate::ms_objectTemplateCompiledExtension = ".iff";

//-----------------------------------------------------------------------

ActionsObjectTemplate::ActionsObjectTemplate()
: QObject(),
Singleton<ActionsObjectTemplate>(),
m_serverCreate(0),
m_serverView(0),
m_serverEdit(0),
m_serverRevert(0),
m_serverRefresh(0),
m_serverSubmit(0),
m_serverCompile(0),
m_serverReload(0),
m_serverReplace(0),
m_clientCreate(0),
m_clientView(0),
m_clientEdit(0),
m_clientRevert(0),
m_clientRefresh(0),
m_clientSubmit(0),
m_clientCompile(0),
m_clientReplace(0),
m_serverAddToFavorites(0),
m_clientAddToFavorites(0),
m_selectedServerPath(),
m_selectedClientPath(),
m_isServerFile(false),
m_isServerNew(false),
m_isServerEdit(false),
m_isClientFile(false),
m_isClientNew(false),
m_isClientEdit(false)
{
	QWidget * const p =&MainFrame::getInstance();

	m_serverCreate    = new ActionHack("New ServerObjectTemplate",                  IL_PIXMAP(hi16_mime_document2),  "&New",     0, p, "ServerObjectTemplate_new");
	IGNORE_RETURN(connect(m_serverCreate, SIGNAL(activated()), this, SLOT(onServerNew())));

	m_serverView      = new ActionHack("View ServerObjectTemplate in Editor",       IL_PIXMAP(hi16_mime_document),   "&View",    0, p, "ServerObjectTemplate_view");
	IGNORE_RETURN(connect(m_serverView, SIGNAL(activated()), this, SLOT(onServerView())));

	m_serverEdit      = new ActionHack("Edit ServerObjectTemplate from Perforce",   IL_PIXMAP(hi16_action_edit),     "&Edit",    0, p, "ServerObjectTemplate_edit");
	IGNORE_RETURN(connect(m_serverEdit, SIGNAL(activated()), this, SLOT(onServerEdit())));

	m_serverRevert    = new ActionHack("Revert ServerObjectTemplate from Perforce", IL_PIXMAP(hi16_action_revert),   "Re&vert",  0, p, "ServerObjectTemplate_revert");
	IGNORE_RETURN(connect(m_serverRevert, SIGNAL(activated()), this, SLOT(onServerRevert())));

	m_serverSubmit    = new ActionHack("Submit ServerObjectTemplate to Perforce",   IL_PIXMAP(hi16_action_finish),   "&Submit",  0, p, "ServerObjectTemplate_submit");
	IGNORE_RETURN(connect(m_serverSubmit, SIGNAL(activated()), this, SLOT(onServerSubmit())));

	m_serverCompile   = new ActionHack("Compile ServerObjectTemplate",              IL_PIXMAP(hi16_action_gear),     "&Compile", 0, p, "ServerObjectTemplate_compile");
	IGNORE_RETURN(connect(m_serverCompile, SIGNAL(activated()), this, SLOT(onServerCompile())));

	m_serverRefresh   = new ActionHack("Refresh ServerObjectTemplate List",         IL_PIXMAP(hi16_action_reload),   "&Refresh", 0, p, "ServerObjectTemplate_refresh");

	m_serverReload  = new ActionHack("Reload ServerObjectTemplate on Server", IL_PIXMAP(hi16_filesys_network), "&Server Reload",  0, p, "ServerObjectTemplate_server_reload");
	IGNORE_RETURN(connect(m_serverReload, SIGNAL(activated()), this, SLOT(onServerReload())));

	m_serverReplace = new ActionHack("Replace selection with ServerObjectTemplate", IL_PIXMAP(hi16_filesys_network), "&Server Replace",  0, p, "ServerObjectTemplate_server_replace");
	IGNORE_RETURN(connect(m_serverReplace, SIGNAL(activated()), this, SLOT(onServerReplace())));

	m_clientCreate    = new ActionHack("New ClientObjectTemplate",                  IL_PIXMAP(hi16_mime_document2),  "&New",     0, p, "ClientObjectTemplate_new");
	IGNORE_RETURN(connect(m_clientCreate, SIGNAL(activated()), this, SLOT(onClientNew())));

	m_clientView      = new ActionHack("View ClientObjectTemplate in Editor",       IL_PIXMAP(hi16_mime_document),   "&View",    0, p, "ClientObjectTemplate_view");
	IGNORE_RETURN(connect(m_clientView, SIGNAL(activated()), this, SLOT(onClientView())));

	m_clientEdit      = new ActionHack("Edit ClientObjectTemplate from Perforce",   IL_PIXMAP(hi16_action_edit),     "&Edit",    0, p, "ClientObjectTemplate_edit");
	IGNORE_RETURN(connect(m_clientEdit, SIGNAL(activated()), this, SLOT(onClientEdit())));

	m_clientRevert    = new ActionHack("Revert ClientObjectTemplate from Perforce", IL_PIXMAP(hi16_action_revert),   "Re&vert",  0, p, "ClientObjectTemplate_revert");
	IGNORE_RETURN(connect(m_clientRevert, SIGNAL(activated()), this, SLOT(onClientRevert())));

	m_clientSubmit    = new ActionHack("Submit ClientObjectTemplate to Perforce",   IL_PIXMAP(hi16_action_finish),   "&Submit",  0, p, "ClientObjectTemplate_submit");
	IGNORE_RETURN(connect(m_clientSubmit, SIGNAL(activated()), this, SLOT(onClientSubmit())));

	m_clientCompile   = new ActionHack("Compile ClientObjectTemplate",              IL_PIXMAP(hi16_action_gear),     "&Compile", 0, p, "ClientObjectTemplate_compile");
	IGNORE_RETURN(connect(m_clientCompile, SIGNAL(activated()), this, SLOT(onClientCompile())));

	m_clientRefresh   = new ActionHack("Refresh ClientObjectTemplate List",         IL_PIXMAP(hi16_action_reload),   "&Refresh", 0, p, "ClientObjectTemplate_refresh");

	m_clientReplace   = new ActionHack("Replace selection with ClientObjectTemplate", IL_PIXMAP(hi16_action_gear),     "&Replace", 0, p, "ClientObjectTemplate_replace");
	IGNORE_RETURN(connect(m_clientReplace, SIGNAL(activated()), this, SLOT(onClientReplace())));

	m_serverAddToFavorites = new ActionHack("Add ServerTemplate to favorites", IL_PIXMAP(hi16_action_gear), "&Add to favorites", 0, p, "ServerObjectTemplate_addToFavorites");
	IGNORE_RETURN(connect(m_serverAddToFavorites, SIGNAL(activated()), this, SLOT(onServerAddToFavorites())));

	m_clientAddToFavorites = new ActionHack("Add ClientTemplate to favorites", IL_PIXMAP(hi16_action_gear), "&Add to favorites", 0, p, "ClientObjectTemplate_addToFavorites");
	IGNORE_RETURN(connect(m_clientAddToFavorites, SIGNAL(activated()), this, SLOT(onClientAddToFavorites())));

	m_serverView->setEnabled        (false);
	m_serverEdit->setEnabled        (false);
	m_serverRevert->setEnabled      (false);
	m_serverReload->setEnabled      (false);
	m_serverSubmit->setEnabled      (false);
	m_serverCompile->setEnabled     (false);
	m_serverReplace->setEnabled     (false);

	m_clientView->setEnabled        (false);
	m_clientEdit->setEnabled        (false);
	m_clientRevert->setEnabled      (false);
	m_clientSubmit->setEnabled      (false);
	m_clientCompile->setEnabled     (false);
	m_clientReplace->setEnabled     (false);

	m_serverAddToFavorites->setEnabled (false);
	m_clientAddToFavorites->setEnabled (false);
}

//-----------------------------------------------------------------------

ActionsObjectTemplate::~ActionsObjectTemplate()
{

	m_serverCreate = 0;
	m_serverView = 0;
	m_serverEdit = 0;
	m_serverRevert = 0;
	m_serverRefresh = 0;
	m_serverSubmit = 0;
	m_serverCompile = 0;
	m_serverReload = 0;
	m_serverReplace = 0;
	m_clientCreate = 0;
	m_clientView = 0;
	m_clientEdit = 0;
	m_clientRevert = 0;
	m_clientRefresh = 0;
	m_clientSubmit = 0;
	m_clientCompile = 0;
	m_clientReplace = 0;
	m_serverAddToFavorites = 0;
	m_clientAddToFavorites = 0;
}

//-----------------------------------------------------------------------

/**
 * Open an arbitrary file in the user's editor.  Editor is set in [GocClient] editor
 *
 */
void ActionsObjectTemplate::doEditFile(const std::string& filename) const
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

	const char * editor = NON_NULL(ConfigGodClient::getData().templateEditor);

	std::string arg0 = editor;

	const size_t slashpos = arg0.find_last_of("\\/");
	if(slashpos != arg0.npos && slashpos <(arg0.size() - 1)) //lint !e737 implicit promotion, bug in STL with size_t and std::string::npos being of different signage
		arg0 = arg0.substr(slashpos + 1);

	const int retval = _spawnlp(_P_NOWAIT, editor, arg0.c_str(), filename.c_str(), 0);

	if(retval == -1)
	{
		const std::string err = std::string("Unable to execute ObjectTemplateEditor [") + editor + "]: " + strerror(errno);
		IGNORE_RETURN(QMessageBox::warning(&MainFrame::getInstance(), "Warning", err.c_str()));
		return;
	}
}
/**
 * Simply convert all / to .
 * Does not strip .class or .ObjectTemplate suffix
 */

//-----------------------------------------------------------------------

const std::string ActionsObjectTemplate::convertToClassPath(const std::string& path)
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


//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
//BEGIN SERVER OBJECT TEMPLATE FUNCTIONS

/**
 * Slot to handle updating the global ObjectTemplate or ObjectTemplate path selection.
 */
void ActionsObjectTemplate::onServerObjectTemplatePathSelectionChanged(const std::string& path, bool isFile, bool isNew, bool isEdit)
{
	m_selectedServerPath = path;
	m_isServerFile       = isFile;
	m_isServerNew        = isNew;
	m_isServerEdit       = isEdit;

	m_serverView->setEnabled        (!m_selectedServerPath.empty() && isFile);
	m_serverEdit->setEnabled        (m_serverView->isEnabled());
	m_serverReload->setEnabled      (m_serverView->isEnabled());

	m_serverRevert->setEnabled      (m_serverView->isEnabled() && (isEdit || isNew));
	m_serverSubmit->setEnabled      (m_serverView->isEnabled() && (isEdit || isNew));
	m_serverCompile->setEnabled     (false);//revert->isEnabled());
	m_serverReplace->setEnabled     (m_serverView->isEnabled());

	m_serverAddToFavorites->setEnabled(m_serverView->isEnabled());
}

//-----------------------------------------------------------------------

/**
 * Revert the selected ObjectTemplate.  Calls p4 revert on both the class and ObjectTemplate file
 */
void ActionsObjectTemplate::onServerRevert()
{
	if(m_selectedServerPath.empty() || !m_isServerFile || !(m_isServerEdit || m_isServerNew))
		return;

	{
		const std::string classPath = ActionsObjectTemplate::convertToClassPath(m_selectedServerPath);
		const std::string msg = std::string("Really revert ObjectTemplate: [") + classPath + "]?";
		const int retval = QMessageBox::warning(&MainFrame::getInstance(), "Warning", msg.c_str(), QMessageBox::Yes, QMessageBox::No);
		
		if(retval == QMessageBox::No)
			return;
	}

	GodClientPerforce::StringVector sv;
	sv.reserve(2);
	sv.push_back(getServerObjectTemplateTpfFilePath());
	sv.push_back(getServerObjectTemplateIffFilePath());

	std::string result;

	if(!GodClientPerforce::getInstance().revertFiles(sv, false, result))
	{
		const std::string msg = "Could not revert class files: " + sv [0] + ", " + sv [1] + "\n" + result;
		IGNORE_RETURN(QMessageBox::warning(&MainFrame::getInstance(), "Warning", msg.c_str()));
	}

	m_serverRefresh->doActivate();
}

//-----------------------------------------------------------------------

/**
 * Opens the editor on the selected .ObjectTemplate file
 *
 */
void ActionsObjectTemplate::onServerView() const
{
	if(m_selectedServerPath.empty() || !m_isServerFile)
		return;

	std::string dummy;
	std::string local;

	const std::string path = getServerObjectTemplateTpfFilePath();

	std::string result;

	//-- map the desired ObjectTemplate classpath to a local source directory
	if(!GodClientPerforce::getInstance().getFileMapping(path, dummy, dummy, local, result))
	{
		const std::string msg = "Can't 'where' directory.  User error?  Perhaps.\n" + result;
		IGNORE_RETURN(QMessageBox::warning(&MainFrame::getInstance(), "Warning", msg.c_str()));
		return;
	}

	doEditFile(local);
}

//-----------------------------------------------------------------------

/**
 * p4 edits the selected .ObjectTemplate and .class file, and opens the editor on the .ObjectTemplate file
 *
 */
void ActionsObjectTemplate::onServerEdit()
{
	if(m_selectedServerPath.empty() || !m_isServerFile)
		return;

	std::string dummy;
	std::string local;
	std::string result;

	GodClientPerforce::StringVector sv;
	sv.reserve(2);
	sv.push_back(getServerObjectTemplateTpfFilePath());
	sv.push_back(getServerObjectTemplateIffFilePath());

	if(!GodClientPerforce::getInstance().editFiles(sv, result))
	{
		const std::string msg = "Could not open file(s) for perforce edit.  View Will be read-only." + result;
		IGNORE_RETURN(QMessageBox::warning(&MainFrame::getInstance(), "Warning", msg.c_str()));

		// TODO: missing a return here???
	}


	//-- map the desired ObjectTemplate classpath to a local source directory
	if(!GodClientPerforce::getInstance().getFileMapping(sv [0], dummy, dummy, local, result))
	{
		const std::string msg = "Can't 'where' directory.  User error?  Perhaps.\n" + result;
		IGNORE_RETURN(QMessageBox::warning(&MainFrame::getInstance(), "Warning", msg.c_str()));
		return;
	}

	m_serverRefresh->doActivate();
	doEditFile(local);
}

//-----------------------------------------------------------------------

/**
 * Prompts the user to select a new .ObjectTemplate file.  Adds the resulting .ObjectTemplate and .class files to perforce.
 * Creates the .ObjectTemplate file and opens the editor on it.
 *
 */
void ActionsObjectTemplate::onServerNew()
{
	std::string actualPath = m_selectedServerPath;

	//-- trim the filename from the path
	if(!actualPath.empty() && m_isServerFile)
	{
		const size_t slashpos = actualPath.rfind('/');

		if(slashpos == actualPath.npos) //lint !e737 implicit promotion, bug in STL with size_t and std::string::npos being of different signage 
			actualPath.clear();
		else
			actualPath = actualPath.substr(0, slashpos);
	}


	const char * const srcPath = NON_NULL(ConfigGodClient::getData().templateServerSourcePath);
	std::string path = GodClientPerforce::concatenateSubpath(srcPath, actualPath);

	std::string dummy;
	std::string local;
	std::string result;

	//-- map the desired ObjectTemplate classpath to a local source directory
	if(!GodClientPerforce::getInstance().getFileMapping(path, dummy, dummy, local, result))
	{
		const std::string msg = "Can't 'where' directory.  User error?  Perhaps.\n" + result;
		IGNORE_RETURN(QMessageBox::warning(&MainFrame::getInstance(), "Warning", msg.c_str()));
		return;
	}

	const QString qselection = QFileDialog::getSaveFileName(local.c_str(), "*.tpf",&MainFrame::getInstance());
	
	if(qselection == QString::null)
		return;

	std::string selection = qselection.latin1();
	if(selection.size() < 4 || selection.substr(selection.size() - 4) != ms_objectTemplateSourceExtension)
		selection += ms_objectTemplateSourceExtension;

	UNREF(selection);

	//-- discover the depot path to the .class file
	size_t dsrc_pos = selection.find("\\dsrc\\");
	if(dsrc_pos == selection.npos) //lint !e737 implicit promotion, bug in STL with size_t and std::string::npos being of different signage
		dsrc_pos = selection.find("/dsrc/");

	if(dsrc_pos == selection.npos) //lint !e737 implicit promotion, bug in STL with size_t and std::string::npos being of different signage 
	{
		IGNORE_RETURN(QMessageBox::warning(&MainFrame::getInstance(), "Warning", "Can't translate ObjectTemplate path to Iff path."));
		return;
	}

	std::string compiledLocalPath = selection;
	IGNORE_RETURN(compiledLocalPath.replace(dsrc_pos + 1, 4, "data"));
	IGNORE_RETURN(compiledLocalPath.replace(compiledLocalPath.size() - 6, 6, "iff"));

	GodClientPerforce::StringVector sv;
	sv.reserve(2);
	sv.push_back(selection);
	sv.push_back(compiledLocalPath);

	if(!GodClientPerforce::getInstance().addFiles(sv, result))
	{
		const std::string msg = "Unable to add new files to perforce.\n" + result;
		IGNORE_RETURN(QMessageBox::warning(&MainFrame::getInstance(), "Warning", msg.c_str()));
		return;
	}

	m_serverRefresh->doActivate();
	doEditFile(selection);
}

//-----------------------------------------------------------------------

/**
 * Requests the server to relaod the ObjectTemplate.
 *
 */
void ActionsObjectTemplate::onServerReload() const
{
	if(m_selectedServerPath.empty() || !m_isServerFile)
		return;

	std::string classPath = "object/";
	classPath += m_selectedServerPath + ".iff";
	const std::string msg = std::string("Really server-reload ObjectTemplate: [") + classPath + "]?";
	const int retval = QMessageBox::warning(&MainFrame::getInstance(), "Warning", msg.c_str(), QMessageBox::Yes, QMessageBox::No);

	if(retval == QMessageBox::No)
		return;

	IGNORE_RETURN(ServerCommander::getInstance().serverTemplateReload(classPath));
}

//-----------------------------------------------------------------------

/**
 * Submit the selected ObjectTemplate to perforce, reverting unchanged first if possible.
 *
 */
void ActionsObjectTemplate::onServerSubmit()
{
	if(m_selectedServerPath.empty() || !m_isServerFile || !(m_isServerEdit || m_isServerNew))
		return;

	const std::string classPath = ActionsObjectTemplate::convertToClassPath(m_selectedServerPath);
	const std::string msg = std::string("Really submit ObjectTemplate: [") + classPath + "]?";
	const int retval = QMessageBox::warning(&MainFrame::getInstance(), "Warning", msg.c_str(), QMessageBox::Yes, QMessageBox::No);

	if(retval == QMessageBox::No)
		return;

	GodClientPerforce::StringVector sv;
	sv.reserve(2);
	sv.push_back(getServerObjectTemplateTpfFilePath());
	sv.push_back(getServerObjectTemplateIffFilePath());

	std::string result;

	IGNORE_RETURN(GodClientPerforce::getInstance().revertFiles(sv, true, result));

	if(!GodClientPerforce::getInstance().submitFiles(sv, result))
	{
		const std::string msg = "Could not submit file(s) to perforce!  Beanhead.\n" + result;
		IGNORE_RETURN(QMessageBox::warning(&MainFrame::getInstance(), "Warning", msg.c_str()));
	}

	m_serverRefresh->doActivate();
}

//-----------------------------------------------------------------------

/**
 * Compile the selected ObjectTemplate..
 * @todo implement ObjectTemplate compile process and popup a text output window for compiler output
 *
 */
void ActionsObjectTemplate::onServerCompile() const
{
	if(m_selectedServerPath.empty() || !m_isServerFile)
		return;
}

//-----------------------------------------------------------------------

const std::string ActionsObjectTemplate::getServerObjectTemplateTpfFilePath() const
{
	if(m_selectedServerPath.empty() || !m_isServerFile)
		return "";

	const char* const srcPath = NON_NULL(ConfigGodClient::getData().templateServerSourcePath);
	return GodClientPerforce::concatenateSubpath(srcPath, m_selectedServerPath) + ms_objectTemplateSourceExtension; 
}

//-----------------------------------------------------------------------

const std::string ActionsObjectTemplate::getServerObjectTemplateIffFilePath() const
{
	if(m_selectedServerPath.empty() || !m_isServerFile)
		return "";

	const char* const iffPath = NON_NULL(ConfigGodClient::getData().templateServerIffPath);
	return GodClientPerforce::concatenateSubpath(iffPath, m_selectedServerPath) + ms_objectTemplateCompiledExtension;
}

//-----------------------------------------------------------------------

bool ActionsObjectTemplate::getSelectedServerObjectTemplate(std::string& path) const
{
	if(m_selectedServerPath.empty() || !m_isServerFile)
		return false;

	path = convertToClassPath(m_selectedServerPath);
	return true;
}

//END SERVER OBJECT TEMPLATE FUNCTIONS
//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
//BEGIN *CLIENT* OBJECT TEMPLATE FUNCTIONS

/**
 * Slot to handle updating the global ObjectTemplate or ObjectTemplate path selection.
 */
void ActionsObjectTemplate::onClientObjectTemplatePathSelectionChanged(const std::string& path, bool isFile, bool isNew, bool isEdit)
{
	m_selectedClientPath = path;
	m_isClientFile = isFile;
	m_isClientNew  = isNew;
	m_isClientEdit = isEdit;

	m_clientView->setEnabled        (!m_selectedClientPath.empty() && isFile);
	m_clientEdit->setEnabled        (m_clientView->isEnabled());

	m_clientRevert->setEnabled      (m_clientView->isEnabled() && (isEdit || isNew));
	m_clientSubmit->setEnabled      (m_clientView->isEnabled() && (isEdit || isNew));
	m_clientCompile->setEnabled     (false);//revert->isEnabled());
	m_clientReplace->setEnabled     (m_clientView->isEnabled());

	m_clientAddToFavorites->setEnabled(m_clientView->isEnabled());
}

//-----------------------------------------------------------------------

/**
 * Revert the selected ObjectTemplate.  Calls p4 revert on both the class and ObjectTemplate file
 */
void ActionsObjectTemplate::onClientRevert()
{
	if(m_selectedClientPath.empty() || !m_isClientFile || !(m_isClientEdit || m_isClientNew))
		return;

	{
		const std::string classPath = ActionsObjectTemplate::convertToClassPath(m_selectedClientPath);
		const std::string msg = std::string("Really revert ObjectTemplate: [") + classPath + "]?";
		const int retval = QMessageBox::warning(&MainFrame::getInstance(), "Warning", msg.c_str(), QMessageBox::Yes, QMessageBox::No);
		
		if(retval == QMessageBox::No)
			return;
	}

	GodClientPerforce::StringVector sv;
	sv.reserve(2);
	sv.push_back(getClientObjectTemplateTpfFilePath());
	sv.push_back(getClientObjectTemplateIffFilePath());

	std::string result;

	if(!GodClientPerforce::getInstance().revertFiles(sv, false, result))
	{
		const std::string msg = "Could not revert class files: " + sv [0] + ", " + sv [1] + "\n" + result;
		IGNORE_RETURN(QMessageBox::warning(&MainFrame::getInstance(), "Warning", msg.c_str()));
	}

	m_clientRefresh->doActivate();
}

//-----------------------------------------------------------------------

/**
 * Opens the editor on the selected .ObjectTemplate file
 *
 */
void ActionsObjectTemplate::onClientView() const
{
	if(m_selectedClientPath.empty() || !m_isClientFile)
		return;

	std::string dummy;
	std::string local;
	std::string result;

	const std::string path = getClientObjectTemplateTpfFilePath();

	//-- map the desired ObjectTemplate classpath to a local source directory
	if(!GodClientPerforce::getInstance().getFileMapping(path, dummy, dummy, local, result))
	{
		const std::string msg = "Can't 'where' directory.  User error?  Perhaps.\n" + result;
		IGNORE_RETURN(QMessageBox::warning(&MainFrame::getInstance(), "Warning", msg.c_str()));
		return;
	}

	doEditFile(local);
}

//-----------------------------------------------------------------------

/**
 * p4 edits the selected .ObjectTemplate and .class file, and opens the editor on the .ObjectTemplate file
 *
 */
void ActionsObjectTemplate::onClientEdit()
{
	if(m_selectedClientPath.empty() || !m_isClientFile)
		return;

	std::string dummy;
	std::string local;
	std::string result;

	GodClientPerforce::StringVector sv;
	sv.reserve(2);
	sv.push_back(getClientObjectTemplateTpfFilePath());
	sv.push_back(getClientObjectTemplateIffFilePath());

	if(!GodClientPerforce::getInstance().editFiles(sv, result))
	{
		const std::string msg = "Could not open file(s) for perforce edit.  View Will be read-only.\n" + result;
		IGNORE_RETURN(QMessageBox::warning(&MainFrame::getInstance(), "Warning", msg.c_str()));
	}

	//-- map the desired ObjectTemplate classpath to a local source directory
	if(!GodClientPerforce::getInstance().getFileMapping(sv [0], dummy, dummy, local, result))
	{
		const std::string msg = "Can't 'where' directory.  User error?  Perhaps.\n" + result;
		IGNORE_RETURN(QMessageBox::warning(&MainFrame::getInstance(), "Warning", msg.c_str()));
		return;
	}

	m_clientRefresh->doActivate();
	doEditFile(local);
}

//-----------------------------------------------------------------------

/**
 * Prompts the user to select a new .ObjectTemplate file.  Adds the resulting .ObjectTemplate and .class files to perforce.
 * Creates the .ObjectTemplate file and opens the editor on it.
 *
 */
void ActionsObjectTemplate::onClientNew()
{
	std::string actualPath = m_selectedClientPath;

	//-- trim the filename from the path
	if(!actualPath.empty() && m_isClientFile)
	{
		const size_t slashpos = actualPath.rfind('/');

		if(slashpos == actualPath.npos) //lint !e737 implicit promotion, bug in STL with size_t and std::string::npos being of different signage 
			actualPath.clear();
		else
			actualPath = actualPath.substr(0, slashpos);
	}


	const char* const srcPath = NON_NULL(ConfigGodClient::getData().templateClientSourcePath);
	std::string path = GodClientPerforce::concatenateSubpath(srcPath, actualPath);

	std::string dummy;
	std::string local;
	std::string result;

	//-- map the desired ObjectTemplate classpath to a local source directory
	if(!GodClientPerforce::getInstance().getFileMapping(path, dummy, dummy, local, result))
	{
		const std::string msg = "Can't 'where' directory.  User error?  Perhaps.\n" + result;
		IGNORE_RETURN(QMessageBox::warning(&MainFrame::getInstance(), "Warning", msg.c_str()));
		return;
	}

	const QString qselection = QFileDialog::getSaveFileName(local.c_str(), "*.tpf",&MainFrame::getInstance());
	
	if(qselection == QString::null)
		return;

	std::string selection = qselection.latin1();
	if(selection.size() < 4 || selection.substr(selection.size() - 4) != ms_objectTemplateSourceExtension)
		selection += ms_objectTemplateSourceExtension;

	UNREF(selection);

	//-- discover the depot path to the .class file
	size_t dsrc_pos = selection.find("\\dsrc\\");
	if(dsrc_pos == selection.npos) //lint !e737 implicit promotion, bug in STL with size_t and std::string::npos being of different signage 
		dsrc_pos = selection.find("/dsrc/");

	if(dsrc_pos == selection.npos) //lint !e737 implicit promotion, bug in STL with size_t and std::string::npos being of different signage 
	{
		IGNORE_RETURN(QMessageBox::warning(&MainFrame::getInstance(), "Warning", "Can't translate ObjectTemplate path to Iff path."));
		return;
	}

	std::string compiledLocalPath = selection;
	IGNORE_RETURN(compiledLocalPath.replace(dsrc_pos + 1, 4, "data"));
	IGNORE_RETURN(compiledLocalPath.replace(compiledLocalPath.size() - 6, 6, "iff"));

	GodClientPerforce::StringVector sv;
	sv.reserve(2);
	sv.push_back(selection);
	sv.push_back(compiledLocalPath);

	if(!GodClientPerforce::getInstance().addFiles(sv, result))
	{
		const std::string msg = "Unable to add new files to perforce.\n" + result;
		IGNORE_RETURN(QMessageBox::warning(&MainFrame::getInstance(), "Warning", msg.c_str()));
		return;
	}

	m_clientRefresh->doActivate();
	doEditFile(selection);
}

//-----------------------------------------------------------------------

/**
 * Submit the selected ObjectTemplate to perforce, reverting unchanged first if possible.
 *
 */
void ActionsObjectTemplate::onClientSubmit()
{
	if(m_selectedClientPath.empty() || !m_isClientFile || !(m_isClientEdit || m_isClientNew))
		return;

	const std::string classPath = ActionsObjectTemplate::convertToClassPath(m_selectedClientPath);
	const std::string msg = std::string("Really submit ObjectTemplate: [") + classPath + "]?";
	const int retval = QMessageBox::warning(&MainFrame::getInstance(), "Warning", msg.c_str(), QMessageBox::Yes, QMessageBox::No);

	if(retval == QMessageBox::No)
		return;

	GodClientPerforce::StringVector sv;
	sv.reserve(2);
	sv.push_back(getClientObjectTemplateTpfFilePath());
	sv.push_back(getClientObjectTemplateIffFilePath());

	std::string result;

	IGNORE_RETURN(GodClientPerforce::getInstance().revertFiles(sv, true, result));

	if(!GodClientPerforce::getInstance().submitFiles(sv, result))
	{
		const std::string msg = "Could not submit file(s) to perforce!  Beanhead.\n" + result;
		IGNORE_RETURN(QMessageBox::warning(&MainFrame::getInstance(), "Warning", msg.c_str()));
	}

	m_clientRefresh->doActivate();
}

//-----------------------------------------------------------------------

/**
 * Compile the selected ObjectTemplate..
 * @todo implement ObjectTemplate compile process and popup a text output window for compiler output
 *
 */
void ActionsObjectTemplate::onClientCompile() const
{
	if(m_selectedClientPath.empty() || !m_isClientFile)
		return;
}

//-----------------------------------------------------------------------

const std::string ActionsObjectTemplate::getClientObjectTemplateTpfFilePath() const
{
	if(m_selectedClientPath.empty() || !m_isClientFile)
		return "";

	const char * const srcPath = NON_NULL(ConfigGodClient::getData().templateClientSourcePath);
	return GodClientPerforce::concatenateSubpath(srcPath, m_selectedClientPath) + ms_objectTemplateSourceExtension; 
}

//-----------------------------------------------------------------------

const std::string ActionsObjectTemplate::getClientObjectTemplateIffFilePath() const
{
	if(m_selectedClientPath.empty() || !m_isClientFile)
		return "";

	const char * const iffPath = NON_NULL(ConfigGodClient::getData().templateClientIffPath);
	return GodClientPerforce::concatenateSubpath(iffPath, m_selectedClientPath) + ms_objectTemplateCompiledExtension;
}

//-----------------------------------------------------------------------

bool ActionsObjectTemplate::getSelectedClientObjectTemplate(std::string& path) const
{
	if(m_selectedClientPath.empty() || !m_isClientFile)
		return false;

	path = convertToClassPath(m_selectedClientPath);
	return true;
}

//-----------------------------------------------------------------------

void ActionsObjectTemplate::onServerReplace()
{
	replaceObject(false);
}

//-----------------------------------------------------------------------

void ActionsObjectTemplate::onClientReplace()
{
	replaceObject(true);
}

//-----------------------------------------------------------------------

void ActionsObjectTemplate::onClientAddToFavorites()
{
	FavoritesWindow *pFavWindow = MainFrame::getInstance().getFavoritesWindow();
	if(pFavWindow)
	{
		pFavWindow->addClientTemplateToRoot(m_selectedClientPath);
	}
}

//-----------------------------------------------------------------------

void ActionsObjectTemplate::onServerAddToFavorites()
{
	FavoritesWindow *pFavWindow = MainFrame::getInstance().getFavoritesWindow();
	if(pFavWindow)
	{
		pFavWindow->addServerTemplateToRoot(m_selectedServerPath);
	}
}

//-----------------------------------------------------------------------

void ActionsObjectTemplate::replaceObject(bool client)
{
	//-- replace the current object with the one in the menu
	GodClientData * gcd = &GodClientData::getInstance();

	if (gcd->getSelectionEmpty())
	{
		const std::string err = std::string("No objects are currently selected!");
		IGNORE_RETURN(QMessageBox::warning(&MainFrame::getInstance(), "Warning", err.c_str()));
		return;
	}

	std::string path;
	
	if (client)
		path = std::string("object/") + m_selectedClientPath + std::string(".iff");
	else
		path = std::string("object/") + m_selectedServerPath + std::string(".iff");

	if (!path.empty())
	{
		GodClientData::ObjectList_t olist;

		gcd->getSelection(olist);
		gcd->clearSelection();

		for(GodClientData::ObjectList_t::iterator it = olist.begin(); it != olist.end(); ++it)
		{
			ClientObject * const obj = *it;

			if (!obj)
			{
				const std::string err = std::string("Invalid object selection list!");
				IGNORE_RETURN(QMessageBox::warning(&MainFrame::getInstance(), "Warning", err.c_str()));
				return;
			}

			Transform t = obj->getTransform_o2p();

			CellProperty * parentCellProperty = obj->getParentCell();
			
			IGNORE_RETURN(ServerCommander::getInstance().createObject("", path, parentCellProperty, t));
			IGNORE_RETURN(ServerCommander::getInstance().deleteObject(obj));
		}
	}
}

//-----------------------------------------------------------------------

//END *CLIENT* OBJECT TEMPLATE FUNCTIONS
//-----------------------------------------------------------------------

