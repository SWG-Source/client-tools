// ============================================================================
//
// SetupSwgClientUserInterface.h
// copyright (c) 2004 Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_SetupSwgClientUserInterface_H
#define INCLUDED_SetupSwgClientUserInterface_H

// ============================================================================

class SetupSwgClientUserInterface
{
public:

	static void install();

private:

	static void remove();

	// Disabled

	SetupSwgClientUserInterface();
	~SetupSwgClientUserInterface();
	SetupSwgClientUserInterface(const SetupSwgClientUserInterface &);
	SetupSwgClientUserInterface &operator =(const SetupSwgClientUserInterface &);
};

// ============================================================================

#endif // INCLUDED_SetupSwgClientUserInterface_H
