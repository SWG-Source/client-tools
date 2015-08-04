// ======================================================================
//
// SwgSpaceZoneEditorDoc.h
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgSpaceZoneEditorDoc_H
#define INCLUDED_SwgSpaceZoneEditorDoc_H

// ======================================================================

#include "sharedMath/Vector.h"
#include "sharedMath/Transform.h"

#include "Configuration.h"

class SpaceZoneMapView;

// ======================================================================

class SwgSpaceZoneEditorDoc : public CDocument
{
public:

	enum Hint
	{
		H_unknown,
		H_selectObject,
		H_updateRulers,
		H_deleteObject,
		H_addPatrolPoint,
		H_addNavPoint,
		H_addSpawner,
		H_addMisc,
		H_selectAllPatrolPoints
	};

	class Object
	{
	public:

		enum Type
		{
			T_navPoint,
			T_spawner,
			T_patrolPoint,
			T_misc
		};

	public:

		explicit Object(CString const & objectTemplateName);
		Object(Object const & rhs);

		CString const & getObjectTemplateName() const;

		Type getType() const;
		bool isNavPoint() const;
		bool isSpawner() const;
		bool isPatrolPointSpawner() const;
		bool isPatrolPoint() const;

		Transform const & getTransform_o2w() const;
		void setTransform_o2w(Transform const & transform_o2w);

		CString const & getName() const;
		void setName(CString const & name);

		CString const & getOriginalObjVars() const;
		void setOriginalObjVars(CString const & objVars);

		int getNumberOfObjVars() const;
		Configuration::ObjVar const & getObjVar(int index) const;
		bool hasObjVar(CString const & key) const;
		CString const & getObjVar(CString const & key) const;
		void setObjVar(CString const & key, CString const & value);
		Configuration::ObjVarList const & getObjVarList() const;

		CString const & getScripts() const;
		void setScripts(CString const & scripts);

		CString const & getPatrolPoints() const;
		void setPatrolPoints(CString const & patrolPoints);

		Object const * getParent() const;
		Object * getParent();
		void setParent(Object * parent);

		void draw(SpaceZoneMapView const * mapView, CDC * pDC) const;

	private:

		CString const m_objectTemplateName;
		Type const m_type;
		Transform m_transform_o2w;
		CString m_originalObjVars;
		Configuration::ObjVarList m_objVarList;
		CString m_scripts;
		Object * m_parent;
	};

public:

	virtual ~SwgSpaceZoneEditorDoc (void);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	//{{AFX_VIRTUAL(SwgSpaceZoneEditorDoc)
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	//}}AFX_VIRTUAL

	int getNumberOfObjects() const;
	Object const * getObject(int index) const;
	Object * getObject(int index);

	void selectObject(Object const * object);
	Object const * getSelectedObject() const;
	Object * getSelectedObject();
	Object const * getSelectedSpawner() const;
	
	Object const * findPatrolPoint(CString const & name) const;
	Object * findPatrolPoint(CString const & name);

	void setMapPosition(Vector const & mapPosition);
	Vector const & getMapPosition() const;

	void deleteObject(Object * object);
	void duplicateObject(Object const * object, CString const newName);

	void addPatrolPoint(Object * object);

	CString const & getCurrentFile();

protected:

	SwgSpaceZoneEditorDoc (void);
	DECLARE_DYNCREATE(SwgSpaceZoneEditorDoc)

	//{{AFX_MSG(SwgSpaceZoneEditorDoc)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	typedef std::vector<Object *> ObjectList;
	ObjectList m_objectList;
	Object const * m_selectedObject;
	Object const * m_selectedSpawner;

	typedef std::map<CString, Object *> PatrolPointMap;
	PatrolPointMap m_patrolPointMap;

	Vector m_mapPosition;

	CString	m_currentFile;
};

// ======================================================================

inline void SwgSpaceZoneEditorDoc::setMapPosition (Vector const & mapPosition)
{
	m_mapPosition = mapPosition;
}

// ----------------------------------------------------------------------

inline Vector const & SwgSpaceZoneEditorDoc::getMapPosition() const
{
	return m_mapPosition;
}

// ----------------------------------------------------------------------

inline CString const & SwgSpaceZoneEditorDoc::getCurrentFile()
{
	return m_currentFile;
}

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif
