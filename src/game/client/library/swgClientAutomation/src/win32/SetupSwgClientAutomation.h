// ============================================================================
//
// SetupSwgClientAutomation.h
// copyright (c) 2005 Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_SetupSwgClientAutomation_H
#define INCLUDED_SetupSwgClientAutomation_H


// ============================================================================

class SetupSwgClientAutomation
{
public:
	
	static void install();

private:

	static void remove();
	
	SetupSwgClientAutomation();
	~SetupSwgClientAutomation();
	SetupSwgClientAutomation(const SetupSwgClientAutomation &);
	SetupSwgClientAutomation &operator =(const SetupSwgClientAutomation &);
	
};
// ============================================================================


#endif // INCLUDED_SetupSwgClientAutomation_H