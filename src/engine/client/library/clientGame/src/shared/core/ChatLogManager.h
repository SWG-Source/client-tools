// ============================================================================
// 
// ChatLogManager.h
// Copyright Sony Online Entertainment, Inc.
//
// ============================================================================

#ifndef INCLUDED_ChatLogManager_H
#define INCLUDED_ChatLogManager_H

//-----------------------------------------------------------------------------
class ChatLogManager
{
public:

	static void install();

public:

	static void setEnabled(bool const enabled);
	static bool isEnabled();
	static void appendLine(Unicode::String const &text);
	static void flush();
	static void showEnabledText();
	static void showDisabledText();
	static void gameStart();

private:

	static void remove();

	// Disable

	ChatLogManager();
	~ChatLogManager();
	ChatLogManager(ChatLogManager const &);
	ChatLogManager &operator =(ChatLogManager const &);
};

// ============================================================================

#endif // INCLUDED_ChatLogManager_H
