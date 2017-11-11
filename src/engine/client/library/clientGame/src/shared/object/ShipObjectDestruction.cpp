//======================================================================
//
// ShipObjectDestruction.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ShipObjectDestruction.h"

#include "clientAudio/Audio.h"
#include "clientGame/ClientDataFile.h"
#include "clientGame/ClientDataFile_DestructionSequence.h"
#include "clientGame/ClientEffect.h"
#include "clientGame/ClientEffectManager.h"
#include "clientGame/ClientEffectTemplate.h"
#include "clientGame/ClientEffectTemplateList.h"
#include "clientGame/DynamicDebrisObject.h"
#include "clientGame/Game.h"
#include "clientGame/ShipObject.h"
#include "clientGraphics/Camera.h"
#include "clientGraphics/DebugPrimitive.h"
#include "clientGraphics/RenderWorld.h"
#include "clientObject/DynamicMeshAppearance.h"
#include "clientUserInterface/CuiPreferences.h"
#include "sharedCollision/CollideParameters.h"
#include "sharedCollision/CollisionInfo.h"
#include "sharedCollision/CollisionProperty.h"
#include "sharedCollision/CollisionWorld.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedMath/AxialBox.h"
#include "sharedMath/Plane.h"
#include "sharedMath/VectorArgb.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/Appearance.h"
#include "sharedRandom/Random.h"

//======================================================================

namespace ShipObjectDestructionNamespace
{
	Vector s_lastLine0;
	Vector s_lastLine1;
	Vector s_lastHit;

	float const s_objectRadiusExplosionCountFactor = 1.0f;

	Plane const generateRandomPlane(Object const & object)
	{
		AxialBox const & tangibleExtent = object.getTangibleExtent();
		float const tangibleExtentRadius = tangibleExtent.getRadius();
		Vector const & tangibleExtentCenter = tangibleExtent.getCenter();
		Vector const & planePosition = tangibleExtentCenter + (Vector::randomUnit() * (tangibleExtentRadius * (Random::randomReal(0.1f, 0.75f))));
		return Plane(Vector::randomUnit(), planePosition);
	}

	bool s_showSplitExplosion = true;
	bool s_showDebugExplosionLines = false;
	bool s_showDebugSplitPlanes = false;
	bool s_randomizePlanes = false;
	bool s_playAmbientLoop = true;
	bool s_playSequenceExplosion = true;

	float s_pushSpeedFactor = 0.0f;
	float s_pushRotationFactor = 0.0f;
	
	float const s_shipRadiusDebrisThreshold = 0.3f;

	//----------------------------------------------------------------------

	bool s_installed = false;
	void remove();
}

using namespace ShipObjectDestructionNamespace;

//----------------------------------------------------------------------
//----------------------------------------------------------------------
//----------------------------------------------------------------------

class ShipObjectDestruction::ObjectData
{
public:

	//lint -e1925 // public data members
	class SplitData
	{
	public:
		void computeNextExplosionTime(float randomStartMin, float randomStartMax);
		bool playExplosion(Object & object);
	private:
		bool playExplosion(Object & object, Plane const & plane_o);

	public:
		Plane m_splitPlane;
		float m_timeUntilSplit;
		float m_totalTimeUntilSplit;
		float m_timeUntilNextExplosion;
		float m_averageTimeBetweenExplosions;
		ShipObjectDestruction * m_shipObjectDestruction;
	};
	
	ObjectData();
	~ObjectData();
	ObjectData(ShipObjectDestruction & shipObjectDestruction, Object & object);
	void addSplitData(Plane const & splitPlane, float totalTimeUntilSplit);
	void addSplitDataFrom(ObjectData const & rhs, int begin);
	bool alter(float elapsedTime, Object *& front, Object *& back, float roll, float pitch);
	bool splitObject(Object & object, Plane const & plane, Object *& front, Object *& back, float roll, float pitch) const;
	int getSplitDataCount() const;	

	Object * getObject() const;

private:
	ObjectWatcher m_object;

	typedef stdvector<SplitData>::fwd SplitDataVector;
	SplitDataVector m_splitDataVector;
	ShipObjectDestruction * m_shipObjectDestruction;
};

//----------------------------------------------------------------------
//----------------------------------------------------------------------
//----------------------------------------------------------------------

ShipObjectDestruction::ObjectData::ObjectData() :
m_object(NULL),
m_splitDataVector(),
m_shipObjectDestruction(NULL)
{
}

//----------------------------------------------------------------------

ShipObjectDestruction::ObjectData::ObjectData(ShipObjectDestruction & shipObjectDestruction, Object & object) :
m_object(&object),
m_splitDataVector(),
m_shipObjectDestruction(&shipObjectDestruction)
{

}

//----------------------------------------------------------------------

ShipObjectDestruction::ObjectData::~ObjectData()
{
	m_object = NULL;
	m_splitDataVector.clear();
	m_shipObjectDestruction = NULL;
}

//----------------------------------------------------------------------

int ShipObjectDestruction::ObjectData::getSplitDataCount() const
{
	return static_cast<int>(m_splitDataVector.size());
}

//----------------------------------------------------------------------

