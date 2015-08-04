// ============================================================================
//
// TemplateEditorPerforce.h
// Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_TemplateEditorPerforce_H
#define INCLUDED_TemplateEditorPerforce_H

#include "Singleton/Singleton.h"
#include <map>
#include <string>

class AbstractFilesystemTree;

//-----------------------------------------------------------------------------
class TemplateEditorPerforce : public Singleton<TemplateEditorPerforce>
{
public:

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

		// Disabled

		CommandMessage();
		CommandMessage& operator=(const CommandMessage& rhs);
		CommandMessage(const CommandMessage& rhs);

	private:

		std::string m_msg;
	};

	enum FileState
	{
		FS_depot,
		FS_add,
		FS_edit,
		FS_delete,
		FS_not
	};

	typedef std::vector<std::string>              StringVector;
	typedef std::map<std::string, enum FileState> OpenFileMap;

public:

	TemplateEditorPerforce();

	static const std::string  concatenateSubpath(const std::string& root, const std::string& subPath);

	bool addFiles(const StringVector& filenames) const;
	bool editFiles(const StringVector& filenames) const;
	void getOpenedFiles(OpenFileMap & target) const;
	bool getFileMapping(const std::string& path, std::string& depot, std::string& client, std::string& local) const;

private:

	// Disabled

	TemplateEditorPerforce(TemplateEditorPerforce const &);
	TemplateEditorPerforce& operator=(TemplateEditorPerforce const &);
};

// ======================================================================

#endif // INCLUDED_TemplateEditorPerforce_H
