//===================================================================
//
// ComponentAppearance.cpp
// copyright 2000-2002, Sony Online Entertainment Inc.
// All Rights Reserved.
//
//===================================================================

#include "clientObject/FirstClientObject.h"
#include "clientObject/ComponentAppearance.h"

#include "clientGraphics/Camera.h"
#include "clientGraphics/RenderWorld.h"
#include "clientObject/ComponentAppearanceTemplate.h"
#include "sharedCollision/CollisionInfo.h"
#include "sharedCollision/BoxExtent.h"
#include "sharedCollision/ExtentList.h"
#include "sharedObject/AlterResult.h"

#include "dpvsObject.hpp"
#include "dpvsModel.hpp"

#include <stdio.h>
#include <string>
#include <vector>

//===================================================================

ComponentAppearance::ComponentAppearance(const ComponentAppearanceTemplate* componentAppearanceTemplate):	
	Appearance(componentAppearanceTemplate),
	m_needExtentCalculated(true),
	m_dpvsObject(NULL),
	m_objectList(NULL)
{
	const int numberOfComponents = componentAppearanceTemplate->getNumberOfComponents();

	//-- create object list to hold 
	m_objectList = new ObjectList(numberOfComponents);

	//-- create objects and add them to the object list
	{
		for (int i = 0; i < numberOfComponents; ++i)
		{
			Object * const     object     = new Object();
			RenderWorld::addObjectNotifications(*object);
			Appearance * const appearance = componentAppearanceTemplate->getAppearanceTemplate(i)->createAppearance();
			object->setAppearance(appearance);

			m_objectList->addObject(object);
		} //lint !e429  //-- object has not been freed or returned
	}

	//-- create the DPVS object for the component appearance.
	//   This will calculate extents if necessary.
	{
		DPVS::Model *const dpvsTestModel = fetchDpvsTestModel(componentAppearanceTemplate);
		m_dpvsObject = RenderWorld::createObject(this, dpvsTestModel);
		IGNORE_RETURN(dpvsTestModel->release());
	}

	m_dpvsObject->set(DPVS::Object::INFORM_VISIBLE, false);

	//-- Set parent DPVS object for all components.
	//   We don't do this when we create the child objects above because
	//   we can't guarantee that we can properly construct the DPVS
	//   object at that time.  If the template data format is old enough, we must
	//   calculate the extents at load-time, which depends on the children
	//   already being created.
	{
		for (int i = 0; i < numberOfComponents; ++i)
		{
			Object *const childObject    = (*m_objectList)[i];
			Appearance *const appearance = childObject->getAppearance();

			DPVS::Object *childDpvsObject = appearance->getDpvsObject();
			NOT_NULL(childDpvsObject);
			childDpvsObject->setVisibilityParent(m_dpvsObject);
		}
	}
}

//-------------------------------------------------------------------

ComponentAppearance::~ComponentAppearance()
{
	IGNORE_RETURN(m_dpvsObject->release());
	m_dpvsObject = 0;

	m_objectList->removeAll(true);
	delete m_objectList;
}

//-------------------------------------------------------------------

inline const ComponentAppearanceTemplate *ComponentAppearance::getComponentAppearanceTemplate() const
{
	return safe_cast<const ComponentAppearanceTemplate*>(getAppearanceTemplate());
}

//-------------------------------------------------------------------

bool ComponentAppearance::isLoaded() const
{
	for (int i = 0; i < m_objectList->getNumberOfObjects(); i++)
		if (!m_objectList->getObject(i)->getAppearance()->isLoaded())
			return false;

	return true;
}

//-------------------------------------------------------------------

void ComponentAppearance::setAlpha(bool const opaqueEnabled, float const opaqueAlpha, bool const alphaEnabled, float const alphaAlpha)
{
	for (int i = 0; i < m_objectList->getNumberOfObjects(); i++)
		if (m_objectList->getObject(i)->getAppearance())
			m_objectList->getObject(i)->getAppearance()->setAlpha(opaqueEnabled, opaqueAlpha, alphaEnabled, alphaAlpha);
}

// ----------------------------------------------------------------------

void ComponentAppearance::render() const
{
	DEBUG_FATAL(true, ("this routine should never be called"));
}

// ----------------------------------------------------------------------

