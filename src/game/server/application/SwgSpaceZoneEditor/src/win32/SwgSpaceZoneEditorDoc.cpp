// ======================================================================
//
// SwgSpaceZoneEditorDoc.cpp
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#include "FirstSwgSpaceZoneEditor.h"
#include "SwgSpaceZoneEditorDoc.h"

#include "Configuration.h"
#include "Resource.h"
#include "sharedFile/Iff.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableWriter.h"
#include "SpaceZoneMapView.h"

// ======================================================================

namespace SwgSpaceZoneEditorDocNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	CString const cms_empty;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	SwgSpaceZoneEditorDoc::Object::Type getType(CString const & name)
	{
		if (name == "object/tangible/space/content_infrastructure/quest_nav_point.iff")
			return SwgSpaceZoneEditorDoc::Object::T_navPoint;

		if (name == "object/tangible/space/content_infrastructure/basic_spawner.iff")
			return SwgSpaceZoneEditorDoc::Object::T_spawner;

		if (name == "object/tangible/space/content_infrastructure/basic_patrol_point.iff")
			return SwgSpaceZoneEditorDoc::Object::T_patrolPoint;

		return SwgSpaceZoneEditorDoc::Object::T_misc;
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
}

using namespace SwgSpaceZoneEditorDocNamespace;

// ======================================================================

SwgSpaceZoneEditorDoc::Object::Object(CString const & objectTemplateName) :
	m_objectTemplateName(objectTemplateName),
	m_type(SwgSpaceZoneEditorDocNamespace::getType(objectTemplateName)),
	m_transform_o2w(),
	m_originalObjVars(),
	m_objVarList(),
	m_scripts()
{
}

// ----------------------------------------------------------------------

SwgSpaceZoneEditorDoc::Object::Object(Object const & rhs) :
	m_objectTemplateName(rhs.m_objectTemplateName),
	m_type(rhs.m_type),
	m_transform_o2w(rhs.m_transform_o2w),
	m_originalObjVars(rhs.m_originalObjVars),
	m_objVarList(rhs.m_objVarList),
	m_scripts(rhs.m_scripts)
{
}

// ----------------------------------------------------------------------

SwgSpaceZoneEditorDoc::Object::Type SwgSpaceZoneEditorDoc::Object::getType() const
{
	return m_type;
}

// ----------------------------------------------------------------------

bool SwgSpaceZoneEditorDoc::Object::isNavPoint() const
{
	return getType() == T_navPoint;
}

// ----------------------------------------------------------------------

bool SwgSpaceZoneEditorDoc::Object::isSpawner() const
{
	return getType() == T_spawner;
}

// ----------------------------------------------------------------------

bool SwgSpaceZoneEditorDoc::Object::isPatrolPointSpawner() const
{
	return getType() == T_spawner && hasObjVar("strPatrolPoints");
}

// ----------------------------------------------------------------------

bool SwgSpaceZoneEditorDoc::Object::isPatrolPoint() const
{
	return getType() == T_patrolPoint;
}

// ----------------------------------------------------------------------

Transform const & SwgSpaceZoneEditorDoc::Object::getTransform_o2w() const
{
	return m_transform_o2w;
}

// ----------------------------------------------------------------------

void SwgSpaceZoneEditorDoc::Object::setTransform_o2w(Transform const & transform_o2w)
{
	m_transform_o2w = transform_o2w;
}

// ----------------------------------------------------------------------

CString const & SwgSpaceZoneEditorDoc::Object::getName() const
{
	switch (m_type)
	{
	case T_navPoint:
		return getObjVar("nav_name");

	case T_spawner:
		return getObjVar("strSpawnerName");

	case T_patrolPoint:
		return getObjVar("strName");
	}

	return m_objectTemplateName;
}

// ----------------------------------------------------------------------

void SwgSpaceZoneEditorDoc::Object::setName(CString const & name)
{
	switch (m_type)
	{
	case T_navPoint:
		setObjVar("nav_name", name);
		break;

	case T_spawner:
		setObjVar("strSpawnerName", name);
		break;

	case T_patrolPoint:
		setObjVar("strName", name);
		break;
	}
}

// ----------------------------------------------------------------------

CString const & SwgSpaceZoneEditorDoc::Object::getObjectTemplateName() const
{
	return m_objectTemplateName;
}

// ----------------------------------------------------------------------

CString const & SwgSpaceZoneEditorDoc::Object::getPatrolPoints() const
{
	return getObjVar("strPatrolPoints");
}

// ----------------------------------------------------------------------

void SwgSpaceZoneEditorDoc::Object::setPatrolPoints(CString const & patrolPoints)
{
	setObjVar("strPatrolPoints", patrolPoints);
}

// ----------------------------------------------------------------------

int SwgSpaceZoneEditorDoc::Object::getNumberOfObjVars() const
{
	return static_cast<int>(m_objVarList.size());
}

// ----------------------------------------------------------------------

Configuration::ObjVar const & SwgSpaceZoneEditorDoc::Object::getObjVar(int const index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getNumberOfObjVars());
	return m_objVarList[static_cast<size_t>(index)];
}

// ----------------------------------------------------------------------

bool SwgSpaceZoneEditorDoc::Object::hasObjVar(CString const & key) const
{
	for (size_t i = 0; i < m_objVarList.size(); ++i)
		if (m_objVarList[i].m_key == key)
			return true;

	return false;
}

// ----------------------------------------------------------------------

CString const & SwgSpaceZoneEditorDoc::Object::getObjVar(CString const & key) const
{
	for (size_t i = 0; i < m_objVarList.size(); ++i)
		if (m_objVarList[i].m_key == key)
			return m_objVarList[i].m_value;

	DEBUG_FATAL(true, ("Unknown objVar %s for object %s [%s]", key, m_objectTemplateName, m_originalObjVars));
	return cms_empty;
}

// ----------------------------------------------------------------------

void SwgSpaceZoneEditorDoc::Object::setObjVar(CString const & key, CString const & value)
{
	for (size_t i = 0; i < m_objVarList.size(); ++i)
	{
		if (m_objVarList[i].m_key == key)
		{
			m_objVarList[i].m_value = value;

			break;
		}
	}
}

// ----------------------------------------------------------------------

Configuration::ObjVarList const & SwgSpaceZoneEditorDoc::Object::getObjVarList() const
{
	return m_objVarList;
}

// ----------------------------------------------------------------------

CString const & SwgSpaceZoneEditorDoc::Object::getOriginalObjVars() const
{
	return m_originalObjVars;
}

// ----------------------------------------------------------------------

void SwgSpaceZoneEditorDoc::Object::setOriginalObjVars(CString const & objVars)
{
	m_originalObjVars = objVars;

	// Clear any current ObjVars we have. This was causing an error where some
	// Objects were getting 2 copies of their objvars.
	m_objVarList.clear();

	Configuration::unpackObjVars(objVars, m_objVarList);
}

// ----------------------------------------------------------------------

CString const & SwgSpaceZoneEditorDoc::Object::getScripts() const
{
	return m_scripts;
}

// ----------------------------------------------------------------------

void SwgSpaceZoneEditorDoc::Object::setScripts(CString const & scripts)
{
	m_scripts = scripts;
}

// ----------------------------------------------------------------------

SwgSpaceZoneEditorDoc::Object const * SwgSpaceZoneEditorDoc::Object::getParent() const
{
	return m_parent;
}

// ----------------------------------------------------------------------

SwgSpaceZoneEditorDoc::Object * SwgSpaceZoneEditorDoc::Object::getParent()
{
	return m_parent;
}

// ----------------------------------------------------------------------

void SwgSpaceZoneEditorDoc::Object::setParent(Object * const parent)
{
	m_parent = parent;
}

// ----------------------------------------------------------------------

