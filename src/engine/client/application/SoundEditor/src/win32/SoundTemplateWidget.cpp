// ============================================================================
//
// SoundTemplateWidget.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstSoundEditor.h"
#include "SoundTemplateWidget.h"
#include "SoundTemplateWidget.moc"

#include "clientAudio/Audio.h"
#include "clientAudio/AudioSampleInformation.h"
#include "clientAudio/Sound2dTemplate.h"
#include "clientAudio/Sound3dTemplate.h"
#include "clientAudio/SoundTemplateList.h"
#include "sharedFile/FileNameUtils.h"
#include "sharedFile/Iff.h"
#include "sharedFile/TreeFile.h"
#include "sharedMath/Vector.h"

// ============================================================================
//
// SoundTemplateWidget
//
// ============================================================================

//-----------------------------------------------------------------------------
SoundTemplateWidgetListViewItem::SoundTemplateWidgetListViewItem(QListView *parent)
 : QListViewItem(parent)
 , m_sampleTime(0.0f)
 , m_sampleBits(0)
 , m_sampleChannels(0)
 , m_sampleCompressed(false)
 , m_samplePath("")
 , m_sampleRate(0)
 , m_sampleSize(0)
{
}

//-----------------------------------------------------------------------------
void SoundTemplateWidgetListViewItem::setSamplePath(std::string const &path)
{
	AudioSampleInformation sampleInformation(Audio::getSampleInformation(path));

	m_sampleTime = sampleInformation.m_time;
	m_sampleBits = sampleInformation.m_bits;
	m_sampleChannels = sampleInformation.m_channels;
	m_sampleFileType = sampleInformation.m_type;
	m_sampleCompressed = (m_sampleFileType != "Standard PCM wav file");
	m_samplePath = path;
	m_sampleRate = sampleInformation.m_rate;
	m_sampleSize = Audio::getSampleSize(path.c_str());
}

//-----------------------------------------------------------------------------
std::string const &SoundTemplateWidgetListViewItem::getSamplePath() const
{
	return m_samplePath;
}

//-----------------------------------------------------------------------------
std::string SoundTemplateWidgetListViewItem::getSampleTreeFilePath() const
{
	std::string treeFilePath;
	bool result = TreeFile::stripTreeFileSearchPathFromFile(m_samplePath, treeFilePath);

	if (!result)
	{
		treeFilePath = m_samplePath;
	}

	return treeFilePath;
}

//-----------------------------------------------------------------------------
std::string const &SoundTemplateWidgetListViewItem::getSampleFileType() const
{
	return m_sampleFileType;
}

//-----------------------------------------------------------------------------
bool SoundTemplateWidgetListViewItem::isSampleCompressed() const
{
	return m_sampleCompressed;
}

//-----------------------------------------------------------------------------
bool SoundTemplateWidgetListViewItem::isSampleStereo() const
{
	return (m_sampleChannels == 2);
}

//-----------------------------------------------------------------------------
int SoundTemplateWidgetListViewItem::getSampleChannels() const
{
	return m_sampleChannels;
}

//-----------------------------------------------------------------------------
int SoundTemplateWidgetListViewItem::getSampleBits() const
{
	return m_sampleBits;
}

//-----------------------------------------------------------------------------
int SoundTemplateWidgetListViewItem::getSampleRate() const
{
	return m_sampleRate;
}

//-----------------------------------------------------------------------------
int SoundTemplateWidgetListViewItem::getSampleSize() const
{
	return m_sampleSize;
}

//-----------------------------------------------------------------------------
float SoundTemplateWidgetListViewItem::getSampleTime() const
{
	return m_sampleTime;
}

// ============================================================================
//
// SoundTemplateWidget
//
// ============================================================================

SoundBoundFloat SoundTemplateWidget::m_startDelayBound(0.0f, 4096.0f);
SoundBoundFloat SoundTemplateWidget::m_fadeInBound(0, 4096.0);
SoundBoundInt   SoundTemplateWidget::m_loopCountBound(1, 4096);
SoundBoundFloat SoundTemplateWidget::m_loopDelayBound(0.0f, 4096.0f);
SoundBoundFloat SoundTemplateWidget::m_fadeOutBound(0, 4096.0);
SoundBoundFloat SoundTemplateWidget::m_volumeSampleRateBound(0, 4096.0);
SoundBoundInt   SoundTemplateWidget::m_volumeBound(0, 100);
SoundBoundFloat SoundTemplateWidget::m_volumeInterpolationRateBound(0, 4096.0);
SoundBoundFloat SoundTemplateWidget::m_pitchSampleRateBound(0, 4096.0);
SoundBoundFloat SoundTemplateWidget::m_pitchBound(-16.0f, 16.0f);
SoundBoundFloat SoundTemplateWidget::m_pitchInterpolationRateBound(0, 4096.0);
SoundBoundFloat SoundTemplateWidget::m_distanceAtMaxVolumeBound(0.0f, 256.0f);
int const       SoundTemplateWidget::m_floatingPointPrecision = 2;

