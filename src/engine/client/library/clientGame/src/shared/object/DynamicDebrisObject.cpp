//======================================================================
//
// DynamicDebrisObject.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/DynamicDebrisObject.h"

#include "clientGame/ClientEffect.h"
#include "clientGame/ClientEffectTemplateList.h"
#include "clientGame/ClientObject.h"
#include "clientGame/ClientWorld.h"
#include "clientGame/ContainerInterface.h"
#include "clientGame/Game.h"
#include "clientGame/ShipObject.h"
#include "clientGraphics/Camera.h"
#include "clientGraphics/DebugPrimitive.h"
#include "clientGraphics/RenderWorld.h"
#include "clientObject/ComponentAppearance.h"
#include "clientObject/DynamicMeshAppearance.h"
#include "clientObject/HardpointObject.h"
#include "clientParticle/ParticleEffectAppearance.h"
#include "clientParticle/SwooshAppearance.h"
#include "sharedCollision/CollideParameters.h"
#include "sharedCollision/CollisionInfo.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/PerformanceTimer.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedMath/Plane.h"
#include "sharedMath/Sphere.h"
#include "sharedMath/VectorArgb.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/ObjectList.h"

// ======================================================================
// STATIC PUBLIC DynamicDebrisObject
// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL (DynamicDebrisObject, true, 0, 0, 0);

// ======================================================================
// PUBLIC DynamicDebrisObject
// ======================================================================

//----------------------------------------------------------------------

namespace DynamicDebrisObjectNamespace
{
	const float s_fadeoutTime = 1.0f;
	const float s_minBounceSpeed = 10.0f;

	bool s_flagsInstalled = false;
	bool s_showTangibleExtent = false;
	bool s_showFallTowardFrame = false;
	bool s_copyNonSplittableChildren = false;

	void flagsInstall()
	{
		if (s_flagsInstalled)
			return;

		s_flagsInstalled = true;
		DebugFlags::registerFlag(s_showTangibleExtent, "ClientObject/DynamicDebrisObject", "showTangibleExtent");
		DebugFlags::registerFlag(s_showFallTowardFrame, "ClientObject/DynamicDebrisObject", "showFallTowardFrame");
	}

	//----------------------------------------------------------------------

	Appearance * duplicateAppearance(Appearance const & appearance)
	{
		//-- don't duplicate skeletal appearances
		if (appearance.asSkeletalAppearance2())
			return NULL;

		AppearanceTemplate const * const appearanceTemplate = appearance.getAppearanceTemplate();
		if (appearanceTemplate != NULL)
		{
			Appearance * const newAppearance = appearanceTemplate->createAppearance();

			if (newAppearance != NULL)
			{
				ParticleEffectAppearance const * const particleAppearance = ParticleEffectAppearance::asParticleEffectAppearance(&appearance);

				if (particleAppearance != NULL)
				{
					ParticleEffectAppearance * const newParticleAppearance = ParticleEffectAppearance::asParticleEffectAppearance(newAppearance);
					if (newParticleAppearance != NULL)
					{
						newParticleAppearance->setUniformScale(particleAppearance->getUniformScale());
						newParticleAppearance->setColorModifier(particleAppearance->getColorModifier());
					}
				}

				return newAppearance;
			}
		}

		return NULL;
	}

	//----------------------------------------------------------------------

	void setAlphaFadeOnObjectAndChildren(Object & obj, float alphaFade)
	{
		DynamicMeshAppearance * const dynamicMeshAppearance = dynamic_cast<DynamicMeshAppearance *>(obj.getAppearance());
		if (dynamicMeshAppearance != NULL)
			dynamicMeshAppearance->setAlphaFade(alphaFade);

		int const numberOfChildren = obj.getNumberOfChildObjects();

		for (int i = 0; i < numberOfChildren; ++i)
		{
			Object * const child = obj.getChildObject(i);
			if (NULL != child)
				setAlphaFadeOnObjectAndChildren(*child, alphaFade);
		}
	}

	//----------------------------------------------------------------------
}

