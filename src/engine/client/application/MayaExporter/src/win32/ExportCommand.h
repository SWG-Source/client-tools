// ======================================================================
//
// ExportCommand.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef EXPORT_COMMAND_H
#define EXPORT_COMMAND_H

#include <string>

// ======================================================================

class Messenger;

#include "maya/MPxCommand.h"

// ======================================================================

class ExportCommand : public MPxCommand
{
	friend BOOL CALLBACK exportDialogProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);

public:

	static void install(Messenger *newMessenger);
	static void remove();

	static void *creator();

public:

	MStatus doIt(const MArgList &argList);

private:
	static std::string ms_command;
};

// ======================================================================
#if !NO_DATABASE

class UseAssetDatabaseCommand : public MPxCommand
{
public:

	static void install(Messenger *newMessenger);
	static void remove();

	static void *creator();

public:

	MStatus doIt(const MArgList &argList);
};

#endif
// ======================================================================
#endif