//-----------------------------------------------------------------------------
SoundTemplateWidget::SoundTemplateWidget(QWidget *parent, char const *name)
 : BaseSoundTemplateWidget(parent, name)
 , m_selectedListViewItem(NULL)
 , m_soundId(0, "")
 , m_sampleListPlayPopupMenu(NULL)
 , m_soundTemplateFileName("")
 , m_currentSound2dTemplate()
 , m_cachedSoundTemplate(NULL)
{
	// Set the caption

	setCaption("Default Template");

	// Save the parent

	m_workSpaceParent = dynamic_cast<QWorkspace *>(parent);

	// Setup the sample list view

	m_sampleListView->setSorting(-1);
	m_sampleListView->setSelectionMode(QListView::Extended);
	m_sampleListView->setAllColumnsShowFocus(true);
	m_sampleListView->setItemMargin(1);

	// Set the column alignment in the SampleList

	m_sampleListView->setColumnAlignment(LVC_size, Qt::AlignRight);
	m_sampleListView->setColumnAlignment(LVC_time, Qt::AlignRight);
	m_sampleListView->setColumnAlignment(LVC_type, Qt::AlignRight);
	m_sampleListView->setColumnAlignment(LVC_bits, Qt::AlignRight);
	m_sampleListView->setColumnAlignment(LVC_rate, Qt::AlignRight);
	m_sampleListView->setColumnAlignment(LVC_channels, Qt::AlignRight);

	// Add a menu bar

	QMenuBar *menuBar = new QMenuBar(this, "SoundTemplateWidgetMenuBar");

	QPopupMenu *fileMenu = new QPopupMenu(this); 

	QAction *fileResetAction = new QAction(this, "FileResetAction");
	fileResetAction->addTo(fileMenu);
	fileResetAction->setMenuText("&Reset");
	connect(fileResetAction, SIGNAL(activated()), this, SLOT(onFileResetActionActivated()));

	fileMenu->insertSeparator();

	QAction *fileSaveAction = new QAction(this, "FileSaveAction");
	fileSaveAction->addTo(fileMenu);
	fileSaveAction->setMenuText("&Save");
	connect(fileSaveAction, SIGNAL(activated()), this, SLOT(onFileSaveActionActivated()));

	QAction *fileSaveAsAction = new QAction(this, "FileSaveAsAction");
	fileSaveAsAction->addTo(fileMenu);
	fileSaveAsAction->setMenuText("Save &As");
	connect(fileSaveAsAction, SIGNAL(activated()), this, SLOT(onFileSaveAsActionActivated()));

	menuBar->insertItem("&File", fileMenu);

	// Sample List Popup Menu
	
	m_sampleListPlayPopupMenu = new QPopupMenu(this);
	m_sampleListPlayPopupMenu->insertItem("Add Sample", SLPM_addSample);
	m_sampleListPlayPopupMenu->insertItem("Remove Sample", SLPM_removeSample);
	m_sampleListPlayPopupMenu->insertSeparator();
	m_sampleListPlayPopupMenu->setCheckable(false);
	connect(m_sampleListPlayPopupMenu, SIGNAL(activated(int)), SLOT(onSoundsToPlayPopupMenuActivated(int)));

	m_loopStartOffsetTextLabel->setText("Not specified");
	m_loopEndOffsetTextLabel->setText("Not specified");

	// Set the validators

	QIntValidator *intValidator = new QIntValidator(this);
	QDoubleValidator *doubleValidator = new QDoubleValidator(this);

	m_startDelayMinLineEdit->setValidator(doubleValidator);
	m_startDelayMaxLineEdit->setValidator(doubleValidator);
	m_loopCountMinLineEdit->setValidator(intValidator);
	m_loopCountMaxLineEdit->setValidator(intValidator);
	m_loopDelayMinLineEdit->setValidator(doubleValidator);
	m_loopDelayMaxLineEdit->setValidator(doubleValidator);
	m_fadeInMinLineEdit->setValidator(doubleValidator);
	m_fadeInMaxLineEdit->setValidator(doubleValidator);
	m_fadeOutMinLineEdit->setValidator(doubleValidator);
	m_fadeOutMaxLineEdit->setValidator(doubleValidator);
	m_volumeSampleRateLineEdit->setValidator(doubleValidator);
	m_volumeMinLineEdit->setValidator(intValidator);
	m_volumeMaxLineEdit->setValidator(intValidator);
	m_volumeInterpolationRateLineEdit->setValidator(doubleValidator);
	m_pitchSampleRateLineEdit->setValidator(doubleValidator);
	m_pitchMinLineEdit->setValidator(intValidator);
	m_pitchMaxLineEdit->setValidator(intValidator);
	m_pitchInterpolationRateLineEdit->setValidator(doubleValidator);
	m_distanceAtMaxVolumeLineEdit->setValidator(doubleValidator);

	// Set the connections

	connect(m_sampleListView, SIGNAL(mouseButtonClicked(int, QListViewItem *, const QPoint &, int)), this, SLOT(onMouseButtonClicked(int, QListViewItem *, const QPoint &, int)));
	//connect(m_sampleListView, SIGNAL(selectionChanged(QListViewItem *)), this, SLOT(selectionChanged(QListViewItem *)));
	//connect(m_sampleListView, SIGNAL())
	connect(m_closePushButton, SIGNAL(clicked()), this, SLOT(close()));
	connect(m_playPushButton, SIGNAL(clicked()), this, SLOT(onPlayPushButtonClicked()));
	connect(m_restartPushButton, SIGNAL(clicked()), this, SLOT(onRestartPushButtonClicked()));
	connect(m_startDelayMinLineEdit, SIGNAL(returnPressed()), this, SLOT(onStartDelayMinLineEditReturnPressed()));
	connect(m_startDelayMaxLineEdit, SIGNAL(returnPressed()), this, SLOT(onStartDelayMaxLineEditReturnPressed()));
	connect(m_loopCountMinLineEdit, SIGNAL(returnPressed()), this, SLOT(onLoopCountMinLineEditReturnPressed()));
	connect(m_loopCountMaxLineEdit, SIGNAL(returnPressed()), this, SLOT(onLoopCountMaxLineEditReturnPressed()));
	connect(m_loopDelayMinLineEdit, SIGNAL(returnPressed()), this, SLOT(onLoopDelayMinLineEditReturnPressed()));
	connect(m_loopDelayMaxLineEdit, SIGNAL(returnPressed()), this, SLOT(onLoopDelayMaxLineEditReturnPressed()));
	connect(m_infiniteLoopingCheckBox, SIGNAL(clicked()), this, SLOT(onInfiniteLoopingCheckBoxClicked()));
	connect(m_fadeInMinLineEdit, SIGNAL(returnPressed()), this, SLOT(onFadeInMinLineEditReturnPressed()));
	connect(m_fadeInMaxLineEdit, SIGNAL(returnPressed()), this, SLOT(onFadeInMaxLineEditReturnPressed()));
	connect(m_fadeOutMinLineEdit, SIGNAL(returnPressed()), this, SLOT(onFadeOutMinLineEditReturnPressed()));
	connect(m_fadeOutMaxLineEdit, SIGNAL(returnPressed()), this, SLOT(onFadeOutMaxLineEditReturnPressed()));
	connect(m_volumeMinLineEdit, SIGNAL(returnPressed()), this, SLOT(onVolumeMinLineEditReturnPressed()));
	connect(m_volumeMaxLineEdit, SIGNAL(returnPressed()), this, SLOT(onVolumeMaxLineEditReturnPressed()));
	connect(m_pitchMinLineEdit, SIGNAL(returnPressed()), this, SLOT(onPitchMinLineEditReturnPressed()));
	connect(m_pitchMaxLineEdit, SIGNAL(returnPressed()), this, SLOT(onPitchMaxLineEditReturnPressed()));
	connect(m_distanceAtMaxVolumeLineEdit, SIGNAL(returnPressed()), this, SLOT(onDistanceMinLineEditReturnPressed()));
	connect(m_loopDelaySampleRateComboBox, SIGNAL(activated(int)), this, SLOT(onLoopDelaySampleRateComboBoxActivated(int)));
	connect(m_fadeInSampleRateComboBox, SIGNAL(activated(int)), this, SLOT(onFadeInSampleRateComboBoxActivated(int)));
	connect(m_fadeOutSampleRateComboBox, SIGNAL(activated(int)), this, SLOT(onFadeOutSampleRateComboBoxActivated(int)));
	connect(m_spatialityMethodComboBox, SIGNAL(activated(int)), this, SLOT(onSpatialityMethodComboBoxActivated(int)));
	connect(m_volumeSampleRateComboBox, SIGNAL(activated(int)), this, SLOT(onVolumeSampleRateComboBoxActivated(int)));
	connect(m_pitchSampleRateComboBox, SIGNAL(activated(int)), this, SLOT(onPitchSampleRateComboBoxActivated(int)));
	connect(m_volumeInterpolationRateLineEdit, SIGNAL(returnPressed()), this, SLOT(onVolumeInterpolationRateLineEditReturnPressed()));
	connect(m_pitchInterpolationRateLineEdit, SIGNAL(returnPressed()), this, SLOT(onPitchInterpolationRateLineEditReturnPressed()));

	onSpatialityMethodComboBoxActivated(0);

	// Verify the initial values

	validateAll();

	// Disable the play and restart buttons since no sound(s) have been specified

	m_playPushButton->setEnabled(false);
	m_restartPushButton->setEnabled(false);

	// Start a timer to poll the status of the sound

	QTimer *timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), SLOT(onPollSound()));
	timer->start(1000 / 30);

	// Setup initial values

	resetSoundTemplate();

	// Save the state of the sound template

	write(m_currentSound2dTemplate);
}

//-----------------------------------------------------------------------------
SoundTemplateWidget::~SoundTemplateWidget()
{
	if (m_soundId.isValid())
	{
		Audio::stopSound(m_soundId);
	}

	if (m_cachedSoundTemplate != NULL)
	{
		SoundTemplateList::release(m_cachedSoundTemplate);
		m_cachedSoundTemplate = NULL;
	}
}

//-----------------------------------------------------------------------------
void SoundTemplateWidget::onPollSound()
{
	// Change to callbacks

	if (m_soundId.getId() != 0)
	{
		if (Audio::isSoundPlaying(m_soundId))
		{
			m_playPushButton->setText("Stop");

			// Set the sample filename

			TemporaryCrcString path;
			Audio::getCurrentSample(m_soundId, path);

			m_sampleFileNameLineEdit->setText(FileNameUtils::get(path.getString(), FileNameUtils::fileName | FileNameUtils::extension).c_str());

			// Set the sample time

			float timeTotal;
			float timeCurrent;

			Audio::getCurrentSampleTime(m_soundId, timeTotal, timeCurrent);

			char text[256];
			sprintf(text, "(%0.2f / %0.2f)", timeCurrent, timeTotal);
			m_sampleTimeTextLabel->setText(text);

			// Offsets

			int loopStartOffset;
			int loopEndOffset;
			bool loopOffsetsValid = Audio::getLoopOffsets(path, loopStartOffset, loopEndOffset);

			if (loopOffsetsValid)
			{
				m_loopStartOffsetTextLabel->setText(QString::number(loopStartOffset));
				m_loopEndOffsetTextLabel->setText(QString::number(loopEndOffset));
			}
			else
			{
				m_loopStartOffsetTextLabel->setText("Not specified");
				m_loopEndOffsetTextLabel->setText("Not specified");
			}

			// Current volume

			int const progress = static_cast<int>(Audio::getSoundVolume(m_soundId) * static_cast<float>(m_volumeProgressBar->totalSteps()));

			m_volumeProgressBar->setProgress(progress);

			// Current pitch

			sprintf(text, "%.2f", Audio::getSoundPitchDelta(m_soundId));
			m_pitchTextLabel->setText(text);
		}
		else
		{
			m_playPushButton->setText("Play");
			m_soundId.invalidate();

			// Reset the volume bar

			m_volumeProgressBar->setProgress(0);

			// Reset the sample time

			m_sampleTimeTextLabel->setText("(0.00 / 0.00)");
		}
	}
	else if (m_sampleListView->childCount() <= 0)
	{
		m_playPushButton->setEnabled(false);
		m_restartPushButton->setEnabled(false);
	}
	else
	{
		m_playPushButton->setEnabled(true);
		m_restartPushButton->setEnabled(true);
	}
}

//-----------------------------------------------------------------------------
void SoundTemplateWidget::addSample()
{
	// Get the last used path

	QSettings settings;
	settings.insertSearchPath(QSettings::Windows, SoundEditorUtility::getSearchPath());
	QString previousUsedPath(settings.readEntry("AddSoundPath", "c:\\"));

	// Get the file

	QStringList selectedFileNames(QFileDialog::getOpenFileNames("Sound files (*.wav *.mp3)", previousUsedPath, this, "AddFileDialog", "Add Sound"));

	for (QStringList::Iterator current = selectedFileNames.begin(); current != selectedFileNames.end(); ++current)
	{
		std::string path(!(*current).isNull() ? static_cast<char const *>(*current) : "");

		// Make sure the specified file exists on disk

		if (FileNameUtils::isReadable(path))
		{
			// Save the path

			settings.writeEntry("AddSoundPath", path.c_str());

			addSample(path);
		}
		else if (!path.empty())
		{
			std::string text("The selected file does not exist on disk: \"" + FileNameUtils::get(path, FileNameUtils::fileName | FileNameUtils::extension) + "\"");
			QMessageBox::warning(this, "File Does Not Exist", text.c_str(), "OK");
		}
	}

	reNumberSamples();
}

