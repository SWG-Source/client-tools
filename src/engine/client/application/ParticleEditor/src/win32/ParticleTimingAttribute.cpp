// ============================================================================
//
// ParticleTimingAttribute.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstParticleEditor.h"
#include "ParticleTimingAttribute.h"
#include "ParticleTimingAttribute.moc"

#include "clientParticle/ParticleTiming.h"

// ============================================================================
//
// ParticleTimingAttribute
//
// ============================================================================

ParticleBoundFloat const ParticleTimingAttribute::m_startDelayBound(4096.0f, 0.0f);
ParticleBoundFloat const ParticleTimingAttribute::m_startDelayRandomBound(4096.0f, 0.0f);
ParticleBoundFloat const ParticleTimingAttribute::m_loopDelayBound(4096.0f, 0.0f);
ParticleBoundFloat const ParticleTimingAttribute::m_loopDelayRandomBound(4096.0f, 0.0f);
ParticleBoundInt const ParticleTimingAttribute::m_loopCountBound(4096, 0);
ParticleBoundInt const ParticleTimingAttribute::m_loopCountRandomBound(4096, 0);

//-----------------------------------------------------------------------------
ParticleTimingAttribute::ParticleTimingAttribute(QWidget *parent, char const *name)
 : BaseParticleTimingAttribute(parent, name)
{
	QDoubleValidator *doubleValidator = new QDoubleValidator(this, "QDoubleValidator");
	NOT_NULL(doubleValidator);

	m_startDelayLineEdit->setValidator(doubleValidator);
	m_startDelayRandomLineEdit->setValidator(doubleValidator);
	m_loopDelayLineEdit->setValidator(doubleValidator);
	m_loopDelayRandomLineEdit->setValidator(doubleValidator);
	m_loopCountLineEdit->setValidator(doubleValidator);
	m_loopCountRandomLineEdit->setValidator(doubleValidator);

	connect(m_startDelayLineEdit, SIGNAL(returnPressed()), this, SLOT(onReturnPressed()));
	connect(m_startDelayRandomLineEdit, SIGNAL(returnPressed()), this, SLOT(onReturnPressed()));
	connect(m_loopDelayLineEdit, SIGNAL(returnPressed()), this, SLOT(onReturnPressed()));
	connect(m_loopDelayRandomLineEdit, SIGNAL(returnPressed()), this, SLOT(onReturnPressed()));
	connect(m_loopCountLineEdit, SIGNAL(returnPressed()), this, SLOT(onReturnPressed()));
	connect(m_loopCountRandomLineEdit, SIGNAL(returnPressed()), this, SLOT(onReturnPressed()));
	connect(m_loopInfiniteCheckBox, SIGNAL(clicked()), this, SLOT(onLoopInfiniteCheckBoxClicked()));
}

//-----------------------------------------------------------------------------
void ParticleTimingAttribute::onReturnPressed()
{
	ParticleEditorUtility::validateLineEditFloat(m_startDelayLineEdit, m_startDelayBound, 2);
	ParticleEditorUtility::validateLineEditFloat(m_startDelayRandomLineEdit, m_startDelayRandomBound, 2);
	ParticleEditorUtility::validateLineEditFloat(m_loopDelayLineEdit, m_loopDelayBound, 2);
	ParticleEditorUtility::validateLineEditFloat(m_loopDelayRandomLineEdit, m_loopDelayRandomBound, 2);

	if (!m_loopInfiniteCheckBox->isChecked())
	{
		ParticleEditorUtility::validateLineEditInt(m_loopCountLineEdit, m_loopCountBound);
		ParticleEditorUtility::validateLineEditInt(m_loopCountRandomLineEdit, m_loopCountRandomBound);
	}

	emit timingChanged();
}

//-----------------------------------------------------------------------------
void ParticleTimingAttribute::onLoopInfiniteCheckBoxClicked()
{
	if (m_loopInfiniteCheckBox->isChecked())
	{
		m_loopCountLineEdit->setText("");
		m_loopCountLineEdit->setEnabled(false);
		m_loopCountRandomLineEdit->setText("");
		m_loopCountRandomLineEdit->setEnabled(false);
	}
	else
	{
		m_loopCountLineEdit->setEnabled(true);
		m_loopCountRandomLineEdit->setEnabled(true);
		m_loopCountLineEdit->setText("0");
		m_loopCountRandomLineEdit->setText("0");

		onReturnPressed();
	}

	emit timingChanged();
}

//-----------------------------------------------------------------------------
ParticleTiming ParticleTimingAttribute::getTiming() const
{
	ParticleTiming timing;

	float const startDelayMin = ParticleEditorUtility::getFloat(m_startDelayLineEdit);
	float const startDelayRandom = ParticleEditorUtility::getFloat(m_startDelayRandomLineEdit);

	timing.setStartDelay(startDelayMin, startDelayMin + startDelayRandom);

	float const loopDelayMin = ParticleEditorUtility::getFloat(m_loopDelayLineEdit);
	float const loopDelayRandom = ParticleEditorUtility::getFloat(m_loopDelayRandomLineEdit);

	timing.setLoopDelay(loopDelayMin, loopDelayMin + loopDelayRandom);
	
	if (m_loopInfiniteCheckBox->isChecked())
	{
		timing.setLoopCount(-1, -1);
	}
	else
	{
		int const loopCountMin = ParticleEditorUtility::getInt(m_loopCountLineEdit);
		int const loopCountRandom = ParticleEditorUtility::getInt(m_loopCountRandomLineEdit);

		timing.setLoopCount(loopCountMin, loopCountMin + loopCountRandom);
	}

	return timing;
}

//-----------------------------------------------------------------------------
void ParticleTimingAttribute::setTiming(ParticleTiming const &timing) const
{
	ParticleEditorUtility::setLineEditFloat(m_startDelayLineEdit, timing.getStartDelayMin(), m_startDelayBound, 2);
	ParticleEditorUtility::setLineEditFloat(m_startDelayRandomLineEdit, timing.getStartDelayMax() - timing.getStartDelayMin(), m_startDelayRandomBound, 2);
	ParticleEditorUtility::setLineEditFloat(m_loopDelayLineEdit, timing.getLoopDelayMin(), m_loopDelayBound, 2);
	ParticleEditorUtility::setLineEditFloat(m_loopDelayRandomLineEdit, timing.getLoopDelayMax() - timing.getLoopDelayMin(), m_loopDelayRandomBound, 2);

	// See if the timing is infinite looping

	if (timing.getLoopCountMin() < 0)
	{
		m_loopInfiniteCheckBox->setChecked(true);
		m_loopCountLineEdit->setText("");
		m_loopCountLineEdit->setEnabled(false);
		m_loopCountRandomLineEdit->setText("");
		m_loopCountRandomLineEdit->setEnabled(false);
	}
	else
	{
		m_loopInfiniteCheckBox->setChecked(false);
		m_loopCountLineEdit->setEnabled(true);
		m_loopCountRandomLineEdit->setEnabled(true);
		ParticleEditorUtility::setLineEditInt(m_loopCountLineEdit, timing.getLoopCountMin(), m_loopCountBound);
		ParticleEditorUtility::setLineEditInt(m_loopCountRandomLineEdit, timing.getLoopCountMax() - timing.getLoopCountMin(), m_loopCountRandomBound);
	}
}

// ============================================================================