// ======================================================================
//
// MainWindow.cpp
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "BugTool/FirstBugTool.h"
#include "BugTool/MainWindow.h"

#include <qaction.h>
#include <qapplication.h>
#include <qcombobox.h>
#include <qfiledialog.h>
#include <qlistview.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qtextedit.h>

#include <assert.h>

// ======================================================================

MainWindow *MainWindow::ms_theMainWindow;
const std::string MainWindow::ms_smtpServer = "mail.station.sony.com";
const std::string MainWindow::ms_smtpPort   = "2525";
const std::string MainWindow::ms_to         = "swgbetatestbugs@soe.sony.com";

// ======================================================================

MainWindow &MainWindow::getInstance()
{
	return *ms_theMainWindow;
}

// ----------------------------------------------------------------------

MainWindow::MainWindow(QWidget* newParent, const char* newName, WFlags fl)
: BaseMainWindow(newParent, newName, fl),
  m_defaultDescriptionString(m_descriptionTextEdit->text()), //use the description string from the Qt-Designer as the default
  m_defaultTitleBar(caption()),                              //use the caption from the Qt-Designer as the default
  m_stationId("unassigned"),
  m_currentDir(".")
{
	updateTitleBar();
	connect(DeleteAttachmentAction, SIGNAL(activated()), this, SLOT(onDeleteAttachment()));
}

// ----------------------------------------------------------------------

MainWindow::~MainWindow()
{
}

// ----------------------------------------------------------------------

/** Update the stationId being stored, also update the title bar to reflect the change
 */
void MainWindow::setStationId(const QString& id)
{
	if(std::string(id.latin1()) == "")
		m_stationId = "unknown";
	else
		m_stationId = id;
	updateTitleBar();
}

// ----------------------------------------------------------------------

/** This function sets the title bar to the Qt-Designer default for the window + the stationId passed in from the command line
 */
void MainWindow::updateTitleBar()
{
	QString n = m_defaultTitleBar;
	n += " - StationId: ";
	n += m_stationId;
	setCaption(n);
}

// ----------------------------------------------------------------------

/** Store the currently selected item
 */
void MainWindow::setAttachmentSelection(QListViewItem* item)
{
	m_selectedAttachment = item;
}

// ----------------------------------------------------------------------

/** Delete the currently selected item
 */
void MainWindow::onDeleteAttachment()
{
	if(m_selectedAttachment)
	{
		QListView* lv = m_selectedAttachment->listView();
		if(lv)
			lv->takeItem(m_selectedAttachment);
	}
}

// ----------------------------------------------------------------------

/** Present the user with a dialog box to add a file to the attachments list
 */
void MainWindow::addAttachment()
{
	//start looking for files in the current directory (which should be close to the useful files)
	QString file = QFileDialog::getOpenFileName(m_currentDir, 
	                                            "SWG Attachments (*.mdmp *.tga *.txt *.log *.cfg *.jpg *.jpeg)",
	                                            this, 
	                                            "open file dialog", 
	                                            "Choose a file");
	//we get a null string if they hit Cancel
	if(file == "")
		return;

	std::string f = file.latin1();
	std::string::size_type pos = f.find_last_of("/");
	if(pos != f.npos)
	{
		f.erase(pos);
	}

	m_currentDir = f.c_str();

	//@todo problem, this returns files with '/', and blat required '\\', sigh

	QListViewItem* element = new QListViewItem(m_attachmentListView, file);
	UNREF(element);
}

// ----------------------------------------------------------------------

/** Remove the current item from the attachments list
 */
void MainWindow::deleteAttachment(QListViewItem* item, const QPoint& pt, int)
{
	QPopupMenu* const pop = new QPopupMenu(this, "menu");

	if(item != 0)
	{
		DeleteAttachmentAction->addTo(pop);
		pop->popup(pt);
	}
}

// ----------------------------------------------------------------------

void MainWindow::quitApplication()
{
	qApp->quit();
}

// ----------------------------------------------------------------------

//@todo hack fix this (create a blat.h)
int callBlat(int argc, char **argv, char **envp);

// ----------------------------------------------------------------------

/** Build the e-mail, attach the attachments files (as stored in the ListView), and send the mail
 */