void ShipObjectDestruction::ObjectData::addSplitData(Plane const & splitPlane, float totalTimeUntilSplit)
{
	m_splitDataVector.resize(m_splitDataVector.size() + 1);

	SplitData & splitData = m_splitDataVector.back();

	{
		totalTimeUntilSplit = clamp(0.1f, totalTimeUntilSplit, 20.0f);
		
		splitData.m_splitPlane = splitPlane;
		splitData.m_totalTimeUntilSplit = totalTimeUntilSplit;
		splitData.m_timeUntilSplit = totalTimeUntilSplit;
		splitData.m_shipObjectDestruction = m_shipObjectDestruction;
	}
}

//----------------------------------------------------------------------

void ShipObjectDestruction::ObjectData::addSplitDataFrom(ObjectData const & rhs, int begin)
{
	m_splitDataVector.clear();

	size_t const numSplitData = rhs.m_splitDataVector.size();
	if (numSplitData <= static_cast<size_t>(begin))
		return;

	AxialBox const & tangibleExtent = m_object->getTangibleExtent();
	float const tangibleExtentRadius = tangibleExtent.getRadius();
	Vector const & tangibleExtentCenter_o = tangibleExtent.getCenter();

	m_splitDataVector.reserve(numSplitData - static_cast<size_t>(begin));
	m_splitDataVector.insert(m_splitDataVector.end(), rhs.m_splitDataVector.begin() + begin, rhs.m_splitDataVector.end());
	
	{
		for (SplitDataVector::iterator it = m_splitDataVector.begin(); it != m_splitDataVector.end(); ++it)
		{
			SplitData & splitData = *it;
			
			if (NULL != m_object.getPointer())
			{
				float const approximateTotalExplosions = tangibleExtentRadius * s_objectRadiusExplosionCountFactor;
				
				if (approximateTotalExplosions > 1.0f)
				{
					splitData.m_averageTimeBetweenExplosions = splitData.m_totalTimeUntilSplit / approximateTotalExplosions;
				}
			}
			
			splitData.computeNextExplosionTime(0.0f, splitData.m_totalTimeUntilSplit * 0.2f);
		}
	}


	while (!m_splitDataVector.empty())
	{
		SplitData & splitData = m_splitDataVector.front();

		float const distanceFromPlane = splitData.m_splitPlane.computeDistanceTo(tangibleExtentCenter_o);

		//-- the plane does not intersect the object, skip it
		if (distanceFromPlane > tangibleExtentRadius)
		{
			m_splitDataVector.erase(m_splitDataVector.begin());
			continue;
		}

		Vector const & normal = splitData.m_splitPlane.getNormal();
		Vector const & pos = normal * (-splitData.m_splitPlane.getD());

		Transform t;
		t.setLocalFrameKJ_p(normal, Vector::perpendicular(normal));
		t.move_l(pos);

		float const randomRotationRange = PI * 0.05f;
		t.pitch_l(Random::randomReal(-randomRotationRange, randomRotationRange));
		t.yaw_l(Random::randomReal(-randomRotationRange, randomRotationRange));

		splitData.m_splitPlane.set(t.getLocalFrameK_p(), pos);
		break;
	}
}

//----------------------------------------------------------------------

