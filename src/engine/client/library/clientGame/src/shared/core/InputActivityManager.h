// ============================================================================
// 
// InputActivityManager.h
// Copyright Sony Online Entertainment, Inc.
//
// ============================================================================

#ifndef INCLUDED_InputActivityManager_H
#define INCLUDED_InputActivityManager_H

//-----------------------------------------------------------------------------
class InputActivityManager
{
public:
	static void install();
	static void alter(float const deltaTime);
	static bool setInactive(bool inactive);

private:

	static void remove();

	// Disable
	InputActivityManager();
	~InputActivityManager();
	InputActivityManager(InputActivityManager const &);
	InputActivityManager &operator =(InputActivityManager const &);
};

// ============================================================================

#endif // INCLUDED_InputActivityManager_H
