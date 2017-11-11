// ======================================================================
//
// LightsaberAppearanceTemplate.cpp
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/LightsaberAppearanceTemplate.h"

#include "clientGame/LightsaberAppearance.h"
#include "clientGraphics/ShaderTemplate.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientObject/BeamAppearance.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedObject/AppearanceTemplateList.h"

#include <algorithm>

// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITHOUT_INSTALL(LightsaberAppearanceTemplate, true, 0, 0, 0);

// ======================================================================

namespace LightsaberAppearanceTemplateNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	Tag const TAG_ASND = TAG(A,S,N,D);
	Tag const TAG_BASE = TAG(B,A,S,E);
	Tag const TAG_BLAD = TAG(B,L,A,D);
	Tag const TAG_CLOS = TAG(C,L,O,S);
	Tag const TAG_COLR = TAG(C,O,L,R);
	Tag const TAG_DAYN = TAG(D,A,Y,N);
	Tag const TAG_LGHT = TAG(L,G,H,T);
	Tag const TAG_LGTH = TAG(L,G,T,H);
	Tag const TAG_LSAT = TAG(L,S,A,T);
	Tag const TAG_OPEN = TAG(O,P,E,N);
	Tag const TAG_RANG = TAG(R,A,N,G);
	Tag const TAG_SHDR = TAG(S,H,D,R);
	Tag const TAG_TIME = TAG(T,I,M,E);
	Tag const TAG_WDTH = TAG(W,D,T,H);


	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool  s_installed;

}

using namespace LightsaberAppearanceTemplateNamespace;

// ======================================================================

class LightsaberAppearanceTemplate::BladeInfo
{
public:

	BladeInfo(int bladeIndex, char const *shaderTemplateName, float length, float width, float openRate, float closeRate);

	int              getBladeIndex() const;
	CrcString const &getShaderTemplateName() const;

	float            getLength() const;
	float            getWidth() const;
	float            getOpenRate() const;
	float            getCloseRate() const;

	BeamAppearance  *createAppearance(PreloadData const *preloadData) const;
	Shader  *createShader(PreloadData const *preloadData) const;

private:

	// Disabled.
	BladeInfo();
	BladeInfo(BladeInfo const & rhs);
	BladeInfo & operator=(BladeInfo const & rhs);

private:

	int                  m_bladeIndex;
	PersistentCrcString  m_shaderTemplateName;
	float                m_length;
	float                m_width;
	float                m_openRate;
	float                m_closeRate;
};

// ======================================================================

class LightsaberAppearanceTemplate::PreloadData
{
public:

	PreloadData(CrcString const &hiltAppearanceTemplateName, BladeInfoVector const &bladeInfoVector);
	~PreloadData();

	Appearance *createHiltAppearance() const;
	Shader     *fetchNewBladeShader(int bladeIndex) const;

private:

	typedef stdvector<ShaderTemplate const*>::fwd  ShaderTemplateVector;

private:

	// Disabled.
	PreloadData();

private:

	AppearanceTemplate const *m_hiltAppearanceTemplate;
	ShaderTemplateVector      m_bladeShaderTemplates;

};

// ======================================================================
// class LightsaberAppearanceTemplate::BladeInfo
// ======================================================================

LightsaberAppearanceTemplate::BladeInfo::BladeInfo(int bladeIndex, char const *shaderTemplateName, float length, float width, float openRate, float closeRate) :
	m_bladeIndex(bladeIndex),
	m_shaderTemplateName(shaderTemplateName, true),
	m_length(length),
	m_width(width),
	m_openRate(openRate),
	m_closeRate(closeRate)
{
}

// ----------------------------------------------------------------------

inline int LightsaberAppearanceTemplate::BladeInfo::getBladeIndex() const
{
	return m_bladeIndex;
}

// ----------------------------------------------------------------------

inline CrcString const &LightsaberAppearanceTemplate::BladeInfo::getShaderTemplateName() const
{
	return m_shaderTemplateName;
}

// ----------------------------------------------------------------------

