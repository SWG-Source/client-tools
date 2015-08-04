//===================================================================
//
// ClientStaticRadialFloraManager.cpp
// asommers 9-11-2000
//
// copyright 2000, verant interactive
//
//===================================================================

#include "clientTerrain/FirstClientTerrain.h"
#include "clientTerrain/ClientStaticRadialFloraManager.h"

#include "clientGraphics/Camera.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/RenderWorld.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientTerrain/ClientProceduralTerrainAppearance.h"
#include "sharedCollision/CollideParameters.h"
#include "sharedCollision/CollisionInfo.h"
#include "sharedCollision/Extent.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/Profiler.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedFoundation/VoidBindSecond.h"
#include "sharedFoundation/VoidMemberFunction.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/Object.h"
#include "sharedRandom/Random.h"
#include "sharedTerrain/ConfigSharedTerrain.h"
#include "sharedUtility/FileName.h"

#include <algorithm>
#include <string>
#include <vector>
#include <cstdio>

//===================================================================

class ClientStaticRadialFloraManager::StaticFloraNode
{
public:

	explicit StaticFloraNode (const ClientProceduralTerrainAppearance::StaticFloraData& staticFloraData);
	~StaticFloraNode ();

	const ClientProceduralTerrainAppearance::StaticFloraData& getStaticFloraData () const;
	Appearance* createAppearance () const;
	void        destroyAppearance (Appearance* appearance) const;

private:

	typedef std::vector<Appearance*> AppearanceList;

private:

	StaticFloraNode ();                                   //lint -esym(754, StaticFloraNode::StaticFloraNode)
	StaticFloraNode (const StaticFloraNode&);             //lint -esym(754, StaticFloraNode::StaticFloraNode)
	StaticFloraNode& operator= (const StaticFloraNode&);  //lint -esym(754, StaticFloraNode::operator=)

private:

	const ClientProceduralTerrainAppearance::StaticFloraData m_staticFloraData;
	mutable AppearanceList                                   m_appearanceList;
	const AppearanceTemplate*                                m_appearanceTemplate;
};

//-------------------------------------------------------------------

ClientStaticRadialFloraManager::StaticFloraNode::StaticFloraNode (const ClientProceduralTerrainAppearance::StaticFloraData& staticFloraData) :
	m_staticFloraData (staticFloraData),
	m_appearanceTemplate (0)
{
	FileName baseName (FileName::P_appearance, staticFloraData.familyChildData->appearanceTemplateName);
	baseName.stripPathAndExt ();

	FileName aptName (FileName::P_appearance, baseName, "apt");
	if (TreeFile::exists (aptName))
		m_appearanceTemplate = AppearanceTemplateList::fetch (aptName);
	else
	{
		FileName satName (FileName::P_appearance, baseName, "sat");
		if (TreeFile::exists (satName))
			m_appearanceTemplate = AppearanceTemplateList::fetch (satName);
		else
		{
			FileName prtName (FileName::P_appearance, baseName, "prt");
			if (TreeFile::exists (prtName))
				m_appearanceTemplate = AppearanceTemplateList::fetch (prtName);
			else
			{
				DEBUG_WARNING (true, ("ClientStaticRadialFloraManager: could not open appearance template %s", staticFloraData.familyChildData->appearanceTemplateName));
				m_appearanceTemplate = AppearanceTemplateList::fetch ("appearance/defaultappearance.apt");
			}
		}
	}
}

//-------------------------------------------------------------------

ClientStaticRadialFloraManager::StaticFloraNode::~StaticFloraNode ()
{
	while (!m_appearanceList.empty())
	{
		delete (m_appearanceList.back());
		m_appearanceList.pop_back ();
	}

	if (m_appearanceTemplate)
	{
		AppearanceTemplateList::release (m_appearanceTemplate);
		m_appearanceTemplate = 0;
	}
}

//-------------------------------------------------------------------

