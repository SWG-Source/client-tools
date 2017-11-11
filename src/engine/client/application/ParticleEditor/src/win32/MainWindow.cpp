// ============================================================================
//
// MainWindow.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstParticleEditor.h"
#include "MainWindow.h"
#include "MainWindow.moc"

#include "clientGame/Game.h"
#include "clientGame/GroundScene.h"
#include "clientObject/GameCamera.h"
#include "clientParticle/ConfigClientParticle.h"
#include "clientParticle/ParticleEffectAppearance.h"
#include "clientParticle/ParticleEffectAppearanceTemplate.h"
#include "clientParticle/ParticleEmitter.h"
#include "clientParticle/ParticleManager.h"
#include "clientParticle/SetupClientParticle.h"
#include "sharedDebug/PerformanceTimer.h"
#include "sharedFile/FileNameUtils.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/ConfigSharedFoundation.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedTerrain/TerrainObject.h"
#include "AttributeEditor.h"
#include "PEAttributeViewer.h"
#include "ColorRampEdit.h"
#include "GameWidget.h"
#include "ParticleEditorIoWin.h"
#include "ParticleEditorUtility.h"
#include "ParticleEffectListViewer.h"
#include "ParticleEffectTransformEdit.h"
#include "WaveFormEdit.h"

#include "PEAttributeViewer.h"

// ============================================================================
//
// MainWindow
//
// ============================================================================

