// ======================================================================
//
// ClientDataTemplateList.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_ClientDataTemplateList_H
#define INCLUDED_ClientDataTemplateList_H

// ======================================================================

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Tag.h"

class CrcString;
class Iff;
class Object;
class ClientDataFile;

// ======================================================================

class ClientDataTemplateList
{
	friend class ClientDataFile;

public:

	typedef ClientDataFile* (*CreateDataResourceFunction) (const std::string& filename);

public:

	static void install ();

	static const ClientDataFile* fetch (const std::string& filename);
	static const ClientDataFile* fetch (const CrcString& filename);
	static const ClientDataFile* fetch (const char* filename);

	static void assignBinding (Tag id, CreateDataResourceFunction createFunc);
	static void removeBinding (Tag tag);

	static void garbageCollect ();

private:

	static void remove ();
	static void release (const ClientDataFile& clientDataFile);

private:

	ClientDataTemplateList ();
	~ClientDataTemplateList ();
	ClientDataTemplateList (const ClientDataTemplateList&);
	ClientDataTemplateList& operator= (const ClientDataTemplateList&);
};

// ======================================================================

#endif	
