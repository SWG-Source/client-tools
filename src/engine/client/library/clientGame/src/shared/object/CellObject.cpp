// ======================================================================
//
// CellObject.cpp
// Copyright 2001 Sony Online Entertainment
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/CellObject.h"

#include "clientGame/ClientObjectTemplate.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGraphics/Light.h"
#include "clientGraphics/RenderWorld.h"
#include "clientObject/InteriorEnvironmentBlock.h"
#include "clientObject/InteriorEnvironmentBlockManager.h"
#include "sharedCollision/Floor.h"
#include "sharedCollision/FloorMesh.h"
#include "sharedCollision/FloorTri.h"
#include "sharedFoundation/DebugInfoManager.h"
#include "sharedFoundation/Production.h"
#include "sharedMath/IndexedTriangleList.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/ContainedByProperty.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/Portal.h"
#include "sharedObject/PortalProperty.h"
#include "sharedObject/PortalPropertyTemplate.h"

// ======================================================================

namespace CellObjectNamespace
{
	Light *createLight(const PortalPropertyTemplateCellLight &lightData);

	std::string const & ms_debugInfoSectionName = "CellObject";
}
using namespace CellObjectNamespace;

// ======================================================================

void CellObject::Callbacks::PublicChanged::modified(CellObject & /*target*/, const bool & /*oldValue*/, const bool & /*newValue*/, bool /*local*/)
{
}

// ======================================================================

CellObject::CellObject(const SharedObjectTemplate *newTemplate)
: ClientObject(newTemplate, ClientWorld::getIntangibleNotification()),
	m_isPublic(true),
	m_cellNumber(-1),
	m_cellLabel(),
	m_labelLocationOffset(),
	m_accessAllowed(true),
	m_radarShape (0),
	m_radarEdges (0),
	m_radarPortalEdges (0)
{
	addSharedVariable(m_isPublic);
	addSharedVariable(m_cellNumber);
	addSharedVariable_np(m_cellLabel);
	addSharedVariable_np(m_labelLocationOffset);

	m_isPublic.setSourceObject(this);

	setDebugName("PortalProperty cell object");
	RenderWorld::addCellNotifications(*this);

	CellProperty *cellProperty = new CellProperty(*this);
	addProperty(*cellProperty);
}

// ----------------------------------------------------------------------

CellObject::~CellObject()
{
	//-- This must be the first line in the destructor to invalidate any watchers watching this object
	nullWatchers();

	delete m_radarShape;
	m_radarShape = 0;

	delete m_radarEdges;
	m_radarEdges = 0;
}

// ----------------------------------------------------------------------

Light *CellObjectNamespace::createLight(const PortalPropertyTemplateCellLight &lightData)
{
	Light *light = NULL;

	switch (lightData.type)
	{
		case PortalPropertyTemplateCellLight::T_ambient:
			light = new Light(Light::T_ambient, lightData.diffuseColor);
			break;

		case PortalPropertyTemplateCellLight::T_parallel:
			light = new Light(Light::T_parallel, lightData.diffuseColor);
			light->setTransform_o2p(lightData.transform);
			light->setSpecularColor(lightData.specularColor);
			break;

		case PortalPropertyTemplateCellLight::T_point:
			light = new Light(Light::T_point, lightData.diffuseColor);
			light->setTransform_o2p(lightData.transform);
			light->setSpecularColor(lightData.specularColor);
			light->setConstantAttenuation(lightData.constantAttenuation);
			light->setLinearAttenuation(lightData.linearAttenuation);
			light->setQuadraticAttenuation(lightData.quadraticAttenuation);
			break;

		default:
			DEBUG_FATAL(true, ("unknown light type"));
	}

	return light;
}

// ----------------------------------------------------------------------