//-----------------------------------------------------------------------------
void SoundTemplateWidget::addSample(std::string const &path)
{
	// Make sure the file exists on disk

	std::string testTreeFilePath;

	if (TreeFile::exists(path.c_str()))
	{
		// Make sure this file is not empty

		int const sampleSize = Audio::getSampleSize(path.c_str());

		if (sampleSize <= 0)
		{
			QString text;
			text.sprintf("WARNING: The file has zero size and will not be loaded: \"%s\"\n", path.c_str());
			SoundEditorUtility::report(text);
		}
		else if ((FileNameUtils::getExtension(path) != "mp3") &&
		         (FileNameUtils::getExtension(path) != "wav"))
		{
			QString text;
			text.sprintf("WARNING: The file is not of type \".mp3\" or \".wav\" and will not be loaded: %s\n", path.c_str());
			SoundEditorUtility::report(text);
		}
		else
		{
			// Make sure the sound is not already in the list, duplicate samples is not allowed

			SoundTemplateWidgetListViewItem *child = dynamic_cast<SoundTemplateWidgetListViewItem *>(m_sampleListView->firstChild());

			bool alreadyUsed = false;
			
			// Sample filenames

			while (child != NULL)
			{
				child->getSamplePath();

				if (path == child->getSamplePath())
				{
					alreadyUsed = true;

					QString text;
					text.sprintf("WARNING: \"%s\" is already in the sample list. Duplicates are not allowed.\n", path.c_str());
					SoundEditorUtility::report(text);
					break;
				}

				child = dynamic_cast<SoundTemplateWidgetListViewItem *>(child->nextSibling());
			}

			if (!alreadyUsed)
			{
				SoundTemplateWidgetListViewItem *listViewItem = new SoundTemplateWidgetListViewItem(m_sampleListView);

				std::string newPath(path);
				FileNameUtils::swapChar(newPath, '/', '\\');

				listViewItem->setSamplePath(newPath);

				char bits[256];
				sprintf(bits, "%d", listViewItem->getSampleBits());

				char rate[256];
				sprintf(rate, "%d", listViewItem->getSampleRate());

				char channels[256];
				sprintf(channels, "%d", listViewItem->getSampleChannels());

				char fileSizeString[256];
				sprintf(fileSizeString, "%d KB", listViewItem->getSampleSize() / 1024);

				QString time;
				time.sprintf("%.2f", listViewItem->getSampleTime());

				listViewItem->setText(LVC_number, "");
				listViewItem->setText(LVC_sample, listViewItem->getSampleTreeFilePath().c_str());
				listViewItem->setText(LVC_size, fileSizeString);
				listViewItem->setText(LVC_time, time);
				listViewItem->setText(LVC_type, listViewItem->getSampleFileType().c_str());
				listViewItem->setText(LVC_bits, bits);
				listViewItem->setText(LVC_rate, rate);
				listViewItem->setText(LVC_channels, channels);

				m_sampleListView->insertItem(listViewItem);
			}
		}
	}
	else
	{
		QString text;
		text.sprintf("WARNING: Unable to find the sample in the tree file path and it will not be loaded: \"%s\"\n", path.c_str());
		SoundEditorUtility::report(text);
	}
}

//-----------------------------------------------------------------------------
void SoundTemplateWidget::removeSample()
{
	QListViewItem *child = m_sampleListView->firstChild();

	// Sample filenames

	while (child != NULL)
	{
		QListViewItem *current = child;

		child = child->nextSibling();

		if (m_sampleListView->isSelected(current))
		{
			delete current;
		}
	}

	m_selectedListViewItem = NULL;

	// Remove all selected items

	m_sampleListView->clearSelection();

	// Re-number all the items

	reNumberSamples();
}

//-----------------------------------------------------------------------------
void SoundTemplateWidget::closeEvent(QCloseEvent *closeEvent)
{
	// Stop any sounds if they are playing

	if (m_soundId.isValid())
	{
		Audio::stopSound(m_soundId);
	}

	// Check if they are about to lose some data changes.

	Sound2dTemplate sound2dTemplate;
	write(sound2dTemplate);

	bool close = true;

	if (sound2dTemplate != m_currentSound2dTemplate)
	{
		QString caption("Warning");
		QString text;

		if (m_soundTemplateFileName.empty())
		{
			text.sprintf("Would you like to save before losing all changes to the new sound template?");
		}
		else
		{
			text.sprintf("Would you like to save before losing all changes to file: \"%s\"", FileNameUtils::get(m_soundTemplateFileName, FileNameUtils::fileName | FileNameUtils::extension).c_str());
		}

		int result = QMessageBox::warning(this, caption, text, QMessageBox::Yes, QMessageBox::No);

		switch (result)
		{
			case QMessageBox::Yes:
				{
					bool saveResult = save();

					if (!saveResult)
					{
						close = false;
					}
				}
				break;
			case QMessageBox::No:
			default:
				{
				}
				break;
		}
	}

	if (close &&
	    (closeEvent != NULL))
	{
		closeEvent->accept();
	}
}

//-----------------------------------------------------------------------------
void SoundTemplateWidget::onPlayPushButtonClicked()
{
	// Verify all the data in the fields

	onFadeInMaxLineEditReturnPressed();
	onFadeOutMaxLineEditReturnPressed();
	onLoopCountMaxLineEditReturnPressed();
	onLoopDelayMaxLineEditReturnPressed();
	onPitchMaxLineEditReturnPressed();
	onVolumeMaxLineEditReturnPressed();
	onDistanceMinLineEditReturnPressed();

	validateAll();

	if (m_soundId.getId() == 0)
	{
		playSoundTemplate();
	}
	else
	{
		Audio::PlayBackStatus status = Audio::getSoundPlayBackStatus(m_soundId);

		if (status != Audio::PS_done)
		{
			Audio::stopSound(m_soundId);
		}
	}
}

//-----------------------------------------------------------------------------
void SoundTemplateWidget::onRestartPushButtonClicked()
{
	// End the current sound

	Audio::stopSound(m_soundId);

	// Make the play button update

	onPollSound();

	// Reset the current sound id

	m_soundId.invalidate();

	// Play the sound again

	onPlayPushButtonClicked();
}

//-----------------------------------------------------------------------------
void SoundTemplateWidget::onInfiniteLoopingCheckBoxClicked()
{
	if (m_infiniteLoopingCheckBox->isChecked())
	{
		m_loopCountMinLineEdit->setEnabled(false);
		m_loopCountMaxLineEdit->setEnabled(false);
		m_loopCountMinLineEdit->setText("");
		m_loopCountMaxLineEdit->setText("");
	}
	else
	{
		m_loopCountMinLineEdit->setEnabled(true);
		m_loopCountMaxLineEdit->setEnabled(true);
		m_loopCountMinLineEdit->setText("1");
		m_loopCountMaxLineEdit->setText("1");
	}
}

//-----------------------------------------------------------------------------
void SoundTemplateWidget::playSoundTemplate()
{
	if (m_sampleListView->firstChild())
	{
		if (isSound3d())
		{
			if (verifySampleListAll3d())
			{
				Sound3dTemplate sound3dTemplate;
			
				write(sound3dTemplate);

				Iff iff(2);
				
				sound3dTemplate.write(iff);
			
				m_soundId = Audio::playSound(iff, Vector::zero);
			}
			else
			{
				QMessageBox::warning(this, "Error", "To play a 3D sound, all the samples must be mono (channels = 1), uncompressed WAV files which is of type (Standard PCM wav file).", "OK");
			}
		}
		else
		{
			Sound2dTemplate sound2dTemplate;

			write(sound2dTemplate);
			
			Iff iff(2);

			sound2dTemplate.write(iff);

			m_soundId = Audio::playSound(iff);
		}
	}
}

//-----------------------------------------------------------------------------
void SoundTemplateWidget::onMouseButtonClicked(int button, QListViewItem *listViewItem, const QPoint &point, int column)
{
	UNREF(column);

	// Save the selected item

	m_selectedListViewItem = listViewItem;

	// The column is -1 when nothing is in the list and zero when something is in the list and 
	// the first column is selected

	switch (button)
	{
		case Qt::RightButton:
			{
				m_sampleListPlayPopupMenu->setItemEnabled(SLPM_removeSample, (m_selectedListViewItem != NULL));
				m_sampleListPlayPopupMenu->exec(point);
			}
			break;
	}
}

//-----------------------------------------------------------------------------
void SoundTemplateWidget::onSoundsToPlayPopupMenuActivated(int index)
{
	UNREF(index);

	switch (index)
	{
		case SLPM_addSample:
			{
				addSample();
			}
			break;
		case SLPM_removeSample:
			{
				removeSample();
			}
			break;
		default:
			{
				assert(0);
			}
			break;
	}
}

//-----------------------------------------------------------------------------
void SoundTemplateWidget::onFileResetActionActivated()
{
	int result = QMessageBox::warning(this, "Warning", "Reset removes any changes from the default sound template.", QMessageBox::Ok, QMessageBox::Cancel);

	if (result == QMessageBox::Ok)
	{
		resetSoundTemplate();
	}
}

//-----------------------------------------------------------------------------
void SoundTemplateWidget::resetSoundTemplate()
{
	// If there is a sound playing, stop it

	if (m_soundId.getId() != 0)
	{
		Audio::stopSound(m_soundId);
	}

	// Remove all the samples

	QListViewItem *child = m_sampleListView->firstChild();

	while (child != NULL)
	{
		QListViewItem *current = child;

		child = child->nextSibling();

		delete current;
	}

	m_selectedListViewItem = NULL;

	// Clear the selected sample

	m_sampleListView->clearSelection();

	// Start Delay

	m_startDelayMinLineEdit->setText("0.00");
	m_startDelayMaxLineEdit->setText("0.00");

	// Loop Count

	m_infiniteLoopingCheckBox->setChecked(false);
	onInfiniteLoopingCheckBoxClicked();

	// Loop Delay

	onLoopDelaySampleRateComboBoxActivated(static_cast<int>(Sound2dTemplate::LDSR_noDelay));

	// Fade-In

	onFadeInSampleRateComboBoxActivated(static_cast<int>(Sound2dTemplate::FISR_noFade));

	// Volume

	onVolumeSampleRateComboBoxActivated(static_cast<int>(Sound2dTemplate::VSRT_fullVolume));

	// Pitch

	onPitchSampleRateComboBoxActivated(static_cast<int>(Sound2dTemplate::PSRT_basePitch));

	// Fade-Out

	onFadeOutSampleRateComboBoxActivated(static_cast<int>(Sound2dTemplate::FOSR_noFade));

	// 3D Distance

	m_spatialityMethodComboBox->setCurrentItem(AM_none);
	m_distanceAtMaxVolumeLineEdit->setEnabled(false);

	// General

	m_playOrderComboBox->setCurrentItem(static_cast<int>(Sound2dTemplate::PO_random));
	m_priorityComboBox->setCurrentItem(9);
	m_categoryComboBox->setCurrentItem(static_cast<int>(Audio::SC_ambient));

	// Reset the sample list title

	m_sampleListGroupBox->setTitle("Sample List");
}