using namespace DynamicDebrisObjectNamespace;

//----------------------------------------------------------------------

DynamicDebrisObject::DynamicDebrisObject () :
Object(),
m_currentVelocity_w(),
m_rotationYawPitchRoll(),
m_lastPositionVsPlayer_o(Vector::zero),
m_skipPlayerCollision(true), // skip collision the first frame, because the object is "moving" from an undefined position
m_lifeTimer(10.0f),
m_fallTowardObject(NULL),
m_keepalive(false)
{
	if (!s_flagsInstalled)
		flagsInstall();
}

//----------------------------------------------------------------------

DynamicDebrisObject::~DynamicDebrisObject ()
{
}

//----------------------------------------------------------------------

float DynamicDebrisObject::alter (float elapsedTime)
{
	float alterResult = Object::alter(elapsedTime);

	//-- if this is a topmost parent, do some physics
	if (!isChildObject())
	{
		Transform const & t_o2w = getTransform_o2w();
		Vector const & currentVelocity_o = t_o2w.rotate_p2l(m_currentVelocity_w);
		move_o(currentVelocity_o * elapsedTime);

		AxialBox const & tangibleExtent = getTangibleExtent();

		Transform t_o2p = getTransform_o2p();
		t_o2p.move_l(tangibleExtent.getCenter());
		t_o2p.yaw_l(m_rotationYawPitchRoll.x * elapsedTime);
		t_o2p.pitch_l(m_rotationYawPitchRoll.y * elapsedTime);
		t_o2p.roll_l(m_rotationYawPitchRoll.z * elapsedTime);
		t_o2p.move_l(-tangibleExtent.getCenter());

		setTransform_o2p(t_o2p);

#ifdef _DEBUG
		if (s_showTangibleExtent)
		{
			const Camera * const camera = Game::getCamera ();
			if (camera != NULL)
				camera->addDebugPrimitive (new BoxDebugPrimitive(UtilityDebugPrimitive::S_z, getTransform_o2w(), tangibleExtent, VectorArgb::solidRed));
		}
#endif

		//--
		//-- find something to fall toward (cap ship or space station)
		//--

		if (m_fallTowardObject == NULL)
			alterFindFallTowardTarget();

		//--
		//-- fall toward our target
		//--
		
		if (m_fallTowardObject != NULL)
		{
			float const alterResultFall = alterFallTowardTarget(elapsedTime);
			
			if (alterResultFall == AlterResult::cms_kill) //lint !e777
				return  alterResultFall;
		}
		
		//--
		//-- check for collision with the player ship
		//--
		
		alterCheckForPlayerShipCollision();
		
		m_lifeTimer -= elapsedTime;
		
		if (!m_keepalive)
		{
			if (m_lifeTimer < 0.0f)
			{
				return AlterResult::cms_kill;
			}
			
			for (int i = 0; i < getNumberOfChildObjects(); ++i)
			{
				Object * const childObject = NON_NULL(getChildObject(i));
				DynamicDebrisObject * const dynamicDebrisChild = dynamic_cast<DynamicDebrisObject *>(childObject);
				if (NULL != dynamicDebrisChild)
				{
					dynamicDebrisChild->m_lifeTimer = m_lifeTimer;
				}
			}
		}
	}
	
	if (!m_keepalive)
	{
		if (m_lifeTimer < s_fadeoutTime)
		{
			float const alphaFade = std::max(0.0f, m_lifeTimer) / s_fadeoutTime;
			setAlphaFadeOnObjectAndChildren(*this, alphaFade);
		}
	}
	
	AlterResult::incorporateAlterResult(alterResult, AlterResult::cms_alterNextFrame);
	return alterResult;
}

//----------------------------------------------------------------------

void DynamicDebrisObject::setPhysicsParameters(float timeToLive, Vector const & initialVelocity_w, Vector const & rotationYawPitchRoll)
{
	m_lifeTimer = timeToLive;
	m_currentVelocity_w = initialVelocity_w;
	m_rotationYawPitchRoll = rotationYawPitchRoll;
}

