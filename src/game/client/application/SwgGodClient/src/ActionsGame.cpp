// ======================================================================
//
// ActionsGame.cpp
// copyright(c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgGodClient/FirstSwgGodClient.h"

#include "ActionsGame.h"
#include "ActionsGame.moc"

#include "ActionHack.h"
#include "GodClientData.h"
#include "IconLoader.h"
#include "MainFrame.h"
#include "RecentDirectory.h"
#include "ServerCommander.h"
#include "clientAudio/Audio.h"
#include "clientGame/ClientWorld.h"
#include "clientGame/Game.h"
#include "clientGame/GroundScene.h"
#include "clientGraphics/Graphics.h"
#include "clientObject/GameCamera.h"
#include "clientTerrain/ClientProceduralTerrainAppearance.h"
#include "clientUserInterface/ConfigClientUserInterface.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiPreferences.h"
#include "sharedFoundation/Os.h"
#include "sharedGame/SharedBuildingObjectTemplate.h"
#include "sharedGame/SharedCreatureObjectTemplate.h"
#include "sharedGame/SharedShipObjectTemplate.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/PortalProperty.h"
#include "sharedObject/World.h"
#include "sharedTerrain/TerrainObject.h"
#include "sharedUtility/FileName.h"
#include "sharedUtility/InteriorLayoutReaderWriter.h"
#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiHud.h"
#include "swgClientUserInterface/SwgCuiHudFactory.h"
#include "swgClientUserInterface/SwgCuiMediatorTypes.h"
#include <qaccel.h>
#include <qfiledialog.h>
#include <qmessagebox.h>

// ======================================================================

