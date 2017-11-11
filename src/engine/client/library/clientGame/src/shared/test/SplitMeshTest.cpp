//===================================================================
//
// SplitMeshTest.cpp
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/SplitMeshTest.h"

#include "clientGraphics/DebugPrimitive.h"
#include "clientObject/ObjectListCamera.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/RenderWorld.h"
#include "clientGraphics/Shader.h"
#include "clientGraphics/ShaderPrimitive.h"
#include "clientGraphics/ShaderPrimitiveSet.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/StaticIndexBuffer.h"
#include "clientGraphics/StaticShader.h"
#include "clientGraphics/StaticVertexBuffer.h"
#include "clientObject/DetailAppearance.h"
#include "clientObject/MeshAppearance.h"
#include "dpvsModel.hpp"
#include "dpvsObject.hpp"
#include "sharedCollision/Extent.h"
#include "sharedCollision/ExtentList.h"
#include "sharedDebug/PerformanceTimer.h"
#include "sharedFile/AsynchronousLoader.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedMath/Plane.h"
#include "sharedMath/Sphere.h"
#include "sharedMath/Triangle3d.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/Object.h"
#include "sharedRandom/Random.h"
#include <algorithm>
#include <vector>
#include "clientObject/DynamicMeshAppearanceShaderSet.h"
#include "clientObject/DynamicMeshAppearance.h"

//----------------------------------------------------------------------

namespace SplitMeshTestNamespace
{
	bool s_installed = false;
	
	std::string s_appearanceName;
	Vector s_planePoint;
	Vector s_planeNormal;
	Vector s_planeYawPitchRoll;
	Plane s_plane;
	Vector s_randomization;
	bool s_randomizationEnable;
	bool s_showBackObject = false;
	bool s_showFrontObject = true;

	float s_seperation = 2.0f;

	//----------------------------------------------------------------------

	void install()
	{
		if (s_installed)
			return;
		
		s_installed = true;
		
		s_appearanceName = ConfigFile::getKeyString("ClientGame", "SplitMeshTestAppearanceName", "appearance/xwing_body.apt");
		s_planePoint.x = ConfigFile::getKeyFloat("ClientGame", "SplitMeshTestPlanePointX", 0.0f);
		s_planePoint.y = ConfigFile::getKeyFloat("ClientGame", "SplitMeshTestPlanePointY", 0.0f);
		s_planePoint.z = ConfigFile::getKeyFloat("ClientGame", "SplitMeshTestPlanePointZ", 0.0f);
		s_planeNormal.x = ConfigFile::getKeyFloat("ClientGame", "SplitMeshTestPlaneNormalX", 0.0f);
		s_planeNormal.y = ConfigFile::getKeyFloat("ClientGame", "SplitMeshTestPlaneNormalY", 0.0f);
		s_planeNormal.z = ConfigFile::getKeyFloat("ClientGame", "SplitMeshTestPlaneNormalZ", 1.0f);
		s_randomization.x = ConfigFile::getKeyFloat("ClientGame", "SplitMeshTestRandomizationX", 0.0f);
		s_randomization.y = ConfigFile::getKeyFloat("ClientGame", "SplitMeshTestRandomizationY", 0.0f);
		s_randomization.z = ConfigFile::getKeyFloat("ClientGame", "SplitMeshTestRandomizationZ", 0.0f);
		s_randomizationEnable = ConfigFile::getKeyBool("ClientGame", "SplitMeshTestRandomizationEnable", 0.0f);
		s_planeYawPitchRoll.x = ConfigFile::getKeyFloat("ClientGame", "SplitMeshTestRandomizationYaw", 0.0f);
		s_planeYawPitchRoll.y = ConfigFile::getKeyFloat("ClientGame", "SplitMeshTestRandomizationPitch", 0.0f);
		s_planeYawPitchRoll.z = ConfigFile::getKeyFloat("ClientGame", "SplitMeshTestRandomizationRoll", 0.0f);
		s_plane = Plane(s_planeNormal, s_planePoint);
	}
}

using namespace SplitMeshTestNamespace;

//===================================================================

SplitMeshTest::SplitMeshTest () : 
TestIoWin ("SplitMeshTest"),
m_objectRadius(0.0f),
m_objectBack(new ObjectWatcher)
{
	if (!s_installed)
		install();
}

//-------------------------------------------------------------------

SplitMeshTest::~SplitMeshTest ()
{
	delete m_objectBack;
	*m_objectBack = NULL;
}

//-------------------------------------------------------------------