//-----------------------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent, char const *name)
 : BaseMainWindow(parent, name)
 , m_attributeViewer(NULL)
 , m_particleEffectListViewer(NULL)
 , m_attributeEditor(NULL)
 , m_workSpace(NULL)
 , m_ioWin(NULL)
 , m_particleEffectIff(NULL)
 , m_fileInfo(FileNameUtils::get(ParticleEditorUtility::getParticleEffectPath().latin1(), FileNameUtils::drive | FileNameUtils::directory).c_str())
 , m_updateTimer(NULL)
{
	// Get rid of the status bar

	statusBar()->hide();

	// Disable some menus and actions until I code them

	helpMenu->setEnabled(false);

	// Add a workspace

	QWorkspace *m_workSpace = new QWorkspace(this);
	setCentralWidget(m_workSpace);

	// Game widget

	m_gameWidget = new GameWidget(m_workSpace, "GameWidget", Qt::WStyle_Customize | Qt::WStyle_DialogBorder | Qt::WStyle_Title);
	ParticleEditorUtility::loadWidget(*m_gameWidget, 385, 3);
	m_gameWidget->runGameLoop();

	// Attribute editor

	m_attributeEditor = new AttributeEditor(m_workSpace, "AttributeEditor", Qt::WStyle_Customize | Qt::WStyle_DialogBorder | Qt::WStyle_Title);

	// Attribute viewer

	m_attributeViewer = new PEAttributeViewer(m_workSpace, "PEAttributeViewer", Qt::WStyle_Customize | Qt::WStyle_DialogBorder | Qt::WStyle_Title);
	m_attributeViewer->setFixedWidth(370);

	// Particle effect list viewer

	m_particleEffectListViewer = new ParticleEffectListViewer(m_workSpace, "ParticleEffectListViewer", Qt::WStyle_Customize | Qt::WStyle_DialogBorder | Qt::WStyle_Title);

	// Particle effect transform edit

	m_particleEffectTransformEdit = new ParticleEffectTransformEdit(m_workSpace, "ParticleEffectTransformEdit", this);

	// Signal that the particle effect transform has changed so we can move the particle effect

	connect(m_particleEffectTransformEdit, SIGNAL(particleEffectTransformChanged(const Transform &)), this, SLOT(onParticleEffectTransformChanged(const Transform &)));
	connect(m_particleEffectTransformEdit, SIGNAL(signalParticleEffectTimeScaleChanged(const float)), this, SLOT(slotParticleEffectTimeScaleChanged(const float)));

	// Signal the attribute editor when an attribute is selected in the attribute viewer

	connect(m_attributeViewer, SIGNAL(waveFormAttributeSelected(const WaveForm &)), m_attributeEditor, SLOT(setWaveForm(const WaveForm &)));
	connect(m_attributeViewer, SIGNAL(colorRampAttributeSelected(const ColorRamp &, const WaveForm &)), m_attributeEditor, SLOT(setColorRamp(const ColorRamp &, const WaveForm &)));

	// Signal the particle effect list viewer when the attribute viewer has a changed attribute

	connect(m_attributeViewer, SIGNAL(particleAttachmentDescriptionChanged(const ParticleAttachmentDescription &)), m_particleEffectListViewer, SLOT(onParticleAttachmentDescriptionChanged(const ParticleAttachmentDescription &)));
	connect(m_attributeViewer, SIGNAL(particleDescriptionChanged(const ParticleDescription &)), m_particleEffectListViewer, SLOT(onParticleDescriptionChanged(const ParticleDescription &)));
	connect(m_attributeViewer, SIGNAL(particleEmitterDescriptionChanged(const ParticleEmitterDescription &)), m_particleEffectListViewer, SLOT(onParticleEmitterDescriptionChanged(const ParticleEmitterDescription &)));
	connect(m_attributeViewer, SIGNAL(particleEmitterGroupDescriptionChanged(const ParticleEmitterGroupDescription &)), m_particleEffectListViewer, SLOT(onParticleEmitterGroupDescriptionChanged(const ParticleEmitterGroupDescription &)));
	connect(m_attributeViewer, SIGNAL(particleEffectDescriptionChanged(const ParticleEffectDescription &)), m_particleEffectListViewer, SLOT(onParticleEffectDescriptionChanged(const ParticleEffectDescription &)));
	connect(m_attributeViewer, SIGNAL(attributeNameChanged(const std::string &)), m_particleEffectListViewer, SLOT(onAttributeNameChanged(const std::string &)));

	// Signal the attribute viewer that the attribute editor has changed and whether a 
	// rebuild needs to occur

	connect(m_attributeEditor, SIGNAL(waveFormChanged(const WaveForm &, const bool)), m_attributeViewer, SLOT(onWaveFormChanged(const WaveForm &, const bool)));
	connect(m_attributeEditor, SIGNAL(colorRampChanged(const ColorRamp &, const WaveForm &, const bool)), m_attributeViewer, SLOT(onColorRampChanged(const ColorRamp &, const WaveForm &, const bool)));

	// Signal the main window to create a new particle effect when an attribute changes

	connect(m_attributeViewer, SIGNAL(buildParticleEffect()), this, SLOT(onBuildParticleEffect()));

	// Signal the attribute viewer when an item in the particle effect list is selected so it can
	// change to the correct attributes

	connect(m_particleEffectListViewer, SIGNAL(signalUnDefine()), m_attributeEditor, SLOT(slotUnDefine()));
	connect(m_particleEffectListViewer, SIGNAL(showParticleAttachmentAttributes(const ParticleAttachmentDescription &)), m_attributeViewer, SLOT(onShowParticleAttachmentAttributes(const ParticleAttachmentDescription &)));
	connect(m_particleEffectListViewer, SIGNAL(showParticleAttributes(const ParticleDescription &)), m_attributeViewer, SLOT(onShowParticleAttributes(const ParticleDescription &)));
	connect(m_particleEffectListViewer, SIGNAL(showParticleEmitterAttributes(const ParticleEmitterDescription &)), m_attributeViewer, SLOT(onShowParticleEmitterAttributes(const ParticleEmitterDescription &)));
	connect(m_particleEffectListViewer, SIGNAL(showParticleEmitterGroupAttributes(const ParticleEmitterGroupDescription &)), m_attributeViewer, SLOT(onShowParticleEmitterGroupAttributes(const ParticleEmitterGroupDescription &)));
	connect(m_particleEffectListViewer, SIGNAL(showParticleEffectAttributes(const ParticleEffectDescription &)), m_attributeViewer, SLOT(onShowParticleEffectAttributes(const ParticleEffectDescription &)));
	connect(m_particleEffectListViewer, SIGNAL(showParticleEffectGroupAttributes()), m_attributeViewer, SLOT(onShowParticleEffectGroupAttributes()));

	// Signal the main window to create a new particle effect when something in the particle effect list viewer
	// changed that needs a rebuild

	connect(m_particleEffectListViewer, SIGNAL(forceRebuild()), this, SLOT(onBuildParticleEffect()));

	// Action connections

	connect(optionsTerrainVisibleAction, SIGNAL(activated()), this, SLOT(onOptionsTerrainVisibleAction()));
	connect(optionsParticleExtentsVisibleAction, SIGNAL(activated()), this, SLOT(onOptionsParticleExtentsVisibleAction()));
	connect(optionsParticleAxisVisibleAction, SIGNAL(activated()), this, SLOT(onOptionsParticleAxisVisibleAction()));
	connect(optionsParticleTexturesVisibleAction, SIGNAL(activated()), this, SLOT(onOptionsParticleTexturesVisibleAction()));
	connect(optionsShowParticleOrientationAction, SIGNAL(activated()), this, SLOT(slotOptionsShowParticleOrientationAction()));
	connect(optionsShowParticleVelocityAction, SIGNAL(activated()), this, SLOT(slotOptionsShowParticleVelocityAction()));
	connect(optionsShowDebugTextAction, SIGNAL(activated()), this, SLOT(onOptionsShowDebugTextAction()));

	// Create the IOWin

	m_ioWin = NON_NULL(new ParticleEditorIoWin(this));

	// Build the initial particle effect

	onBuildParticleEffect();

	// Open the IOWin initially

	m_ioWin->open();

	// Enable some default parameters

	ParticleManager::setDebugVelocityEnabled(true);
	ParticleManager::setDebugAxisEnabled(true);

	optionsTerrainVisibleAction->setOn(true);
	optionsParticleExtentsVisibleAction->setOn(ParticleManager::isDebugExtentsEnabled());
	optionsParticleAxisVisibleAction->setOn(ParticleManager::isDebugAxisEnabled());
	optionsParticleTexturesVisibleAction->setOn(ParticleManager::isTexturingEnabled());
	optionsShowParticleOrientationAction->setOn(ParticleManager::isDebugParticleOrientationEnabled());
	optionsShowParticleVelocityAction->setOn(ParticleManager::isDebugVelocityEnabled());

	// Load the child widget settings

	ParticleEditorUtility::loadWidget(*m_attributeEditor, 385, 638, 541, 291);
	ParticleEditorUtility::loadWidget(*m_attributeViewer, 5, 298, 370, 630);
	ParticleEditorUtility::loadWidget(*m_particleEffectListViewer, 4, 4, 370, 256);
	ParticleEditorUtility::loadWidget(*m_particleEffectTransformEdit, 934, 639, 250, 290);
	ParticleEditorUtility::loadWidget(*this, 0, 0, 1194, 990);

	m_updateTimer = new QTimer(this, "UpdateTimer");
	m_updateTimer->start(1000 / 40);
	connect(m_updateTimer, SIGNAL(timeout()), this, SLOT(slotUpdateTimerTimeOut()));
}

