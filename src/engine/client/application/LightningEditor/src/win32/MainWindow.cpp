// ============================================================================
//
// MainWindow.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstLightningEditor.h"
#include "MainWindow.h"
#include "MainWindow.moc"

#include "clientGame/Game.h"
#include "clientGame/GroundScene.h"
#include "clientObject/GameCamera.h"
#include "clientParticle/LightningAppearanceTemplate.h"
#include "LightningEditorIoWin.h"
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
 , m_largeBoltColor("white")
 , m_smallBoltColor("white")
 , m_fileInfo()
 , m_updateTimer(NULL)
{
	m_fileInfo.setFile("");

	// Get rid of the status bar

	statusBar()->hide();

	// Create an IoWin

	m_ioWin = NON_NULL(new LightningEditorIoWin(this));

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

	{
		for (int i = 0; i <= 95; i += 5)
		{
			char text[256];
			snprintf(text, sizeof(text), "%d %%", i);
			m_largeBoltStartUntilComboBox->insertItem(text);
			m_smallBoltStartUntilComboBox->insertItem(text);
		}
	}

	{
		for (int i = 5; i <= 100; i += 5)
		{
			char text[256];
			snprintf(text, sizeof(text), "%d %%", i);
			m_largeBoltEndFromComboBox->insertItem(text);
			m_smallBoltEndFromComboBox->insertItem(text);
		}
	}

	// Widget connections

	connect(m_fileNewAction, SIGNAL(activated()), this, SLOT(slotFileNewAction()));
	connect(m_fileSaveAction, SIGNAL(activated()), this, SLOT(slotFileSaveAction()));
	connect(m_fileSaveAsAction, SIGNAL(activated()), this, SLOT(slotFileSaveAsAction()));
	connect(m_fileOpenAction, SIGNAL(activated()), this, SLOT(slotFileOpenAction()));
	connect(m_fileExitAction, SIGNAL(activated()), this, SLOT(slotFileExitAction()));

	connect(m_selectShaderPushButton, SIGNAL(pressed()), this, SLOT(slotSelectShaderPushButton()));
	connect(m_clearShaderPushButton, SIGNAL(pressed()), this, SLOT(slotClearShaderPushButton()));
	connect(m_shaderFrameCountComboBox, SIGNAL(activated(int)), this, SLOT(slotValidate(int)));
	connect(m_shaderFramesPerSecondComboBox, SIGNAL(activated(int)), this, SLOT(slotValidate(int)));

	connect(m_largeBoltAlphaComboBox, SIGNAL(activated(int)), this, SLOT(slotLargeBoltAlphaComboBoxChanged(int)));
	connect(m_largeBoltAlphaLineEdit, SIGNAL(returnPressed()), this, SLOT(slotLargeBoltAlphaLineEditReturnPressed()));
	connect(m_largeBoltColorPushButton, SIGNAL(pressed()), SLOT(slotLargeBoltColorButtonPressed()));
	connect(m_largeBoltChaosComboBox, SIGNAL(activated(int)), this, SLOT(slotLargeBoltChaosComboBoxChanged(int)));
	connect(m_largeBoltChaosLineEdit, SIGNAL(returnPressed()), this, SLOT(slotLargeBoltChaosLineEditReturnPressed()));
	connect(m_largeBoltThicknessLineEdit, SIGNAL(returnPressed()), this, SLOT(slotValidate()));
	connect(m_largeBoltBaseAmplitudeLineEdit, SIGNAL(returnPressed()), this, SLOT(slotValidate()));
	connect(m_largeBoltStartAmplitudeLineEdit, SIGNAL(returnPressed()), this, SLOT(slotValidate()));
	connect(m_largeBoltStartUntilLineEdit, SIGNAL(returnPressed()), this, SLOT(slotLargeBoltStartUntilLineEditReturnPressed()));
	connect(m_largeBoltStartUntilComboBox, SIGNAL(activated(int)), this, SLOT(slotLargeBoltStartUntilComboBoxActivated(int)));
	connect(m_largeBoltEndFromLineEdit, SIGNAL(returnPressed()), this, SLOT(slotLargeBoltEndFromLineEditReturnPressed()));
	connect(m_largeBoltEndFromComboBox, SIGNAL(activated(int)), this, SLOT(slotLargeBoltEndFromComboBoxActivated(int)));
	connect(m_largeBoltEndAmplitudeLineEdit, SIGNAL(returnPressed()), this, SLOT(slotValidate()));
	connect(m_largeBoltArcLineEdit, SIGNAL(returnPressed()), this, SLOT(slotLargeBoltArcLineEditReturnPressed()));
	connect(m_largeBoltArcComboBox, SIGNAL(activated(int)), this, SLOT(slotLargeBoltArcComboBoxChanged(int)));
	connect(m_largeBoltShaderStretchDistanceLineEdit, SIGNAL(returnPressed()), this, SLOT(slotValidate()));
	connect(m_largeBoltShaderScrollSpeedLineEdit, SIGNAL(returnPressed()), this, SLOT(slotValidate()));

	connect(m_smallBoltAlphaComboBox, SIGNAL(activated(int)), this, SLOT(slotSmallBoltAlphaComboBoxChanged(int)));
	connect(m_smallBoltAlphaLineEdit, SIGNAL(returnPressed()), this, SLOT(slotSmallBoltAlphaLineEditReturnPressed()));
	connect(m_smallBoltColorPushButton, SIGNAL(pressed()), SLOT(slotSmallBoltColorButtonPressed()));
	connect(m_smallBoltChaosComboBox, SIGNAL(activated(int)), this, SLOT(slotSmallBoltChaosComboBoxChanged(int)));
	connect(m_smallBoltChaosLineEdit, SIGNAL(returnPressed()), this, SLOT(slotSmallBoltChaosLineEditReturnPressed()));
	connect(m_smallBoltThicknessLineEdit, SIGNAL(returnPressed()), this, SLOT(slotValidate()));
	connect(m_smallBoltBaseAmplitudeLineEdit, SIGNAL(returnPressed()), this, SLOT(slotValidate()));
	connect(m_smallBoltStartAmplitudeLineEdit, SIGNAL(returnPressed()), this, SLOT(slotValidate()));
	connect(m_smallBoltStartUntilLineEdit, SIGNAL(returnPressed()), this, SLOT(slotSmallBoltStartUntilLineEditReturnPressed()));
	connect(m_smallBoltStartUntilComboBox, SIGNAL(activated(int)), this, SLOT(slotSmallBoltStartUntilComboBoxActivated(int)));
	connect(m_smallBoltEndFromLineEdit, SIGNAL(returnPressed()), this, SLOT(slotSmallBoltEndFromLineEditReturnPressed()));
	connect(m_smallBoltEndFromComboBox, SIGNAL(activated(int)), this, SLOT(slotSmallBoltEndFromComboBoxActivated(int)));
	connect(m_smallBoltEndAmplitudeLineEdit, SIGNAL(returnPressed()), this, SLOT(slotValidate()));
	connect(m_smallBoltArcLineEdit, SIGNAL(returnPressed()), this, SLOT(slotSmallBoltArcLineEditReturnPressed()));
	connect(m_smallBoltArcComboBox, SIGNAL(activated(int)), this, SLOT(slotSmallBoltArcComboBoxChanged(int)));
	connect(m_smallBoltShaderStretchDistanceLineEdit, SIGNAL(returnPressed()), this, SLOT(slotValidate()));
	connect(m_smallBoltShaderScrollSpeedLineEdit, SIGNAL(returnPressed()), this, SLOT(slotValidate()));

	connect(m_selectStartAppearancePushButton, SIGNAL(pressed()), this, SLOT(slotSelectStartAppearancePushButton()));
	connect(m_clearStartAppearancePushButton, SIGNAL(pressed()), this, SLOT(slotClearStartAppearancePushButton()));
	connect(m_selectEndAppearancePushButton, SIGNAL(pressed()), this, SLOT(slotSelectEndAppearancePushButton()));
	connect(m_clearEndAppearancePushButton, SIGNAL(pressed()), this, SLOT(slotClearEndAppearancePushButton()));
	connect(m_selectSoundPushButton, SIGNAL(pressed()), this, SLOT(slotSelectSoundPushButton()));
	connect(m_clearSoundPushButton, SIGNAL(pressed()), this, SLOT(slotClearSoundPushButton()));
	connect(m_startAppearanceComboBox, SIGNAL(activated(int)), this, SLOT(slotValidate(int)));
	connect(m_endAppearanceComboBox, SIGNAL(activated(int)), this, SLOT(slotValidate(int)));

	connect(m_largeBoltPercentComboBox, SIGNAL(activated(int)), this, SLOT(slotValidate(int)));
	connect(m_boltCountComboBox, SIGNAL(activated(int)), this, SLOT(slotValidate(int)));
	connect(m_moveEndPointsComboBox, SIGNAL(activated(int)), this, SLOT(slotMoveEndPointsComboBoxActivated(int)));
	connect(m_playPushButton, SIGNAL(pressed()), this, SLOT(slotPlayPushButtonPressed()));

	m_boltCountComboBox->setCurrentItem(1);
	m_moveEndPointsComboBox->setCurrentItem(0);

	reset();

	// Planet view timer

	m_updateTimer = new QTimer(this, "UpdateTimer");
	m_updateTimer->start(1000 / 30);
	connect(m_updateTimer, SIGNAL(timeout()), this, SLOT(slotUpdateTimerTimeOut()));
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

	int result = QMessageBox::warning(this, "Reset", "Resetting the lightning effect will lose all current changes.", QMessageBox::Ok, QMessageBox::Cancel);

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
	QString selectedFileName(QFileDialog::getOpenFileName(lastPath, "Lightning files (*.ltn)", this, "OpenFileDialog", "Open Lightning Effect"));
	
	// Make sure the specified file exists on disk
	
	if (   !selectedFileName.isNull()
	    && FileNameUtils::isReadable(selectedFileName.latin1()))
	{
		// Make sure this is a valid particle effect Iff file
	
		if (QtUtility::isValidIffFile(this, selectedFileName.latin1(), LightningAppearanceTemplate::getTag()))
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
	
	QString saveFileName(QFileDialog::getSaveFileName(FileNameUtils::get(QtUtility::getLastPath().latin1(), FileNameUtils::drive | FileNameUtils::directory).c_str(), "Lightning files (*.ltn)", this, "SaveFileDialog", "Save Lightning Effect"));
	
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
	QString newPath((FileNameUtils::get(path.latin1(), FileNameUtils::drive | FileNameUtils::directory | FileNameUtils::fileName).c_str()) + QString(".ltn"));

	if (   !newPath.isNull()
	    && FileNameUtils::isWritable(newPath.latin1()))
	{
		m_fileInfo.setFile(newPath);
	
		// Save the path
	
		QtUtility::setLastPath(m_fileInfo.filePath());
	
		// Get the current particle effect iff
	
		Iff iff(1);

		writeLightningIff(iff);

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
void MainWindow::slotLargeBoltAlphaComboBoxChanged(int)
{
	switch (m_largeBoltAlphaComboBox->currentItem())
	{
		case  0: break;
		case  1: { m_largeBoltAlphaLineEdit->setText("1.0"); } break;
		case  2: { m_largeBoltAlphaLineEdit->setText("0.9"); } break;
		case  3: { m_largeBoltAlphaLineEdit->setText("0.8"); } break;
		case  4: { m_largeBoltAlphaLineEdit->setText("0.7"); } break;
		case  5: { m_largeBoltAlphaLineEdit->setText("0.6"); } break;
		case  6: { m_largeBoltAlphaLineEdit->setText("0.5"); } break;
		case  7: { m_largeBoltAlphaLineEdit->setText("0.4"); } break;
		case  8: { m_largeBoltAlphaLineEdit->setText("0.3"); } break;
		case  9: { m_largeBoltAlphaLineEdit->setText("0.2"); } break;
		case 10: { m_largeBoltAlphaLineEdit->setText("0.1"); } break;
		default:
			{
				DEBUG_FATAL(true, ("Invalid value selected."));
			}
			break;
	}

	validate();
}

//-----------------------------------------------------------------------------
void MainWindow::slotSmallBoltAlphaComboBoxChanged(int)
{
	switch (m_smallBoltAlphaComboBox->currentItem())
	{
		case  0: break;
		case  1: { m_smallBoltAlphaLineEdit->setText("1.0"); } break;
		case  2: { m_smallBoltAlphaLineEdit->setText("0.9"); } break;
		case  3: { m_smallBoltAlphaLineEdit->setText("0.8"); } break;
		case  4: { m_smallBoltAlphaLineEdit->setText("0.7"); } break;
		case  5: { m_smallBoltAlphaLineEdit->setText("0.6"); } break;
		case  6: { m_smallBoltAlphaLineEdit->setText("0.5"); } break;
		case  7: { m_smallBoltAlphaLineEdit->setText("0.4"); } break;
		case  8: { m_smallBoltAlphaLineEdit->setText("0.3"); } break;
		case  9: { m_smallBoltAlphaLineEdit->setText("0.2"); } break;
		case 10: { m_smallBoltAlphaLineEdit->setText("0.1"); } break;
		default:
			{
				DEBUG_FATAL(true, ("Invalid value selected."));
			}
			break;
	}

	validate();
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
void MainWindow::slotLargeBoltColorButtonPressed()
{
	QColor resultColor(QColorDialog::getColor(m_largeBoltColor, this, "QColorDialog"));

	if (resultColor.isValid())
	{
		m_largeBoltColor = resultColor;
	}

	validate();
}

//-----------------------------------------------------------------------------
void MainWindow::slotSmallBoltColorButtonPressed()
{
	QColor resultColor(QColorDialog::getColor(m_smallBoltColor, this, "QColorDialog"));

	if (resultColor.isValid())
	{
		m_smallBoltColor = resultColor;
	}

	validate();
}

//-----------------------------------------------------------------------------
void MainWindow::slotLargeBoltChaosComboBoxChanged(int)
{
	switch (m_largeBoltChaosComboBox->currentItem())
	{
		case  0: break;
		case  1: { m_largeBoltChaosLineEdit->setText("0.25"); } break;
		case  2: { m_largeBoltChaosLineEdit->setText("0.5"); } break;
		case  3: { m_largeBoltChaosLineEdit->setText("1.0"); } break;
		case  4: { m_largeBoltChaosLineEdit->setText("2.0"); } break;
		case  5: { m_largeBoltChaosLineEdit->setText("4.0"); } break;
		default:
			{
				DEBUG_FATAL(true, ("Invalid value selected."));
			}
			break;
	}

	validate();
}

//-----------------------------------------------------------------------------
void MainWindow::slotSmallBoltChaosComboBoxChanged(int)
{
	switch (m_smallBoltChaosComboBox->currentItem())
	{
		case  0: break;
		case  1: { m_smallBoltChaosLineEdit->setText("0.25"); } break;
		case  2: { m_smallBoltChaosLineEdit->setText("0.5"); } break;
		case  3: { m_smallBoltChaosLineEdit->setText("1.0"); } break;
		case  4: { m_smallBoltChaosLineEdit->setText("2.0"); } break;
		case  5: { m_smallBoltChaosLineEdit->setText("4.0"); } break;
		default:
			{
				DEBUG_FATAL(true, ("Invalid value selected."));
			}
			break;
	}

	validate();
}

//-----------------------------------------------------------------------------
float MainWindow::getLargeBoltPercent()
{
	float result = 0.0f;

	switch (m_largeBoltPercentComboBox->currentItem())
	{
		case   0: { result = 1.0f; } break;
		case   1: { result = 0.8f; } break;
		case   2: { result = 0.6f; } break;
		case   3: { result = 0.4f; } break;
		case   4: { result = 0.2f; } break;
		case   5: { result = 0.0f; } break;
		default:
			{
				DEBUG_FATAL(true, ("Invalid value selected."));
			}
			break;
	}

	return result;
}

//-----------------------------------------------------------------------------
void MainWindow::writeLightningIff(Iff &iff)
{
	LightningAppearanceTemplate lightningAppearanceTemplate;

	{
		float const red = static_cast<float>(m_largeBoltColor.red()) / 255.0f;
		float const green = static_cast<float>(m_largeBoltColor.green()) / 255.0f;
		float const blue = static_cast<float>(m_largeBoltColor.blue()) / 255.0f;

		lightningAppearanceTemplate.setLargeBoltColor(red, green, blue);
		lightningAppearanceTemplate.setLargeBoltChaos(QtUtility::getFloat(m_largeBoltChaosLineEdit));
		lightningAppearanceTemplate.setLargeBoltAlpha(QtUtility::getFloat(m_largeBoltAlphaLineEdit));
		lightningAppearanceTemplate.setLargeBoltThickness(QtUtility::getFloat(m_largeBoltThicknessLineEdit));
		lightningAppearanceTemplate.setLargeBoltShaderStretchDistance(QtUtility::getFloat(m_largeBoltShaderStretchDistanceLineEdit));
		lightningAppearanceTemplate.setLargeBoltShaderScrollSpeed(QtUtility::getFloat(m_largeBoltShaderScrollSpeedLineEdit));
	}

	{
		float const red = static_cast<float>(m_smallBoltColor.red()) / 255.0f;
		float const green = static_cast<float>(m_smallBoltColor.green()) / 255.0f;
		float const blue = static_cast<float>(m_smallBoltColor.blue()) / 255.0f;

		lightningAppearanceTemplate.setSmallBoltColor(red, green, blue);
		lightningAppearanceTemplate.setSmallBoltChaos(QtUtility::getFloat(m_smallBoltChaosLineEdit));
		lightningAppearanceTemplate.setSmallBoltAlpha(QtUtility::getFloat(m_smallBoltAlphaLineEdit));
		lightningAppearanceTemplate.setSmallBoltThickness(QtUtility::getFloat(m_smallBoltThicknessLineEdit));
		lightningAppearanceTemplate.setSmallBoltShaderStretchDistance(QtUtility::getFloat(m_smallBoltShaderStretchDistanceLineEdit));
		lightningAppearanceTemplate.setSmallBoltShaderScrollSpeed(QtUtility::getFloat(m_smallBoltShaderScrollSpeedLineEdit));
	}

	lightningAppearanceTemplate.setLargeBoltPercent(getLargeBoltPercent());
	
	char const *shaderPath = m_shaderLineEdit->text().latin1();
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

	lightningAppearanceTemplate.setShader(shaderPath, frameCount, framesPerSecond, startFrame, endFrame);

	lightningAppearanceTemplate.setStartAppearancePath(m_startAppearanceLineEdit->text().latin1());
	lightningAppearanceTemplate.setEndAppearancePath(m_endAppearanceLineEdit->text().latin1());
	lightningAppearanceTemplate.setSoundPath(m_soundLineEdit->text().latin1());

	float const largeBoltBaseAmplitude = QtUtility::getFloat(m_largeBoltBaseAmplitudeLineEdit);
	float const largeBoltStartAmplitude = QtUtility::getFloat(m_largeBoltStartAmplitudeLineEdit);
	float const largeBoltStartUntil = QtUtility::getFloat(m_largeBoltStartUntilLineEdit);
	float const largeBoltEndFrom = QtUtility::getFloat(m_largeBoltEndFromLineEdit);
	float const largeBoltEndAmplitude = QtUtility::getFloat(m_largeBoltEndAmplitudeLineEdit);

	WaveForm largeBoltAmplitude;
	largeBoltAmplitude.insert(WaveFormControlPoint(0.0f, largeBoltStartAmplitude));
	largeBoltAmplitude.insert(WaveFormControlPoint(largeBoltStartUntil, largeBoltBaseAmplitude));
	largeBoltAmplitude.insert(WaveFormControlPoint(largeBoltEndFrom, largeBoltBaseAmplitude));
	largeBoltAmplitude.insert(WaveFormControlPoint(1.0f, largeBoltEndAmplitude));
	lightningAppearanceTemplate.setLargeBoltAmplitude(largeBoltAmplitude);
	lightningAppearanceTemplate.setLargeBoltArc(QtUtility::getFloat(m_largeBoltArcLineEdit));

	float const smallBoltBaseAmplitude = QtUtility::getFloat(m_smallBoltBaseAmplitudeLineEdit);
	float const smallBoltStartAmplitude = QtUtility::getFloat(m_smallBoltStartAmplitudeLineEdit);
	float const smallBoltStartUntil = QtUtility::getFloat(m_smallBoltStartUntilLineEdit);
	float const smallBoltEndFrom = QtUtility::getFloat(m_smallBoltEndFromLineEdit);
	float const smallBoltEndAmplitude = QtUtility::getFloat(m_smallBoltEndAmplitudeLineEdit);

	WaveForm smallBoltAmplitude;
	smallBoltAmplitude.insert(WaveFormControlPoint(0.0f, smallBoltStartAmplitude));
	smallBoltAmplitude.insert(WaveFormControlPoint(smallBoltStartUntil, smallBoltBaseAmplitude));
	smallBoltAmplitude.insert(WaveFormControlPoint(smallBoltEndFrom, smallBoltBaseAmplitude));
	smallBoltAmplitude.insert(WaveFormControlPoint(1.0f, smallBoltEndAmplitude));
	lightningAppearanceTemplate.setSmallBoltAmplitude(smallBoltAmplitude);
	lightningAppearanceTemplate.setSmallBoltArc(QtUtility::getFloat(m_smallBoltArcLineEdit));

	switch (m_startAppearanceComboBox->currentItem())
	{
		case 0: { lightningAppearanceTemplate.setStartAppearanceCount(LightningAppearanceTemplate::AC_single); } break;
		case 1: { lightningAppearanceTemplate.setStartAppearanceCount(LightningAppearanceTemplate::AC_onePerBolt); } break;
		default: { DEBUG_FATAL(true, ("Invalid idnex specified.")); } break;
	}

	switch (m_endAppearanceComboBox->currentItem())
	{
		case 0: { lightningAppearanceTemplate.setEndAppearanceCount(LightningAppearanceTemplate::AC_single); } break;
		case 1: { lightningAppearanceTemplate.setEndAppearanceCount(LightningAppearanceTemplate::AC_onePerBolt); } break;
		default: { DEBUG_FATAL(true, ("Invalid idnex specified.")); } break;
	}

	lightningAppearanceTemplate.write(iff);
}

//-----------------------------------------------------------------------------
void MainWindow::loadIff(Iff &iff)
{
	AppearanceTemplate const *appearanceTemplate = AppearanceTemplateList::fetch(&iff);

	int const boltCount = m_boltCountComboBox->currentItem() + 1;
	
	// Set the new appearance

	m_ioWin->setAppearanceTemplate(appearanceTemplate, boltCount);

	// Unpause if paused

	m_ioWin->setTimeScale(1.0f);
	m_playPushButton->setText("Pause");

	AppearanceTemplateList::release(appearanceTemplate);

	// Set all the widget values

	LightningAppearanceTemplate const *lightningAppearanceTemplate = dynamic_cast<LightningAppearanceTemplate const *>(appearanceTemplate);

	if (lightningAppearanceTemplate != NULL)
	{
		// Large Bolt

		{
			int const red = static_cast<int>(lightningAppearanceTemplate->getLargeBoltColor().r * 255.0f);
			int const green = static_cast<int>(lightningAppearanceTemplate->getLargeBoltColor().g * 255.0f);
			int const blue = static_cast<int>(lightningAppearanceTemplate->getLargeBoltColor().b * 255.0f);

			m_largeBoltColor.setRgb(red, green, blue);
			QtUtility::setLineEditFloat(m_largeBoltChaosLineEdit, lightningAppearanceTemplate->getLargeBoltChaos(), 0.0f, 32.0f, 2);
			QtUtility::setLineEditFloat(m_largeBoltThicknessLineEdit, lightningAppearanceTemplate->getLargeBoltThickness(), 0.01f, 16.0f, 2);
			QtUtility::setLineEditFloat(m_largeBoltAlphaLineEdit, lightningAppearanceTemplate->getLargeBoltAlpha(), 0.0f, 1.0f, 2);
			QtUtility::setLineEditFloat(m_largeBoltBaseAmplitudeLineEdit, lightningAppearanceTemplate->getLargeBoltBaseAmplitude(), 0.0f, 8.0f, 2);
			QtUtility::setLineEditFloat(m_largeBoltStartAmplitudeLineEdit, lightningAppearanceTemplate->getLargeBoltStartAmplitude(), 0.0f, 8.0f, 2);
			QtUtility::setLineEditFloat(m_largeBoltStartUntilLineEdit, lightningAppearanceTemplate->getLargeBoltStartUntil(), 0.0f, 1.0f, 2);
			QtUtility::setLineEditFloat(m_largeBoltEndFromLineEdit, lightningAppearanceTemplate->getLargeBoltEndFrom(), 0.0f, 1.0f, 2);
			QtUtility::setLineEditFloat(m_largeBoltEndAmplitudeLineEdit, lightningAppearanceTemplate->getLargeBoltEndAmplitude(), 0.0f, 8.0f, 2);
			QtUtility::setLineEditFloat(m_largeBoltArcLineEdit, lightningAppearanceTemplate->getLargeBoltArc(), 0.0f, 1.0f, 2);
			QtUtility::setLineEditFloat(m_largeBoltShaderStretchDistanceLineEdit, lightningAppearanceTemplate->getLargeBoltShaderStretchDistance(), 0.01f, 128.0f, 2);
			QtUtility::setLineEditFloat(m_largeBoltShaderScrollSpeedLineEdit, lightningAppearanceTemplate->getLargeBoltShaderScrollSpeed(), -256.0f, 256.0f, 2);
		}

		// Small Bolt

		{
			int const red = static_cast<int>(lightningAppearanceTemplate->getSmallBoltColor().r * 255.0f);
			int const green = static_cast<int>(lightningAppearanceTemplate->getSmallBoltColor().g * 255.0f);
			int const blue = static_cast<int>(lightningAppearanceTemplate->getSmallBoltColor().b * 255.0f);

			m_smallBoltColor.setRgb(red, green, blue);
			QtUtility::setLineEditFloat(m_smallBoltChaosLineEdit, lightningAppearanceTemplate->getSmallBoltChaos(), 0.0f, 32.0f, 2);
			QtUtility::setLineEditFloat(m_smallBoltThicknessLineEdit, lightningAppearanceTemplate->getSmallBoltThickness(), 0.01f, 16.0f, 2);
			QtUtility::setLineEditFloat(m_smallBoltAlphaLineEdit, lightningAppearanceTemplate->getSmallBoltAlpha(), 0.01f, 1.0f, 2);
			QtUtility::setLineEditFloat(m_smallBoltBaseAmplitudeLineEdit, lightningAppearanceTemplate->getSmallBoltBaseAmplitude(), 0.0f, 8.0f, 2);
			QtUtility::setLineEditFloat(m_smallBoltStartAmplitudeLineEdit, lightningAppearanceTemplate->getSmallBoltStartAmplitude(), 0.0f, 8.0f, 2);
			QtUtility::setLineEditFloat(m_smallBoltStartUntilLineEdit, lightningAppearanceTemplate->getSmallBoltStartUntil(), 0.0f, 1.0f, 2);
			QtUtility::setLineEditFloat(m_smallBoltEndFromLineEdit, lightningAppearanceTemplate->getSmallBoltEndFrom(), 0.0f, 1.0f, 2);
			QtUtility::setLineEditFloat(m_smallBoltEndAmplitudeLineEdit, lightningAppearanceTemplate->getSmallBoltEndAmplitude(), 0.0f, 8.0f, 2);
			QtUtility::setLineEditFloat(m_smallBoltArcLineEdit, lightningAppearanceTemplate->getSmallBoltArc(), 0.0f, 1.0f, 2);
			QtUtility::setLineEditFloat(m_smallBoltShaderStretchDistanceLineEdit, lightningAppearanceTemplate->getSmallBoltShaderStretchDistance(), 0.01f, 128.0f, 2);
			QtUtility::setLineEditFloat(m_smallBoltShaderScrollSpeedLineEdit, lightningAppearanceTemplate->getSmallBoltShaderScrollSpeed(), -256.0f, 256.0f, 2);
		}

		m_largeBoltColorPushButton->setPaletteBackgroundColor(m_largeBoltColor);
		m_smallBoltColorPushButton->setPaletteBackgroundColor(m_smallBoltColor);

		if      (lightningAppearanceTemplate->getLargeBoltPercent() < 0.2f) { m_largeBoltPercentComboBox->setCurrentItem(5); }
		else if (lightningAppearanceTemplate->getLargeBoltPercent() < 0.4f) { m_largeBoltPercentComboBox->setCurrentItem(4); }
		else if (lightningAppearanceTemplate->getLargeBoltPercent() < 0.6f) { m_largeBoltPercentComboBox->setCurrentItem(3); }
		else if (lightningAppearanceTemplate->getLargeBoltPercent() < 0.8f) { m_largeBoltPercentComboBox->setCurrentItem(2); }
		else if (lightningAppearanceTemplate->getLargeBoltPercent() < 1.0f) { m_largeBoltPercentComboBox->setCurrentItem(1); }
		else                                                                { m_largeBoltPercentComboBox->setCurrentItem(0); }

		m_shaderLineEdit->setText(lightningAppearanceTemplate->getParticleTexture().getShaderPath().getString());

		if      (lightningAppearanceTemplate->getParticleTexture().getFrameCount() == 1) { m_shaderFrameCountComboBox->setCurrentItem(0); }
		else if (lightningAppearanceTemplate->getParticleTexture().getFrameCount() == 4) { m_shaderFrameCountComboBox->setCurrentItem(1); }
		else                                                                     { m_shaderFrameCountComboBox->setCurrentItem(2); }

		int const framesPerSecond = static_cast<int>(lightningAppearanceTemplate->getParticleTexture().getFramesPerSecond());

		m_shaderFramesPerSecondComboBox->setCurrentItem(framesPerSecond - 1);

		m_startAppearanceLineEdit->setText(lightningAppearanceTemplate->getStartAppearancePath().c_str());
		m_endAppearanceLineEdit->setText(lightningAppearanceTemplate->getEndAppearancePath().c_str());
		m_soundLineEdit->setText(lightningAppearanceTemplate->getSoundPath().c_str());

		switch (lightningAppearanceTemplate->getStartAppearanceCount())
		{
			case LightningAppearanceTemplate::AC_single: { m_startAppearanceComboBox->setCurrentItem(0); } break;
			case LightningAppearanceTemplate::AC_onePerBolt: { m_startAppearanceComboBox->setCurrentItem(1); } break;
			default: { DEBUG_FATAL(true, ("Invalid index specified.")); } break;
		}

		switch (lightningAppearanceTemplate->getEndAppearanceCount())
		{
			case LightningAppearanceTemplate::AC_single: { m_endAppearanceComboBox->setCurrentItem(0); } break;
			case LightningAppearanceTemplate::AC_onePerBolt: { m_endAppearanceComboBox->setCurrentItem(1); } break;
			default: { DEBUG_FATAL(true, ("Invalid index specified.")); } break;
		}
	}
}

//-----------------------------------------------------------------------------
void MainWindow::validate()
{
	QtUtility::validateLineEditFloat(m_largeBoltThicknessLineEdit, 0.01f, 16.0f, 2);
	QtUtility::validateLineEditFloat(m_largeBoltAlphaLineEdit, 0.01f, 1.0f, 2);
	QtUtility::validateLineEditFloat(m_largeBoltChaosLineEdit, 0.0f, 32.0f, 2);
	QtUtility::validateLineEditFloat(m_largeBoltBaseAmplitudeLineEdit, 0.0f, 8.0f, 2);
	QtUtility::validateLineEditFloat(m_largeBoltStartAmplitudeLineEdit, 0.0f, 8.0f, 2);
	QtUtility::validateLineEditFloat(m_largeBoltStartUntilLineEdit, 0.0f, 1.0f, 2);
	QtUtility::validateLineEditFloat(m_largeBoltEndFromLineEdit, 0.0f, 1.0f, 2);
	QtUtility::validateLineEditFloat(m_largeBoltEndAmplitudeLineEdit, 0.0f, 8.0f, 2);
	QtUtility::validateLineEditFloat(m_largeBoltArcLineEdit, 0.0f, 1.0f, 2);
	QtUtility::validateLineEditFloat(m_largeBoltShaderStretchDistanceLineEdit, 0.01f, 128.0f, 2);
	QtUtility::validateLineEditFloat(m_largeBoltShaderScrollSpeedLineEdit, -256.0f, 256.0f, 2);
	QtUtility::validateLineEditFloat(m_smallBoltThicknessLineEdit, 0.01f, 16.0f, 2);
	QtUtility::validateLineEditFloat(m_smallBoltAlphaLineEdit, 0.01f, 1.0f, 2);
	QtUtility::validateLineEditFloat(m_smallBoltChaosLineEdit, 0.0f, 32.0f, 2);
	QtUtility::validateLineEditFloat(m_smallBoltBaseAmplitudeLineEdit, 0.0f, 8.0f, 2);
	QtUtility::validateLineEditFloat(m_smallBoltStartAmplitudeLineEdit, 0.0f, 8.0f, 2);
	QtUtility::validateLineEditFloat(m_smallBoltStartUntilLineEdit, 0.0f, 1.0f, 2);
	QtUtility::validateLineEditFloat(m_smallBoltEndFromLineEdit, 0.0f, 1.0f, 2);
	QtUtility::validateLineEditFloat(m_smallBoltEndAmplitudeLineEdit, 0.0f, 8.0f, 2);
	QtUtility::validateLineEditFloat(m_smallBoltArcLineEdit, 0.0f, 1.0f, 2);
	QtUtility::validateLineEditFloat(m_smallBoltShaderStretchDistanceLineEdit, 0.01f, 128.0f, 2);
	QtUtility::validateLineEditFloat(m_smallBoltShaderScrollSpeedLineEdit, -256.0f, 256.0f, 2);

	m_largeBoltColorPushButton->setPaletteBackgroundColor(m_largeBoltColor);
	m_smallBoltColorPushButton->setPaletteBackgroundColor(m_smallBoltColor);

	Iff iff(2);

	writeLightningIff(iff);

	loadIff(iff);
}

//-----------------------------------------------------------------------------
void MainWindow::slotLargeBoltAlphaLineEditReturnPressed()
{
	m_largeBoltAlphaComboBox->setCurrentItem(0);

	validate();
}

//-----------------------------------------------------------------------------
void MainWindow::slotLargeBoltChaosLineEditReturnPressed()
{
	m_largeBoltChaosComboBox->setCurrentItem(0);

	validate();
}

//-----------------------------------------------------------------------------
void MainWindow::slotSmallBoltAlphaLineEditReturnPressed()
{
	m_smallBoltAlphaComboBox->setCurrentItem(0);

	validate();
}

//-----------------------------------------------------------------------------
void MainWindow::slotSmallBoltChaosLineEditReturnPressed()
{
	m_smallBoltChaosComboBox->setCurrentItem(0);

	validate();
}

//-----------------------------------------------------------------------------
void MainWindow::reset()
{
	m_largeBoltColor = QColor("white");
	m_smallBoltColor = QColor("white");

	m_largeBoltPercentComboBox->setCurrentItem(3);

	m_largeBoltAlphaLineEdit->setText("1.0");
	m_largeBoltChaosLineEdit->setText("1.0");
	m_largeBoltThicknessLineEdit->setText("0.4");
	m_largeBoltBaseAmplitudeLineEdit->setText("1.0");
	m_largeBoltStartAmplitudeLineEdit->setText("0.0");
	m_largeBoltStartUntilLineEdit->setText("0.5");
	m_largeBoltEndFromLineEdit->setText("0.95");
	m_largeBoltEndAmplitudeLineEdit->setText("0.0");
	m_largeBoltArcLineEdit->setText("0.0");
	m_largeBoltShaderStretchDistanceLineEdit->setText("0.5");
	m_largeBoltShaderScrollSpeedLineEdit->setText("0.0");

	m_smallBoltAlphaLineEdit->setText("1.0");
	m_smallBoltChaosLineEdit->setText("1.0");
	m_smallBoltThicknessLineEdit->setText("0.15");
	m_smallBoltBaseAmplitudeLineEdit->setText("1.0");
	m_smallBoltStartAmplitudeLineEdit->setText("0.0");
	m_smallBoltStartUntilLineEdit->setText("0.5");
	m_smallBoltEndFromLineEdit->setText("0.95");
	m_smallBoltEndAmplitudeLineEdit->setText("0.0");
	m_smallBoltArcLineEdit->setText("0.0");
	m_smallBoltShaderStretchDistanceLineEdit->setText("0.5");
	m_smallBoltShaderScrollSpeedLineEdit->setText("0.0");

	m_shaderLineEdit->setText("shader/pt_force_lightning_anim.sht");
	m_shaderFrameCountComboBox->setCurrentItem(1);
	m_shaderFramesPerSecondComboBox->setCurrentItem(19);

	m_startAppearanceLineEdit->setText("");
	m_endAppearanceLineEdit->setText("");
	m_soundLineEdit->setText("");
	m_startAppearanceComboBox->setCurrentItem(0);
	m_endAppearanceComboBox->setCurrentItem(0);

	m_largeBoltAlphaComboBox->setCurrentItem(0);
	m_largeBoltChaosComboBox->setCurrentItem(0);
	m_largeBoltStartUntilComboBox->setCurrentItem(0);
	m_largeBoltEndFromComboBox->setCurrentItem(0);
	m_largeBoltArcComboBox->setCurrentItem(0);

	m_smallBoltAlphaComboBox->setCurrentItem(0);
	m_smallBoltChaosComboBox->setCurrentItem(0);
	m_smallBoltStartUntilComboBox->setCurrentItem(0);
	m_smallBoltEndFromComboBox->setCurrentItem(0);
	m_smallBoltArcComboBox->setCurrentItem(0);

	m_fileInfo.setFile("");

	validate();
}

//-----------------------------------------------------------------------------
void MainWindow::paintEvent(QPaintEvent *paintEvent)
{
	// Set the caption

	std::string path(FileNameUtils::get(m_fileInfo.filePath().latin1(), FileNameUtils::fileName | FileNameUtils::extension));

	char text[256];
	sprintf(text, "SWG Lightning Editor (%s - %s) : %s", __DATE__, __TIME__, (m_fileInfo.filePath() == "") ? "Default" : path.c_str());
	setCaption(text);

	BaseMainWindow::paintEvent(paintEvent);
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

	validate();
}

//-----------------------------------------------------------------------------
void MainWindow::slotSelectStartAppearancePushButton()
{
	// Get the last valid path

	QSettings settings;
	settings.insertSearchPath(QSettings::Windows, QtUtility::getSearchPath());
	QString result(settings.readEntry("AppearancePath", "c://"));

	QString appearancePath(QFileDialog::getOpenFileName(result, "Appearance files (*.apt *.prt *.msh *.sat)", this, "StartAppearanceFileDialog", "Open Appearance"));

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

		m_startAppearanceLineEdit->setText(strippedPath.c_str());

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
void MainWindow::slotSelectEndAppearancePushButton()
{
	// Get the last valid path

	QSettings settings;
	settings.insertSearchPath(QSettings::Windows, QtUtility::getSearchPath());
	QString result(settings.readEntry("AppearancePath", "c://"));

	QString appearancePath(QFileDialog::getOpenFileName(result, "Appearance files (*.apt *.prt *.msh *.sat)", this, "EndAppearanceFileDialog", "Open Appearance"));

	if (!appearancePath.isNull())
	{
		// Save the path

		settings.writeEntry("AppearancePath", static_cast<char const *>(appearancePath));

		std::string path(appearancePath.latin1());
		FileNameUtils::swapChar(path, '/', '\\');
		std::string strippedPath;

		bool result = TreeFile::stripTreeFileSearchPathFromFile(path, strippedPath);

		if (!result)
		{
			strippedPath = path;

			IGNORE_RETURN(QMessageBox::information(this, "Warning", "The appearance path does not appear to be tree file relative, this may cause the default appearance to be visible on some systems, please proceed with caution.", QMessageBox::Ok));
		}

		m_endAppearanceLineEdit->setText(strippedPath.c_str());

		validate();
	}
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
void MainWindow::slotLargeBoltArcLineEditReturnPressed()
{
	m_largeBoltArcComboBox->setCurrentItem(0);

	validate();
}

//-----------------------------------------------------------------------------
void MainWindow::slotLargeBoltArcComboBoxChanged(int)
{
	switch (m_largeBoltArcComboBox->currentItem())
	{
		case 0: break;
		case 1: { m_largeBoltArcLineEdit->setText("0.0"); } break;
		case 2: { m_largeBoltArcLineEdit->setText("0.5"); } break;
		case 3: { m_largeBoltArcLineEdit->setText("1.0"); } break;
		default:
			{
				DEBUG_FATAL(true, ("Invalid value selected."));
			}
			break;
	}

	validate();
}

//-----------------------------------------------------------------------------
void MainWindow::slotLargeBoltStartUntilComboBoxActivated(int)
{
	if (m_largeBoltStartUntilComboBox->currentItem() > 0)
	{
		char text[256];
		snprintf(text, sizeof(text), "%.2f", static_cast<float>((m_largeBoltStartUntilComboBox->currentItem() - 1) * 5) / 100.0f);
		m_largeBoltStartUntilLineEdit->setText(text);

		validate();
	}
}

//-----------------------------------------------------------------------------
void MainWindow::slotLargeBoltEndFromComboBoxActivated(int)
{
	if (m_largeBoltEndFromComboBox->currentItem() > 0)
	{
		char text[256];
		snprintf(text, sizeof(text), "%.2f", static_cast<float>(5 + (m_largeBoltEndFromComboBox->currentItem() - 1) * 5) / 100.0f);
		m_largeBoltEndFromLineEdit->setText(text);

		validate();
	}
}

//-----------------------------------------------------------------------------
void MainWindow::slotLargeBoltStartUntilLineEditReturnPressed()
{
	m_largeBoltStartUntilComboBox->setCurrentItem(0);

	validate();
}

//-----------------------------------------------------------------------------
void MainWindow::slotLargeBoltEndFromLineEditReturnPressed()
{
	m_largeBoltEndFromComboBox->setCurrentItem(0);

	validate();
}

//-----------------------------------------------------------------------------
void MainWindow::slotMoveEndPointsComboBoxActivated(int)
{
	switch (m_moveEndPointsComboBox->currentItem())
	{
		case 0: { m_ioWin->setEndPoints(LightningEditorIoWin::EP_fixedPosition); } break;
		case 1: { m_ioWin->setEndPoints(LightningEditorIoWin::EP_contractExpand); } break;
		case 2: { m_ioWin->setEndPoints(LightningEditorIoWin::EP_spreadOut); } break;
		case 3: { m_ioWin->setEndPoints(LightningEditorIoWin::EP_rotate); } break;
		default:
			{
				DEBUG_FATAL(true, ("Invalid value selected."));
			}
			break;
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
void MainWindow::slotSmallBoltStartUntilLineEditReturnPressed()
{
	m_smallBoltStartUntilComboBox->setCurrentItem(0);

	validate();
}

//-----------------------------------------------------------------------------
void MainWindow::slotSmallBoltStartUntilComboBoxActivated(int)
{
	if (m_smallBoltStartUntilComboBox->currentItem() > 0)
	{
		char text[256];
		snprintf(text, sizeof(text), "%.2f", static_cast<float>((m_smallBoltStartUntilComboBox->currentItem() - 1) * 5) / 100.0f);
		m_smallBoltStartUntilLineEdit->setText(text);

		validate();
	}
}

//-----------------------------------------------------------------------------
void MainWindow::slotSmallBoltEndFromLineEditReturnPressed()
{
	m_smallBoltEndFromComboBox->setCurrentItem(0);

	validate();
}

//-----------------------------------------------------------------------------
void MainWindow::slotSmallBoltEndFromComboBoxActivated(int)
{
	if (m_smallBoltEndFromComboBox->currentItem() > 0)
	{
		char text[256];
		snprintf(text, sizeof(text), "%.2f", static_cast<float>(5 + (m_smallBoltEndFromComboBox->currentItem() - 1) * 5) / 100.0f);
		m_smallBoltEndFromLineEdit->setText(text);

		validate();
	}
}

//-----------------------------------------------------------------------------
void MainWindow::slotSmallBoltArcLineEditReturnPressed()
{
	m_smallBoltArcComboBox->setCurrentItem(0);

	validate();
}

//-----------------------------------------------------------------------------
void MainWindow::slotSmallBoltArcComboBoxChanged(int)
{
	switch (m_smallBoltArcComboBox->currentItem())
	{
		case 0: break;
		case 1: { m_smallBoltArcLineEdit->setText("0.0"); } break;
		case 2: { m_smallBoltArcLineEdit->setText("0.5"); } break;
		case 3: { m_smallBoltArcLineEdit->setText("1.0"); } break;
		default:
			{
				DEBUG_FATAL(true, ("Invalid value selected."));
			}
			break;
	}

	validate();
}

//-----------------------------------------------------------------------------
void MainWindow::slotPlayPushButtonPressed()
{
	if (m_ioWin != NULL)
	{
		if (m_ioWin->getTimeScale() > 0.0f)
		{
			m_ioWin->setTimeScale(0.0f);
			m_playPushButton->setText("Play");
		}
		else
		{
			m_ioWin->setTimeScale(1.0f);
			m_playPushButton->setText("Pause");
		}
	}
}

// ============================================================================
