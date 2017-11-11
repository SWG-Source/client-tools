// ======================================================================
//
// Intro.cpp
// asommers
//
// copyright 2000, verant interactive
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/Intro.h"

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiBackdrop.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientAudio/Audio.h"
#include "clientBugReporting/ClientBugReporting.h"
#include "clientGame/ClientWorld.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGame/CreatureController.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/DustAppearance.h"
#include "clientGame/Game.h"
#include "clientGame/GameLight.h"
#include "clientGame/GameNetwork.h"
#include "clientGraphics/DynamicVertexBuffer.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/Light.h"
#include "clientGraphics/RenderWorld.h"
#include "clientGraphics/Shader.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/StaticShader.h"
#include "clientObject/GameCamera.h"
#include "clientObject/ShaderAppearance.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientSkeletalAnimation/TransformAnimationResolver.h"
#include "clientTerrain/SkyBoxAppearance.h"
#include "clientTerrain/StarAppearance.h"
#include "clientUserInterface/ConfigClientUserInterface.h"
#include "clientUserInterface/CuiBugSubmissionPage.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMediatorTypes.h"
#include "sharedFile/AsynchronousLoader.h"
#include "sharedFile/Iff.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/Production.h"
#include "sharedMath/VectorArgb.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/Object.h"
#include "sharedObject/ObjectList.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedRandom/Random.h"

#include <dinput.h>

class TransformAnimationController;

// ======================================================================

namespace IntroNamespace
{
	const Tag TAG_CAM  = TAG3 (C,A,M);
	const Tag TAG_INTR = TAG (I,N,T,R);
	const Tag TAG_LHTS = TAG (L,H,T,S);
	const Tag TAG_OBJ  = TAG3 (O,B,J);
	const Tag TAG_OBJS = TAG (O,B,J,S);
	const Tag TAG_OLST = TAG (O,L,S,T);
	const Tag TAG_PLST = TAG (P,L,S,T);
	const Tag TAG_POBJ = TAG (P,O,B,J);
	const Tag TAG_SCEN = TAG (S,C,E,N);
	const Tag TAG_SHDR = TAG (S,H,D,R);
	const Tag TAG_STAT = TAG (S,T,A,T);
	Tag const TAG_PLNT = TAG(P,L,N,T);
	const Tag TAG_TRAN = TAG (T,R,A,N);

	enum State
	{
		S_prestart,
		S_prestart2,
		S_start,
		S_longFadeIn,
		S_long,
		S_longFadeOut,
		S_longPause,
		S_logoMovingBackwards,
		S_logoFading,
		S_textScroll,
		S_textFading,
		S_action,
		S_screenFade,
		S_end,
		S_wait
	};

	class PostureObjectData
	{
	public:

		PersistentCrcString objectTemplateName;
		Vector              position_w;
		int                 startPosture;
		int                 endPosture;

	private:
		PostureObjectData & operator=(PostureObjectData const & rhs);
	};

	class ObjectData
	{
	public:

		PersistentCrcString objectTemplateName;
		Vector              position_w;
	private:
		ObjectData & operator=(ObjectData const & rhs);
	};

	void animationMessageCallback (void* context, const CrcLowerString& animationMessageName, const TransformAnimationController* /*controller*/)
	{
		//-- remove the context
		const char* buffer = animationMessageName.getString ();
		while (buffer && *buffer != '_')
			++buffer;

		TemporaryCrcString cameraHardpointName (buffer + 1, false);
		reinterpret_cast<Intro*> (context)->setCameraHardpointName (cameraHardpointName);
	}
}

using namespace IntroNamespace;

// ======================================================================
// PRIVATE Intro::Data
// ======================================================================

struct Intro::Data
{
public:

