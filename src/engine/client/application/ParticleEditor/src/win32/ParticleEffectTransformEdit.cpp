// ============================================================================
//
// ParticleEffectTransformEdit.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstParticleEditor.h"
#include "ParticleEffectTransformEdit.h"
#include "ParticleEffectTransformEdit.moc"

#include "clientGame/Game.h"
#include "clientParticle/ParticleEffectAppearance.h"
#include "MainWindow.h"
#include "ParticleEditorUtility.h"
#include "sharedMath/Transform.h"
#include "sharedTerrain/TerrainObject.h"

// ============================================================================
//
// ParticleEffectTransformEdit
//
// ============================================================================

ParticleBoundFloat const ParticleEffectTransformEdit::m_boundTranslation(32000.0f);
ParticleBoundFloat const ParticleEffectTransformEdit::m_boundRotation(359.0f, 0.0f);
ParticleBoundFloat const ParticleEffectTransformEdit::m_boundEffectScale(32.0f, 0.001f);
ParticleBoundFloat const ParticleEffectTransformEdit::m_boundTimeScale(32000.0f, 0.0f);
ParticleBoundFloat const ParticleEffectTransformEdit::m_boundWind(256.0f);

ParticleBoundFloat const ParticleEffectTransformEdit::m_boundSpeed(5000.0f, 0.001f);
ParticleBoundFloat const ParticleEffectTransformEdit::m_boundSize(5000.0f, 0.001f);

