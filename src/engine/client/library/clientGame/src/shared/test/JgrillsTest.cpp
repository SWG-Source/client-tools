//===================================================================
//
// JgrillsTest.cpp
//
// copyright 2002, sony online entertainment
//
//===================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/JgrillsTest.h"

#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/PortalProperty.h"

#include "clientGraphics/Graphics.h"
#include "clientGraphics/GraphicsOptionTags.h"
#include "clientGraphics/Light.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/TextureList.h"
#include "clientObject/MouseCursor.h"
#include "clientObject/ObjectListCamera.h"
#include "clientObject/ShadowVolume.h"
#include "sharedMath/VectorArgb.h"

//===================================================================

#define LIGHT_ATTENUATION       1
#define JABBA_KITCHEN           0
#define VERTEX_SHADER_LIGHTING  0

const Tag TAG_VSPS = TAG(V,S,P,S);

//===================================================================

JgrillsTest::JgrillsTest()
: TestIoWin("JgrillsTest")
{
}

//-------------------------------------------------------------------

JgrillsTest::~JgrillsTest()
{
}

//-------------------------------------------------------------------

IoResult JgrillsTest::processEvent(IoEvent* event)
{
	const IoResult result = TestIoWin::processEvent(event);

#if LIGHT_ATTENUATION
	static bool init = false;
	if (!init)
	{
		init = true;
		m_cameraZoom = m_object->getAppearanceSphereRadius () * 0.75f;
		m_cameraPivot = m_object->getAppearanceSphereCenter ();
		m_cameraPitch = PI;
		m_shouldUpdate = true;
	}
#endif

	switch (event->type)
	{
		case IOET_Character:
			{
#if VERTEX_SHADER_LIGHTING
				Light *newLight = 0;
#endif

				switch (event->arg2)
				{
#if VERTEX_SHADER_LIGHTING
					case 'a':
						newLight = new Light(Light::T_parallel, VectorArgb::solidWhite);
						break;

					case 'A':
						newLight = new Light(Light::T_parallel, VectorArgb::solidGray);
						newLight->setSpecularColor(VectorArgb::solidRed);
						break;

					case 'o':
						newLight = new Light(Light::T_point, VectorArgb::solidWhite);
						break;

					case 'O':
						newLight = new Light(Light::T_point, VectorArgb::solidGray);
						newLight->setSpecularColor(VectorArgb::solidRed);
						break;
#endif

					case 'm':
						DebugHelp::writeMiniDump();
						break;

					case 'c':
						*((int*)0) = 0;
						break;

					case 'v':
						GraphicsOptionTags::toggle(TAG_VSPS);
						reload();
						break;
				}

#if VERTEX_SHADER_LIGHTING
				if (newLight)
				{
					if (m_light)
					{
						m_lightList->removeObject(m_light);
						delete m_light;
					}

					m_light = newLight;
					m_lightList->addObject(m_light);
				}
#endif
			}
			break;

		case IOET_Update:
			{
				DEBUG_REPORT_PRINT(true,("camera theta: %1.2f degrees\n", convertRadiansToDegrees(m_camera->getObjectFrameK_w().theta())));
				DEBUG_REPORT_PRINT(true,("camera phi:   %1.2f degrees\n", convertRadiansToDegrees(m_camera->getObjectFrameK_w().phi())));
				DEBUG_REPORT_PRINT(true,("camera zoom:  %1.2f\n", m_cameraZoom));
				DEBUG_REPORT_PRINT(true,("vsps:         %s\n", GraphicsOptionTags::get(TAG_VSPS) ? "on" : "off"));
				DEBUG_REPORT_PRINT(true,("Q      quit\n"));
			}
			break;
	}

	return result;
}

//-------------------------------------------------------------------

void JgrillsTest::draw() const
{
	const PackedArgb clearColor (255, 161, 161, 161);
	Graphics::setViewport (0, 0, Graphics::getCurrentRenderTargetWidth (), Graphics::getCurrentRenderTargetHeight ());
	Graphics::clearViewport (true, clearColor.getArgb (), true, 1.0f, true, 0);
	Graphics::setStaticShader(ShaderTemplateList::get2dVertexColorStaticShader(), 0);
	Graphics::drawRectangle(100, 100, 300, 300, VectorArgb::solidRed);

//	TestIoWin::draw();
}

//-------------------------------------------------------------------

void JgrillsTest::createFloor ()
{
}

//-------------------------------------------------------------------

void JgrillsTest::createLights()
{
#if LIGHT_ATTENUATION
	Light *ambient = new Light(Light::T_ambient, VectorArgb(0.f, 0.15f, 0.15f, 0.15f));
	ambient->setPosition_p(Vector(0.f, 0.0f, 0.0f));
	m_lightList->addObject(ambient);

	m_light = new Light(Light::T_point, VectorArgb(0.f, 100.0f, 100.0f, 100.0f));
	m_light->setSpecularColor(VectorArgb(1.f, 100.0f, 100.0f, 100.0f));
	m_light->setPosition_p(Vector(0.f, 4.0f, 0.0f));
	m_light->setConstantAttenuation(0.0f);
	m_light->setLinearAttenuation(0.0f);
	m_light->setQuadraticAttenuation(1.0f);
	m_lightList->addObject(m_light);
	m_light = NULL;
#endif
}

//-------------------------------------------------------------------

Object* JgrillsTest::createObjectAndAppearance()
{
#if LIGHT_ATTENUATION || VERTEX_SHADER_LIGHTING
	Object* const object = new Object();
	object->setAppearance(AppearanceTemplateList::createAppearance("appearance/lightgrid.msh"));
#endif

#if JABBA_KITCHEN
	Object* const object = new Object();
	PortalProperty *property = new PortalProperty(*object, "appearance/thm_tato_jabbas_palace.pob");

	const int cell = 12;

	static bool installed = false;
	if (!installed)
	{
		installed = true;

		const int numberOfLights = property->getNumberOfLights(cell);
		for (int i = 0; i < numberOfLights; ++i)
		{
			const PortalProperty::LightData &lightData = property->getLightData(cell, i);

			Light *light = NULL;

			switch (lightData.type)
			{
				case PortalProperty::LightData::T_ambient:
					light = new Light(Light::T_ambient, lightData.diffuseColor);
					break;

				case PortalProperty::LightData::T_parallel:
					light = new Light(Light::T_parallel, lightData.diffuseColor);
					light->setTransform_o2p(lightData.transform);
					light->setSpecularColor(lightData.specularColor);
					break;

				case PortalProperty::LightData::T_point:
					light = new Light(Light::T_point, lightData.diffuseColor);
					light->setTransform_o2p(lightData.transform);
					light->setSpecularColor(lightData.specularColor);
					light->setConstantAttenuation(lightData.constantAttenuation);
					light->setLinearAttenuation(lightData.linearAttenuation);
					light->setQuadraticAttenuation(lightData.quadraticAttenuation);
					break;
			}

			light->setSimulatesPrecalculatedVertexLighting(true);

			m_objectList->addObject(light);
		}
	}

	object->setAppearance(AppearanceTemplateList::createAppearance(property->getCellAppearanceName(cell)));
	delete property;
#endif

	return object;
}

//===================================================================
