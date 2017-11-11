// ============================================================================
//
// SoundEditor.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstSoundEditor.h"
#include "SoundEditor.h"
#include "SoundEditor.moc"

#include <string>
#include "AudioDebugWidget.h"
#include "SoundTemplateWidget.h"
#include "SoundGroupTemplateWidget.h"
#include "SoundDebugInformationWidget.h"
#include "SoundEditorUtility.h"
#include "clientAudio/Audio.h"
#include "clientAudio/ConfigClientAudio.h"
#include "clientAudio/SetupClientAudio.h"
#include "clientAudio/SoundTemplateList.h"
#include "clientAudio/Sound2dTemplate.h"
#include "clientAudio/Sound3dTemplate.h"
#include "clientBugReporting/SetupClientBugReporting.h"
#include "clientBugReporting/ToolBugReporting.h"
#include "sharedCompression/SetupSharedCompression.h"
#include "sharedDebug/PerformanceTimer.h"
#include "sharedDebug/SetupSharedDebug.h"
#include "sharedFile/FileNameUtils.h"
#include "sharedFile/Iff.h"
#include "sharedFile/SetupSharedFile.h"
#include "sharedFile/TreeFile.h"
#include "sharedObject/Object.h"
#include "sharedFoundation/ApplicationVersion.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/SetupSharedFoundation.h"
#include "sharedRandom/Random.h"
#include "sharedRandom/SetupSharedRandom.h"
#include "sharedThread/SetupSharedThread.h"
#include "sharedUtility/SetupSharedUtility.h"

#include <map>
#include <vector>

// ============================================================================
//
// SoundEditorNamespace
//
// ============================================================================

namespace SoundEditorNamespace
{
	void    createSpreadSheet();
	void    addSpreadSheetData(QDir const &directory, QFile &file);
	QString getSoundCategoryName(Audio::SoundCategory const soundCategory);
	QString getSoundAttenuationMethodName(Audio::AttenuationMethod const attenuationMethod);

	static QString s_soundTemplateExtension("snd");

	typedef std::map<std::string, int> FileSizeList;
	FileSizeList s_fileSizeList;

	typedef std::multimap<int, std::string> SortedFileSizeList;
	SortedFileSizeList s_sortedFileSizeList;
};

using namespace SoundEditorNamespace;

//-----------------------------------------------------------------------------
QString SoundEditorNamespace::getSoundCategoryName(Audio::SoundCategory const soundCategory)
{
	switch (soundCategory)
	{
		case Audio::SC_ambient:         { return "ambient"; } break;
		case Audio::SC_explosion:       { return "explosion"; } break;
		case Audio::SC_item:            { return "item"; } break;
		case Audio::SC_movement:        { return "movement"; } break;
		case Audio::SC_userInterface:   { return "user interface"; } break;
		case Audio::SC_vehicle:         { return "vehicle"; } break;
		case Audio::SC_vocalization:    { return "vocalization"; } break;
		case Audio::SC_weapon:          { return "weapon"; } break;
		case Audio::SC_backGroundMusic: { return "background music"; } break;
		case Audio::SC_playerMusic:     { return "player music"; } break;
		case Audio::SC_machine:         { return "machine"; } break;
		case Audio::SC_installation:    { return "installation"; } break;
		case Audio::SC_combatMusic:     { return "combat music"; } break;
		default: break;
	}

	return "no category";
}

//-----------------------------------------------------------------------------
QString SoundEditorNamespace::getSoundAttenuationMethodName(Audio::AttenuationMethod const attenuationMethod)
{
	switch (attenuationMethod)
	{
		case Audio::AM_none: { return "stereo"; } break;
		case Audio::AM_2d:   { return "2d"; } break;
		case Audio::AM_3d:   { return "3d"; } break;
		default: break;
	}

	return "no attenuation";
}