//----------------------------------------------------------------------

bool DynamicDebrisObject::splitObjects(Object & objectToSplit, Plane const & plane, DynamicDebrisObject *& front, DynamicDebrisObject *& back, float const debrisParticleThreshold)
{
	{
		Appearance const * const appearance = objectToSplit.getAppearance();
		if (appearance != NULL)
		{
			float const appearanceRadius = objectToSplit.getTangibleExtent().getRadius();
			float const randomizationMagnitude = std::min(20.0f, appearanceRadius * 0.01f);
			
			Vector const randomizationVector(randomizationMagnitude, randomizationMagnitude, randomizationMagnitude);
			
			DynamicMeshAppearance * dynamicMeshAppearanceFront = NULL;
			DynamicMeshAppearance * dynamicMeshAppearanceBack = NULL;
			
			Object * objFront = NULL;
			Object * objBack = NULL;
			
			if (!DynamicMeshAppearance::splitAppearance(*appearance, plane, randomizationVector, objFront, objBack, dynamicMeshAppearanceFront, dynamicMeshAppearanceBack, debrisParticleThreshold))
			{
				DEBUG_FATAL(NULL != objFront || NULL != objBack, ("DynamicDebrisObject split returned false but objs exist"));
				DEBUG_FATAL(NULL != dynamicMeshAppearanceFront || NULL != dynamicMeshAppearanceBack, ("DynamicDebrisObject split returned false but appearances exist"));
				return false;
			}
			
			if (dynamicMeshAppearanceFront != NULL)
			{
				DEBUG_FATAL(NULL != objFront, ("DynamicDebrisObject returned front appearance as well as object"));
				front = new DynamicDebrisObject;
				front->setAppearance(dynamicMeshAppearanceFront);
				DynamicMeshAppearance::attachDebrisParticles(*front, -plane.getNormal(), debrisParticleThreshold);
			}
			
			if (dynamicMeshAppearanceBack != NULL)
			{
				DEBUG_FATAL(NULL != objBack, ("DynamicDebrisObject returned back appearance as well as object"));
				back = new DynamicDebrisObject;
				back->setAppearance(dynamicMeshAppearanceBack);
				DynamicMeshAppearance::attachDebrisParticles(*back, plane.getNormal(), debrisParticleThreshold);
			}
			
			if (objFront != NULL)
			{
				front = new DynamicDebrisObject;
				front->addChildObject_o(objFront);
			}
			
			if (objBack != NULL)
			{
				back = new DynamicDebrisObject;
				back->addChildObject_o(objBack);
			}
		}
	}
		
	for (int i = 0; i < objectToSplit.getNumberOfChildObjects(); ++i)
	{
		Object * const childObject = NON_NULL(objectToSplit.getChildObject(i));
		Appearance * const childAppearance = childObject->getAppearance();
		if (childAppearance == NULL)
			continue;

		//-- child object is being killed, skip it
		if (childObject->getKill())
			continue;
		
		Vector const & planePoint = plane.getNormal() * (-plane.getD());
		Plane const childPlane (childObject->rotate_p2o(plane.getNormal()), childObject->rotateTranslate_p2o(planePoint));
		DynamicDebrisObject * childFront = NULL;
		DynamicDebrisObject * childBack = NULL;
		
		if (DynamicDebrisObject::splitObjects(*childObject, childPlane, childFront, childBack, debrisParticleThreshold))
		{
			if (childFront != NULL)
			{
				if (front != NULL)
				{
					front->addChildObject_o(childFront);
					childFront->setTransform_o2p(childObject->getTransform_o2p());
					RenderWorld::addObjectNotifications (*childFront);
				}
				else
					front = childFront;
			}
			
			if (childBack != NULL)
			{
				if (back != NULL)
				{
					back->addChildObject_o(childBack);
					childBack->setTransform_o2p(childObject->getTransform_o2p());
					RenderWorld::addObjectNotifications (*childBack);
				}
				else
					back = childBack;
			}
		}
		//-- the child could not be split
		//-- this means it is some kind of effect like particles, swooshes, glows, etc...
		//-- let's just attach the object as a child here
		else
		{
			Object * attachTo = NULL;

			if (plane.computeDistanceTo(childObject->getPosition_p()) > 0.0f)
				attachTo = front;
			else
				attachTo = back;
			
			if (attachTo)
			{
				if (!s_copyNonSplittableChildren)
				{
					ParticleEffectAppearance * const pea = ParticleEffectAppearance::asParticleEffectAppearance(childAppearance);
					SwooshAppearance * const swoosh = SwooshAppearance::asSwooshAppearance(childAppearance);

					//-- don't move swooshes, they usually depend on hardpoints
					if (NULL != swoosh)
					{
						continue;
					}

					ClientObject * const clientChildObject = childObject->asClientObject();
					if (clientChildObject)
					{
						//-- don't move creature objects, including the player
						if (clientChildObject->asCreatureObject())
							continue;
					}

					if (NULL != pea)
						pea->setRestartOnRemoveFromWorld(false);

					Transform const & t = childObject->getTransform_o2p();
					objectToSplit.removeChildObject(childObject, Object::DF_none);
					attachTo->addChildObject_o(childObject);

					childObject->setTransform_o2p(t);

					if (NULL != pea)
						pea->setRestartOnRemoveFromWorld(true);

					//-- hardpoint objects will no longer function because dynamic debris objects don't have hardpoints.
					HardpointObject * const hardpointObject = (NULL == clientChildObject) ? dynamic_cast<HardpointObject *>(childObject) : NULL;
					if (NULL != hardpointObject)
					{
						hardpointObject->setHardpointName(PersistentCrcString::empty);
					}

					//-- keep iterator at current location
					--i;
				}
				else
				{
					Appearance * const newAppearance = duplicateAppearance(*childAppearance);
					if (newAppearance != NULL)
					{
						Object * const newObject = new Object;
						newObject->setAppearance(newAppearance);
						newObject->setTransform_o2p(childObject->getTransform_o2p());
						RenderWorld::addObjectNotifications (*newObject);
						attachTo->addChildObject_o(newObject);
					} //lint !e429 //newObject owned by parent
				}
			}
		}
	}
	
	return true;
}