//-----------------------------------------------------------------------------
ParticleEffectTransformEdit::ParticleEffectTransformEdit(QWidget *parent, char const *name, MainWindow *particleEditor)
 : BaseParticleEffectTransformEdit(parent, name, Qt::WStyle_Customize | Qt::WStyle_DialogBorder | Qt::WStyle_Title)
 , m_particleEditor(particleEditor)
 , m_playBackRate(1)
 , m_playBack(PB_playing)
 , m_playBackType(PBT_button)
{
	QDoubleValidator *doubleValidator = new QDoubleValidator(this);

	// Object Movement

	connect(m_objectMovementComboBox, SIGNAL(activated(int)), this, SLOT(slotObjectMovementComboBoxActivated(int)));


	// Size/Scale for bolt and circle
	m_sizeLineEdit->setText("0.0");
	m_sizeLineEdit->setValidator(doubleValidator);
	connect(m_sizeLineEdit, SIGNAL(returnPressed()), this, SLOT(onLineEditReturnPressed()));
	m_speedLineEdit->setText("0.0");
	m_speedLineEdit->setValidator(doubleValidator);
	connect(m_speedLineEdit, SIGNAL(returnPressed()), this, SLOT(onLineEditReturnPressed()));

	// Snap To Terrain

	connect(m_snapToTerrainPushButton, SIGNAL(clicked()), this, SLOT(slotSnapToTerrainPushButtonClicked()));

	// Snap To Player

	connect(m_snapToPlayerPushButton, SIGNAL(clicked()), this, SLOT(slotSnapToPlayerPushButtonClicked()));

	// Translation x

	m_translationXLineEdit->setText("0.0");
	m_translationXLineEdit->setValidator(doubleValidator);
	connect(m_translationXLineEdit, SIGNAL(returnPressed()), this, SLOT(onLineEditReturnPressed()));

	// Translation y

	m_translationYLineEdit->setText("0.0");
	m_translationYLineEdit->setValidator(doubleValidator);
	connect(m_translationYLineEdit, SIGNAL(returnPressed()), this, SLOT(onLineEditReturnPressed()));

	// Translation z

	m_translationZLineEdit->setText("0.0");
	m_translationZLineEdit->setValidator(doubleValidator);
	connect(m_translationZLineEdit, SIGNAL(returnPressed()), this, SLOT(onLineEditReturnPressed()));

	// Translation reset

	connect(m_resetTranslationPushButton, SIGNAL(clicked()), this, SLOT(slotResetTranslationPushButtonClicked()));

	// Pitch

	connect(m_pitchDial, SIGNAL(valueChanged(int)), this, SLOT(onDialValueChanged(int)));

	m_pitchLineEdit->setText("0.0");
	m_pitchLineEdit->setValidator(doubleValidator);
	connect(m_pitchLineEdit, SIGNAL(returnPressed()), this, SLOT(onLineEditReturnPressed()));

	// Heading

	connect(m_headingDial, SIGNAL(valueChanged(int)), this, SLOT(onDialValueChanged(int)));

	m_headingLineEdit->setText("0.0");
	m_headingLineEdit->setValidator(doubleValidator);
	connect(m_headingLineEdit, SIGNAL(returnPressed()), this, SLOT(onLineEditReturnPressed()));

	// Bank

	connect(m_bankDial, SIGNAL(valueChanged(int)), this, SLOT(onDialValueChanged(int)));

	m_bankLineEdit->setText("0.0");
	m_bankLineEdit->setValidator(doubleValidator);
	connect(m_bankLineEdit, SIGNAL(returnPressed()), this, SLOT(onLineEditReturnPressed()));

	// Wind

	m_windXLineEdit->setText("0.0");
	m_windYLineEdit->setText("0.0");
	m_windZLineEdit->setText("0.0");
	m_windXLineEdit->setValidator(doubleValidator);
	m_windYLineEdit->setValidator(doubleValidator);
	m_windZLineEdit->setValidator(doubleValidator);
	connect(m_windXLineEdit, SIGNAL(returnPressed()), this, SLOT(onLineEditReturnPressed()));
	connect(m_windYLineEdit, SIGNAL(returnPressed()), this, SLOT(onLineEditReturnPressed()));
	connect(m_windZLineEdit, SIGNAL(returnPressed()), this, SLOT(onLineEditReturnPressed()));
	connect(m_resetWindPushButton, SIGNAL(clicked()), this, SLOT(slotResetWindPushButtonClicked()));

	// VCR Controls

    connect(m_restartPushButton, SIGNAL(clicked()), this, SLOT(onRestartPushButtonClicked()));
    connect(m_playPushButton, SIGNAL(clicked()), this, SLOT(onPlayPushButtonClicked()));
    connect(m_pausePushButton, SIGNAL(clicked()), this, SLOT(onPausePushButtonClicked()));
    connect(m_stopPushButton, SIGNAL(clicked()), this, SLOT(onStopPushButtonClicked()));
    connect(m_slowerPushButton, SIGNAL(clicked()), this, SLOT(onSlowerPushButtonClicked()));
    connect(m_fasterPushButton, SIGNAL(clicked()), this, SLOT(onFasterPushButtonClicked()));

    connect(m_cycleTimeOfDayCheckBox, SIGNAL(clicked()), this, SLOT(onCycleTimeOfDayCheckBoxClicked()));

	// Effect Scale

	m_effectScaleLineEdit->setText("1.000");
	m_effectScaleLineEdit->setValidator(doubleValidator);
	connect(m_effectScaleLineEdit, SIGNAL(returnPressed()), this, SLOT(onLineEditReturnPressed()));
	connect(m_effectScaleResetPushButton, SIGNAL(clicked()), this, SLOT(slotEffectScaleResetPushButtonClicked()));

	// Playback Rate

	m_playBackRateLineEdit->setText("1.000");
	m_playBackRateLineEdit->setValidator(doubleValidator);
	connect(m_playBackRateLineEdit, SIGNAL(returnPressed()), this, SLOT(slotPlayBackRateLineEditReturnPressed()));
	connect(m_playBackRateResetPushButton, SIGNAL(clicked()), this, SLOT(slotPlayBackRateResetPushButtonClicked()));

	setPlayBackRate();

	m_objectMovementComboBox->setCurrentItem(OM_specifiedTranslation);
	m_translationXLineEdit->setEnabled(true);
	m_translationYLineEdit->setEnabled(true);
	m_translationZLineEdit->setEnabled(true);
	m_sizeLineEdit->setEnabled(false);
	m_speedLineEdit->setEnabled(false);

}