//-----------------------------------------------------------------------------
void SoundEditorNamespace::createSpreadSheet()
{
	QDir dir;
	QFile file("soundTemplate_spreadSheet.tab");

	if (file.open(IO_WriteOnly))
	{
		SoundEditorUtility::report("Creating sound template spreadsheet: " + file.name());
		SoundEditorUtility::report("This takes a few minutes");

		{
			QTextStream stream(&file);
		
			stream << "filename" << '\t';
			stream << "sound category" << '\t';
			stream << "priority" << '\t';
			stream << "spatial method" << '\t';
			stream << "audible distance" << '\t';
			stream << "fade in" << '\t';
			stream << "fade out" << '\t';
			stream << "volume" << '\t';
			stream << "pitch" << '\t';
			stream << "# times to play" << '\t';
			stream << "start delay" << '\t';
			stream << "loop delay" << '\t';
			stream << "directory" << '\t';
			stream << endl;
		}

		s_fileSizeList.clear();

		addSpreadSheetData(QDir("../../data/sku.0/sys.client/built"), file);

		// Done writing spreadsheet

		file.close();

		// Process the file size data

		s_sortedFileSizeList.clear();
		FileSizeList::const_iterator iterFileSizeList = s_fileSizeList.begin();
		int cachedCount = 0;

		for (; iterFileSizeList != s_fileSizeList.end(); ++iterFileSizeList)
		{
			int bytes = iterFileSizeList->second;

			s_sortedFileSizeList.insert(std::make_pair(bytes, iterFileSizeList->first));

			if (bytes <= ConfigClientAudio::getMaxCached2dSampleSize())
			{
				++cachedCount;
			}
		}

		file.setName("soundTemplate_sizeInfo.txt");

		if (file.open(IO_WriteOnly))
		{
			QTextStream stream(&file);

			stream << "Non-3d Sample Count:        " << static_cast<int>(s_fileSizeList.size()) << endl;
			stream << "Max Cached 2d Samples Size: " << ConfigClientAudio::getMaxCached2dSampleSize() / 1024 << " KB" << endl;
			stream << "Cached Sample Count:        " << cachedCount << endl;
			stream << "Non-cached Sample Count:    " << (static_cast<int>(s_fileSizeList.size()) - cachedCount) << endl;
			stream << endl;
			stream << "** Non-3d Sample List **" << endl;
			stream << endl;

			SortedFileSizeList::reverse_iterator iterSortedFileSizeList = s_sortedFileSizeList.rbegin();
			int index = 1;

			for (; iterSortedFileSizeList != s_sortedFileSizeList.rend(); ++iterSortedFileSizeList)
			{
				char text[1024];
				sprintf(text, "%4d: %4d KB %s", index, iterSortedFileSizeList->first / 1024, iterSortedFileSizeList->second.c_str());
				stream << text << endl;

				++index;
			}

			file.close();
		}

		SoundEditorUtility::report("DONE creating the spreadsheet.");
	}
	else
	{
		SoundEditorUtility::report("Unable to create the sound spreadsheet, make sure it is not currently open: " + file.name());
	}
}

