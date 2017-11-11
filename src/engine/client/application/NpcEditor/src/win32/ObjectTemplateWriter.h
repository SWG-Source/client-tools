// ======================================================================
//
// ObjectTemplateWriter.h
// Copyright (c) 2004 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ObjectTemplateWriter_H
#define INCLUDED_ObjectTemplateWriter_H

// ======================================================================

/**
 * Utility class that writes out template files.
 */ 

class ObjectTemplateWriter
{
public:

	typedef stdmap<std::string, std::string>::fwd ObjectTemplateParameterMap;

	static bool write(char const *outputFileName, char const *templateFileName, ObjectTemplateParameterMap const *parameterMap);
};

// ======================================================================

#endif
