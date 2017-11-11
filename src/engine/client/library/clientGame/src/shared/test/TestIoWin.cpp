//===================================================================
//
// TestIoWin.cpp
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/TestIoWin.h"

#include "clientGame/ConfigClientGame.h"
#include "clientGame/Game.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/GraphicsOptionTags.h"
#include "clientGraphics/Light.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientObject/MouseCursor.h"
#include "clientObject/ObjectListCamera.h"
#include "clientObject/ShaderAppearance.h"
#include "clientTerrain/StarAppearance.h"
#include "sharedCollision/BoxExtent.h"
#include "sharedMath/Rectangle2d.h"
#include "sharedMath/VectorArgb.h"
#include "sharedObject/AppearanceTemplateList.h"

//===================================================================

const Tag TAG_DOT3 = TAG(D,O,T,3);

//===================================================================

TestIoWin::TestIoWin (const char* const name) : 
	IoWin (name ? name : "TestIoWin"),
	m_initialized (false),
	m_paused (true),
	m_mouseCursor (0),
	m_camera (0),
	m_lightList (0),
	m_objectList (0),
	m_cameraYaw (0.f),
	m_cameraPitch (PI_OVER_4),
	m_cameraZoom (30.f),
	m_cameraPivot (),
	m_lightYaw (PI_OVER_2),
	m_lightPitch (PI_OVER_4),
	m_lightZoom (20.f),
	m_lightPivot (),
	m_object (0),
	m_light (0),
	m_button0 (false),
	m_button1 (false),
	m_button2 (false),
	m_shouldUpdate (true),
	m_first(false),
	m_wire(false)
{
	int i;
	for (i = 0; i < K_count; ++i)
		m_keys [i] = 0;
}

//-------------------------------------------------------------------

TestIoWin::~TestIoWin ()
{
	delete m_mouseCursor;
	m_mouseCursor = 0;

	delete m_camera;
	m_camera = 0;

	m_lightList->removeAll(true);
	delete m_lightList;
	m_lightList = 0;

	m_objectList->removeAll(true);
	delete m_objectList;
	m_objectList = 0;

	m_light = 0;
	m_object = 0;
}

//-------------------------------------------------------------------

