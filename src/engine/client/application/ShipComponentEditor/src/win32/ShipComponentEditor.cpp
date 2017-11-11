// ============================================================================
//
// ShipComponentEditor.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstShipComponentEditor.h"
#include "ShipComponentEditor.h"
#include "ShipComponentEditor.moc"

#include "AttachmentsChassisEditor.h"
#include "ChassisEditor.h"
#include "ComponentDescriptors.h"
#include "ConfigShipComponentEditor.h"
#include "ShipComponentEditorServerTemplateManager.h"
#include "ShipComponentEditorUtility.h"
#include "clientAnimation/SetupClientAnimation.h"
#include "clientAudio/Audio.h"
#include "clientAudio/SetupClientAudio.h"
#include "clientDirectInput/DirectInput.h"
#include "clientDirectInput/SetupClientDirectInput.h"
#include "clientGame/SetupClientGame.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/SetupClientGraphics.h"
#include "clientObject/SetupClientObject.h"
#include "clientParticle/SetupClientParticle.h"
#include "clientSkeletalAnimation/SetupClientSkeletalAnimation.h"
#include "sharedCompression/SetupSharedCompression.h"
#include "sharedDebug/PerformanceTimer.h"
#include "sharedDebug/SetupSharedDebug.h"
#include "sharedFile/FileNameUtils.h"
#include "sharedFile/Iff.h"
#include "sharedFile/SetupSharedFile.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/SetupSharedFoundation.h"
#include "sharedGame/SetupSharedGame.h"
#include "sharedGame/ShipChassis.h"
#include "sharedGame/ShipComponentAttachmentManager.h"
#include "sharedGame/ShipComponentDescriptor.h"
#include "sharedImage/SetupSharedImage.h"
#include "sharedIoWin/SetupSharedIoWin.h"
#include "sharedMath/SetupSharedMath.h"
#include "sharedObject/Object.h"
#include "sharedObject/SetupSharedObject.h"
#include "sharedRandom/Random.h"
#include "sharedRandom/SetupSharedRandom.h"
#include "sharedThread/SetupSharedThread.h"
#include "sharedUtility/DataTableWriter.h"
#include "sharedUtility/OptionManager.h"
#include "sharedUtility/SetupSharedUtility.h"
#include <map>
#include <qtabwidget.h>
#include <string>
#include <vector>

// ============================================================================
//
// ShipComponentEditorNamespace
//
// ============================================================================

namespace ShipComponentEditorNamespace
{
	typedef std::map<std::string, int> FileSizeList;
	FileSizeList s_fileSizeList;

	typedef std::multimap<int, std::string> SortedFileSizeList;
	SortedFileSizeList s_sortedFileSizeList;

	enum // WorkSpacePopupMenuOptions
	{
		WSPMO_reloadTemplates,
		WSPMO_regenerateTemplateDb,
		WSPMO_p4edit,
		WSPMO_saveAll
	};

};

using namespace ShipComponentEditorNamespace;

// ============================================================================
//
// ShipComponentEditor
//
// ============================================================================