//-----------------------------------------------------------------------------
bool SoundTemplateWidget::save()
{
	validateAll();

	bool result = true;

	if (m_sampleListView->childCount() <= 0)
	{
		QMessageBox::warning(this, "Error", "Saving a sound template without any samples assigned in the Sample List is not allowed.", "OK");
		result = false;
	}
	else if (isSound3d() && !verifySampleListAll3d())
	{
		QMessageBox::warning(this, "Error", "To save a 3D sound, all the samples must be mono, uncompressed WAV files which is of type (Standard PCM wav file).", "OK");
		result = false;
	}
	else
	{
		// Make sure the filename is specified

		if (!m_soundTemplateFileName.empty())
		{
			// Make sure the filename is writable

			if (FileNameUtils::isWritable(m_soundTemplateFileName))
			{
				// Set the name of the current saved file

				setCaption(m_soundTemplateFileName.c_str());

				// Save the file

				Iff iff(2);

				write(iff);

				// Save the state of the sound template

				write(m_currentSound2dTemplate);

				QString text;
				text.sprintf("SAVE: \"%s\"\n", m_soundTemplateFileName.c_str());
				SoundEditorUtility::report(text);

				iff.write(m_soundTemplateFileName.c_str());
			}
			else
			{
				std::string text("Error saving file. Make sure the file is not read only: " + FileNameUtils::get(m_soundTemplateFileName, FileNameUtils::fileName | FileNameUtils::extension));
				QMessageBox::warning(this, "Error Saving File", text.c_str(), "I will check out the file before trying to save it.");
				result = false;
			}
		}
		else
		{
			result = saveAs();
		}
	}

	return result;
}

//-----------------------------------------------------------------------------
void SoundTemplateWidget::onFileSaveActionActivated()
{
	save();
}

//-----------------------------------------------------------------------------
bool SoundTemplateWidget::saveAs()
{
	bool result = true;

	if (m_sampleListView->childCount() <= 0)
	{
		QMessageBox::warning(this, "Error", "Saving a sound template without any samples assigned in the Sample List is not allowed.", "OK");

		result = false;
	}
	else if (isSound3d() && !verifySampleListAll3d())
	{
		QMessageBox::warning(this, "Error", "To save a 3D sound, all the samples must be mono, uncompressed WAV files which is of type (Standard PCM wav file).", "OK");

		result = false;
	}
	else
	{
		// Get the last used path

		QSettings settings;
		settings.insertSearchPath(QSettings::Windows, SoundEditorUtility::getSearchPath());
		QString previousUsedPath(settings.readEntry("SaveSoundTemplatePath", "c:\\"));

		// Get the file

		QString selectedFileName(QFileDialog::getSaveFileName(previousUsedPath, "Sound files (*.snd)", this, "SaveSoundTemplateDialog", "Save Sound Template"));

		if (!selectedFileName.isNull())
		{
			// Slap the correct extension on the file

			m_soundTemplateFileName = FileNameUtils::get(static_cast<char const *>(selectedFileName), FileNameUtils::drive | FileNameUtils::directory | FileNameUtils::fileName) + std::string(".snd");

			// Save the path

			settings.writeEntry("SaveSoundTemplatePath", m_soundTemplateFileName.c_str());

			// Save the file

			result = save();
		}
		else
		{
			result = false;
		}
	}

	return result;
}

//-----------------------------------------------------------------------------
void SoundTemplateWidget::onFileSaveAsActionActivated()
{
	saveAs();
}