IoResult TestIoWin::processEvent (IoEvent* event)
{
	if (!isInitialized ())
		initialize ();

	m_mouseCursor->processEvent (event);

	switch (event->type)
	{
	case IOET_WindowKill:
		return IOR_PassKillMe;

	case IOET_InputReset:
		{
			m_button0 = false;
			m_button1 = false;
			m_button2 = false;

			int i;
			for (i = 0; i < K_count; ++i)
				m_keys [i] = 0;
		}
		break;

	case IOET_KeyDown:
		{
			switch (event->arg2)
			{
			case 200:  m_keys [K_up]       = true;  break;
			case 208:  m_keys [K_down]     = true;  break;
			case 203:  m_keys [K_left]     = true;  break;
			case 205:  m_keys [K_right]    = true;  break;
			case 210:  m_keys [K_insert]   = true;  break;
			case 211:  m_keys [K_delete]   = true;  break;
			case 199:  m_keys [K_home]     = true;  break;
			case 207:  m_keys [K_end]      = true;  break;
			case 201:  m_keys [K_pageUp]   = true;  break;
			case 209:  m_keys [K_pageDown] = true;  break;
			case  59:  m_keys [K_f1]       = true;  break;
			case  60:  m_keys [K_f2]       = true;  break;
			case  61:  m_keys [K_f3]       = true;  break;
			case  62:  m_keys [K_f4]       = true;  break;
			case  56:  m_keys [K_alt]      = true;  break;
			case 184:  m_keys [K_alt]      = true;  break;
			case  54:  m_keys [K_shift]    = true;  break;
			case  42:  m_keys [K_shift]    = true;  break;
			}
		}
		break;

	case IOET_KeyUp:
		{
			switch (event->arg2)
			{
			case 200:  m_keys [K_up]       = false;  break;
			case 208:  m_keys [K_down]     = false;  break;
			case 203:  m_keys [K_left]     = false;  break;
			case 205:  m_keys [K_right]    = false;  break;
			case 210:  m_keys [K_insert]   = false;  break;
			case 211:  m_keys [K_delete]   = false;  break;
			case 199:  m_keys [K_home]     = false;  break;
			case 207:  m_keys [K_end]      = false;  break;
			case 201:  m_keys [K_pageUp]   = false;  break;
			case 209:  m_keys [K_pageDown] = false;  break;
			case  59:  m_keys [K_f1]       = false;  break;
			case  60:  m_keys [K_f2]       = false;  break;
			case  61:  m_keys [K_f3]       = false;  break;
			case  62:  m_keys [K_f4]       = false;  break;
			case  56:  m_keys [K_alt]      = false;  break;
			case 184:  m_keys [K_alt]      = false;  break;
			case  54:  m_keys [K_shift]    = false;  break;
			case  42:  m_keys [K_shift]    = false;  break;
			}
		}
		break;

	case IOET_MouseButtonDown:
		{
			if (event->arg2 == 0)
				m_button0 = true;

			if (event->arg2 == 1)
				m_button1 = true;

			if (event->arg2 == 2)
				m_button2 = true;
		}
		break;

	case IOET_MouseButtonUp:
		{
			if (event->arg2 == 0)
				m_button0 = false;

			if (event->arg2 == 1)
				m_button1 = false;

			if (event->arg2 == 2)
				m_button2 = false;
		}
		break;

	case IOET_Character:
		{
			if (tolower (event->arg2) == 'q')
			{
				Game::quit ();
				return IOR_BlockKillMe;
			}	

			if (tolower (event->arg2) == '3')
			{
				GraphicsOptionTags::toggle (TAG_DOT3);
				reload();
			}

			if (tolower (event->arg2) == 'r')
				reload();

			if (tolower (event->arg2) == 'f')
			{
				m_cameraZoom = m_object->getAppearanceSphereRadius () * 4.f;
				m_cameraPivot = m_object->getAppearanceSphereCenter ();
				m_cameraPitch = PI_OVER_4;

				m_lightYaw = PI_OVER_2;
				m_lightPitch = PI_OVER_4;
				m_lightZoom = 20.f,
				m_lightPivot = m_cameraPivot;

				m_shouldUpdate = true;
			}

			if (tolower (event->arg2) == 'z')
			{
				m_wire = !m_wire;
				Graphics::setFillMode (m_wire ? GFM_wire : GFM_solid);
			}

			if (event->arg2 == ' ')
				m_paused = !m_paused;
		}
		break;

	case IOET_Update:
		{
			const float elapsedTime = event->arg3;

			//-- update the "world"
			m_lightList->prepareToAlter ();
			m_lightList->alter (elapsedTime);
			m_lightList->conclude ();

			m_objectList->prepareToAlter ();
			m_objectList->alter (elapsedTime);
			m_objectList->conclude ();

			if (!m_paused)
			{
				//-- update object
				m_object->yaw_o (event->arg3);
			}

			//-- deal with the camera
			{
				const Vector2d delta (static_cast<float> (m_mouseCursor->getX ()), static_cast<float> (m_mouseCursor->getY ()));

				float yawMod   = 0.f;
				float pitchMod = 0.f;
				float zoomMod  = 0.f;
				Vector translateMod;

				if (m_keys [K_alt])
				{
					Rectangle2d rect;
					m_camera->getViewport (rect.x0, rect.y0, rect.x1, rect.y1);

					//-- rotate
					if ((m_button0 && !m_button1 && !m_button2) || (!m_button0 && m_button1 && !m_button2))
					{
						if (m_first)
							m_first = false;
						else
						{
							yawMod   = PI * delta.x / rect.getWidth ();
							pitchMod = PI * delta.y * 0.5f / rect.getHeight ();

							m_shouldUpdate = true;
						}
					}
					else
						//-- translate
						if (!m_button0 && !m_button1 && m_button2)
						{
							if (m_first)
								m_first = false;
							else
							{
								translateMod.x = -m_object->getAppearanceSphereRadius () * delta.x /  rect.getWidth ();
								translateMod.y =  m_object->getAppearanceSphereRadius () * delta.y /  rect.getHeight ();

								m_shouldUpdate = true;
							}
						}
						else
							//-- zoom
							if ((m_button0 && !m_button1 && m_button2) || (!m_button0 && m_button1 && m_button2))
							{
								if (m_first)
									m_first = false;
								else
								{
									zoomMod = PI * delta.x / rect.getWidth () * m_object->getAppearanceSphereRadius ();

									m_shouldUpdate = true;
								}
							}
				}
				else
					m_first = true;
			
				if (m_shouldUpdate)
				{
					m_shouldUpdate = false;

					//-- update camera
					if (!m_keys[K_shift])
					{
						m_cameraYaw   += yawMod;
						m_cameraPitch += pitchMod;
						m_cameraPitch = clamp (-PI_OVER_2, m_cameraPitch, PI_OVER_2);
						m_cameraZoom  -= zoomMod;
						m_cameraPivot += m_camera->rotate_o2w(translateMod);
					}

					m_camera->resetRotateTranslate_o2p ();
					m_camera->move_p (m_cameraPivot);
					m_camera->yaw_o (m_cameraYaw);
					m_camera->pitch_o (m_cameraPitch);
					m_camera->move_o (-Vector::unitZ * m_cameraZoom);

					//-- update light
					if (m_light)
					{
						if (m_keys[K_shift])
						{
							m_lightYaw   -= yawMod;
							m_lightPitch -= pitchMod;
							m_lightPitch = clamp (-PI_OVER_2, m_lightPitch, PI_OVER_2);
							m_lightZoom  -= zoomMod;
							m_lightPivot += m_light->rotate_o2w(-translateMod);
						}

						m_light->resetRotateTranslate_o2p ();
						m_light->move_p (m_lightPivot);
						m_light->yaw_o (m_lightYaw);
						m_light->pitch_o (m_lightPitch);
 						m_light->move_o (-Vector::unitZ * m_lightZoom);
					}
				}
			}
		}
		break;
	}

	return IOR_Block;
}

