// ============================================================================
//
// ParticleTexture.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_ParticleTexture_H
#define INCLUDED_ParticleTexture_H

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/PersistentCrcString.h"

class Iff;
class ShaderTemplate;

// Particle texture are assumed to be a square and a power of 2.
//-----------------------------------------------------------------------------
class ParticleTexture
{
public:

	ParticleTexture();
	ParticleTexture(ParticleTexture const &rhs);
	ParticleTexture(char const *shaderPath, int const frameCount = 1, int const startFrame = 0, int const endFrame = 0, float const framesPerSecond = -1.0f, bool textureVisible = true);
	~ParticleTexture();
	ParticleTexture &operator =(ParticleTexture const &rhs);

	void                         setShaderPath(char const *path);
	PersistentCrcString const   &getShaderPath() const;

	int                getFrameCount() const;
	int                getFrameStart() const;
	int                getFrameEnd() const;
	float              getFramesPerSecond() const;
	void               getUVs(float const particleAgePercent, float const age, float &au, float &av, float &bu, float &bv, float &cu, float &cv, float &du, float &dv) const;
	void               getFrameUVs(int const frame, float &au, float &av, float &bu, float &bv, float &cu, float &cv, float &du, float &dv) const;
	bool               isTextureVisible() const;

	void               setStartFrame(int const startFrame);
	void               setEndFrame(int const endFrame);
	void               setFrameCount(int const frameCount);
	void               setFramesPerSecond(float const framesPerSecond);

	void write(Iff &iff) const;
	void load(Iff &iff);

private:

	void setShaderTemplateName(const std::string &shaderTemplateName);
	void load_0000(Iff &iff);

	ShaderTemplate const *m_cachedShaderTemplate;
	PersistentCrcString   m_shaderPath;
	int                   m_frameCount;
	int                   m_frameStart;
	int                   m_frameEnd;
 	float                 m_frameUVSize;
	int                   m_framesPerColumn;
	float                 m_framesPerSecond;
	bool                  m_textureVisible;
};

//=============================================================================

#endif // INCLUDED_ParticleTexture_H