//-----------------------------------------------------------------------------
ShipComponentEditor::ShipComponentEditor(QWidget *parent, char const *name)
 : BaseShipComponentEditor(parent, name)
 , m_workSpace(NULL)
 , m_workspacePopUpMenu(NULL)
 , m_listener(new Object)
{
	installEngine();

	QString caption;
	caption.sprintf("SWG ShipComponent Editor - build %s %s", __DATE__, __TIME__);
	setCaption(caption);

	// Get rid of the status bar

	statusBar()->hide();

	// Add a workspace

	/*
	m_workSpace = new QWorkspace(this);
	*/
	QTabWidget * const m_tabs = new QTabWidget(this);
//	setCentralWidget(m_workSpace);
	setCentralWidget(m_tabs);


	ChassisEditor * const chassisEditor = new ChassisEditor(m_tabs, "Chassis Editor Widget");//, Qt::WStyle_Customize | Qt::WStyle_DialogBorder | Qt::WStyle_Title);
	m_tabs->addTab(chassisEditor, "Chassis Editor");
	UNREF(chassisEditor);

	ComponentDescriptors * const componentDescriptors = new ComponentDescriptors(m_tabs, "Chassis Editor Widget");//, Qt::WStyle_Customize | Qt::WStyle_DialogBorder | Qt::WStyle_Title);
	m_tabs->addTab(componentDescriptors, "Component Descriptors");
	UNREF(componentDescriptors);

	AttachmentsChassisEditor * const attachmentsChassisEditor = new AttachmentsChassisEditor(m_tabs, "Attachments (chassis) Editor Widget");//, Qt::WStyle_Customize | Qt::WStyle_DialogBorder | Qt::WStyle_Title);
	m_tabs->addTab(attachmentsChassisEditor, "Attachments by Chassis");

	//-- this menu is populated here because the Qt Designer has a bug that corrupts the .ui file when editing the menu bar
	
	QPopupMenu * const menuFile = new QPopupMenu(this);
	menuFile->insertItem("&P4 Edit Files", this, SLOT(onMenuP4EditFiles()), CTRL+Key_P);
	menuFile->insertItem("Save All", this, SLOT(onMenuSaveAll()), CTRL+Key_S);
	menuFile->insertSeparator();
	menuFile->insertItem("Reload &Templates", this, SLOT(onMenuReloadTemplates()), CTRL+Key_T);
	menuFile->insertItem("Regen Template &DB", this, SLOT(onMenuRegenerateTemplateDb()), CTRL+Key_D);
	menuFile->insertSeparator();
	menuFile->insertItem("E&xit", this, SLOT(onMenuExit()));
	QMenuBar * const menu = menuBar();
	menu->clear();
	menu->insertItem("File", menuFile);

	// Load the child widget settings

	ShipComponentEditorUtility::loadWidget(*this, 0, 0, 1097, 838);

	QTimer *timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), SLOT(alter()));
	timer->start(1000 / 24);
}

//----------------------------------------------------------------------
//-- SLOTS
//----------------------------------------------------------------------

void ShipComponentEditor::onMenuReloadTemplates()
{
	ShipComponentEditorServerTemplateManager::load();
}

//----------------------------------------------------------------------

void ShipComponentEditor::onMenuRegenerateTemplateDb()
{
	ShipComponentEditorServerTemplateManager::regenerateTemplateDb();
}

//----------------------------------------------------------------------