//-----------------------------------------------------------------------------
void ParticleEffectTransformEdit::slotSnapToTerrainPushButtonClicked()
{
	float const x = ParticleEditorUtility::getFloat(m_translationXLineEdit);
	float const y = ParticleEditorUtility::getFloat(m_translationYLineEdit);
	float const z = ParticleEditorUtility::getFloat(m_translationZLineEdit);
	float height;

	if (!TerrainObject::getInstance()->getHeight(Vector(x, y, z), height))
	{
		height = 0.0f;
	}

	ParticleEditorUtility::setLineEditFloat(m_translationYLineEdit, height, m_boundTranslation, 1);

	updateTransform();
	m_particleEditor->restartParticleSystem();
}

//-----------------------------------------------------------------------------
void ParticleEffectTransformEdit::slotSnapToPlayerPushButtonClicked()
{
	ParticleEditorUtility::setLineEditFloat(m_translationXLineEdit, Game::getPlayer()->getPosition_w().x, m_boundTranslation, 1);
	ParticleEditorUtility::setLineEditFloat(m_translationYLineEdit, Game::getPlayer()->getPosition_w().y, m_boundTranslation, 1);
	ParticleEditorUtility::setLineEditFloat(m_translationZLineEdit, Game::getPlayer()->getPosition_w().z, m_boundTranslation, 1);

	updateTransform();
	m_particleEditor->restartParticleSystem();
}

//-----------------------------------------------------------------------------
void ParticleEffectTransformEdit::slotObjectMovementComboBoxActivated(int)
{
	ObjectMovement const objectMovement = static_cast<ObjectMovement>(m_objectMovementComboBox->currentItem());

	m_particleEditor->setObjectMovement(objectMovement);

	switch (objectMovement)
	{
		case OM_moveInACircle:
			{
				m_translationXLineEdit->setEnabled(true);
				m_translationYLineEdit->setEnabled(true);
				m_translationZLineEdit->setEnabled(true);
				m_sizeLineEdit->setEnabled(true);
				m_speedLineEdit->setEnabled(true);
				m_sizeLineEdit->setText("9.0");
				m_speedLineEdit->setText("20.0");
				// set the speed and size
				m_particleEditor->setSpeed(ParticleEditorUtility::getFloat(m_speedLineEdit));
				m_particleEditor->setSize(ParticleEditorUtility::getFloat(m_sizeLineEdit));
			}
			break;
		case OM_simulateBoltGround:
			{
				m_translationXLineEdit->setEnabled(true);
				m_translationYLineEdit->setEnabled(true);
				m_translationZLineEdit->setEnabled(true);
				m_sizeLineEdit->setEnabled(true);
				m_speedLineEdit->setEnabled(true);
				m_sizeLineEdit->setText("80.0");
				m_speedLineEdit->setText("40.0");
				// set the speed and size
				m_particleEditor->setSpeed(ParticleEditorUtility::getFloat(m_speedLineEdit));
				m_particleEditor->setSize(ParticleEditorUtility::getFloat(m_sizeLineEdit));
			}
			break;
		case OM_simulateBoltSpace:
			{
				m_translationXLineEdit->setEnabled(true);
				m_translationYLineEdit->setEnabled(true);
				m_translationZLineEdit->setEnabled(true);
				m_sizeLineEdit->setEnabled(true);
				m_speedLineEdit->setEnabled(true);
				m_sizeLineEdit->setText("800.0");
				m_speedLineEdit->setText("400.0");
				// set the speed and size
				m_particleEditor->setSpeed(ParticleEditorUtility::getFloat(m_speedLineEdit));
				m_particleEditor->setSize(ParticleEditorUtility::getFloat(m_sizeLineEdit));
			}
			break;
		case OM_specifiedTranslation:	
		case OM_simulateGrenade:
			{
				m_translationXLineEdit->setEnabled(true);
				m_translationYLineEdit->setEnabled(true);
				m_translationZLineEdit->setEnabled(true);
				m_sizeLineEdit->setEnabled(false);
				m_speedLineEdit->setEnabled(false);
			}
			break;
		case OM_lockToXYZ:
			{
				m_translationXLineEdit->setEnabled(false);
				m_translationYLineEdit->setEnabled(false);
				m_translationZLineEdit->setEnabled(false);
				m_sizeLineEdit->setEnabled(false);
				m_speedLineEdit->setEnabled(false);
			}
			break;
		case OM_lockToXZUseWorldY:
		case OM_lockToXZUsePlayerRelativeY:
		case OM_spawnToXZUseWorldY:
		case OM_spawnToXZUsePlayerRelativeY:
			{
				m_translationXLineEdit->setEnabled(false);
				m_translationYLineEdit->setEnabled(true);
				m_translationZLineEdit->setEnabled(false);
				m_sizeLineEdit->setEnabled(false);
				m_speedLineEdit->setEnabled(false);
			}
			break;
		default:
			{
				DEBUG_FATAL(true, ("Unexpected object movement specified."));
			}
			break;
	}
}

