// ======================================================================
//
// GodClientPerforce.h
// copyright(c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_GodClientPerforce_H
#define INCLUDED_GodClientPerforce_H

// ======================================================================

#include "Singleton/Singleton.h"
#include <map>
#include <string>

// ======================================================================

class AbstractFilesystemTree;

//-----------------------------------------------------------------
/**
* GodClientPerforce is a singleton which supplies various utility methods for
* interacting with the perforce server.
*
* @todo: add a mechanism for sending requests and retrieving responses from the perforce server asynchronously
* This should include a request and response queues, and a new thread for each request.  The response queue should
* be polled in a threadsafe manner for completed requests.  The GodClientPerforce should handle this polling internally
* and issue a message through the MessageDispatch::Emitter system once responses are completed.  It is vitally important that
* the MessageDispatch::Emitter messages are emitted from the QT EVENT THREAD only.  This requires that you should pump the
* response queue polling mechanism from the QT event thread.  You could do this by making the GodClientPerforce
* itself a QObject and have a pumping slot connected to a timer somewhere, but I wouldnt make GodClientPerforce
* a QObject.  Instead I would have an external object do the pumping indirectly.
*/

class GodClientPerforce : public Singleton<GodClientPerforce>
{
public:

	//----------------------------------------------------------------------

	struct Messages
	{
		static const char* const COMMAND_MESSAGE;
	};

	class CommandMessage : public MessageDispatch::MessageBase
	{
	public:
		explicit CommandMessage(const std::string& msg);
		const std::string& getMessage() const;

	private:
		//disabled
		CommandMessage();
		CommandMessage& operator=(const CommandMessage& rhs);
		CommandMessage(const CommandMessage& rhs);

	private:
		std::string m_msg;
	};

	//----------------------------------------------------------------------


	enum FileState
	{
		FileState_depot,
		FileState_add,
		FileState_edit,
		FileState_delete,
		FileState_not
	};

	typedef std::vector<std::string> StringVector;

	GodClientPerforce();
	static const std::string  concatenateSubpath(const std::string& root, const std::string& subPath);
	bool                      isAuthenticated  (std::string& result, bool attemptLogin = true) const;
	AbstractFilesystemTree*   getFileTree      (const std::string& path, const char* extension, std::string& result, FileState state = FileState_depot) const;
	bool                      getFileMapping   (const std::string& path, std::string& depot, std::string& client, std::string& local, std::string& result) const;
	void                      getOpenedFiles   (std::map<std::string, enum FileState> & target, std::string& result) const;
	bool                      addFiles         (const StringVector& filenames, std::string& result) const;
	bool                      editFiles        (const StringVector& filenames, std::string& result) const;
	bool                      editFilesAndLock (const StringVector& filenames, std::string& result) const;
	bool                      revertFiles      (const StringVector& filenames, bool unchanged, std::string& result) const;
	bool                      submitFiles      (const StringVector& filenames, std::string& result) const;
	bool                      fileAlsoOpenedBy (std::string const & path, StringVector & alsoOpenedBy, std::string & result);

private:
	//disabled
	GodClientPerforce(const GodClientPerforce& rhs);
	GodClientPerforce& operator=(const GodClientPerforce& rhs);

};

// ======================================================================

#endif