void ShipComponentEditor::onMenuSaveAll()
{
	bool ok = true;

	{
		std::string chassisFilename = ConfigShipComponentEditor::getSharedPathDsrc();
		chassisFilename.push_back('/');
		chassisFilename += ShipChassis::getFilename();
		
		if (chassisFilename.find(".iff") != (chassisFilename.size() - 4))
		{
			ok = false; 
			QMessageBox::warning(this, 
				"ShipComponentEditor invalid chassis filename", 
				"ShipComponentEditor invalid chassis filename.");
		}
		else
		{
			chassisFilename.replace(chassisFilename.size() - 4, 4, ".tab");
			
			if (!ShipChassis::save(chassisFilename))
			{
				ok = false; 
				QMessageBox::warning(this, 
					"ERROR: Failed to write ShipChasis table", 
					"Failed to write ShipChasis table.\nMake sure the file is checked out.\nMake sure the file is not opened in another editor ");
			}
			else
			{
				DataTableWriter dtw;
				dtw.loadFromSpreadsheet(chassisFilename.c_str());
				std::string const & outputFilename = ConfigShipComponentEditor::getSharedPathData() + "/" + ShipChassis::getFilename();
				if (!dtw.save(outputFilename.c_str(), true))
				{
					ok = false; 
					QMessageBox::warning(this, 
						"ERROR: Failed to compile ShipChasis table", 
						"An unknown error occurred while compiling the ShipChassis table");
				}
			}
		}
	}
	
	//----------------------------------------------------------------------
	
	{
		std::string componentFilename = ConfigShipComponentEditor::getSharedPathDsrc();
		componentFilename .push_back('/');
		componentFilename  += ShipComponentDescriptor::getFilename();
		
		if (componentFilename .find(".iff") != (componentFilename .size() - 4))
		{
			ok = false; 
			QMessageBox::warning(this, 
				"ShipComponentEditor invalid ShipComponentDescriptor filename", 
				"ShipComponentEditor invalid ShipComponentDescriptor filename.");
		}
		else
		{
			componentFilename .replace(componentFilename .size() - 4, 4, ".tab");
			
			if (!ShipComponentDescriptor::save(componentFilename ))
			{
				ok = false; 
				QMessageBox::warning(this, 
					"ERROR: Failed to write ShipComponentDescriptor table", 
					"Failed to write ShipComponentDescriptor table.\nMake sure the file is checked out.\nMake sure the file is not opened in another editor ");
			}
			else
			{
				DataTableWriter dtw;
				dtw.loadFromSpreadsheet(componentFilename .c_str());
				std::string const & outputFilename = ConfigShipComponentEditor::getSharedPathData() + "/" + ShipComponentDescriptor::getFilename();
				if (!dtw.save(outputFilename.c_str(), true))
				{
					ok = false; 
					QMessageBox::warning(this, 
						"ERROR: Failed to compile ShipChasis table", 
						"An unknown error occurred while compiling the ShipChassis table");
				}
			}
		}
	}

	//----------------------------------------------------------------------

	{
		if (!ShipComponentAttachmentManager::save(ConfigShipComponentEditor::getSharedPathDsrc(), ConfigShipComponentEditor::getSharedPathData()))
		{
			ok = false;
			QMessageBox::warning(this, 
				"ShipComponentEditor Attachment save failed", 
				"ShipComponentEditor Attachment save failed.\nCheck DebugView warnings for more details.");
		}
	}

	//----------------------------------------------------------------------
	
	if (ok)
	{
		QMessageBox::information(this, 
			"Save Successful", 
			"Saved of Chassis & Components Successful.");
	}
}

//----------------------------------------------------------------------

void ShipComponentEditor::onMenuExit()
{
	qApp->quit();
}

//----------------------------------------------------------------------

void ShipComponentEditor::onMenuP4EditFiles()
{
	char buf[1024];
	size_t const buf_size = sizeof(buf);
	snprintf(buf, buf_size, 
		"p4 edit %s/datatables/space/ship_... %s/datatables/space/ship_...", 
		ConfigShipComponentEditor::getSharedPathDsrc().c_str(), ConfigShipComponentEditor::getSharedPathData().c_str());
	system(buf);
}

//----------------------------------------------------------------------
//-- END SLOTS
//----------------------------------------------------------------------

//----------------------------------------------------------------------