Object * SplitMeshTest::createObjectAndAppearance ()
{
	Transform t = Transform::identity;

	t.yaw_l(s_planeYawPitchRoll.x);
	t.pitch_l(s_planeYawPitchRoll.y);
	t.roll_l(s_planeYawPitchRoll.z);
	
	s_planeNormal = t.rotate_l2p(Vector::unitZ);

	s_plane = Plane(s_planeNormal, s_planePoint);

	bool const asynchronousLoaderEnabled = AsynchronousLoader::isEnabled();
	AsynchronousLoader::disable();
	
	Appearance * const appearance = AppearanceTemplateList::createAppearance(s_appearanceName.c_str());
	
	if (asynchronousLoaderEnabled)
		AsynchronousLoader::enable();
	
	if (appearance == NULL)
		return NULL;
	
	Vector const & randomization = s_randomizationEnable ? s_randomization : Vector::zero;
	
	DynamicMeshAppearance * appearanceFront = NULL;
	DynamicMeshAppearance * appearanceBack = NULL;

	DynamicMeshAppearance::splitAppearance(*appearance, s_plane, randomization, appearanceFront, appearanceBack, -1.0f);
	
	Object * const objFront = (appearanceFront != NULL) ? new Object : NULL;
	Object * const objBack = (appearanceBack != NULL) ? new Object : NULL;

	if (objFront != NULL)
		objFront->setAppearance(appearanceFront);
	if (objBack != NULL)
	{
		objBack->setAppearance(appearanceBack);
		Vector const & seperationVector = s_planeNormal * (-s_seperation);
		objBack->move_o(seperationVector);
	}

	//-- cleanup old 'back' object
	if (m_objectBack->getPointer())
		m_objectList->removeObject(m_objectBack->getPointer());
	*m_objectBack = NULL;

	Object * const objs[2] =
	{
		(s_showFrontObject && objFront != NULL) ? objFront : (s_showBackObject ? objBack : objFront),
		(s_showBackObject && s_showFrontObject) ? objBack : NULL
	};

	if (objs[1] != NULL)
	{
		m_objectList->addObject(objs[1]);
		*m_objectBack = objs[1];
	}

	delete appearance;
	
	NOT_NULL(objs[0]);

	return objs[0];
}

//----------------------------------------------------------------------

IoResult SplitMeshTest::processEvent (IoEvent* event)
{
	NOT_NULL(event);
	if (!isInitialized ())
		initialize ();

	switch (event->type)
	{
		case IOET_Character:
		{
			if (tolower(event->arg2) == 's')
			{
				if (m_keys[K_down])
				{
					s_seperation *= 0.75f;
					reload();
					return IOR_Block;
				}
				else if (m_keys[K_up])
				{
					s_seperation *= 1.25f;
					reload();
					return IOR_Block;
				}
			}

			if (tolower (event->arg2) == 'p')
			{
				s_planeYawPitchRoll = Vector::zero;
				reload();
				return IOR_Block;
			}
			if (event->arg2 == 'b')
			{
				s_showBackObject = !s_showBackObject;
				reload();
				return IOR_Block;
			}
			else if (event->arg2 == 'B')
			{
				s_showFrontObject = !s_showFrontObject;
				reload();
				return IOR_Block;
			}
			if (tolower (event->arg2) == 'k')
			{
				if (m_keys[K_up])
				{
					s_planePoint.z += 0.05f;
					reload();
					return IOR_Block;
				}

				else if (m_keys[K_down])
				{
					s_planePoint.z -= 0.05f;
					reload();
					return IOR_Block;
				}
			}
			else if (tolower (event->arg2) == 'i')
			{
				if (m_keys[K_up])
				{
					s_planePoint.x += 0.05f;
					reload();
					return IOR_Block;
				}

				else if (m_keys[K_down])
				{
					s_planePoint.x -= 0.05f;
					reload();
					return IOR_Block;
				}

				if (m_keys[K_left])
				{
					s_planeYawPitchRoll.y = clamp(-PI_OVER_2, s_planeYawPitchRoll.y + 0.05f, PI_OVER_2);
					reload();
					return IOR_Block;
				}

				else if (m_keys[K_right])
				{
					s_planeYawPitchRoll.y = clamp(-PI_OVER_2, s_planeYawPitchRoll.y - 0.05f, PI_OVER_2);
					reload();
					return IOR_Block;
				}
			}
			else if (tolower (event->arg2) == 'j')
			{
				if (m_keys[K_up])
				{
					s_planePoint.y += 0.05f;
					reload();
					return IOR_Block;
				}

				else if (m_keys[K_down])
				{
					s_planePoint.y -= 0.05f;
					reload();
					return IOR_Block;
				}

				if (m_keys[K_left])
				{
					s_planeYawPitchRoll.x = clamp(-PI_OVER_2, s_planeYawPitchRoll.x + 0.05f, PI_OVER_2);
					reload();
					return IOR_Block;
				}

				else if (m_keys[K_right])
				{
					s_planeYawPitchRoll.x = clamp(-PI_OVER_2, s_planeYawPitchRoll.x - 0.05f, PI_OVER_2);
					reload();
					return IOR_Block;
				}
			}

			else if (tolower(event->arg2) == 'n')
			{
				s_randomizationEnable = !s_randomizationEnable;
				reload();
				return IOR_Block;
			}
		break;
		}
	}

	return TestIoWin::processEvent(event);

}

//----------------------------------------------------------------------

void SplitMeshTest::draw () const
{
	//-- render splitter

	Graphics::setStaticShader (ShaderTemplateList::get3dVertexColorZStaticShader ());
	Transform t;
	t.setLocalFrameKJ_p(s_planeNormal, Vector::unitY);
	t.setPosition_p(s_planePoint);
	Graphics::setObjectToWorldTransformAndScale (t, Vector::xyz111);
	Graphics::drawFrame ();

	TestIoWin::draw();
}

//===================================================================