	PersistentCrcString            longShaderTemplateName;
	PersistentCrcString            logoShaderTemplateName;
	PersistentCrcString            crawlShaderTemplateName;
	float                          longTime;
	float                          longFade;
	float                          longPause;
	float                          logoSizeX;
	float                          logoSizeY;
	float                          logoTimePrefade;
	float                          logoInitialPosition;
	float                          logoSpeed;
	float                          logoTimeFade;
	float                          crawlTime;
	float                          crawlSizeX;
	float                          crawlSizeY;
	float                          crawlInitialPositionForward;
	float                          crawlInitialPositionDown;
	float                          crawlAngle;
	float                          crawlInitialPositionBack;
	float                          crawlSpeed;
	float                          crawlTimeFade;
	float                          actionTime;
	float                          screenFadeTime;

	PersistentCrcString            starColorRampFileName;
	int                            numberOfStars;

	PersistentCrcString            skyShaderTemplateName;
	PersistentCrcString            environmentTextureName;

	float                          cameraNearPlane;
	float                          cameraFarPlane;
	float                          cameraFieldOfView;
	PersistentCrcString            cameraObjectTemplateName;
	Vector                         cameraPosition_w;
	int                            cameraStartPosture;
	int                            cameraEndPosture;
	CrcLowerString                 cameraHardpointName;

	std::vector<PostureObjectData> postureObjectDataList;
	std::vector<ObjectData>        objectDataList;

public:

	Data ();

private:

	Data & operator=(Data const & rhs);
};

// ----------------------------------------------------------------------

Intro::Data::Data () :
	longShaderTemplateName (),
	logoShaderTemplateName (),
	crawlShaderTemplateName (),
	longTime (0),
	longFade (0),
	longPause (0),
	logoSizeX (0),
	logoSizeY (0),
	logoTimePrefade (0),
	logoInitialPosition (0),
	logoSpeed (0),
	logoTimeFade (0),
	crawlTime (0),
	crawlSizeX (0),
	crawlSizeY (0),
	crawlInitialPositionForward (0),
	crawlInitialPositionDown (0),
	crawlAngle (0),
	crawlInitialPositionBack (0),
	crawlSpeed (0),
	crawlTimeFade (0),
	actionTime (0),
	screenFadeTime (0),
	starColorRampFileName (),
	numberOfStars (0),
	cameraNearPlane (1),
	cameraFarPlane (10000),
	cameraFieldOfView (PI_OVER_3),
	cameraObjectTemplateName (),
	cameraPosition_w (),
	cameraStartPosture (0),
	cameraEndPosture (0),
	cameraHardpointName (),
	skyShaderTemplateName (),
	environmentTextureName ()
{
}

// ======================================================================
// PUBLIC Intro
// ======================================================================

Intro::Intro() : 
	IoWin ("Intro"),
	m_data (new Data),
	m_camera (new GameCamera ()),
	m_postureObjectList (new ObjectList (10)),
	m_logoCrawlObject (new Object ()),
	m_cameraHardpointName (),
	m_skyboxObject (0),
	m_starsObject (0),
	m_cameraObject (0),
	m_timer (0.f),
	m_state (S_prestart),
	m_introSoundId (0),
	m_longShader (0),
	m_longAlpha (0.f),
	m_screenAlpha (0.f)
{
	DEBUG_REPORT_LOG_PRINT (true, ("Intro::Intro - it has begun!\n"));

	AsynchronousLoader::disable();

	ClientWorld::install ();
	ClientWorld::addCamera (m_camera);

	CreatureObject::setDisablePostureRevert (true);

	if (ConfigClientGame::getSkipIntro ())
	{
		m_state = S_end;

		//-- prevent this object from getting leaked.
		delete m_logoCrawlObject;
	}
	else
	{
		load ("scene/intro.iff");

		//-- 
		m_longShader = ShaderTemplateList::fetchShader (m_data->longShaderTemplateName);

		//-- set camera parameters
		m_camera->setViewport (0, Graphics::getCurrentRenderTargetHeight () / 6, Graphics::getCurrentRenderTargetWidth (), Graphics::getCurrentRenderTargetHeight () * 2 / 3);
		m_camera->setNearPlane (m_data->cameraNearPlane);
		m_camera->setFarPlane (m_data->cameraFarPlane);
		m_camera->setHorizontalFieldOfView (m_data->cameraFieldOfView);
	}
}