void SwgSpaceZoneEditorDoc::Object::draw(SpaceZoneMapView const * const mapView, CDC * const pDC) const
{
	Vector const position_w = m_transform_o2w.getPosition_p();
	int const radius = std::max(4, mapView->convertWorldToScreen(32));
	CPoint const point = mapView->convertWorldToScreen(position_w);
	pDC->Ellipse(point.x - radius, point.y - radius, point.x + radius, point.y + radius);
}

// ======================================================================

IMPLEMENT_DYNCREATE(SwgSpaceZoneEditorDoc, CDocument)

BEGIN_MESSAGE_MAP(SwgSpaceZoneEditorDoc, CDocument)
	//{{AFX_MSG_MAP(SwgSpaceZoneEditorDoc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ======================================================================

SwgSpaceZoneEditorDoc::SwgSpaceZoneEditorDoc() :
	m_objectList(),
	m_selectedObject(0),
	m_selectedSpawner(0),
	m_patrolPointMap(),
	m_mapPosition(),
	m_currentFile()
{
}

// ----------------------------------------------------------------------

SwgSpaceZoneEditorDoc::~SwgSpaceZoneEditorDoc()
{
	std::for_each(m_objectList.begin(), m_objectList.end(), PointerDeleter());
	m_objectList.clear();
}

// ----------------------------------------------------------------------

BOOL SwgSpaceZoneEditorDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	return TRUE;
}

// ----------------------------------------------------------------------

void SwgSpaceZoneEditorDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
	}
	else
	{
	}
}

// ----------------------------------------------------------------------

#ifdef _DEBUG
void SwgSpaceZoneEditorDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void SwgSpaceZoneEditorDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

// ----------------------------------------------------------------------

BOOL SwgSpaceZoneEditorDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	m_currentFile = lpszPathName;

	//-- Load space_mobile.iff
	if (TreeFile::exists(lpszPathName))
	{
		DEBUG_REPORT_LOG(true, ("Loading %s\n", lpszPathName));

		std::for_each(m_objectList.begin(), m_objectList.end(), PointerDeleter());
		m_objectList.clear();

		DataTableWriter writer;
		writer.loadFromSpreadsheet(lpszPathName);

		Iff iff(1024);
		iff.allowNonlinearFunctions();
		writer.save(iff);
		iff.goToTopOfForm();

		DataTable dataTable;
		dataTable.load(iff);

		bool objVarsPackedUnpackedCorrectly = true;
		{
			PatrolPointMap loadedPatrolPointMap;

			//-- Load rows of the datatable.  Patrol points will not be added to the object list until after processing
			{
				for (int i = 0; i < dataTable.getNumRows(); ++i)
				{
					CString const objectTemplateName = dataTable.getStringValue(std::string("strObject"), i);

					Object * const object = new Object(objectTemplateName);

					Vector j;
					j.x = dataTable.getFloatValue(std::string("fltJX"), i);
					j.y = dataTable.getFloatValue(std::string("fltJY"), i);
					j.z = dataTable.getFloatValue(std::string("fltJZ"), i);

					Vector k;
					k.x = dataTable.getFloatValue(std::string("fltKX"), i);
					k.y = dataTable.getFloatValue(std::string("fltKY"), i);
					k.z = dataTable.getFloatValue(std::string("fltKZ"), i);

					Vector position;
					position.x = dataTable.getFloatValue(std::string("fltPX"), i);
					position.y = dataTable.getFloatValue(std::string("fltPY"), i);
					position.z = dataTable.getFloatValue(std::string("fltPZ"), i);

					Transform transform_o2w;
					transform_o2w.setLocalFrameKJ_p(k, j);
					transform_o2w.setPosition_p(position);
					object->setTransform_o2w(transform_o2w);

					object->setOriginalObjVars(dataTable.getStringValue(std::string("strObjVars"), i));
					object->setScripts(dataTable.getStringValue(std::string("strScripts"), i));

					if (object->getType() == Object::T_patrolPoint)
					{
						if (!object->hasObjVar("strName"))
						{
							delete object;
							continue;
						}

						std::pair<PatrolPointMap::iterator, bool> result = loadedPatrolPointMap.insert(std::make_pair(object->getName(), object));
						if (!result.second)
						{
							CString message;
							message.Format("Multiple objects with name %s already exist.\n", object->getName());
							DEBUG_REPORT_LOG(true, ("%s\n", message));
							MessageBox(0, message, "Error", MB_OK);

							return FALSE;
						}
					}
					else
						m_objectList.push_back(object);

					{
						Configuration::ObjVarList objVarList;
						Configuration::unpackObjVars(object->getOriginalObjVars(), objVarList);
						for (size_t j = 0; j < objVarList.size(); ++j)
						{
							Configuration::ObjVar const & objVar = objVarList[j];

							Configuration::PropertyTemplate const * const propertyTemplate = Configuration::getPropertyTemplate(objVar.m_key);
							if (!propertyTemplate)
							{
								CString const message(CString("There is no property information found for objVar ") + objVar.m_key + " in SwgSpaceZoneEditor.ini.  Please consult asommers or drubenfield.");
								CString const caption(CString("Can't open ") + lpszPathName);
								DEBUG_REPORT_LOG(true, ("%s\n", message));
								MessageBox(0, message, caption, MB_OK);

								return FALSE;
							}

//							DEBUG_REPORT_LOG(true, ("xxx: %s=%s\n", objVar.m_key, objVar.m_value)); 
						}

#if 0
						//-- Pack/unpack verification
						{
							CString objVars;
							Configuration::packObjVars(objVarList, objVars);
							if (objVars != object->getOriginalObjVars())
							{
								objVarsPackedUnpackedCorrectly = false;
								DEBUG_REPORT_LOG(true, ("%s\n\told=%s\n\tnew=%s\n", object->getName(), object->getOriginalObjVars(), objVars));
							}
						}
#endif
					}
				}
			}

			DEBUG_REPORT_LOG(true, ("%i %i\n", m_objectList.size(), loadedPatrolPointMap.size()));

			//-- Make a second pass, fixup patrol point names, and remove any patrol points are orphaned
			{
				int i;
				for (i = 0; i < getNumberOfObjects(); ++i)
				{
					Object * const object = getObject(i);
					if (object->isPatrolPointSpawner())
					{
						StringList patrolPointList;

						StringList loadedPatrolPointList;
						Configuration::unpackString(object->getPatrolPoints(), loadedPatrolPointList, ':');
						for (size_t j = 0; j < loadedPatrolPointList.size(); ++j)
						{
							PatrolPointMap::iterator iter = loadedPatrolPointMap.find(loadedPatrolPointList[j]);
							if (iter != loadedPatrolPointMap.end())
							{
								Object * const loadedPatrolPoint = iter->second;
								if (loadedPatrolPoint)
								{
									Object * const patrolPoint = new Object(*loadedPatrolPoint);

									CString patrolPointName;
									patrolPointName.Format("%s_%02d", object->getName(), j);
									patrolPoint->setName(patrolPointName);
									patrolPoint->setParent(object);
									patrolPointList.push_back(patrolPointName);
									m_objectList.push_back(patrolPoint);

									std::pair<PatrolPointMap::iterator, bool> result = m_patrolPointMap.insert(std::make_pair(patrolPointName, patrolPoint));
									DEBUG_FATAL(!result.second, ("%s already exists\n", patrolPointName));
								}
							}
						}

						CString patrolPoints;
						Configuration::packString(patrolPointList, patrolPoints, ':');
						object->setPatrolPoints(patrolPoints);
					}
				}

				DEBUG_REPORT_LOG(true, ("%i %i\n", m_objectList.size(), loadedPatrolPointMap.size()));

				//-- Delete everything out of the loaded patrol point map
				std::for_each(loadedPatrolPointMap.begin(), loadedPatrolPointMap.end(), PointerDeleterPairSecond());
			}
		}

		if (!objVarsPackedUnpackedCorrectly)
			MessageBox(0, "Found a case where the objVars did not pack to the exact same string as unpack.  Please check the DebugView output and/or consult asommers.", "SwgSpaceZoneEditor", MB_OK);

		return TRUE;
	}

	return FALSE;
}

