// ============================================================================
//
// SwooshAppearanceTemplate.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_SwooshAppearanceTemplate_H
#define INCLUDED_SwooshAppearanceTemplate_H

#include "clientParticle/ColorRamp.h"
#include "clientParticle/ParticleTexture.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/PersistentCrcString.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Tag.h"
#include "sharedMath/VectorArgb.h"
#include "sharedMath/WaveForm.h"
#include "sharedObject/AppearanceTemplate.h"

class Appearance;
class Iff;

//-----------------------------------------------------------------------------
class SwooshAppearanceTemplate : public AppearanceTemplate
{
public:

	enum FadeAlpha
	{
		FA_none,
		FA_0,
		FA_1,
		FA_2,
		FA_3,
		FA_4,
		FA_5,
		FA_6,
		FA_7,
		FA_8,
		FA_9,
		FA_count
	};

	enum TaperGeometry
	{
		TG_none,
		TG_0,
		TG_1,
		TG_2,
		TG_3,
		TG_4,
		TG_5,
		TG_6,
		TG_7,
		TG_8,
		TG_9,
		TG_count
	};

	enum AppearancePosition
	{
		AP_top,
		AP_middle,
		AP_bottom,
		AP_random
	};

public:

	SwooshAppearanceTemplate();
	explicit SwooshAppearanceTemplate(const char *name, Iff *iff = NULL);
	virtual ~SwooshAppearanceTemplate();

	static void                            setDefaultTemplate(SwooshAppearanceTemplate const &swooshAppearanceTemplate);
	static SwooshAppearanceTemplate const *getDefaultTemplate();

#ifdef _DEBUG
	// This is used by the swoosh editor
	static bool                            useFinalGameSwooshes();
	static void                            setUseFinalGameSwooshes(bool const enabled);
#endif // _DEBUG

	virtual Appearance *       createAppearance() const;
	static AppearanceTemplate *create(const char *name, Iff *iff);
	static void                install();
	static void                remove();
	static Tag                 getTag();

	void                       load(Iff &iff);
	void                       write(Iff &iff) const;

	ParticleTexture const &    getParticleTexture() const;

	void                       setColor(float const red, float const green, float const blue);
	VectorArgb const &         getColor() const;

	void                       setAlpha(float const alpha);
	float                      getAlpha() const;

	void                       setWidth(float const width);
	float                      getWidth() const;

	void                       setShaderStretchDistance(float const distance);
	float                      getShaderStretchDistance() const;

	void                       setShaderScrollSpeed(float const scrollSpeed);
	float                      getShaderScrollSpeed() const;

	void                       setShader(char const *path, int const frameCount, float const framesPerSecond, int const startFrame, int const endFrame);

	void                       setStartAppearancePath(std::string const &path);
	std::string const &        getStartAppearancePath() const;

	void                       setStartAppearancePosition(AppearancePosition const appearancePosition);
	AppearancePosition         getStartAppearancePosition() const;

	void                       setEndAppearancePath(std::string const &path);
	std::string const &        getEndAppearancePath() const;

	void                       setEndAppearancePosition(AppearancePosition const appearancePosition);
	AppearancePosition         getEndAppearancePosition() const;

	void                       setSoundPath(std::string const &path);
	std::string const &        getSoundPath() const;

	void                       setFadeAlpha(FadeAlpha const fadeAlpha);
	FadeAlpha                  getFadeAlpha() const;

	void                       setTaperGeometry(TaperGeometry const taperGeometry);
	TaperGeometry              getTaperGeometry() const;

	void                       setMultiplyColorByAlpha(bool const enabled);
	bool                       isMultiplyColorByAlpha() const;

	void setSamplesPerSecond(float const samplesPerSecond);
	float getSamplesPerSecond() const;
	static float getMinSamplesPerSecond();
	static float getMaxSamplesPerSecond();

	void setSamplePositionCount(unsigned int const samplePositionCount);
	unsigned int getSamplePositionCount() const;

	void setSplineSubQuads(int const splineSubQuads);
	int getSplineSubQuads() const;

	static void                setFullVisibilitySpeed(float const speed);
	static float               getFullVisibilitySpeed();

	static void                setNoVisibilitySpeed(float const speed);
	static float               getNoVisibilitySpeed();

private:

	void load_0000(Iff &iff);
	void load_0001(Iff &iff);

	ParticleTexture    m_particleTexture;
	float              m_alpha;
	VectorArgb         m_color;
	float              m_width;
	float              m_shaderStretchDistance;
	float              m_shaderScrollSpeed;
	FadeAlpha          m_fadeAlpha;
	TaperGeometry      m_taperGeometry;
	bool               m_multiplyColorByAlpha;
	std::string        m_startAppearancePath;
	std::string        m_endAppearancePath;
	std::string        m_soundPath;
	AppearancePosition m_startAppearancePosition;
	AppearancePosition m_endAppearancePosition;
	float m_samplesPerSecond;
	unsigned int m_samplePositionCount;
	int m_splineSubQuads;

private:

	// Disabled

	SwooshAppearanceTemplate(const SwooshAppearanceTemplate &);
	SwooshAppearanceTemplate &operator =(SwooshAppearanceTemplate const &);
};

//=============================================================================

#endif // INCLUDED_SwooshAppearanceTemplate_H