ActionsGame::ActionsGame() : 
QObject(),
Singleton<ActionsGame>(),
hud(NULL),
gameFocusAllowed(NULL),
renderWireframe(NULL),
renderTextured(NULL),
showObjectNames(NULL),
showObjectNetworkIds(NULL),
console(NULL),
showDebugMenu(NULL),
watchTargetPath(NULL),
ignoreTargetPath(NULL),
togglePathMap(NULL),
showRegionBrowser(NULL),
purgeTerrain(NULL),
reloadTerrain(NULL),
m_saveInteriorLayout(0)
{
	QWidget* const p = &MainFrame::getInstance();

	//create the "game" actions
	hud                      = new ActionHack("Toggle HUD",             IconLoader::fetchPixmap(IconLoader::hi16_action_hud),  "&HUD",                    QT_ACCEL2(CTRL,Key_H),    p, "hud",                      true );
	gameFocusAllowed         = new ActionHack("Game Focus",             IconLoader::fetchPixmap(IconLoader::hi16_action_gear), "&Game Focus",             QT_ACCEL(Key_Enter),      p, "gameFocus",                true );
//	interiorCameraAllowed    = new ActionHack("Interior Camera",        IconLoader::fetchPixmap(IconLoader::hi16_action_gear), "&Interior Camera",        QT_ACCEL2(CTRL, Key_F11), p, "interiorCameraAllowed",    true );
	console                  = new ActionHack("Toggle Console",         IL_PIXMAP(hi16_action_console),                        "Toggle &Console",         QT_ACCEL(Key_AsciiTilde), p, "toggle_console",           true );
	renderWireframe          = new ActionHack("Render Wireframe",       IL_PIXMAP(hi16_action_renderWireframe),                "Render &Wireframe",       0,                        p, "renderWireframe",          true );
	renderTextured           = new ActionHack("Render Textured",        IL_PIXMAP(hi16_action_renderTextured),                 "Render Te&xtured",        0,                        p, "renderTextured",           true );
	showObjectNames          = new ActionHack("Show Object Names",      IL_PIXMAP(hi16_action_showObjectNames),                "Show Object &Names",      0,                        p, "showObjectNames",          true );
	showObjectNetworkIds     = new ActionHack("Show Object NetworkIds", IL_PIXMAP(hi16_action_showObjectNetworkIds),           "Show Object Network&Ids", 0,                        p, "showObjectNetworkIds",     true );
	reloadTerrain            = new ActionHack("Reload Terrain",         QIconSet(),                                            "&Reload Terrain",         0,                        p, "terrainReload",            false);
	purgeTerrain             = new ActionHack("Purge Terrain",          QIconSet(),                                            "&Purge Terrain",          0,                        p, "terrainPurge",             false);
	showDebugMenu            = new ActionHack("Show Debug Menu",        IL_PIXMAP(hi16_action_console),                        "Show &Debug Menu",        0,                        p, "showDebugMenu",            false);
	watchTargetPath          = new ActionHack("Show Target Path",       QIconSet(),                                            "Show Target &Path",       0,                        p, "showTargetPath",           false);
	ignoreTargetPath         = new ActionHack("Ignore Target Path",     QIconSet(),                                            "&Ignore Target Path",     0,                        p, "ignoreTargetPath",         false);
	togglePathMap            = new ActionHack("Toggle Path Map",        IL_PIXMAP(hi16_action_console),                        "Show &Path Map",          0,                        p, "togglePathMap",            false);
	showRegionBrowser        = new ActionHack("Show RegionBrowser",     QIconSet(),                                            "Show &Region Browser",    0,                        p, "showRegionBrowser",        false);
	m_saveInteriorLayout     = new ActionHack("Save Interior Layout",   QIconSet(),                                            "Save Interior La&yout",   0,                        p, "saveInteriorLayout",       false);

	QAccel * acc = new QAccel(p, "gameFocusAllowed accel");
	IGNORE_RETURN(acc->connectItem(acc->insertItem(QT_ACCEL2(CTRL,Key_F8)), gameFocusAllowed, SLOT(doActivate())));
	IGNORE_RETURN(acc->connectItem(acc->insertItem(QT_ACCEL (Key_QuoteLeft)), console, SLOT(doActivate())));

	//set up initial state of togglable actions
	hud->setOn(true);
	renderTextured->setOn(true);

	IGNORE_RETURN(connect(hud,                        SIGNAL(toggled(bool)), this, SLOT(onToggleHud               (bool))));
	IGNORE_RETURN(connect(console,                    SIGNAL(toggled(bool)), this, SLOT(onToggleConsole           (bool))));
	IGNORE_RETURN(connect(renderWireframe,            SIGNAL(toggled(bool)), this, SLOT(onRenderWireframe         (bool))));
	IGNORE_RETURN(connect(renderTextured,             SIGNAL(toggled(bool)), this, SLOT(onRenderTextured          (bool))));
	IGNORE_RETURN(connect(showObjectNames,            SIGNAL(toggled(bool)), this, SLOT(onShowObjectNames         (bool))));
	IGNORE_RETURN(connect(showObjectNetworkIds,       SIGNAL(toggled(bool)), this, SLOT(onShowObjectNetworkIds    (bool))));
	IGNORE_RETURN(connect(showDebugMenu,              SIGNAL(activated()  ), this, SLOT(onShowDebugMenu           ()    )));
	IGNORE_RETURN(connect(watchTargetPath,            SIGNAL(activated()  ), this, SLOT(onWatchTargetPath         ()    )));
	IGNORE_RETURN(connect(ignoreTargetPath,           SIGNAL(activated()  ), this, SLOT(onIgnoreTargetPath        ()    )));
	IGNORE_RETURN(connect(togglePathMap,              SIGNAL(toggled(bool)), this, SLOT(onTogglePathMap       (bool))));
	IGNORE_RETURN(connect(showRegionBrowser,          SIGNAL(activated()  ), this, SLOT(onShowRegionBrowser       ()    )));
	IGNORE_RETURN(connect(purgeTerrain,               SIGNAL(activated()  ), this, SLOT(onPurgeTerrain            ()    )));
	IGNORE_RETURN(connect(reloadTerrain,              SIGNAL(activated()  ), this, SLOT(onReloadTerrain           ()    )));
	IGNORE_RETURN(connect(m_saveInteriorLayout,       SIGNAL(activated()  ), this, SLOT(onSaveInteriorLayout      ()    )));
}
//----------------------------------------------------------------------