//----------------------------------------------------------------------

bool DynamicDebrisObject::splitObjects(Object & objectToSplit, PlaneVector const & planes, DynamicDebrisObjectVector & results, float const debrisParticleThreshold)
{
	if (planes.empty())
		return false;
			
	PerformanceTimer ptimer;
	ptimer.start();	

	PlaneVector::const_iterator plane_it = planes.begin();

	//-- first cut
	{
		Plane const & plane = *plane_it;
		
		DynamicDebrisObject * newDynamicDebrisObject[2] = {NULL, NULL};
		if (DynamicDebrisObject::splitObjects(objectToSplit, plane, newDynamicDebrisObject[0], newDynamicDebrisObject[1], debrisParticleThreshold))
		{
			if (newDynamicDebrisObject[0] != NULL)
				results.push_back(newDynamicDebrisObject[0]);
			if (newDynamicDebrisObject[1] != NULL)
				results.push_back(newDynamicDebrisObject[1]);
		}
	}
	
	//-- recursively cut
	for (++plane_it; plane_it != planes.end(); ++plane_it)
	{
		Plane const & plane = *plane_it;
		
		DynamicDebrisObjectVector resultsToAdd;
		
		for (DynamicDebrisObjectVector::iterator it = results.begin(); it != results.end(); ++it)
		{
			DynamicDebrisObject * const dynamicDebrisObject = *it;
			if (dynamicDebrisObject == NULL)
				continue;
			
			DynamicDebrisObject * newDynamicDebrisObject[2] = {NULL, NULL};
			if (DynamicDebrisObject::splitObjects(*dynamicDebrisObject, plane, newDynamicDebrisObject[0], newDynamicDebrisObject[1], debrisParticleThreshold))
			{
				if (newDynamicDebrisObject[0] != NULL)
					resultsToAdd.push_back(newDynamicDebrisObject[0]);
				if (newDynamicDebrisObject[1] != NULL)
					resultsToAdd.push_back(newDynamicDebrisObject[1]);
				
				//-- this  mesh appearance has been subdivided, so get rid of it
				delete dynamicDebrisObject;
				(*it) = NULL;
			}
		}
		
		results.insert(results.end(), resultsToAdd.begin(), resultsToAdd.end());
	}
	
	ptimer.stop();

	return true;
}