//-----------------------------------------------------------------------------
void SoundEditorNamespace::addSpreadSheetData(QDir const &directory, QFile &file)
{
	//DEBUG_REPORT_LOG(true, ("%s\n", directory.path().latin1()));
	UNREF(file);

	// Check all the files in this directory

	QStringList files(directory.entryList(QDir::Files, QDir::Name | QDir::IgnoreCase));
	QStringList::Iterator iterFiles = files.begin();

	for (; iterFiles != files.end(); ++iterFiles)
	{
		QFileInfo fileInfo(*iterFiles);
		QString extension(fileInfo.extension());

		if (extension == s_soundTemplateExtension)
		{
			TemporaryCrcString fixupPath((directory.path() + "/" + fileInfo.fileName()).latin1(), true);
			TemporaryCrcString shortestPath(TreeFile::getShortestExistingPath(fixupPath.getString()), true);

			//static int count = 1;
			//DEBUG_REPORT_LOG(true, ("%d %s\n", count++, shortestPath.getString()));

			SoundTemplate const *soundTemplate = SoundTemplateList::fetch(shortestPath.getString());
			
			if (soundTemplate != NULL)
			{
				// Save the file size information

				if (!soundTemplate->is3d())
				{
					SoundTemplate::StringList::const_iterator iterSampleList = soundTemplate->getSampleList().begin();

					for (; iterSampleList != soundTemplate->getSampleList().end(); ++iterSampleList)
					{
						CrcString const *crcString = (*iterSampleList);

						NOT_NULL(crcString);

						if (crcString != NULL)
						{
							int bytes = Audio::getSampleSize(crcString->getString());

							s_fileSizeList.insert(std::make_pair(crcString->getString(), bytes));
						}
					}
				}

				// Save the spreadsheet information

				QTextStream stream(&file);
			
				stream << fileInfo.fileName() << '\t';
				stream << getSoundCategoryName(soundTemplate->getSoundCategory()) << '\t';
				stream << QString::number(soundTemplate->getPriority()) << '\t';
				stream << getSoundAttenuationMethodName(soundTemplate->getAttenuationMethod()) << '\t';

				if (soundTemplate->getAttenuationMethod() == Audio::AM_none)
				{
					stream << "NA" << '\t';
				}
				else
				{
					stream << "[" << QString::number(soundTemplate->getDistanceAtMaxVolume()) << " ... " << QString::number(Audio::getFallOffDistance(soundTemplate->getDistanceAtMaxVolume())) << "]" << '\t';
				}

				// Fade in

				if (soundTemplate->getFadeInSampleRate() == SoundTemplate::FISR_noFade)
				{
					stream << "none" << '\t';
				}
				else
				{
					switch (soundTemplate->getFadeInSampleRate())
					{
						case SoundTemplate::FISR_firstSample: { stream << "first "; } break;
						case SoundTemplate::FISR_everySample: { stream << "every "; } break;
						default:                              { stream << "invalid "; } break;
					}

					stream << "[" << QString::number(soundTemplate->getFadeInMin()) << " ... " << QString::number(soundTemplate->getFadeInMax()) << "]" << '\t';
				}

				// Fade out

				if (soundTemplate->getFadeOutSampleRate() == SoundTemplate::FISR_noFade)
				{
					stream << "none" << '\t';
				}
				else
				{
					switch (soundTemplate->getFadeOutSampleRate())
					{
						case SoundTemplate::FISR_firstSample: { stream << "first "; } break;
						case SoundTemplate::FISR_everySample: { stream << "every "; } break;
						default:                              { stream << "invalid "; } break;
					}

					stream << "[" << QString::number(soundTemplate->getFadeOutMin()) << " ... " << QString::number(soundTemplate->getFadeOutMax()) << "]" << '\t';
				}

				// Volume

				switch (soundTemplate->getVolumeSampleRateType())
				{
					case SoundTemplate::VSRT_fullVolume: { stream << "default"; } break;
					case SoundTemplate::VSRT_initial: { stream << "initial"; } break;
					case SoundTemplate::VSRT_everySample: { stream << "every sample"; } break;
					case SoundTemplate::VSRT_seconds:
						{
							stream << "every "<< QString::number(soundTemplate->getVolumeSampleRate()) << " seconds";
						}
						break;
				}

				if (soundTemplate->getVolumeSampleRateType() != SoundTemplate::VSRT_fullVolume)
				{
					stream << " [" << QString::number(static_cast<int>(soundTemplate->getVolumeMin() * 100.0f)) << " ... " << QString::number(static_cast<int>(soundTemplate->getVolumeMax() * 100.0f)) << "]";
				}

				if (soundTemplate->getVolumeSampleRateType() == SoundTemplate::VSRT_seconds)
				{
					stream << " over " << QString::number(soundTemplate->getVolumeInterpolationRate()) << " seconds";
				}

				stream << '\t';

				// Pitch

				switch (soundTemplate->getPitchSampleRateType())
				{
					case SoundTemplate::PSRT_basePitch: { stream << "default"; } break;
					case SoundTemplate::PSRT_initial: { stream << "initial"; } break;
					case SoundTemplate::PSRT_everySample: { stream << "every sample"; } break;
					case SoundTemplate::PSRT_seconds:
						{
							stream << "every "<< QString::number(soundTemplate->getPitchSampleRate()) << " seconds";
						}
						break;
				}

				if (soundTemplate->getPitchSampleRateType() != SoundTemplate::PSRT_basePitch)
				{
					stream << " [" << QString::number(soundTemplate->getPitchMin()) << " ... " << QString::number(soundTemplate->getPitchMax()) << "]";
				}

				if (soundTemplate->getPitchSampleRateType() == SoundTemplate::PSRT_seconds)
				{
					stream << " over " << QString::number(soundTemplate->getPitchInterpolationRate()) << " seconds";
				}

				stream << '\t';

				// Number of times to play

				if (soundTemplate->getLoopCountMin() == -1)
				{
					stream << "infinite" << '\t';
				}
				else if (soundTemplate->getLoopCountMin() == soundTemplate->getLoopCountMax())
				{
					stream << QString::number(soundTemplate->getLoopCountMin()) << '\t';
				}
				else
				{
					stream << "[" << QString::number(soundTemplate->getLoopCountMin()) << " ... " << QString::number(soundTemplate->getLoopCountMax()) << "]" << '\t';
				}

				// Start delay

				if (soundTemplate->getStartDelayMin() == soundTemplate->getStartDelayMax())
				{
					stream << QString::number(soundTemplate->getStartDelayMin()) << '\t';
				}
				else
				{
					stream << "[" << QString::number(soundTemplate->getStartDelayMin()) << " ... " << QString::number(soundTemplate->getStartDelayMax()) << "]" << '\t';
				}

				// Loop delay

				if (soundTemplate->getLoopDelayMin() == soundTemplate->getLoopDelayMax())
				{
					stream << QString::number(soundTemplate->getLoopDelayMin()) << '\t';
				}
				else
				{
					stream << "[" << QString::number(soundTemplate->getLoopDelayMin()) << " ... " << QString::number(soundTemplate->getLoopDelayMax()) << "]" << '\t';
				}

				// Directory

				stream << directory.path() << '\t';
				stream << endl;

				SoundTemplateList::release(soundTemplate);
			}
		}
	}

	// Check all the directories

	QStringList directories(directory.entryList(QDir::Dirs, QDir::Name | QDir::IgnoreCase));
	QStringList::Iterator iterDirectories = directories.begin();

	for (; iterDirectories != directories.end(); ++iterDirectories)
	{	
		QString path(*iterDirectories);

		if (path != "." &&
		    path != "..")
		{
			QDir newDirectory(directory);
			newDirectory.cd(*iterDirectories);

			addSpreadSheetData(newDirectory, file);
		}
	}
}