bool ShipObjectDestruction::ObjectData::splitObject(Object & object, Plane const & plane, Object *& front, Object *& back, float roll, float pitch) const
{	
	DynamicDebrisObject const * const dynamicDebrisObject = dynamic_cast<DynamicDebrisObject const *>(&object);
	
	front = NULL;
	back = NULL;

	Vector currentVelocity;
	Vector currentYawPitchRoll;
	
	if (NULL != dynamicDebrisObject)
	{
		currentVelocity = dynamicDebrisObject->getCurrentVelocity_w();
		currentYawPitchRoll = dynamicDebrisObject->getCurrentYawPitchRollRates();
	}
	else
	{
		ShipObject const * const shipObject = dynamic_cast<ShipObject const *>(&object);
		if (NULL != shipObject)
			currentVelocity = shipObject->getCurrentSpeed() * shipObject->getObjectFrameK_w();

		currentYawPitchRoll.z = roll;
		currentYawPitchRoll.x = pitch;
	}
	
	ShipObject::DestructionDebrisList debrisList;
	
	//-- one split

	float const objectRadius = m_object->getTangibleExtent().getRadius();

	int const oldDynamicMeshAppearanceDetailLevelBias = DynamicMeshAppearance::getDetailLevelBias();
	DynamicMeshAppearance::setDetailLevelBias(CuiPreferences::getCapshipDestroyDetailBias());

	typedef stdvector<DynamicDebrisObject *>::fwd DynamicDebrisObjectVector;
	DynamicDebrisObjectVector dynamicDebrisObjectVector;

	DynamicDebrisObject * debrisFront = NULL;
	DynamicDebrisObject * debrisBack = NULL;

	if (!DynamicDebrisObject::splitObjects(object, plane, debrisFront, debrisBack, m_shipObjectDestruction->getOriginalShipRadius() * s_shipRadiusDebrisThreshold))
	{
		DEBUG_FATAL(NULL != debrisFront || NULL != debrisBack, ("ShipObjectDestruction splitObjects failed but created some objects"));
		return false;
	}

	front = debrisFront;
	back = debrisBack;

	Vector rotation = currentYawPitchRoll;

	rotation.y += plane.getNormal().x * 0.01f;
	rotation.x += plane.getNormal().y * 0.01f;

	Vector const & planeNormal_w = object.rotate_o2w(plane.getNormal());
	
	if (NULL != debrisFront)
	{
		debrisFront->setTransform_o2w(object.getTransform_o2w());
		debrisFront->setPhysicsParameters(10.0f, currentVelocity + (planeNormal_w * objectRadius * 0.005f), rotation);
		RenderWorld::addObjectNotifications (*debrisFront);
		debrisFront->addNotification(ClientWorld::getIntangibleNotification());
		debrisFront->addToWorld();
		debrisFront->scheduleForAlter();

		ClientEffectManager::transferOwnerShipOfAllClientEffects(object, *debrisFront, &plane, false);
		Audio::transferOwnershipOfSounds(object, *debrisFront, &plane);
	}
	
	if (NULL != debrisBack)
	{
		debrisBack->setTransform_o2w(object.getTransform_o2w());
		debrisBack->setPhysicsParameters(10.0f, currentVelocity + (planeNormal_w * -objectRadius * 0.005f), -rotation);
		RenderWorld::addObjectNotifications (*debrisBack);
		debrisBack->addNotification(ClientWorld::getIntangibleNotification());
		debrisBack->addToWorld();
		debrisBack->scheduleForAlter();

		Plane reversePlane(-plane.getNormal(), -plane.getD());
		ClientEffectManager::transferOwnerShipOfAllClientEffects(object, *debrisBack, &reversePlane, false);
		Audio::transferOwnershipOfSounds(object, *debrisBack, &reversePlane);
	}
	
	DynamicMeshAppearance::setDetailLevelBias(oldDynamicMeshAppearanceDetailLevelBias);
	
	//-- split explosion
	if (NULL != debrisBack || NULL != debrisFront)
	{
		//-- ensure that these pointers have default values
		if (NULL == debrisBack)
			debrisBack = debrisFront;
		else if (NULL == debrisFront)
			debrisFront = debrisBack;

		NOT_NULL(debrisFront);
		NOT_NULL(debrisBack);
		NOT_NULL(m_shipObjectDestruction);

		if (s_showSplitExplosion)
		{
			Vector const & debrisPosFront_o = debrisFront->getTangibleExtent().getCenter();
			Vector const & debrisPosBack_o = debrisBack->getTangibleExtent().getCenter();
			
			//-- @todo: this centerpoint is not really accurate
			Vector const & debrisCenterPoint_o = (debrisPosFront_o + debrisPosBack_o) * 0.5f;
			
			DestructionSequence const * const ds = m_shipObjectDestruction->getDestructionSequence();
		
			if (NULL != ds)
			{
				DestructionSequence::ClientEffectData const & ced = ds->getClientEffectDataSplit();

				if (NULL != ced.m_clientEffectTemplate)
				{
					float scaleToUse = 1.0f;
					
					if (ced.m_nominalSize > 0.0f)
						scaleToUse = objectRadius / ced.m_nominalSize;
					
					Vector const & debrisCenterPoint_w = debrisFront->rotateTranslate_o2p(debrisCenterPoint_o);
					ClientEffect * const clientEffect = ced.m_clientEffectTemplate->createClientEffect(m_shipObjectDestruction->getParentCell(), debrisCenterPoint_w, planeNormal_w);
					if (clientEffect != NULL)
					{
						clientEffect->setUniformScale(scaleToUse);
						clientEffect->execute();
						delete clientEffect;
					}
				}
			}
		}
	}

	return true;
}

//----------------------------------------------------------------------

bool ShipObjectDestruction::ObjectData::alter(float elapsedTime, Object *& front, Object *& back, float roll, float pitch)
{
	front = NULL;
	back = NULL;

	if (NULL == m_object.getPointer())
		return false;

	if (m_splitDataVector.empty())
		return false;

	SplitData & splitData = m_splitDataVector.front();
	
	{
		splitData.m_timeUntilNextExplosion -= elapsedTime;
		splitData.m_timeUntilSplit -= elapsedTime;
		
		if (splitData.m_timeUntilSplit <= 0.0f)
		{
			if (!splitObject(*m_object, splitData.m_splitPlane, front, back, roll, pitch))
				WARNING(true, ("ShipObjectDestruction failed to split object"));
			return false;
		}
		
		else if (splitData.m_timeUntilNextExplosion <= 0.0f)
		{
			if (s_playSequenceExplosion)
			{
				IGNORE_RETURN(splitData.playExplosion(*m_object));
			}
		}

#ifdef _DEBUG
		Camera const * camera = Game::getCamera();
		if (NULL != camera)
		{
			if (s_showDebugExplosionLines)
				camera->addDebugPrimitive(new Line3dDebugPrimitive(Line3dDebugPrimitive::S_z, Transform::identity, s_lastLine0, s_lastLine1, VectorArgb::solidRed));

			if (s_showDebugSplitPlanes)
			{
				const float extentRadius = m_object->getTangibleExtent().getRadius();
				Vector const boxVector(extentRadius * 0.5f, extentRadius * 0.5f, extentRadius * 0.1f);
				AxialBox const ab(-boxVector, boxVector);			
				Vector const & planeNormal = splitData.m_splitPlane.getNormal();
				Transform planeTransform;
				planeTransform.setLocalFrameKJ_p(planeNormal, Vector::perpendicular(planeNormal));
				planeTransform.move_l(Vector::unitZ * (-splitData.m_splitPlane.getD()));
				camera->addDebugPrimitive(new BoxDebugPrimitive(BoxDebugPrimitive::S_z, m_object->getTransform_o2w().rotateTranslate_l2p(planeTransform), ab, VectorArgb::solidBlue));
			}
		}
#endif
	}

	return true;
}

//----------------------------------------------------------------------