const ClientProceduralTerrainAppearance::StaticFloraData& ClientStaticRadialFloraManager::StaticFloraNode::getStaticFloraData () const
{
	return m_staticFloraData;
}

//-------------------------------------------------------------------

Appearance* ClientStaticRadialFloraManager::StaticFloraNode::createAppearance () const
{
	Appearance* appearance = 0;

	if (m_appearanceList.empty())
	{
		//-- create one
		appearance = m_appearanceTemplate->createAppearance ();
	}
	else
	{
		//-- just take a new one off the list or create it
		appearance = m_appearanceList.back();
		m_appearanceList.pop_back();
	}

	return appearance;
}

//-------------------------------------------------------------------

void ClientStaticRadialFloraManager::StaticFloraNode::destroyAppearance (Appearance* appearance) const
{
	m_appearanceList.push_back(appearance);
}

//===================================================================

class ClientStaticRadialFloraManager::StaticRadialNode : public ClientRadialFloraManager::RadialNode
{
public:

	Object* const m_object;        //lint !e1925  //-- public data member
	float         deltaDirection;  //lint !e1925  //-- public data member
	float         scale;           //lint !e1925  //-- public data member

public:

	StaticRadialNode ();
	virtual ~StaticRadialNode ();

	virtual void             enabledChanged ();
	virtual const Vector     getPosition () const;
	virtual void             setPosition (const Vector& position);
	virtual bool             shouldFloat () const;
	virtual bool             shouldAlignToTerrain () const;
	virtual const Transform& getTransform () const;
	virtual void             setTransform (const Transform& transform);
	virtual void             setHasFlora (bool hasFlora);
	virtual void             alter (float deltaTime);

	void                     draw () const;

	StaticFloraNode*         getStaticFloraNode ();
	const StaticFloraNode*   getStaticFloraNode () const;
	void                     setStaticFloraNode (StaticFloraNode* staticFloraNode);

private:

	StaticRadialNode (const StaticRadialNode&);             //lint -esym(754, StaticRadialNode::StaticRadialNode)
	StaticRadialNode& operator= (const StaticRadialNode&);  //lint -esym(754, StaticRadialNode::operator=)

private:

	StaticFloraNode* m_staticFloraNode;
	Vector           m_oldJ;
};

//-------------------------------------------------------------------

ClientStaticRadialFloraManager::StaticRadialNode::StaticRadialNode () :
	ClientRadialFloraManager::RadialNode (),
	m_object (new Object),
	deltaDirection (0.f),
	scale (0.f),
	m_staticFloraNode (0),
	m_oldJ (Vector::unitY)
{
}

//-------------------------------------------------------------------

ClientStaticRadialFloraManager::StaticRadialNode::~StaticRadialNode ()
{
	delete m_object;

	m_staticFloraNode = 0;
}

//-------------------------------------------------------------------

ClientStaticRadialFloraManager::StaticFloraNode* ClientStaticRadialFloraManager::StaticRadialNode::getStaticFloraNode ()
{
	return m_staticFloraNode;
}

//-------------------------------------------------------------------

const ClientStaticRadialFloraManager::StaticFloraNode* ClientStaticRadialFloraManager::StaticRadialNode::getStaticFloraNode () const
{
	return m_staticFloraNode;
}

//-------------------------------------------------------------------

void ClientStaticRadialFloraManager::StaticRadialNode::setStaticFloraNode (ClientStaticRadialFloraManager::StaticFloraNode* staticFloraNode)
{
	m_staticFloraNode = staticFloraNode;
}

//-------------------------------------------------------------------

void ClientStaticRadialFloraManager::StaticRadialNode::enabledChanged ()
{
	if (m_manager->isEnabled())
	{
		if (getHasFlora())
		{
			if (!m_object->isInWorld())
			{
				m_object->addToWorld ();
			}
		}
		else
		{
			if (m_object->isInWorld())
			{
				m_object->removeFromWorld();
			}
		}
	}
	else
	{
		if (getHasFlora () && m_object->isInWorld ())
		{
			m_object->removeFromWorld ();
		}
	}
}

