// ============================================================================
//
// ParticleAttachmentDescription.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_ParticleAttachmentDescription_H
#define INCLUDED_ParticleAttachmentDescription_H

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Tag.h"
#include <string>

class AppearanceTemplate;
class Iff;

//-----------------------------------------------------------------------------
class ParticleAttachmentDescription
{
public:

	enum Spawn
	{
		S_whenParentIsCreated,
		S_whenParentDies,
		S_atPercent,
		S_onCollision
	};

public:

	ParticleAttachmentDescription();
	ParticleAttachmentDescription(ParticleAttachmentDescription const &rhs);
	~ParticleAttachmentDescription();
	ParticleAttachmentDescription &operator =(ParticleAttachmentDescription const &rhs);

	static Tag getTag();

	virtual void initializeDefault();
	virtual bool load(Iff &iff);
	virtual void write(Iff &iff) const;

	void setAttachmentPath(std::string const &path);
	void setStartPercent(float const min, float const max);
	void setKillAttachmentWhenParticleDies(bool const killAttachmentWhenParticleDies);
	void setSpawn(Spawn const spawn);

	std::string const &getAttachmentPath() const;
	float              getStartPercentMin() const;
	float              getStartPercentMax() const;
	Spawn              getSpawn() const;
	bool               isKillAttachmentWhenParticleDies() const;
	bool               isInfiniteLooping() const;

private:

	static Tag const          m_tag;
	AppearanceTemplate const *m_cachedAppearanceTemplate;
	std::string               m_attachmentPath;
	float                     m_startPercentMin;
	float                     m_startPercentMax;
	bool                      m_killAttachmentWhenParticleDies;
	Spawn                     m_spawn;

	void load_0000(Iff &iff);
	void load_0001(Iff &iff);
};

// ============================================================================

#endif // ParticleAttachmentDescription_H