//-----------------------------------------------------------------------------
void ParticleEffectTransformEdit::slotEffectScaleResetPushButtonClicked()
{
	m_effectScaleLineEdit->setText("1.000");
	onLineEditReturnPressed();
}

//-----------------------------------------------------------------------------
void ParticleEffectTransformEdit::slotPlayBackRateResetPushButtonClicked()
{
	m_playBackRateLineEdit->setText("1.000");
	slotPlayBackRateLineEditReturnPressed();
}

//-----------------------------------------------------------------------------
void ParticleEffectTransformEdit::onLineEditReturnPressed()
{
	validate();
	updateTransform();
}

//-----------------------------------------------------------------------------
void ParticleEffectTransformEdit::onDialValueChanged(int)
{
	// Pitch

	ParticleEditorUtility::setLineEditFloat(m_pitchLineEdit, m_boundRotation.getValue(ParticleEditorUtility::getRangeControlPercent(m_pitchDial)), m_boundRotation, 1);

	// Heading

	ParticleEditorUtility::setLineEditFloat(m_headingLineEdit, m_boundRotation.getValue(ParticleEditorUtility::getRangeControlPercent(m_headingDial)), m_boundRotation, 1);

	// Bank

	ParticleEditorUtility::setLineEditFloat(m_bankLineEdit, m_boundRotation.getValue(ParticleEditorUtility::getRangeControlPercent(m_bankDial)), m_boundRotation, 1);

	validate();
	updateTransform();
}

//-----------------------------------------------------------------------------
void ParticleEffectTransformEdit::validate()
{
	// Validate all the values

	ParticleEditorUtility::validateLineEditFloat(m_translationXLineEdit, m_boundTranslation, 1);
	ParticleEditorUtility::validateLineEditFloat(m_translationYLineEdit, m_boundTranslation, 1);
	ParticleEditorUtility::validateLineEditFloat(m_translationZLineEdit, m_boundTranslation, 1);
	ParticleEditorUtility::validateLineEditFloat(m_effectScaleLineEdit, m_boundEffectScale, 3);
	ParticleEditorUtility::validateLineEditFloat(m_playBackRateLineEdit, m_boundTimeScale, 3);
	ParticleEditorUtility::validateLineEditFloat(m_windXLineEdit, m_boundWind, 1);
	ParticleEditorUtility::validateLineEditFloat(m_windYLineEdit, m_boundWind, 1);
	ParticleEditorUtility::validateLineEditFloat(m_windZLineEdit, m_boundWind, 1);

	ParticleEditorUtility::validateLineEditFloat(m_speedLineEdit, m_boundSpeed, 1);
	ParticleEditorUtility::validateLineEditFloat(m_sizeLineEdit,m_boundSize,1);

	float const pitch = ParticleEditorUtility::validateLineEditFloat(m_pitchLineEdit, m_boundRotation, 1);
	m_pitchDial->setValue(static_cast<int>(m_boundRotation.getPercent(pitch) * m_boundRotation.getDifference()));

	float const heading = ParticleEditorUtility::validateLineEditFloat(m_headingLineEdit, m_boundRotation, 1);
	m_headingDial->setValue(static_cast<int>(m_boundRotation.getPercent(heading) * m_boundRotation.getDifference()));

	float const bank = ParticleEditorUtility::validateLineEditFloat(m_bankLineEdit, m_boundRotation, 1);
	m_bankDial->setValue(static_cast<int>(m_boundRotation.getPercent(bank) * m_boundRotation.getDifference()));
}