//----------------------------------------------------------------------

void DynamicDebrisObject::alterFindFallTowardTarget()
{
	if (m_lifeTimer <= 0.0f)
		return;

	m_fallTowardObject = NULL;
	
	static ClientWorld::ObjectVector objectVector;
	objectVector.clear();
	
	Vector const position_w = getPosition_w();
	float const distanceCanTravel = m_currentVelocity_w.magnitude() * m_lifeTimer;

	//-- only look for objects that can be hit within half of our lifetime potential travel distance
	ClientWorld::findObjectsInRange (position_w, distanceCanTravel * 0.5f, objectVector);
	
	float closestObjectDistanceSquared = 1000000.0f;
	
	for (ClientWorld::ObjectVector::const_iterator it = objectVector.begin(); it != objectVector.end(); ++it)
	{
		Object const * const obj = *it;
		ClientObject const * const clientObject = (obj != NULL) ? obj->asClientObject() : NULL;
		
		if (clientObject == NULL)
			continue;

		if (clientObject->getKill())
			continue;
		
		int const gameObjectType = clientObject->getGameObjectType();
		
		//-- only fall toward cap ships & space stations
		
		if (gameObjectType == SharedObjectTemplate::GOT_ship_capital || gameObjectType == SharedObjectTemplate::GOT_ship_station)
		{
			float distanceToObjectSquared = clientObject->getPosition_w().magnitudeBetweenSquared(position_w);
			
			if (distanceToObjectSquared < closestObjectDistanceSquared)
			{
				Vector const & vectorToTarget_w = clientObject->getPosition_w() - position_w;
				
				//-- things behind us are likely ignored, if something is closer
				if (m_currentVelocity_w.dot(vectorToTarget_w) < 0.0f)
					distanceToObjectSquared *= 2.0f;
				
				if (distanceToObjectSquared < closestObjectDistanceSquared)
				{
					closestObjectDistanceSquared = distanceToObjectSquared;
					m_fallTowardObject = clientObject;
				}
			}
		}
	}
}

//----------------------------------------------------------------------

