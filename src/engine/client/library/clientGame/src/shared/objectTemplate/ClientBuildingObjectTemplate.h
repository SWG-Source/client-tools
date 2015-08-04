// ======================================================================
//
// ClientBuildingObjectTemplate.h
// copyright 2001 Sony Online Entertainment
//
// A wrapper around SharedBuildingObjectTemplate to create BuildingObjects.
//
// ======================================================================

#ifndef INCLUDED_ClientBuildingObjectTemplate_H
#define INCLUDED_ClientBuildingObjectTemplate_H

// ======================================================================

#include "sharedGame/SharedBuildingObjectTemplate.h"

class BuildingObject;
class InteriorLayoutReaderWriter;

// ======================================================================

class ClientBuildingObjectTemplate : public SharedBuildingObjectTemplate
{
public:

	static void install (bool allowDefaultTemplateParams=false);

public:

	explicit ClientBuildingObjectTemplate (const std::string& filename);
	virtual ~ClientBuildingObjectTemplate ();

	virtual Object* createObject () const;
	virtual void    preloadAssets() const;
	virtual void    garbageCollect() const;

	InteriorLayoutReaderWriter const * getInteriorLayout() const;
	void applyInteriorLayout (BuildingObject* buildingObject) const;

protected:

	virtual void postLoad ();

private:

	static ObjectTemplate* create (const std::string& filename);

private:

	ClientBuildingObjectTemplate ();
	ClientBuildingObjectTemplate (const ClientBuildingObjectTemplate&);
	ClientBuildingObjectTemplate& operator= (const ClientBuildingObjectTemplate&);

private:

	InteriorLayoutReaderWriter* const m_interiorLayoutReader;

private:

	class PreloadManager;
	friend class PreloadManager;
	mutable PreloadManager* m_preloadManager;
};

// ======================================================================

#endif
