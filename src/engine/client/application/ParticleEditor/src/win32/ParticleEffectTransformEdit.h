// ============================================================================
//
// ParticleEffectTransformEdit.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_ParticleEffectTransformEdit_H
#define INCLUDED_ParticleEffectTransformEdit_H

#include "BaseParticleEffectTransformEdit.h"

class MainWindow;
class Transform;

//-----------------------------------------------------------------------------
class ParticleEffectTransformEdit : public BaseParticleEffectTransformEdit
{
	Q_OBJECT

public:

	enum ObjectMovement
	{
		OM_specifiedTranslation,
		OM_moveInACircle,
		OM_simulateBoltGround,
		OM_simulateBoltSpace,
		OM_simulateGrenade,
		OM_lockToXYZ,
		OM_lockToXZUseWorldY,
		OM_lockToXZUsePlayerRelativeY,
		OM_spawnToXZUseWorldY,
		OM_spawnToXZUsePlayerRelativeY
	};

public:

	ParticleEffectTransformEdit(QWidget *parent, char const *name, MainWindow *particleEditor);

private slots:

	void slotObjectMovementComboBoxActivated(int);
	void slotSnapToTerrainPushButtonClicked();
	void slotSnapToPlayerPushButtonClicked();
	void onLineEditReturnPressed();
	void onDialValueChanged(int);
    void onRestartPushButtonClicked();
    void onPlayPushButtonClicked();
    void onPausePushButtonClicked();
    void onStopPushButtonClicked();
    void onSlowerPushButtonClicked();
    void onFasterPushButtonClicked();
	void slotPlayBackRateLineEditReturnPressed();
	void slotPlayBackRateResetPushButtonClicked();
	void slotEffectScaleResetPushButtonClicked();
	void slotResetTranslationPushButtonClicked();
	void slotResetWindPushButtonClicked();
	void onCycleTimeOfDayCheckBoxClicked();

private:

	enum PlayBack
	{
		PB_playing,
		PB_paused,
		PB_stopped
	};

	enum PlayBackType
	{
		PBT_button,
		PBT_lineEdit
	};

	void    updateTransform();
	void    updateTimeScale();
	QString getPlayBackRateString() const;
	void    setPlayBackRate();
	float   getPlayBackRate() const;
	void    validate();

	PlayBack                        m_playBack;
	PlayBackType                    m_playBackType;
	int                             m_playBackRate;
	MainWindow *                    m_particleEditor;

	static ParticleBoundFloat const m_boundTranslation;
	static ParticleBoundFloat const m_boundRotation;
	static ParticleBoundFloat const m_boundEffectScale;
	static ParticleBoundFloat const m_boundTimeScale;
	static ParticleBoundFloat const m_boundWind;
	static ParticleBoundFloat const m_boundSpeed;
	static ParticleBoundFloat const m_boundSize;
};

// ============================================================================

#endif // INCLUDED_ParticleEffectTransformEdit_H