//-----------------------------------------------------------------------------
void SoundTemplateWidget::load(Iff &iff)
{
	QString text;
	text.sprintf("LOAD: \"%s\"\n", iff.getFileName());
	SoundEditorUtility::report(text);

	// Put the filename in the caption

	setCaption(iff.getFileName());

	// Save the loaded filename

	m_soundTemplateFileName = iff.getFileName();

	// Put the iff info into a sound template

	if (m_cachedSoundTemplate != NULL)
	{
		SoundTemplateList::release(m_cachedSoundTemplate);
		m_cachedSoundTemplate = NULL;
	}

	m_cachedSoundTemplate = SoundTemplateList::fetch(&iff);

	Sound2dTemplate const *sound2dTemplate = dynamic_cast<Sound2dTemplate const *>(m_cachedSoundTemplate);

	DEBUG_FATAL((sound2dTemplate == NULL), ("SoundTemplateWidget::load) - The sound 2d template is NULL."));

	if (sound2dTemplate != NULL)
	{
		// Samples

		Sound2dTemplate::StringList::const_iterator iterSampleImagePathList = sound2dTemplate->getSampleList().begin();
		
		for (; iterSampleImagePathList != sound2dTemplate->getSampleList().end(); ++iterSampleImagePathList)
		{
			CrcString const *path = (*iterSampleImagePathList);

			addSample(path->getString());
		}

		// Start Delay

		SoundEditorUtility::setLineEditFloat(m_startDelayMinLineEdit, sound2dTemplate->getStartDelayMin(), m_startDelayBound, m_floatingPointPrecision);
		SoundEditorUtility::setLineEditFloat(m_startDelayMaxLineEdit, sound2dTemplate->getStartDelayMax(), m_startDelayBound, m_floatingPointPrecision);

		// Loop Count

		if (sound2dTemplate->getLoopCountMin() < 0)
		{
			// Infinite looping

			m_infiniteLoopingCheckBox->setChecked(true);

			m_loopCountMinLineEdit->setText("");
			m_loopCountMaxLineEdit->setText("");
			m_loopCountMinLineEdit->setEnabled(false);
			m_loopCountMaxLineEdit->setEnabled(false);
		}
		else
		{
			m_infiniteLoopingCheckBox->setChecked(false);

			SoundEditorUtility::setLineEditInt(m_loopCountMinLineEdit, sound2dTemplate->getLoopCountMin(), m_loopCountBound);
			SoundEditorUtility::setLineEditInt(m_loopCountMaxLineEdit, sound2dTemplate->getLoopCountMax(), m_loopCountBound);
		}

		// Loop Delay

		m_loopDelaySampleRateComboBox->setCurrentItem(static_cast<int>(sound2dTemplate->getLoopDelaySampleRate()));

		switch (sound2dTemplate->getLoopDelaySampleRate())
		{
			case Sound2dTemplate::LDSR_noDelay:
				{
					m_loopDelayMinLineEdit->setText("");
					m_loopDelayMaxLineEdit->setText("");
					m_loopDelayMinLineEdit->setEnabled(false);
					m_loopDelayMaxLineEdit->setEnabled(false);
				}
				break;
			case Sound2dTemplate::LDSR_initial:
				{
					SoundEditorUtility::setLineEditFloat(m_loopDelayMinLineEdit, sound2dTemplate->getLoopDelayMin(), m_loopDelayBound, m_floatingPointPrecision);
					SoundEditorUtility::setLineEditFloat(m_loopDelayMaxLineEdit, sound2dTemplate->getLoopDelayMax(), m_loopDelayBound, m_floatingPointPrecision);
					m_loopDelayMinLineEdit->setEnabled(true);
					m_loopDelayMaxLineEdit->setEnabled(true);
				}
				break;
			case Sound2dTemplate::LDSR_everySample:
				{
					SoundEditorUtility::setLineEditFloat(m_loopDelayMinLineEdit, sound2dTemplate->getLoopDelayMin(), m_loopDelayBound, m_floatingPointPrecision);
					SoundEditorUtility::setLineEditFloat(m_loopDelayMaxLineEdit, sound2dTemplate->getLoopDelayMax(), m_loopDelayBound, m_floatingPointPrecision);
					m_loopDelayMinLineEdit->setEnabled(true);
					m_loopDelayMaxLineEdit->setEnabled(true);
				}
				break;
			default:
				{
					DEBUG_FATAL(true, ("Invalid loop delay type specified."));
				}
		}

		// Fade-In

		m_fadeInSampleRateComboBox->setCurrentItem(static_cast<int>(sound2dTemplate->getFadeInSampleRate()));

		switch (sound2dTemplate->getFadeInSampleRate())
		{
			case Sound2dTemplate::FISR_noFade:
				{
					m_fadeInMinLineEdit->setText("");
					m_fadeInMaxLineEdit->setText("");
					m_fadeInMinLineEdit->setEnabled(false);
					m_fadeInMaxLineEdit->setEnabled(false);
				}
				break;
			case Sound2dTemplate::FISR_firstSample:
				{
					SoundEditorUtility::setLineEditFloat(m_fadeInMinLineEdit, sound2dTemplate->getFadeInMin(), m_fadeInBound, m_floatingPointPrecision);
					SoundEditorUtility::setLineEditFloat(m_fadeInMaxLineEdit, sound2dTemplate->getFadeInMax(), m_fadeInBound, m_floatingPointPrecision);
					m_fadeInMinLineEdit->setEnabled(true);
					m_fadeInMaxLineEdit->setEnabled(true);
				}
				break;
			case Sound2dTemplate::FISR_everySample:
				{
					SoundEditorUtility::setLineEditFloat(m_fadeInMinLineEdit, sound2dTemplate->getFadeInMin(), m_fadeInBound, m_floatingPointPrecision);
					SoundEditorUtility::setLineEditFloat(m_fadeInMaxLineEdit, sound2dTemplate->getFadeInMax(), m_fadeInBound, m_floatingPointPrecision);
					m_fadeInMinLineEdit->setEnabled(true);
					m_fadeInMaxLineEdit->setEnabled(true);
				}
				break;
			default:
				{
					DEBUG_FATAL(true, ("Invalid fade in type specified."));
				}
		}

		// Volume

		m_volumeSampleRateComboBox->setCurrentItem(static_cast<int>(sound2dTemplate->getVolumeSampleRateType()));

		switch (sound2dTemplate->getVolumeSampleRateType())
		{
			case Sound2dTemplate::VSRT_fullVolume:
				{
					m_volumeSampleRateLineEdit->setEnabled(false);
					m_volumeMinLineEdit->setEnabled(false);
					m_volumeMaxLineEdit->setEnabled(false);
					m_volumeInterpolationRateLineEdit->setEnabled(false);
					m_volumeSampleRateLineEdit->setText("");
					m_volumeMinLineEdit->setText("");
					m_volumeMaxLineEdit->setText("");
					m_volumeInterpolationRateLineEdit->setText("");
				}
				break;
			case Sound2dTemplate::VSRT_initial:
				{
					m_volumeSampleRateLineEdit->setEnabled(false);
					m_volumeMinLineEdit->setEnabled(true);
					m_volumeMaxLineEdit->setEnabled(true);
					m_volumeInterpolationRateLineEdit->setEnabled(false);
					m_volumeSampleRateLineEdit->setText("");
					
					int min = static_cast<int>(sound2dTemplate->getVolumeMin() * 100.5f);
					int max = static_cast<int>(sound2dTemplate->getVolumeMax() * 100.5f);

					SoundEditorUtility::setLineEditInt(m_volumeMinLineEdit, min, m_volumeBound);
					SoundEditorUtility::setLineEditInt(m_volumeMaxLineEdit, max, m_volumeBound);
					m_volumeInterpolationRateLineEdit->setText("");
				}
				break;
			case Sound2dTemplate::VSRT_everySample:
				{
					m_volumeSampleRateLineEdit->setEnabled(false);
					m_volumeMinLineEdit->setEnabled(true);
					m_volumeMaxLineEdit->setEnabled(true);
					m_volumeInterpolationRateLineEdit->setEnabled(false);
					m_volumeSampleRateLineEdit->setText("");

					int min = static_cast<int>(sound2dTemplate->getVolumeMin() * 100.5f);
					int max = static_cast<int>(sound2dTemplate->getVolumeMax() * 100.5f);

					SoundEditorUtility::setLineEditInt(m_volumeMinLineEdit, min, m_volumeBound);
					SoundEditorUtility::setLineEditInt(m_volumeMaxLineEdit, max, m_volumeBound);
					m_volumeInterpolationRateLineEdit->setText("");
				}
				break;
			case Sound2dTemplate::VSRT_seconds:
				{
					m_volumeSampleRateLineEdit->setEnabled(true);
					m_volumeMinLineEdit->setEnabled(true);
					m_volumeMaxLineEdit->setEnabled(true);
					m_volumeInterpolationRateLineEdit->setEnabled(true);
					SoundEditorUtility::setLineEditFloat(m_volumeSampleRateLineEdit, sound2dTemplate->getVolumeSampleRate(), m_volumeSampleRateBound, m_floatingPointPrecision);

					int min = static_cast<int>(sound2dTemplate->getVolumeMin() * 100.5f);
					int max = static_cast<int>(sound2dTemplate->getVolumeMax() * 100.5f);

					SoundEditorUtility::setLineEditInt(m_volumeMinLineEdit, min, m_volumeBound);
					SoundEditorUtility::setLineEditInt(m_volumeMaxLineEdit, max, m_volumeBound);
					SoundEditorUtility::setLineEditFloat(m_volumeInterpolationRateLineEdit, sound2dTemplate->getVolumeInterpolationRate(), m_volumeInterpolationRateBound, m_floatingPointPrecision);
				}
				break;
			default:
				{
					DEBUG_FATAL(true, ("Invalid volume sample rate type specified."));
				}
		}

		// Pitch

		m_pitchSampleRateComboBox->setCurrentItem(static_cast<int>(sound2dTemplate->getPitchSampleRateType()));

		switch (sound2dTemplate->getPitchSampleRateType())
		{
			case Sound2dTemplate::PSRT_basePitch:
				{
					m_pitchSampleRateLineEdit->setEnabled(false);
					m_pitchMinLineEdit->setEnabled(false);
					m_pitchMaxLineEdit->setEnabled(false);
					m_pitchInterpolationRateLineEdit->setEnabled(false);
					m_pitchSampleRateLineEdit->setText("");
					m_pitchMinLineEdit->setText("");
					m_pitchMaxLineEdit->setText("");
					m_pitchInterpolationRateLineEdit->setText("");
				}
				break;
			case Sound2dTemplate::PSRT_everySample:
				{
					m_pitchSampleRateLineEdit->setEnabled(false);
					m_pitchMinLineEdit->setEnabled(true);
					m_pitchMaxLineEdit->setEnabled(true);
					m_pitchInterpolationRateLineEdit->setEnabled(false);
					m_pitchSampleRateLineEdit->setText("");
					SoundEditorUtility::setLineEditFloat(m_pitchMinLineEdit, sound2dTemplate->getPitchMin(), m_pitchBound, m_floatingPointPrecision);
					SoundEditorUtility::setLineEditFloat(m_pitchMaxLineEdit, sound2dTemplate->getPitchMax(), m_pitchBound, m_floatingPointPrecision);
					m_pitchInterpolationRateLineEdit->setText("");
				}
				break;
			case Sound2dTemplate::PSRT_initial:
				{
					m_pitchSampleRateLineEdit->setEnabled(false);
					m_pitchMinLineEdit->setEnabled(true);
					m_pitchMaxLineEdit->setEnabled(true);
					m_pitchInterpolationRateLineEdit->setEnabled(false);
					m_pitchSampleRateLineEdit->setText("");
					SoundEditorUtility::setLineEditFloat(m_pitchMinLineEdit, sound2dTemplate->getPitchMin(), m_pitchBound, m_floatingPointPrecision);
					SoundEditorUtility::setLineEditFloat(m_pitchMaxLineEdit, sound2dTemplate->getPitchMax(), m_pitchBound, m_floatingPointPrecision);
					m_pitchInterpolationRateLineEdit->setText("");
				}
				break;
			case Sound2dTemplate::PSRT_seconds:
				{
					m_pitchSampleRateLineEdit->setEnabled(true);
					m_pitchMinLineEdit->setEnabled(true);
					m_pitchMaxLineEdit->setEnabled(true);
					m_pitchInterpolationRateLineEdit->setEnabled(true);
					SoundEditorUtility::setLineEditFloat(m_pitchSampleRateLineEdit, sound2dTemplate->getPitchSampleRate(), m_pitchSampleRateBound, m_floatingPointPrecision);
					SoundEditorUtility::setLineEditFloat(m_pitchMinLineEdit, sound2dTemplate->getPitchMin(), m_pitchBound, m_floatingPointPrecision);
					SoundEditorUtility::setLineEditFloat(m_pitchMaxLineEdit, sound2dTemplate->getPitchMax(), m_pitchBound, m_floatingPointPrecision);
					SoundEditorUtility::setLineEditFloat(m_pitchInterpolationRateLineEdit, sound2dTemplate->getPitchInterpolationRate(), m_pitchInterpolationRateBound, m_floatingPointPrecision);
				}
				break;
			default:
				{
					DEBUG_FATAL(true, ("Invalid pitch sample rate type specified."));
				}
		}

		// Fade-Out

		m_fadeOutSampleRateComboBox->setCurrentItem(static_cast<int>(sound2dTemplate->getFadeOutSampleRate()));

		switch (sound2dTemplate->getFadeOutSampleRate())
		{
			case Sound2dTemplate::FOSR_noFade:
				{
					m_fadeOutMinLineEdit->setText("");
					m_fadeOutMaxLineEdit->setText("");
					m_fadeOutMinLineEdit->setEnabled(false);
					m_fadeOutMaxLineEdit->setEnabled(false);
				}
				break;
			case Sound2dTemplate::FOSR_lastSample:
				{
					SoundEditorUtility::setLineEditFloat(m_fadeOutMinLineEdit, sound2dTemplate->getFadeOutMin(), m_fadeOutBound, m_floatingPointPrecision);
					SoundEditorUtility::setLineEditFloat(m_fadeOutMaxLineEdit, sound2dTemplate->getFadeOutMax(), m_fadeOutBound, m_floatingPointPrecision);
					m_fadeOutMinLineEdit->setEnabled(true);
					m_fadeOutMaxLineEdit->setEnabled(true);
				}
				break;
			case Sound2dTemplate::FOSR_everySample:
				{
					SoundEditorUtility::setLineEditFloat(m_fadeOutMinLineEdit, sound2dTemplate->getFadeOutMin(), m_fadeOutBound, m_floatingPointPrecision);
					SoundEditorUtility::setLineEditFloat(m_fadeOutMaxLineEdit, sound2dTemplate->getFadeOutMax(), m_fadeOutBound, m_floatingPointPrecision);
					m_fadeOutMinLineEdit->setEnabled(true);
					m_fadeOutMaxLineEdit->setEnabled(true);
				}
				break;
			default:
				{
					DEBUG_FATAL(true, ("Invalid fade out type specified."));
				}
		}

		// Play order

		m_playOrderComboBox->setCurrentItem(static_cast<int>(sound2dTemplate->getPlayOrder()));

		// Priority

		m_priorityComboBox->setCurrentItem(sound2dTemplate->getPriority());

		// Category

		m_categoryComboBox->setCurrentItem(static_cast<int>(sound2dTemplate->getSoundCategory()));

		// Distance attenuation

		SoundEditorUtility::setLineEditFloat(m_distanceAtMaxVolumeLineEdit, sound2dTemplate->getDistanceAtMaxVolume(), m_distanceAtMaxVolumeBound, m_floatingPointPrecision);

		if (sound2dTemplate->getDistanceAtMaxVolume() > 0.0f)
		{
			m_spatialityMethodComboBox->setCurrentItem(AM_2d);
			m_distanceAtMaxVolumeLineEdit->setEnabled(true);
		}
		else
		{
			m_spatialityMethodComboBox->setCurrentItem(AM_none);
			m_distanceAtMaxVolumeLineEdit->setEnabled(false);
			m_distanceAtMaxVolumeLineEdit->setText("");
		}
	}

	// 3D Distance

	Sound3dTemplate const *sound3dTemplate = dynamic_cast<Sound3dTemplate const *>(m_cachedSoundTemplate);

	if (sound3dTemplate != NULL)
	{
		m_spatialityMethodComboBox->setCurrentItem(AM_3d);
		m_distanceAtMaxVolumeLineEdit->setEnabled(true);
	}

	reNumberSamples();
	validateAll();

	// Save the state of the sound template

	write(m_currentSound2dTemplate);
}