void ComponentAppearance::objectListCameraRender() const
{
	const Transform &a2w = getTransform_w();
	const ComponentAppearanceTemplate * componentAppearanceTemplate = getComponentAppearanceTemplate();

	Transform t(Transform::IF_none);
	for (int i = 0; i < m_objectList->getNumberOfObjects(); i++)
	{
		const Object *component = m_objectList->getObject(i);
		const Appearance *componentsAppearance = component->getAppearance();
		if (componentsAppearance)
		{
			t.multiply(a2w, componentAppearanceTemplate->getTransform(i));
			componentsAppearance->setTransform_w(t);
			componentsAppearance->objectListCameraRender();
		}
	}
}

//-------------------------------------------------------------------

float ComponentAppearance::alter(float time)
{
	if (m_needExtentCalculated)
		calculateExtent();

	UNREF(Appearance::alter(time));
	m_objectList->prepareToAlter();
	return m_objectList->alter(time);
}

//-------------------------------------------------------------------

bool ComponentAppearance::collide(Vector const & start_o, Vector const & end_o, CollideParameters const & collideParameters, CollisionInfo & result) const
{
	ComponentAppearanceTemplate const * const componentAppearanceTemplate = getComponentAppearanceTemplate();
	if (!componentAppearanceTemplate)
		return false;

	bool collided = false;

	result.setPoint(end_o);

	for (int i = 0; i < m_objectList->getNumberOfObjects(); i++)
	{
		//-- TODO: investigate bailing out early on the first collision if the quality is low
		Object const * const object = NON_NULL(m_objectList->getObject(i));
		Transform const & transform = componentAppearanceTemplate->getTransform(i);
		Vector const start_c = transform.rotateTranslate_p2l(start_o);
		Vector const end_c = transform.rotateTranslate_p2l(result.getPoint());

		if (object && object->getAppearance() && object->getAppearance()->collide(start_c, end_c, collideParameters, result))
		{
			result.setPoint(transform.rotateTranslate_l2p(result.getPoint()));
			result.setNormal(transform.rotate_l2p(result.getNormal()));

			collided = true;
		}
	}

	return collided;
}

// ----------------------------------------------------------------------

bool ComponentAppearance::implementsCollide() const
{
	return true;
}

// ----------------------------------------------------------------------

const Extent *ComponentAppearance::getExtent() const
{
	//-- ComponentAppearance uses the ComponentAppearanceTemplate's extent.
	const AppearanceTemplate *const appearanceTemplate = getAppearanceTemplate();
	NOT_NULL(appearanceTemplate);

	return appearanceTemplate->getExtent();
}

// ----------------------------------------------------------------------

AxialBox const ComponentAppearance::getTangibleExtent() const
{
	AxialBox result;

	if (isLoaded())
	{
		ComponentAppearanceTemplate const * const componentAppearanceTemplate = getComponentAppearanceTemplate();
		NOT_NULL(componentAppearanceTemplate);

		int const numberOfObjects = m_objectList->getNumberOfObjects();
		for (int i = 0; i < numberOfObjects; ++i)
		{
	  		Object const * const object = m_objectList->getObject(i);
			NOT_NULL(object);

			AxialBox axialBox = object->getTangibleExtent();

			//-- Transform box from child space into parent space
			if (!axialBox.isEmpty())
			{
				//-- Grow axial box in parent space
				Transform const & transform = object->getTransform_o2p();

				for (int j = 0; j < 8; ++j)
					result.add(transform.rotateTranslate_l2p(axialBox.getCorner(j)));
			}
		}
	}

	return result;
}

//-------------------------------------------------------------------