void MainWindow::sendBug()
{
	//Grab the bugtype, repeatable, and description text fields
	const QString bugType    = m_bugTypeComboBox->currentText();
	const QString repeatable = m_repeatableComboBox->currentText();
	const QString system     = m_systemComboBox->currentText();
	const QString severity   = m_severityComboBox->currentText();
	const QString server     = m_serverComboBox->currentText();
	const QString desc       = m_descriptionTextEdit->text();

	//If they didn't type a description, warn and return
	if (desc == m_defaultDescriptionString || desc == "")
	{
		QMessageBox::warning(this, "Need a description", "Please type a description first.");
		return;
	}

	//Build the "from" field, which is meaningless and doesn't have to be a real e-mail address
	//Use it to pass the stationId and help the receivers sort on good/bad testers, or whatever
	std::string station_account = "SWGTester_StationID_";
	station_account += m_stationId;
	station_account += "@swgbetatest.station.sony.com";

	//Build the e-mail body.  Place stationId, bugtype and repeatable on their own lines, then add the description next to it
	std::string mail_body;
	mail_body += "StationId: ";
	mail_body += m_stationId.latin1();
	mail_body += "\n";

	mail_body += "Bug Type: ";
	mail_body += bugType.latin1();
	mail_body += "\n";

	mail_body += "Repeatable: ";
	mail_body += repeatable.latin1();
	mail_body += "\n";

	mail_body += "Game System: ";
	mail_body += system.latin1();
	mail_body += "\n";

	mail_body += "Severity: ";
	mail_body += severity.latin1();
	mail_body += "\n";

	mail_body += "Server ";
	mail_body += server.latin1();
	mail_body += "\n";

	mail_body += "\n";
	mail_body += desc.latin1();

	//Build the e-mail subject (the first slice of the bug description is used, with ellipis if we cut it off)
	std::string std_desc = desc.latin1();
	std::string short_desc = std_desc.substr(0, 50);
	if(std_desc.size() > 50)
		short_desc += "...";

	std::string subject = bugType.latin1();
	subject += " Bug: ";
	subject += short_desc;

	//Build the call to blat (since it likes to work as a standalone, it expect an argc, argv)
	const int static_args = 13;
	const int attachments = m_attachmentListView->childCount();

	int argc = static_args + (2 * attachments);
	char** argv = new char*[argc];
	int argv_value = 1;
	argv[argv_value++] = "-to";
	argv[argv_value++] = const_cast<char*>(ms_to.c_str());
	argv[argv_value++] = "-subject";
	argv[argv_value++] = const_cast<char*>(subject.c_str());
	argv[argv_value++] = "-smtphost";
	argv[argv_value++] = const_cast<char*>(ms_smtpServer.c_str());
	argv[argv_value++] = "-port";
	argv[argv_value++] = const_cast<char*>(ms_smtpPort.c_str());
	argv[argv_value++] = "-f";
	argv[argv_value++] =  const_cast<char*>(station_account.c_str());
	argv[argv_value++] = "-body";
	argv[argv_value++] = const_cast<char*>(mail_body.c_str());

	std::vector<char*> files;
	std::string backSlashedAttachment;
	//iterate through all attachments, building the argv entries for them
	for(QListViewItem* item = m_attachmentListView->firstChild(); item != NULL; item = item->nextSibling())
	{
		argv[argv_value++] = "-attach";
		//Qt returns filepaths with '/', blat requires '\\', so convert
		char* file = convertToSystemPath(item->text(0).latin1());
		files.push_back(file);
		argv[argv_value++] = file;
	}

	//ensure we built the correct size argv
	assert(argv_value == argc);

	char* envp = new char[256];
	memset(envp, 0, 256);

	//call into blat to send the mail
	int result = callBlat(argc, argv, &envp);
	//clean up allocated memory
	delete[] argv;
	delete[] envp;

	for(std::vector<char*>::iterator i = files.begin(); i != files.end(); ++i)
	{
		delete (*i);
	}

	if(result != 0)
		QMessageBox::warning(this, "Error occured", "Some error occured when sending the mail.");
	else
	{
		QMessageBox::information(this, "Thank you", "Bug submitted.  Thank you.");
		m_descriptionTextEdit->clear();
		m_attachmentListView->clear();
	}
}

// ======================================================================

/** returns a newly allocated string, caller is responsible for deleting it
 */
char* MainWindow::convertToSystemPath(const std::string & path)
{
	std::string newpath = path;
	std::string buffer;
	char* temp = new char[256];
	std::string::size_type pos = newpath.find_first_of('/');
	while(pos != std::string::npos)
	{
		buffer = newpath.substr(0, pos);
		buffer += "\\";
		buffer.append(newpath.substr(pos+1));
		newpath = buffer;
		pos = newpath.find_first_of('/');
	}
	delete[] temp;
	char* file = new char[newpath.size()+1];
	int end = newpath.copy(file, newpath.size());
	file[end] = 0;
	return file;
}	// Filename::convertToSystemPath

// ======================================================================