void ShipComponentEditor::installEngine()
{
	//-- ignore settings. don't load or save them
	OptionManager::setOptionManagersEnabled(false);

	//-- we need to tell the Ship Chassis system to load instances of writable chassis rather than the immutable ones
	ShipChassis::setUseWritableChassis(true);
	ShipComponentDescriptor::setUseWritableComponentDescriptor(true);

	Audio::setToolApplication(true);

	SetupSharedThread::install();
	SetupSharedDebug::install(4096);
	
	SetupSharedFoundation::Data data(SetupSharedFoundation::Data::D_mfc);
	data.useWindowHandle    = true;
	data.processMessagePump = false;
	data.windowHandle       = static_cast<HWND>(winId());
	data.configFile         = "ShipComponentEditor.cfg";
	data.clockUsesSleep     = true;
	SetupSharedFoundation::install(data);
		
	ConfigShipComponentEditor::install();
	std::string configTestResult;
	if (!ConfigShipComponentEditor::testFolders(configTestResult))
	{
		QMessageBox::warning(this, QString("Errors in paths"), QString((std::string("The paths are invalid:\n" + configTestResult)).c_str()));
	}

	//-- file
	SetupSharedFile::install(true);
		
	//-- math
	SetupSharedMath::install();
	
	//-- utility
	SetupSharedUtility::Data setupUtilityData;
	SetupSharedUtility::setupGameData (setupUtilityData);
	setupUtilityData.m_allowFileCaching = true;
	SetupSharedUtility::install (setupUtilityData);
	
//	SetupSharedIoWin::install();

	//-- random
	SetupSharedRandom::install(static_cast<uint32>(time(NULL)));
		
	//-- image
	SetupSharedImage::Data setupImageData;
	SetupSharedImage::setupDefaultData (setupImageData);
	SetupSharedImage::install (setupImageData);
		
	//-- object
	SetupSharedObject::Data setupObjectData;
	SetupSharedObject::setupDefaultGameData (setupObjectData);
	setupObjectData.useTimedAppearanceTemplates = true;
	// we want the SlotIdManager initialized, and we need the associated hardpoint names loaded.
	SetupSharedObject::addSlotIdManagerData(setupObjectData, true);
	// we want CustomizationData support on the client.
	SetupSharedObject::addCustomizationSupportData(setupObjectData);
	SetupSharedObject::install (setupObjectData);
	
	ShipComponentEditorServerTemplateManager::install();

	//-- game
	SetupSharedGame::Data setupSharedGameData;
	setupSharedGameData.setUseGameScheduler (true);
	setupSharedGameData.setUseMountValidScaleRangeTable (true);
	SetupSharedGame::install (setupSharedGameData);
	
	//-- terrain
//	SetupSharedTerrain::Data setupSharedTerrainData;
//	SetupSharedTerrain::setupGameData(setupSharedTerrainData);
//	SetupSharedTerrain::install(setupSharedTerrainData);
	
//	SetupSharedXml::install();
	
	SetupClientAudio::install();

	SetupClientGraphics::Data setupGraphicsData;
	SetupClientGraphics::setupDefaultGameData(setupGraphicsData);
	//setupGraphicsData.screenWidth  = 1024;
	//setupGraphicsData.screenHeight = 768;
	//setupGraphicsData.alphaBufferBitDepth = 0;
	SetupClientGraphics::install(setupGraphicsData);
	
	//-- directinput
	SetupClientDirectInput::install(GetModuleHandle(NULL), Os::getWindow(), DIK_LCONTROL, Graphics::isWindowed);
//	DirectInput::setScreenShotFunction(ScreenShotHelper::screenShot);
	DirectInput::suspendInput();
	
	//-- object
	SetupClientObject::Data setupClientObjectData;
	SetupClientObject::setupGameData (setupClientObjectData);
	SetupClientObject::install (setupClientObjectData);

	SetupClientAnimation::install();

	SetupClientSkeletalAnimation::Data skeletalData;
	SetupClientSkeletalAnimation::install(skeletalData);

	//-- particle system
	SetupClientParticle::install ();

	SetupClientGame::Data clientGameData;
	SetupClientGame::install(clientGameData);

	//ServerShipObjectTemplate::install();
}

//----------------------------------------------------------------------

void ShipComponentEditor::alter()
{
	static bool altering = false;

	if (altering)
	{
		return;
	}

	// See if Audio is available, if not tell them to enable it in the client config.

	if (!Audio::isEnabled())
	{
		QMessageBox::warning(this, "ERROR: Ship Component Editor", "Audio is disabled.");
		return;
	}

	altering = true;
	static PerformanceTimer performanceTimer;

	performanceTimer.stop();
	float const deltaTime = performanceTimer.getElapsedTime();
	performanceTimer.start();

	Audio::alter(deltaTime, m_listener);

	altering = false;
}

//-----------------------------------------------------------------------------

ShipComponentEditor::~ShipComponentEditor()
{
	ShipComponentEditorUtility::saveWidget(*this);
	
	delete m_listener;
	m_listener = NULL;

	// Kickoff the exit chain

	SetupSharedFoundation::remove();
	SetupSharedThread::remove();
}

//-----------------------------------------------------------------------------

void ShipComponentEditor::mouseReleaseEvent(QMouseEvent *mouseEvent)
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

// ============================================================================