BoxExtent* ComponentAppearance::createExtent()
{
	if (!isLoaded())
	{
		//-- bail: can't calculate the extents in this condition.
		return 0;
	}

	//-- set extents
	BoxExtent* internalExtent = new BoxExtent();
	internalExtent->setMin(Vector::maxXYZ);
	internalExtent->setMax(Vector::negativeMaxXYZ);

	const ComponentAppearanceTemplate *componentAppearanceTemplate = getComponentAppearanceTemplate();

	bool any = false;
	const int numberOfObjects = m_objectList->getNumberOfObjects();
	for (int i = 0; i < numberOfObjects; ++i)
	{
  	Object *object = m_objectList->getObject(i);
		Appearance *appearance = object->getAppearance();
		const Vector position = componentAppearanceTemplate->getTransform(i).getPosition_p();

		const Extent *extent = appearance->getExtent();
		if (extent)
		{
			const BoxExtent* boxExtent = dynamic_cast<const BoxExtent*>(extent);
			if (boxExtent)
			{
				internalExtent->updateMinAndMax(boxExtent->getMin() + position);
				internalExtent->updateMinAndMax(boxExtent->getMax() + position);
			}
			else
			{
				extent = appearance->getExtent();
				const Sphere &sphere = extent->getSphere();
				const Vector radius(sphere.getRadius(), sphere.getRadius(), sphere.getRadius());
				internalExtent->updateMinAndMax((sphere.getCenter() - radius) + position);
				internalExtent->updateMinAndMax((sphere.getCenter() + radius) + position);
			}

			any = true;
		}
	}

	if (!any)
	{
		DEBUG_WARNING(true,("No child extents were found"));
		internalExtent->setMin(-Vector::xyz111);
		internalExtent->setMax(Vector::xyz111);
	}

	//-- complete the extent calculation.
	internalExtent->calculateCenterAndRadius();

	return internalExtent;
}

// ----------------------------------------------------------------------

void ComponentAppearance::calculateExtent()
{
	if (!m_needExtentCalculated)
	{
		DEBUG_WARNING(true, ("calculateExtent() called when not needed."));
		return;
	}

	//-- Get the appearance template's DPVS object.  If this is non-NULL,
	//   extents already have been calculated.
	const ComponentAppearanceTemplate *const appearanceTemplate = getComponentAppearanceTemplate();
	NOT_NULL(appearanceTemplate);

	DPVS::Model* testShape = appearanceTemplate->fetchDpvsTestShape();
	if (testShape)
	{
		// Appearance template's extent has been calculated already by another appearance.
		const BoxExtent *const extent = safe_cast<const BoxExtent*>(appearanceTemplate->getExtent());
		NOT_NULL(extent);

		// Set this DPVS object's model to the given model.
		{
			DPVS::OBBModel *const model = RenderWorld::fetchBoxModel(extent->getBox());
			m_dpvsObject->setTestModel(model);
			IGNORE_RETURN(model->release());
		}

		m_needExtentCalculated = false;

		IGNORE_RETURN(testShape->release());
		return;
	}

	//-- Extent still needs to be calculated.
	BoxExtent* extent = createExtent();
	if (!extent)
	{
		// Nothing to do yet.
		return;
	}

	//-- Set the appearance template's extents so they are correct for all to use.
	const_cast<ComponentAppearanceTemplate*>(appearanceTemplate)->setExtent(ExtentList::fetch(extent));
	m_needExtentCalculated = false;

	//-- Create the test shape model.
	{
		DPVS::Model *const newDpvsModel = RenderWorld::fetchBoxModel(extent->getBox());

		//-- Tell the appearance template about the model so we don't have to do this again.
		const_cast<ComponentAppearanceTemplate*>(appearanceTemplate)->setDpvsTestShape(newDpvsModel);

		//-- Set DPVS object's model.
		m_dpvsObject->setTestModel(newDpvsModel);
		IGNORE_RETURN(newDpvsModel->release());
	}
}

//-------------------------------------------------------------------

void ComponentAppearance::setOwner(Object *newOwner)
{
	Appearance::setOwner(newOwner);

	const ComponentAppearanceTemplate * componentAppearanceTemplate = getComponentAppearanceTemplate();

	if (newOwner == NULL)
	{
		for (int i = 0; i < m_objectList->getNumberOfObjects(); i++)
		{
			Object *child = m_objectList->getObject(i);
			if (child->getAttachedTo())
				child->detachFromObject(Object::DF_parent);
			child->setTransform_o2p(componentAppearanceTemplate->getTransform(i));
		}
	}
	else
	{
		for (int i = 0; i < m_objectList->getNumberOfObjects(); i++)
		{
			Object *child = m_objectList->getObject(i);
			child->attachToObject_p(newOwner, false);
			child->setTransform_o2p(componentAppearanceTemplate->getTransform(i));
		}
	}
}

//-------------------------------------------------------------------

DPVS::Object *ComponentAppearance::getDpvsObject() const
{
	return m_dpvsObject;
} //lint !e1763 // (Info -- Member function 'ComponentAppearance::getDpvsObject(void) const' marked as const indirectly modifies class) // As designed.

//-------------------------------------------------------------------

