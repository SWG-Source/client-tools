// ======================================================================
//
// MainWindow.cpp
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "ClientEffectEditor/FirstClientEffectEditor.h"
#include "ClientEffectEditor/MainWindow.h"

#include "sharedFile/Iff.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/Object.h"

#include "clientDirectInput/ForceFeedbackEffectTemplateList.h"
#include "clientGame/ClientEffect.h"
#include "clientGame/Game.h"

#include <qfiledialog.h>
#include <qlistview.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qsettings.h>
#include <qstring.h>

#include <vector>

// ======================================================================

namespace MainWindowNamespace
{
	int const cms_nameColumn = 0;
	int const cms_valueColumn = 1;

	QString const cms_particleAppearanceBaseItemName("Particle Appearances");
	QString const cms_soundBaseItemName("Sounds");
	QString const cms_cameraShakeBaseItemName("Camera Shakes");
	QString const cms_lightBaseItemName("Lights");
	QString const cms_forceFeedbackBaseItemName("Force Feedback Effects");

	QString const cms_clientEffectEditorCaption("ClientEffectEditor");

	QString ms_utilityString;

	QString ms_currentFile;

	std::vector<QListViewItem *> ms_baseItems;

	std::string const cms_clientEffectExtension("cef");
	std::string const cms_true("true");
	std::string const cms_false("false");

	void deleteAllListViewChildren(QListViewItem const * const base)
	{
		if(base)
		{
			QListViewItem * node = base->firstChild();
			while(node)
			{
				delete node;
				node = base->firstChild();
			}
		}
	}

	void closeAllItemChildren(QListViewItem const * const base)
	{
		if(base)
		{
			QListViewItem * node = base->firstChild();
			while(node)
			{
				node->setOpen(false);
				node = node->nextSibling();
			}
		}
	}

	QListViewItem * getLastListViewItemChild(QListViewItem const * const base)
	{
		QListViewItem * lastChild = NULL;
		if(base)
		{
			QListViewItem * node = base->firstChild();
			while(node)
			{
				lastChild = node;
				node = node->nextSibling();
			}
		}
		return lastChild;
	}

	MainWindow * ms_theMainWindow = NULL;

	MainWindow & getInstance()
	{                                  //lint !e1929 returning a reference (this is ok, it's there for the whole run)
		NOT_NULL(ms_theMainWindow);
		return *ms_theMainWindow;
	}

	enum MenuOptions
	{
		MO_new,
		MO_delete
	};
}

using namespace MainWindowNamespace;

// ======================================================================

class ClientEffectEditorListViewItem : public QListViewItem
{
public:
	enum Type
	{
		DT_floatGreaterThanZero,
		DT_int0To255,
		DT_intGreaterThanZero,
		DT_bool,
		DT_entryMainNode,
		DT_none
	};

public:
	ClientEffectEditorListViewItem(QListViewItem * parent, QString label1, QString label2 = QString::null, QString label3 = QString::null, QString label4 = QString::null, QString label5 = QString::null, QString label6 = QString::null, QString label7 = QString::null, QString label8 = QString::null);
	ClientEffectEditorListViewItem(QListViewItem * parent, QListViewItem * after, QString label1, QString label2 = QString::null, QString label3 = QString::null, QString label4 = QString::null, QString label5 = QString::null, QString label6 = QString::null, QString label7 = QString::null, QString label8 = QString::null);
	void setType(Type const & dt);
	Type getType() const;

protected:
	virtual void okRename(int col);

private:
	//disabled
	ClientEffectEditorListViewItem();

private:
	Type m_dataType;
};

// ----------------------------------------------------------------------

ClientEffectEditorListViewItem::ClientEffectEditorListViewItem(QListViewItem * const theParent, QString const label1, QString const label2, QString const label3, QString const label4, QString const label5, QString const label6, QString const label7, QString const label8)
: QListViewItem(theParent, label1, label2, label3, label4, label5, label6, label7, label8),
  m_dataType(DT_none)
{
} //lint !e1746 labels could be const (yes, but we can't change the signature for an overridden function)

// ----------------------------------------------------------------------

ClientEffectEditorListViewItem::ClientEffectEditorListViewItem(QListViewItem * const theParent, QListViewItem * const after, QString const label1, QString const label2, QString const label3, QString const label4, QString const label5, QString const label6, QString const label7, QString const label8)
: QListViewItem(theParent, after, label1, label2, label3, label4, label5, label6, label7, label8),
  m_dataType(DT_none)
{
} //lint !e1746 labels could be const (yes, but we can't change the signature for an overridden function)

// ----------------------------------------------------------------------

void ClientEffectEditorListViewItem::setType(ClientEffectEditorListViewItem::Type const & dt)
{
	m_dataType = dt;
}

// ----------------------------------------------------------------------

ClientEffectEditorListViewItem::Type ClientEffectEditorListViewItem::getType() const
{
	return m_dataType;
}

// ----------------------------------------------------------------------