//-----------------------------------------------------------------------------
MainWindow::~MainWindow()
{
	// Save the widget settings

	ParticleEditorUtility::saveWidget(*m_attributeEditor);
	ParticleEditorUtility::saveWidget(*m_attributeViewer);
	ParticleEditorUtility::saveWidget(*m_particleEffectListViewer);
	ParticleEditorUtility::saveWidget(*m_particleEffectTransformEdit);
	ParticleEditorUtility::saveWidget(*m_gameWidget);
	ParticleEditorUtility::saveWidget(*this);
	
	if (m_ioWin)
	{
		m_ioWin->close();
		delete m_ioWin;
	}
}

//-----------------------------------------------------------------------------
void MainWindow::slotUpdateTimerTimeOut()
{
	static PerformanceTimer performanceTimer;
	performanceTimer.stop();
	float const deltaTime = performanceTimer.getElapsedTime();
	performanceTimer.start();
	
	if (deltaTime < 1.0f)
	{
		m_ioWin->alter(deltaTime);
	}
}

//-----------------------------------------------------------------------------
void MainWindow::paintEvent(QPaintEvent *paintEvent)
{
	UNREF(paintEvent);

	char const * const fullPath = m_fileInfo.filePath().latin1();
	std::string const & fileName = FileNameUtils::get(fullPath, FileNameUtils::fileName | FileNameUtils::extension);
	
	char text[4096];
	snprintf(text, sizeof(text) - 1, "SWG Particle Editor (%s - %s) : %s", __DATE__, __TIME__, (m_fileInfo.fileName() == "") ? "Default" : fileName.c_str());
	text[sizeof(text) - 1] = '\0';
	setCaption(text);
}

//-----------------------------------------------------------------------------
void MainWindow::fileNew()
{
	// Make sure the user wants to lose their changes

	int result = QMessageBox::warning(this, "Reset", "Reseting the particle effect will lose all current changes.", QMessageBox::Ok, QMessageBox::Cancel);

	if (result == QMessageBox::Ok)
	{
		m_fileInfo.setFile("");
		m_attributeEditor->reset();
		m_particleEffectListViewer->reset();

		onBuildParticleEffect();
	}
}

