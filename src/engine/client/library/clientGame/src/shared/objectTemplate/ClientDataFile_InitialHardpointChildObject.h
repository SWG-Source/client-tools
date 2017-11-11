// ======================================================================
//
// ClientDataFile_InitialHardpointChildObject.h
//
// Copyright 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ClientDataFile_InitialHardpointChildObject_H
#define INCLUDED_ClientDataFile_InitialHardpointChildObject_H

// ======================================================================

#include "clientGame/ClientDataFile.h"

// ======================================================================

class AppearanceTemplate;

// ======================================================================

struct ClientDataFile::InitialHardpointChildObject
{
public:
	std::string m_appearanceTemplateName;
	std::string m_hardpointName;
	float m_recoilDistance;
	float m_recoilRate;
	float m_recoilReturnRate;
	AppearanceTemplate const *m_appearanceTemplate;
	
public:	
	InitialHardpointChildObject();
	~InitialHardpointChildObject();

	void load(Iff &iff);	
};

//======================================================================

#endif
