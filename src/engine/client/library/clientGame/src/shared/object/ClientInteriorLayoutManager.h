// ======================================================================
//
// ClientInteriorLayoutManager.h
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ClientInteriorLayoutManager_H
#define INCLUDED_ClientInteriorLayoutManager_H

// ======================================================================

class InteriorLayoutReaderWriter;
class TangibleObject;

// ======================================================================

class ClientInteriorLayoutManager
{
public:

	static void install(bool disableLazyInteriorLayoutCreation);

	static void applyInteriorLayout (TangibleObject * objectWithPortalProperty, InteriorLayoutReaderWriter const * interiorLayoutReader, char const * fileName);
	static void update();
};

// ======================================================================

#endif
