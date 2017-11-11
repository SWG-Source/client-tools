// ======================================================================
//
// RemoteDebugToolStatic.h
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef RemoteDebugToolStatic_H
#define RemoteDebugToolStatic_H

//-----------------------------------------------------------------------

class ApplicationWindow;
class RemoteDebugToolConnection;

//-----------------------------------------------------------------------

class RemoteDebugToolStatic
{
public:
	static void install(ApplicationWindow* window);
	static void remove();
	static void open(const char *server, uint16 port);
	static void close();
	static void send(void *buffer, uint32 bufferLen);
	static void receive(void *buffer, uint32 bufferLen);
	static void newStream(uint32 streamNumber, const char *newStreamName);
	static void messageOnStream(uint32 streamNumber, const char *message);
	static void newVariable(uint32 variableNumber, const char *newVariableName);
	static void variableType(uint32 variableNumber, RemoteDebug::VARIABLE_TYPES type);
	static void variableValue(uint32 variableNumber, const char *value);
	static void newStaticView(uint32 staticViewNumber, const char *newStaticViewName);
	static void messageOnStaticView(uint32 staticViewNumber, const char *message);
	static void beginFrame(uint32 staticViewNumber);
	static void endFrame(uint32 staticViewNumber);
	static void isReady();
	static void connectionClosed();
	static void connectionCreated(RemoteDebugToolConnection* c);

private:
	///true if system has been installed, false otherwise
	static bool                ms_installed;

	static ApplicationWindow*  ms_window;
};

//-----------------------------------------------------------------------

#endif

