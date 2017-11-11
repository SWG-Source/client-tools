// ============================================================================
//
// ParticleEditorIoWin.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstParticleEditor.h"
#include "ParticleEditorIoWin.h"

#include "clientGame/ClientWorld.h"
#include "clientGame/FreeCamera.h"
#include "clientGame/Game.h"
#include "clientGame/GroundScene.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/RenderWorld.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientObject/GameCamera.h"
#include "clientParticle/ParticleEffectAppearance.h"
#include "MainWindow.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/Object.h"
#include "sharedObject/World.h"
#include "sharedTerrain/TerrainObject.h"

//=============================================================================
//
// ParticleEditorIoWin
//
//=============================================================================

//-----------------------------------------------------------------------------
ParticleEditorIoWin::ParticleEditorIoWin(MainWindow *particleEditor)
 : IoWin ("ParticleEditorIoWin")
 , m_object(NULL)
 , m_objectTransform(Transform::identity)
 , m_playBackRate(1.0f)
 , m_effectScale(1.0f)
 , m_objectMovement(ParticleEffectTransformEdit::OM_specifiedTranslation)
 , m_particleEditor(particleEditor)
 , m_deltaX(40.0f)
 , m_radian(0.0f)
 , m_size(40.0f)
 , m_speed(40.0f)
{
	m_object = new Object();
	NOT_NULL(m_object);

	RenderWorld::addObjectNotifications(*m_object);
	m_object->addNotification(ClientWorld::getIntangibleNotification());
	m_object->addToWorld();
	setTimeOfDayCycle(false);
}

//-----------------------------------------------------------------------------
ParticleEditorIoWin::~ParticleEditorIoWin()
{
	delete m_object;
}

//-----------------------------------------------------------------------------
void ParticleEditorIoWin::setAppearanceTemplate(AppearanceTemplate const * const appearanceTemplate)
{
	// Recreate the object each time a appearance is assigned

	if ((m_object != NULL) &&
	    (appearanceTemplate != NULL))
	{
		Appearance *appearance = appearanceTemplate->createAppearance();
		NOT_NULL(appearance);

		ParticleEffectAppearance *particleEffectAppearance = dynamic_cast<ParticleEffectAppearance *>(appearance);
		NOT_NULL(particleEffectAppearance);

		// Do not allow the appearance to delete the object

		particleEffectAppearance->setAutoDelete(false);
		particleEffectAppearance->setUnBounded(true);
		particleEffectAppearance->setPlayBackRate(m_playBackRate);
		particleEffectAppearance->setUniformScale(m_effectScale);

		m_object->removeFromWorld();
		m_object->setAppearance(particleEffectAppearance);
		m_object->addToWorld();

		m_deltaX = m_size;

		m_radian = 0.0f;
		setObjectMovement(0.0f);

		AppearanceTemplateList::release(appearanceTemplate);
	}
}

//-----------------------------------------------------------------------------
void ParticleEditorIoWin::draw() const
{
	IoWin::draw();

	//// Draw the object transform
	//
	//Graphics::setStaticShader(ShaderTemplateList::getVertexColorZStaticShader());
	//float const minSize = 5.0f;
	//float const width = minSize;
	//float const height = minSize;
	//float const length = minSize;
	//
	//Graphics::setObjectToWorldTransformAndScale(m_object->getTransform_o2w(), Vector(length, height, width));
	//Graphics::drawFrame();

	//GroundScene *groundScene = dynamic_cast<GroundScene *>(Game::getScene());
	//
	//const GameCamera * const camera = groundScene->getCurrentView();
	//Transform c2w(camera->getWorldToCameraTransform());
	//
	//Vector localOrigin(c2w.rotateTranslate_l2p(Vector(0.0f, 0.0f, 3.0f)));
	//
	//NON_NULL(camera);
	//
	//Graphics::drawLine(100, 100, 500, 500, VectorArgb::solidBlue);
	//Graphics::drawLine(localOrigin, Vector(localOrigin.x, localOrigin.y + 10.0f, localOrigin.z), VectorArgb::solidRed);
	//Graphics::drawSphere(localOrigin, 2.0f, 8, 8);

	//FreeCamera const * const freeCamera = dynamic_cast<FreeCamera const *>(Game::getScene()->getCurrentView());
	//
	//if(freeCamera)
	//{
	//	Graphics::setObjectToWorldTransformAndScale(Transform::identity, Vector::xyz111);
	//	
	//	const Vector& pivotPt = freeCamera->getPivotPoint();
	//	const real     ylen     = CONST_REAL(10);
	//	const real     crosslen = CONST_REAL(1);
	//
	//	const Vector pivot1(pivotPt.x, pivotPt.y - CONST_REAL(ylen), pivotPt.z);
	//	const Vector pivot2(pivotPt.x, pivotPt.y + CONST_REAL(ylen), pivotPt.z);
	//
	//	Graphics::drawLine(pivot1, pivot2, VectorArgb::solidBlue);
	//
	//	const Vector crossBars[4] =
	//	{
	//		Vector(pivotPt.x - crosslen, pivotPt.y, pivotPt.z),
	//		Vector(pivotPt.x + crosslen, pivotPt.y, pivotPt.z),
	//		Vector(pivotPt.x,            pivotPt.y, pivotPt.z - crosslen),
	//		Vector(pivotPt.x,            pivotPt.y, pivotPt.z + crosslen)
	//	};
	//
	//	Graphics::drawLine(crossBars[0], crossBars[1], VectorArgb::solidGreen);
	//	Graphics::drawLine(crossBars[2], crossBars[3], VectorArgb::solidGreen);
	//}
}