//-------------------------------------------------------------------

const Vector ClientStaticRadialFloraManager::StaticRadialNode::getPosition () const
{
	return m_object->getPosition_w ();
}

//-------------------------------------------------------------------

void ClientStaticRadialFloraManager::StaticRadialNode::setPosition (const Vector& position)
{
	m_object->setPosition_p (position);
}

//-------------------------------------------------------------------

bool ClientStaticRadialFloraManager::StaticRadialNode::shouldFloat () const
{
	NOT_NULL (m_staticFloraNode);
	return m_staticFloraNode->getStaticFloraData ().floats;
}

//-------------------------------------------------------------------

bool ClientStaticRadialFloraManager::StaticRadialNode::shouldAlignToTerrain () const
{
	NOT_NULL (m_staticFloraNode);
	return m_staticFloraNode->getStaticFloraData ().familyChildData->alignToTerrain;
}

//-------------------------------------------------------------------

const Transform& ClientStaticRadialFloraManager::StaticRadialNode::getTransform () const
{
	NOT_NULL (m_object);
	return m_object->getTransform_o2w ();
}

//-------------------------------------------------------------------

void ClientStaticRadialFloraManager::StaticRadialNode::setTransform (const Transform& transform)
{
	NOT_NULL (m_object);
	m_object->setTransform_o2p (transform);
	m_oldJ = transform.getLocalFrameJ_p ();
}

//-------------------------------------------------------------------

void ClientStaticRadialFloraManager::StaticRadialNode::setHasFlora (bool hasFlora)
{
	RadialNode::setHasFlora (hasFlora);

	//-- add to or remove from world
	if (m_manager->isEnabled ())
	{
		if (!getHasFlora () && m_object->isInWorld ())
			m_object->removeFromWorld ();

		if (getHasFlora () && !m_object->isInWorld ())
			m_object->addToWorld ();
	}
}

//-------------------------------------------------------------------

void ClientStaticRadialFloraManager::StaticRadialNode::alter (float time)
{
	if (!getHasFlora ())
		return;

#if 1
	if (m_object)
	{
		m_object->alter(time);
	}
#else
	//-- sway flora
	NOT_NULL (m_staticFloraNode);
	const FloraGroup::FamilyChildData& fcd = m_staticFloraNode->getStaticFloraData()->familyChildData;

	if (fcd.shouldSway)
	{
		deltaDirection += PI_TIMES_2 * time;

		const Vector vi = m_object->getObjectFrameI_w ();
		const Vector vj = m_oldJ + (fcd.shouldSway ? (fcd.displacement * m_object->rotate_w2o (vi) * sinf (fcd.period * deltaDirection)) : Vector::zero);
		const Vector vk = m_object->getObjectFrameK_w ();

		m_object->setTransformIJK_o2p (vi, vj, vk);
	}
#endif
}

//-------------------------------------------------------------------

void ClientStaticRadialFloraManager::StaticRadialNode::draw () const
{
	DEBUG_FATAL (getHasFlora () && !m_object->isInWorld (), ("object has flora but it's not in the world"));
	DEBUG_FATAL (!getHasFlora () && m_object->isInWorld (), ("object has flora but it's not in the world"));

	if (!getHasFlora ())
		return;

	float alpha = getAlpha();
	m_object->getAppearance ()->setAlpha (true, alpha, true, alpha);
}

//===================================================================

