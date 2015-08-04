// ======================================================================
//
// ShaderEffectList.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ShaderEffectList_H
#define INCLUDED_ShaderEffectList_H

// ======================================================================

class CrcString;
class Iff;
class ShaderEffect;
class StringCompare;
class RecursiveMutex;

// ======================================================================

class ShaderEffectList
{
	friend class ShaderEffect;

public:

	static void install();
	static void assignAsynchronousLoaderFunctions();
	
	static const ShaderEffect *fetch(Iff &iff);
	static const ShaderEffect *fetch(const char *name);
	static const ShaderEffect *fetch(CrcString const &name);

private:

	static void remove();

	static const void           *asynchronousLoaderFetchNoCreate(const char *fileName);
	static void                  asynchronousLoaderRelease(const void *shaderEffect);

	static const ShaderEffect *fetch(CrcString const &fileName, bool create);
	static const ShaderEffect *fetch(CrcString const &fileName, Iff &iff);
	static void                remove(const ShaderEffect *shaderEffect);

	static void                enterCriticalSection();
	static void                leaveCriticalSection();
};

// ======================================================================

#endif