// ----------------------------------------------------------------------

Intro::~Intro()
{
	if (m_state < S_long)
		delete m_logoCrawlObject;

	if (m_skyboxObject)
	{
		RenderWorld::removeWorldEnvironmentObject (m_skyboxObject);

		delete m_skyboxObject;
		m_skyboxObject = 0;
	}

	if (m_starsObject)
	{
		RenderWorld::removeWorldEnvironmentObject (m_starsObject);

		delete m_starsObject;
		m_starsObject = 0;
	}

	ClientWorld::removeCamera (m_camera);
	delete m_camera;

	ClientWorld::remove ();

	m_postureObjectList->removeAll (false);
	delete m_postureObjectList;

	if (m_longShader)
	{
		m_longShader->release();
		m_longShader = 0;
	}

	delete m_data;

	AsynchronousLoader::enable();
}

// ----------------------------------------------------------------------

IoResult Intro::processEvent(IoEvent *event)
{
	switch (event->type)
	{
	case IOET_WindowKill:
		return IOR_PassKillMe;

	case IOET_Character:
		{
#if PRODUCTION == 0
			if (tolower (event->arg2) == 'z')
			{
				static bool m_wire = false;
				m_wire = !m_wire;
				Graphics::setFillMode (m_wire ? GFM_wire : GFM_solid);
			}

			if (tolower (event->arg2) == 'r')
			{
				//-- reset state
				m_state = S_start;
				m_timer.setExpireTime (0.f);

				//-- reset sound
				Audio::stopSound (m_introSoundId, 0.5f);

				//-- reset camera
				if (m_cameraObject)
					m_cameraObject->setVisualPosture (static_cast<int8> (m_data->cameraStartPosture));

				m_cameraHardpointName.set (m_data->cameraHardpointName.getString (), false);

				//-- reset all posture objects
				{
					int i;
					for (i = 0; i < m_postureObjectList->getNumberOfObjects (); ++i)
					{
						const PostureObjectData& data = m_data->postureObjectDataList [static_cast<uint> (i)];
						CreatureObject* const creatureObject = safe_cast<CreatureObject*> (m_postureObjectList->getObject (i));
						creatureObject->setVisualPosture (static_cast<int8> (data.startPosture));
					}
				}
			}
#endif
		}
		break;

	case IOET_KeyUp:
		{
			if (event->arg2 == DIK_ESCAPE)
			{
				leaveIntro();
				return IOR_BlockKillMe;
			}
		}
		break;

	case IOET_Update:
		{
			//-- udpate state information
			update (event->arg3);

			//-- alter object lists
			ClientWorld::update (event->arg3);

			Transform hardpointTransform;
			if (m_cameraObject && m_cameraObject->getAppearance ()->findHardpoint (m_cameraHardpointName, hardpointTransform))
				m_camera->setTransform_o2p (hardpointTransform);

			if (m_state == S_end)
			{
				leaveIntro ();
				return IOR_BlockKillMe;
			}
		}
		break;

	default:
		break;
	}

	return IOR_Block;
}

// ----------------------------------------------------------------------

