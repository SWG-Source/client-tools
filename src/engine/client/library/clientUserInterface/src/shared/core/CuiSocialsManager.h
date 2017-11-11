// ======================================================================
//
// CuiSocialsManager.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CuiSocialsManager_H
#define INCLUDED_CuiSocialsManager_H

// ====================================================================== 

class ClientObject;
class LocalizedStringTable;
class MessageQueueSocial;

class NetworkId;

//----------------------------------------------------------------------

class CuiSocialsManager
{
public:

	struct Messages
	{
		struct SocialReceived
		{
			typedef Unicode::String Payload;
		};
	};

	static void                         install ();
	static void                         remove ();

	static void                         processMessage (const MessageQueueSocial & cst);
	static void                         sendMessage    (const NetworkId & targetId, uint32 socialType, bool animationOk = true, bool textOk = true);
	static void                         sendMessage    (const NetworkId & sourceId, const NetworkId & targetId, uint32 socialType, bool animationOk = true, bool textOk = true);

	static void                         processWhisperMessage (const NetworkId &sourceId, const NetworkId & targetId, bool printMessage);

	static uint32                       getWhisperSocialId ();

	static uint32                       getSocialIdForCommandName (const std::string & name);

	static const LocalizedStringTable * getSocialsTable ();

	static void                         listSocials (const Unicode::String & abbrev, Unicode::String & result);

	static void                         testSocials (const NetworkId & target, Unicode::String & result);

private:

	static LocalizedStringTable * ms_socialsTable;

private:
	CuiSocialsManager ();
	CuiSocialsManager (const CuiSocialsManager & rhs);
	CuiSocialsManager & operator= (const CuiSocialsManager & rhs);
	
	static uint32            ms_whisperSocialId;
};

//----------------------------------------------------------------------

inline uint32 CuiSocialsManager::getWhisperSocialId ()
{
	return ms_whisperSocialId;
}

//----------------------------------------------------------------------

inline const LocalizedStringTable * CuiSocialsManager::getSocialsTable ()
{
	return ms_socialsTable;
}

// ======================================================================

#endif
