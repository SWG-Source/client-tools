//======================================================================
//
// ClientBuffManager.h
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ClientBuffManager_H
#define INCLUDED_ClientBuffManager_H

//======================================================================

class UIImage;
class UIImageStyle;
class Buff;

//----------------------------------------------------------------------

class ClientBuffManager
{
public:
	static void install();
	static void remove();

	static int  getBuffState(uint32 buffNameCrc);
	static float getBuffDefaultDuration(uint32 buffNameCrc);
	static bool getBuffIsDebuff(uint32 buffNameCrc);
	static bool getBuffIsGroupVisible(uint32 buffNameCrc);
	static bool getBuffGroupAndPriority(uint32 buffNameCrc, uint32 & group1Crc, uint32 & group2Crc, int & priority);
	static int  getBuffMaxStacks(uint32 buffNameCrc);
	static bool getBuffIsCelestial(uint32 buffNameCrc);
	static bool getBuffIsDispellable(uint32 buffNameCrc);
	static int  getBuffDisplayOrder(uint32 buffNameCrc);

	static UIImageStyle * getBuffIconStyle(uint32 buffNameCrc);
	static void getBuffDescription(Buff const & buff, Unicode::String & result);
	static void addTimestampToBuffDescription(Unicode::String const & description, int timeLeft, Unicode::String & result);

protected:
	
};


//======================================================================

#endif
