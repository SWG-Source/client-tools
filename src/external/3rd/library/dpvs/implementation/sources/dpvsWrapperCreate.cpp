/*****************************************************************************
 *
 * dPVS
 * -----------------------------------------
 *
 * (C) 1999-2001 Hybrid Holding, Ltd.
 * All Rights Reserved.
 *
 * This file consists of unpublished, proprietary source code of
 * Hybrid Holding, and is considered Confidential Information for
 * purposes of non-disclosure agreement. Disclosure outside the terms
 * outlined in signed agreement may result in irrepairable harm to
 * Hybrid Holding and legal action against the party in breach.
 *
 * Description: 	Wrapper code for pseudo-constructors
 *
 * $Archive: /dpvs/implementation/sources/dpvsWrapperCreate.cpp $
 * $Author: wili $ 
 * $Revision: #2 $
 * $Modtime: 8.10.02 14:17 $
 * $Date: 2003/11/18 $
 * 
 ******************************************************************************/


#include "dpvsWrapper.hpp"
#include "dpvsNameHash.hpp"
#include "dpvsMemory.hpp"
#include "dpvsMatrixConverter.hpp"
#include "dpvsIntersect.hpp"
#include "dpvsImpCamera.hpp"
#include "dpvsImpRegionOfInfluence.hpp"
#include "dpvsBounds.hpp"
#include "dpvsImpCell.hpp"
#include "dpvsImpModel.hpp"
#include "dpvsImpObject.hpp"
#include "dpvsImpPhysicalPortal.hpp"
#include "dpvsImpVirtualPortal.hpp"
#include "dpvsImpMeshModel.hpp"
#include "dpvsImpMiscModel.hpp"

#if defined (DPVS_MT)
#	include "dpvsMT.hpp"
#	define DPVS_MUTEX_LOCK Lock lock;
#else
#	define DPVS_MUTEX_LOCK
#endif


using namespace DPVS;

NameHash g_nameHash;	// name hash table

/*****************************************************************************
 *
 * Function:		DPVS::ReferenceCount::setUserPointer()
 *
 * Description:		Defines a user pointer for the object
 *
 * Parameters:		ID = used-defined ID value (pointer)
 *
 * Notes:			DPVS never modifies this value. The user ID is meant to 
 *					help application users mapping any DPVS classes into 
 *					application-side objects during visibility callbacks
 *					etc.
 *
 *					User IDs are initialized to zero in the constructor. The
 *					application user can define a new user ID at any time.
 *
 * See Also:		DPVS::ReferenceCount::getUserPointer()
 *
 *****************************************************************************/
				
void ReferenceCount::setUserPointer	(void* ID)
{ 
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_ASSERT_SELF_OK();
	m_userPointer = ID;
}

/*****************************************************************************
 *
 * Function:		DPVS::Object::getUserPointer()
 *
 * Description:		Returns user ID (pointer) of the object
 *
 * Returns:			user ID of the object
 *
 * See Also:		DPVS::ReferenceCount::setUserPointer()
 *
 *****************************************************************************/

void* ReferenceCount::getUserPointer (void) const		
{ 
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_ASSERT_SELF_OK();
	return m_userPointer;
}

/*****************************************************************************
 *
 * Function:		DPVS::Object::debugIsValidPointer()
 *
 * Description:		Debug build function that determines if a ReferenceCount-
 *					based object is a valid pointer 
 * 
 * Parameters:		p	Pointer to reference-counted object
 *
 * Returns:			true if pointer points to a valid ReferenceCount object
 *
 * Notes:			NULL is not a valid pointer. In release build only
 *					minimal error-checking is performed. This function
 *					is intended only for debugging purposes!
 *
 *****************************************************************************/

bool ReferenceCount::debugIsValidPointer (const ReferenceCount* p) 
{
	if (!p)
		return false;
	
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();

	if (!Memory::isValidPointer(p))	
		return false;

	if (p->m_referenceCount < 0)		// huh??
		return false;

	return true;
}