Object * ShipObjectDestruction::ObjectData::getObject() const
{
	return m_object.getPointer();
}

//----------------------------------------------------------------------
//----------------------------------------------------------------------
//----------------------------------------------------------------------

bool ShipObjectDestruction::ObjectData::SplitData::playExplosion(Object & object, Plane const & plane_o)
{
	Camera const * camera = Game::getCamera();
	if (NULL == camera)
		return false;

	Appearance const * const appearance = object.getAppearance();
	if (NULL == appearance)
		return false;
	
	AxialBox const & tangibleExtent = object.getTangibleExtent();
	float const extentRadius = tangibleExtent.getRadius();
	Vector const & extentCenter = tangibleExtent.getCenter();
	float const extentRadiusForCollideRay = extentRadius * 3.0f;

	Vector const & planeNormal = plane_o.getNormal();
	
	float const timeRemainingRatio = m_timeUntilSplit / m_totalTimeUntilSplit;

	Vector const & extentPointProjectedOnPlane = plane_o.project(extentCenter);

	Transform planeTransform;
	planeTransform.setLocalFrameKJ_p(planeNormal, Vector::perpendicular(planeNormal));
	planeTransform.move_p(extentPointProjectedOnPlane);
	planeTransform.roll_l(PI_TIMES_2 * timeRemainingRatio);
	
	Vector intersectionPoint0_o = planeTransform.rotateTranslate_l2p(Vector::unitX * extentRadiusForCollideRay);
	
	float const randomRadiansStart = PI * 0.9f;
	float const randomRadiansEnd = PI * 1.1f;
	
	planeTransform.roll_l(Random::randomReal(randomRadiansStart, randomRadiansEnd));
	Vector intersectionPoint1_o = planeTransform.rotateTranslate_l2p(Vector::unitX * extentRadiusForCollideRay);
	
	{
		Vector const & intersectionPointRay_w = object.rotate_o2w(intersectionPoint1_o - intersectionPoint0_o);
		Vector const & intersectionPointRay_c = camera->rotate_w2o(intersectionPointRay_w);
		
		if (intersectionPointRay_c.z < 0.0f)
		{
			//-- save copy
			Vector const tmp = intersectionPoint0_o;
			intersectionPoint0_o = intersectionPoint1_o;
			intersectionPoint1_o = tmp;
		}
	}
	
	s_lastLine0 = object.rotateTranslate_o2p(intersectionPoint0_o);
	s_lastLine1 = object.rotateTranslate_o2p(intersectionPoint1_o);
	
	CollisionInfo collisionInfoResult;
	if (!appearance->collide(intersectionPoint0_o, intersectionPoint1_o, CollideParameters::cms_default, collisionInfoResult))
	{
		//			WARNING(true, ("ShipObjectDestruction::alter no collide"));
	}
	else
	{
		s_lastHit = object.rotateTranslate_o2p(collisionInfoResult.getPoint());
		
		//-- sequence explosion
		{
			Transform explosionTransform;
			explosionTransform.setLocalFrameKJ_p(Vector::perpendicular(collisionInfoResult.getNormal()), collisionInfoResult.getNormal());
			explosionTransform.move_p(collisionInfoResult.getPoint());

			float const appearanceRadius = object.getAppearanceSphereRadius();

			float scaleToUse = 1.0f;
			ClientEffectTemplate const * const clientEffectTemplate = m_shipObjectDestruction->findRandomExplosion(appearanceRadius, scaleToUse);
			
			if (NULL != clientEffectTemplate)
			{
				ClientEffect * const clientEffect = clientEffectTemplate->createClientEffect(&object, explosionTransform);				
				if (clientEffect != NULL)
				{
					clientEffect->setUniformScale(scaleToUse);
					clientEffect->execute();
					delete clientEffect;
				}
			}
		}
	}

	return true;
} //lint !e1762 //not const

//----------------------------------------------------------------------

bool ShipObjectDestruction::ObjectData::SplitData::playExplosion(Object & object)
{
	{
		int const numChildObjects = object.getNumberOfChildObjects();

		for (int i = 0; i < numChildObjects; ++i)
		{
			Object * const child = object.getChildObject(i);
			if (NULL != child)
			{
				Plane plane_o(m_splitPlane);
				plane_o.transform_p2l(child->getTransform_o2p());
				IGNORE_RETURN(playExplosion(*child, plane_o));
			}
		}
	}

	IGNORE_RETURN(playExplosion(object, m_splitPlane));
	
	computeNextExplosionTime(0.0f, 0.0f);

	return true;
}

//----------------------------------------------------------------------

void ShipObjectDestruction::ObjectData::SplitData::computeNextExplosionTime(float randomStartMin, float randomStartMax)
{
	float const timeRemainingRatio = m_timeUntilSplit / m_totalTimeUntilSplit;

	m_timeUntilNextExplosion = timeRemainingRatio * m_averageTimeBetweenExplosions;

	if (randomStartMax > randomStartMin)
	{
		float const randomTimeIncrease = Random::randomReal(randomStartMin, randomStartMax);
		m_timeUntilNextExplosion += randomTimeIncrease;
		m_timeUntilSplit += randomTimeIncrease;
	}
}

//----------------------------------------------------------------------
//----------------------------------------------------------------------
//----------------------------------------------------------------------