// ----------------------------------------------------------------------

int SwgSpaceZoneEditorDoc::getNumberOfObjects() const
{
	return static_cast<int>(m_objectList.size());
}

// ----------------------------------------------------------------------

SwgSpaceZoneEditorDoc::Object const * SwgSpaceZoneEditorDoc::getObject(int const index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getNumberOfObjects());
	return m_objectList[static_cast<size_t>(index)];
}

// ----------------------------------------------------------------------

SwgSpaceZoneEditorDoc::Object * SwgSpaceZoneEditorDoc::getObject(int const index)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getNumberOfObjects());
	return m_objectList[static_cast<size_t>(index)];
}

// ----------------------------------------------------------------------

void SwgSpaceZoneEditorDoc::selectObject(SwgSpaceZoneEditorDoc::Object const * const object)
{
	m_selectedObject = object;

	if (object && (object->isPatrolPoint() || object->isSpawner()))
	{
		Object const * const spawner = object->isPatrolPoint() ? object->getParent() : object;
		m_selectedSpawner = spawner;
	}
	
	UpdateAllViews(0, SwgSpaceZoneEditorDoc::H_selectObject, const_cast<CObject *>(reinterpret_cast<CObject const *>(object)));
}

// ----------------------------------------------------------------------

SwgSpaceZoneEditorDoc::Object const * SwgSpaceZoneEditorDoc::getSelectedObject() const
{
	return m_selectedObject;
}

// ----------------------------------------------------------------------

SwgSpaceZoneEditorDoc::Object * SwgSpaceZoneEditorDoc::getSelectedObject()
{
	return const_cast<Object *>(m_selectedObject);
}

// ----------------------------------------------------------------------

SwgSpaceZoneEditorDoc::Object const * SwgSpaceZoneEditorDoc::getSelectedSpawner() const
{
	return m_selectedSpawner;
}

// ----------------------------------------------------------------------

SwgSpaceZoneEditorDoc::Object const * SwgSpaceZoneEditorDoc::findPatrolPoint(CString const & name) const
{
	return const_cast<SwgSpaceZoneEditorDoc *>(this)->findPatrolPoint(name);
}

// ----------------------------------------------------------------------

SwgSpaceZoneEditorDoc::Object * SwgSpaceZoneEditorDoc::findPatrolPoint(CString const & name)
{
	PatrolPointMap::const_iterator iter = m_patrolPointMap.find(name);
	if (iter != m_patrolPointMap.end())
		return iter->second;

	return 0;
}

// ----------------------------------------------------------------------

void SwgSpaceZoneEditorDoc::deleteObject(Object * const object)
{
	if (object->isPatrolPointSpawner())
	{
		StringList patrolPointList;
		Configuration::unpackString(object->getPatrolPoints(), patrolPointList, ':');
		for (size_t i = 0; i < patrolPointList.size(); ++i)
		{
			PatrolPointMap::iterator iter = m_patrolPointMap.find(patrolPointList[i]);
			if (iter != m_patrolPointMap.end())
			{
				Object * const patrolPoint = iter->second;

				m_objectList.erase(std::find(m_objectList.begin(), m_objectList.end(), patrolPoint));
				delete patrolPoint;

				m_patrolPointMap.erase(iter);
			}
		}
	}

	if (object->getType() == Object::T_patrolPoint)
	{
		m_patrolPointMap.erase(m_patrolPointMap.find(object->getName()));

		SwgSpaceZoneEditorDoc::Object * const spawner = object->getParent();
		StringList patrolPointList;
		Configuration::unpackString(spawner->getPatrolPoints(), patrolPointList, ':');
		patrolPointList.erase(std::find(patrolPointList.begin(), patrolPointList.end(), object->getName()));
		CString patrolPoints;
		Configuration::packString(patrolPointList, patrolPoints, ':');
		spawner->setPatrolPoints(patrolPoints);
	}

	m_objectList.erase(std::find(m_objectList.begin(), m_objectList.end(), object));

	UpdateAllViews(0, H_deleteObject, reinterpret_cast<CObject *>(object));

	delete object;
}

