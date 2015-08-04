// ======================================================================
//
// RemoteDebugToolStatic.cpp
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#include "FirstRemoteDebugTool.h"
#include "RemoteDebugToolStatic.h"

#include "ApplicationWindow.h"

//-----------------------------------------------------------------------

bool RemoteDebugToolStatic::ms_installed;
ApplicationWindow* RemoteDebugToolStatic::ms_window;

//-----------------------------------------------------------------------

void RemoteDebugToolStatic::install(ApplicationWindow* window)
{
	ms_window = window;
	ms_installed = true;
}

//-----------------------------------------------------------------------

void RemoteDebugToolStatic::remove()
{
	ms_window = NULL;
	ms_installed = false;
}

//-----------------------------------------------------------------------

void RemoteDebugToolStatic::open(const char *server, uint16 port)
{
	DEBUG_FATAL(!ms_installed, ("RemoteDebugToolStatic not installed"));
	ms_window->open(server, port);
}

//-----------------------------------------------------------------------

void RemoteDebugToolStatic::close()
{
	DEBUG_FATAL(!ms_installed, ("RemoteDebugToolStatic not installed"));
//	ms_window.close();
}

//-----------------------------------------------------------------------

void RemoteDebugToolStatic::send(void *buffer, uint32 bufferLen)
{
	DEBUG_FATAL(!ms_installed, ("RemoteDebugToolStatic not installed"));
	ms_window->send(buffer, bufferLen);
}

//-----------------------------------------------------------------------

void RemoteDebugToolStatic::receive(void *buffer, uint32 bufferLen)
{
	DEBUG_FATAL(!ms_installed, ("RemoteDebugToolStatic not installed"));
//	ms_window.receive(buffer, bufferLen);
	UNREF(buffer);
	UNREF(bufferLen);
}

//-----------------------------------------------------------------------

void RemoteDebugToolStatic::newStream(uint32 streamNumber, const char *newStreamName)
{
	DEBUG_FATAL(!ms_installed, ("RemoteDebugToolStatic not installed"));
	ms_window->newStream(streamNumber, newStreamName);
}

//-----------------------------------------------------------------------

void RemoteDebugToolStatic::messageOnStream(uint32 streamNumber, const char *message)
{
	DEBUG_FATAL(!ms_installed, ("RemoteDebugToolStatic not installed"));
	ms_window->messageOnStream(streamNumber, message);
}

//-----------------------------------------------------------------------

void RemoteDebugToolStatic::newVariable(uint32 variableNumber, const char *newVariableName)
{
	DEBUG_FATAL(!ms_installed, ("RemoteDebugToolStatic not installed"));
	ms_window->newVariable(variableNumber, newVariableName);
}

//-----------------------------------------------------------------------

void RemoteDebugToolStatic::variableType(uint32 variableNumber, RemoteDebug::VARIABLE_TYPES type)
{
	DEBUG_FATAL(!ms_installed, ("RemoteDebugToolStatic not installed"));
	ms_window->variableType(variableNumber, type);
}

//-----------------------------------------------------------------------

void RemoteDebugToolStatic::variableValue(uint32 variableNumber, const char *value)
{
	DEBUG_FATAL(!ms_installed, ("RemoteDebugToolStatic not installed"));
	ms_window->variableValue(variableNumber, value);
}

//-----------------------------------------------------------------------

void RemoteDebugToolStatic::newStaticView(uint32 staticViewNumber, const char *newStaticViewName)
{
	DEBUG_FATAL(!ms_installed, ("RemoteDebugToolStatic not installed"));
	ms_window->newStaticView(staticViewNumber, newStaticViewName);
}

//-----------------------------------------------------------------------

void RemoteDebugToolStatic::messageOnStaticView(uint32 staticViewNumber, const char *message)
{
	DEBUG_FATAL(!ms_installed, ("RemoteDebugToolStatic not installed"));
	ms_window->messageOnStaticView(staticViewNumber, message);
}

//-----------------------------------------------------------------------

void RemoteDebugToolStatic::beginFrame(uint32 staticViewNumber)
{
	DEBUG_FATAL(!ms_installed, ("RemoteDebugToolStatic not installed"));
	ms_window->beginFrame(staticViewNumber);
}

//-----------------------------------------------------------------------

void RemoteDebugToolStatic::endFrame(uint32 staticViewNumber)
{
	DEBUG_FATAL(!ms_installed, ("RemoteDebugToolStatic not installed"));
	ms_window->endFrame(staticViewNumber);
}

//-----------------------------------------------------------------------

void RemoteDebugToolStatic::isReady()
{
	ms_window->isReady();
}

//-----------------------------------------------------------------------

void RemoteDebugToolStatic::connectionClosed()
{
	ms_window->connectionClosed();
}

//-----------------------------------------------------------------------

void RemoteDebugToolStatic::connectionCreated(RemoteDebugToolConnection* c)
{
	ms_window->setTheConnection(c);
}

//-----------------------------------------------------------------------