void ShipObjectDestruction::install()
{
	InstallTimer const installTimer("ShipObjectDestruction::install");

	DEBUG_FATAL(s_installed, ("ShipObjectDestruction::install: already installed"));
	s_installed = true;

	DebugFlags::registerFlag(s_showSplitExplosion, "ClientGame/ShipObjectDestruction", "destructShowSplitExplosion");
	DebugFlags::registerFlag(s_showDebugExplosionLines, "ClientGame/ShipObjectDestruction", "destructShowDebugExplosionLines");
	DebugFlags::registerFlag(s_randomizePlanes, "ClientGame/ShipObjectDestruction", "destructRandomizePlanes");
	DebugFlags::registerFlag(s_playAmbientLoop, "ClientGame/ShipObjectDestruction", "destructPlayAmbientLoop");
	DebugFlags::registerFlag(s_playSequenceExplosion, "ClientGame/ShipObjectDestruction", "destructPlaySequenceExplosion");

	ExitChain::add(remove, "ShipObjectDestruction::remove");
}	

//----------------------------------------------------------------------

void ShipObjectDestructionNamespace::remove()
{
	DEBUG_FATAL(!s_installed, ("ShipObjectDestruction::install: not installed"));
	s_installed = false;

	DebugFlags::unregisterFlag(s_showSplitExplosion);
	DebugFlags::unregisterFlag(s_showDebugExplosionLines);
	DebugFlags::unregisterFlag(s_randomizePlanes);
	DebugFlags::unregisterFlag(s_playAmbientLoop);
	DebugFlags::unregisterFlag(s_playSequenceExplosion);
}

//----------------------------------------------------------------------

ShipObjectDestruction::ShipObjectDestruction(ShipObject & ship, float) :
Object(),
m_ship(&ship),
m_objectDataVector(),
m_objectSizeSplitThreshold(0.0f),
m_soundIdVector(),
m_debrisVector(),
m_timeUntilFinale(2.0f),
m_originalShipRadius(0.0f),
m_originalShipCenter(),
m_destructionRollRate(PI * 0.005f),
m_destructionPitchRate(PI * 0.001f),
m_clientDataFile(NULL)
{
	m_clientDataFile = ship.getClientData();
	if (NULL != m_clientDataFile)
	{
		m_clientDataFile->addReference();
	}

	Camera const * camera = Game::getCamera();
	if (NULL != camera)
	{
		//-- be sure to roll/pitch toward the camera

		Vector const & cameraPos_ship = ship.rotateTranslate_w2o(camera->getPosition_w());

		if (cameraPos_ship.x > 0.0f)
			m_destructionRollRate = -m_destructionRollRate;

		if (cameraPos_ship.z < 0.0f)
			m_destructionPitchRate = -m_destructionPitchRate;
	}

	AxialBox const & tangibleExtent = ship.getTangibleExtent();

	CollisionWorld::removeObject(&ship);

	Vector const & tangibleExtentCenter = tangibleExtent.getCenter();
	float const radius = tangibleExtent.getRadius();

	m_originalShipCenter = tangibleExtentCenter;

	m_originalShipRadius = radius;

	m_objectSizeSplitThreshold = radius * 0.02f;

	ObjectData * const objectData = new ObjectData(*this, ship);
	m_objectDataVector.push_back(objectData);

	RenderWorld::addObjectNotifications (*this);
	this->addNotification(ClientWorld::getIntangibleNotification());
	this->addToWorld();
	this->scheduleForAlter();

	DestructionSequence const * const ds = getDestructionSequence();
		
	if (NULL != ds)
		ds->applyChildObjects(*m_ship);
		
	//----------------------------------------------------------------------
	
	if (s_randomizePlanes)
	{
		int const numPlanes = 10;
		float const timePerPlane = 6.0f / numPlanes;
		
		for (int i = 0; i < 10; ++i)
			objectData->addSplitData(generateRandomPlane(ship), Random::randomReal(timePerPlane * 0.5f, timePerPlane * 1.5f));
	}
	else
	{
		if (NULL != ds)
		{
			DestructionSequence::AmbientSoundDataVector const & ambientSounds = ds->getAmbientSounds();
			{
				Vector const & shipPosition_w = ship.getPosition_w();
				for (DestructionSequence::AmbientSoundDataVector::const_iterator it = ambientSounds.begin(); it != ambientSounds.end(); ++it)
				{
					DestructionSequence::AmbientSoundData const & asd = *it;
					if (NULL != asd.m_soundTemplate)
					{
						SoundId const & soundId = Audio::playSound(asd.m_soundTemplateName.c_str(), shipPosition_w, NULL);
						m_soundIdVector.push_back(soundId);
					}
					else
					{
						//-- sound template is invalid, just push an invalid one onto the vector so the vector size
						//-- stays equal to the sound vector on the DestructionSequence object
						m_soundIdVector.push_back(SoundId());
					}
				}
			}
			
			DestructionSequence::BreakpointVector const & breakpoints = ds->getBreakpoints();
			{
				if (NULL != m_clientDataFile)
				{				
					ClientDataFile::PlaneVector planeVector;
					if (m_clientDataFile->processBreakpoints(ship, breakpoints, 1.0f, planeVector))
					{
						if (planeVector.empty())
							WARNING(true, ("ShipObjectDestruction found no planes to split with"));
						else
						{
							float const totalTime = ds->getSequenceTimeLength();
							
							float timeFactor = 0.5f;
							for (ClientDataFile::PlaneVector::const_iterator it = planeVector.begin(); it != planeVector.end(); ++it, timeFactor *= 0.5f)
							{
								float const timeThisCut = totalTime * timeFactor;
								Plane const & plane = *it;
								objectData->addSplitData(plane, timeThisCut);
							}
						}
					}
				}
			}
		}
	}
}

