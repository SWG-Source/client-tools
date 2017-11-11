//======================================================================
//
// ClientDataFile_HardpointChildObject.h
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ClientDataFile_HardpointChildObject_H
#define INCLUDED_ClientDataFile_HardpointChildObject_H

//======================================================================

#include "clientGame/ClientDataFile.h"

class AppearanceTemplate;

//----------------------------------------------------------------------

struct ClientDataFile::HardpointChildObject
{
public:

	std::string m_appearanceTemplateName;
	std::string m_hardpointName;
	
public:
	
	const AppearanceTemplate* m_appearanceTemplate;
	
public:
	
	HardpointChildObject ();
	void load (Iff & iff);	
	~HardpointChildObject ();
};

//======================================================================

#endif