ActionsGame::~ActionsGame()
{
}

//-----------------------------------------------------------------

bool ActionsGame::getHUDEnabled(bool & enabled) const
{
	if(Game::getScene() == NULL)
		return false;	

	SwgCuiHud * const theHud = SwgCuiHudFactory::findMediatorForCurrentHud ();
	enabled = theHud && theHud->getHudEnabled();
	return true;
}

//-----------------------------------------------------------------

void ActionsGame::onToggleHud(bool b) const
{
	if(Game::getScene() == NULL)
		return;	

	SwgCuiHud * const theHud = SwgCuiHudFactory::findMediatorForCurrentHud ();
	if (theHud)
		theHud->setHudEnabled(b);
}

//-----------------------------------------------------------------

void ActionsGame::onRenderWireframe(bool b) const
{
	UNREF(b);
	Graphics::setFillMode(GFM_wire);
	renderTextured->setOn(false);
}

//-----------------------------------------------------------------

void ActionsGame::onRenderTextured(bool b) const
{
	UNREF(b);
	Graphics::setFillMode(GFM_solid);
	renderWireframe->setOn(false);
}

//-----------------------------------------------------------------

void ActionsGame::onShowObjectNames(bool b) const
{
	showObjectNames->setOn(b);
	CuiPreferences::setDrawObjectNames (b);
}

//-----------------------------------------------------------------

void ActionsGame::onShowObjectNetworkIds(bool b) const
{
	showObjectNetworkIds->setOn(b);
	CuiPreferences::setDrawNetworkIds (b ? 1 : 0);
}

//-----------------------------------------------------------------

void ActionsGame::onPurgeTerrain() const
{
	//since we could call this before we log in and generate terrain
	TerrainObject* to = TerrainObject::getInstance();
	if(to)
	{
		Appearance* app = to->getAppearance();
		if(app)
		{
			ClientProceduralTerrainAppearance* const cpta = dynamic_cast<ClientProceduralTerrainAppearance*>(app);
			if (cpta)
				cpta->purgeChunks ();
		}
	}
}

//-----------------------------------------------------------------

void ActionsGame::onReloadTerrain() const
{
	GroundScene* const gs = dynamic_cast<GroundScene*>(Game::getScene ());
	if (gs)
		gs->reloadTerrain ();
}

//-----------------------------------------------------------------

void ActionsGame::onToggleConsole(bool b) const
{
	UNREF(b);
	CuiMediator * const theConsole = NON_NULL(CuiMediatorFactory::get(CuiMediatorTypes::Console));

	if(theConsole->isActive())
		theConsole->deactivate();
	else
		theConsole->activate();
}

//-----------------------------------------------------------------

void ActionsGame::onShowDebugMenu() const
{
	Os::requestPopupDebugMenu();
}

//-----------------------------------------------------------------

void ActionsGame::onWatchTargetPath() const
{
	GodClientData::ObjectList_t olist;
	GodClientData::getInstance().getSelection(olist);
	if(olist.empty())
		return;

	for(GodClientData::ObjectList_t::iterator i = olist.begin(); i != olist.end(); ++i)
	{
		ClientObject * const obj = *(i);
		const NetworkId networkId = obj->getNetworkId();
		ServerCommander::getInstance().watchTargetPath(networkId);
	}
}

//-----------------------------------------------------------------

void ActionsGame::onIgnoreTargetPath() const
{
	GodClientData::ObjectList_t olist;
	GodClientData::getInstance().getSelection(olist);
	if(olist.empty())
		return;

	for(GodClientData::ObjectList_t::iterator i = olist.begin(); i != olist.end(); ++i)
	{
		ClientObject * const obj = *(i);
		const NetworkId networkId = obj->getNetworkId();
		ServerCommander::getInstance().ignoreTargetPath(networkId);
		GodClientData::getInstance().ignoreTargetPath(networkId);
	}
}

//-----------------------------------------------------------------

void ActionsGame::onTogglePathMap(bool b) const
{
	ServerCommander::getInstance().watchPathMap(b);
	GodClientData::getInstance().setRenderAIPaths(b);
}