void CellObject::endBaselines()
{
	ClientObject::endBaselines();

	DEBUG_FATAL(m_cellNumber.get() < 1, ("Invalid cell number"));
	ContainedByProperty *containedByProperty = getContainedByProperty();
	NetworkId networkId = containedByProperty->getContainedByNetworkId();
	Object *container = containedByProperty->getContainedBy();
	FATAL(container == NULL, ("CellObject container is NULL, cell=%s, container=%s", getNetworkId().getValueString().c_str(), networkId.getValueString().c_str()));
	PortalProperty *portalProperty = container->getPortalProperty();
	FATAL(portalProperty == NULL, ("CellObject's container's portal property is NULL"));
	portalProperty->cellLoaded(m_cellNumber.get(), *this, true);

	if (ConfigClientGame::getAmbientLightInCells())
	{
		Light *light = new Light(Light::T_ambient, VectorArgb::solidWhite);
		light->attachToObject_p(this, true);
	}

	PortalPropertyTemplateCell::LightList const *lightList = portalProperty->getPortalPropertyTemplate().getCell(m_cellNumber.get()).getLightList();
	if (lightList)
	{
		PortalPropertyTemplateCell::LightList::const_iterator const iEnd = lightList->end();
		for (PortalPropertyTemplateCell::LightList::const_iterator i = lightList->begin(); i != iEnd; ++i)
		{
			const PortalPropertyTemplateCellLight &lightData = *i;

			{
				Light *light = createLight(lightData);
				light->setAffectsShadersWithPrecalculatedVertexLighting(false);
				light->attachToObject_p(this, true);
			}	

			if (lightData.specularColor.r != 0.0f || lightData.specularColor.g != 0.0f || lightData.specularColor.b != 0.0f)
			{
				Light *light = createLight(lightData);
				light->setDiffuseColor(VectorArgb::solidBlack);
				light->setAffectsShadersWithoutPrecalculatedVertexLighting(false);
				light->attachToObject_p(this, true);
			}
		}
	}

	//-- build the radar shape
	{
		const CellProperty* const cellProperty = getCellProperty ();
		if (cellProperty)
		{
			const Floor* const floor = cellProperty->getFloor ();
			if (floor)
			{
				const FloorMesh* const floorMesh = floor->getFloorMesh ();
				if (floorMesh)
				{
					std::vector<Vector> vertices;
					floorMesh->getVertices (vertices);

					std::vector<int> indices;
					floorMesh->getIndices (indices);

					m_radarShape = new IndexedTriangleList;
					m_radarShape->addIndexedTriangleList (&vertices [0], static_cast<int> (vertices.size ()), &indices [0], static_cast<int> (indices.size ()));
				}
			}
		}
	}

	//-- build the radar edges
	{
		const CellProperty* const cellProperty = getCellProperty ();
		if (cellProperty)
		{
			const Floor* const floor = cellProperty->getFloor ();
			if (floor)
			{
				const FloorMesh* const floorMesh = floor->getFloorMesh ();
				if (floorMesh)
				{
					m_radarEdges = new std::vector<Vector>;

					int i;
					for (i = 0; i < floorMesh->getTriCount (); ++i)
					{
						const FloorTri& floorTri = floorMesh->getFloorTri (i);

						int j;
						for (j = 0; j < 3; ++j)
						{
							if (!floorTri.isCrossable (j) && floorTri.getPortalId (j) == -1)
							{
								const Vector& v0 = floorMesh->getVertex (floorTri.getCornerIndex (j));
								m_radarEdges->push_back (v0);

								const Vector& v1 = floorMesh->getVertex (floorTri.getCornerIndex (j + 1));
								m_radarEdges->push_back (v1);
							}
						}
					}
				}
			}
		}
	}

	//-- build the radar portal edges
	{
		const CellProperty* const cellProperty = getCellProperty ();
		if (cellProperty)
		{
			int i;
			for (i = 0; i < cellProperty->getNumberOfPortalObjects (); ++i)
			{
				const CellProperty::PortalObjectEntry& portalObjectEntry = cellProperty->getPortalObject (i);

				uint j;
				for (j = 0; j < portalObjectEntry.portalList->size (); ++j)
				{
					const Portal* const portal = (*portalObjectEntry.portalList) [j];
					if (portal && portal->getNeighbor () && portal->getNeighbor ()->getParentCell () == CellProperty::getWorldCellProperty ())
					{
						IndexedTriangleList const & portalGeometry = portal->getGeometry ();
						std::vector<Vector> const & vertices = portalGeometry.getVertices();
						std::vector<int> const & indices = portalGeometry.getIndices();
						uint const numberOfFaces = indices.size() / 3;

						if (!m_radarPortalEdges)
							m_radarPortalEdges = new std::vector<Vector>;

						//-- we found a portal to the outside world
						for (uint faceIndex = 0, index = 0; faceIndex < numberOfFaces; ++faceIndex)
						{
							Vector const & v0 = vertices[indices[index++]];
							Vector const & v1 = vertices[indices[index++]];
							Vector const & v2 = vertices[indices[index++]];

							m_radarPortalEdges->push_back(v0);
							m_radarPortalEdges->push_back(v1);
							m_radarPortalEdges->push_back(v1);
							m_radarPortalEdges->push_back(v2);
							m_radarPortalEdges->push_back(v2);
							m_radarPortalEdges->push_back(v0);
						}
					}
				}
			}	
		}
	}

	//-- hook up the cell property's environment texture
	{
		CellProperty* const cellProperty = getCellProperty ();
		const char* const cellName = cellProperty->getCellName ();
		const PortalProperty* const portalProperty = cellProperty->getPortalProperty ();
		const char* const pobShortName = portalProperty->getPobShortName ();

		const InteriorEnvironmentBlock* const interiorEnvironmentBlock = NON_NULL (InteriorEnvironmentBlockManager::getEnvironmentBlock (pobShortName, cellName));
		if (interiorEnvironmentBlock)
		{
			cellProperty->setEnvironmentTexture (interiorEnvironmentBlock->getEnvironmentTexture ());
			cellProperty->setFogEnabled (interiorEnvironmentBlock->getFogEnabled ());
			cellProperty->setFogColor (interiorEnvironmentBlock->getFogColor ());
			cellProperty->setFogDensity (interiorEnvironmentBlock->getFogDensity ());
		}
	}
}