//-----------------------------------------------------------------------------
void ParticleEffectTransformEdit::updateTransform()
{
	// Transform

	float const translationX = ParticleEditorUtility::getFloat(m_translationXLineEdit);
	float const translationY = ParticleEditorUtility::getFloat(m_translationYLineEdit);
	float const translationZ = ParticleEditorUtility::getFloat(m_translationZLineEdit);
	float const pitch = ParticleEditorUtility::getFloat(m_pitchLineEdit);
	float const heading = ParticleEditorUtility::getFloat(m_headingLineEdit);
	float const bank = ParticleEditorUtility::getFloat(m_bankLineEdit);

	Transform transform;
	transform.setPosition_p(Vector(translationX, translationY, translationZ));
	transform.yaw_l(heading * PI_OVER_180); // heading (y)
	transform.pitch_l(pitch * PI_OVER_180); // pitch (x)
	transform.roll_l(bank * PI_OVER_180);   // bank (z)

	m_particleEditor->setObjectTransform(transform);

	// Effect Scale

	float const effectScale = ParticleEditorUtility::getFloat(m_effectScaleLineEdit);

	m_particleEditor->setEffectScale(effectScale);

	// Time Scale

	m_particleEditor->setPlayBackRate(getPlayBackRate());

	// Set the global wind

	float const windX = ParticleEditorUtility::getFloat(m_windXLineEdit);
	float const windY = ParticleEditorUtility::getFloat(m_windYLineEdit);
	float const windZ = ParticleEditorUtility::getFloat(m_windZLineEdit);
	ParticleEffectAppearance::setGlobalWind(Vector(windX, windY, windZ));

	// set the speed and size
	m_particleEditor->setSpeed(ParticleEditorUtility::getFloat(m_speedLineEdit));
	m_particleEditor->setSize(ParticleEditorUtility::getFloat(m_sizeLineEdit));
}

//-----------------------------------------------------------------------------
void ParticleEffectTransformEdit::onRestartPushButtonClicked()
{
	m_playBack = PB_playing;
	setPlayBackRate();

	m_particleEditor->restartParticleSystem();
}

//-----------------------------------------------------------------------------
void ParticleEffectTransformEdit::onPlayPushButtonClicked()
{
	m_playBack = PB_playing;

	setPlayBackRate();
}

//-----------------------------------------------------------------------------
void ParticleEffectTransformEdit::onPausePushButtonClicked()
{
	m_playBack = PB_paused;

	setPlayBackRate();
}

//-----------------------------------------------------------------------------
void ParticleEffectTransformEdit::onStopPushButtonClicked()
{
	m_playBack = PB_stopped;
	m_playBackType = PBT_button;
	m_playBackRate = 1;

	m_particleEditor->restartParticleSystem();

	setPlayBackRate();
}

//-----------------------------------------------------------------------------
void ParticleEffectTransformEdit::onSlowerPushButtonClicked()
{
	--m_playBackRate;

	if (m_playBackType != PBT_button)
	{
		m_playBackType = PBT_button;
		m_playBackRate = 1;
	}

	if (m_playBackRate == 0)
	{
		m_playBackRate = -1;
	}
	else if (m_playBackRate < -10)
	{
		m_playBackRate = -10;
	}

	setPlayBackRate();
}

//-----------------------------------------------------------------------------
void ParticleEffectTransformEdit::onFasterPushButtonClicked()
{
	++m_playBackRate;

	if (m_playBackType != PBT_button)
	{
		m_playBackType = PBT_button;
		m_playBackRate = 1;
	}

	if (m_playBackRate == 0)
	{
		m_playBackRate = 1;
	}

	setPlayBackRate();
}