float DynamicDebrisObject::alterFallTowardTarget(float const elapsedTimeSecs)
{
	Object const * const fallTowardObject = m_fallTowardObject.getPointer();
	
	if (fallTowardObject == NULL)
		return AlterResult::cms_alterNextFrame;
	
	Appearance const * const fallTowardAppearance = fallTowardObject->getAppearance();
	
	if (fallTowardAppearance == NULL)
		return AlterResult::cms_alterNextFrame;
	
	Vector const position_w = getPosition_w();

	Vector m_currentVelocity_w_normalized = m_currentVelocity_w;
	IGNORE_RETURN(m_currentVelocity_w_normalized.normalize());
	
	//-- check for collision with the target
	//-- cast a ray from the appearance position of the debris last frame to the 'front' of the debris appearance sphere this frame

	Vector const & position_a_w = getAppearanceSphereCenter_w();
	Vector const & position_o = fallTowardObject->rotateTranslate_w2o(position_a_w - (m_currentVelocity_w * elapsedTimeSecs));
	Vector const & end_o = fallTowardObject->rotateTranslate_w2o(position_a_w + (m_currentVelocity_w_normalized * getAppearanceSphereRadius()));
	
	CollisionInfo result;
	if (fallTowardAppearance->collide(position_o, end_o, CollideParameters::cms_default, result))
	{
		//@todo: data-drive & precache these clienteffects

		//-- explosion for debris destruction
		{
			AxialBox const & tangibleExtent = getTangibleExtent();
			Vector const & tangibleExtentCenter_w = rotateTranslate_o2p(tangibleExtent.getCenter());
			ClientEffect * const clientEffect = ClientEffectTemplateList::createClientEffect(ConstCharCrcLowerString("clienteffect/combat_ship_hit_death.cef"), getParentCell(), tangibleExtentCenter_w, getObjectFrameK_p());
			if (clientEffect != NULL)
			{
				float const sphereRadius = tangibleExtent.getRadius();
				if (sphereRadius > 50.0f)
					clientEffect->setUniformScale(sphereRadius / 50.0f);
				clientEffect->execute();
				delete clientEffect;
			}
		}
		
		//-- explosion for impact
		{
			ClientEffect * const clientEffect = ClientEffectTemplateList::createClientEffect(ConstCharCrcLowerString("clienteffect/combat_ship_hit_component.cef"), getParentCell(), result.getPoint(), getObjectFrameK_p());
			if (clientEffect != NULL)
			{
				clientEffect->execute();
				delete clientEffect;
			}
		}
		
		return AlterResult::cms_kill;
	}
	
	BoxExtent const * const boxExtent = dynamic_cast<BoxExtent const *>(fallTowardAppearance->getExtent());
	if (boxExtent == NULL)
		return AlterResult::cms_alterNextFrame;
	
	//--
	//-- fall toward an appropriate point on the target
	//--

	float const width = boxExtent->getWidth();
	float const height = boxExtent->getHeight();
	float const length = boxExtent->getLength();
	
	Vector const & longAxis = (width > height && width > length) ? (Vector::unitX * (width * 0.45f)) : ((length > height && length > width) ? (Vector::unitZ * (length * 0.45f)): Vector::unitY * (height * 0.45f));
	
	Vector const & axisStart_w = fallTowardObject->rotateTranslate_o2w(boxExtent->getCenter() -longAxis);
	Vector const & axisEnd_w = fallTowardObject->rotateTranslate_o2w(boxExtent->getCenter() + longAxis);
	
	//-- fall toward the closest point along the longest axis of the target
	//-- this is the closest point to our origin's predicted position when one quarter of the debris remaining life is expended
	Vector const & predictedDebrisPoint_w = position_w + (m_currentVelocity_w * (m_lifeTimer * 0.25f));
	Vector const & pointToFallToward_w = predictedDebrisPoint_w.findClosestPointOnLineSegment(axisStart_w, axisEnd_w);
	
	Vector vectorToTarget_w_normalized = pointToFallToward_w - position_w;
	
	//-- if we aren't already moving in roughly the correct direction, don't do anything
	if (m_currentVelocity_w_normalized.dot(vectorToTarget_w_normalized) > 0.3f)
	{
#ifdef _DEBUG
		if (s_showFallTowardFrame)
		{
			const Camera * const camera = Game::getCamera ();
			if (camera != NULL)
			{
				Transform fallTowardTransform_o2w = Transform::identity;
				fallTowardTransform_o2w.move_l(pointToFallToward_w);
				camera->addDebugPrimitive(new FrameDebugPrimitive(UtilityDebugPrimitive::S_z, fallTowardTransform_o2w, 40.0f));
				camera->addDebugPrimitive(new Line3dDebugPrimitive(Line3dDebugPrimitive::S_z, Transform::identity, position_w, pointToFallToward_w, VectorArgb::solidWhite));
			}
		}
#endif
		
		IGNORE_RETURN(vectorToTarget_w_normalized.normalize());
		vectorToTarget_w_normalized *= m_currentVelocity_w.magnitude();
		
		static float const s_velocityInterpolationRate = 0.3f;
		m_currentVelocity_w = Vector::linearInterpolate(m_currentVelocity_w, vectorToTarget_w_normalized, std::min(elapsedTimeSecs * s_velocityInterpolationRate, 1.0f));
	}

	return AlterResult::cms_alterNextFrame;
}

//----------------------------------------------------------------------