//-----------------------------------------------------------------------------
void SoundTemplateWidget::write(Iff &iff) const
{
	if (isSound3d())
	{
		// Create a sound 3d template

		Sound3dTemplate sound3dTemplate;

		// Write the template

		write(sound3dTemplate);

		// Write the template to iff

		sound3dTemplate.write(iff);
	}
	else
	{
		// Create a sound 2d template

		Sound2dTemplate sound2dTemplate;

		// Write the template

		write(sound2dTemplate);

		// Write the template to iff

		sound2dTemplate.write(iff);
	}
}

//-----------------------------------------------------------------------------
void SoundTemplateWidget::write(Sound2dTemplate &sound2dTemplate) const
{
	// Save all the sample file names

	SoundTemplateWidgetListViewItem *child = dynamic_cast<SoundTemplateWidgetListViewItem *>(m_sampleListView->firstChild());

	// Sample filenames
	
	sound2dTemplate.clearSampleList();

	while (child != NULL)
	{
		std::string const &treeFilePath = child->getSampleTreeFilePath();

		sound2dTemplate.addSample(treeFilePath.c_str(), false);

		child = dynamic_cast<SoundTemplateWidgetListViewItem *>(child->nextSibling());
	}

	sound2dTemplate.setStartDelayMin(SoundEditorUtility::getFloat(m_startDelayMinLineEdit));
	sound2dTemplate.setStartDelayMax(SoundEditorUtility::getFloat(m_startDelayMaxLineEdit));
	sound2dTemplate.setFadeInMin(SoundEditorUtility::getFloat(m_fadeInMinLineEdit));
	sound2dTemplate.setFadeInMax(SoundEditorUtility::getFloat(m_fadeInMaxLineEdit));

	if (m_infiniteLoopingCheckBox->isChecked())
	{
		// Denote infinite looping

		sound2dTemplate.setLoopCountMin(-1);
		sound2dTemplate.setLoopCountMax(-1);
	}
	else
	{
		sound2dTemplate.setLoopCountMin(SoundEditorUtility::getInt(m_loopCountMinLineEdit));
		sound2dTemplate.setLoopCountMax(SoundEditorUtility::getInt(m_loopCountMaxLineEdit));
	}

	sound2dTemplate.setLoopDelayMin(SoundEditorUtility::getFloat(m_loopDelayMinLineEdit));
	sound2dTemplate.setLoopDelayMax(SoundEditorUtility::getFloat(m_loopDelayMaxLineEdit));
	sound2dTemplate.setFadeOutMin(SoundEditorUtility::getFloat(m_fadeOutMinLineEdit));
	sound2dTemplate.setFadeOutMax(SoundEditorUtility::getFloat(m_fadeOutMaxLineEdit));

	// Loop delay sample rate

	sound2dTemplate.setLoopDelaySampleRate(static_cast<Sound2dTemplate::LoopDelaySampleRate>(m_loopDelaySampleRateComboBox->currentItem()));

	// Fade-in sample rate

	sound2dTemplate.setFadeInSampleRate(static_cast<Sound2dTemplate::FadeInSampleRate>(m_fadeInSampleRateComboBox->currentItem()));

	// Fade-out sample rate

	sound2dTemplate.setFadeOutSampleRate(static_cast<Sound2dTemplate::FadeOutSampleRate>(m_fadeOutSampleRateComboBox->currentItem()));

	// Volume

	sound2dTemplate.setVolumeMin(SoundEditorUtility::getFloat(m_volumeMinLineEdit) / 100.0f);
	sound2dTemplate.setVolumeMax(SoundEditorUtility::getFloat(m_volumeMaxLineEdit) / 100.0f);
	sound2dTemplate.setVolumeSampleRate(SoundEditorUtility::getFloat(m_volumeSampleRateLineEdit));
	sound2dTemplate.setVolumeInterpolationRate(SoundEditorUtility::getFloat(m_volumeInterpolationRateLineEdit));
	sound2dTemplate.setVolumeSampleRateType(static_cast<Sound2dTemplate::VolumeSampleRateType>(m_volumeSampleRateComboBox->currentItem()));

	switch (sound2dTemplate.getVolumeSampleRateType())
	{
		case Sound2dTemplate::VSRT_fullVolume:
			{
				sound2dTemplate.setVolumeMin(1.0f);
				sound2dTemplate.setVolumeMax(1.0f);
			}
			break;
	}

	// Pitch

	sound2dTemplate.setPitchMin(SoundEditorUtility::getFloat(m_pitchMinLineEdit));
	sound2dTemplate.setPitchMax(SoundEditorUtility::getFloat(m_pitchMaxLineEdit));
	sound2dTemplate.setPitchSampleRate(SoundEditorUtility::getFloat(m_pitchSampleRateLineEdit));
	sound2dTemplate.setPitchInterpolationRate(SoundEditorUtility::getFloat(m_pitchInterpolationRateLineEdit));

	sound2dTemplate.setPitchSampleRateType(static_cast<Sound2dTemplate::PitchSampleRateType>(m_pitchSampleRateComboBox->currentItem()));

	switch (sound2dTemplate.getPitchSampleRateType())
	{
		case Sound2dTemplate::PSRT_basePitch:
			{
				sound2dTemplate.setPitchMin(0.0f);
				sound2dTemplate.setPitchMax(0.0f);
			}
			break;
	}

	// General

	sound2dTemplate.setDistanceAtMaxVolume(SoundEditorUtility::getFloat(m_distanceAtMaxVolumeLineEdit));
	sound2dTemplate.setPlayOrder(static_cast<Sound2dTemplate::PlayOrder>(m_playOrderComboBox->currentItem()));
	sound2dTemplate.setPriority(m_priorityComboBox->currentItem());
	sound2dTemplate.setSoundCategory(static_cast<Audio::SoundCategory>(m_categoryComboBox->currentItem()));
}

//-----------------------------------------------------------------------------
void SoundTemplateWidget::write(Sound3dTemplate &sound3dTemplate) const
{
	// Write the 2d portion

	Sound2dTemplate *sound2dTemplate = dynamic_cast<Sound2dTemplate *>(&sound3dTemplate);

	write(*sound2dTemplate);

	// Write the 3d portion

	// no difference right now
}

//-----------------------------------------------------------------------------
bool SoundTemplateWidget::isSound3d() const
{
	return (m_spatialityMethodComboBox->currentItem() == AM_3d);
}

//-----------------------------------------------------------------------------
void SoundTemplateWidget::reNumberSamples()
{
	SoundTemplateWidgetListViewItem *child = dynamic_cast<SoundTemplateWidgetListViewItem *>(m_sampleListView->firstChild());

	int count = 1;
	int totalSize = 0;

	// Sample filenames

	while (child != NULL)
	{
		char text[256];
		sprintf(text, "%2d", count);

		child->setText(0, text);
		totalSize += child->getSampleSize();

		child = dynamic_cast<SoundTemplateWidgetListViewItem *>(child->nextSibling());

		++count;
	}

	// Display the number of samples loaded in the Sample List group box title

	char title[256];
	sprintf(title, "Sample List (%d samples @ %d KB)", count - 1, totalSize / 1024);
	m_sampleListGroupBox->setTitle(title);
}

//-----------------------------------------------------------------------------
void SoundTemplateWidget::onLoopDelaySampleRateComboBoxActivated(int index)
{
	switch (static_cast<Sound2dTemplate::LoopDelaySampleRate>(index))
	{
		case Sound2dTemplate::LDSR_noDelay:
			{
				m_loopDelayMinLineEdit->setEnabled(false);
				m_loopDelayMaxLineEdit->setEnabled(false);
				m_loopDelayMinLineEdit->setText("");
				m_loopDelayMaxLineEdit->setText("");
			}
			break;
		case Sound2dTemplate::LDSR_everySample:
		case Sound2dTemplate::LDSR_initial:
			{
				if (!m_loopDelayMinLineEdit->isEnabled())
				{
					m_loopDelayMinLineEdit->setEnabled(true);
					m_loopDelayMaxLineEdit->setEnabled(true);
					m_loopDelayMinLineEdit->setText("0.00");
					m_loopDelayMaxLineEdit->setText("0.00");
				}
			}
			break;
		default:
			{
				assert(0);
				DEBUG_FATAL(1, ("SoundTemplateWidget::onLoopDelaySampleRateComboBoxActivated() - Unknown loop delay sample rate specified."));
			}
	}
}