// ----------------------------------------------------------------------

void CellObject::setAccessAllowed(bool allowed)
{
	m_accessAllowed = allowed;
	// this is now handled by a auto delta variable
	// TODO : remove the set and dependent messages completely
}

// ----------------------------------------------------------------------

const IndexedTriangleList* CellObject::getRadarShape () const
{
	return m_radarShape;
}

// ----------------------------------------------------------------------

const std::vector<Vector>* CellObject::getRadarEdges () const
{
	return m_radarEdges;
}

// ----------------------------------------------------------------------

const std::vector<Vector>* CellObject::getRadarPortalEdges () const
{
	return m_radarPortalEdges;
}

//-----------------------------------------------------------------------

void CellObject::getObjectInfo(std::map<std::string, std::map<std::string, Unicode::String> > & propertyMap) const
{
#if PRODUCTION == 0
/**
  When adding a variable to this class, please add it here.  Variable that aren't easily displayable are still listed, for tracking purposes.
*/

/**
	Don't compile in production build because this maps human-readable values to data members and makes hacking easier
*/
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "CellNumber", m_cellNumber.get());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "AccessAllowed", m_isPublic.get());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "CellLabel", m_cellLabel.get());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "LabelLocationOffset", m_labelLocationOffset.get());

//	IndexedTriangleList*             m_radarShape;
//	stdvector<Vector>::fwd*          m_radarEdges;
//	stdvector<Vector>::fwd*          m_radarPortalEdges;

	ClientObject::getObjectInfo(propertyMap);

#else
	UNREF(propertyMap);
#endif
}

// ----------------------------------------------------------------------

bool CellObject::getAccessAllowed() const
{
	return m_isPublic.get() && m_accessAllowed;
}

// ----------------------------------------------------------------------

CellObject * CellObject::getCellObject(NetworkId const & networkId)
{
	return CellObject::asCellObject(NetworkIdManager::getObjectById(networkId));
}

// ----------------------------------------------------------------------

CellObject * CellObject::asCellObject(Object * object)
{
	ClientObject * const clientObject = (object != NULL) ? object->asClientObject() : NULL;
	CellObject * const cellObject = ( clientObject != NULL) ? clientObject->asCellObject() : NULL;

	return cellObject;
}

// ----------------------------------------------------------------------

CellObject const * CellObject::asCellObject(Object const * object)
{
	ClientObject const * const clientObject = (object != NULL) ? object->asClientObject() : NULL;
	CellObject const * const cellObject = (clientObject != NULL) ? clientObject->asCellObject() : NULL;

	return cellObject;
}
 
// ----------------------------------------------------------------------

CellObject * CellObject::asCellObject()
{
	return this;
}

// ----------------------------------------------------------------------

CellObject const * CellObject::asCellObject() const
{
	return this;
}
// ======================================================================