inline float LightsaberAppearanceTemplate::BladeInfo::getLength() const
{
	return m_length;
}

// ----------------------------------------------------------------------

float LightsaberAppearanceTemplate::BladeInfo::getWidth() const
{
	return m_width;
}

// ----------------------------------------------------------------------

float LightsaberAppearanceTemplate::BladeInfo::getOpenRate() const
{
	return m_openRate;
}

// ----------------------------------------------------------------------

float LightsaberAppearanceTemplate::BladeInfo::getCloseRate() const
{
	return m_closeRate;
}

// ----------------------------------------------------------------------

BeamAppearance *LightsaberAppearanceTemplate::BladeInfo::createAppearance(PreloadData const *preloadData) const
{
	//-- Fetch the appropriate shader.
	Shader *bladeShader;

	if (preloadData)
		bladeShader = preloadData->fetchNewBladeShader(m_bladeIndex);
	else
		bladeShader = ShaderTemplateList::fetchModifiableShader(m_shaderTemplateName);

	// Create beam appearance with no length.
	return new BeamAppearance(0.0f, m_width, bladeShader);
}

// ----------------------------------------------------------------------

Shader *LightsaberAppearanceTemplate::BladeInfo::createShader(PreloadData const *preloadData) const
{
	//-- Fetch the appropriate shader.
	Shader *bladeShader;

	if (preloadData)
		bladeShader = preloadData->fetchNewBladeShader(m_bladeIndex);
	else
		bladeShader = ShaderTemplateList::fetchModifiableShader(m_shaderTemplateName);

	// Create beam appearance with no length.
	return bladeShader;
}

// ======================================================================
// class LightsaberAppearanceTemplate::PreloadData
// ======================================================================

LightsaberAppearanceTemplate::PreloadData::PreloadData(CrcString const &hiltAppearanceTemplateName, BladeInfoVector const &bladeInfoVector) :
	m_hiltAppearanceTemplate(0),
	m_bladeShaderTemplates(bladeInfoVector.size())
{
	m_hiltAppearanceTemplate = AppearanceTemplateList::fetch(hiltAppearanceTemplateName.getString());

	size_t bladeCount = bladeInfoVector.size();
	for (size_t i = 0; i < bladeCount; ++i)
		m_bladeShaderTemplates[i] = ShaderTemplateList::fetch(NON_NULL(bladeInfoVector[i])->getShaderTemplateName());
}

// ----------------------------------------------------------------------

LightsaberAppearanceTemplate::PreloadData::~PreloadData()
{
	if (m_hiltAppearanceTemplate)
	{
		AppearanceTemplateList::release(m_hiltAppearanceTemplate);
		m_hiltAppearanceTemplate = 0;
	}

	ShaderTemplateVector::iterator const endIt = m_bladeShaderTemplates.end();
	for (ShaderTemplateVector::iterator it = m_bladeShaderTemplates.begin(); it != endIt; ++it)
	{
		ShaderTemplate const *shaderTemplate = *it;
		if (shaderTemplate)
		{
			shaderTemplate->release();
			*it = 0;
		}
	}
}

// ----------------------------------------------------------------------

Appearance *LightsaberAppearanceTemplate::PreloadData::createHiltAppearance() const
{
	if (m_hiltAppearanceTemplate)
		return m_hiltAppearanceTemplate->createAppearance();
	else
		return 0;
}

// ----------------------------------------------------------------------

Shader *LightsaberAppearanceTemplate::PreloadData::fetchNewBladeShader(int bladeIndex) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, bladeIndex, static_cast<int>(m_bladeShaderTemplates.size()));
	return NON_NULL(m_bladeShaderTemplates[static_cast<ShaderTemplateVector::size_type>(bladeIndex)])->fetchModifiableShader();
}

// ======================================================================
// class LightsaberAppearanceTemplate: PUBLIC STATIC
// ======================================================================

