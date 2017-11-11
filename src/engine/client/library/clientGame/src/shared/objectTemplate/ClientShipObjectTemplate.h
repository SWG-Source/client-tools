// ======================================================================
//
// ClientShipObjectTemplate.h
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef	_INCLUDED_ClientShipObjectTemplate_H
#define	_INCLUDED_ClientShipObjectTemplate_H

// ======================================================================

#include "sharedGame/SharedShipObjectTemplate.h"

class ShipObject;
class InteriorLayoutReaderWriter;

// ======================================================================

class ClientShipObjectTemplate: public SharedShipObjectTemplate
{
public:
	ClientShipObjectTemplate(std::string const &filename);
	virtual ~ClientShipObjectTemplate();

	static void install();

	virtual Object *createObject() const;
	virtual void preloadAssets() const;
	virtual void garbageCollect() const;

	InteriorLayoutReaderWriter const * getInteriorLayout() const;
	void applyInteriorLayout(ShipObject * shipObject) const;

protected:

	virtual void postLoad();

private:
	ClientShipObjectTemplate(ClientShipObjectTemplate const &);
	ClientShipObjectTemplate &operator=(ClientShipObjectTemplate const &);

	static ObjectTemplate *create(std::string const &filename);

private:

	InteriorLayoutReaderWriter * m_interiorLayoutReader;

private:

	class PreloadManager;
	friend class PreloadManager;
	mutable PreloadManager * m_preloadManager;
};

// ======================================================================

#endif	// _INCLUDED_ClientShipObjectTemplate_H