//-----------------------------------------------------------------------------
void ParticleEditorIoWin::setObjectMovement(float const deltaTime) const
{
	// Get the delta time between frames

	if (deltaTime > 10.0f)
	{
		return;
	}

	// Set the object movement

	switch (m_objectMovement)
	{
		case ParticleEffectTransformEdit::OM_specifiedTranslation:
			{
				m_object->setTransform_o2p(m_objectTransform);
			}
			break;
		case ParticleEffectTransformEdit::OM_moveInACircle:
			{

				float const radius = m_size;

				float const x = m_objectTransform.getPosition_p().x + sinf(m_radian) * radius;
				float const y = m_objectTransform.getPosition_p().y;
				float const z = m_objectTransform.getPosition_p().z + cosf(m_radian) * radius;

				if(radius != 0.0f)
				{
					m_radian += (m_speed/radius) * deltaTime;
				}

				Transform transform;
				transform.yaw_l(m_radian + PI_OVER_2);
				transform.setPosition_p(x, y, z);
				m_object->setTransform_o2p(transform);

			}
			break;
		case ParticleEffectTransformEdit::OM_simulateBoltGround:
		case ParticleEffectTransformEdit::OM_simulateBoltSpace:
			{

				m_deltaX -= deltaTime * m_speed;
				if(m_deltaX < -m_size)
				{
					m_deltaX = m_size;
					m_particleEditor->restartParticleSystem();
				}

				float const x = m_objectTransform.getPosition_p().x + m_deltaX;
				float const y = m_objectTransform.getPosition_p().y;
				float const z = m_objectTransform.getPosition_p().z;

				Transform transform;
				transform.yaw_l(-PI_OVER_2);
				transform.setPosition_p(x, y, z);
				m_object->setTransform_o2p(transform);

			}
			break;
		case ParticleEffectTransformEdit::OM_simulateGrenade:
			{
			}
			break;
		case ParticleEffectTransformEdit::OM_lockToXYZ:
			{
				float const x = Game::getPlayer()->getPosition_w().x;
				float const y = Game::getPlayer()->getPosition_w().y;
				float const z = Game::getPlayer()->getPosition_w().z;

				Transform transform;
				transform.setPosition_p(x, y, z);
				m_object->setTransform_o2p(transform);
			}
			break;
		case ParticleEffectTransformEdit::OM_lockToXZUseWorldY:
			{
				float const x = Game::getPlayer()->getPosition_w().x;
				float const y = m_objectTransform.getPosition_p().y;
				float const z = Game::getPlayer()->getPosition_w().z;

				Transform transform;
				transform.setPosition_p(x, y, z);
				m_object->setTransform_o2p(transform);
			}
			break;
		case ParticleEffectTransformEdit::OM_lockToXZUsePlayerRelativeY:
			{
				float const x = Game::getPlayer()->getPosition_w().x;
				float const y = Game::getPlayer()->getPosition_w().y + m_objectTransform.getPosition_p().y;
				float const z = Game::getPlayer()->getPosition_w().z;

				Transform transform;
				transform.setPosition_p(x, y, z);
				m_object->setTransform_o2p(transform);
			}
			break;
		case ParticleEffectTransformEdit::OM_spawnToXZUseWorldY:
			{
				ParticleEffectAppearance *particleEffectAppearance = dynamic_cast<ParticleEffectAppearance *>(m_object->getAppearance());
				NOT_NULL(particleEffectAppearance);

				if (particleEffectAppearance->isDeletable())
				{
					float const x = Game::getPlayer()->getPosition_w().x;
					float const y = m_objectTransform.getPosition_p().y;
					float const z = Game::getPlayer()->getPosition_w().z;

					Transform transform;
					transform.setPosition_p(x, y, z);
					m_object->setTransform_o2p(transform);
				}
			}
			break;
		case ParticleEffectTransformEdit::OM_spawnToXZUsePlayerRelativeY:
			{
				ParticleEffectAppearance *particleEffectAppearance = dynamic_cast<ParticleEffectAppearance *>(m_object->getAppearance());
				NOT_NULL(particleEffectAppearance);

				if (particleEffectAppearance->isDeletable())
				{
					float const x = Game::getPlayer()->getPosition_w().x;
					float const y = Game::getPlayer()->getPosition_w().y + m_objectTransform.getPosition_p().y;
					float const z = Game::getPlayer()->getPosition_w().z;

					Transform transform;
					transform.setPosition_p(x, y, z);
					m_object->setTransform_o2p(transform);
				}
			}
			break;
	}
}