void ClientEffectEditorListViewItem::okRename(int const col)
{
	//now run the base call to make the actual change to data
	QListViewItem::okRename(col);

	DEBUG_FATAL(col != cms_valueColumn, ("Edited non-value column.  This shouldn't be possible."));
	QString newValue = text(cms_valueColumn);

	if(m_dataType == DT_bool)
	{
		//massage the value if needed
		if(newValue != cms_true.c_str() && newValue != cms_false.c_str())
		{
			if(newValue == "T" || newValue == "t" || newValue == "1" || newValue == "TRUE")
				newValue = cms_true.c_str();
			else if(newValue == "F" || newValue == "f" || newValue == "0" || newValue == "FALSE")
				newValue = cms_false.c_str();
			else
			{
				IGNORE_RETURN(QMessageBox::warning(&getInstance(), "Bad boolean value", "Not a valid boolean value, setting to FALSE"));
				newValue = cms_false.c_str();
			}
			setText(1, newValue);
		}
	}
	else if(m_dataType == DT_floatGreaterThanZero)
	{
		bool ok;
		float const value = newValue.toFloat(&ok);
		if(!ok)
		{
			IGNORE_RETURN(QMessageBox::warning(&getInstance(), "Bad float value", "Not a valid float, setting to 0.0"));
			setText(1, "0.0");
		}
		if(value < 0.0f)
		{
			IGNORE_RETURN(QMessageBox::warning(&getInstance(), "Bad float value", "Value must be > 0.0, setting to 0.0"));
			setText(1, "0.0");
		}
	}
	else if(m_dataType == DT_intGreaterThanZero)
	{
		bool ok;
		float const value = newValue.toInt(&ok);
		if(!ok)
		{
			IGNORE_RETURN(QMessageBox::warning(&getInstance(), "Bad int value", "Not a valid int, setting to 0"));
			setText(1, "0");
		}
		if(value < 0)
		{
			IGNORE_RETURN(QMessageBox::warning(&getInstance(), "Bad int value", "Value must be greater than 0, setting to 0"));
			setText(1, "0");
		}
	}
	else if(m_dataType == DT_int0To255)
	{
		bool ok;
		float const value = newValue.toInt(&ok);
		if(!ok)
		{
			IGNORE_RETURN(QMessageBox::warning(&getInstance(), "Bad int value", "Not a valid int, setting to 0"));
			setText(1, "0");
		}
		if(value < 0 || value > 255)
		{
			IGNORE_RETURN(QMessageBox::warning(&getInstance(), "Bad int value", "Value must be between 0 and 255, setting to 0"));
			setText(1, "0");
		}
	}
	else if(m_dataType == DT_entryMainNode)
	{
		DEBUG_FATAL(true, ("Renaming the main node for an entry, only this node's children should be renamable"));
	}
	else if(m_dataType == DT_none)
	{
		DEBUG_FATAL(true, ("Renaming an item that has no type set."));
	}
	//update the clienteffect
	getInstance().copyGUIToIff();
	getInstance().m_effectTemplateModified = true;
}

// ======================================================================
// START CONSTRUCTORS/DESTRUCTORS

MainWindow::MainWindow(QWidget * const newParent, char const * newName, WFlags const fl)
: BaseMainWindow(newParent, newName, fl),
  m_effectTemplate(),
  m_effectTemplateModified(false),
  m_avatar(NULL)
{
	m_listView->setSorting(-1);

	//ensure all root items already exist
	QListViewItem * item = m_listView->findItem(cms_particleAppearanceBaseItemName, cms_nameColumn);
	DEBUG_FATAL(!item, ("Could not find the expected \"Particle Appearances\" item in m_listView, this is set in the QT Designer."));
	ms_baseItems.push_back(item);
	item = m_listView->findItem(cms_soundBaseItemName, cms_nameColumn);
	DEBUG_FATAL(!item, ("Could not find the expected \"Sounds\" item in m_listView, this is set in the QT Designer."));
	ms_baseItems.push_back(item);
	item = m_listView->findItem(cms_cameraShakeBaseItemName, cms_nameColumn);
	DEBUG_FATAL(!item, ("Could not find the expected \"Camera Shakes\" item in m_listView, this is set in the QT Designer."));
	ms_baseItems.push_back(item);
	item = m_listView->findItem(cms_lightBaseItemName, cms_nameColumn);
	DEBUG_FATAL(!item, ("Could not find the expected \"Lights\" item in m_listView, this is set in the QT Designer."));
	ms_baseItems.push_back(item);
	item = m_listView->findItem(cms_forceFeedbackBaseItemName, cms_nameColumn);
	DEBUG_FATAL(!item, ("Could not find the expected \"Force Feedback Effects\" item in m_listView, this is set in the QT Designer."));
	ms_baseItems.push_back(item);

	//Set the single MainWindow instance for the app.
	DEBUG_FATAL(ms_theMainWindow, ("A main window has been set already, that shouldn't happen."));
	ms_theMainWindow = this;

	Object * const childObject = new Object;
	childObject->setPosition_p(Vector(1.0f, 1.0f, 0.0f));
	Game::getPlayer()->addChildObject_o(childObject);

	m_avatar = childObject;

	updateCaption();
}

// ----------------------------------------------------------------------

MainWindow::~MainWindow()
{
	ms_theMainWindow = 0;
	m_avatar = NULL;
	ms_baseItems.clear();
}

// END CONSTRUCTORS/DESTRUCTORS
// ======================================================================
// START PUBLIC SLOT HANDLERS

void MainWindow::newClientEffect()
{
	bool clearData = false;

	if(m_effectTemplateModified)
	{
		//give the user a chance to cancel (since it will destroy unsaved work)
		QMessageBox mb("Are you sure?", "Discard changes to the current clienteffect and create a blank one?", QMessageBox::Information, QMessageBox::Yes, QMessageBox::No | QMessageBox::Default, QMessageBox::Cancel | QMessageBox::Escape );
		switch(mb.exec())
		{
			//save, and if successful, clear the effect
			case QMessageBox::Yes:
				IGNORE_RETURN(saveClientEffect());
				clearData = true;
				break;

			//clear the effect
			case QMessageBox::No:
				clearData = true;
				break;

			//don't do jack
			case QMessageBox::Cancel:
			default:
				break;
		}
	}
	else
	{
		clearData = true;
	}

	if(clearData)
	{
		resetClientEffect();
		ms_currentFile = QString::null;
		updateCaption();
	}
}