namespace ClientStaticRadialFloraManagerNamespace
{
	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	static inline float scurve (float t)
	{
		return ((3.0f - (2.0f * t)) * t * t);
	}

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	static inline float computeAlpha (const float minimum, const float value, const float maximum, const float featherIn)
	{
		if (value < minimum)
			return 0.f;

		if (value > maximum)
			return 0.f;

		const float feather = featherIn * (maximum - minimum) * 0.5f;

		if (value < minimum + feather)
		{
			return (minimum == 0.f) ? 1.f : (scurve ((value - minimum) / feather));
		}
		else
			if (value > (maximum - feather))
			{
				return scurve ((maximum - value) / feather);
			}
			
		return 1.f;
	}

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool ms_disableRender;
	bool ms_renderClearFloraEntries;
	bool ms_renderDetailLevel;
	ClientStaticRadialFloraManager::RenderDetailLevelFunction ms_renderDetailLevelFunction;
}

using namespace ClientStaticRadialFloraManagerNamespace;

//===================================================================

void ClientStaticRadialFloraManager::install ()
{
	DEBUG_REPORT_LOG_PRINT (ConfigSharedTerrain::getDebugReportInstall (), ("ClientStaticRadialFloraManager::install\n"));
	DebugFlags::registerFlag (ms_disableRender, "ClientTerrain", "noRenderStaticFlora");
	DebugFlags::registerFlag (ms_renderDetailLevel, "ClientTerrain", "renderDetailLevel");
}

//-------------------------------------------------------------------

void ClientStaticRadialFloraManager::remove ()
{
	DebugFlags::unregisterFlag (ms_disableRender);
	DebugFlags::unregisterFlag (ms_renderDetailLevel);
}

//-------------------------------------------------------------------

void ClientStaticRadialFloraManager::setRenderDetailLevelFunction (RenderDetailLevelFunction renderDetailLevelFunction)
{
	ms_renderDetailLevelFunction = renderDetailLevelFunction;
}

//===================================================================

ClientStaticRadialFloraManager::ClientStaticRadialFloraManager (const ClientProceduralTerrainAppearance& terrainAppearance, const bool& enabled, float minimumDistance, float const & maximumDistance, FindFloraFunction findFloraFunction) :
	ClientRadialFloraManager (terrainAppearance, enabled, minimumDistance, maximumDistance),
	m_findFloraFunction (findFloraFunction)
#ifdef _DEBUG
	, m_nextObjectId (0)
#endif
{
}

//-------------------------------------------------------------------

ClientStaticRadialFloraManager::~ClientStaticRadialFloraManager (void)
{
	std::for_each (m_floraNodeList.begin (), m_floraNodeList.end (), PointerDeleter ());

	m_findFloraFunction = 0;
}

//-------------------------------------------------------------------

void ClientStaticRadialFloraManager::maximumDistanceChanged ()
{
	ClientRadialFloraManager::maximumDistanceChanged ();

	std::for_each (m_floraNodeList.begin (), m_floraNodeList.end (), PointerDeleter ());
	m_floraNodeList.clear ();
}

//-------------------------------------------------------------------

void ClientStaticRadialFloraManager::alter (float const elapsedTime)
{
	if (m_terrainAppearance.getRenderedThisFrame())
	{
		//-- update the flora
		if (getReferenceObject ())
		{
			update (getReferenceObject ()->getPosition_w ());
		}

		//-- reset the extent list for the next frame
		clearRegionList ();
	}

	ClientRadialFloraManager::alter (elapsedTime);

	if (!isEnabled())
	{
		return;
	}

	//////////////////////////////////////////////////////////////
	//-- alter the flora
	RadialNodeList::const_iterator rni;
	for (rni=m_radialNodeList.begin();rni!=m_radialNodeList.end();++rni)
	{
		StaticRadialNode *rn = safe_cast<StaticRadialNode *>(rni->nodePointer);
		rn->alter(elapsedTime);
	}
	//////////////////////////////////////////////////////////////
}

//-------------------------------------------------------------------