/*****************************************************************************
 *
 * Function:		DPVS::ReferenceCount::ReferenceCount()
 *
 * Description:		Constructor
 *
 * Notes:			Initializes the object's reference count to 1.
 *
 *					The input parameter for the ctor is just a hack to
 *					prevent "other people" from inheriting the class. The
 *					function is also inline and not exported at all.
 *
 * See Also:		DPVS::ReferenceCount::release(),DPVS::ReferenceCount::autoRelease()
 *
 *****************************************************************************/

DPVS_FORCE_INLINE ReferenceCount::ReferenceCount(class ImpReferenceCount*) : 
	m_referenceCount(1),
	m_autoReleased(false),
	m_userPointer(null)
{ 
	DPVS_ASSERT_INIT();
	DPVS_ASSERT_SELF_OK();
}

/*****************************************************************************
 *
 * Function:		DPVS::ReferenceCount::~ReferenceCount()
 *
 * Description:		Destructs a reference counted object
 *
 * Notes:			The destructor of all reference-counted objects
 *					is protected. This means that one cannot directly
 *					destruct any reference-counted objects. Instead
 *					the function DPVS::ReferenceCount::release()
 *					should be used. 
 *
 *****************************************************************************/

DPVS_FORCE_INLINE ReferenceCount::~ReferenceCount (void) 			
{ 
	DPVS_ASSERT_INIT();
	DPVS_ASSERT_SELF_OK();
	setName(null);		//remove from hash
}

/*****************************************************************************
 *
 * Function:		DPVS::ReferenceCount::addReference()
 *
 * Description:		Increases reference count of the object by one
 *
 *****************************************************************************/

void ReferenceCount::addReference (void)			
{ 
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_ASSERT_SELF_OK();
	++m_referenceCount;		
}

/*****************************************************************************
 *
 * Function:		DPVS::ReferenceCount::getReferenceCount()
 *
 * Description:		Returns reference count of the object
 *
 * Returns:			reference count of the object
 *
 *****************************************************************************/

int ReferenceCount::getReferenceCount (void) const	
{ 
	DPVS_ASSERT_INIT();
	DPVS_ASSERT_SELF_OK();
	return m_referenceCount; 
}

/*****************************************************************************
 *
 * Function:		DPVS::ReferenceCount::autoRelease()
 *
 * Description:		Decreases reference count of the object by one, but
 *					doesn't destroy object if counter reaches zero.
 *
 * Notes:			Since the object's reference count is initialized to one
 *					in the constructor, this function can be used to
 *					let DPVS "internally manage" the object. For example
 *					the user can create an DPVS::MeshModel, assign it
 *					to some object (the object will take a reference to
 *					the model, thus increasing the reference count to two).
 *					By calling the autorelease function, the reference
 *					becomes one. When the object is destructed, the second
 *					reference is released and the model is automatically
 *					destructed.
 *
 *					If the function is called multiple times, the subsequent
 *					calls don't decrease the reference count.
 *
 * See Also:		DPVS::ReferenceCount::release()
 *
 *****************************************************************************/

void ReferenceCount::autoRelease (void)			
{ 
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_API_ASSERT(this && "ReferenceCount::autoRelease() -- autoreleasing null pointer");
	if (!this || m_autoReleased)
		return;
	DPVS_ASSERT_SELF_OK();
	m_autoReleased = true; 
	m_referenceCount--; 
}

/*****************************************************************************
 *
 * Function:		DPVS::ReferenceCount::release()
 *
 * Description:		Decreases reference count of the object by one. If count
 *					reaches zero or becomes negative, the object's destructor
 *					is called.
 *
 * Returns:			true if object got destructed, false otherwise
 *
 * Notes:			It is valid to call this function for a null pointer
 *					(in such a case nothing is done and 'true' is returned).
 *
 * See Also:		DPVS::ReferenceCount::autoRelease(), DPVS::ReferenceCount()::~ReferenceCount()
 *
 *****************************************************************************/

bool ReferenceCount::release (void) const	
{ 
	DPVS_ASSERT_INIT();
	if(!this) 
		return true; 
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_SELF_OK();

	const_cast<ReferenceCount*>(this)->m_referenceCount--;

	if(m_referenceCount<=0)		// we reached zero
	{ 
		destruct();				// kill the object
		return true;				
	} 
	return false; 
}

