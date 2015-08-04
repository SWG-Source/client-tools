// ======================================================================
//
// ToolProcess.h
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ToolProcess_H
#define INCLUDED_ToolProcess_H

// ----------------------------------------------------------------------

#include <qprocess.h>

// ----------------------------------------------------------------------

#include <list>
#include <string>
#include <vector>

// ----------------------------------------------------------------------

class ToolProcess : public QProcess
{
	Q_OBJECT //lint !e830 !e1511 !e1516 !e1924

public:
	explicit ToolProcess(char const * const toolDirectory);
    ~ToolProcess();

	void addToPerforce(char const * const filename);
	void compileDataTable(char const * const filename);
	void buildQuestCrcStringTables(char const * const branch);
	void checkQuest(char const * const filename);

protected:
	void startNextProcess();

signals:
	void consoleOutput(const QString&);

private slots:
	void slotReadProcessOutput();
	void slotProcessExited();

private:
	typedef std::vector<std::string> StringVector;
	typedef std::list<StringVector> ProcessArgumentList;

	ProcessArgumentList m_procArgList;

private: //-- disabled
	ToolProcess();
	ToolProcess(ToolProcess const &);
	ToolProcess &operator=(ToolProcess const &);
};

// ======================================================================

#endif // INCLUDED_ToolProcess_H