void ClientStaticRadialFloraManager::draw () const
{
	PROFILER_AUTO_BLOCK_DEFINE ("ClientStaticRadialFloraManager::draw");

	//-- chain up
	ClientRadialFloraManager::draw ();

	//-- if we're not enabled, don't do anything
	if (!isEnabled ())
		return;

	Vector refPosition_w;
	if (getReferenceObject())
	{
		refPosition_w=getReferenceObject()->getPosition_w();
	}
	else
	{
		const Camera& camera = ShaderPrimitiveSorter::getCurrentCamera ();
		refPosition_w=camera.getPosition_w();
	}

	//////////////////////////////////////////////////////////////
	//-- render the flora
	RadialNodeList::const_iterator rni;
	for (rni=m_radialNodeList.begin();rni!=m_radialNodeList.end();++rni)
	{
		StaticRadialNode *rn = safe_cast<StaticRadialNode *>(rni->nodePointer);

		float depth = refPosition_w.magnitudeBetween(rn->getPosition());
		float alpha = computeAlpha(m_minimumDistance, depth, m_maximumDistance, 0.75f);
		rn->setAlpha(alpha);

		rn->draw();
	}
	//////////////////////////////////////////////////////////////

#ifdef _DEBUG
	if (ms_renderDetailLevel && ms_renderDetailLevelFunction)
	{
		RadialNodeList::const_iterator end  = m_radialNodeList.end ();
		RadialNodeList::const_iterator iter = m_radialNodeList.begin ();
		for (; iter != end; ++iter)
		{
			const StaticRadialNode* const staticRadialNode = safe_cast<const StaticRadialNode*>(iter->nodePointer);

			if (staticRadialNode->getHasFlora ())
				ms_renderDetailLevelFunction (staticRadialNode->m_object);
		}
	}
#endif
}

//-------------------------------------------------------------------

bool ClientStaticRadialFloraManager::collide (const Vector& start_w, const Vector& end_w, CollisionInfo& result) const
{
	const float length = start_w.magnitudeBetween (end_w);
	if (length <= 0.f)
		return false;

	result.setPoint (end_w);

	bool collided = false;

	RadialNodeList::const_iterator end  = m_radialNodeList.end ();
	RadialNodeList::const_iterator iter = m_radialNodeList.begin ();
	for (; iter != end; ++iter)
	{
		const StaticRadialNode* const staticRadialNode = safe_cast<const StaticRadialNode*>(iter->nodePointer);

		if (staticRadialNode->getHasFlora ())
		{
			const Vector& scale = staticRadialNode->m_object->getScale ();

			Vector start_o = staticRadialNode->m_object->rotateTranslate_w2o (start_w);
			start_o.x /= scale.x;
			start_o.y /= scale.y;
			start_o.z /= scale.z;

			Vector end_o = staticRadialNode->m_object->rotateTranslate_w2o (result.getPoint ());
			end_o.x /= scale.x;
			end_o.y /= scale.y;
			end_o.z /= scale.z;

			if (staticRadialNode->m_object->getAppearance()->collide(start_o, end_o, CollideParameters::cms_default, result))
			{
				collided = true;

				Vector point_o = result.getPoint ();
				point_o.x *= scale.x;
				point_o.y *= scale.y;
				point_o.z *= scale.z;

				result.setObject (staticRadialNode->m_object);
				result.setPoint (staticRadialNode->m_object->rotateTranslate_o2w (point_o));
				result.setNormal (staticRadialNode->m_object->rotate_o2w (result.getNormal ()));
			}
		}
	}

	return collided;
}

//-------------------------------------------------------------------

