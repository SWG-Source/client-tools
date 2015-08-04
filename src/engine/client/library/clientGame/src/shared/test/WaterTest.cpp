//===================================================================
//
// WaterTest.cpp
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/WaterTest.h"

#include "clientGame/WaterTestAppearance.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "sharedObject/Object.h"

//===================================================================

WaterTest::WaterTest () : 
	TestIoWin ("WaterTest")
{
}

//-------------------------------------------------------------------

WaterTest::~WaterTest ()
{
}

//-------------------------------------------------------------------

void WaterTest::createFloor ()
{
}

//-------------------------------------------------------------------

Object* WaterTest::createObjectAndAppearance ()
{
	Object* const object = new Object ();
	Shader* const shader = ShaderTemplateList::fetchModifiableShader ("shader/water_test.sht");
	object->setAppearance (new WaterTestAppearance (10.f, 10.f, shader, 5.f));

	return object;
}

//===================================================================