//----------------------------------------------------------------------

ShipObjectDestruction::~ShipObjectDestruction()
{
	{
		for (SoundIdVector::const_iterator it = m_soundIdVector.begin(); it != m_soundIdVector.end(); ++it)
		{
			SoundId const & soundId = *it;
			
			if (it == m_soundIdVector.begin())
				Audio::stopSound(soundId, 20.0f);
			else
				Audio::stopSound(soundId, 3.0f);
		}
	}

	std::for_each(m_objectDataVector.begin(), m_objectDataVector.end(), PointerDeleter());
	m_objectDataVector.clear();

	if (NULL != m_clientDataFile)
	{
		m_clientDataFile->releaseReference();
		m_clientDataFile = NULL;
	}
}

//----------------------------------------------------------------------

void ShipObjectDestruction::pushObjectsAwayFrom(Object const & objectToSplit)
{
	//-- push all the other pieces around
	AxialBox const & tangibleExtent = objectToSplit.getTangibleExtent();
	Vector const & tangibleExtentCenter_w = objectToSplit.rotateTranslate_o2w(tangibleExtent.getCenter());
	float const tangibleExtentRadius = tangibleExtent.getRadius();
	
	if (tangibleExtentRadius > 0.0f)
	{
		for (ObjectDataVector::iterator pit = m_objectDataVector.begin(); pit != m_objectDataVector.end(); ++pit)
		{
			ObjectData * const objectDataToPush = NON_NULL(*pit);
			Object * const objectToPush = objectDataToPush->getObject();
			
			DynamicDebrisObject * const dynamicDebrisObjectToPush = dynamic_cast<DynamicDebrisObject *>(objectToPush);
			if (NULL != dynamicDebrisObjectToPush)
			{
				AxialBox const & objectToPushTangibleExtent = objectToPush->getTangibleExtent();
				Vector const & objectToPushTangibleExtentCenter_w = objectToPush->rotateTranslate_o2w(objectToPushTangibleExtent.getCenter());
				float const objectToPushTangibleExtentRadius = objectToPushTangibleExtent.getRadius();
				
				if (objectToPushTangibleExtentRadius > 0.0f)
				{
					float const distanceToExplosion = (objectToPushTangibleExtentCenter_w - tangibleExtentCenter_w).approximateMagnitude() - objectToPushTangibleExtentRadius;
					
					float const distanceToExplosionRatioSqared = sqr((distanceToExplosion > tangibleExtentRadius) ? (tangibleExtentRadius / distanceToExplosion) : 1.0f);
					
					Vector pushDirection_w = (objectToPushTangibleExtentCenter_w - tangibleExtentCenter_w);
					if (pushDirection_w.approximateNormalize())
					{
						float const explosionEffect = (distanceToExplosionRatioSqared * tangibleExtentRadius) /  objectToPushTangibleExtentRadius;
						
						//-- at a distance equal to the radius of the exploding object, the added velocity is equal to 1/10 the radius per second
						float const speedIncrease = explosionEffect * s_pushSpeedFactor;
						
						dynamicDebrisObjectToPush->setCurrentVelocity_w(dynamicDebrisObjectToPush->getCurrentVelocity_w() + (pushDirection_w * speedIncrease));
						
						float const rotationIncrease = (explosionEffect * PI) * s_pushRotationFactor;
						
						dynamicDebrisObjectToPush->setCurrentYawPitchRollRates(dynamicDebrisObjectToPush->getCurrentYawPitchRollRates() + (Vector::randomUnit() * rotationIncrease));
					}
				}
			}
		}
	}
}

//----------------------------------------------------------------------

