// ======================================================================
//
// ToolProcess.cpp
// Copyright 2004, Sony Online Entertainment Inc.
//
// ======================================================================

#include "FirstQuestEditor.h"

// ----------------------------------------------------------------------

#include "ToolProcess.h"

// ----------------------------------------------------------------------

#include "ToolProcess.moc"

// ----------------------------------------------------------------------

ToolProcess::ToolProcess(char const * const toolDirectory)
: QProcess()
, m_procArgList()
{
	IGNORE_RETURN(connect(this, SIGNAL(readyReadStdout()), this, SLOT(slotReadProcessOutput())));
	IGNORE_RETURN(connect(this, SIGNAL(readyReadStderr()), this, SLOT(slotReadProcessOutput())));
	IGNORE_RETURN(connect(this, SIGNAL(processExited()), this, SLOT(slotProcessExited())));

	QProcess::setWorkingDirectory(QDir(toolDirectory));
}

// ----------------------------------------------------------------------

ToolProcess::~ToolProcess()
{
}

// ----------------------------------------------------------------------

void ToolProcess::startNextProcess()
{
	QString cmdString;

	if (isRunning())
		return;

	if (m_procArgList.empty())
	{
		consoleOutput("\n*********** DONE ***********\n\n");
		QApplication::beep();
		return;
	}

	StringVector v = m_procArgList.front();
	m_procArgList.pop_front();

	clearArguments();
	for (StringVector::iterator i = v.begin(); i != v.end(); ++i)
	{
		addArgument(i->c_str());

		if(!cmdString.isEmpty())
			IGNORE_RETURN(cmdString.append(" "));

		IGNORE_RETURN(cmdString.append(i->c_str()));
	}

	if (!start())
	{
		emit consoleOutput("*** Failure: [" + cmdString + "] ***\n");

		//-- if there is a failure, run the next process
		emit processExited();
	}
	else
		emit consoleOutput(cmdString + "\n");
}

// ----------------------------------------------------------------------

void ToolProcess::slotReadProcessOutput()
{
	emit consoleOutput(readStdout());
	emit consoleOutput(readStderr());
}

// ----------------------------------------------------------------------

void ToolProcess::slotProcessExited()
{
	startNextProcess();
}

// ----------------------------------------------------------------------

void ToolProcess::addToPerforce(char const * const filename)
{
	StringVector v;

	v.push_back("p4");
	v.push_back("edit");
	v.push_back(filename);

	m_procArgList.push_back(v);

	v.clear();

	v.push_back("p4");
	v.push_back("add");
	v.push_back(filename);

	m_procArgList.push_back(v);

	startNextProcess();
}

// ----------------------------------------------------------------------

void ToolProcess::compileDataTable(char const * const filename)
{
	StringVector v;

	v.push_back("DataTableTool");
	v.push_back("-i");
	v.push_back(filename);

	m_procArgList.push_back(v);

	startNextProcess();
}

// ----------------------------------------------------------------------

void ToolProcess::checkQuest(char const * const filename)
{
	StringVector v;

	v.push_back("perl");
	v.push_back("../exe/win32/QuestChecker.pl");
	v.push_back(filename);

	m_procArgList.push_back(v);

	startNextProcess();
}

// ----------------------------------------------------------------------

void ToolProcess::buildQuestCrcStringTables(char const * const branch)
{
	StringVector v;

	v.push_back("perl");
	v.push_back("buildQuestCrcStringTables.pl");
	v.push_back("--local");
	v.push_back(branch);

	m_procArgList.push_back(v);

	startNextProcess();
}

// ======================================================================
