// ============================================================================
//
// Sound3dTemplate.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "clientAudio/FirstClientAudio.h"
#include "clientAudio/Sound3dTemplate.h"

#include "clientAudio/Sound3d.h"
#include "clientAudio/SoundTemplateList.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/Tag.h"

#include <vector>

// ============================================================================
//
// Sound3dTemplate
//
// ============================================================================

bool Sound3dTemplate::m_installed = false;
Tag const Sound3dTemplate::m_tag = TAG(S,D,3,D);

//-----------------------------------------------------------------------------
Sound3dTemplate::Sound3dTemplate()
 : Sound2dTemplate()
{
}

//-----------------------------------------------------------------------------
Sound3dTemplate::Sound3dTemplate(const char *name)
 : Sound2dTemplate(name)
{
}

//-----------------------------------------------------------------------------
Sound3dTemplate::~Sound3dTemplate()
{
}

//-----------------------------------------------------------------------------
Tag const &Sound3dTemplate::getTag()
{
	return m_tag;
}

//-----------------------------------------------------------------------------
Sound2 *Sound3dTemplate::createSound() const
{
	return new Sound3d(this, SoundId(0, getName()));
}

//-----------------------------------------------------------------------------
SoundTemplate *Sound3dTemplate::create(char const *name, Iff& iff)
{
	SoundTemplate *soundTemplate = new Sound3dTemplate(name);
	soundTemplate->load(iff);

	return soundTemplate;
}

//-----------------------------------------------------------------------------
void Sound3dTemplate::install()
{
	DEBUG_FATAL(m_installed, ("Install has already been called."));
	m_installed = true;

	SoundTemplateList::assignBinding(m_tag, create);
}

//-----------------------------------------------------------------------------
void Sound3dTemplate::remove()
{
	SoundTemplateList::removeBinding(m_tag);
}

//--------------------------------------------------------------------------
bool Sound3dTemplate::is3d() const
{
	return true;
}

//-----------------------------------------------------------------------------
void Sound3dTemplate::load(Iff &iff)
{
	iff.enterForm(m_tag);
	
	// Load the base class stuff

	bool const forceCacheSample = true;

	Sound2dTemplate::load(iff, forceCacheSample);

	// Load the local stuff

	switch (iff.getCurrentName())
	{
		case TAG_0000:
			{
				load_0000(iff);
				break;
			}
		case TAG_0001:
			{
				load_0001(iff);
				break;
			}
		default:
			{
				char currentTagName[256];
				ConvertTagToString(iff.getCurrentName(), currentTagName);
	
				FATAL(true, ("Unsupported data version: %s", currentTagName));
			}
	}
	
	iff.exitForm(m_tag);

	// Set it to a 3d sound

	m_attenuationMethod = Audio::AM_3d;

#ifdef _DEBUG
	if (DataLint::isEnabled())
	{
		if (strstr(getName(), "player_music") != NULL)
		{
			DEBUG_WARNING(true, ("The sound template is for player music and should be 2D attenuated."));
		}

		// Check all the file sizes

		StringList const &stringList = getSampleList();

		StringList::const_iterator iterStringList = stringList.begin();

		for (; iterStringList != stringList.end(); ++iterStringList)
		{
			CrcString const *path = (*iterStringList);
			NOT_NULL(path);

			int const bytes = Audio::getSampleSize(path->getString());

			if (bytes > 512 * 1024)
			{
				DEBUG_WARNING(true, ("Large 3d sound file: [%dk] %s", bytes / 1024, path->getString()));
			}
		}
	}
#endif // _DEBUG
}

//-----------------------------------------------------------------------------
void Sound3dTemplate::load_0000(Iff &iff)
{
	iff.enterChunk(TAG_0000);
	{
		m_distanceAtMaxVolume = iff.read_float();

		float const dummy = iff.read_float();
		UNREF(dummy);
	}
	iff.exitChunk();
}

//-----------------------------------------------------------------------------
void Sound3dTemplate::load_0001(Iff &iff)
{
	iff.enterChunk(TAG_0001);
	{
	}
	iff.exitChunk();
}

//--------------------------------------------------------------------------
void Sound3dTemplate::write(Iff &iff, bool const moveToTopOfForm) const
{
	iff.insertForm(m_tag);
	{
		// Write the sound 2d template

		Sound2dTemplate::write(iff, false);

		// Write the local data

		iff.insertChunk(TAG_0001);
		{
		}
		iff.exitChunk();
	}
	iff.exitForm(m_tag);

	if (moveToTopOfForm)
	{
		iff.allowNonlinearFunctions();
		IGNORE_RETURN(iff.seek(m_tag));
		iff.goToTopOfForm();
	}
}

// ============================================================================
