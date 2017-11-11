// ============================================================================
//
// MainWindow.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstSwooshEditor.h"
#include "MainWindow.h"
#include "MainWindow.moc"

#include "clientAudio/Audio.h"
#include "clientGame/ClientCombatActionInfo.h"
#include "clientGame/Game.h"
#include "clientGame/GroundScene.h"
#include "clientObject/GameCamera.h"
#include "clientParticle/ParticleManager.h"
#include "clientParticle/SwooshAppearanceTemplate.h"
#include "SwooshEditorIoWin.h"
#include "QtUtility.h"
#include "sharedDebug/PerformanceTimer.h"
#include "sharedFile/FileNameUtils.h"
#include "sharedFile/Iff.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/ConfigSharedFoundation.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedTerrain/TerrainObject.h"
#include "sharedUtility/DataTableWriter.h"

// ============================================================================
//
// MainWindow
//
// ============================================================================

//-----------------------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent, char const *name)
 : BaseMainWindow(parent, name)
 , m_ioWin(NULL)
 , m_mousePosition()
 , m_color("white")
 , m_fileInfo()
 , m_updateTimer(NULL)
 , m_animationDataTable()
{
	m_fileInfo.setFile("");

	// Get rid of the status bar

	statusBar()->hide();

	// Create an IoWin

	m_ioWin = NON_NULL(new SwooshEditorIoWin(this));

	// Open the IOWin initially

	m_ioWin->open();

	// Load the child widget settings

	QtUtility::loadWidget(*this, 0, 0, 1024, 768);

	// Build the frames per second combo box

	{
		for (int i = 1; i <= 64; ++i)
		{
			char text[256];
			snprintf(text, sizeof(text), "%d", i);
			m_shaderFramesPerSecondComboBox->insertItem(text);
		}
	}

	// Widget connections

	connect(m_fileNewAction, SIGNAL(activated()), this, SLOT(slotFileNewAction()));
	connect(m_fileSaveAction, SIGNAL(activated()), this, SLOT(slotFileSaveAction()));
	connect(m_fileSaveAsAction, SIGNAL(activated()), this, SLOT(slotFileSaveAsAction()));
	connect(m_fileOpenAction, SIGNAL(activated()), this, SLOT(slotFileOpenAction()));
	connect(m_fileExitAction, SIGNAL(activated()), this, SLOT(slotFileExitAction()));

	connect(m_optionsDrawExtentsAction, SIGNAL(activated()), this, SLOT(slotOptionsDrawExtentsActionActivated()));
	connect(m_optionsShowQuadsAction, SIGNAL(activated()), this, SLOT(slotOptionsShowQuadsActionActivated()));

	connect(m_selectShaderPushButton, SIGNAL(pressed()), this, SLOT(slotSelectShaderPushButton()));
	connect(m_clearShaderPushButton, SIGNAL(pressed()), this, SLOT(slotClearShaderPushButton()));
	connect(m_shaderStretchDistanceLineEdit, SIGNAL(returnPressed()), this, SLOT(slotValidate()));
	connect(m_shaderFrameCountComboBox, SIGNAL(activated(int)), this, SLOT(slotValidate(int)));
	connect(m_shaderFramesPerSecondComboBox, SIGNAL(activated(int)), this, SLOT(slotValidate(int)));
	connect(m_shaderScrollSpeedLineEdit, SIGNAL(returnPressed()), this, SLOT(slotValidate()));

	connect(m_alphaComboBox, SIGNAL(activated(int)), this, SLOT(slotAlphaComboBoxChanged(int)));
	connect(m_alphaLineEdit, SIGNAL(returnPressed()), this, SLOT(slotAlphaLineEditReturnPressed()));
	connect(m_colorPushButton, SIGNAL(pressed()), SLOT(slotColorButtonPressed()));
	connect(m_selectStartAppearancePushButton, SIGNAL(pressed()), this, SLOT(slotSelectStartAppearancePushButton()));
	connect(m_clearStartAppearancePushButton, SIGNAL(pressed()), this, SLOT(slotClearStartAppearancePushButton()));
	connect(m_selectEndAppearancePushButton, SIGNAL(pressed()), this, SLOT(slotSelectEndAppearancePushButton()));
	connect(m_clearEndAppearancePushButton, SIGNAL(pressed()), this, SLOT(slotClearEndAppearancePushButton()));
	connect(m_selectSoundPushButton, SIGNAL(pressed()), this, SLOT(slotSelectSoundPushButton()));
	connect(m_clearSoundPushButton, SIGNAL(pressed()), this, SLOT(slotClearSoundPushButton()));
	connect(m_circlingSwooshSpeedComboBox, SIGNAL(activated(int)), this, SLOT(slotCirclingSwooshSpeedComboBoxActivated(int)));
	connect(m_fadeAlphaComboBox, SIGNAL(activated(int)), this, SLOT(slotValidate(int)));
	connect(m_taperGeometryComboBox, SIGNAL(activated(int)), this, SLOT(slotValidate(int)));
	connect(m_referenceSwooshComboBox, SIGNAL(activated(int)), this, SLOT(slotValidate(int)));
	connect(m_splineSubQuadsComboBox, SIGNAL(activated(int)), this, SLOT(slotValidate(int)));
	connect(m_widthLineEdit, SIGNAL(returnPressed()), this, SLOT(slotValidate()));
	connect(m_samplesPerSecondLineEdit, SIGNAL(returnPressed()), this, SLOT(slotValidate()));
	connect(m_samplePositionCountComboBox, SIGNAL(activated(int)), this, SLOT(slotValidate(int)));
	connect(m_multiplyColorByAlphaCheckBox, SIGNAL(clicked()), this, SLOT(slotValidate()));
	connect(m_debugAnimationComboBox, SIGNAL(activated(int)), this, SLOT(slotDebugAnimationComboBoxActivated(int)));
	connect(m_startAppearancePositionComboBox, SIGNAL(activated(int)), this, SLOT(slotValidate(int)));
	connect(m_endAppearancePositionComboBox, SIGNAL(activated(int)), this, SLOT(slotValidate(int)));
	connect(m_volumeSlider, SIGNAL(valueChanged(int)), this, SLOT(slotVolumeSliderValueChanged(int)));
	connect(m_fullVisibilitySpeedLineEdit, SIGNAL(returnPressed()), this, SLOT(slotFullVisibilitySpeedLineEditReturnPressed()));
	connect(m_noVisibilitySpeedLineEdit, SIGNAL(returnPressed()), this, SLOT(slotNoVisibilitySpeedLineEditReturnPressed()));
	connect(m_showFinalGameSwooshesCheckBox, SIGNAL(clicked()), this, SLOT(slotValidate()));
	connect(m_pauseAfterAnimationCheckBox, SIGNAL(clicked()), this, SLOT(slotPauseAfterAnimationCheckBoxClicked()));
	connect(m_defaultSamplesPerSecondPushButton, SIGNAL(pressed()), this, SLOT(slotDefaultSamplesPerSecondPushButtonPressed()));

	reset();

	// Set the default object rotation speed

	m_circlingSwooshSpeedComboBox->setCurrentItem(2);
	slotCirclingSwooshSpeedComboBoxActivated(0);
	m_pauseAfterAnimationCheckBox->setChecked(m_ioWin->isPauseAfterEachAnimation());

	m_updateTimer = new QTimer(this, "UpdateTimer");
	m_updateTimer->start(1000 / 40);
	connect(m_updateTimer, SIGNAL(timeout()), this, SLOT(slotUpdateTimerTimeOut()));

	// Set the volume

	int const volume = m_volumeSlider->minValue() + static_cast<int>(static_cast<float>(m_volumeSlider->maxValue() - m_volumeSlider->minValue() + 0.5f) * Audio::getMasterVolume());
	m_volumeSlider->setValue(volume);

	QtUtility::setLineEditFloat(m_fullVisibilitySpeedLineEdit, SwooshAppearanceTemplate::getFullVisibilitySpeed(), 0.00f, 64.0f, 4);
	QtUtility::setLineEditFloat(m_noVisibilitySpeedLineEdit, SwooshAppearanceTemplate::getNoVisibilitySpeed(), 0.00f, 64.0f, 4);

	loadAnimationDataTable();
}

