// ======================================================================
//
// ShaderImplementationList.h
//
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ShaderImplementationList_H
#define INCLUDED_ShaderImplementationList_H

// ======================================================================

class Iff;
class ShaderImplementation;
class StringCompare;
class RecursiveMutex;

// ======================================================================

class ShaderImplementationList
{
	friend class ShaderImplementation;

public:

	static void install();
	static void remove();
	
	static const ShaderImplementation *fetch(Iff &iff);
	static const ShaderImplementation *fetch(const char *name);

private:

	static const ShaderImplementation *fetch(const char *name, Iff &iff);
	static void                         remove(const ShaderImplementation *shaderImplementation);

private:

	typedef stdmap<const char *, ShaderImplementation *, StringCompare>::fwd ShaderImplementationMap;

private:

	static RecursiveMutex            ms_criticalSection;
	static ShaderImplementationMap  *ms_shaderImplementationMap;
};

// ======================================================================

#endif
