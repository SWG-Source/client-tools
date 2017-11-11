// ============================================================================
//
// ForceFeedbackEffectTemplate.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef ForceFeedbackEffectTemplate_H
#define ForceFeedbackEffectTemplate_H

//-----------------------------------------------------------------------------

#include "clientDirectInput/DirectInput.h"

#include <vector>

struct IDirectInputEffect;

//-----------------------------------------------------------------------------

class ForceFeedbackEffectTemplate
{
friend class ForceFeedbackEffectTemplateList;

public:
	static void update();
	static void deleteAllQueuedDeletes(bool deletePlayingEffects = true);

public:
	void fetch() const;
	int getReferenceCount() const;
	bool playEffect(int iterations) const;
	bool stopEffect() const;
	bool isPlaying() const;
	uint32 getCrc() const;
	void addEffect(DirectInput::ForceFeedbackEffect * effect);

private:
	void release() const;
	ForceFeedbackEffectTemplate(std::string const & name, bool fireAndForget);
	virtual ~ForceFeedbackEffectTemplate();

private:
	mutable int m_referenceCount;
	uint32 m_crc;
	std::vector<DirectInput::ForceFeedbackEffect *> m_effects;
	bool m_fireAndForget;

private:
	//disabled
	ForceFeedbackEffectTemplate(ForceFeedbackEffectTemplate const &);
	ForceFeedbackEffectTemplate &operator =(ForceFeedbackEffectTemplate const &);
};

//=============================================================================

#endif // ForceFeedbackEffectTemplate_H
