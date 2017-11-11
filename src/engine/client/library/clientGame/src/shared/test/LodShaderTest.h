//===================================================================
//
// LodShaderTest.h
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_LodShaderTest_H
#define INCLUDED_LodShaderTest_H

//===================================================================

#include "clientGame/TestIoWin.h"

//===================================================================

class LodShaderTest : public TestIoWin
{
public:

	LodShaderTest ();
	virtual ~LodShaderTest ();

	virtual void    initialize ();
	virtual void    createFloor ();

private:

	LodShaderTest (const LodShaderTest&);
	LodShaderTest& operator= (const LodShaderTest&);
};

//===================================================================

#endif