void LightsaberAppearanceTemplate::install()
{
	InstallTimer const installTimer("LightsaberAppearanceTemplate::install");

	DEBUG_FATAL(s_installed, ("LightsaberAppearanceTemplate already installed."));

	//-- Setup MBM.
	installMemoryBlockManager();

	//-- Register this type with appearance template list.
	AppearanceTemplateList::assignBinding(TAG_LSAT, create);

	//-- Put cleanup on exit chain.
	s_installed = true;
	ExitChain::add(remove, "LightsaberAppearanceTemplate");
}

// ======================================================================
// class LightsaberAppearanceTemplate: PUBLIC
// ======================================================================

Appearance *LightsaberAppearanceTemplate::createAppearance() const
{
	return new LightsaberAppearance(*this);
}

// ----------------------------------------------------------------------

void LightsaberAppearanceTemplate::preloadAssets () const
{
	if (!m_preloadData)
		m_preloadData = new PreloadData(m_hiltAppearanceTemplateName, m_bladeInfoVector);
}

// ----------------------------------------------------------------------

void LightsaberAppearanceTemplate::garbageCollect () const
{
	delete m_preloadData;
	m_preloadData = 0;
}

// ----------------------------------------------------------------------

Appearance *LightsaberAppearanceTemplate::createBaseAppearance() const
{
	if (m_preloadData)
		return m_preloadData->createHiltAppearance();
	else
		return AppearanceTemplateList::createAppearance(m_hiltAppearanceTemplateName.getString());
}

// ----------------------------------------------------------------------

CrcString const &LightsaberAppearanceTemplate::getAmbientSoundTemplateName() const
{
	return m_ambientSoundTemplateName;
}

// ----------------------------------------------------------------------

bool LightsaberAppearanceTemplate::useLightFlicker() const
{
	return m_useLightFlicker;
}

// ----------------------------------------------------------------------

PackedArgb const &LightsaberAppearanceTemplate::getFlickerSecondaryColor() const
{
	return m_flickerSecondaryColor;
}

// ----------------------------------------------------------------------

void LightsaberAppearanceTemplate::getFlickerRange(float &min, float &max) const
{
	min = m_flickerMinRange;
	max = m_flickerMaxRange;
}

// ----------------------------------------------------------------------

void LightsaberAppearanceTemplate::getFlickerTime(float &min, float &max) const
{
	min = m_flickerMinTime;
	max = m_flickerMaxTime;
}

// ----------------------------------------------------------------------

bool LightsaberAppearanceTemplate::getFlickerIsDayNightAware() const
{
	return m_flickerIsDayNightAware;
}

// ----------------------------------------------------------------------

int LightsaberAppearanceTemplate::getBladeCount() const
{
	return static_cast<int>(m_bladeInfoVector.size());
}

// ----------------------------------------------------------------------

BeamAppearance *LightsaberAppearanceTemplate::createBladeAppearance(int bladeNumber) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, bladeNumber, getBladeCount());
	BladeInfo const &bladeInfo = getBladeInfo(bladeNumber);
	return bladeInfo.createAppearance(m_preloadData);
}

// ----------------------------------------------------------------------

Shader *LightsaberAppearanceTemplate::createBladeShader(int bladeNumber) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, bladeNumber, getBladeCount());
	BladeInfo const &bladeInfo = getBladeInfo(bladeNumber);
	return bladeInfo.createShader(m_preloadData);
}

// ----------------------------------------------------------------------

float LightsaberAppearanceTemplate::getBladeLength(int bladeNumber) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, bladeNumber, getBladeCount());
	BladeInfo const &bladeInfo = getBladeInfo(bladeNumber);
	return bladeInfo.getLength();
}

// ----------------------------------------------------------------------

float LightsaberAppearanceTemplate::getBladeWidth(int bladeNumber) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, bladeNumber, getBladeCount());
	BladeInfo const &bladeInfo = getBladeInfo(bladeNumber);
	return bladeInfo.getWidth();
}

// ----------------------------------------------------------------------

float LightsaberAppearanceTemplate::getBladeOpenRate(int bladeNumber) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, bladeNumber, getBladeCount());
	BladeInfo const &bladeInfo = getBladeInfo(bladeNumber);
	return bladeInfo.getOpenRate();
}