// ----------------------------------------------------------------------

void MainWindow::loadClientEffect()
{
	if(!closeClientEffect())
		return;

	QSettings settings;
	settings.insertSearchPath(QSettings::Windows, "/SOE/ClientEffectEditor");
	QString previousUsedPath(settings.readEntry("OpenClientEffectPath", "c:\\"));

	//Get file to open from the Qt common file dialog.
	ms_currentFile = QFileDialog::getOpenFileName(previousUsedPath, "ClientEffect Files (*.cef)", this, "open file dialog", "Choose a file to open");
	if (ms_currentFile == QString::null)
		return;

	IGNORE_RETURN(settings.writeEntry("OpenClientEffectPath", ms_currentFile));

	//Try to convert Qt full pathname to a TreeFile-relative path.  This allows the
	//data file "name" attribute to match what other assets will load in the game,
	//causing the editor and the game to use the same (potentially modified) data assets.
	std::string treeFileRelativePath;

	if (!TreeFile::stripTreeFileSearchPathFromFile(std::string(ms_currentFile), treeFileRelativePath))
	{
		WARNING(true, ("User: the specified file [%s] is not mappable to your TreeFile path.  Fix path before opening.", static_cast<char const *>(ms_currentFile)));
		return;
	}

	//Check if specified file exists.
	if (!TreeFile::exists(treeFileRelativePath.c_str()))
	{
		// File doesn't exist, exit here.
		REPORT_LOG(true, ("Specified file [%s] does not exist, skipping.", static_cast<char const *>(ms_currentFile)));
		return;
	}

	//Check extension.
	std::string::size_type const extensionStartPosition = treeFileRelativePath.rfind('.');
	if (static_cast<int>(extensionStartPosition) == static_cast<int>(std::string::npos))
	{
		WARNING(true, ("Could not determine a file type because specified path [%s] has no extension.", treeFileRelativePath.c_str()));
		return;
	}

	std::string const extension(treeFileRelativePath, extensionStartPosition + 1);
	if(extension == cms_clientEffectExtension.c_str())
	{
		resetClientEffect();
		IGNORE_RETURN(loadPathedClientEffect(treeFileRelativePath));
	}
	else
	{
		WARNING(true, ("openFile(): unsupported extension [%s] on file [%s].", extension.c_str(), treeFileRelativePath.c_str()));
	}

	updateCaption();
}

// ----------------------------------------------------------------------

bool MainWindow::saveClientEffect()
{
	if(m_effectTemplateModified)
	{
		QString const pathName = QFileDialog::getSaveFileName(ms_currentFile, "ClientEffect Files (*.cef)", this, "save file dialog", "Choose a file to save into");
		if (pathName == QString::null)
			return false;

		std::string const path(pathName);

		//check extension.
		std::string::size_type const extensionStartPosition = path.rfind('.');
		if (static_cast<int>(extensionStartPosition) == static_cast<int>(std::string::npos))
		{
			WARNING(true, ("Could not determine a file type because specified path [%s] has no extension.", path.c_str()));
			return false;
		}

		std::string const extension(path, extensionStartPosition + 1);
		if(extension == cms_clientEffectExtension.c_str())
		{
			IGNORE_RETURN(savePathedClientEffect(path));
		}
		else
		{
			WARNING(true, ("openFile(): unsupported extension [%s] on file [%s].", extension.c_str(), path.c_str()));
			return false;
		}
	}

	return true;
}

// ----------------------------------------------------------------------

bool MainWindow::closeClientEffect()
{
	if(m_effectTemplateModified)
	{
		//give the user a chance to cancel (since it will destroy unsaved work)
		QMessageBox mb("Are you sure?", "Save before closing?", QMessageBox::Information, QMessageBox::Yes, QMessageBox::No | QMessageBox::Default, QMessageBox::Cancel | QMessageBox::Escape );
		switch(mb.exec())
		{
			//save, and if successful, clear the effect
			case QMessageBox::Yes:
				if (saveClientEffect())
					resetClientEffect();
				break;

			//clear the effect
			case QMessageBox::No:
				resetClientEffect();
				break;

			//don't do jack
			case QMessageBox::Cancel:
				return false;

			default:
				break;
		}
	}
	else
		resetClientEffect();

	updateCaption();
	return true;
}

// ----------------------------------------------------------------------

void MainWindow::playClientEffect()
{
	ClientEffect* effect = NULL;
	effect = m_effectTemplate.createClientEffect(m_avatar, CrcLowerString::empty);
	if(effect)
		effect->execute();
}

// ----------------------------------------------------------------------