/*****************************************************************************
 *
 * Function:		DPVS::ReferenceCount::destruct()
 *
 * Description:		"Virtual" destructor for all ReferenceCounted objects
 *
 * Notes:			The function should call the dtor of the object and
 *					release the associated memory. 
 *
 *					This function is provided so that if classes perform
 *					trickery in their ::create() functions, they can fix
 *					things here...
 *
 *					The function is marked "const" just for the looks
 *
 *****************************************************************************/

void ReferenceCount::destruct (void) const
{
	if (this)						// DELETE(this)
	{
		DPVS_MUTEX_LOCK
		DPVS_ASSERT_SELF_OK();
		this->~ReferenceCount();
		FREE(const_cast<ReferenceCount*>(this));
	}
}	

/*****************************************************************************
 *
 * Function:		DPVS::ReferenceCount::getName()
 *
 * Description:		Returns name of the object or empty string ""
 *
 * Returns:			constant string containing the name of the object
 *
 *****************************************************************************/

const char* ReferenceCount::getName (void) const			
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_ASSERT_SELF_OK();
	const char* name = g_nameHash.get(this); 
	return name ? name : ""; 
}

/*****************************************************************************
 *
 * Function:		DPVS::ReferenceCount::setName()
 *
 * Description:		Assigns a name for the object (debugging purposes)
 *
 * Parameters:		name = name of the object (may be null)
 *
 * Notes:			Submit null as name if the name string should be removed
 *
 *****************************************************************************/

void ReferenceCount::setName (const char *name)
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_ASSERT_SELF_OK();
	g_nameHash.insert(this, name);		// this works even if name is null (in which case it will just be removed)
}

/*****************************************************************************
 *
 * Function:		DPVS::Camera::Camera()
 *
 * Description:		Constructs a new camera 
 *
 * Parameters:		imp = pointer to ImpCamera
 *
 *****************************************************************************/

DPVS_FORCE_INLINE Camera::Camera (class ImpCamera* imp) : ReferenceCount(null), m_imp(imp)
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_ASSERT(m_imp);
	DPVS_ASSERT_SELF_OK();
	m_imp->setUserCamera(this);
}

/*****************************************************************************
 *
 * Function:		DPVS::Camera::~Camera()
 *
 * Description:		Destructor
 *
 * Notes:			The destructor cannot be called directly. Instead call
 *					DPVS::Camera::release().
 *
 *****************************************************************************/

Camera::~Camera()
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_ASSERT_SELF_OK();
	DPVS_ASSERT(m_imp);
	DELETE(m_imp);
	m_imp = null;
}

/*****************************************************************************
 *
 * Function:		DPVS::Camera::create()
 *
 * Description:		Public construction function that can fail
 *
 * Notes:			The Camera class does not use the asbestos patterns
 *					since we must be able to use copy ctors for the ImpCameras...
 *
 *****************************************************************************/

Camera* Camera::create (void)
{
	DPVS_MUTEX_LOCK
	// here we could perform error checking etc. (memory availability etc.)
	DPVS_ASSERT_INIT();
	ImpCamera* imp = NEW<ImpCamera>();
	return new(MALLOC(sizeof(Camera))) Camera(imp);
}

/*****************************************************************************
 *
 * Function:		DPVS::Cell::create()
 *
 * Returns:			pointer to cell or NULL on failure
 *
 *****************************************************************************/

DPVS_FORCE_INLINE Cell::Cell(ImpReferenceCount*) : ReferenceCount(null) {}
Cell* Cell::create (void)
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	char* foo = NEW_ARRAY<char>(sizeof(Cell)+sizeof(ImpCell));
	new(foo+sizeof(Cell)) ImpCell();
	new(foo)Cell(null);
	return reinterpret_cast<Cell*>(foo);
}

/*****************************************************************************
 *
 * Function:		DPVS::Cell::destruct()
 *
 * Description:		Destroys the cell
 *
 *****************************************************************************/

void Cell::destruct (void) const
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_ASSERT_SELF_OK();
	char* foo = (char*)(const_cast<Cell*>(this));
	getImplementation()->~ImpCell();	// call destructor for implementation
	this->~Cell();						// call destructor for self
	DELETE_ARRAY(foo);						// release the memory
}

/*****************************************************************************
 *
 * Function:		DPVS::Model::destruct()
 *
 * Description:		Model destructor
 *
 *****************************************************************************/

