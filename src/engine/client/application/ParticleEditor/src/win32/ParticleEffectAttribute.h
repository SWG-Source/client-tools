// ============================================================================
//
// ParticleEffectAttribute.h
// copyright 2004 Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_ParticleEffectAttribute_H
#define INCLUDED_ParticleEffectAttribute_H

#include "BaseParticleEffectAttribute.h"

//-----------------------------------------------------------------------------
class ParticleEffectAttribute : public BaseParticleEffectAttribute
{
	Q_OBJECT

public:

	ParticleEffectAttribute(QWidget * parent, char const * name);

	void setInitialPlayBackRate(float const initialPlayBackRate);
	float getInitialPlayBackRate() const;

	void setInitialPlayBackRateTime(float const initialPlayBackRateTime);
	float getInitialPlayBackRateTime() const;

	void setPlayBackRate(float const playBackRate);
	float getPlayBackRate() const;

	void setScale(float const scale);
	float getScale() const;

signals:

	void dataChanged();

public slots:

	void onReturnPressed();

private:

	void validate();
};

// ============================================================================

#endif // INCLUDED_ParticleEffectAttribute_H