float ShipObjectDestruction::alter(float elapsedTime)
{
	
	//-- update ambient sounds
	{
		if (NULL != m_ship.getPointer())
		{
			m_ship->roll_o(elapsedTime * m_destructionRollRate);
			m_ship->pitch_o(elapsedTime * m_destructionPitchRate);
			setTransform_o2p(m_ship->getTransform_o2p());
		}
		
		Camera const * camera = Game::getCamera();
		if (NULL != camera)
		{
			Vector const & position_w = getPosition_w() + rotateTranslate_o2w(m_originalShipCenter);
			Vector const & cameraPosition_w = camera->getPosition_w();
			
			Vector directionToCamera_w = (cameraPosition_w - position_w);
			float const distanceToCenter = directionToCamera_w.approximateMagnitude();
			if (directionToCamera_w.approximateNormalize())
			{
				DestructionSequence const * const ds = getDestructionSequence();
				
				if (NULL != ds)
				{
					DestructionSequence::AmbientSoundDataVector const & ambientSounds = ds->getAmbientSounds();
					
					if (ambientSounds.size() != m_soundIdVector.size())
					{
						DEBUG_FATAL(true, ("bad sounds vector"));
					}
					else
					{
						size_t const numSounds = m_soundIdVector.size();
						for (size_t i = 0; i < numSounds; ++i)
						{
							DestructionSequence::AmbientSoundData const & asd = ambientSounds[i];
							float const distanceMultiplier = asd.m_distanceFactor;
							SoundId const & soundId = m_soundIdVector[i];
							
							float const distance = std::min(distanceToCenter, m_originalShipRadius);
							Vector const & soundPos_w = position_w + (directionToCamera_w * (distanceMultiplier * distance));
							Audio::setSoundPosition_w(soundId, soundPos_w);
						}					
					}
				}
			}
		}
	}

	{
		ObjectDataVector resultObjects;

		for (ObjectDataVector::iterator it = m_objectDataVector.begin(); it != m_objectDataVector.end();)
		{
			Object * objectFront = NULL;
			Object * objectBack = NULL;

			ObjectData * const objectData = NON_NULL(*it);
			Object * objectToSplit = objectData->getObject();
			if (NULL == objectToSplit)
			{
				it = m_objectDataVector.erase(it);
				delete objectData;
				continue;
			}
			else if (!objectData->alter(elapsedTime, objectFront, objectBack, m_destructionRollRate, m_destructionPitchRate))
			{				
				pushObjectsAwayFrom(*objectToSplit);
				
				DynamicDebrisObject * const debrisFront = dynamic_cast<DynamicDebrisObject *>(objectFront);
				DynamicDebrisObject * const debrisBack = dynamic_cast<DynamicDebrisObject *>(objectBack);
				
				if (NULL != debrisFront)
				{
					debrisFront->setKeepalive(true);
					m_debrisVector.push_back(DynamicDebrisObjectWatcher(debrisFront));
				}
				
				if (NULL != debrisBack)
				{
					debrisBack->setKeepalive(true);
					m_debrisVector.push_back(DynamicDebrisObjectWatcher(debrisBack));
				}
				
				if (objectData->getSplitDataCount() > 1)
				{
					if (NULL != objectFront)
					{
						float const extentRadius = objectFront->getTangibleExtent().getRadius();
						if (extentRadius > m_objectSizeSplitThreshold)
						{
							ObjectData * const objectDataFront = new ObjectData(*this, *objectFront);
							objectDataFront->addSplitDataFrom(*objectData, 1);
							resultObjects.push_back(objectDataFront);
						}
					}
					
					if (NULL != objectBack)
					{
						float const extentRadius = objectBack->getTangibleExtent().getRadius();
						if (extentRadius > m_objectSizeSplitThreshold)
						{
							ObjectData * const objectDataBack = new ObjectData(*this, *objectBack);
							objectDataBack->addSplitDataFrom(*objectData, 1);
							resultObjects.push_back(objectDataBack);
						}
					}
				}
				
				it = m_objectDataVector.erase(it);

				if (m_ship != objectToSplit)
					objectToSplit->kill();
				else 
				{
					if (m_ship->isShipKillableByDestructionSequence())
						objectToSplit->kill();
					else
						m_ship->hideShipPostDestruction();
				}

				delete objectData;
				continue;
			}
			
			++it;
		}

		if (!resultObjects.empty())
			m_objectDataVector.insert(m_objectDataVector.end(), resultObjects.begin(), resultObjects.end());
	}
	
	//-- kill this object when no debris pieces remain
	if (m_objectDataVector.empty())
	{
		m_timeUntilFinale -= elapsedTime;
		
		//-- split up large pieces, one per frame
		{
			for (DynamicDebrisObjectWatcherVector::iterator it = m_debrisVector.begin(); it != m_debrisVector.end(); ++it)
			{
				DynamicDebrisObjectWatcher & ddow = *it;
				DynamicDebrisObject * const ddo = ddow.getPointer();
				if (NULL != ddo)
				{
					if (!ddo->getKill())
					{
						if (ddo->isKeepalive())
						{
							AxialBox const & ddoTangibleExtent = ddo->getTangibleExtent();
							float const ddoTangibleExtentRadius = ddoTangibleExtent.getRadius();

							if (ddoTangibleExtentRadius > m_objectSizeSplitThreshold)
							{
								Plane const & plane = generateRandomPlane(*ddo);

								DynamicDebrisObject * debrisFront = NULL;
								DynamicDebrisObject * debrisBack = NULL;

								if (!DynamicDebrisObject::splitObjects(*ddo, plane, debrisFront, debrisBack, m_originalShipRadius * s_shipRadiusDebrisThreshold))
								{
									DEBUG_FATAL(NULL != debrisFront || NULL != debrisBack, ("ShipObjectDestruction splitObjects failed but created object(s)"));
								}
								else
								{
									if (NULL != debrisFront)
									{
										debrisFront->setTransform_o2w(ddo->getTransform_o2w());
										debrisFront->setPhysicsParameters(10.0f, ddo->getCurrentVelocity_w(), ddo->getCurrentYawPitchRollRates());
										RenderWorld::addObjectNotifications (*debrisFront);
										debrisFront->addNotification(ClientWorld::getIntangibleNotification());
										debrisFront->addToWorld();
										debrisFront->scheduleForAlter();
										debrisFront->setKeepalive(true);
										m_debrisVector.push_back(DynamicDebrisObjectWatcher(debrisFront));
										
										ClientEffectManager::transferOwnerShipOfAllClientEffects(*ddo, *debrisFront, &plane, false);
										Audio::transferOwnershipOfSounds(*ddo, *debrisFront, &plane);
									}
									
									if (NULL != debrisBack)
									{
										debrisBack->setTransform_o2w(ddo->getTransform_o2w());
										debrisBack->setPhysicsParameters(10.0f, ddo->getCurrentVelocity_w(), ddo->getCurrentYawPitchRollRates());
										RenderWorld::addObjectNotifications (*debrisBack);
										debrisBack->addNotification(ClientWorld::getIntangibleNotification());
										debrisBack->addToWorld();
										debrisBack->scheduleForAlter();
										debrisBack->setKeepalive(true);
										m_debrisVector.push_back(DynamicDebrisObjectWatcher(debrisBack));
										
										ClientEffectManager::transferOwnerShipOfAllClientEffects(*ddo, *debrisBack, &plane, false);
										Audio::transferOwnershipOfSounds(*ddo, *debrisBack, &plane);
									}

									ddo->kill();

									//-- push out the finale time a bit, but not so far that we can't catch up, so we can finish splitting debris
									m_timeUntilFinale += elapsedTime * 0.5f;
									break;
								}
							}
						}
					}
				}
			}
		}

		//-- play the final effect
		if (m_timeUntilFinale <= 0.0f)
		{
			DestructionSequence const * const ds = getDestructionSequence();
		
			if (NULL != ds)
			{
				DestructionSequence::ClientEffectData const & cedf = ds->getClientEffectDataFinale();

				if (NULL != cedf.m_clientEffectTemplate)
				{
					AxialBox ab;
					
					if (!m_debrisVector.empty())
					{
						for (DynamicDebrisObjectWatcherVector::iterator it = m_debrisVector.begin(); it != m_debrisVector.end(); ++it)
						{
							DynamicDebrisObjectWatcher & ddow = *it;
							DynamicDebrisObject * const ddo = ddow.getPointer();
							if (NULL != ddo)
							{
								if (!ddo->getKill())
								{
									if (ddo->isKeepalive())
									{
										AxialBox const & ddoTangibleExtent = ddo->getTangibleExtent();
										Vector const & ddoTangibleExtentCenter_w = ddo->rotateTranslate_o2w(ddoTangibleExtent.getCenter());
										float const ddoTangibleExtentRadius = ddoTangibleExtent.getRadius();

										ab.add(ddoTangibleExtentCenter_w);

										float velocityAdd = m_originalShipRadius;

										if (ddoTangibleExtentRadius > (m_objectSizeSplitThreshold * 2.0f))
											velocityAdd *= ((m_objectSizeSplitThreshold * 2.0f) / ddoTangibleExtentRadius);

										Vector const & randomFlyDirection = Vector::randomUnit();

										ddo->setPhysicsParameters(10.0f, ddo->getCurrentVelocity_w() + (randomFlyDirection * velocityAdd), ddo->getCurrentYawPitchRollRates() + (randomFlyDirection * PI));
										ddo->setKeepalive(false);
										
										DestructionSequence::ClientEffectData const & ceds = ds->getClientEffectDataSplit();
										
										if (NULL != ceds.m_clientEffectTemplate)
										{
											float scaleToUse = 1.0f;
											
											if (ceds.m_nominalSize > 0.0f)
												scaleToUse = ddoTangibleExtent.getRadius() / ceds.m_nominalSize;
											
											ClientEffect * const clientEffect = ceds.m_clientEffectTemplate->createClientEffect(getParentCell(), ddoTangibleExtentCenter_w, ddo->getObjectFrameJ_w());
											
											if (clientEffect != NULL)
											{
												clientEffect->setUniformScale(scaleToUse);
												clientEffect->execute();
												delete clientEffect;
											}
											
											//										//-- delay the next explosion
											//										m_timeUntilFinale = 0.1f;
											//										return AlterResult::cms_alterNextFrame;
										}
									}
								}

								ddow = NULL;
							}
						}
						m_debrisVector.clear();
					}
					else
					{
						ab.setMin(Vector(-50.0f, -50.0f, -50.0f));
						ab.setMax(Vector(50.0f, 50.0f, 50.0f));
					}

					float scaleToUse = 1.0f;
					
					if (cedf.m_nominalSize > 0.0f)
						scaleToUse = ab.getRadius() / cedf.m_nominalSize;

					ClientEffect * const clientEffect = cedf.m_clientEffectTemplate->createClientEffect(getParentCell(), ab.getCenter(), this->getObjectFrameJ_w());

					if (clientEffect != NULL)
					{
						clientEffect->setUniformScale(scaleToUse);
						clientEffect->execute();
						delete clientEffect;
					}
				}
			}

			return AlterResult::cms_kill;
		}
	}
	
	return AlterResult::cms_alterNextFrame;
}

//----------------------------------------------------------------------

DestructionSequence const * ShipObjectDestruction::getDestructionSequence() const
{
	return (NULL != m_clientDataFile) ? m_clientDataFile->getDestructionSequence() : NULL;
}

//----------------------------------------------------------------------

ClientEffectTemplate const * ShipObjectDestruction::findRandomExplosion(float debrisSize, float & scaleToUse) const
{
	DestructionSequence const * const ds = getDestructionSequence();

	if (NULL == ds)
		return NULL;

	DestructionSequence::ClientEffectDataVector const & sequenceExplosions = ds->getSequenceExplosions();

	if (sequenceExplosions.empty())
		return NULL;

	DestructionSequence::ClientEffectData const & ced = sequenceExplosions[static_cast<size_t>(Random::random(0, static_cast<int>(sequenceExplosions.size() - 1)))];

	if (ced.m_nominalSize > 0.0f)
		scaleToUse = debrisSize / ced.m_nominalSize;
	else
		scaleToUse = 1.0f;

	return ced.m_clientEffectTemplate;
}

//----------------------------------------------------------------------

float ShipObjectDestruction::getOriginalShipRadius() const
{
	return m_originalShipRadius;
}

//======================================================================