DPVS_FORCE_INLINE Model::Model(ImpReferenceCount*) : ReferenceCount(null){} // HACK HACK!

void Model::destruct (void) const
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_ASSERT_SELF_OK();
	char* foo = (char*)(const_cast<Model*>(this));
	getImplementation()->~ImpModel();	// call destructor for implementation
	this->~Model();						// call destructor for self
	DELETE_ARRAY(foo);						// release the memory
}
	

/*****************************************************************************
 *
 * Function:		DPVS::MeshModel::create(const Vector3*,const Vector3i*, int, int,bool)
 *
 * Description:		Constructs a new mesh model
 *
 * Parameters:		pos = vertex positions in object space (cannot be null)
 *					ind = triangle vertex indices (cannot be null)
 *					vn  = number of vertices in mesh (must be > 0)
 *					tn	= number of triangles in mesh (must be > 0)
 *					clockwise = are triangle indices in clockwise order?
 *
 * Notes:			A copy of the input vertex/triangle data is taken 
 *					internally. Thus the application user is free to
 *					release the original mesh data after this call.
 *
 *					Each triangle should have three vertex indices in the 
 *					range [0,vnum[. 
 *
 *					DPVS performs some mesh analysis/optimizations inside
 *					the constructor (takes a short while). For example 
 *					duplicate vertices and degenerate triangles are removed.
 *
 *****************************************************************************/

DPVS_FORCE_INLINE MeshModel::MeshModel (ImpReferenceCount*) : Model(null){}

MeshModel* MeshModel::create (const Vector3* vertices, const Vector3i* triangles,int numVertices,int numTriangles, bool clockwise)
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_API_ASSERT("MeshModel::create() - NULL 'vertices' pointer" && vertices);
	DPVS_API_ASSERT("MeshModel::create() - NULL 'triangles' pointer" && triangles);
	DPVS_API_ASSERT("MeshModel::create() - zero or negative vertices" && numVertices>0);
	DPVS_API_ASSERT("MeshModel::create() - zero or negative triangles" && numTriangles>0);

	if (!vertices || !triangles || numVertices<=0 || numTriangles<=0)
		return null;

	char* foo = NEW_ARRAY<char>(sizeof(MeshModel)+sizeof(ImpMeshModel));
	new(foo+sizeof(MeshModel)) ImpMeshModel(vertices,triangles,numVertices,numTriangles,clockwise);
	new(foo)MeshModel(null);
	return reinterpret_cast<MeshModel*>(foo);
}

/*****************************************************************************
 *
 * Function:		DPVS::OBBModel::create (const Matrix4x4& obb)
 *
 * Description:		Constructs a new OBB model
 *
 * Parameters:		obb = 4x3 matrix describing the OBB
 *
 * Returns:			pointer to OBB model or NULL on failure
 *
 *****************************************************************************/

DPVS_FORCE_INLINE OBBModel::OBBModel (ImpReferenceCount*) : Model(null) {}
OBBModel* OBBModel::create (const Matrix4x4& obb)
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();

	Matrix4x3 m(NO_CONSTRUCTOR);
	MatrixConverter::convertMatrix(m,obb);
	DPVS_API_ASSERT("OBBModel::create() - matrix contains zero scaling terms!" && Math::isInvertible(m));

	char* foo = NEW_ARRAY<char>(sizeof(OBBModel)+sizeof(ImpOBBModel));
	new(foo+sizeof(OBBModel)) ImpOBBModel(m);
	new(foo)OBBModel(null);
	return reinterpret_cast<OBBModel*>(foo);
}

/*****************************************************************************
 *
 * Function:		DPVS::OBBModel::create(const Vector3* vertices, int numVertices)
 *
 * Description:		Constructs a new OBB model
 *
 * Parameters:		vertices    = vertices to be used for OBB calculation
 *					numVertices = number of vertices
 *
 * Returns:			pointer to OBB model or NULL on failure
 *
 *****************************************************************************/

