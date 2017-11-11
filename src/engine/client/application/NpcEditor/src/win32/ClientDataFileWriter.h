// ============================================================================
//
// ClientDataFileWriter.h
// Copyright (c) 2004 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ============================================================================

#ifndef INCLUDED_ClientDataFileWriter_H
#define INCLUDED_ClientDataFileWriter_H

// ----------------------------------------------------------------------------

class Object;

// ----------------------------------------------------------------------------

class ClientDataFileWriter
{
public:

	static bool write(Object const * object, char const *outputFileName, char const *sourceFileName, bool mifSource);

};

// ============================================================================

#endif