//-----------------------------------------------------------------------------
void SoundTemplateWidget::onFadeInSampleRateComboBoxActivated(int index)
{
	switch (static_cast<Sound2dTemplate::FadeInSampleRate>(index))
	{
		case Sound2dTemplate::FISR_noFade:
			{
				m_fadeInMinLineEdit->setEnabled(false);
				m_fadeInMaxLineEdit->setEnabled(false);
				m_fadeInMinLineEdit->setText("");
				m_fadeInMaxLineEdit->setText("");
			}
			break;
		case Sound2dTemplate::FISR_everySample:
		case Sound2dTemplate::FISR_firstSample:
			{
				if (!m_fadeInMinLineEdit->isEnabled())
				{
					m_fadeInMinLineEdit->setEnabled(true);
					m_fadeInMaxLineEdit->setEnabled(true);
					m_fadeInMinLineEdit->setText("0.00");
					m_fadeInMaxLineEdit->setText("0.00");
				}
			}
			break;
		default:
			{
				assert(0);
				DEBUG_FATAL(1, ("SoundTemplateWidget::onFadeInSampleRateComboBoxActivated() - Unknown fade-in sample rate specified."));
			}
	}
}

//-----------------------------------------------------------------------------
void SoundTemplateWidget::onFadeOutSampleRateComboBoxActivated(int index)
{
	switch (static_cast<Sound2dTemplate::FadeOutSampleRate>(index))
	{
		case Sound2dTemplate::FOSR_noFade:
			{
				m_fadeOutMinLineEdit->setEnabled(false);
				m_fadeOutMaxLineEdit->setEnabled(false);
				m_fadeOutMinLineEdit->setText("");
				m_fadeOutMaxLineEdit->setText("");
			}
			break;
		case Sound2dTemplate::FOSR_everySample:
		case Sound2dTemplate::FOSR_lastSample:
			{
				if (!m_fadeOutMinLineEdit->isEnabled())
				{
					m_fadeOutMinLineEdit->setEnabled(true);
					m_fadeOutMaxLineEdit->setEnabled(true);
					m_fadeOutMinLineEdit->setText("0.00");
					m_fadeOutMaxLineEdit->setText("0.00");
				}
			}
			break;
		default:
			{
				DEBUG_FATAL(1, ("SoundTemplateWidget::onFadeOutSampleRateComboBoxActivated() - Unknown fade-out sample rate specified."));
			}
	}
}

//-----------------------------------------------------------------------------
void SoundTemplateWidget::onSpatialityMethodComboBoxActivated(int index)
{
	switch (index)
	{
		case AM_none:
			{
				m_distanceAtMaxVolumeLineEdit->setText("");
				m_distanceAtMaxVolumeLineEdit->setEnabled(false);
			}
			break;
		case AM_2d:
		case AM_3d:
			{
				m_distanceAtMaxVolumeLineEdit->setEnabled(true);

				if (m_distanceAtMaxVolumeLineEdit->text().isEmpty())
				{
					m_distanceAtMaxVolumeLineEdit->setText("2.0");
				}
			}
			break;
		default:
			{
				DEBUG_FATAL(true, ("Unexpected attenuation method specified."));
			}
			break;
	}

	validateAll();
}

//-----------------------------------------------------------------------------
void SoundTemplateWidget::onVolumeSampleRateComboBoxActivated(int index)
{
	switch (static_cast<Sound2dTemplate::VolumeSampleRateType>(index))
	{
		case Sound2dTemplate::VSRT_fullVolume:
			{
				m_volumeSampleRateLineEdit->setEnabled(false);
				m_volumeMinLineEdit->setEnabled(false);
				m_volumeMaxLineEdit->setEnabled(false);
				m_volumeInterpolationRateLineEdit->setEnabled(false);
				m_volumeInterpolationRateLineEdit->setText("");
				m_volumeSampleRateLineEdit->setText("");
				m_volumeMinLineEdit->setText("");
				m_volumeMaxLineEdit->setText("");
			}
			break;
		case Sound2dTemplate::VSRT_everySample:
		case Sound2dTemplate::VSRT_initial:
			{
				m_volumeSampleRateLineEdit->setEnabled(false);
				m_volumeSampleRateLineEdit->setText("");
				m_volumeInterpolationRateLineEdit->setEnabled(false);
				m_volumeInterpolationRateLineEdit->setText("");

				if (!m_volumeMinLineEdit->isEnabled())
				{
					m_volumeMinLineEdit->setEnabled(true);
					m_volumeMaxLineEdit->setEnabled(true);
					m_volumeSampleRateLineEdit->setText("");
					m_volumeMinLineEdit->setText("100");
					m_volumeMaxLineEdit->setText("100");
				}
			}
			break;
		case Sound2dTemplate::VSRT_seconds:
			{
				if (!m_volumeMinLineEdit->isEnabled())
				{
					m_volumeMinLineEdit->setText("100");
					m_volumeMaxLineEdit->setText("100");
				}

				m_volumeSampleRateLineEdit->setEnabled(true);
				m_volumeMinLineEdit->setEnabled(true);
				m_volumeMaxLineEdit->setEnabled(true);
				m_volumeInterpolationRateLineEdit->setEnabled(true);
				m_volumeSampleRateLineEdit->setText("2.00");

				m_volumeInterpolationRateLineEdit->setText("1.00");
			}
			break;
		default:
			{
				DEBUG_FATAL(1, ("SoundTemplateWidget::onVolumeSampleRateComboBoxActivated() - Unknown volume sample rate specified."));
			}
	}
}

//-----------------------------------------------------------------------------
void SoundTemplateWidget::onPitchSampleRateComboBoxActivated(int index)
{
	switch (static_cast<Sound2dTemplate::PitchSampleRateType>(index))
	{
		case Sound2dTemplate::PSRT_basePitch:
			{
				m_pitchSampleRateLineEdit->setEnabled(false);
				m_pitchMinLineEdit->setEnabled(false);
				m_pitchMaxLineEdit->setEnabled(false);
				m_pitchInterpolationRateLineEdit->setEnabled(false);
				m_pitchSampleRateLineEdit->setText("");
				m_pitchMinLineEdit->setText("");
				m_pitchMaxLineEdit->setText("");
				m_pitchInterpolationRateLineEdit->setText("");
			}
			break;
		case Sound2dTemplate::PSRT_everySample:
		case Sound2dTemplate::PSRT_initial:
			{
				m_pitchSampleRateLineEdit->setEnabled(false);
				m_pitchSampleRateLineEdit->setText("");
				m_pitchInterpolationRateLineEdit->setEnabled(false);
				m_pitchInterpolationRateLineEdit->setText("");

				if (!m_pitchMinLineEdit->isEnabled())
				{
					m_pitchMinLineEdit->setEnabled(true);
					m_pitchMaxLineEdit->setEnabled(true);
					m_pitchMinLineEdit->setText("0.00");
					m_pitchMaxLineEdit->setText("0.00");
				}
			}
			break;
		case Sound2dTemplate::PSRT_seconds:
			{
				if (!m_pitchMinLineEdit->isEnabled())
				{
					m_pitchMinLineEdit->setText("0.00");
					m_pitchMaxLineEdit->setText("0.00");
				}

				m_pitchSampleRateLineEdit->setEnabled(true);
				m_pitchMinLineEdit->setEnabled(true);
				m_pitchMaxLineEdit->setEnabled(true);
				m_pitchInterpolationRateLineEdit->setEnabled(true);
				m_pitchSampleRateLineEdit->setText("1.00");
				m_pitchInterpolationRateLineEdit->setText("1.00");
			}
			break;
		default:
			{
				DEBUG_FATAL(1, ("SoundTemplateWidget::onPitchSampleRateComboBoxActivated() - Unknown pitch sample rate specified."));
			}
	}
}

