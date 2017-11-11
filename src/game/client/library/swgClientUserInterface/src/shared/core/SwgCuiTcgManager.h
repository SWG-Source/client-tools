// ======================================================================
//
// SwgCuiTcgManager.h
// copyright (c) 2008 Sony Online Entertainment LLC
//
// ======================================================================

#ifndef INCLUDED_SwgCuiTcgManager_H
#define INCLUDED_SwgCuiTcgManager_H

// ======================================================================

class SwgCuiTcgManager
{
public:
	static void install();
	static void remove();
	static void update(float deltaTimeSecs);
	static void launch();
	static void setLoginInfo(char const * const username, char const * const sessionId);

private: //disabled
	SwgCuiTcgManager();
	SwgCuiTcgManager(const SwgCuiTcgManager &rhs);
	SwgCuiTcgManager& operator= (const SwgCuiTcgManager &rhs);

private:
};

// ======================================================================

#endif