//-----------------------------------------------------------------------------
QString ParticleEffectTransformEdit::getPlayBackRateString() const
{
	QString result;

	switch (m_playBackType)
	{
		case PBT_button:
			{
				if (m_playBackRate >= 0)
				{
					result.sprintf("%dx", m_playBackRate);
				}
				else if (m_playBackRate < 0)
				{
					int const rate = static_cast<int>(powf(2.0f, abs(static_cast<float>(m_playBackRate))));

					result.sprintf("1/%dx", rate);
				}
			}
			break;
		case PBT_lineEdit:
			{
				result.sprintf("%.3fx", ParticleEditorUtility::getFloat(m_playBackRateLineEdit));
			}
			break;
	}
	
	return result;
}

//-----------------------------------------------------------------------------
void ParticleEffectTransformEdit::setPlayBackRate()
{
	QString title;

	switch (m_playBack)
	{
		case PB_playing:
			{
				title.sprintf("Playing ( %s )", getPlayBackRateString().latin1());
			}
			break;
		case PB_paused:
			{
				title.sprintf("Paused ( %s )", getPlayBackRateString().latin1());
			}
			break;
		case PB_stopped:
			{
				title.sprintf("Stopped ( %s )", getPlayBackRateString().latin1());
			}
			break;
		default:
			{
				DEBUG_FATAL(true, ("ParticleEffectTransformEdit::setPlayBackTite() - Unexpected playback state"));
			}
			break;
	}

	// Match the time scale line edit to the button clicks

	QString playBackRate;
	playBackRate.sprintf("%.3f", getPlayBackRate());
	m_playBackRateLineEdit->setText(playBackRate);

	// Set the playback group box title

	m_playBackGroupBox->setTitle(title);

	updateTimeScale();
}

//-----------------------------------------------------------------------------
void ParticleEffectTransformEdit::updateTimeScale()
{
	float percent = 0.0f;

	if (m_playBack == PB_playing)
	{
		percent = getPlayBackRate();
	}

	m_particleEditor->setPlayBackRate(percent);
}

//-----------------------------------------------------------------------------
float ParticleEffectTransformEdit::getPlayBackRate() const
{
	float percent = 1.0f;

	switch (m_playBackType)
	{
		case PBT_button:
			{
				if (m_playBackRate >= 1)
				{
					percent = static_cast<float>(m_playBackRate);
				}
				else if (m_playBackRate < 0)
				{
					int const rate = static_cast<int>(powf(2.0f, abs(static_cast<float>(m_playBackRate))));

					percent = 1.0f / static_cast<float>(abs(rate));
				}
			}
			break;
		case PBT_lineEdit:
			{
				percent = ParticleEditorUtility::getFloat(m_playBackRateLineEdit);
			}
			break;
	}

	return percent;
}

//-----------------------------------------------------------------------------
void ParticleEffectTransformEdit::slotPlayBackRateLineEditReturnPressed()
{
	m_playBackType = PBT_lineEdit;

	onLineEditReturnPressed();

	setPlayBackRate();
}

//-----------------------------------------------------------------------------
void ParticleEffectTransformEdit::slotResetTranslationPushButtonClicked()
{
	m_translationXLineEdit->setText("0.0");
	m_translationYLineEdit->setText("0.0");
	m_translationZLineEdit->setText("0.0");

	onLineEditReturnPressed();
}

//-----------------------------------------------------------------------------
void ParticleEffectTransformEdit::slotResetWindPushButtonClicked()
{
	m_windXLineEdit->setText("0.0");
	m_windYLineEdit->setText("0.0");
	m_windZLineEdit->setText("0.0");

	onLineEditReturnPressed();
}

//-----------------------------------------------------------------------------
void ParticleEffectTransformEdit::onCycleTimeOfDayCheckBoxClicked()
{
	m_particleEditor->setTimeOfDayCycle(m_cycleTimeOfDayCheckBox->isChecked());
}

// ============================================================================
