// ============================================================================
//
// LightningAppearanceTemplate.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_LightningAppearanceTemplate_H
#define INCLUDED_LightningAppearanceTemplate_H

#include "clientParticle/ColorRamp.h"
#include "clientParticle/ParticleTexture.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Tag.h"
#include "sharedMath/VectorArgb.h"
#include "sharedMath/WaveForm.h"
#include "sharedObject/AppearanceTemplate.h"

class Appearance;
class Iff;

//-----------------------------------------------------------------------------
class LightningAppearanceTemplate : public AppearanceTemplate
{
public:

	enum AppearanceCount
	{
		AC_single,
		AC_onePerBolt
	};

public:

	LightningAppearanceTemplate();
	explicit LightningAppearanceTemplate(const char *name, Iff *iff = NULL);
	virtual ~LightningAppearanceTemplate();

	virtual Appearance *       createAppearance() const;
	static AppearanceTemplate *create(const char *name, Iff *iff);
	static void                install();
	static void                remove();
	static Tag                 getTag();

	void                       load(Iff &iff);
	void                       write(Iff &iff) const;

	ParticleTexture const &    getParticleTexture() const;

	void                       setLargeBoltPercent(float const percent);
	float                      getLargeBoltPercent() const;

	void                       setLargeBoltColor(float const red, float const green, float const blue);
	VectorArgb const &         getLargeBoltColor() const;

	void                       setLargeBoltThickness(float const thickness);
	float                      getLargeBoltThickness() const;

	void                       setLargeBoltAlpha(float const alpha);
	float                      getLargeBoltAlpha() const;

	void                       setLargeBoltChaos(float const chaos);
	float                      getLargeBoltChaos() const;

	void                       setLargeBoltAmplitude(WaveForm const &amplitude);
	WaveForm const &           getLargeBoltAmplitude() const;
	float                      getLargeBoltBaseAmplitude() const;
	float                      getLargeBoltStartAmplitude() const;
	float                      getLargeBoltStartUntil() const;
	float                      getLargeBoltEndFrom() const;
	float                      getLargeBoltEndAmplitude() const;

	void                       setLargeBoltArc(float const arc);
	float                      getLargeBoltArc() const;

	void                       setLargeBoltShaderScrollSpeed(float const scrollSpeed);
	float                      getLargeBoltShaderScrollSpeed() const;

	void                       setLargeBoltShaderStretchDistance(float const distance);
	float                      getLargeBoltShaderStretchDistance() const;

	void                       setSmallBoltColor(float const red, float const green, float const blue);
	VectorArgb const &         getSmallBoltColor() const;

	void                       setSmallBoltThickness(float const thickness);
	float                      getSmallBoltThickness() const;

	void                       setSmallBoltAlpha(float const alpha);
	float                      getSmallBoltAlpha() const;

	void                       setSmallBoltChaos(float const chaos);
	float                      getSmallBoltChaos() const;

	void                       setSmallBoltAmplitude(WaveForm const &amplitude);
	WaveForm const &           getSmallBoltAmplitude() const;
	float                      getSmallBoltBaseAmplitude() const;
	float                      getSmallBoltStartAmplitude() const;
	float                      getSmallBoltStartUntil() const;
	float                      getSmallBoltEndFrom() const;
	float                      getSmallBoltEndAmplitude() const;

	void                       setSmallBoltArc(float const arc);
	float                      getSmallBoltArc() const;

	void                       setSmallBoltShaderScrollSpeed(float const scrollSpeed);
	float                      getSmallBoltShaderScrollSpeed() const;

	void                       setSmallBoltShaderStretchDistance(float const distance);
	float                      getSmallBoltShaderStretchDistance() const;

	void                       setShader(char const *path, int const frameCount, float const framesPerSecond, int const startFrame, int const endFrame);

	void                       setStartAppearancePath(std::string const &startAppearancePath);
	std::string const &        getStartAppearancePath() const;
	void                       setStartAppearanceCount(AppearanceCount const appearanceCount);
	AppearanceCount            getStartAppearanceCount() const;

	void                       setEndAppearancePath(std::string const &endAppearancePath);
	std::string const &        getEndAppearancePath() const;
	void                       setEndAppearanceCount(AppearanceCount const appearanceCount);
	AppearanceCount            getEndAppearanceCount() const;

	void                       setSoundPath(std::string const &soundPath);
	std::string const &        getSoundPath() const;

	void                       setTaperStart(bool const taper);
	bool                       isStartTapered() const;

private:

	void load_0000(Iff &iff);
	void load_0001(Iff &iff);
	void load_0002(Iff &iff);

	ParticleTexture m_particleTexture;
	float           m_largeBoltPercent;
	float           m_largeBoltThickness;
	float           m_largeBoltAlpha;
	VectorArgb      m_largeBoltColor;
	float           m_largeBoltChaos;
	WaveForm        m_largeBoltAmplitude;
	float           m_largeBoltArc;
	float           m_largeBoltShaderStretchDistance;
	float           m_largeBoltShaderScrollSpeed;
	float           m_smallBoltThickness;
	float           m_smallBoltAlpha;
	VectorArgb      m_smallBoltColor;
	float           m_smallBoltChaos;
	WaveForm        m_smallBoltAmplitude;
	float           m_smallBoltArc;
	float           m_smallBoltShaderStretchDistance;
	float           m_smallBoltShaderScrollSpeed;
	std::string     m_startAppearancePath;
	std::string     m_endAppearancePath;
	std::string     m_soundPath;
	AppearanceCount m_startAppearanceCount;
	AppearanceCount m_endAppearanceCount;

private:

	// Disabled

	LightningAppearanceTemplate(const LightningAppearanceTemplate &);
	LightningAppearanceTemplate &operator =(const LightningAppearanceTemplate &);
};

//=============================================================================

#endif // INCLUDED_LightningAppearanceTemplate_H