bool ClientStaticRadialFloraManager::approximateCollide (const Vector& start_w, const Vector& end_w, CollisionInfo& result) const
{
	result.setPoint (end_w);

	bool collided = false;

	RadialNodeList::const_iterator end  = m_radialNodeList.end ();
	RadialNodeList::const_iterator iter = m_radialNodeList.begin ();
	for (; iter != end; ++iter)
	{
		const StaticRadialNode* const staticRadialNode = safe_cast<const StaticRadialNode*>(iter->nodePointer);

		if (staticRadialNode->getHasFlora ())
		{
			const Vector start_o = staticRadialNode->m_object->rotateTranslate_w2o (start_w);
			const Vector end_o   = staticRadialNode->m_object->rotateTranslate_w2o (result.getPoint ());
			
			const Extent* const extent = staticRadialNode->m_object->getAppearance ()->getExtent ();
			
			float t = 1.0f;
			if (extent->intersect (start_o, end_o, &t))
			{
				collided = true;

				const Vector point_o  = Vector::linearInterpolate (start_o, end_o, t);
				
				Vector normal_o = point_o - extent->getSphere ().getCenter ();
				normal_o.normalize ();

				result.setObject (staticRadialNode->m_object);
				result.setPoint (staticRadialNode->m_object->rotateTranslate_o2w (point_o));
				result.setNormal (staticRadialNode->m_object->rotate_o2w (normal_o));
			}
		}
	}

	return collided;
}

//-------------------------------------------------------------------

bool ClientStaticRadialFloraManager::createFlora (float positionX, float positionZ, RadialNode* radialNode, bool& floraAllowed) const
{
	//-- see if flora exists at that position on the terrain
	ClientProceduralTerrainAppearance::StaticFloraData staticFloraData;
	if ((m_terrainAppearance.*m_findFloraFunction) (positionX, positionZ, staticFloraData, floraAllowed))
	{
		StaticRadialNode* const staticRadialNode = safe_cast<StaticRadialNode*> (radialNode);

		//-- find appearance if it has been created
		uint i;
		for (i = 0; i < m_floraNodeList.size (); ++i)
			if (m_floraNodeList[i]->getStaticFloraData ().familyChildData->appearanceTemplateName == staticFloraData.familyChildData->appearanceTemplateName)
				break;

		//-- we can't use an existing template, so create a new one
		if (i == m_floraNodeList.size ())
			m_floraNodeList.push_back (new StaticFloraNode (staticFloraData));

		//-- see if the radial node currently represents the object's existing appearance
		if (staticRadialNode->getStaticFloraNode () != m_floraNodeList[i])
		{
			//
			//-- set the object's appearance
			//
			Object* const          object             = staticRadialNode->m_object;
			object->resetRotate_o2p ();
			object->yaw_o (positionX + positionZ);
			StaticFloraNode* const newStaticFloraNode = m_floraNodeList[i];

			//-- cache the old one
			if (object->getAppearance ())
			{
				NOT_NULL (staticRadialNode->getStaticFloraNode ());

				StaticFloraNode* const oldStaticFloraNode = staticRadialNode->getStaticFloraNode ();
				oldStaticFloraNode->destroyAppearance (object->stealAppearance ());
			}

			//-- set the new one
			Appearance * const appearance = newStaticFloraNode->createAppearance ();
			appearance->setKeepAlive (true);
			object->setAppearance (appearance);

			//-- record which appearance data we're using
			DEBUG_FATAL (i >= m_floraNodeList.size (), (""));
			staticRadialNode->setStaticFloraNode (newStaticFloraNode);

			const Vector scale = staticFloraData.familyChildData->shouldScale ? Vector::xyz111 * Random::randomReal (staticFloraData.familyChildData->minimumScale, staticFloraData.familyChildData->maximumScale) : Vector::xyz111;
			object->setScale (scale);
		}

		return true;
	}

	return false;
}

//-------------------------------------------------------------------

ClientStaticRadialFloraManager::RadialNode* ClientStaticRadialFloraManager::createRadialNode (const Vector& position) const
{
	const float offset = position.x + position.z;

	StaticRadialNode* const node = new StaticRadialNode;
	node->m_object->setPosition_p (position);
	node->m_object->yaw_o (offset);
	node->deltaDirection = Random::randomReal (PI_TIMES_2);
	RenderWorld::addObjectNotifications (*node->m_object);

#ifdef _DEBUG
	char buffer [100];
	sprintf (buffer, "%s_%4i", m_debugName->c_str (), ++m_nextObjectId);
	node->m_object->setDebugName (buffer);
#endif

	return node;
}

//===================================================================