void Intro::draw () const
{
#ifdef _DEBUG
	DEBUG_REPORT_PRINT (true, ("yaw=%1.2f  pitch=%1.2f  ", m_camera->getObjectFrameK_w ().theta (), m_camera->getObjectFrameK_w ().phi ()));
	m_camera->getPosition_w ().debugPrint ("");
//	DEBUG_REPORT_PRINT (true, ("cameraHardpointName = %s\n", m_cameraHardpointName.getString ()));
#endif

	Graphics::setViewport (0, 0, Graphics::getCurrentRenderTargetWidth (), Graphics::getCurrentRenderTargetHeight ());
	Graphics::clearViewport(true, 0, true, 1.0f, true, 0);

	const int x0 = m_camera->getViewportX0 ();
	const int y0 = m_camera->getViewportY0 ();
	const int x1 = x0 + m_camera->getViewportWidth () - 1;
	const int y1 = y0 + m_camera->getViewportHeight() - 1;

	Graphics::setViewport (static_cast<int> (x0), static_cast<int> (y0), static_cast<int> (x1), static_cast<int> (y1));

	switch (m_state)
	{
	default:
		break;

	case S_longFadeIn:
	case S_long:
	case S_longFadeOut:
	case S_longPause:
		{
			VertexBufferFormat format;
			format.setPosition();
			format.setTransformed();
			format.setColor0();
			format.setNumberOfTextureCoordinateSets(1);
			format.setTextureCoordinateSetDimension(0, 2);
			DynamicVertexBuffer vertexBuffer (format);
			vertexBuffer.lock(4);

				const float offsetX = static_cast<float> (m_camera->getViewportWidth () - 512) * 0.5f;
				const float offsetY = static_cast<float> (m_camera->getViewportHeight () - 128) * 0.5f;
				const uint8 alpha   = static_cast<uint8> (m_longAlpha * 255.f);

				VertexBufferWriteIterator v = vertexBuffer.begin();
			
				v.setPosition (Vector (0.f + offsetX, 0.f + offsetY, 0.f));
				v.setColor0 (PackedArgb (alpha, 255, 255, 255));
				v.setOoz (1.f);
				v.setTextureCoordinates (0, 0.f, 0.f);
				++v;

				v.setPosition (Vector (511.f + offsetX, 0.f + offsetY, 0.f));
				v.setColor0 (PackedArgb (alpha, 255, 255, 255));
				v.setOoz (1.f);
				v.setTextureCoordinates (0, 1.f, 0.f);
				++v;

				v.setPosition (Vector (511.f + offsetX, 127.f + offsetY, 0.f));
				v.setColor0 (PackedArgb (alpha, 255, 255, 255));
				v.setOoz (1.f);
				v.setTextureCoordinates (0, 1.f, 1.f);
				++v;

				v.setPosition (Vector (0.f + offsetX, 127.f + offsetY, 0.f));
				v.setColor0 (PackedArgb (alpha, 255, 255, 255));
				v.setOoz (1.f);
				v.setTextureCoordinates (0, 0.f, 1.f);

			vertexBuffer.unlock();

			Graphics::setStaticShader (m_longShader->prepareToView());
			Graphics::setVertexBuffer (vertexBuffer);
			Graphics::drawTriangleFan ();
		}
		break;

	case S_logoMovingBackwards:
	case S_logoFading:
	case S_textScroll:
	case S_textFading:
	case S_action:
		{
			m_camera->renderScene ();
		}
		break;

	case S_screenFade:
		{
			m_camera->renderScene ();

			VertexBufferFormat format;
			format.setPosition ();
			format.setTransformed ();
			format.setColor0 ();
			DynamicVertexBuffer vertexBuffer (format);

			vertexBuffer.lock (4);

				VertexBufferWriteIterator v = vertexBuffer.begin ();

				const float screenX0 = 0.f;
				const float screenX1 = static_cast<float> (Graphics::getCurrentRenderTargetWidth ());
				const float screenY0 = 0.f;
				const float screenY1 = static_cast<float> (Graphics::getCurrentRenderTargetHeight ());

				VectorArgb color (VectorArgb::solidBlack);
				color.a = m_screenAlpha;

				v.setPosition (Vector (screenX0, screenY0, 1.f));
				v.setOoz (1.f);
				v.setColor0 (color);
				++v;

				v.setPosition (Vector (screenX1, screenY0, 1.f));
				v.setOoz (1.f);
				v.setColor0 (color);
				++v;

				v.setPosition (Vector (screenX1, screenY1, 1.f));
				v.setOoz (1.f);
				v.setColor0 (color);
				++v;

				v.setPosition (Vector (screenX0, screenY1, 1.f));
				v.setOoz (1.f);
				v.setColor0 (color);

			vertexBuffer.unlock ();

			Graphics::setStaticShader (ShaderTemplateList::get2dVertexColorAStaticShader ().prepareToView());
			Graphics::setVertexBuffer (vertexBuffer);
			Graphics::drawTriangleFan ();
		}
		break;

	case S_end:
		break;
	}
}