//-----------------------------------------------------------------------------
MainWindow::~MainWindow()
{
	// Save the widget settings

	QtUtility::saveWidget(*this);
	
	if (m_ioWin)
	{
		m_ioWin->close();
		delete m_ioWin;
		m_ioWin = NULL;
	}
}

//-----------------------------------------------------------------------------
void MainWindow::slotFileNewAction()
{
	// Make sure the user wants to lose their changes

	int result = QMessageBox::warning(this, "Reset", "Resetting the swoosh effect will lose all current changes.", QMessageBox::Ok, QMessageBox::Cancel);

	if (result == QMessageBox::Ok)
	{
		reset();
	}
}

//-----------------------------------------------------------------------------
void MainWindow::slotFileOpenAction()
{
	// Get the file
	
	QString lastPath(FileNameUtils::get(QtUtility::getLastPath().latin1(), FileNameUtils::drive | FileNameUtils::directory).c_str());
	QString selectedFileName(QFileDialog::getOpenFileName(lastPath, "Swoosh files (*.swh)", this, "OpenFileDialog", "Open Swoosh Effect"));
	
	// Make sure the specified file exists on disk
	
	if (   !selectedFileName.isNull()
	    && FileNameUtils::isReadable(selectedFileName.latin1()))
	{
		// Make sure this is a valid particle effect Iff file
	
		if (QtUtility::isValidIffFile(this, selectedFileName.latin1(), SwooshAppearanceTemplate::getTag()))
		{
			reset();

			// Save the path
	
			m_fileInfo.setFile(selectedFileName);
			QtUtility::setLastPath(selectedFileName);
	
			// Get the current particle effect iff
	
			Iff iff(selectedFileName.latin1());
			loadIff(iff);

			validate();
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
void MainWindow::slotFileSaveAsAction()
{
	// Get the file
	
	QString saveFileName(QFileDialog::getSaveFileName(FileNameUtils::get(QtUtility::getLastPath().latin1(), FileNameUtils::drive | FileNameUtils::directory).c_str(), "Swoosh files (*.swh)", this, "SaveFileDialog", "Save Swoosh Effect"));
	
	if (!saveFileName.isNull())
	{
		if (saveFileName.isEmpty())
		{
			QMessageBox::warning(this, "Error Saving File", "Empty filename specified.", "OK.");
	
			// Offer them to choose a new filename
	
			slotFileSaveAsAction();
		}
		else
		{
			QString newPath((FileNameUtils::get(saveFileName.latin1(), FileNameUtils::drive | FileNameUtils::directory | FileNameUtils::fileName).c_str()) + QString(".swh"));

			if (!FileNameUtils::isWritable(newPath.latin1()))
			{
				QString text;
				text.sprintf("Error saving: \"%s\". Make sure the file is not read only.", FileNameUtils::get(newPath.latin1(), FileNameUtils::fileName | FileNameUtils::extension));
				QMessageBox::warning(this, "Error Saving File", text, "OK.");
		
				// Offer them to choose a new filename
		
				slotFileSaveAsAction();
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
void MainWindow::slotFileSaveAction()
{
	if (m_fileInfo.isFile())
	{
		save(m_fileInfo.filePath());
	}
	else
	{
		slotFileSaveAsAction();
	}
}

//-----------------------------------------------------------------------------
void MainWindow::save(QString const &path)
{
	QString newPath((FileNameUtils::get(path.latin1(), FileNameUtils::drive | FileNameUtils::directory | FileNameUtils::fileName).c_str()) + QString(".swh"));

	if (   !newPath.isNull()
	    && FileNameUtils::isWritable(newPath.latin1()))
	{
		m_fileInfo.setFile(newPath);
	
		// Save the path
	
		QtUtility::setLastPath(m_fileInfo.filePath());
	
		// Get the current particle effect iff
	
		Iff iff(1);

		writeSwooshIff(iff);

		// Write it to disk
	
		iff.write(m_fileInfo.filePath().latin1());
	}
	else if (!newPath.isEmpty())
	{
		QString text;
		text.sprintf("Error saving: \"%s\". Make sure the file is not read only.", FileNameUtils::get(newPath.latin1(), FileNameUtils::fileName | FileNameUtils::extension));
		QMessageBox::warning(this, "Error Saving File", text, "OK.");
	}
}

//-----------------------------------------------------------------------------
void MainWindow::slotFileExitAction()
{
	// Verify there are no changes that will be lost

	close();
}

//-----------------------------------------------------------------------------
void MainWindow::slotValidate(int)
{
	validate();
}

//-----------------------------------------------------------------------------
void MainWindow::slotValidate()
{
	validate();
}

//-----------------------------------------------------------------------------
void MainWindow::writeSwooshIff(Iff &iff)
{
	SwooshAppearanceTemplate swooshAppearanceTemplate;

	float const red = static_cast<float>(m_color.red()) / 255.0f;
	float const green = static_cast<float>(m_color.green()) / 255.0f;
	float const blue = static_cast<float>(m_color.blue()) / 255.0f;

	swooshAppearanceTemplate.setColor(red, green, blue);
	swooshAppearanceTemplate.setAlpha(QtUtility::getFloat(m_alphaLineEdit));
	swooshAppearanceTemplate.setWidth(QtUtility::getFloat(m_widthLineEdit));
	swooshAppearanceTemplate.setSamplesPerSecond(QtUtility::getFloat(m_samplesPerSecondLineEdit));

	switch (m_samplePositionCountComboBox->currentItem())
	{
		case 0: { swooshAppearanceTemplate.setSamplePositionCount(10); break; }
		case 1: { swooshAppearanceTemplate.setSamplePositionCount(20); break; }
		case 2: { swooshAppearanceTemplate.setSamplePositionCount(30); break; }
		case 3: { swooshAppearanceTemplate.setSamplePositionCount(40); break; }
		case 4: { swooshAppearanceTemplate.setSamplePositionCount(50); break; }
		case 5: { swooshAppearanceTemplate.setSamplePositionCount(60); break; }
		case 6: { swooshAppearanceTemplate.setSamplePositionCount(70); break; }
		case 7: { swooshAppearanceTemplate.setSamplePositionCount(80); break; }
		case 8: { swooshAppearanceTemplate.setSamplePositionCount(90); break; }
		case 9: { swooshAppearanceTemplate.setSamplePositionCount(100); break; }
		default: DEBUG_FATAL(true, ("Invalid index(%d)", m_samplePositionCountComboBox->currentItem()));
	}

	swooshAppearanceTemplate.setShaderStretchDistance(QtUtility::getFloat(m_shaderStretchDistanceLineEdit));
	swooshAppearanceTemplate.setShaderScrollSpeed(QtUtility::getFloat(m_shaderScrollSpeedLineEdit));
	
	std::string shaderPath(m_shaderLineEdit->text().latin1());
	int frameCount = 1;

	switch (m_shaderFrameCountComboBox->currentItem())
	{
		case 1:  { frameCount =  4; } break;
		case 2:  { frameCount = 16; } break;
		default: { DEBUG_FATAL((m_shaderFrameCountComboBox->currentItem() != 0), ("Invalid frame count specified.")); } break;
	}

	m_shaderFramesPerSecondComboBox->setEnabled(m_shaderFrameCountComboBox->currentItem() != 0);

	float const framesPerSecond = atof(static_cast<char const *>(m_shaderFramesPerSecondComboBox->currentText()));
	int const startFrame = 0;
	int const endFrame = frameCount - 1;

	swooshAppearanceTemplate.setShader(shaderPath.c_str(), frameCount, framesPerSecond, startFrame, endFrame);

	swooshAppearanceTemplate.setStartAppearancePath(m_startAppearanceLineEdit->text().latin1());
	swooshAppearanceTemplate.setEndAppearancePath(m_endAppearanceLineEdit->text().latin1());
	swooshAppearanceTemplate.setSoundPath(m_soundLineEdit->text().latin1());

	if (m_fadeAlphaComboBox->currentItem() < static_cast<int>(SwooshAppearanceTemplate::FA_count))
	{
		swooshAppearanceTemplate.setFadeAlpha(static_cast<SwooshAppearanceTemplate::FadeAlpha>(m_fadeAlphaComboBox->currentItem()));
	}
	else
	{
		DEBUG_FATAL(true, ("Invalid entry"));
	}

	if (m_taperGeometryComboBox->currentItem() < static_cast<int>(SwooshAppearanceTemplate::TG_count))
	{
		swooshAppearanceTemplate.setTaperGeometry(static_cast<SwooshAppearanceTemplate::TaperGeometry>(m_taperGeometryComboBox->currentItem()));
	}
	else
	{
		DEBUG_FATAL(true, ("Invalid entry"));
	}

	switch (m_splineSubQuadsComboBox->currentItem())
	{
		case 0: { swooshAppearanceTemplate.setSplineSubQuads(5); break; }
		case 1: { swooshAppearanceTemplate.setSplineSubQuads(4); break; }
		case 2: { swooshAppearanceTemplate.setSplineSubQuads(3); break; }
		case 3: { swooshAppearanceTemplate.setSplineSubQuads(2); break; }
		case 4: { swooshAppearanceTemplate.setSplineSubQuads(1); break; }
	}

	swooshAppearanceTemplate.setMultiplyColorByAlpha(m_multiplyColorByAlphaCheckBox->isChecked());
	swooshAppearanceTemplate.setStartAppearancePosition(static_cast<SwooshAppearanceTemplate::AppearancePosition>(m_startAppearancePositionComboBox->currentItem()));
	swooshAppearanceTemplate.setEndAppearancePosition(static_cast<SwooshAppearanceTemplate::AppearancePosition>(m_endAppearancePositionComboBox->currentItem()));

	swooshAppearanceTemplate.write(iff);

	{
		// Update the poly count display

		QString text("Spline Points Per Quad");
		text += " (";
		text += QString::number((swooshAppearanceTemplate.getSamplePositionCount() - 1) * swooshAppearanceTemplate.getSplineSubQuads() * 2);
		text += " polygons)";
		m_splinePointsPerQuadTextLabel->setText(text);
	}
}

//-----------------------------------------------------------------------------
void MainWindow::loadIff(Iff &iff)
{
	AppearanceTemplate const *appearanceTemplate = AppearanceTemplateList::fetch(&iff);

	m_ioWin->setAppearanceTemplate(appearanceTemplate);

	// Set all the widget values

	SwooshAppearanceTemplate const *swooshAppearanceTemplate = dynamic_cast<SwooshAppearanceTemplate const *>(appearanceTemplate);

	if (swooshAppearanceTemplate != NULL)
	{
		int const red = static_cast<int>(swooshAppearanceTemplate->getColor().r * 255.0f);
		int const green = static_cast<int>(swooshAppearanceTemplate->getColor().g * 255.0f);
		int const blue = static_cast<int>(swooshAppearanceTemplate->getColor().b * 255.0f);

		m_color.setRgb(red, green, blue);
		QtUtility::setLineEditFloat(m_alphaLineEdit, swooshAppearanceTemplate->getAlpha(), 0.0f, 1.0f, 2);
		QtUtility::setLineEditFloat(m_widthLineEdit, swooshAppearanceTemplate->getWidth(), 0.001f, 16.0f, 3);
		QtUtility::setLineEditFloat(m_samplesPerSecondLineEdit, swooshAppearanceTemplate->getSamplesPerSecond(), SwooshAppearanceTemplate::getMinSamplesPerSecond(), SwooshAppearanceTemplate::getMaxSamplesPerSecond(), 3);

		switch (swooshAppearanceTemplate->getSamplePositionCount())
		{
			case  10: { m_samplePositionCountComboBox->setCurrentItem(0); break; }
			case  20: { m_samplePositionCountComboBox->setCurrentItem(1); break; }
			case  30: { m_samplePositionCountComboBox->setCurrentItem(2); break; }
			case  40: { m_samplePositionCountComboBox->setCurrentItem(3); break; }
			case  50: { m_samplePositionCountComboBox->setCurrentItem(4); break; }
			case  60: { m_samplePositionCountComboBox->setCurrentItem(5); break; }
			case  70: { m_samplePositionCountComboBox->setCurrentItem(6); break; }
			case  80: { m_samplePositionCountComboBox->setCurrentItem(7); break; }
			case  90: { m_samplePositionCountComboBox->setCurrentItem(8); break; }
			case 100: { m_samplePositionCountComboBox->setCurrentItem(9); break; }
			default: { m_samplePositionCountComboBox->setCurrentItem(0); }
		}

		m_colorPushButton->setPaletteBackgroundColor(m_color);

		QtUtility::setLineEditFloat(m_shaderStretchDistanceLineEdit, swooshAppearanceTemplate->getShaderStretchDistance(), 0.01f, 128.0f, 2);
		QtUtility::setLineEditFloat(m_shaderScrollSpeedLineEdit, swooshAppearanceTemplate->getShaderScrollSpeed(), -256.0f, 256.0f, 2);
		
		m_shaderLineEdit->setText(swooshAppearanceTemplate->getParticleTexture().getShaderPath().getString());

		if      (swooshAppearanceTemplate->getParticleTexture().getFrameCount() == 1) { m_shaderFrameCountComboBox->setCurrentItem(0); }
		else if (swooshAppearanceTemplate->getParticleTexture().getFrameCount() == 4) { m_shaderFrameCountComboBox->setCurrentItem(1); }
		else                                                                     { m_shaderFrameCountComboBox->setCurrentItem(2); }

		int const framesPerSecond = static_cast<int>(swooshAppearanceTemplate->getParticleTexture().getFramesPerSecond());

		m_shaderFramesPerSecondComboBox->setCurrentItem(framesPerSecond - 1);

		m_startAppearanceLineEdit->setText(swooshAppearanceTemplate->getStartAppearancePath().c_str());
		m_endAppearanceLineEdit->setText(swooshAppearanceTemplate->getEndAppearancePath().c_str());
		m_soundLineEdit->setText(swooshAppearanceTemplate->getSoundPath().c_str());

		m_fadeAlphaComboBox->setCurrentItem(static_cast<int>(swooshAppearanceTemplate->getFadeAlpha()));
		m_taperGeometryComboBox->setCurrentItem(static_cast<int>(swooshAppearanceTemplate->getTaperGeometry()));

		switch (swooshAppearanceTemplate->getSplineSubQuads())
		{
			case 5: { m_splineSubQuadsComboBox->setCurrentItem(0); break; }
			case 4: { m_splineSubQuadsComboBox->setCurrentItem(1); break; }
			case 3: { m_splineSubQuadsComboBox->setCurrentItem(2); break; }
			case 2: { m_splineSubQuadsComboBox->setCurrentItem(3); break; }
			case 1: { m_splineSubQuadsComboBox->setCurrentItem(4); break; }
		}

		m_multiplyColorByAlphaCheckBox->setChecked(swooshAppearanceTemplate->isMultiplyColorByAlpha());

		m_startAppearancePositionComboBox->setCurrentItem(static_cast<int>(swooshAppearanceTemplate->getStartAppearancePosition()));
		m_endAppearancePositionComboBox->setCurrentItem(static_cast<int>(swooshAppearanceTemplate->getEndAppearancePosition()));
	}

	AppearanceTemplateList::release(appearanceTemplate);
}

//-----------------------------------------------------------------------------
void MainWindow::validate()
{
	QtUtility::validateLineEditFloat(m_alphaLineEdit, 0.01f, 1.0f, 2);
	QtUtility::validateLineEditFloat(m_widthLineEdit, 0.001f, 16.0f, 3);
	QtUtility::validateLineEditFloat(m_samplesPerSecondLineEdit, SwooshAppearanceTemplate::getMinSamplesPerSecond(), SwooshAppearanceTemplate::getMaxSamplesPerSecond(), 3);
	QtUtility::validateLineEditFloat(m_shaderStretchDistanceLineEdit, 0.01f, 128.0f, 2);
	QtUtility::validateLineEditFloat(m_shaderScrollSpeedLineEdit, -256.0f, 256.0f, 2);
	m_colorPushButton->setPaletteBackgroundColor(m_color);
	QtUtility::validateLineEditFloat(m_fullVisibilitySpeedLineEdit, 0.00f, 64.0f, 4);
	QtUtility::validateLineEditFloat(m_noVisibilitySpeedLineEdit, 0.00f, 64.0f, 4);

	SwooshAppearanceTemplate::setFullVisibilitySpeed(QtUtility::getFloat(m_fullVisibilitySpeedLineEdit));
	SwooshAppearanceTemplate::setNoVisibilitySpeed(QtUtility::getFloat(m_noVisibilitySpeedLineEdit));
#ifdef _DEBUG
	SwooshAppearanceTemplate::setUseFinalGameSwooshes(m_showFinalGameSwooshesCheckBox->isChecked());
	m_mainToolsFrame->setEnabled(!m_showFinalGameSwooshesCheckBox->isChecked());
#else
	m_showFinalGameSwooshesCheckBox->setChecked(false);
	m_mainToolsFrame->setEnabled(false);
#endif

	switch (m_referenceSwooshComboBox->currentItem())
	{
		case 0: { m_ioWin->setReferenceSwoosh(SwooshEditorIoWin::RS_none); break; }
		case 1: { m_ioWin->setReferenceSwoosh(SwooshEditorIoWin::RS_circling); break; }
		case 2: { m_ioWin->setReferenceSwoosh(SwooshEditorIoWin::RS_spiraling); break; }
		default: { DEBUG_FATAL(true, ("Invalid entry(%d)", m_referenceSwooshComboBox->currentItem())); }
	}

	Iff iff(2);

	writeSwooshIff(iff);

	loadIff(iff);
}

//-----------------------------------------------------------------------------
void MainWindow::slotAlphaLineEditReturnPressed()
{
	m_alphaComboBox->setCurrentItem(0);

	validate();
}

//-----------------------------------------------------------------------------
void MainWindow::reset()
{
	SwooshAppearanceTemplate swooshAppearanceTemplate;

	int const red = static_cast<int>(swooshAppearanceTemplate.getColor().r * 255.0f);
	int const green = static_cast<int>(swooshAppearanceTemplate.getColor().g * 255.0f);
	int const blue = static_cast<int>(swooshAppearanceTemplate.getColor().b * 255.0f);

	m_color = QColor(red, green, blue);
	m_alphaLineEdit->setText(QString::number(swooshAppearanceTemplate.getAlpha()));
	m_widthLineEdit->setText(QString::number(swooshAppearanceTemplate.getWidth()));
	m_samplesPerSecondLineEdit->setText(QString::number(swooshAppearanceTemplate.getSamplesPerSecond()));
	m_samplePositionCountComboBox->setCurrentItem(0);
	m_splineSubQuadsComboBox->setCurrentItem(0);
	m_alphaComboBox->setCurrentItem(0);
	m_alphaLineEdit->setText(QString::number(swooshAppearanceTemplate.getAlpha()));
	m_fadeAlphaComboBox->setCurrentItem(static_cast<int>(swooshAppearanceTemplate.getFadeAlpha()));
	m_multiplyColorByAlphaCheckBox->setChecked(false);
	m_shaderStretchDistanceLineEdit->setText(QString::number(swooshAppearanceTemplate.getShaderStretchDistance()));
	m_shaderScrollSpeedLineEdit->setText(QString::number(swooshAppearanceTemplate.getShaderScrollSpeed()));
	m_shaderLineEdit->setText("");
	m_shaderFrameCountComboBox->setCurrentItem(0);
	m_shaderFramesPerSecondComboBox->setCurrentItem(0);
	m_startAppearanceLineEdit->setText("");
	m_endAppearanceLineEdit->setText("");
	m_soundLineEdit->setText("");
	m_startAppearancePositionComboBox->setCurrentItem(static_cast<int>(swooshAppearanceTemplate.getStartAppearancePosition()));
	m_endAppearancePositionComboBox->setCurrentItem(static_cast<int>(swooshAppearanceTemplate.getEndAppearancePosition()));
	m_fullVisibilitySpeedLineEdit->setText(QString::number(swooshAppearanceTemplate.getFullVisibilitySpeed()));
	m_noVisibilitySpeedLineEdit->setText(QString::number(swooshAppearanceTemplate.getNoVisibilitySpeed()));

	m_fileInfo.setFile("");

	validate();
}

//-----------------------------------------------------------------------------
void MainWindow::paintEvent(QPaintEvent *paintEvent)
{
	BaseMainWindow::paintEvent(paintEvent);

	// Set the caption

	std::string path(FileNameUtils::get(m_fileInfo.filePath().latin1(), FileNameUtils::fileName | FileNameUtils::extension));

	char text[256];
	sprintf(text, "SWG Swoosh Editor (%s - %s) : %s", __DATE__, __TIME__, (m_fileInfo.fileName() == "") ? "Default" : path.c_str());
	setCaption(text);
}

//-----------------------------------------------------------------------------
void MainWindow::slotSelectShaderPushButton()
{
	// Get the last valid path

	QSettings settings;
	settings.insertSearchPath(QSettings::Windows, QtUtility::getSearchPath());
	QString result(settings.readEntry("ShaderPath", "c://"));

	QString shaderPath(QFileDialog::getOpenFileName(result, "Shader files (*.sht)", this, "ShaderFileDialog", "Open Shader"));

	if (!shaderPath.isNull())
	{
		// Save the path

		settings.writeEntry("ShaderPath", static_cast<char const *>(shaderPath));

		std::string path(shaderPath.latin1());
		FileNameUtils::swapChar(path, '/', '\\');
		std::string strippedPath;

		bool result = TreeFile::stripTreeFileSearchPathFromFile(path, strippedPath);

		if (!result)
		{
			strippedPath = path;

			IGNORE_RETURN(QMessageBox::information(this, "Warning", "The shader path does not appear to be tree file relative, this may cause the shader to appear as the default texture on some systems, please proceed with caution.", QMessageBox::Ok));
		}

		m_shaderLineEdit->setText(strippedPath.c_str());

		validate();
	}
}

//-----------------------------------------------------------------------------
void MainWindow::slotClearShaderPushButton()
{
	m_shaderLineEdit->setText("");
	DEBUG_REPORT_LOG(true, ("Shader %s\n", m_shaderLineEdit->text().latin1()));

	validate();
}

//-----------------------------------------------------------------------------
void MainWindow::slotSelectStartAppearancePushButton()
{
	selectAppearance(m_startAppearanceLineEdit);
}

//-----------------------------------------------------------------------------
void MainWindow::slotSelectEndAppearancePushButton()
{
	selectAppearance(m_endAppearanceLineEdit);
}

//-----------------------------------------------------------------------------
void MainWindow::selectAppearance(QLineEdit *lineEdit)
{
	// Get the last valid path

	QSettings settings;
	settings.insertSearchPath(QSettings::Windows, QtUtility::getSearchPath());
	QString result(settings.readEntry("AppearancePath", "c://"));

	QString appearancePath(QFileDialog::getOpenFileName(result, "Appearance files (*.apt *.prt *.msh *.sat)", this, "AppearanceFileDialog", "Open Appearance"));

	if (!appearancePath.isNull())
	{
		// Save the path

		settings.writeEntry("AppearancePath", static_cast<char const *>(appearancePath));

		// Save the shader path

		std::string path(appearancePath.latin1());
		FileNameUtils::swapChar(path, '/', '\\');
		std::string strippedPath;

		bool result = TreeFile::stripTreeFileSearchPathFromFile(path, strippedPath);

		if (!result)
		{
			strippedPath = path;

			IGNORE_RETURN(QMessageBox::information(this, "Warning", "The appearance path does not appear to be tree file relative, this may cause the default appearance to be visible on some systems, please proceed with caution.", QMessageBox::Ok));
		}

		lineEdit->setText(strippedPath.c_str());

		validate();
	}
}

//-----------------------------------------------------------------------------
void MainWindow::slotClearStartAppearancePushButton()
{
	m_startAppearanceLineEdit->setText("");

	validate();
}

//-----------------------------------------------------------------------------
void MainWindow::slotClearEndAppearancePushButton()
{
	m_endAppearanceLineEdit->setText("");

	validate();
}

//-----------------------------------------------------------------------------
void MainWindow::slotSelectSoundPushButton()
{
	// Get the last valid path

	QSettings settings;
	settings.insertSearchPath(QSettings::Windows, QtUtility::getSearchPath());
	QString result(settings.readEntry("SoundPath", "c://"));

	QString soundPath(QFileDialog::getOpenFileName(result, "Sound Template files (*.snd)", this, "SoundTemplateFileDialog", "Open Sound Template"));

	if (!soundPath.isNull())
	{
		// Save the path

		settings.writeEntry("SoundPath", static_cast<char const *>(soundPath));

		std::string path(soundPath.latin1());
		FileNameUtils::swapChar(path, '/', '\\');
		std::string strippedPath;

		bool result = TreeFile::stripTreeFileSearchPathFromFile(path, strippedPath);

		if (!result)
		{
			strippedPath = path;

			IGNORE_RETURN(QMessageBox::information(this, "Warning", "The sound template path does not appear to be tree file relative, this may cause the sound to not play on some systems, please proceed with caution.", QMessageBox::Ok));
		}

		m_soundLineEdit->setText(strippedPath.c_str());

		validate();
	}
}

//-----------------------------------------------------------------------------
void MainWindow::slotClearSoundPushButton()
{
	m_soundLineEdit->setText("");

	validate();
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
void MainWindow::slotColorButtonPressed()
{
	QColor resultColor(QColorDialog::getColor(m_color, this, "QColorDialog"));

	if (resultColor.isValid())
	{
		m_color = resultColor;
	}

	validate();
}

//-----------------------------------------------------------------------------
void MainWindow::slotAlphaComboBoxChanged(int)
{
	switch (m_alphaComboBox->currentItem())
	{
		case  0: break;
		case  1: { m_alphaLineEdit->setText("1.0"); } break;
		case  2: { m_alphaLineEdit->setText("0.9"); } break;
		case  3: { m_alphaLineEdit->setText("0.8"); } break;
		case  4: { m_alphaLineEdit->setText("0.7"); } break;
		case  5: { m_alphaLineEdit->setText("0.6"); } break;
		case  6: { m_alphaLineEdit->setText("0.5"); } break;
		case  7: { m_alphaLineEdit->setText("0.4"); } break;
		case  8: { m_alphaLineEdit->setText("0.3"); } break;
		case  9: { m_alphaLineEdit->setText("0.2"); } break;
		case 10: { m_alphaLineEdit->setText("0.1"); } break;
		default: { DEBUG_FATAL(true, ("Invalid value selected.")); } break;
	}

	validate();
}

//-----------------------------------------------------------------------------
void MainWindow::slotCirclingSwooshSpeedComboBoxActivated(int)
{
	switch (m_circlingSwooshSpeedComboBox->currentItem())
	{
		case 0:  { m_ioWin->setObjectRotationSpeed(4.0f); } break;
		case 1:  { m_ioWin->setObjectRotationSpeed(2.0f); } break;
		case 2:  { m_ioWin->setObjectRotationSpeed(1.0f); } break;
		case 3:  { m_ioWin->setObjectRotationSpeed(0.5f); } break;
		default: { DEBUG_FATAL(true, ("Invalid value selected.")); } break;
	}

	validate();
}

//-----------------------------------------------------------------------------
void MainWindow::slotDebugAnimationComboBoxActivated(int)
{
	int const row = m_debugAnimationComboBox->currentItem();
	std::string animationName(m_animationDataTable.getStringValue("AnimationName", row));
	std::string weaponName(m_animationDataTable.getStringValue("WeaponName", row));
	int trailFlags = 0;
	
	trailFlags |= (m_animationDataTable.getIntValue("LeftHandTrail", row) != 0) ? ClientCombatActionInfo::TL_lhand : 0;
	trailFlags |= (m_animationDataTable.getIntValue("RightHandTrail", row) != 0) ? ClientCombatActionInfo::TL_rhand : 0;
	trailFlags |= (m_animationDataTable.getIntValue("LeftFootTrail", row) != 0) ? ClientCombatActionInfo::TL_lfoot : 0;
	trailFlags |= (m_animationDataTable.getIntValue("RightFootTrail", row) != 0) ? ClientCombatActionInfo::TL_rfoot : 0;
	trailFlags |= (m_animationDataTable.getIntValue("WeaponTrail", row) != 0) ? ClientCombatActionInfo::TL_weapon : 0;

	float const defenderOffset = m_animationDataTable.getFloatValue("DefenderOffset", row);

	m_ioWin->setAnimation(animationName, weaponName, trailFlags, defenderOffset);
}

//-----------------------------------------------------------------------------
void MainWindow::slotOptionsDrawExtentsActionActivated()
{
	ParticleManager::setDebugExtentsEnabled(m_optionsDrawExtentsAction->isOn());
}

//-----------------------------------------------------------------------------
void MainWindow::slotOptionsShowQuadsActionActivated()
{
	ParticleManager::setDebugShowSwooshQuadsEnabled(m_optionsShowQuadsAction->isOn());
}

//-----------------------------------------------------------------------------
void MainWindow::slotVolumeSliderValueChanged(int)
{
	float const volume = static_cast<float>(m_volumeSlider->value()) / static_cast<float>(m_volumeSlider->maxValue() - m_volumeSlider->minValue());

	Audio::setMasterVolume(volume);
}

//-----------------------------------------------------------------------------
void MainWindow::loadAnimationDataTable()
{
	char const *fileName = "../../exe/win32/SwooshEditor.tab";

	if (FileNameUtils::isReadable(fileName))
	{
		// Get the animation data

		DataTableWriter dataTableWriter;
		dataTableWriter.loadFromSpreadsheet(fileName);
		Iff iff(2);
		dataTableWriter.save(iff);

		iff.allowNonlinearFunctions();
		iff.goToTopOfForm();

		m_animationDataTable.load(iff);

		if (m_animationDataTable.getNumRows() > 0)
		{
			m_debugAnimationComboBox->clear();
		}

		// Populate the animation combo box

		for (int row = 0; row < m_animationDataTable.getNumRows(); ++row)
		{
			m_debugAnimationComboBox->insertItem(m_animationDataTable.getStringValue("Name", row));
		}

		if (m_debugAnimationComboBox->count() > 0)
		{
			m_debugAnimationComboBox->setCurrentItem(0);
			slotDebugAnimationComboBoxActivated(0);
		}
	}
}

//-----------------------------------------------------------------------------
void MainWindow::slotFullVisibilitySpeedLineEditReturnPressed()
{
	if (QtUtility::getFloat(m_fullVisibilitySpeedLineEdit) < QtUtility::getFloat(m_noVisibilitySpeedLineEdit))
	{
		m_noVisibilitySpeedLineEdit->setText(m_fullVisibilitySpeedLineEdit->text());
	}

	validate();
}

//-----------------------------------------------------------------------------
void MainWindow::slotNoVisibilitySpeedLineEditReturnPressed()
{
	if (QtUtility::getFloat(m_noVisibilitySpeedLineEdit) > QtUtility::getFloat(m_fullVisibilitySpeedLineEdit))
	{
		m_fullVisibilitySpeedLineEdit->setText(m_noVisibilitySpeedLineEdit->text());
	}

	validate();
}

//-----------------------------------------------------------------------------
void MainWindow::slotPauseAfterAnimationCheckBoxClicked()
{
	m_ioWin->setPauseAfterEachAnimation(m_pauseAfterAnimationCheckBox->isChecked());
}

//-----------------------------------------------------------------------------
void MainWindow::slotDefaultSamplesPerSecondPushButtonPressed()
{
	m_samplesPerSecondLineEdit->setText(QString::number(30));

	validate();
}

// ============================================================================