// ----------------------------------------------------------------------

float LightsaberAppearanceTemplate::getBladeCloseRate(int bladeNumber) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, bladeNumber, getBladeCount());
	BladeInfo const &bladeInfo = getBladeInfo(bladeNumber);
	return bladeInfo.getCloseRate();
}

// ======================================================================
// class LightsaberAppearanceTemplate: PRIVATE STATIC
// ======================================================================

void LightsaberAppearanceTemplate::remove()
{
	DEBUG_FATAL(!s_installed, ("LightsaberAppearanceTemplate not installed."));
	s_installed = false;

	AppearanceTemplateList::removeBinding(TAG_LSAT);
	removeMemoryBlockManager();
}

// ----------------------------------------------------------------------

AppearanceTemplate *LightsaberAppearanceTemplate::create(const char *name, Iff *iff)
{
	NOT_NULL(iff);
	return new LightsaberAppearanceTemplate(name, *iff);
}

// ======================================================================
// class LightsaberAppearanceTemplate: PRIVATE
// ======================================================================

LightsaberAppearanceTemplate::LightsaberAppearanceTemplate(const char *name, Iff &iff) :
	AppearanceTemplate(name),
	m_hiltAppearanceTemplateName(),
	m_ambientSoundTemplateName(),
	m_useLightFlicker(false),
	m_flickerSecondaryColor(PackedArgb(255, 32, 32, 32)),
	m_flickerMinRange(1.0f),
	m_flickerMaxRange(2.0f),
	m_flickerMinTime(0.02f),
	m_flickerMaxTime(0.10f),
	m_flickerIsDayNightAware(false),
	m_bladeInfoVector(),
	m_preloadData(0)
{
	iff.enterForm(TAG_LSAT);

		Tag const version = iff.getCurrentName();
		switch (version)
		{
			case TAG_0000:
				load_0000(iff);
				break;

			default:
				{
					char tagName[5];
					ConvertTagToString(version, tagName);
					FATAL(true, ("Unsupported LightsaberAppearanceTemplate version [%s].", tagName));
				}
		}

	iff.exitForm(TAG_LSAT);
}

// ----------------------------------------------------------------------

LightsaberAppearanceTemplate::~LightsaberAppearanceTemplate()
{
	std::for_each(m_bladeInfoVector.begin(), m_bladeInfoVector.end(), PointerDeleter());
}

// ----------------------------------------------------------------------

