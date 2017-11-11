// ============================================================================
// 
// AwayFromKeyBoardManager.h
// Copyright Sony Online Entertainment, Inc.
//
// ============================================================================

#ifndef INCLUDED_AwayFromKeyBoardManager_H
#define INCLUDED_AwayFromKeyBoardManager_H

//-----------------------------------------------------------------------------
class AwayFromKeyBoardManager
{
public:

	static void install();
	static void alter(float const deltaTime);
	static void touch();
	static void toggleAwayFromKeyBoard();
	static bool isAwayFromKeyBoard();

	static void setAutomaticAwayFromKeyBoardMinutes(int const automaticAwayFromKeyBoardMinutes);
	static int  getAutomaticAwayFromKeyBoardMinutes();
	static int  getAutomaticAwayFromKeyBoardMinutesDefault();
	static int  getAutomaticAwayFromKeyBoardMinutesMin();
	static int  getAutomaticAwayFromKeyBoardMinutesMax();

	static bool isAutoAwayFromKeyBoardEnabled();
	static bool isAutoAwayFromKeyBoardEnabledDefault();
	static void setAutoAwayFromKeyBoardEnabled(bool enabled);

	static void                   setAutomaticResponseMessage(Unicode::String const &message);
	static Unicode::String const &getAutomaticResponseMessage();
	static Unicode::String        getAutomaticResponseMessageDefault();
	static Unicode::String        getAutomaticResponseMessagePrefix();

	static void resetAwayFromKeyBoard();

private:

	static void remove();

	// Disable

	AwayFromKeyBoardManager();
	~AwayFromKeyBoardManager();
	AwayFromKeyBoardManager(AwayFromKeyBoardManager const &);
	AwayFromKeyBoardManager &operator =(AwayFromKeyBoardManager const &);
};

// ============================================================================

#endif // INCLUDED_AwayFromKeyBoardManager_H