void ComponentAppearance::addToWorld()
{
	m_objectList->addToWorld();
	Appearance::addToWorld();
}

//-------------------------------------------------------------------

void ComponentAppearance::removeFromWorld()
{
	m_objectList->removeFromWorld();
	Appearance::removeFromWorld();
}

//-------------------------------------------------------------------

void ComponentAppearance::setTexture(const Tag tag, const Texture &texture)
{
	for (int i = 0; i < m_objectList->getNumberOfObjects(); i++)
		if (m_objectList->getObject(i)->getAppearance())
			m_objectList->getObject(i)->getAppearance()->setTexture(tag, texture);
}

// ----------------------------------------------------------------------

const IndexedTriangleList* ComponentAppearance::getRadarShape () const
{
	return getComponentAppearanceTemplate ()->getRadarShape ();
}

// ----------------------------------------------------------------------

#ifdef _DEBUG

int ComponentAppearance::getPolygonCount () const
{
	int total = 0;

	for (int i = 0; i < m_objectList->getNumberOfObjects(); i++)
		if (m_objectList->getObject(i) && m_objectList->getObject(i)->getAppearance())
			total += m_objectList->getObject(i)->getAppearance()->getPolygonCount ();

	return total;
}

static void indent(std::string &result, const int indentLevel)
{
	int i;
	for (i = 0; i < indentLevel; ++i)
		result += "  ";
}

void ComponentAppearance::debugDump(std::string &result, const int indentLevel) const
{
	char buffer [1024];

	sprintf(buffer, "ComponentAppearance %s\r\n", getAppearanceTemplate()->getName());
	indent(result, indentLevel);
	result += buffer;

	sprintf(buffer, "%i components\r\n", m_objectList->getNumberOfObjects());
	indent(result, indentLevel);
	result += buffer;

	int i;
	for (i = 0; i < m_objectList->getNumberOfObjects(); i++)
	{
		sprintf(buffer, "component %i\r\n", i);
		indent(result, indentLevel + 1);
		result += buffer;

		const Appearance* const appearance = m_objectList->getObject(i)->getAppearance();
		appearance->debugDump(result, indentLevel + 2);
	}
}

#endif

// ----------------------------------------------------------------------

DPVS::Model* ComponentAppearance::fetchDpvsTestModel(const ComponentAppearanceTemplate* componentAppearanceTemplate)
{
	// Nothing to do if there's no appearance template.
	if (!componentAppearanceTemplate)
		return 0;

	//-- Get the appearance template's test shape.
	DPVS::Model* dpvsModel = componentAppearanceTemplate->fetchDpvsTestShape();
	if (dpvsModel)
		return dpvsModel;

	// Test shape doesn't exist.  This implies the appearance template is old enough that it doesn't contain
	// valid extents.

	//-- Create the extents.
	BoxExtent* extent = createExtent();
	if (!extent)
	{
		//-- Use the default test model: this appearance must not be loaded.
		return RenderWorld::fetchDefaultModel();
	}

	//-- Set the appearance template's extents so they are correct for all to use.
	const_cast<ComponentAppearanceTemplate*>(componentAppearanceTemplate)->setExtent(ExtentList::fetch(extent));
	m_needExtentCalculated = false;

	//-- Create the test shape model.
	DPVS::Model *const newDpvsModel = RenderWorld::fetchBoxModel(extent->getBox());

	//-- Tell the appearance template about the model so we don't have to do this again.
	const_cast<ComponentAppearanceTemplate*>(componentAppearanceTemplate)->setDpvsTestShape(newDpvsModel);

	return newDpvsModel;
}

//----------------------------------------------------------------------

ComponentAppearance * ComponentAppearance::asComponentAppearance()
{
	return this;
}

//----------------------------------------------------------------------

ComponentAppearance const * ComponentAppearance::asComponentAppearance() const
{
	return this;
}

//----------------------------------------------------------------------

void ComponentAppearance::setShadowBlobAllowed()
{
	Appearance::setShadowBlobAllowed();

	for (int i = 0; i < m_objectList->getNumberOfObjects(); ++i)
		if (m_objectList->getObject(i)->getAppearance())
			m_objectList->getObject(i)->getAppearance()->setShadowBlobAllowed();
}

//----------------------------------------------------------------------

ObjectList const & ComponentAppearance::getObjectList() const
{
	return *NON_NULL(m_objectList);
}

// ======================================================================

