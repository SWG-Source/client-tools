// ======================================================================
// 
// GameMusicManager.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_GameMusicManager_H
#define INCLUDED_GameMusicManager_H

// ======================================================================

class Object;
class NetworkId;

// ======================================================================

class GameMusicManager
{
public:

	static void install ( const char* buildoutAreaName );
	static void remove ();

	static void setReferenceObject (const Object* referenceObject);
	static void setMusicTrackOverride(char const * soundTemplateName);
	static void cancelMusicTrackOverride();
	static void setSoundTrackOverride(char const * soundTemplateName);
	static void update (float elapsedTime);
	static void startCombatMusic(NetworkId const & attacker);

private:

	GameMusicManager ();
	~GameMusicManager ();
	GameMusicManager (const GameMusicManager&);
	GameMusicManager& operator= (const GameMusicManager&);
};

// ======================================================================

#endif