void MainWindow::listViewContextMenuRequested(QListViewItem * const item, QPoint const & point, int const)
{
	//Qt *will* pass null to this func, so be sure to check
	if(!item)
		return;

	bool somethingChanged = false;

	QPopupMenu * const popupMenu = new QPopupMenu(this);
	ClientEffectEditorListViewItem * const ceelvi = dynamic_cast<ClientEffectEditorListViewItem *>(item);
	//if it's of our derived class type (every but the root nodes are)
	if(ceelvi)
	{
		//if it's a main entry node, we might want to delete it
		if(ceelvi->getType() == ClientEffectEditorListViewItem::DT_entryMainNode)
		{
			IGNORE_RETURN(popupMenu->insertItem("&Delete", MO_delete)); //lint !e641 enum to int
			switch(popupMenu->exec(point))
			{
				case MO_delete:
					delete item;
					somethingChanged = true;
					break;
				default:
					break;
			}
		}
	}
	//else it's a root node, we might want to add new children
	else
	{
		if(item->text(0) == cms_particleAppearanceBaseItemName)
		{
			IGNORE_RETURN(popupMenu->insertItem("&New ParticleEffect", MO_new)); //lint !e641 enum to int
			switch(popupMenu->exec(point))
			{
				case MO_new:
					addNewParticleEffect();
					somethingChanged = true;
					break;
				default:
					break;
			}
		}
		else if(item->text(0) == cms_soundBaseItemName)
		{
			IGNORE_RETURN(popupMenu->insertItem("&New Sound", MO_new)); //lint !e641 enum to int
			switch(popupMenu->exec(point))
			{
				case MO_new:
					addNewSound();
					somethingChanged = true;
					break;
				default:
					break;
			}
		}
		else if(item->text(0) == cms_cameraShakeBaseItemName)
		{
			IGNORE_RETURN(popupMenu->insertItem("&New Camera Shake", MO_new)); //lint !e641 enum to int
			switch(popupMenu->exec(point))
			{
				case MO_new:
					addNewCameraShake();
					somethingChanged = true;
					break;
				default:
					break;
			}
		}
		else if(item->text(0) == cms_lightBaseItemName)
		{
			IGNORE_RETURN(popupMenu->insertItem("&New Light", MO_new)); //lint !e641 enum to int
			switch(popupMenu->exec(point))
			{
				case MO_new:
					addNewLight();
					somethingChanged = true;
					break;
				default:
					break;
			}
		}
		else if(item->text(0) == cms_forceFeedbackBaseItemName)
		{
			IGNORE_RETURN(popupMenu->insertItem("&New ForceFeedback Effect", MO_new)); //lint !e641 enum to int
			switch(popupMenu->exec(point))
			{
				case MO_new:
					addNewForceFeedbackEffect();
					somethingChanged = true;
					break;
				default:
					break;
			}
		}
	}

	if(somethingChanged)
	{
		copyGUIToIff();
		m_effectTemplateModified = true;
	}
	delete popupMenu;
}

// END PUBLIC SLOT HANDLERS
// ======================================================================
// START MISC PRIVATE METHODS

void MainWindow::addNewParticleEffect()
{
	QSettings settings;
	settings.insertSearchPath(QSettings::Windows, "/SOE/ClientEffectEditor");
	QString previousUsedPath(settings.readEntry("NewParticleEffectPath", "c:\\"));

	//Get file to open from the Qt common file dialog.
	QString const newParticleEffect = QFileDialog::getOpenFileName(previousUsedPath, "ParticleEffect Files (*.prt)", this, "open file dialog", "Choose a file to open");
	if (newParticleEffect == QString::null)
		return;

	char const * existingPath = TreeFile::getShortestExistingPath(newParticleEffect.ascii());
	if(existingPath)
	{
		ClientEffectTemplate::CreateAppearanceFunc func;
		func.appearanceTemplateName = existingPath;
		func.softParticleTerminate = true;
		func.timeInSeconds = 1.0f;
		func.minScale = 1.0f;
		func.maxScale = 1.0f;
		func.minPlaybackRate = 1.0f;
		func.maxPlaybackRate = 1.0f;
		addParticleAppearanceTreeNode(func);
	}

	IGNORE_RETURN(settings.writeEntry("NewParticleEffectPath", newParticleEffect));
}

// ----------------------------------------------------------------------

void MainWindow::addNewSound()
{
	QSettings settings;
	settings.insertSearchPath(QSettings::Windows, "/SOE/ClientEffectEditor");
	QString previousUsedPath(settings.readEntry("NewSoundPath", "c:\\"));

	//Get file to open from the Qt common file dialog.
	QString const newSound = QFileDialog::getOpenFileName(previousUsedPath, "Sound Files (*.snd)", this, "open file dialog", "Choose a file to open");
	if (newSound == QString::null)
	{
		return;
	}

	char const * existingPath = TreeFile::getShortestExistingPath(newSound.ascii());
	if(existingPath)
	{
		ClientEffectTemplate::PlaySoundFunc func;
		func.soundTemplateName = existingPath;
		addSoundTreeNode(func);
	}

	IGNORE_RETURN(settings.writeEntry("NewSoundPath", newSound));
}

// ----------------------------------------------------------------------

void MainWindow::addNewCameraShake() const
{
	ClientEffectTemplate::CameraShakeFunc func;
	func.magnitudeInMeters = 0.5;
	func.frequencyInHz = 1.0f;
	func.timeInSeconds = 1.0f;
	func.falloffRadius = 100.0f;
	addCameraShakeTreeNode(func);
}

// ----------------------------------------------------------------------

void MainWindow::addNewLight() const
{
	ClientEffectTemplate::CreateLightFunc func;
	func.r = 255;
	func.g = 255;
	func.b = 255;
	func.timeInSeconds = 1.0f;
	func.range = 500;
	func.constantAttenuation = 0.0f;
	func.linearAttenuation = 0.5f;
	func.quadraticAttenuation = 1.0f;
	addLightTreeNode(func);
}

// ----------------------------------------------------------------------

