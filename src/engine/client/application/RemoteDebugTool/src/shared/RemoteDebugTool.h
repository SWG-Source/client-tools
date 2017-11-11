// ======================================================================
//
// remotedebugtool.h
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef RemoteDebugTool_H
#define RemoteDebugTool_H
//-----------------------------------------------------------------------

class ApplicationWindow;
class RemoteDebugSetup;

//-----------------------------------------------------------------------

/** This class's only purpose is to wrap the Qt application code in a class
  * That can be called from the engine (found in main.cpp).
  */
class remoteDebugTool
{
///allow a friend so that packets can be forwards to and from the channel
friend RemoteDebugSetup;

public:
	/// This function is called from the engine, and starts up the Qt application.
	static void run();

private:
	/** a static instance of the application window, helpful in letting other parts of code
	  * (specificially the VNL Channel and Handler) gain access to pass data on to the app.
	  */
	static ApplicationWindow *m_mainWindow;
};

//-----------------------------------------------------------------------

#endif // RemoteDebugTool_H
