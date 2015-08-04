// ============================================================================
//
// ParticleEffectAttribute.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstParticleEditor.h"
#include "ParticleEffectAttribute.h"
#include "ParticleEffectAttribute.moc"

#include "sharedMath/Range.h"

// ============================================================================
//
// ParticleEffectAttributeNamespace
//
// ============================================================================

namespace ParticleEffectAttributeNamespace
{
	Range const s_initialPlayBackRateRange(1.0f, 64.0f);
	Range const s_initialPlayBackRateTimeRange(0.0f, 8.0f);
	Range const s_playBackRateRange(0.01f, 64.0f);
	Range const s_scaleRange(0.01f, 256.0f);
}

using namespace ParticleEffectAttributeNamespace;

// ============================================================================
//
// ParticleEffectAttribute
//
// ============================================================================

//-----------------------------------------------------------------------------
ParticleEffectAttribute::ParticleEffectAttribute(QWidget * parent, char const * name)
 : BaseParticleEffectAttribute(parent, name)
{
	QDoubleValidator * const doubleValidator = new QDoubleValidator(this, "QDoubleValidator");

	m_initialPlayBackRateLineEdit->setValidator(doubleValidator);
	m_initialPlayBackRateTimeLineEdit->setValidator(doubleValidator);
	m_playBackRateLineEdit->setValidator(doubleValidator);
	m_scaleLineEdit->setValidator(doubleValidator);

	connect(m_initialPlayBackRateLineEdit, SIGNAL(returnPressed()), this, SLOT(onReturnPressed()));
	connect(m_initialPlayBackRateTimeLineEdit, SIGNAL(returnPressed()), this, SLOT(onReturnPressed()));
	connect(m_playBackRateLineEdit, SIGNAL(returnPressed()), this, SLOT(onReturnPressed()));
	connect(m_scaleLineEdit, SIGNAL(returnPressed()), this, SLOT(onReturnPressed()));
}

//-----------------------------------------------------------------------------
void ParticleEffectAttribute::onReturnPressed()
{
	validate();

	emit dataChanged();
}

//-----------------------------------------------------------------------------
void ParticleEffectAttribute::setInitialPlayBackRate(float const initialPlayBackRate)
{
	ParticleEditorUtility::setLineEditFloat(m_initialPlayBackRateLineEdit, initialPlayBackRate, s_initialPlayBackRateRange);
	validate();
}

//-----------------------------------------------------------------------------
float ParticleEffectAttribute::getInitialPlayBackRate() const
{
	return ParticleEditorUtility::getFloat(m_initialPlayBackRateLineEdit);
}

//-----------------------------------------------------------------------------
void ParticleEffectAttribute::setInitialPlayBackRateTime(float const initialPlayBackRateTime)
{
	ParticleEditorUtility::setLineEditFloat(m_initialPlayBackRateTimeLineEdit, initialPlayBackRateTime, s_initialPlayBackRateTimeRange);
	validate();
}

//-----------------------------------------------------------------------------
float ParticleEffectAttribute::getInitialPlayBackRateTime() const
{
	return ParticleEditorUtility::getFloat(m_initialPlayBackRateTimeLineEdit);
}

//-----------------------------------------------------------------------------
void ParticleEffectAttribute::setPlayBackRate(float const playBackRate)
{
	ParticleEditorUtility::setLineEditFloat(m_playBackRateLineEdit, playBackRate, s_playBackRateRange);
	validate();
}

//-----------------------------------------------------------------------------
float ParticleEffectAttribute::getPlayBackRate() const
{
	return ParticleEditorUtility::getFloat(m_playBackRateLineEdit);
}

//-----------------------------------------------------------------------------
void ParticleEffectAttribute::setScale(float const scale)
{
	ParticleEditorUtility::setLineEditFloat(m_scaleLineEdit, scale, s_scaleRange);
	validate();
}

//-----------------------------------------------------------------------------
float ParticleEffectAttribute::getScale() const
{
	return ParticleEditorUtility::getFloat(m_scaleLineEdit);
}

//-----------------------------------------------------------------------------
void ParticleEffectAttribute::validate()
{
	ParticleEditorUtility::validateLineEditFloat(m_initialPlayBackRateLineEdit, s_initialPlayBackRateRange);
	ParticleEditorUtility::validateLineEditFloat(m_initialPlayBackRateTimeLineEdit, s_initialPlayBackRateTimeRange);
	ParticleEditorUtility::validateLineEditFloat(m_playBackRateLineEdit, s_playBackRateRange);
	ParticleEditorUtility::validateLineEditFloat(m_scaleLineEdit, s_scaleRange);

	if (getInitialPlayBackRateTime() <= 0.0f)
	{
		m_initialPlayBackRateLineEdit->setEnabled(false);
		ParticleEditorUtility::setLineEditFloat(m_initialPlayBackRateLineEdit, 1.0f, s_initialPlayBackRateRange);
	}
	else
	{
		m_initialPlayBackRateLineEdit->setEnabled(true);
	}
}

// ============================================================================