void MainWindow::addNewForceFeedbackEffect()
{
	QSettings settings;
	settings.insertSearchPath(QSettings::Windows, "/SOE/ClientEffectEditor");
	QString previousUsedPath(settings.readEntry("NewFeedbackEffectPath", "c:\\"));

	//Get file to open from the Qt common file dialog.
	QString const newFFE = QFileDialog::getOpenFileName(previousUsedPath, "ForceFeedback Effect Files (*.ffe)", this, "open file dialog", "Choose a file to open");
	if (newFFE == QString::null)
	{
		return;
	}

	char const * existingPath = TreeFile::getShortestExistingPath(newFFE.ascii());
	if(existingPath)
	{
		ClientEffectTemplate::ForceFeedbackFunc func;
		func.forceFeedbackFile = existingPath;
		func.iterations = 1;
		func.range = 20.0f;
		addForceFeedbackEffectTreeNode(func);
	}

	IGNORE_RETURN(settings.writeEntry("NewFeedbackEffectPath", newFFE));
}

// ----------------------------------------------------------------------

void MainWindow::resetClientEffect()
{
	m_effectTemplate.clear();
	m_effectTemplateModified = false;
	copyIffToGUI();
}

// ----------------------------------------------------------------------

bool MainWindow::loadPathedClientEffect(std::string const & relativeFileName)
{
	resetClientEffect();
	Iff iff(relativeFileName.c_str());
	m_effectTemplate.load(iff);
	copyIffToGUI();
	setupListView();
	return true;
}

// ----------------------------------------------------------------------

bool MainWindow::savePathedClientEffect(std::string const & fullyPathedFileName)
{
	Iff iff(1024, true, true);
	m_effectTemplate.save(iff);
	bool const result = iff.write(fullyPathedFileName.c_str(), true);
	if(result)
	{
		m_effectTemplateModified = false;
	}
	else
	{
		IGNORE_RETURN(QMessageBox::information(this, "Couldn't save file", "Could not write the file, it is probably read-only."));
	}
	return result;
}

// ----------------------------------------------------------------------

bool MainWindow::close(bool const alsoDelete)
{
	if(m_effectTemplateModified)
	{
		bool const success = closeClientEffect();
		//don't quit if they canceled the close
		if(!success)
			return false;
	}
	return BaseMainWindow::close(alsoDelete);
}

// ----------------------------------------------------------------------

void MainWindow::copyIffToGUI()
{
	deleteAllListViewChildren(getParticleAppearancesBaseItem());
	deleteAllListViewChildren(getSoundsBaseItem());
	deleteAllListViewChildren(getCameraShakesBaseItem());
	deleteAllListViewChildren(getLightsBaseItem());
	deleteAllListViewChildren(getForceFeedbackEffectsBaseItem());

	ClientEffectTemplate::CreateAppearanceList const & particleAppearances = m_effectTemplate.getParticleSystems();
	for (ClientEffectTemplate::CreateAppearanceList::const_iterator i = particleAppearances.begin(); i != particleAppearances.end(); ++i)
	{
		addParticleAppearanceTreeNode(*i);
	}

	ClientEffectTemplate::PlaySoundList const & sounds = m_effectTemplate.getSounds();
	for (ClientEffectTemplate::PlaySoundList::const_iterator i2 = sounds.begin(); i2 != sounds.end(); ++i2)
	{
		addSoundTreeNode(*i2);
	}

	ClientEffectTemplate::CreateLightList const & lights = m_effectTemplate.getLights();
	for (ClientEffectTemplate::CreateLightList::const_iterator i3 = lights.begin(); i3 != lights.end(); ++i3)
	{
		addLightTreeNode(*i3);
	}

	ClientEffectTemplate::CameraShakeList const & cameraShakes = m_effectTemplate.getCameraShakes();
	for (ClientEffectTemplate::CameraShakeList::const_iterator i4 = cameraShakes.begin(); i4 != cameraShakes.end(); ++i4)
	{
		addCameraShakeTreeNode(*i4);
	}

	ClientEffectTemplate::ForceFeedbackList const & forceFeedbacks = m_effectTemplate.getForceFeedbackEffects();
	for (ClientEffectTemplate::ForceFeedbackList::const_iterator i5 = forceFeedbacks.begin(); i5 != forceFeedbacks.end(); ++i5)
	{
		addForceFeedbackEffectTreeNode(*i5);
	}
}

// ----------------------------------------------------------------------

/** Open the top level items, so users always see the data in the clienteffect
*/
void MainWindow::setupListView() const
{
	for(std::vector<QListViewItem *>::iterator i = ms_baseItems.begin(); i != ms_baseItems.end(); ++i)
	{
		if(*i)
		{
			(*i)->setOpen(true);
			closeAllItemChildren(*i);
		}
	}
}


// ----------------------------------------------------------------------

void MainWindow::updateCaption()
{
	QString capt = cms_clientEffectEditorCaption;
	if(ms_currentFile != QString::null)
	{
		capt += " - ";
		capt += ms_currentFile;
	}
	setCaption(capt);
}

// ----------------------------------------------------------------------