//-----------------------------------------------------------------

void ActionsGame::onShowRegionBrowser() const
{
	QMessageBox::information(NULL, "Feature disabled", "This feature was disabled in a previous version of the GodClient,\nplease see a programmer about getting it rewritten to meet the current specs.\n", QMessageBox::Ok);
}

//-----------------------------------------------------------------

void ActionsGame::onSaveInteriorLayout() const
{
	const Object* building = 0;

	//-- is an object currently selected, and if so, is it a building?
	if (!GodClientData::getInstance ().isBuildingSelected (&building))
	{
		//-- get the building the player is in instead
		const Object* const player = Game::getPlayer ();
		if (player && !player->isInWorldCell ())
			building = &player->getParentCell ()->getPortalProperty ()->getOwner ();
	}

	if (building && building->getObjectTemplate ())
	{
		QString interiorLayoutFileName;
		
		std::string templateName;
		
		//-- get the name of the file to save
		{
			const SharedBuildingObjectTemplate* const sot = dynamic_cast<const SharedBuildingObjectTemplate*> (building->getObjectTemplate ());
			if (sot)
			{
				FileName fileName (sot->getInteriorLayoutFileName ().c_str ());
				fileName.stripPathAndExt ();
				
				interiorLayoutFileName = fileName;
				
				templateName = sot->getName();
			}
		}
		
		//-- now see if it is a ship
		
		{
			const SharedShipObjectTemplate* const sot = dynamic_cast<const SharedShipObjectTemplate*> (building->getObjectTemplate ());
			if (sot)
			{
				FileName fileName (sot->getInteriorLayoutFileName ().c_str ());
				fileName.stripPathAndExt ();
				
				interiorLayoutFileName = fileName;
				
				templateName = sot->getName();
			}
		}

		interiorLayoutFileName = RecentDirectory::find ("InteriorLayoutFile") + QString("/") + interiorLayoutFileName;
		interiorLayoutFileName = QFileDialog::getSaveFileName(interiorLayoutFileName, "*.ilf", &MainFrame::getInstance (), "", "Select interior layout file");

		if (!interiorLayoutFileName.isEmpty ())
		{
			if (interiorLayoutFileName.find (".ilf") == -1)
				interiorLayoutFileName += ".ilf";

			RecentDirectory::update("InteriorLayoutFile",interiorLayoutFileName);
			InteriorLayoutReaderWriter writer;
			
			const PortalProperty* const portalProperty = building->getPortalProperty ();

			int i;
			for (i = 1; i < portalProperty->getNumberOfCells (); ++i)
			{
				const CellProperty* const cellProperty = portalProperty->getCell (i);
				const char* const cellName = cellProperty->getCellName ();

				ClientWorld::ObjectVector clientObjectList;
				ClientWorld::getObjectsInCell (cellProperty, clientObjectList);

				uint j;
				for (j = 0; j < clientObjectList.size (); ++j)
				{
					const Object* const object = clientObjectList [j];
					if (object->getObjectTemplate ()->getId () != SharedCreatureObjectTemplate::SharedCreatureObjectTemplate_tag)
					{
						const char* const objectTemplateName = object->getObjectTemplate ()->getName ();
						Transform const & transform_o2p = object->getTransform_o2p();

						writer.addObject(cellName, objectTemplateName, transform_o2p);
					}
				}
			}

			if (writer.save (interiorLayoutFileName))
			{
				QString message = "Interior layout for object template ";
				message += templateName.c_str();
				message += " saved to ";
				message += interiorLayoutFileName;
				QMessageBox::information (0, "Success", message, QMessageBox::Ok);
			}
			else
			{
				QString message = "Could not write interior layout file ";
				message += interiorLayoutFileName;
				message += ". Is the file read only?";
				QMessageBox::information (0, "Error", message, QMessageBox::Ok);
			}
		}
	}
	else
		QMessageBox::information (0, "Error", "To save an interior layout, you must either have a single building selected, or your avatar must be in the building.", QMessageBox::Ok);
}

// ================================================================