//-----------------------------------------------------------------------------
void ParticleEditorIoWin::setObjectTransform(Transform const &transform)
{
	m_objectTransform = transform;

	setObjectMovement(0.0f);
}

//-----------------------------------------------------------------------------
void ParticleEditorIoWin::setPlayBackRate(float const playBackRate)
{
	if (m_object != NULL)
	{
		ParticleEffectAppearance *particleEffectAppearance = dynamic_cast<ParticleEffectAppearance *>(m_object->getAppearance());
		NOT_NULL(particleEffectAppearance);

		m_playBackRate = playBackRate;

		particleEffectAppearance->setPlayBackRate(playBackRate);
	}
}

//-----------------------------------------------------------------------------
void ParticleEditorIoWin::setEffectScale(float const effectScale)
{
	if (m_object != NULL)
	{
		ParticleEffectAppearance *particleEffectAppearance = dynamic_cast<ParticleEffectAppearance *>(m_object->getAppearance());
		NOT_NULL(particleEffectAppearance);

		m_effectScale = effectScale;

		particleEffectAppearance->setUniformScale(effectScale);
	}
}

//-----------------------------------------------------------------------------
void ParticleEditorIoWin::setObjectMovement(ParticleEffectTransformEdit::ObjectMovement const objectMovement)
{
	m_objectMovement = objectMovement;

	setObjectMovement(0.0f);

	if (objectMovement == ParticleEffectTransformEdit::OM_spawnToXZUseWorldY)
	{
		ParticleEffectAppearance *particleEffectAppearance = dynamic_cast<ParticleEffectAppearance *>(m_object->getAppearance());
		NOT_NULL(particleEffectAppearance);

		Transform transform;

		float const x = Game::getPlayer()->getPosition_w().x;
		float const y = m_objectTransform.getPosition_p().y;
		float const z = Game::getPlayer()->getPosition_w().z;

		transform.setPosition_p(x, y, z);

		m_object->setTransform_o2p(transform);
	}

	m_particleEditor->restartParticleSystem();
}

//-----------------------------------------------------------------------------
Object const * const ParticleEditorIoWin::getObject() const
{
	return m_object;
}

//-----------------------------------------------------------------------------
void ParticleEditorIoWin::setTimeOfDayCycle(bool const timeOfDayCycle)
{

	TerrainObject * const terrainObject = TerrainObject::getInstance();
		
	if (terrainObject != NULL)
	{
		terrainObject->setPauseEnvironment(!timeOfDayCycle);
	}

}

//-----------------------------------------------------------------------------

void ParticleEditorIoWin::setSize(float size)
{
	m_size = size;
}

//-----------------------------------------------------------------------------

void ParticleEditorIoWin::setSpeed(float speed)
{
	m_speed = speed;
}

//-----------------------------------------------------------------------------
void ParticleEditorIoWin::alter(float const deltaTime)
{
	// Position the object

	setObjectMovement(deltaTime);

	// See if we can restart the emitter

	ParticleEffectAppearance *particleEffectAppearance = dynamic_cast<ParticleEffectAppearance *>(m_object->getAppearance());
	NOT_NULL(particleEffectAppearance);

	if (particleEffectAppearance->isDeletable())
	{
		m_particleEditor->restartParticleSystem();
	}
}

//=============================================================================