void MainWindow::copyGUIToIff()
{
	m_effectTemplate.clear();

	QListViewItem const * baseItem = getParticleAppearancesBaseItem();
	NOT_NULL(baseItem);
	QListViewItem const * entry = baseItem->firstChild();
	while(entry)
	{
		ClientEffectTemplateRW::CreateAppearanceFunc func;
		func.appearanceTemplateName = (entry->text(0)).ascii();
		QListViewItem * c = entry->firstChild();
		func.timeInSeconds = (c->text(1)).toFloat();
		c = c->nextSibling();

		std::string softTerminate = c->text(1).ascii();
		if(softTerminate != cms_true && softTerminate != cms_false)
		{
			IGNORE_RETURN(QMessageBox::warning(this, "Error", "Soft terminate is an invalid value, setting to TRUE"));
			softTerminate = cms_true;
		}
		func.softParticleTerminate = (softTerminate == cms_true) ? true : false;

		c = c->nextSibling();
		QString const minScaleText = c->text(1);
		func.minScale = (c->text(1)).toFloat();
		c = c->nextSibling();
		func.maxScale = (c->text(1)).toFloat();

		if(func.maxScale < func.minScale)
		{
			IGNORE_RETURN(QMessageBox::warning(this, "Error", "Max Scale is > Min Scale, adjusting..."));
			func.maxScale = func.minScale;
			c->setText(1, minScaleText);
		}

		c = c->nextSibling();
		func.minPlaybackRate = (c->text(1)).toFloat();
		QString const minPlaybackRate = c->text(1);
		c = c->nextSibling();
		func.maxPlaybackRate = (c->text(1)).toFloat();

		if(func.maxPlaybackRate < func.minPlaybackRate)
		{
			IGNORE_RETURN(QMessageBox::warning(this, "Error", "Max Playback Rate is > Min Playback Rate, adjusting..."));
			func.maxPlaybackRate = func.minPlaybackRate;
			c->setText(1, minPlaybackRate);
		}

		m_effectTemplate.addAppearance (func.appearanceTemplateName, func.timeInSeconds, func.minScale, func.maxScale, func.minPlaybackRate, func.maxPlaybackRate, func.softParticleTerminate);
		entry = entry->nextSibling();
	}

	baseItem = getSoundsBaseItem();
	NOT_NULL(baseItem);
	entry = baseItem->firstChild();
	while(entry)
	{
		ClientEffectTemplateRW::PlaySoundFunc func;
		func.soundTemplateName = (entry->text(0)).ascii();
		m_effectTemplate.addSound (func.soundTemplateName);
		entry = entry->nextSibling();
	}

	baseItem = getLightsBaseItem();
	NOT_NULL(baseItem);
	entry = baseItem->firstChild();
	while(entry)
	{
		ClientEffectTemplateRW::CreateLightFunc func;
		QListViewItem const * c = entry->firstChild();
		func.r = static_cast<unsigned char>((c->text(1)).toInt());
		c = c->nextSibling();
		func.g = static_cast<unsigned char>((c->text(1)).toInt());
		c = c->nextSibling();
		func.b = static_cast<unsigned char>((c->text(1)).toInt());
		c = c->nextSibling();
		func.timeInSeconds = (c->text(1)).toFloat();
		c = c->nextSibling();
		func.range = (c->text(1)).toFloat();
		c = c->nextSibling();
		func.constantAttenuation = (c->text(1)).toFloat();
		c = c->nextSibling();
		func.linearAttenuation = (c->text(1)).toFloat();
		c = c->nextSibling();
		func.quadraticAttenuation = (c->text(1)).toFloat();
		m_effectTemplate.addLight (func);
		entry = entry->nextSibling();
	}

	baseItem = getCameraShakesBaseItem();
	NOT_NULL(baseItem);
	entry = baseItem->firstChild();
	while(entry)
	{
		ClientEffectTemplateRW::CameraShakeFunc func;
		QListViewItem const * c = entry->firstChild();
		func.timeInSeconds = (c->text(1)).toFloat();
		c = c->nextSibling();
		func.magnitudeInMeters = (c->text(1)).toFloat();
		c = c->nextSibling();
		func.frequencyInHz = (c->text(1)).toFloat();
		c = c->nextSibling();
		func.falloffRadius = (c->text(1)).toFloat();
		m_effectTemplate.addCameraShake (func);
		entry = entry->nextSibling();
	}

	baseItem = getForceFeedbackEffectsBaseItem();
	NOT_NULL(baseItem);
	entry = baseItem->firstChild();
	while(entry)
	{
		ClientEffectTemplateRW::ForceFeedbackFunc func;
		func.forceFeedbackFile = (entry->text(0)).ascii();
		func.forceFeedbackTemplate = ForceFeedbackEffectTemplateList::fetch (func.forceFeedbackFile.c_str ());
		QListViewItem const * c = entry->firstChild();
		func.iterations = (c->text(1)).toInt();
		c = c->nextSibling();
		func.range = (c->text(1)).toFloat();
		m_effectTemplate.addForceFeedbackEffect(func);
		entry = entry->nextSibling();
	}
}

// ----------------------------------------------------------------------

QListViewItem * MainWindow::getParticleAppearancesBaseItem() const
{
	return ms_baseItems[0];
}

// ----------------------------------------------------------------------

QListViewItem * MainWindow::getSoundsBaseItem() const
{
	return ms_baseItems[1];
}

// ----------------------------------------------------------------------

QListViewItem * MainWindow::getCameraShakesBaseItem() const
{
	return ms_baseItems[2];
}

// ----------------------------------------------------------------------

QListViewItem * MainWindow::getLightsBaseItem() const
{
	return ms_baseItems[3];
}

// ----------------------------------------------------------------------

QListViewItem * MainWindow::getForceFeedbackEffectsBaseItem() const
{
	return ms_baseItems[4];
}

// ----------------------------------------------------------------------

