// ============================================================================
//
// ParticleTimingAttribute.h
// copyright 2001 Sony Online Entertainment
//
// ============================================================================

#ifndef PARTICLETIMINGATTRIBUTE_H
#define PARTICLETIMINGATTRIBUTE_H

#include "BaseParticleTimingAttribute.h"
#include "ParticleEditorUtility.h"

class ParticleTiming;

//-----------------------------------------------------------------------------
class ParticleTimingAttribute : public BaseParticleTimingAttribute
{
	Q_OBJECT

public:

	ParticleTimingAttribute(QWidget *parent, char const *name);

	ParticleTiming getTiming() const;
	void           setTiming(ParticleTiming const &timing) const;

signals:

	void timingChanged();

public slots:

	void onReturnPressed();
	void onLoopInfiniteCheckBoxClicked();

private:

	static ParticleBoundFloat const m_startDelayBound;
	static ParticleBoundFloat const m_startDelayRandomBound;
	static ParticleBoundFloat const m_loopDelayBound;
	static ParticleBoundFloat const m_loopDelayRandomBound;
	static ParticleBoundInt const   m_loopCountBound;
	static ParticleBoundInt const   m_loopCountRandomBound;
};

// ============================================================================

#endif // PARTICLETIMINGATTRIBUTE_H