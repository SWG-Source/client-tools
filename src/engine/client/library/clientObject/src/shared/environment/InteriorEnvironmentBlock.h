//===================================================================
//
// InteriorEnvironmentBlock.h
// asommers
//
// copyright 2002, sony online entertainment
// 
//===================================================================

#ifndef INCLUDED_InteriorEnvironmentBlock_H
#define INCLUDED_InteriorEnvironmentBlock_H

//===================================================================

#include "sharedMath/VectorArgb.h"

class CrcString;
class PersistentCrcString;
class Texture;

//===================================================================

class InteriorEnvironmentBlockData
{
public:

	InteriorEnvironmentBlockData ();

public:

	const char* m_name;
	const char* m_dayAmbientSoundTemplateName;
	const char* m_nightAmbientSoundTemplateName;
	const char* m_firstMusicSoundTemplateName;
	const char* m_surfaceType;
	bool        m_fogEnabled;
	float       m_fogDensity;
	VectorArgb  m_fogColor;
	const char* m_environmentTextureName;
	int         m_roomType;
};

//===================================================================

class InteriorEnvironmentBlock
{
public:

	InteriorEnvironmentBlock ();
	~InteriorEnvironmentBlock ();

	void                  setData (const InteriorEnvironmentBlockData& data);

	const CrcString*      getName () const;
	const CrcString*      getDayAmbientSoundTemplateName () const;
	const CrcString*      getNightAmbientSoundTemplateName () const;
	const CrcString*      getFirstMusicSoundTemplateName () const;
	const char*           getSurfaceType () const;
	bool                  getFogEnabled () const;
	float                 getFogDensity () const;
	const VectorArgb&     getFogColor () const;
	const Texture*        getEnvironmentTexture () const;
	int                   getRoomType () const;

private:

	void                  clear ();

private:

	InteriorEnvironmentBlock (const InteriorEnvironmentBlock&);
	InteriorEnvironmentBlock& operator= (const InteriorEnvironmentBlock&);

private:

	PersistentCrcString* const m_name;
	PersistentCrcString* const m_dayAmbientSoundTemplateName;
	PersistentCrcString* const m_nightAmbientSoundTemplateName;
	PersistentCrcString* const m_firstMusicSoundTemplateName;
	char*                      m_surfaceType;
	bool                       m_fogEnabled;
	float                      m_fogDensity;
	VectorArgb                 m_fogColor;
	const Texture*             m_environmentTexture;
	int                        m_roomType;
};

//===================================================================

#endif