// ============================================================================
//
// SoundEditor
//
// ============================================================================

//-----------------------------------------------------------------------------
SoundEditor::SoundEditor(QWidget *parent, char const *name)
 : BaseSoundEditor(parent, name)
 , m_workSpace(NULL)
 , m_debugInformationWidget(NULL)
 , m_audioDebugWidget(NULL)
 , m_workspacePopUpMenu(NULL)
 , m_listener(new Object)
{
	QString caption;
	caption.sprintf("SWG Sound Editor - build %s %s", __DATE__, __TIME__);
	setCaption(caption);

	Audio::setToolApplication(true);
	SetupSharedThread::install();
	SetupSharedDebug::install(4096);

	SetupSharedFoundation::Data data(SetupSharedFoundation::Data::D_mfc);
	data.useWindowHandle    = true;
	data.processMessagePump = false;
	data.windowHandle       = static_cast<HWND>(winId());
	data.configFile         = "client.cfg";
	data.clockUsesSleep     = true;
    data.writeMiniDumps		= ApplicationVersion::isBootlegBuild();
	SetupSharedFoundation::install(data);

	SetupClientBugReporting::install();

    if (ApplicationVersion::isBootlegBuild())
    {
	    ToolBugReporting::startCrashReporter();
    }

	SetupSharedCompression::install();

	SetupSharedFile::install(false);
	SetupSharedRandom::install(static_cast<uint32>(time(NULL)));

	// Utility

	SetupSharedUtility::Data setupUtilityData;
	SetupSharedUtility::setupGameData (setupUtilityData);
	SetupSharedUtility::install (setupUtilityData);
	SetupClientAudio::install();

	// Get rid of the status bar

	statusBar()->hide();

	// Add a workspace

	m_workSpace = new QWorkspace(this);
	setCentralWidget(m_workSpace);

	// Add the debug information widget

	m_debugInformationWidget = new SoundDebugInformationWidget(m_workSpace, "SoundDebugInformationWidget", Qt::WStyle_Customize | Qt::WStyle_DialogBorder | Qt::WStyle_Title);

	SoundEditorUtility::setOutputWindow(*(m_debugInformationWidget->m_outputTextEdit));

	// Add the audio debug widget

	m_audioDebugWidget = new AudioDebugWidget(m_workSpace, "AudioDebugWidget", Qt::WStyle_Customize | Qt::WStyle_DialogBorder | Qt::WStyle_Title);

	// Create the workspace area popup menu

	m_workspacePopUpMenu = new QPopupMenu(this);
	m_workspacePopUpMenu->insertItem("New Sound Template", WSPMO_newSoundTemplate);
	m_workspacePopUpMenu->insertSeparator();
	m_workspacePopUpMenu->insertItem("Open...", WSPMO_open);
	m_workspacePopUpMenu->setCheckable(false);
	connect(m_workspacePopUpMenu, SIGNAL(activated(int)), SLOT(workspacePopUpMenuActivated(int)));

	// Get the sound providers

	std::vector<std::string> providers(Audio::get3dProviders());

	// Display all the supported sound providers

	m_debugInformationWidget->append("Supported 3D sound providers:\n");

	std::vector<std::string>::const_iterator iterProviders = providers.begin();

	for (; iterProviders != providers.end(); ++iterProviders)
	{
		QString text;
		text.sprintf(" - %s\n", iterProviders->c_str());
		m_debugInformationWidget->append(text);
	}
	
	char text[256];

	// Display the Miles version

	sprintf(text, "Miles sound system version: %s\n", Audio::getMilesVersion());
	m_debugInformationWidget->append(text);

	// Display the max digital mixer channels

	sprintf(text, "Max digital mixer channels: %d\n", Audio::getMaxDigitalMixerChannels());
	m_debugInformationWidget->append(text);

	// Load the child widget settings

	SoundEditorUtility::loadWidget(*m_debugInformationWidget, 380, 400, 710, 330);
	SoundEditorUtility::loadWidget(*m_audioDebugWidget, 380, 400, 710, 330);
	SoundEditorUtility::loadWidget(*this, 0, 0, 1097, 838);

	QTimer *timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), SLOT(alter()));
	timer->start(1000 / 24);

	connect(fileCreateSpreadSheetAction, SIGNAL(activated()), this, SLOT(slotFileCreateSpreadSheetActionActivated()));
}