void LightsaberAppearanceTemplate::load_0000(Iff &iff)
{
	int bladeIndex = 0;

	iff.enterForm(TAG_0000);

		while (!iff.atEndOfForm())
		{
			Tag const nextTag = iff.getCurrentName();
			switch (nextTag)
			{
				case TAG_BASE:
					{
						//-- Get appearance template name for lightsaber's hilt (base) appearance.
						iff.enterChunk(TAG_BASE);

							char pathName[MAX_PATH];
							iff.read_string(pathName, sizeof(pathName) - 1);
							pathName[sizeof(pathName) - 1] = 0;

							m_hiltAppearanceTemplateName.set(pathName, true);

						iff.exitChunk(TAG_BASE);
					}
					break;

				case TAG_ASND:
					{
						//-- Get appearance template name for lightsaber's hilt (base) appearance.
						iff.enterChunk(TAG_ASND);

							char pathName[MAX_PATH];
							iff.read_string(pathName, sizeof(pathName) - 1);
							pathName[sizeof(pathName) - 1] = 0;

							m_ambientSoundTemplateName.set(pathName, true);

						iff.exitChunk(TAG_ASND);
					}
					break;

				case TAG_LGHT:
					{
						iff.enterForm(TAG_LGHT);
						{
							m_useLightFlicker = true;

							while (!iff.atEndOfForm())
							{
								Tag const lightTag = iff.getCurrentName();
								switch (lightTag)
								{
									case TAG_COLR:
										iff.enterChunk(TAG_COLR);
											m_flickerSecondaryColor.setR(iff.read_uint8());
											m_flickerSecondaryColor.setG(iff.read_uint8());
											m_flickerSecondaryColor.setB(iff.read_uint8());
											m_flickerSecondaryColor.setA(iff.read_uint8());
										iff.exitChunk(TAG_COLR);
										break;

									case TAG_RANG:
										iff.enterChunk(TAG_RANG);
											m_flickerMinRange = iff.read_float();
											m_flickerMaxRange = iff.read_float();
										iff.exitChunk(TAG_RANG);
										break;

									case TAG_TIME:
										iff.enterChunk(TAG_TIME);
											m_flickerMinTime = iff.read_float();
											m_flickerMaxTime = iff.read_float();
										iff.exitChunk(TAG_TIME);
										break;

									case TAG_DAYN:
										iff.enterChunk(TAG_DAYN);
											m_flickerIsDayNightAware = (iff.read_uint8() != 0);
										iff.exitChunk(TAG_DAYN);
										break;

									default:
										{
											char tagName[5];
											ConvertTagToString(lightTag, tagName);
											FATAL(true, ("LightsaberAppearanceTemplate: version 0: unsupported light data tag [%s].", tagName));
										}
										break;
								}
							}
						}
						iff.exitForm(TAG_LGHT);
					}
					break;

				case TAG_BLAD:
					{
						//-- Setup defaults for blade info.
						char   shaderTemplateName[MAX_PATH];
						float  length    = 1.8f;
						float  width     = 0.15f;
						float  openRate  = 0.5f;
						float  closeRate = 0.5f;

						shaderTemplateName[0] = '\0';

						iff.enterForm(TAG_BLAD);
						{
							while (!iff.atEndOfForm())
							{
								Tag const bladeTag = iff.getCurrentName();
								switch (bladeTag)
								{
									case TAG_SHDR:
										iff.enterChunk(TAG_SHDR);

											iff.read_string(shaderTemplateName, sizeof(shaderTemplateName) - 1);
											shaderTemplateName[sizeof(shaderTemplateName) - 1] = 0;

										iff.exitChunk(TAG_SHDR);
										break;

									case TAG_LGTH:
										iff.enterChunk(TAG_LGTH);
											length = iff.read_float();
										iff.exitChunk(TAG_LGTH);
										break;

									case TAG_WDTH:
										iff.enterChunk(TAG_WDTH);
											width = iff.read_float();
										iff.exitChunk(TAG_WDTH);
										break;

									case TAG_OPEN:
										iff.enterChunk(TAG_OPEN);
											openRate = iff.read_float();
										iff.exitChunk(TAG_OPEN);
										break;

									case TAG_CLOS:
										iff.enterChunk(TAG_CLOS);
											closeRate = iff.read_float();
										iff.exitChunk(TAG_CLOS);
										break;

									default:
										{
											char tagName[5];
											ConvertTagToString(bladeTag, tagName);
											FATAL(true, ("LightsaberAppearanceTemplate: version 0: unsupported blade data tag [%s].", tagName));
										}
										break;
								}
							}
						}
						iff.exitForm(TAG_BLAD);

						//-- Create the blade info.
						m_bladeInfoVector.push_back(new BladeInfo(bladeIndex, shaderTemplateName, length, width, openRate, closeRate));
						++bladeIndex;
					}
					break;

				default:
					{
						char name[5];
						ConvertTagToString(nextTag, name);
						FATAL(true, ("LightsaberAppearanceTemplate: version 0: found unsupported data, tag=[%s].", name));
					}
					break;
			}
		}

	iff.exitForm(TAG_0000);
}

// ======================================================================

LightsaberAppearanceTemplate::BladeInfo const &LightsaberAppearanceTemplate::getBladeInfo(int bladeIndex) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, bladeIndex, static_cast<int>(m_bladeInfoVector.size()));
	return *NON_NULL(m_bladeInfoVector[static_cast<BladeInfoVector::size_type>(bladeIndex)]);
}

// ======================================================================
