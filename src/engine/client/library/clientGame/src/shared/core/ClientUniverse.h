//===================================================================
//
// ClientUniverse.h
//
// copyright 2002 Sony Online Entertainment
//
//===================================================================

#ifndef INCLUDED_ClientUniverse_H
#define INCLUDED_ClientUniverse_H

//===================================================================

#include "sharedGame/Universe.h"

//===================================================================

class ClientUniverse : public Universe
{
public:
	static void install();
	
	virtual void                  resourceClassTreeLoaded     (stdmap<int, ResourceClassObject*>::fwd &resourceClasses);
	virtual ResourceClassObject * makeResourceClassObject     ();
	
public:
	static void setupResourceTree();

private:
	ClientUniverse();
	virtual ~ClientUniverse();
};

//===================================================================

#endif

