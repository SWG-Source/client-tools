// ======================================================================
//
// ObjectTemplateData.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ObjectTemplateData_H
#define INCLUDED_ObjectTemplateData_H

// ======================================================================

#include "Singleton/Singleton.h"

class AbstractFilesystemTree;

//-----------------------------------------------------------------
/**
* ObjectTemplateData is a singleton which stores the tree structure representing
* available ObjectTemplates.  The tree structure is populated from the user's
* hard drive.  This is based on the assumption that all object templates have
* both a server and client representation.
*
* @todo: the root path for object templates is hard-coded, add to ConfigGodClient
*/

class ObjectTemplateData : public Singleton<ObjectTemplateData>
{
public:

	struct DragMessages
	{
		static const char* const SERVER_TEMPLATE_DRAGGED;
		static const char* const CLIENT_TEMPLATE_DRAGGED;
	};

	ObjectTemplateData  ();
	~ObjectTemplateData ();

	const AbstractFilesystemTree* getTree() const;

	void  repopulate();

private:
	//disabled
	ObjectTemplateData(const ObjectTemplateData & rhs);
	ObjectTemplateData& operator=(const ObjectTemplateData & rhs);

private:
	AbstractFilesystemTree* m_tree;
};
//-----------------------------------------------------------------

inline const AbstractFilesystemTree* ObjectTemplateData::getTree() const
{
	return m_tree;
}

// ======================================================================

#endif
