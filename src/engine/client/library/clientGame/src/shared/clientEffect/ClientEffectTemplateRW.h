// ======================================================================
//
// ClientEffectTemplateRW.h
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================


#ifndef INCLUDED_ClientEffectTemplateRW_H
#define INCLUDED_ClientEffectTemplateRW_H

// ======================================================================

#include "clientGame/ClientEffectTemplate.h"

// ======================================================================

class ClientEffectTemplateRW : public ClientEffectTemplate
{
public:
	
	ClientEffectTemplateRW();
	virtual ~ClientEffectTemplateRW();

	void save(Iff& iff);
	void clear();

	void addAppearance (const std::string& appearanceTemplateName, float time, float minScale, float maxScale, float minPlaybackRate, float maxPlaybackRate, bool softTerminate = false);
	void addSound (const std::string& soundTemplateName);
	void addLight (const CreateLightFunc& light);
	void addCameraShake (const CameraShakeFunc& cameraShake);
	void addForceFeedbackEffect (const ForceFeedbackFunc& forceFeedbackEffect);

	const CreateAppearanceList& getParticleSystems() const;
	const PlaySoundList&        getSounds() const;
	const CreateLightList&      getLights() const; 
	const CameraShakeList&      getCameraShakes() const;
	const ForceFeedbackList&    getForceFeedbackEffects() const;

private:
	//disabled
	ClientEffectTemplateRW(const ClientEffectTemplateRW&);
	ClientEffectTemplateRW& operator=(const ClientEffectTemplateRW&);
};

// ======================================================================

#endif
