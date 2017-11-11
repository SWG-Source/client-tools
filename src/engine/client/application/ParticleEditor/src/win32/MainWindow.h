// ============================================================================
//
// MainWindow.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_MainWindow_H
#define INCLUDED_MainWindow_H

#include "BaseMainWindow.h"
#include "ParticleEffectTransformEdit.h"

class AttributeEditor;
class ColorRamp;
class GameWidget;
class Iff;
class ParticleEditorIoWin;
class ParticleEffectListViewer;
class ParticleEffectTransformEdit;
class PEAttributeViewer;
class QString;
class QWorkspace;
class Transform;
class WaveForm;

//-----------------------------------------------------------------------------
class MainWindow : public BaseMainWindow
{
	Q_OBJECT

public:

	MainWindow(QWidget *parent, char const *name);
	virtual ~MainWindow();

	virtual void fileNew();
	virtual void fileOpen();
	virtual void fileOpen(QString &selectedFileName);
	virtual void fileReload();
	virtual void fileSave();
	virtual void fileSaveAs();
	virtual void fileExit();

	void restartParticleSystem();
	void setObjectTransform(Transform const &transform);
	void setEffectScale(float const effectScale);
	void setPlayBackRate(float const playBackRate);
	void setObjectMovement(ParticleEffectTransformEdit::ObjectMovement const objectMovement);
	void setTimeOfDayCycle(bool const timeOfDayCycle);
	void setSpeed(float const speed);
	void setSize(float const size);

private slots:

	void onBuildParticleEffect();
	void onOptionsTerrainVisibleAction();
	void onOptionsParticleExtentsVisibleAction();
	void onOptionsParticleAxisVisibleAction();
	void onOptionsParticleTexturesVisibleAction();
	void slotOptionsShowParticleOrientationAction();
	void slotOptionsShowParticleVelocityAction();
	void onOptionsShowDebugTextAction();
	void slotUpdateTimerTimeOut();

private:

	PEAttributeViewer *          m_attributeViewer;
	ParticleEffectListViewer *   m_particleEffectListViewer;
	QWorkspace *                 m_workSpace;
	QPoint                       m_mousePosition;
	AttributeEditor *            m_attributeEditor;
	GameWidget *                 m_gameWidget;
	ParticleEffectTransformEdit *m_particleEffectTransformEdit;
	ParticleEditorIoWin *        m_ioWin;
	Iff *                        m_particleEffectIff;
	QFileInfo                    m_fileInfo;
	QTimer *                     m_updateTimer;

	virtual void paintEvent(QPaintEvent *paintEvent);
	virtual void keyReleaseEvent(QKeyEvent *keyEvent);

	void         save(QString const &path);
	void         setParticleEffectAppearanceTemplate(Iff &iff);

private:

	// Disabled

	MainWindow(MainWindow const &);
	MainWindow &operator =(MainWindow const &);
};

// ============================================================================

#endif // INCLUDED_MainWindow_H