void DynamicDebrisObject::alterCheckForPlayerShipCollision()
{
	Object* player = Game::getPlayer();
	if (player)
	{
		Object* topmost = ContainerInterface::getTopmostContainer(*player, false);
		if(topmost)
		{
			AxialBox const collisionBox = topmost->getTangibleExtent();
			BoxExtent const extent(collisionBox);
			
			Vector m_currentVelocity_w_normalized = m_currentVelocity_w;
			IGNORE_RETURN(m_currentVelocity_w_normalized.normalize());
			
			Vector const & position_a_w = getAppearanceSphereCenter_w();
			Vector const & end_o = topmost->rotateTranslate_w2o(position_a_w); 
			
			float time;
			Vector normal_o;
			if (extent.intersect(m_lastPositionVsPlayer_o, end_o, &normal_o, &time))
			{
				if (!m_skipPlayerCollision)
				{
					Vector hitPoint_o(Vector::linearInterpolate(m_lastPositionVsPlayer_o, end_o, time));

					ClientEffect * const clientEffect = ClientEffectTemplateList::createClientEffect(ConstCharCrcLowerString("clienteffect/combat_ship_hit_component.cef"), getParentCell(), getPosition_w(), getObjectFrameK_p());
					if (clientEffect != NULL)
					{
						clientEffect->execute();
						delete clientEffect;
					}

					//-- bounce off the player's ship and fly away (cancelling the "fall to" effect temporarily)				
					// TODO: If the piece is large, bounce the player's ship off the piece instead instead
					
					m_fallTowardObject = NULL;
					setPosition_w(topmost->rotateTranslate_o2w(hitPoint_o));
					Vector newDirection_o = normal_o.reflectOutgoing(m_lastPositionVsPlayer_o - hitPoint_o);
					IGNORE_RETURN(newDirection_o.normalize());
					if (topmost->asClientObject() && topmost->asClientObject()->asShipObject())
						m_currentVelocity_w = newDirection_o * std::max(s_minBounceSpeed, topmost->asClientObject()->asShipObject()->getCurrentSpeed() * 1.2f);
					else
						m_currentVelocity_w = newDirection_o * std::max(s_minBounceSpeed, m_currentVelocity_w.magnitude() * 1.2f);
					m_rotationYawPitchRoll=Vector(Random::randomReal(-2.0f, 2.0f),Random::randomReal(-2.0f, 2.0f),Random::randomReal(-2.0f, 2.0f));
				}
				m_skipPlayerCollision = true; // skip collision next frame, so that the debris can't get trapped in the ship
			}
			else
				m_skipPlayerCollision = false;

			m_lastPositionVsPlayer_o = end_o; // topmost->rotateTranslate_w2o(position_a_w);
		}
	}
}

//----------------------------------------------------------------------

Vector const & DynamicDebrisObject::getCurrentVelocity_w() const
{
	return m_currentVelocity_w;
}

//----------------------------------------------------------------------

void DynamicDebrisObject::setCurrentVelocity_w(Vector const & velocity_w)
{
	m_currentVelocity_w = velocity_w;
}

//----------------------------------------------------------------------

Vector const & DynamicDebrisObject::getCurrentYawPitchRollRates() const
{
	return m_rotationYawPitchRoll;
}

//----------------------------------------------------------------------

void DynamicDebrisObject::setCurrentYawPitchRollRates(Vector const & yawPitchRoll)
{
	m_rotationYawPitchRoll = yawPitchRoll;
}

//----------------------------------------------------------------------

float DynamicDebrisObject::getLifeTimer() const
{
	return m_lifeTimer;
}

//----------------------------------------------------------------------

void DynamicDebrisObject::setKeepalive(bool keepalive)
{
	m_keepalive = keepalive;
}

//----------------------------------------------------------------------

void DynamicDebrisObject::setLifeTimerToStartFade()
{
	m_lifeTimer = s_fadeoutTime;
}

//----------------------------------------------------------------------

bool DynamicDebrisObject::isKeepalive() const
{
	return m_keepalive;
}

//======================================================================
