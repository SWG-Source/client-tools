// ======================================================================
//
// ClientBakedWearableWriter.h
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ClientBakedWearableWriter_H
#define INCLUDED_ClientBakedWearableWriter_H

// ======================================================================

class Object;

// ======================================================================
/**
 * Utility class that writes out the data needed to generate a client-baked
 * wearable from the .mgn(s) and customization data associated with the
 * specified object.
 */ 

class ClientBakedWearableWriter
{
public:

	static bool write(Object const &object, char const *outputFileName);

};

// ======================================================================

#endif