//-----------------------------------------------------------------------------
SoundEditor::~SoundEditor()
{
	SoundEditorUtility::saveWidget(*m_debugInformationWidget);
	SoundEditorUtility::saveWidget(*m_audioDebugWidget);
	SoundEditorUtility::saveWidget(*this);

	QObject *object = NULL;
	
	while ((object = m_workSpace->child("SoundTemplateWidget", NULL, true)) != NULL)
	{
		QObject *parent = object->parent();
		QObjectListIt iterChildren(*parent->children());
		QObject *child = NULL;

		// Delete all the children SoundTemplateWidget objects

		while ((child = iterChildren.current()) != 0)
		{
			++iterChildren;
		
			char const *theObjectName = child->name();
			char const *theClassName = child->className();
			UNREF(theObjectName);

			if (_stricmp(theClassName, "SoundTemplateWidget") == 0)
			{
				QWidget *childWidget = dynamic_cast<QWidget *>(child);
				childWidget->close(true);
			}
		}
	}

	delete m_listener;
	m_listener = NULL;

	// Kickoff the exit chain

	SetupSharedFoundation::remove();
	SetupSharedThread::remove();
}

//-----------------------------------------------------------------------------
void SoundEditor::alter()
{
	static bool altering = false;

	if (altering)
	{
		return;
	}

	altering = true;

	// See if Audio is available, if not tell them to enable it in the client config.

	if (!Audio::isEnabled())
	{
		close();
		QMessageBox::warning(this, "ERROR: Sound Editor", "Audio is disabled and is required for the Sound Editor. To enable audio, set \"disable=0\" in the [ClientAudio] section of client.cfg.", "OK");
		return;
	}

	static PerformanceTimer performanceTimer;

	performanceTimer.stop();
	float const deltaTime = performanceTimer.getElapsedTime();
	performanceTimer.start();

	Audio::alter(deltaTime, m_listener);

	altering = false;
}

