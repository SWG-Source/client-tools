//===================================================================
//
// SetTextureTest.cpp
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/SetTextureTest.h"

#include "clientGame/ConfigClientGame.h"
#include "clientGraphics/Texture.h"
#include "clientGraphics/TextureList.h"
#include "clientObject/ObjectListCamera.h"
#include "sharedFoundation/Timer.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/Object.h"
#include "sharedObject/ObjectList.h"
#include "sharedRandom/Random.h"

#include <algorithm>
#include <vector>

//===================================================================

class TextureRelease
{
public:

	void operator() (const Texture* const texture) const
	{
		texture->release ();
	}
};

//===================================================================

SetTextureTest::SetTextureTest () : 
	TestIoWin ("SetTextureTest"),
	m_textureList(new TextureList2)
{
}

//-------------------------------------------------------------------

SetTextureTest::~SetTextureTest ()
{
	std::for_each (m_textureList->begin (), m_textureList->end (), TextureRelease ());
	delete m_textureList;
}

//-------------------------------------------------------------------

IoResult SetTextureTest::processEvent (IoEvent* const event)
{
	switch (event->type)
	{
	case IOET_Update:
		{
			static Timer timer (1.f);

			const float elapsedTime = event->arg3;

			if (timer.updateZero (elapsedTime))
			{
				const uint textureChoice = static_cast<uint> (Random::random (0, static_cast<int> (m_textureList->size () - 1)));
				const Texture* const texture = (*m_textureList) [textureChoice];
				m_object->getAppearance ()->setTexture (TAG (M,A,I,N), *texture);
			}
		}
		break;
	}

	return TestIoWin::processEvent (event);
}

//===================================================================
// PROTECTED SetTextureTest
//===================================================================

void SetTextureTest::initialize ()
{
	TestIoWin::initialize ();

	m_textureList->push_back (TextureList::fetch ("texture/env_naboo.dds"));
	m_textureList->push_back (TextureList::fetch ("texture/roadtest.dds"));
	m_textureList->push_back (TextureList::fetch ("texture/swglogo.dds"));
}

//-------------------------------------------------------------------

void SetTextureTest::createFloor ()
{
}

//===================================================================