// ----------------------------------------------------------------------

void SwgSpaceZoneEditorDoc::duplicateObject(Object const * object, CString const newName)
{
	// Get the original ObjVars and replace the name value contained within them.
	CString objVars = object->getOriginalObjVars();
	objVars.Replace(object->getName(), newName );
	
	// Create a new object which is just a copy of the old object.
	Object * const newObject = new Object( *object );
	bool const spawner  = newObject->isPatrolPointSpawner();

	// If we are a spawner, we need to grab the patrol list before we overwrite the Original ObjVars.
	StringList loadedPatrolPointList;
	if(spawner)
	{
		Configuration::unpackString(newObject->getPatrolPoints(), loadedPatrolPointList, ':');
	}
	
	// Change the name and the ObjVars to the new ObjVars
	newObject->setName( newName );
	newObject->setOriginalObjVars( objVars );

	// Push it on the vector.
	m_objectList.push_back( newObject );

	// Spawner type, we need to copy and rename all the spawn points.
	if(spawner)
	{
		PatrolPointMap loadedPatrolPointMap;

		StringList patrolPointList;

		for (size_t j = 0; j < loadedPatrolPointList.size(); ++j)
		{
			PatrolPointMap::iterator iter = m_patrolPointMap.find(loadedPatrolPointList[j]);
			if (iter != m_patrolPointMap.end())
			{
					Object * const loadedPatrolPoint = iter->second;
					if (loadedPatrolPoint)
					{
						Object * const patrolPoint = new Object(*loadedPatrolPoint);

						CString patrolPointName;
						patrolPointName.Format("%s_%02d", newObject->getName(), j);
						patrolPoint->setName(patrolPointName);
						patrolPoint->setParent(newObject);
						patrolPointList.push_back(patrolPointName);
						m_objectList.push_back(patrolPoint);

						std::pair<PatrolPointMap::iterator, bool> result = m_patrolPointMap.insert(std::make_pair(patrolPointName, patrolPoint));
						DEBUG_FATAL(!result.second, ("%s already exists\n", patrolPointName));
					}
			}


		}

		// Write the new, copied Nav points to the ObjVars.
		CString patrolPoints;
		Configuration::packString(patrolPointList, patrolPoints, ':');
		newObject->setPatrolPoints(patrolPoints);
	}

	// Depending on the type we need to update differently. This will update the tree view.
	switch(newObject->getType())
	{
	case SwgSpaceZoneEditorDoc::Object::T_misc:
		UpdateAllViews(0, H_addMisc, reinterpret_cast<CObject *>(newObject));
		break;
	case SwgSpaceZoneEditorDoc::Object::T_navPoint:
		UpdateAllViews(0, H_addNavPoint, reinterpret_cast<CObject *>(newObject));
		break;
	case SwgSpaceZoneEditorDoc::Object::T_spawner:
		UpdateAllViews(0, H_addSpawner, reinterpret_cast<CObject *>(newObject));
		break;
	case SwgSpaceZoneEditorDoc::Object::T_patrolPoint:
		UpdateAllViews(0, H_addPatrolPoint, reinterpret_cast<CObject *>(newObject));
		break;
	default:
		MessageBox(NULL, "Unknown Object Type when trying to duplicate.", "Warning - Unknown type", MB_ICONWARNING|MB_OK);
		break;
	}
}

// ----------------------------------------------------------------------