//-----------------------------------------------------------------------------
void MainWindow::fileOpen(QString &selectedFileName)
{
	// Make sure the specified file exists on disk

	if (!selectedFileName.isNull() && FileNameUtils::isReadable(selectedFileName.latin1()))
	{
		// Make sure this is a valid particle effect Iff file

		if (ParticleEditorUtility::isValidIffFile(this, selectedFileName.latin1(), ParticleEffectAppearanceTemplate::getTag()))
		{
			m_attributeEditor->reset();

			// Save the path

			m_fileInfo = selectedFileName;
			ParticleEditorUtility::setParticleEffectPath(selectedFileName);

			//Flush the old data
			m_particleEffectListViewer->reset();
			onBuildParticleEffect();

			// Get the current particle effect iff

			Iff iff(selectedFileName.latin1());
			m_particleEffectListViewer->load(iff);
			setParticleEffectAppearanceTemplate(iff);
		}
	}
	else if (!selectedFileName.isEmpty())
	{
		QString text;
		text.sprintf("The selected file does not exist on disk: \"%s\"", selectedFileName.latin1());
		QMessageBox::warning(this, "File Error", text, "OK");
	}
}

//-----------------------------------------------------------------------------
void MainWindow::fileOpen()
{
	// Get the file

	QString selectedFileName(QFileDialog::getOpenFileName(m_fileInfo.filePath(), "Particle Effect files (*.prt)", this, "OpenFileDialog", "Open Particle Effect"));

	fileOpen(selectedFileName);
}

//-----------------------------------------------------------------------------
void MainWindow::fileReload()
{
	// Get the file

	if (!m_fileInfo.isFile())
	{
		return;
	}
	int result = QMessageBox::warning(this, "Reset", "Reloading the particle effect will lose all current changes.", QMessageBox::Ok, QMessageBox::Cancel);

	if (result == QMessageBox::Ok)
	{
		QString selectedFileName(m_fileInfo.filePath());

		fileOpen(selectedFileName);		
	}
}

//-----------------------------------------------------------------------------
void MainWindow::setParticleEffectAppearanceTemplate(Iff &iff)
{
	AppearanceTemplate const *appearanceTemplate = AppearanceTemplateList::fetch(&iff);

	m_ioWin->setAppearanceTemplate(appearanceTemplate);

	m_particleEffectListViewer->setObject(m_ioWin->getObject());
}

//-----------------------------------------------------------------------------
void MainWindow::fileSaveAs()
{
	// Get the file

	QString saveFileName(QFileDialog::getSaveFileName(m_fileInfo.filePath(), "Particle Effect files (*.prt)", this, "SaveFileDialog", "Save Particle Effect"));

	if (!saveFileName.isNull())
	{
		if (saveFileName.isEmpty())
		{
			QMessageBox::warning(this, "Error Saving File", "Empty filename specified.", "OK.");

			// Offer them to choose a new filename

			fileSaveAs();
		}
		else
		{
			QString newPath((FileNameUtils::get(saveFileName.latin1(), FileNameUtils::drive | FileNameUtils::directory | FileNameUtils::fileName).c_str()) + QString(".prt"));
		
			if (!FileNameUtils::isWritable(newPath.latin1()))
			{
				QString text;
				text.sprintf("Error saving: \"%s\". Make sure the file is not read only.", newPath.latin1());
				QMessageBox::warning(this, "Error Saving File", text, "OK.");
		
				// Offer them to choose a new filename
		
				fileSaveAs();
			}
			else
			{
				// The file is good to save
		
				save(newPath);
			}
		}
	}
	else
	{
		// Esc was pressed so fall through
	}
}

//-----------------------------------------------------------------------------
void MainWindow::fileSave()
{
	if (m_fileInfo.isFile())
	{
		save(m_fileInfo.filePath());
	}
	else
	{
		fileSaveAs();
	}
}

