//===================================================================
//
// LodShaderTest.cpp
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/LodShaderTest.h"

#include "clientGame/ConfigClientGame.h"
#include "clientGraphics/Graphics.h"
#include "clientObject/ObjectListCamera.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/Object.h"
#include "sharedObject/ObjectList.h"
#include "sharedRandom/RandomGenerator.h"

//===================================================================

LodShaderTest::LodShaderTest () : 
	TestIoWin ("LodShaderTest")
{
}

//-------------------------------------------------------------------

LodShaderTest::~LodShaderTest ()
{
}

//===================================================================
// PROTECTED LodShaderTest
//===================================================================

void LodShaderTest::initialize ()
{
	TestIoWin::initialize ();

	RandomGenerator randomGenerator (99);

	//-- add the objects
	const int size   = 20;
	const int size_2 = size / 2;
	int x;
	int z;
	for (z = -size_2; z <= size_2; ++z)
		for (x = -size_2; x <= size_2; ++x)
		{
			Object* const object = new Object ();
			object->setAppearance (AppearanceTemplateList::createAppearance (ConfigClientGame::getTestObject1 ()));

			const float radius = object->getAppearanceSphereRadius ();
			object->setPosition_p (Vector (x * radius, randomGenerator.randomReal (-radius, radius), z * radius));
			object->yaw_o (randomGenerator.randomReal (radius));

			m_objectList->addObject (object);
		}

	//-- move camera a bit farther out
	m_cameraPitch  = 0.f;
	m_cameraZoom  *= 3.f;

	//-- fog camera
	Graphics::setFog (true, 0.0017f, PackedArgb (255, 153, 147, 132));
}

//-------------------------------------------------------------------

void LodShaderTest::createFloor ()
{
}

//===================================================================