//-----------------------------------------------------------------------------
void SoundTemplateWidget::validateAll()
{
	// Start Delay

	SoundEditorUtility::validateLineEditFloat(m_startDelayMinLineEdit, m_startDelayBound, m_floatingPointPrecision);
	SoundEditorUtility::validateLineEditFloat(m_startDelayMaxLineEdit, m_startDelayBound, m_floatingPointPrecision);
	SoundEditorUtility::checkMinMax(m_startDelayMinLineEdit, m_startDelayMaxLineEdit);

	// Loop Count

	if (m_loopCountMinLineEdit->isEnabled())
	{
		SoundEditorUtility::validateLineEditInt(m_loopCountMinLineEdit, m_loopCountBound);
		SoundEditorUtility::validateLineEditInt(m_loopCountMaxLineEdit, m_loopCountBound);
		SoundEditorUtility::checkMinMax(m_loopCountMinLineEdit, m_loopCountMaxLineEdit);
	}

	// Loop Delay

	if (m_loopDelayMinLineEdit->isEnabled())
	{
		SoundEditorUtility::validateLineEditFloat(m_loopDelayMinLineEdit, m_loopDelayBound, m_floatingPointPrecision);
		SoundEditorUtility::validateLineEditFloat(m_loopDelayMaxLineEdit, m_loopDelayBound, m_floatingPointPrecision);
		SoundEditorUtility::checkMinMax(m_loopDelayMinLineEdit, m_loopDelayMaxLineEdit);
	}

	// Fade In

	if (m_fadeInMinLineEdit->isEnabled())
	{
		SoundEditorUtility::validateLineEditFloat(m_fadeInMinLineEdit, m_fadeInBound, m_floatingPointPrecision);
		SoundEditorUtility::validateLineEditFloat(m_fadeInMaxLineEdit, m_fadeInBound, m_floatingPointPrecision);
		SoundEditorUtility::checkMinMax(m_fadeInMinLineEdit, m_fadeInMaxLineEdit);
	}

	// Fade Out

	if (m_fadeOutMinLineEdit->isEnabled())
	{
		SoundEditorUtility::validateLineEditFloat(m_fadeOutMinLineEdit, m_fadeOutBound, m_floatingPointPrecision);
		SoundEditorUtility::validateLineEditFloat(m_fadeOutMaxLineEdit, m_fadeOutBound, m_floatingPointPrecision);
		SoundEditorUtility::checkMinMax(m_fadeOutMinLineEdit, m_fadeOutMaxLineEdit);
	}

	// Volume

	SoundEditorUtility::validateLineEditFloat(m_volumeSampleRateLineEdit, m_volumeSampleRateBound, m_floatingPointPrecision);
	SoundEditorUtility::validateLineEditFloat(m_volumeInterpolationRateLineEdit, SoundBoundFloat(0.0f, SoundEditorUtility::getFloat(m_volumeSampleRateLineEdit)), m_floatingPointPrecision);

	if (m_volumeMinLineEdit->isEnabled())
	{
		SoundEditorUtility::validateLineEditInt(m_volumeMinLineEdit, m_volumeBound);
		SoundEditorUtility::validateLineEditInt(m_volumeMaxLineEdit, m_volumeBound);
		SoundEditorUtility::checkMinMax(m_volumeMinLineEdit, m_volumeMaxLineEdit);
	}

	// Pitch

	SoundEditorUtility::validateLineEditFloat(m_pitchSampleRateLineEdit, m_pitchSampleRateBound, m_floatingPointPrecision);
	SoundEditorUtility::validateLineEditFloat(m_pitchInterpolationRateLineEdit, SoundBoundFloat(0.0f, SoundEditorUtility::getFloat(m_pitchSampleRateLineEdit)), m_floatingPointPrecision);

	if (m_pitchMinLineEdit->isEnabled())
	{
		SoundEditorUtility::validateLineEditFloat(m_pitchMinLineEdit, m_pitchBound, m_floatingPointPrecision);
		SoundEditorUtility::validateLineEditFloat(m_pitchMaxLineEdit, m_pitchBound, m_floatingPointPrecision);
		SoundEditorUtility::checkMinMax(m_pitchMinLineEdit, m_pitchMaxLineEdit);
	}

	// Distance

	if (m_distanceAtMaxVolumeLineEdit->isEnabled())
	{
		SoundEditorUtility::validateLineEditFloat(m_distanceAtMaxVolumeLineEdit, m_distanceAtMaxVolumeBound, m_floatingPointPrecision);

		float const fallOffDistance = Audio::getFallOffDistance(m_distanceAtMaxVolumeLineEdit->text().toFloat());
		m_minVolumeDistanceTextLabel->setText(QString::number(fallOffDistance));
	}
	else
	{
		m_minVolumeDistanceTextLabel->setText("NA");
	}
}

//-----------------------------------------------------------------------------
void SoundTemplateWidget::onStartDelayMinLineEditReturnPressed()
{
	if (SoundEditorUtility::getFloat(m_startDelayMinLineEdit) > SoundEditorUtility::getFloat(m_startDelayMaxLineEdit))
	{
		m_startDelayMaxLineEdit->setText(m_startDelayMinLineEdit->text());
	}

	validateAll();
}

//-----------------------------------------------------------------------------
void SoundTemplateWidget::onStartDelayMaxLineEditReturnPressed()
{
	if (SoundEditorUtility::getFloat(m_startDelayMaxLineEdit) < SoundEditorUtility::getFloat(m_startDelayMinLineEdit))
	{
		m_startDelayMinLineEdit->setText(m_startDelayMaxLineEdit->text());
	}

	validateAll();
}

//-----------------------------------------------------------------------------
void SoundTemplateWidget::onLoopCountMinLineEditReturnPressed()
{
	if (SoundEditorUtility::getFloat(m_loopCountMinLineEdit) > SoundEditorUtility::getFloat(m_loopCountMaxLineEdit))
	{
		m_loopCountMaxLineEdit->setText(m_loopCountMinLineEdit->text());
	}

	validateAll();
}

//-----------------------------------------------------------------------------
void SoundTemplateWidget::onLoopCountMaxLineEditReturnPressed()
{
	if (SoundEditorUtility::getFloat(m_loopCountMaxLineEdit) < SoundEditorUtility::getFloat(m_loopCountMinLineEdit))
	{
		m_loopCountMinLineEdit->setText(m_loopCountMaxLineEdit->text());
	}

	validateAll();
}

//-----------------------------------------------------------------------------
void SoundTemplateWidget::onLoopDelayMinLineEditReturnPressed()
{
	if (SoundEditorUtility::getFloat(m_loopDelayMinLineEdit) > SoundEditorUtility::getFloat(m_loopDelayMaxLineEdit))
	{
		m_loopDelayMaxLineEdit->setText(m_loopDelayMinLineEdit->text());
	}

	validateAll();
}

//-----------------------------------------------------------------------------
void SoundTemplateWidget::onLoopDelayMaxLineEditReturnPressed()
{
	if (SoundEditorUtility::getFloat(m_loopDelayMaxLineEdit) < SoundEditorUtility::getFloat(m_loopDelayMinLineEdit))
	{
		m_loopDelayMinLineEdit->setText(m_loopDelayMaxLineEdit->text());
	}

	validateAll();
}

//-----------------------------------------------------------------------------
void SoundTemplateWidget::onFadeInMinLineEditReturnPressed()
{
	if (SoundEditorUtility::getFloat(m_fadeInMinLineEdit) > SoundEditorUtility::getFloat(m_fadeInMaxLineEdit))
	{
		m_fadeInMaxLineEdit->setText(m_fadeInMinLineEdit->text());
	}

	validateAll();
}

//-----------------------------------------------------------------------------
void SoundTemplateWidget::onFadeInMaxLineEditReturnPressed()
{
	if (SoundEditorUtility::getFloat(m_fadeInMaxLineEdit) < SoundEditorUtility::getFloat(m_fadeInMinLineEdit))
	{
		m_fadeInMinLineEdit->setText(m_fadeInMaxLineEdit->text());
	}

	validateAll();
}

//-----------------------------------------------------------------------------
void SoundTemplateWidget::onFadeOutMinLineEditReturnPressed()
{
	if (SoundEditorUtility::getFloat(m_fadeOutMinLineEdit) > SoundEditorUtility::getFloat(m_fadeOutMaxLineEdit))
	{
		m_fadeOutMaxLineEdit->setText(m_fadeOutMinLineEdit->text());
	}

	validateAll();
}

//-----------------------------------------------------------------------------
void SoundTemplateWidget::onFadeOutMaxLineEditReturnPressed()
{
	if (SoundEditorUtility::getFloat(m_fadeOutMaxLineEdit) < SoundEditorUtility::getFloat(m_fadeOutMinLineEdit))
	{
		m_fadeOutMinLineEdit->setText(m_fadeOutMaxLineEdit->text());
	}

	validateAll();
}

//-----------------------------------------------------------------------------
void SoundTemplateWidget::onVolumeMinLineEditReturnPressed()
{
	if (SoundEditorUtility::getInt(m_volumeMinLineEdit) > SoundEditorUtility::getInt(m_volumeMaxLineEdit))
	{
		m_volumeMaxLineEdit->setText(m_volumeMinLineEdit->text());
	}

	validateAll();
}

//-----------------------------------------------------------------------------
void SoundTemplateWidget::onVolumeMaxLineEditReturnPressed()
{
	if (SoundEditorUtility::getInt(m_volumeMaxLineEdit) < SoundEditorUtility::getInt(m_volumeMinLineEdit))
	{
		m_volumeMinLineEdit->setText(m_volumeMaxLineEdit->text());
	}

	validateAll();
}

//-----------------------------------------------------------------------------
void SoundTemplateWidget::onPitchMinLineEditReturnPressed()
{
	if (SoundEditorUtility::getFloat(m_pitchMinLineEdit) > SoundEditorUtility::getFloat(m_pitchMaxLineEdit))
	{
		m_pitchMaxLineEdit->setText(m_pitchMinLineEdit->text());
	}

	validateAll();
}

//-----------------------------------------------------------------------------
void SoundTemplateWidget::onPitchMaxLineEditReturnPressed()
{
	if (SoundEditorUtility::getFloat(m_pitchMaxLineEdit) < SoundEditorUtility::getFloat(m_pitchMinLineEdit))
	{
		m_pitchMinLineEdit->setText(m_pitchMaxLineEdit->text());
	}

	validateAll();
}

//-----------------------------------------------------------------------------
void SoundTemplateWidget::onDistanceMinLineEditReturnPressed()
{
	validateAll();
}

//-----------------------------------------------------------------------------
bool SoundTemplateWidget::verifySampleListAll3d()
{
	bool result = true;

	// Make sure all the samples are uncompressed, mono WAV files

	SoundTemplateWidgetListViewItem *child = dynamic_cast<SoundTemplateWidgetListViewItem *>(m_sampleListView->firstChild());

	// Sample filenames

	while (child != NULL)
	{
		if (child->isSampleCompressed() || child->isSampleStereo())
		{
			result = false;
			break;
		}

		child = dynamic_cast<SoundTemplateWidgetListViewItem *>(child->nextSibling());
	}

	return result;
}

//-----------------------------------------------------------------------------
void SoundTemplateWidget::setCaption(QString const &caption)
{
	std::string newCaption("Sound Template - " + std::string(static_cast<char const *>(caption)));
	QWidget::setCaption(newCaption.c_str());
}

//-----------------------------------------------------------------------------
void SoundTemplateWidget::onVolumeInterpolationRateLineEditReturnPressed()
{
	validateAll();
}

//-----------------------------------------------------------------------------
void SoundTemplateWidget::onPitchInterpolationRateLineEditReturnPressed()
{
	validateAll();
}

// ============================================================================