void SwgSpaceZoneEditorDoc::addPatrolPoint(Object * object)
{
	if (object->isPatrolPoint())
		object = object->getParent();

	if (!object->isPatrolPointSpawner())
		return;

	if (m_patrolPointMap.empty())
		return;

	//-- Unpack patrol points
	StringList patrolPointList;
	Configuration::unpackString(object->getPatrolPoints(), patrolPointList, ':');

	//-- Create new patrol point
	Object * const patrolPoint = new Object(*m_patrolPointMap.begin()->second);

	int patrolPointIndex = 0;
	if (!patrolPointList.empty())
	{
		CString lastPatrolPointName(patrolPointList.back());
		lastPatrolPointName = lastPatrolPointName.Right(2);
		patrolPointIndex = atoi(lastPatrolPointName);
	}

	CString patrolPointName;
	patrolPointName.Format("%s_%02d", object->getName(), patrolPointIndex + 1);
	patrolPoint->setName(patrolPointName);
	patrolPoint->setParent(object);
	patrolPoint->setTransform_o2w(patrolPointList.empty() ? object->getTransform_o2w() : findPatrolPoint(patrolPointList.back())->getTransform_o2w());
	patrolPointList.push_back(patrolPointName);
	m_objectList.push_back(patrolPoint);
	std::pair<PatrolPointMap::iterator, bool> result = m_patrolPointMap.insert(std::make_pair(patrolPointName, patrolPoint));
	DEBUG_FATAL(!result.second, ("%s already exists\n", patrolPointName));

	CString patrolPoints;
	Configuration::packString(patrolPointList, patrolPoints, ':');
	object->setPatrolPoints(patrolPoints);

	UpdateAllViews(0, H_addPatrolPoint, reinterpret_cast<CObject *>(patrolPoint));

	selectObject(patrolPoint);
}

// ----------------------------------------------------------------------

BOOL SwgSpaceZoneEditorDoc::OnSaveDocument(LPCTSTR lpszPathName) 
{
	UNREF(lpszPathName);

	CString buffer;
	buffer += "strObject\tfltJX\tfltJY\tfltJZ\tfltKX\tfltKY\tfltKZ\tfltPX\tfltPY\tfltPZ\tstrObjVars\tstrScripts\n";
	buffer += "s\tf\tf\tf\tf\tf\tf\tf\tf\tf\tp\ts\n";

	for (int i = 0; i < getNumberOfObjects(); ++i)
	{
		Object const * const object = getObject(i);

		CString objVars;
		Configuration::packObjVars(object->getObjVarList(), objVars);

		Transform const & transform_o2w = object->getTransform_o2w();

		CString temp;
		temp.Format("%s\t%1.6f\t%1.6f\t%1.6f\t%1.6f\t%1.6f\t%1.6f\t%1.2f\t%1.2f\t%1.2f\t%s\t%s\n", 
			object->getObjectTemplateName(),
			transform_o2w.getLocalFrameJ_p().x, transform_o2w.getLocalFrameJ_p().y, transform_o2w.getLocalFrameJ_p().z, 
			transform_o2w.getLocalFrameK_p().x, transform_o2w.getLocalFrameK_p().y, transform_o2w.getLocalFrameK_p().z, 
			transform_o2w.getPosition_p().x, transform_o2w.getPosition_p().y, transform_o2w.getPosition_p().z, 
			objVars,
			object->getScripts());

		buffer += temp;
	}

	//-- write tab
	bool wroteTab = false;
	{
		CStdioFile outfile;
		if (outfile.Open(lpszPathName, CFile::modeCreate | CFile::modeWrite | CFile::typeText))
		{
			outfile.WriteString(buffer);
			wroteTab = true;
		}
		else
			MessageBox(0, CString("Could not write ") + lpszPathName, 0, MB_ICONWARNING | MB_OK);
	}

	//-- write iff
	bool wroteIff = false;
	{
		CString iffFileName(lpszPathName);

		//-- Replace dsrc w/ data
		iffFileName.Replace("dsrc", "data");

		//-- Replace .tab w/ .iff
		iffFileName.Replace(".tab", ".iff");

		DataTableWriter writer;
		writer.loadFromSpreadsheet(lpszPathName);

		Iff iff(1024);
		writer.save(iff);

		if (iff.write(iffFileName, true))
			wroteIff = true;
		else
			MessageBox(0, CString("Could not write ") + iffFileName, 0, MB_ICONWARNING | MB_OK);
	}

	if (wroteTab && wroteIff)
	{
		SetModifiedFlag(false);
		return TRUE;
	}

	return FALSE;
}

// ======================================================================
