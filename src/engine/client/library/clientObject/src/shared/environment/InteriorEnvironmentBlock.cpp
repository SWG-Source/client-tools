//===================================================================
//
// InteriorEnvironmentBlock.cpp
// asommers
//
// copyright 2002, sony online entertainment
// 
//===================================================================

#include "clientObject/FirstClientObject.h"
#include "clientObject/InteriorEnvironmentBlock.h"

#include "clientGraphics/Texture.h"
#include "clientGraphics/TextureList.h"
#include "sharedFoundation/PersistentCrcString.h"

//===================================================================
// PUBLIC InteriorEnvironmentBlockData
//===================================================================

InteriorEnvironmentBlockData::InteriorEnvironmentBlockData () :
	m_name (0),
	m_dayAmbientSoundTemplateName (0),
	m_nightAmbientSoundTemplateName (0),
	m_firstMusicSoundTemplateName (0),
	m_surfaceType (0),
	m_fogEnabled (false),
	m_fogDensity (0.f),
	m_fogColor (),
	m_environmentTextureName (0),
	m_roomType (0)
{
}

//===================================================================
// PUBLIC InteriorEnvironmentBlock
//===================================================================

InteriorEnvironmentBlock::InteriorEnvironmentBlock () :
	m_name (new PersistentCrcString),
	m_dayAmbientSoundTemplateName (new PersistentCrcString),
	m_nightAmbientSoundTemplateName (new PersistentCrcString),
	m_firstMusicSoundTemplateName (new PersistentCrcString),
	m_surfaceType (0),
	m_fogEnabled (false),
	m_fogDensity (0.f),
	m_fogColor (),
	m_environmentTexture (0),
	m_roomType (0)
{
}

//-------------------------------------------------------------------

InteriorEnvironmentBlock::~InteriorEnvironmentBlock ()
{
	clear ();

	delete m_name;
	delete m_dayAmbientSoundTemplateName;
	delete m_nightAmbientSoundTemplateName;
	delete m_firstMusicSoundTemplateName;
}

//-------------------------------------------------------------------

void InteriorEnvironmentBlock::setData (const InteriorEnvironmentBlockData& data)
{
	clear ();

	m_name->set (data.m_name, true);

	if (data.m_dayAmbientSoundTemplateName && *data.m_dayAmbientSoundTemplateName)
		m_dayAmbientSoundTemplateName->set (data.m_dayAmbientSoundTemplateName, true);

	if (data.m_nightAmbientSoundTemplateName && *data.m_nightAmbientSoundTemplateName)
		m_nightAmbientSoundTemplateName->set (data.m_nightAmbientSoundTemplateName, true);

	if (data.m_firstMusicSoundTemplateName && *data.m_firstMusicSoundTemplateName)
		m_firstMusicSoundTemplateName->set (data.m_firstMusicSoundTemplateName, true);

	if (data.m_surfaceType && *data.m_surfaceType)
		m_surfaceType = DuplicateString (data.m_surfaceType);

	m_fogEnabled = data.m_fogEnabled;
	m_fogDensity = data.m_fogDensity;
	m_fogColor = data.m_fogColor;

	if (data.m_environmentTextureName && *data.m_environmentTextureName)
		m_environmentTexture = TextureList::fetch (data.m_environmentTextureName);
	else
		m_environmentTexture = TextureList::fetchDefaultEnvironmentTexture ();

	m_roomType = data.m_roomType;
}

//-------------------------------------------------------------------

const CrcString* InteriorEnvironmentBlock::getName () const
{
	return m_name;
}

//-------------------------------------------------------------------

const CrcString* InteriorEnvironmentBlock::getDayAmbientSoundTemplateName () const
{
	return m_dayAmbientSoundTemplateName;
}

//-------------------------------------------------------------------

const CrcString* InteriorEnvironmentBlock::getNightAmbientSoundTemplateName () const
{
	return m_nightAmbientSoundTemplateName;
}

//-------------------------------------------------------------------

const CrcString* InteriorEnvironmentBlock::getFirstMusicSoundTemplateName () const
{
	return m_firstMusicSoundTemplateName;
}

//-------------------------------------------------------------------

const char* InteriorEnvironmentBlock::getSurfaceType () const
{
	return m_surfaceType;
}

//-------------------------------------------------------------------

bool InteriorEnvironmentBlock::getFogEnabled () const
{
	return m_fogEnabled;
}

//-------------------------------------------------------------------

float InteriorEnvironmentBlock::getFogDensity () const
{
	return m_fogDensity;
}

//-------------------------------------------------------------------

const VectorArgb& InteriorEnvironmentBlock::getFogColor () const
{
	return m_fogColor;
}

//-------------------------------------------------------------------

const Texture* InteriorEnvironmentBlock::getEnvironmentTexture () const
{
	return m_environmentTexture;
}

//-------------------------------------------------------------------

int InteriorEnvironmentBlock::getRoomType () const
{
	return m_roomType;
}

//===================================================================
// PRIVATE InteriorEnvironmentBlock
//===================================================================

void InteriorEnvironmentBlock::clear ()
{
	m_name->set ("", false);
	m_dayAmbientSoundTemplateName->set ("", false);
	m_nightAmbientSoundTemplateName->set ("", false);
	m_firstMusicSoundTemplateName->set ("", false);

	delete [] m_surfaceType;
	m_surfaceType = 0;

	if (m_environmentTexture)
	{
		m_environmentTexture->release ();
		m_environmentTexture = 0;
	}
}

//===================================================================