void MainWindow::addParticleAppearanceTreeNode(ClientEffectTemplate::CreateAppearanceFunc const & value)
{
	if(!TreeFile::exists(value.appearanceTemplateName.c_str()))
	{
		std::string error = "The Particle Effect file [";
		error += value.appearanceTemplateName + "] could not be found, check the filename.";
		IGNORE_RETURN(QMessageBox::warning(this, "Possible bad file", error.c_str()));
	}

	QListViewItem * const base = getParticleAppearancesBaseItem();
	if(base)
	{
		QListViewItem * const lastChild = getLastListViewItemChild(base);
		ClientEffectEditorListViewItem * const node = new ClientEffectEditorListViewItem(base, lastChild, value.appearanceTemplateName.c_str());
		IGNORE_RETURN(ms_utilityString.setNum(value.timeInSeconds));
		node->setType(ClientEffectEditorListViewItem::DT_entryMainNode);
		ClientEffectEditorListViewItem * newNode = new ClientEffectEditorListViewItem (node, NULL, "Time (secs)", ms_utilityString.ascii());
		newNode->setRenameEnabled(1, true);
		newNode->setType(ClientEffectEditorListViewItem::DT_floatGreaterThanZero);
		std::string val = value.softParticleTerminate ? cms_true : cms_false;
		newNode = new ClientEffectEditorListViewItem (node, newNode, "Soft Terminate", val.c_str());
		newNode->setRenameEnabled(1, true);
		newNode->setType(ClientEffectEditorListViewItem::DT_bool);
		IGNORE_RETURN(ms_utilityString.setNum(value.minScale));
		newNode = new ClientEffectEditorListViewItem (node, newNode, "Min Scale", ms_utilityString.ascii());
		newNode->setRenameEnabled(1, true);
		newNode->setType(ClientEffectEditorListViewItem::DT_floatGreaterThanZero);
		IGNORE_RETURN(ms_utilityString.setNum(value.maxScale));
		newNode = new ClientEffectEditorListViewItem (node, newNode, "Max Scale", ms_utilityString.ascii());
		newNode->setRenameEnabled(1, true);
		newNode->setType(ClientEffectEditorListViewItem::DT_floatGreaterThanZero);
		IGNORE_RETURN(ms_utilityString.setNum(value.minPlaybackRate));
		newNode = new ClientEffectEditorListViewItem (node, newNode, "Min Playback Rate", ms_utilityString.ascii());
		newNode->setRenameEnabled(1, true);
		newNode->setType(ClientEffectEditorListViewItem::DT_floatGreaterThanZero);
		IGNORE_RETURN(ms_utilityString.setNum(value.maxPlaybackRate));
		newNode = new ClientEffectEditorListViewItem (node, newNode, "Max Playback Rate", ms_utilityString.ascii());
		newNode->setRenameEnabled(1, true);
		newNode->setType(ClientEffectEditorListViewItem::DT_floatGreaterThanZero);
	}//lint !e429 newNode not released (Qt owns it)
}

// ----------------------------------------------------------------------

void MainWindow::addSoundTreeNode(ClientEffectTemplate::PlaySoundFunc const & value)
{
	if(!TreeFile::exists(value.soundTemplateName.c_str()))
	{
		std::string error = "The Sound file [";
		error += value.soundTemplateName + "] could not be found, check the filename.";
		IGNORE_RETURN(QMessageBox::warning(this, "Possible bad file", error.c_str()));
	}

	QListViewItem * const base = getSoundsBaseItem();
	if(base)
	{
		QListViewItem * const lastChild = getLastListViewItemChild(base);
		ClientEffectEditorListViewItem * const node = new ClientEffectEditorListViewItem(base, lastChild, value.soundTemplateName.c_str());
		node->setType(ClientEffectEditorListViewItem::DT_entryMainNode);
	} //lint !e429 node not released (Qt owns it)
}

// ----------------------------------------------------------------------

void MainWindow::addLightTreeNode(ClientEffectTemplate::CreateLightFunc const & value) const
{
	QListViewItem * const base = getLightsBaseItem();
	if(base)
	{
		QListViewItem * const lastChild = getLastListViewItemChild(base);
		int const numChildren = base->childCount();
		IGNORE_RETURN(ms_utilityString.setNum(numChildren));
		std::string lightString = "Light ";
		lightString += ms_utilityString.ascii();
		ClientEffectEditorListViewItem * const node = new ClientEffectEditorListViewItem(base, lastChild, lightString.c_str());
		node->setType(ClientEffectEditorListViewItem::DT_entryMainNode);
		IGNORE_RETURN(ms_utilityString.setNum(value.r));
		ClientEffectEditorListViewItem * newNode = new ClientEffectEditorListViewItem (node, NULL, "R", ms_utilityString.ascii());
		newNode->setRenameEnabled(1, true);
		newNode->setType(ClientEffectEditorListViewItem::DT_int0To255);
		IGNORE_RETURN(ms_utilityString.setNum(value.g));
		newNode = new ClientEffectEditorListViewItem (node, newNode, "G", ms_utilityString.ascii());
		newNode->setRenameEnabled(1, true);
		newNode->setType(ClientEffectEditorListViewItem::DT_int0To255);
		IGNORE_RETURN(ms_utilityString.setNum(value.b));
		newNode = new ClientEffectEditorListViewItem (node, newNode, "B", ms_utilityString.ascii());
		newNode->setRenameEnabled(1, true);
		newNode->setType(ClientEffectEditorListViewItem::DT_int0To255);
		IGNORE_RETURN(ms_utilityString.setNum(value.timeInSeconds));
		newNode = new ClientEffectEditorListViewItem (node, newNode, "Time (secs)", ms_utilityString.ascii());
		newNode->setRenameEnabled(1, true);
		newNode->setType(ClientEffectEditorListViewItem::DT_floatGreaterThanZero);
		IGNORE_RETURN(ms_utilityString.setNum(value.range));
		newNode = new ClientEffectEditorListViewItem (node, newNode, "Range (m)", ms_utilityString.ascii());
		newNode->setRenameEnabled(1, true);
		newNode->setType(ClientEffectEditorListViewItem::DT_floatGreaterThanZero);
		IGNORE_RETURN(ms_utilityString.setNum(value.constantAttenuation));
		newNode = new ClientEffectEditorListViewItem (node, newNode, "Constant Attenuation", ms_utilityString.ascii());
		newNode->setRenameEnabled(1, true);
		newNode->setType(ClientEffectEditorListViewItem::DT_floatGreaterThanZero);
		IGNORE_RETURN(ms_utilityString.setNum(value.linearAttenuation));
		newNode = new ClientEffectEditorListViewItem (node, newNode, "Linear Attenuation", ms_utilityString.ascii());
		newNode->setRenameEnabled(1, true);
		newNode->setType(ClientEffectEditorListViewItem::DT_floatGreaterThanZero);
		IGNORE_RETURN(ms_utilityString.setNum(value.quadraticAttenuation));
		newNode = new ClientEffectEditorListViewItem (node, newNode, "Quadratic Attenuation", ms_utilityString.ascii());
		newNode->setRenameEnabled(1, true);
		newNode->setType(ClientEffectEditorListViewItem::DT_floatGreaterThanZero);
	} //lint !e429 newNode not released (Qt owns it)
}