OBBModel* OBBModel::create (const Vector3* vertices, int numVertices)
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_API_ASSERT("OBBModel::create() - invalid input values" && vertices && numVertices>0);
	if (!vertices || numVertices<=0)
		return null;

	char* foo = NEW_ARRAY<char>(sizeof(OBBModel)+sizeof(ImpOBBModel));
	new(foo+sizeof(OBBModel)) ImpOBBModel(vertices,numVertices);
	new(foo)OBBModel(null);
	return reinterpret_cast<OBBModel*>(foo);
}

/*****************************************************************************
 *
 * Function:		DPVS::OBBModel::OBBModel(const Vector3& mn, const Vector3& mx)
 *
 * Description:		Constructs a new OBB model
 *
 * Parameters:		mn = minimum location of AABB
 *					mx = maximum location of AABB
 *
 * Returns:			pointer to OBB model or NULL on failure
 *
 *****************************************************************************/

OBBModel* OBBModel::create (const Vector3& mn, const Vector3& mx)
{
	DPVS_ASSERT_INIT();
	DPVS_API_ASSERT("OBBModel::create() - minimum must be <= maximum" && mn.x<=mx.x && mn.y<=mx.y && mn.z<=mx.z);

	AABB	aabb;
	aabb.set(mn,mx);
	aabb.validateBounds();						// make sure the AABB has dimensions..

	OBB		obb;								// calculate OBB from AABB
	calculateOBB(obb,aabb);

	char* foo = NEW_ARRAY<char>(sizeof(OBBModel)+sizeof(ImpOBBModel));
	new(foo+sizeof(OBBModel)) ImpOBBModel(obb.getMatrix());
	new(foo)OBBModel(null);
	return reinterpret_cast<OBBModel*>(foo);
}

/*****************************************************************************
 *
 * Function:		DPVS::SphereModel::create()
 *
 * Description:		Constructs a new sphere model
 *
 * Parameters:		center = object-space center position
 *					radius = object-space radius (must be >= 0.0)
 *
 * Returns:			pointer to constructed model or NULL on failure
 *
 *****************************************************************************/

DPVS_FORCE_INLINE SphereModel::SphereModel (ImpReferenceCount*) : Model(null) {}
SphereModel* SphereModel::create (const Vector3& center,   float radius)
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_API_ASSERT("SphereModel::create() - invalid radius" && radius >= 0.0f && radius <= FLT_MAX);

	char* foo = NEW_ARRAY<char>(sizeof(SphereModel)+sizeof(ImpSphereModel));
	new(foo+sizeof(SphereModel)) ImpSphereModel(center,radius);
	new(foo)SphereModel(null);
	return reinterpret_cast<SphereModel*>(foo);
}

/*****************************************************************************
 *
 * Function:		DPVS::SphereModel::SphereModel()
 *
 * Description:		Constructs a new sphere model
 *
 * Parameters:		vertices	= array of vertices
 *					numVertices = number of vertices in the array
 *
 *****************************************************************************/

SphereModel* SphereModel::create (const Vector3* vertices, int numVertices)
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_API_ASSERT("SphereModel::create() - no vertices specified" && vertices && numVertices>=0);

	Sphere sphere;
	calculateSphere(sphere,vertices,numVertices);

	char* foo = NEW_ARRAY<char>(sizeof(SphereModel)+sizeof(ImpSphereModel));
	new(foo+sizeof(SphereModel)) ImpSphereModel(sphere.getCenter(),sphere.getRadius());
	new(foo)SphereModel(null);
	return reinterpret_cast<SphereModel*>(foo);
}

/*****************************************************************************
 *
 * Function:		DPVS::Object::create(Model*)
 *
 * Description:		Object constructor
 *
 * Parameters:		testModel  = pointer to test model (cannot be null)
 *
 * Notes:			See DPVS::Object::Property for default values of the
 *					object's properties.
 *
 *					The object's transformation matrix is initialized to identity.
 *					The object's cost is initialized based on the test model
 *					cost. The user ID is initialized to 0.
 *
 *					The object is not located in any cell during initialization,
 *					thus a separate call is needed to place the object into some
 *					cell.
 *
 *					A write model hasn't is not defined for the object. In order
 *					to use the object as an occluder, define a write model
 *					by calling Object::setWriteModel().
 *
 *					In order to give a better idea about the actual rendering
 *					cost, one should call DPVS::Object::setCost().
 *
 * Returns:			pointer to Object on success, NULL on failure
 *
 *****************************************************************************/