// ----------------------------------------------------------------------

void Intro::setCameraHardpointName (const CrcString& cameraHardpointName)
{
	m_cameraHardpointName.set (cameraHardpointName.getString (), false);
}

// ======================================================================
// PRIVATE Intro
// ======================================================================

void Intro::load (const char* filename)
{
	Iff iff (filename);

	iff.enterForm (TAG_INTR);

		{
			iff.enterChunk (TAG_SHDR);

				std::string shaderTemplateName;
				iff.read_string (shaderTemplateName);
				m_data->longShaderTemplateName.set (shaderTemplateName.c_str (), true);
				iff.read_string (shaderTemplateName);
				m_data->logoShaderTemplateName.set (shaderTemplateName.c_str (), true);
				iff.read_string (shaderTemplateName);
				m_data->crawlShaderTemplateName.set (shaderTemplateName.c_str (), true);

			iff.exitChunk ();
		}

		{
			iff.enterChunk (TAG_DATA);

				m_data->longTime                    = iff.read_float ();
				m_data->longFade                    = iff.read_float ();
				m_data->longPause                   = iff.read_float ();
				m_data->logoSizeX                   = iff.read_float ();
				m_data->logoSizeY                   = iff.read_float ();
				m_data->logoTimePrefade             = iff.read_float ();
				m_data->logoInitialPosition         = iff.read_float ();
				m_data->logoSpeed                   = iff.read_float ();
				m_data->logoTimeFade                = iff.read_float ();
				m_data->crawlTime                   = iff.read_float ();
				m_data->crawlSizeX                  = iff.read_float ();
				m_data->crawlSizeY                  = iff.read_float ();
				m_data->crawlInitialPositionForward = iff.read_float ();
				m_data->crawlInitialPositionDown    = iff.read_float ();
				m_data->crawlAngle                  = iff.read_float ();
				m_data->crawlInitialPositionBack    = iff.read_float ();
				m_data->crawlSpeed                  = iff.read_float ();
				m_data->crawlTimeFade               = iff.read_float ();
				m_data->actionTime                  = iff.read_float ();
				m_data->screenFadeTime              = iff.read_float ();

			iff.exitChunk ();
		}

		//-- lights
		{
			iff.enterForm (TAG_LHTS);

				const int numberOfLights = iff.getNumberOfBlocksLeft ();

				int i;
				for (i = 0; i < numberOfLights; ++i)
				{
					Light* const light = GameLight::createLight (&iff);
					light->addNotification (ClientWorld::getIntangibleNotification ());
					light->addToWorld ();
				}

			iff.exitForm ();
		}

		//-- scene
		{
			iff.enterChunk (TAG_SCEN);

				std::string skyShaderTemplateName;
				iff.read_string (skyShaderTemplateName);
				m_data->skyShaderTemplateName.set (skyShaderTemplateName.c_str (), true);

				std::string starColorRampFileName;
				iff.read_string (starColorRampFileName);
				m_data->starColorRampFileName.set (starColorRampFileName.c_str (), true);

				m_data->numberOfStars = iff.read_int32 ();

				std::string environmentTextureName;
				iff.read_string (environmentTextureName);
				m_data->environmentTextureName.set (environmentTextureName.c_str (), true);

			iff.exitChunk ();
		}

		//-- planet
		{
			iff.enterChunk(TAG_PLNT);

				std::string planetAppearanceTemplateName;
				iff.read_string(planetAppearanceTemplateName);
				
				Vector const position_p = iff.read_floatVector();

				Object * const planet = new Object();
				planet->setAppearance(AppearanceTemplateList::createAppearance(planetAppearanceTemplateName.c_str()));
				planet->addNotification(ClientWorld::getIntangibleNotification());
				planet->setPosition_p(position_p);
				RenderWorld::addObjectNotifications(*planet);
				planet->addToWorld();

			iff.exitChunk(TAG_PLNT);
		}

		//-- camera
		{
			iff.enterChunk (TAG_CAM);

				m_data->cameraNearPlane = iff.read_float ();
				m_data->cameraFarPlane = iff.read_float ();
				m_data->cameraFieldOfView = convertDegreesToRadians (iff.read_float ());

				std::string objectTemplateName;
				iff.read_string (objectTemplateName);
				m_data->cameraObjectTemplateName.set (objectTemplateName.c_str (), true);

				m_data->cameraStartPosture = iff.read_int32 ();
				m_data->cameraEndPosture = iff.read_int32 ();

				std::string hardpointName;
				iff.read_string (hardpointName);
				m_data->cameraHardpointName.set (hardpointName.c_str (), false);

			iff.exitChunk ();
		}

		//-- posture object data
		{
			iff.enterForm (TAG_PLST);

				while (iff.getNumberOfBlocksLeft ())
				{
					PostureObjectData data;

					iff.enterChunk (TAG_POBJ);

						std::string objectTemplateName;
						iff.read_string (objectTemplateName);
						data.objectTemplateName.set (objectTemplateName.c_str (), true);
						data.position_w = iff.read_floatVector ();
						data.startPosture = iff.read_int32 ();
						data.endPosture = iff.read_int32 ();
					
					iff.exitChunk ();

					m_data->postureObjectDataList.push_back (data);
				}

			iff.exitForm (TAG_PLST);
		}

		//-- object data
		{
			iff.enterForm (TAG_OLST);

				while (iff.getNumberOfBlocksLeft ())
				{
					ObjectData data;

					iff.enterChunk (TAG_OBJ);

						std::string objectTemplateName;
						iff.read_string (objectTemplateName);
						data.objectTemplateName.set (objectTemplateName.c_str (), true);
						data.position_w = iff.read_floatVector ();

					iff.exitChunk ();

					m_data->objectDataList.push_back (data);
				}

			iff.exitForm ();
		}

	iff.exitForm (true);
}  //lint !e429  //-- planet has not been freed or returned