//-------------------------------------------------------------------

void TestIoWin::draw () const
{
	//
	//-- render the scene 
	//
	{
		//-- set and clear the entire screen
		{
//			const PackedArgb clearColor (255, 161, 161, 161);
			const PackedArgb clearColor (PackedArgb::solidBlack);
			Graphics::setViewport (0, 0, Graphics::getCurrentRenderTargetWidth (), Graphics::getCurrentRenderTargetHeight ());
			Graphics::clearViewport (true, clearColor.getArgb (), true, 1.0f, true, 0);

			m_camera->renderScene ();
		}

		//-- show light
		if (m_light)
		{
			Graphics::setStaticShader (ShaderTemplateList::get3dVertexColorZStaticShader ());
			Graphics::setObjectToWorldTransformAndScale (m_light->getTransform_o2w (), Vector::xyz111);
			Graphics::drawFrame ();
		}
	}
}

//===================================================================
// PROTECTED TestIoWIn
//===================================================================

bool TestIoWin::isInitialized () const
{
	return m_initialized;
}

//-------------------------------------------------------------------

void TestIoWin::initialize ()
{
	m_mouseCursor = new MouseCursor (0, MouseCursor::S_relative);

	m_camera = new ObjectListCamera (2);
	m_lightList = new ObjectList;
	m_camera->addObjectList (m_lightList);
	m_objectList = new ObjectList;
	m_camera->addObjectList (m_objectList);

	createLights();

	m_object = createObjectAndAppearance ();
	m_objectList->addObject (m_object);
	m_cameraZoom = m_object->getAppearanceSphereRadius () * 4.0f;
	m_cameraPivot = m_object->getAppearanceSphereCenter ();
	m_lightPivot = m_cameraPivot;

	createFloor();

	m_initialized = true;
}

//-------------------------------------------------------------------

void TestIoWin::createLights()
{
	Light *ambientLight = new Light (Light::T_ambient, VectorArgb (1.f, 0.33f, 0.33f, 0.33f));
	m_objectList->addObject (ambientLight);

	m_light = new Light (Light::T_parallel, VectorArgb::solidWhite);
	m_objectList->addObject (m_light);
}

//-------------------------------------------------------------------

void TestIoWin::createFloor ()
{
	if (!ConfigClientGame::getTestFloor ())
		return;

	Object *floor = new Object ();
	floor->setAppearance (new ShaderAppearance (m_object->getAppearanceSphereRadius () * 5.f, m_object->getAppearanceSphereRadius () * 5.f, ShaderTemplateList::fetchModifiableShader ("shader/defaultshader.sht"), VectorArgb::solidWhite));

	const BoxExtent* boxExtent = dynamic_cast<const BoxExtent*> (m_object->getAppearance ()->getExtent ());
	if (boxExtent)
	{
		Vector position = m_object->getAppearanceSphereCenter ();
		position.y = boxExtent->getMin ().y;
		floor->setPosition_p (position);
	}
	else
	{
		floor->move_p (m_object->getAppearanceSphereCenter ());
		floor->move_p (m_object->getAppearanceSphereRadius () * Vector::negativeUnitY);
	}

	m_objectList->addObject (floor);
}

//-------------------------------------------------------------------

Object* TestIoWin::createObjectAndAppearance ()
{
	Object* const object = new Object ();
	object->setAppearance (AppearanceTemplateList::createAppearance (ConfigClientGame::getTestObject1 ()));
//	object->setAppearance (new StarAppearance ("terrain/colorramp/stars.tga", 5000, false));

	return object;
}

//-------------------------------------------------------------------

void TestIoWin::reload()
{
	m_objectList->removeObject (m_object);
	delete m_object;
	m_object = createObjectAndAppearance ();
	m_objectList->addObject (m_object);
}	

//===================================================================

