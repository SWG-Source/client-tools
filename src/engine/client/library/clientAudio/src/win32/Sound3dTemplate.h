// ============================================================================
//
// Sound3dTemplate.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef SOUND3DTEMPLATE_H
#define SOUND3DTEMPLATE_H

#include "clientAudio/Sound2dTemplate.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Tag.h"

class Iff;

//-----------------------------------------------------------------------------
class Sound3dTemplate : public Sound2dTemplate
{
public:

	Sound3dTemplate();
	explicit Sound3dTemplate(char const *name);
	virtual ~Sound3dTemplate();

	virtual Sound2 *      createSound() const;
	static SoundTemplate *create(char const *name, Iff &iff);
	static void           install();
	static void           remove();
	static Tag const &    getTag();

	virtual void          write(Iff &iff, bool const moveToTopOfForm = true) const;
	virtual void          load(Iff &iff);

	virtual bool          is3d() const;

private:

	static bool      m_installed;
	static Tag const m_tag;

	void load_0000(Iff &iff);
	void load_0001(Iff &iff);

private:

	// Disabled

	Sound3dTemplate(const Sound3dTemplate &);
	Sound3dTemplate &operator =(Sound3dTemplate const &);
};

//=============================================================================

#endif // SOUND3DTEMPLATE_H