// ----------------------------------------------------------------------

void Intro::preloadAssets ()
{
	//-- add stars, dust, and skybox
	{
//		m_skyboxObject = new Object ();
//		m_skyboxObject->setAppearance (new SkyBoxAppearance (m_data->skyShaderTemplateName.getString ()));
//		m_skyboxObject->setScale (Vector::xyz111 * 10.f);
//		RenderWorld::addWorldEnvironmentObject (m_skyboxObject);

		m_starsObject = new Object ();
		m_starsObject->setAppearance (new StarAppearance (m_data->starColorRampFileName.getString (), m_data->numberOfStars, false));
		RenderWorld::addWorldEnvironmentObject (m_starsObject);
	}

	//-- create object
	if (!m_logoCrawlObject->isInWorld ())
	{
		m_logoCrawlObject->addNotification (ClientWorld::getIntangibleNotification ());
		RenderWorld::addObjectNotifications (*m_logoCrawlObject);
		m_logoCrawlObject->addToWorld ();
	}

	//-- create camera object
	m_cameraObject = safe_cast<CreatureObject*> (ObjectTemplateList::createObject (m_data->cameraObjectTemplateName));
	m_cameraObject->setController (new CreatureController (m_cameraObject));
	m_cameraObject->endBaselines ();
	m_cameraObject->setVisualPosture (static_cast<int8> (m_data->cameraStartPosture));
	RenderWorld::addObjectNotifications (*m_cameraObject);
	m_cameraObject->addToWorld ();

	//-- Tell AnimationResolver to add the callback to all animation controllers.
	safe_cast<SkeletalAppearance2*> (m_cameraObject->getAppearance ())->getAnimationResolver ().addAnimationCallback (animationMessageCallback, this);
	m_cameraHardpointName.set (m_data->cameraHardpointName.getString (), false);

	//-- create posture objects
	{
		uint i;
		for (i = 0; i < m_data->postureObjectDataList.size (); ++i)
		{
			const PostureObjectData& data = m_data->postureObjectDataList [i];

			Object* const object = ObjectTemplateList::createObject (data.objectTemplateName);
			CreatureObject* const creatureObject = dynamic_cast<CreatureObject*> (object);

			if (creatureObject)
			{
				creatureObject->setPosition_w (data.position_w);
				creatureObject->setController (new CreatureController (creatureObject));
				creatureObject->endBaselines ();
				creatureObject->setVisualPosture (static_cast<int8> (data.startPosture));
				RenderWorld::addObjectNotifications (*creatureObject);
				creatureObject->addToWorld ();

				m_postureObjectList->addObject (creatureObject);
			}
			else
			{
				DEBUG_WARNING (true, ("Intro::Intro: could not create object %s", data.objectTemplateName.getString ()));
				delete object;
			}
		}
	}

	//-- create station
	{
		uint i;
		for (i = 0; i < m_data->objectDataList.size (); ++i)
		{
			const ObjectData& data = m_data->objectDataList [i];

			ClientObject * const object = safe_cast<ClientObject *> (ObjectTemplateList::createObject (data.objectTemplateName));
			if (object)
			{
				object->endBaselines ();
				object->setPosition_w (data.position_w);
				RenderWorld::addObjectNotifications (*object);
				object->addToWorld ();
			}
			else
				DEBUG_WARNING (true, ("Intro::Intro: could not create object %s", data.objectTemplateName.getString ()));
		}
	}
}

