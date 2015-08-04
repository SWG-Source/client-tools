
// ============================================================================
//
// ConfigSwgClientUserInterface.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_ConfigSwgClientUserInterface_H
#define INCLUDED_ConfigSwgClientUserInterface_H

//-----------------------------------------------------------------------------
class ConfigSwgClientUserInterface
{
public:

	static void  install();
	static void  remove();

public:

private:

	// Disabled

	ConfigSwgClientUserInterface();
	~ConfigSwgClientUserInterface();
	ConfigSwgClientUserInterface(const ConfigSwgClientUserInterface &);
	ConfigSwgClientUserInterface &operator =(const ConfigSwgClientUserInterface &);
};

// ============================================================================

#endif // INCLUDED_ConfigSwgClientUserInterface_H