DPVS_FORCE_INLINE Object::Object (class ImpReferenceCount*) : ReferenceCount(null) {}
Object*	Object::create (Model* testModel)
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_API_ASSERT(testModel && "Object::create() - test model not defined!\n");
	if (!testModel)
		return null;

	char* foo = NEW_ARRAY<char>(sizeof(Object)+sizeof(ImpObject));
	new(foo+sizeof(Object)) ImpObject(testModel);
	new(foo)Object(null);
	return reinterpret_cast<Object*>(foo);
}

/*****************************************************************************
 *
 * Function:		DPVS::Object::destruct()
 *
 * Description:		Object destructor
 *
 *****************************************************************************/

void Object::destruct (void) const
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	char* foo = (char*)(const_cast<Object*>(this));
	getImplementation()->~ImpObject();	// call destructor for implementation
	this->~Object();					// call destructor for self
	DELETE_ARRAY(foo);						// release the memory
}

/*****************************************************************************
 *
 * Function:		DPVS::RegionOfInfluence::create(Model*)
 *
 * Description:		Constructs a new Region Of Influence
 *
 * Parameters:		testModel = pointer to test model (non-null)
 *
 *****************************************************************************/

DPVS_FORCE_INLINE RegionOfInfluence::RegionOfInfluence(class ImpReferenceCount*) : Object(null) {}

RegionOfInfluence* RegionOfInfluence::create (Model* testModel)
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_API_ASSERT(testModel && "RegionOfInfluence::create() - test model not defined!\n");
	if (!testModel)
		return null;

	char* foo = NEW_ARRAY<char>(sizeof(RegionOfInfluence)+sizeof(ImpRegionOfInfluence));
	new(foo+sizeof(RegionOfInfluence)) ImpRegionOfInfluence(testModel);
	new(foo)RegionOfInfluence(null);
	return reinterpret_cast<RegionOfInfluence*>(foo);
}

/*****************************************************************************
 *
 * Function:		DPVS::PhysicalPortal::PhysicalPortal(Model*,Cell*)
 *
 * Description:		Constructs a new physical portal
 *
 * Parameters:		testModel = pointer to test model (non-null)
 *					targetCell = pointer to target cell (can be null)
 *
 *****************************************************************************/

DPVS_FORCE_INLINE PhysicalPortal::PhysicalPortal(class ImpReferenceCount*) : Object(null) {}
PhysicalPortal* PhysicalPortal::create (Model* testModel, Cell* targetCell)
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_API_ASSERT(testModel && "PhysicalPortal::create() - test model not defined!\n");
	if (!testModel)
		return null;
	
	char* foo = NEW_ARRAY<char>(sizeof(PhysicalPortal)+sizeof(ImpPhysicalPortal));
	new(foo+sizeof(PhysicalPortal)) ImpPhysicalPortal(testModel,targetCell);
	new(foo)PhysicalPortal(null);
	return reinterpret_cast<PhysicalPortal*>(foo);
}

/*****************************************************************************
 *
 * Function:		DPVS::VirtualPortal::create(Model*,PhysicalPortal*)
 *
 * Description:		Constructs a new Region Of Influence
 *
 * Parameters:		testModel		= pointer to test model
 *					targetPortal	= pointer to physical portal
 *
 * Returns:			pointer to new VirtualPortal or null on failure
 *
 *****************************************************************************/

DPVS_FORCE_INLINE VirtualPortal::VirtualPortal(class ImpReferenceCount*) : PhysicalPortal(null) {}
VirtualPortal* VirtualPortal::create (Model* testModel, PhysicalPortal* targetPortal)
{
	DPVS_MUTEX_LOCK
	DPVS_ASSERT_INIT();
	DPVS_API_ASSERT(testModel && "VirtualPortal::create() - test model not defined!\n");
	if (!testModel)
		return null;

	char* foo = NEW_ARRAY<char>(sizeof(VirtualPortal)+sizeof(ImpVirtualPortal));
	new(foo+sizeof(VirtualPortal)) ImpVirtualPortal(testModel,null,targetPortal);
	new(foo)VirtualPortal(null);
	return reinterpret_cast<VirtualPortal*>(foo);
}
 
//------------------------------------------------------------------------
