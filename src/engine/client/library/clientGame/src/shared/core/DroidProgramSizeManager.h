// ============================================================================
// 
// DroidProgramSizeManager.h
// Copyright Sony Online Entertainment, Inc.
//
// ============================================================================

#ifndef INCLUDED_DroidProgramSizeManager_H
#define INCLUDED_DroidProgramSizeManager_H

//-----------------------------------------------------------------------------

class ClientObject;

//-----------------------------------------------------------------------------

class DroidProgramSizeManager
{
public:
	static void install();
	static int getDroidProgramSize(std::string const & droidProgramCommandName);
	static bool isDroidProgrammable(ClientObject const & droidControlDevice);

private:
	static void remove();

private:
	//disabled
	DroidProgramSizeManager();
	~DroidProgramSizeManager();
	DroidProgramSizeManager(DroidProgramSizeManager const &);
	DroidProgramSizeManager &operator =(DroidProgramSizeManager const &);
};

// ============================================================================

#endif // INCLUDED_DroidProgramSizeManager_H