// ----------------------------------------------------------------------

void Intro::update (const float elapsedTime)
{
	switch (m_state)
	{
	case S_prestart:
		{
			m_timer.setExpireTime (2.f);

			m_state = S_prestart2;
		}
		break;

	case S_prestart2:
		{
			if (m_timer.updateZero (elapsedTime))
				m_state = S_start;
		}
		break;

	case S_start:
		{
			m_timer.setExpireTime (m_data->longFade);

			m_state = S_longFadeIn;
		}
		break;

	case S_longFadeIn:
		{
			m_longAlpha = m_timer.getElapsedRatio ();
			
			if (m_timer.updateZero (elapsedTime))
			{
				m_longAlpha = 1.f;

				m_timer.setExpireTime (m_data->longTime);

				m_state = S_long;
			}
		}
		break;

	case S_long:
		{
			preloadAssets ();

			m_timer.setExpireTime (m_data->longFade);
			
			m_state = S_longFadeOut;
		}
		break;
		
	case S_longFadeOut:
		{
			m_longAlpha = m_timer.getRemainingRatio ();

			if (m_timer.updateZero (elapsedTime))
			{
				m_longAlpha = 0.f;

				m_timer.setExpireTime (m_data->longPause);
				
				m_state = S_longPause;
			}
		}
		break;

	case S_longPause:
		{
			if (m_timer.updateZero (elapsedTime))
			{
				m_timer.setExpireTime (m_data->logoTimePrefade);

				//--
				Shader* const logoShader = ShaderTemplateList::fetchModifiableShader (m_data->logoShaderTemplateName);
				m_logoCrawlObject->setAppearance (new ShaderAppearance (m_data->logoSizeX, m_data->logoSizeY, logoShader, VectorArgb::solidWhite));
				m_logoCrawlObject->setTransform_o2p (m_camera->getTransform_o2w ());
				m_logoCrawlObject->move_o (Vector::unitZ * m_data->logoInitialPosition);

				//-- start music
				m_introSoundId = Audio::playSound ("sound/intro.snd", NULL);

				m_state = S_logoMovingBackwards;
			}
		}
		break;

	case S_logoMovingBackwards:
		{
			m_logoCrawlObject->move_o (Vector::unitZ * m_data->logoSpeed * elapsedTime);

			if (m_timer.updateZero (elapsedTime))
			{
				m_timer.setExpireTime (m_data->logoTimeFade);

				m_state = S_logoFading;
			}
		}
		break;

	case S_logoFading:
		{
			m_logoCrawlObject->move_o (Vector::unitZ * m_data->logoSpeed * elapsedTime);

			if (m_logoCrawlObject->getAppearance ())
				m_logoCrawlObject->getAppearance ()->setAlpha (true, m_timer.getRemainingRatio (), true, m_timer.getRemainingRatio ());

			if (m_timer.updateZero (elapsedTime))
			{
				m_timer.setExpireTime (m_data->crawlTime);

				Shader* crawlShader = ShaderTemplateList::fetchModifiableShader (m_data->crawlShaderTemplateName);
				m_logoCrawlObject->setAppearance (new ShaderAppearance (m_data->crawlSizeX, m_data->crawlSizeY, crawlShader, VectorArgb::solidWhite));

				m_logoCrawlObject->setTransform_o2p (m_camera->getTransform_o2w ());
				m_logoCrawlObject->move_o (Vector (0.f, m_data->crawlInitialPositionDown, m_data->crawlInitialPositionForward));
				m_logoCrawlObject->pitch_o (convertDegreesToRadians (90.f - m_data->crawlAngle));
				m_logoCrawlObject->move_o (Vector::negativeUnitY * m_data->crawlInitialPositionBack);
				
				m_state = S_textScroll;
			}
		}
		break;

	case S_textScroll:
		{
			m_logoCrawlObject->move_o (Vector::unitY * m_data->crawlSpeed * elapsedTime);

			if (m_timer.updateZero (elapsedTime))
			{
				m_timer.setExpireTime (m_data->crawlTimeFade);

				m_state = S_textFading;
			}
		}
		break;

	case S_textFading:
		{
			m_logoCrawlObject->move_o (Vector::unitY * m_data->crawlSpeed * elapsedTime);

			if (m_logoCrawlObject->getAppearance ())
				m_logoCrawlObject->getAppearance ()->setAlpha (true, m_timer.getRemainingRatio (), true, m_timer.getRemainingRatio ());

			if (m_timer.updateZero (elapsedTime))
			{
				m_timer.setExpireTime (m_data->actionTime);

				//-- set the camera to its end posture
				m_cameraObject->setVisualPosture (static_cast<int8> (m_data->cameraEndPosture));

				//-- set all posture objects to end posture
				{
					int i;
					for (i = 0; i < m_postureObjectList->getNumberOfObjects (); ++i)
					{
						const PostureObjectData& data = m_data->postureObjectDataList [static_cast<uint> (i)];
						CreatureObject* const creatureObject = safe_cast<CreatureObject*> (m_postureObjectList->getObject (i));
						creatureObject->setVisualPosture (static_cast<int8> (data.endPosture));
					}
				}

				m_state = S_action;
			}
		}
		break;

	case S_action:
		{
			if (m_timer.updateZero (elapsedTime))
			{
				m_timer.setExpireTime (m_data->screenFadeTime);

				m_state = S_screenFade;
			}
		}
		break;

	case S_screenFade:
		{
			m_screenAlpha = m_timer.getElapsedRatio ();

			if (m_timer.updateZero (elapsedTime))
				m_state = S_end;
//				m_state = S_wait;
		}
		break;

	case S_wait:
	default:
		break;
	}
}

// ----------------------------------------------------------------------

void Intro::leaveIntro ()
{
	REPORT_LOG_PRINT (true, ("Intro::leaveIntro()\n"));

	CreatureObject::setDisablePostureRevert (false);

	Audio::stopSound (m_introSoundId, 0.5f);
//	Audio::setMusicVolume (ConfigClientGame::getMusicVolume());

	// -------------------------------------------------------------------------

	Graphics::setViewport (0, 0, Graphics::getCurrentRenderTargetWidth (), Graphics::getCurrentRenderTargetHeight ());
	
	CuiBackdrop * backdrop = dynamic_cast<CuiBackdrop*>(CuiMediatorFactory::get (CuiMediatorTypes::Backdrop));
	NOT_NULL (backdrop);
	backdrop->setRenderData (0, 0);
	
	CuiMediatorFactory::activate (CuiMediatorTypes::Backdrop);

	CuiMediatorFactory::activate ("LoginScreen"); // SwgCuiMediatorTypes::LoginScreen;
	CuiManager::restartMusic ();
}

// ======================================================================