//-----------------------------------------------------------------------------
void SoundEditor::fileNewSoundTemplateAction_activated()
{
	SoundTemplateWidget *widget = new SoundTemplateWidget(m_workSpace, "SoundTemplateWidget");

	widget->show();
}

//-----------------------------------------------------------------------------
void SoundEditor::fileExit()
{
	close();
}

//-----------------------------------------------------------------------------
void SoundEditor::fileOpen()
{
	// Get the last used path

	QSettings settings;
	settings.insertSearchPath(QSettings::Windows, SoundEditorUtility::getSearchPath());
	QString previousUsedPath(settings.readEntry("OpenSoundPath", "c:\\"));

	// Get the file

	QStringList selectedFileNames(QFileDialog::getOpenFileNames("Sound files (*.snd)", previousUsedPath, this, "OpenFileDialog", "Open Sound"));

	QStringList::Iterator current = selectedFileNames.begin();
	for (; current != selectedFileNames.end(); ++current)
	{
		std::string path(!(*current).isNull() ? static_cast<char const *>(*current) : "");

		// Make sure the specified file exists on disk and is writable

		if (FileNameUtils::isReadable(path))
		{
			// Save the path

			settings.writeEntry("OpenSoundPath", path.c_str());

			// Make sure this is an iff file

			if (FileNameUtils::isIff(path))
			{
				// Make sure this is the correct kind of iff

				Iff iff(path.c_str());

				if ((iff.getCurrentName() == Sound2dTemplate::getTag()) || (iff.getCurrentName() == Sound3dTemplate::getTag()))
				{
					SoundTemplateWidget *widget = new SoundTemplateWidget(m_workSpace, "SoundTemplateWidget");

					widget->move(0, 0);
					widget->show();
					widget->load(iff);
				}
				else
				{
					char sound2dTemplateTag[256];
					ConvertTagToString(Sound2dTemplate::getTag(), sound2dTemplateTag);

					char sound3dTemplateTag[256];
					ConvertTagToString(Sound3dTemplate::getTag(), sound3dTemplateTag);

					char currentTagString[256];
					ConvertTagToString(iff.getCurrentName(), currentTagString);
					char text[1024];
					sprintf(text, "The specified file (%s) is not a valid (%s) or (%s) IFF file. Initial tag is (%s).", path.c_str(), sound2dTemplateTag, sound3dTemplateTag, currentTagString);
					QMessageBox::warning(this, "File Error", text, "OK");
				}
			}
			else
			{
				char text[1024];
				sprintf(text, "The specified file (%s) is not a valid IFF file.", path.c_str());
				QMessageBox::warning(this, "File Error", text, "OK");
			}
		}
		else if (!path.empty())
		{
			std::string text("The selected file does not exist on disk: \"" + FileNameUtils::get(path, FileNameUtils::fileName | FileNameUtils::extension) + "\"");
			QMessageBox::warning(this, "File Does Not Exist", text.c_str(), "OK");
		}
	}
}

//-----------------------------------------------------------------------------
void SoundEditor::mouseReleaseEvent(QMouseEvent *mouseEvent)
{
	if (mouseEvent == NULL)
	{
		return;
	}

	if (mouseEvent->button() == Qt::RightButton)
	{
		QPoint const &point = mouseEvent->pos();
		QPoint mappedPoint(mapToGlobal(point));
		m_workspacePopUpMenu->exec(mappedPoint);
	}
}

//-----------------------------------------------------------------------------
void SoundEditor::workspacePopUpMenuActivated(int index)
{
	switch (index)
	{
		case WSPMO_newSoundTemplate:
			{
				fileNewSoundTemplateAction_activated();
			}
			break;
		case WSPMO_open:
			{
				fileOpen();
			}
			break;
		default:
			{
				DEBUG_FATAL(1, ("SoundEditor::workspacePopUpMenuActivated() - Unknown workspace popup menu index selected."));
			}
	}
}

//-----------------------------------------------------------------------------
void SoundEditor::slotFileCreateSpreadSheetActionActivated()
{
	createSpreadSheet();
}

// ============================================================================