//-----------------------------------------------------------------------------
void MainWindow::save(QString const &path)
{
	if (!path.isNull() && FileNameUtils::isWritable(path.latin1()))
	{
		m_fileInfo.setFile(QString(FileNameUtils::get(path.latin1(), FileNameUtils::drive | FileNameUtils::directory | FileNameUtils::fileName).c_str()) + QString(".prt"));

		// Save the path

		ParticleEditorUtility::setParticleEffectPath(m_fileInfo.filePath());

		//Get the current particle effect iff

		Iff iff(1);
		m_particleEffectListViewer->write(iff);
		m_particleEffectListViewer->setParticleEffectFileName(FileNameUtils::get(m_fileInfo.filePath().latin1(), FileNameUtils::fileName | FileNameUtils::extension));

		// Write it to disk

		iff.write(m_fileInfo.filePath().latin1());
	}
	else if (!path.isEmpty())
	{
		QString text;
		text.sprintf("Error saving: \"%s\". Make sure the file is not read only.", FileNameUtils::get(path.latin1(), FileNameUtils::fileName | FileNameUtils::extension));
		QMessageBox::warning(this, "Error Saving File", text, "OK.");
	}
}

//-----------------------------------------------------------------------------
void MainWindow::fileExit()
{
	close();
}

//-----------------------------------------------------------------------------
void MainWindow::onBuildParticleEffect()
{
	// Write the intiial iff from the particle effect list viewer
	
	Iff iff(1);
	m_particleEffectListViewer->write(iff);
	setParticleEffectAppearanceTemplate(iff);
}

//-----------------------------------------------------------------------------
void MainWindow::keyReleaseEvent(QKeyEvent *keyEvent)
{
	keyEvent->ignore();
}

//-----------------------------------------------------------------------------
void MainWindow::onOptionsTerrainVisibleAction()
{
	if (optionsTerrainVisibleAction->isOn())
	{
		TerrainObject::getInstance()->addToWorld();
	}
	else
	{
		TerrainObject::getInstance()->removeFromWorld();
	}
}

//-----------------------------------------------------------------------------
void MainWindow::onOptionsParticleExtentsVisibleAction()
{
	ParticleManager::setDebugExtentsEnabled(optionsParticleExtentsVisibleAction->isOn());
}

//-----------------------------------------------------------------------------
void MainWindow::onOptionsParticleAxisVisibleAction()
{
	ParticleManager::setDebugAxisEnabled(optionsParticleAxisVisibleAction->isOn());
}

//-----------------------------------------------------------------------------
void MainWindow::onOptionsParticleTexturesVisibleAction()
{
	ParticleManager::setTexturingEnabled(optionsParticleTexturesVisibleAction->isOn());
}

//-----------------------------------------------------------------------------
void MainWindow::slotOptionsShowParticleOrientationAction()
{
	ParticleManager::setDebugParticleOrientationEnabled(optionsShowParticleOrientationAction->isOn());
}

//-----------------------------------------------------------------------------
void MainWindow::slotOptionsShowParticleVelocityAction()
{
	ParticleManager::setDebugVelocityEnabled(optionsShowParticleVelocityAction->isOn());
	ParticleManager::setDebugFlockingCageEnabled(optionsShowParticleVelocityAction->isOn());
}

//-----------------------------------------------------------------------------
void MainWindow::onOptionsShowDebugTextAction()
{
	ParticleManager::setDebugWorldTextEnabled(optionsShowDebugTextAction->isOn());
}

//-----------------------------------------------------------------------------
void MainWindow::restartParticleSystem()
{
	onBuildParticleEffect();
}

//-----------------------------------------------------------------------------
void MainWindow::setObjectTransform(Transform const &transform)
{
	if (m_ioWin)
	{
		m_ioWin->setObjectTransform(transform);
	}
}

//-----------------------------------------------------------------------------
void MainWindow::setEffectScale(float const effectScale)
{
	if (m_ioWin)
	{
		m_ioWin->setEffectScale(effectScale);
	}
}

//-----------------------------------------------------------------------------
void MainWindow::setPlayBackRate(float const playBackRate)
{
	if (m_ioWin)
	{
		m_ioWin->setPlayBackRate(playBackRate);
	}
}

//-----------------------------------------------------------------------------
void MainWindow::setObjectMovement(ParticleEffectTransformEdit::ObjectMovement const objectMovement)
{
	m_ioWin->setObjectMovement(objectMovement);
}

//-----------------------------------------------------------------------------
void MainWindow::setTimeOfDayCycle(bool const timeOfDayCycle)
{
	m_ioWin->setTimeOfDayCycle(timeOfDayCycle);
}

void MainWindow::setSpeed(float const speed)
{
	m_ioWin->setSpeed(speed);
}

void MainWindow::setSize(float const size)
{
	m_ioWin->setSize(size);
}

// ============================================================================