// ----------------------------------------------------------------------

void MainWindow::addCameraShakeTreeNode(ClientEffectTemplate::CameraShakeFunc const & value) const
{
	QListViewItem * const base = getCameraShakesBaseItem();
	if(base)
	{
		QListViewItem * const lastChild = getLastListViewItemChild(base);
		int const numChildren = base->childCount();
		IGNORE_RETURN(ms_utilityString.setNum(numChildren));
		std::string cameraShakeString = "Camera Shake ";
		cameraShakeString += ms_utilityString.ascii();
		ClientEffectEditorListViewItem * const node = new ClientEffectEditorListViewItem(base, lastChild, cameraShakeString.c_str());
		node->setType(ClientEffectEditorListViewItem::DT_entryMainNode);
		IGNORE_RETURN(ms_utilityString.setNum(value.timeInSeconds));
		ClientEffectEditorListViewItem * newNode = new ClientEffectEditorListViewItem (node, NULL, "Time (secs)", ms_utilityString.ascii());
		newNode->setRenameEnabled(1, true);
		newNode->setType(ClientEffectEditorListViewItem::DT_floatGreaterThanZero);
		IGNORE_RETURN(ms_utilityString.setNum(value.magnitudeInMeters));
		newNode = new ClientEffectEditorListViewItem (node, newNode, "Magnitude", ms_utilityString.ascii());
		newNode->setRenameEnabled(1, true);
		newNode->setType(ClientEffectEditorListViewItem::DT_floatGreaterThanZero);
		IGNORE_RETURN(ms_utilityString.setNum(value.frequencyInHz));
		newNode = new ClientEffectEditorListViewItem (node, newNode, "Frequency (Hz)", ms_utilityString.ascii());
		newNode->setRenameEnabled(1, true);
		newNode->setType(ClientEffectEditorListViewItem::DT_floatGreaterThanZero);
		IGNORE_RETURN(ms_utilityString.setNum(value.falloffRadius));
		newNode = new ClientEffectEditorListViewItem (node, newNode, "Falloff Radius (m)", ms_utilityString.ascii());
		newNode->setRenameEnabled(1, true);
		newNode->setType(ClientEffectEditorListViewItem::DT_floatGreaterThanZero);
	}  //lint !e429 newNode not released (Qt owns it)
}

// ----------------------------------------------------------------------

void MainWindow::addForceFeedbackEffectTreeNode(ClientEffectTemplate::ForceFeedbackFunc const & value) const
{
	if(!TreeFile::exists(value.forceFeedbackFile.c_str()))
	{
		std::string error = "The ForceFeedback file [";
		error += value.forceFeedbackFile + "] could not be found, check the filename.";
		IGNORE_RETURN(QMessageBox::warning(&getInstance(), "Possible bad file", error.c_str()));
	}

	QListViewItem * const base = getForceFeedbackEffectsBaseItem();
	if(base)
	{
		QListViewItem * const lastChild = getLastListViewItemChild(base);
		ClientEffectEditorListViewItem * const node = new ClientEffectEditorListViewItem(base, lastChild, value.forceFeedbackFile.c_str());
		node->setType(ClientEffectEditorListViewItem::DT_entryMainNode);
		IGNORE_RETURN(ms_utilityString.setNum(value.iterations));
		ClientEffectEditorListViewItem * newNode = new ClientEffectEditorListViewItem (node, NULL, "Iterations", ms_utilityString.ascii());
		newNode->setRenameEnabled(1, true);
		newNode->setType(ClientEffectEditorListViewItem::DT_intGreaterThanZero);
		IGNORE_RETURN(ms_utilityString.setNum(value.range));
		newNode = new ClientEffectEditorListViewItem (node, newNode, "Range (m)", ms_utilityString.ascii());
		newNode->setRenameEnabled(1, true);
		newNode->setType(ClientEffectEditorListViewItem::DT_floatGreaterThanZero);
	}  //lint !e429 newNode not released (Qt owns it)
}

// END MISC PRIVATE METHODS
// ======================================================================
