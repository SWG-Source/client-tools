//======================================================================
//
// TemplateNewDialog.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "FirstShipComponentEditor.h"
#include "TemplateNewDialog.h"
#include "TemplateNewDialog.moc"

#include "ConfigShipComponentEditor.h"
#include "QStringUtil.h"
#include "ShipComponentEditorServerTemplate.h"
#include "ShipComponentEditorServerTemplateManager.h"
#include "fileInterface/StdioFile.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/Crc.h"
#include "sharedGame/ShipChassis.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/ObjectTemplateList.h"

#if WIN32
#include <direct.h>
#endif

//======================================================================

namespace TemplateNewDialogNamespace
{
	//----------------------------------------------------------------------

	bool writeAndCompileTemplate(std::string const & templatePathTpf, std::string const & templatePathIff, std::string const & buffer)
	{
		bool retval = false;

		StdioFileFactory sff;
		AbstractFile * const af = sff.createFile(templatePathTpf.c_str(), "wb");
		if (NULL != af && af->isOpen())
		{
			int const bytesWritten = af->write(buffer.size(), buffer.c_str());
			retval = (bytesWritten == static_cast<int>(buffer.size()));
		}
		else
		{
			WARNING(true, ("ShipComponentDescriptor failed to write file [%s]", templatePathTpf.c_str()));
		}

		delete af;

		if (!retval)
		{
			WARNING(true, ("TemplateNewDialog writeAndCompileTemplate unable to write data to file [%s]", templatePathTpf.c_str()));
			return false;
		}

		char buf[1024];
		size_t const buf_size = sizeof(buf);

		//-- compile
		{
			size_t const last_slashpos = templatePathTpf.rfind ('/');
			if (std::string::npos == last_slashpos)
			{
				WARNING(true, ("Failed to compile because path could not be split [%s]", templatePathTpf.c_str()));
			}
			else
			{
				std::string const & tpfDirPath = templatePathTpf.substr(0, last_slashpos);
				std::string const & tpfFileName = templatePathTpf.substr(last_slashpos + 1);

				std::string const cwd (_getcwd(buf, buf_size));
				_chdir(tpfDirPath.c_str());
				snprintf(buf, buf_size, "TemplateCompiler %s", tpfFileName);
				system(buf);
				_chdir(cwd.c_str());
			}
		}

		//-- add to perforce
		snprintf(buf, buf_size, "p4 add %s %s", templatePathTpf.c_str(), templatePathIff.c_str());
		system(buf);

		//-- open in the text editor
		snprintf(buf, buf_size, "start %s %s", ConfigShipComponentEditor::getTextEditorPath().c_str(), templatePathTpf.c_str());
		system(buf);

		return true;
	}

	//----------------------------------------------------------------------

	std::string generateTpfPath(std::string const & basePath, std::string const & templateName)
	{
		std::string templatePathTpf = basePath + "/" + templateName;
	
		if (templatePathTpf.size() <= 4)
		{
			WARNING(true, ("TemplateNewDialog invalid templateName [%s]", templateName.c_str()));
			templatePathTpf.clear();
		}
		else
			templatePathTpf.replace(templatePathTpf.size() - 4, 4, ".tpf");

		return templatePathTpf;
	}

	//----------------------------------------------------------------------

	bool generateServerShipTemplate(std::string const & templateName, std::string const & baseTemplateName, std::string const & sharedTemplateName, std::string const & shipType)
	{
		std::string const & templatePathIff = ConfigShipComponentEditor::getServerPathData() + "/" + templateName;
		std::string const & templatePathTpf = generateTpfPath(ConfigShipComponentEditor::getServerPathDsrc(), templateName);
	
		//-- parameters:
		//-- - base
		//-- - shipType
		//-- - sharedTemplate

		const char * const formattedBuffer =
			"// this template created by the Ship Component Editor\n\n"
			"@base %s\n\n"
			"@class ship_object_template 1\n\n"
			"shipType = \"%s\"\n\n"
			"@class tangible_object_template 4\n\n"
			"@class object_template 8\n\n"
			"sharedTemplate = \"%s\"\n\n"
			"scripts =+ [\"space.combat.combat_ship\"]\n";

		char buf[2048];
		size_t const buf_size = sizeof(buf);
		snprintf(buf, buf_size, formattedBuffer, baseTemplateName.c_str(), shipType.c_str(), sharedTemplateName.c_str());
		buf[buf_size-1] = 0;

		writeAndCompileTemplate(templatePathTpf, templatePathIff, buf);

		return true;
	}

	//----------------------------------------------------------------------

	bool generateSharedShipTemplate(std::string const & sharedTemplateName, std::string const & sharedBaseTemplateName)
	{
		std::string const & templatePathIff = ConfigShipComponentEditor::getSharedPathData() + "/" + sharedTemplateName;
		std::string const & templatePathTpf = generateTpfPath(ConfigShipComponentEditor::getSharedPathDsrc(), sharedTemplateName);
	
		//-- parameters:
		//-- - shared base

		const char * const formattedBuffer =
			"// this template created by the Ship Component Editor\n\n"
			"@base %s\n\n"
			"@class ship_object_template 4\n\n"
			"@class tangible_object_template 9\n\n"
			"@class object_template 9\n\n"
			"objectName = \"ship_n\" \"nametoken\"\n"
			"detailedDescription = \"ship_d\" \"nametoken\"\n\n"
			"appearanceFilename = \"\"\n\n"
			"containerType = CT_slotted\n"
			"slotDescriptorFilename = \"abstract/slot/descriptor/ship_single_seat.iff\"\n\n"
			"clientDataFile = \"\"\n";

		char buf[2048];
		size_t const buf_size = sizeof(buf);
		snprintf(buf, buf_size, formattedBuffer, sharedBaseTemplateName.c_str());
		buf[buf_size-1] = 0;

		writeAndCompileTemplate(templatePathTpf, templatePathIff, buf);

		return true;
	}

	//----------------------------------------------------------------------

	bool generateServerComponentTemplate(std::string const & templateName, std::string const & baseTemplateName, std::string const & sharedTemplateName)
	{
		std::string const & templatePathIff = ConfigShipComponentEditor::getServerPathData() + "/" + templateName;
		std::string const & templatePathTpf = generateTpfPath(ConfigShipComponentEditor::getServerPathDsrc(), templateName);
	
		//-- parameters:
		//-- - base
		//-- - sharedTemplate

		const char * const formattedBuffer =
			"// this template created by the Ship Component Editor\n\n"
			"@base %s\n\n"
			"@class tangible_object_template 4\n\n"
			"@class object_template 8\n\n"
			"sharedTemplate = \"%s\"\n\n"
			"moveFlags = [ MF_player ]\n";

		char buf[2048];
		size_t const buf_size = sizeof(buf);
		snprintf(buf, buf_size, formattedBuffer, baseTemplateName.c_str(), sharedTemplateName.c_str());
		buf[buf_size-1] = 0;

		writeAndCompileTemplate(templatePathTpf, templatePathIff, buf);

		return true;
	}

	//----------------------------------------------------------------------

	bool generateSharedComponentTemplate(std::string const & sharedTemplateName, std::string const & sharedBaseTemplateName)
	{
		std::string const & templatePathIff = ConfigShipComponentEditor::getSharedPathData() + "/" + sharedTemplateName;
		std::string const & templatePathTpf = generateTpfPath(ConfigShipComponentEditor::getSharedPathDsrc(), sharedTemplateName);
	
		//-- parameters:
		//-- - shared base

		const char * const formattedBuffer =
			"// this template created by the Ship Component Editor\n\n"
			"@base %s\n\n"
			"@class ship_object_template 4\n\n"
			"@class tangible_object_template 9\n\n"
			"certificationsRequired = [\"\"]\n\n"
			"@class object_template 9\n\n"
			"objectName = \"ship_n\" \"nametoken\"\n"
			"detailedDescription = \"ship_d\" \"nametoken\"\n\n"
			"appearanceFilename = \"\"\n\n"
			"containerType = CT_none\n"
			"clientDataFile = \"\"\n\n"
			"gameObjectType = GOT_ship_component_weapon\n";

		char buf[2048];
		size_t const buf_size = sizeof(buf);
		snprintf(buf, buf_size, formattedBuffer, sharedBaseTemplateName.c_str());
		buf[buf_size-1] = 0;

		writeAndCompileTemplate(templatePathTpf, templatePathIff, buf);

		return true;
	}

	//----------------------------------------------------------------------

}

using namespace TemplateNewDialogNamespace;

//----------------------------------------------------------------------

TemplateNewDialog::TemplateNewDialog(QWidget *parent, char const *name, std::string const & selectChassisName, std::string const & selectTemplateName) :
BaseTemplateNewDialog(parent, name),
m_newTemplateValid(false),
m_baseTemplateValid(false),
m_sharedTemplateValid(false),
m_sharedBaseTemplateValid(false),
m_creatingNewTemplate(false),
m_creatingSharedTemplate(false),
m_mode(M_chassis)
{
	setupSignals();

	populateChassisCombo();
	
	if (!selectTemplateName.empty())
	{
		//-- this causes a trickle-down effect to the other fields
		m_lineEditNewTemplate->setText(selectTemplateName.c_str());
	}
	else if (!selectChassisName.empty())
	{
		selectChassis(selectChassisName);
	}

	updateTextFieldValidity();
}

//----------------------------------------------------------------------

TemplateNewDialog::TemplateNewDialog(QWidget *parent, char const *name, std::string const & selectTemplateName) :
BaseTemplateNewDialog(parent, name),
m_newTemplateValid(false),
m_baseTemplateValid(false),
m_sharedTemplateValid(false),
m_sharedBaseTemplateValid(false),
m_creatingNewTemplate(false),
m_creatingSharedTemplate(false),
m_mode(M_component)
{
	setupSignals();
	
	m_comboChassis->hide();

	if (!selectTemplateName.empty())
	{
		//-- this causes a trickle-down effect to the other fields
		m_lineEditNewTemplate->setText(selectTemplateName.c_str());
	}

	updateTextFieldValidity();
}

//----------------------------------------------------------------------

void TemplateNewDialog::setupSignals()
{
	connect(m_lineEditSharedTemplate, SIGNAL(textChanged(const QString&)), SLOT(onLineEditSharedTemplateTextChanged(const QString &)));
	connect(m_lineEditBaseTemplate, SIGNAL(textChanged(const QString&)), SLOT(onLineEditBaseTemplateTextChanged(const QString &)));
	connect(m_lineEditNewTemplate, SIGNAL(textChanged(const QString&)), SLOT(onLineEditNewTemplateTextChanged(const QString &)));
	connect(m_lineEditSharedBaseTemplate, SIGNAL(textChanged(const QString&)), SLOT(onLineEditSharedBaseTemplateTextChanged(const QString &)));

	connect(m_pushButtonBrowseNewTemplate, SIGNAL(clicked()), SLOT(onPushButtonBrowseNewTemplateClicked()));
	connect(m_pushButtonBrowseBaseTemplate, SIGNAL(clicked()), SLOT(onPushButtonBrowseBaseTemplateClicked()));
	connect(m_pushButtonBrowseSharedTemplate, SIGNAL(clicked()), SLOT(onPushButtonBrowseSharedTemplateClicked()));
	connect(m_pushButtonBrowseSharedBaseTemplate, SIGNAL(clicked()), SLOT(onPushButtonBrowseSharedBaseTemplateClicked()));

	connect(m_pushButtonBrowseFileNewTemplate, SIGNAL(clicked()), SLOT(onPushButtonBrowseFileNewTemplateClicked()));
	connect(m_pushButtonBrowseFileBaseTemplate, SIGNAL(clicked()), SLOT(onPushButtonBrowseFileBaseTemplateClicked()));
	connect(m_pushButtonBrowseFileSharedTemplate, SIGNAL(clicked()), SLOT(onPushButtonBrowseFileSharedTemplateClicked()));
	connect(m_pushButtonBrowseFileSharedBaseTemplate, SIGNAL(clicked()), SLOT(onPushButtonBrowseFileSharedBaseTemplateClicked()));

	connect(m_pushButtonCreateTemplate, SIGNAL(clicked()), SLOT(onPushButtonCreateTemplateClicked()));
}

//----------------------------------------------------------------------

TemplateNewDialog::~TemplateNewDialog()
{

}

//----------------------------------------------------------------------
//-- SLOTS
//----------------------------------------------------------------------

void TemplateNewDialog::onLineEditSharedTemplateTextChanged(const QString & text)
{
	m_creatingSharedTemplate = false;
	m_sharedTemplateValid = true;
	std::string const & textStr = Unicode::toLower(QStringUtil::toString(text));
	if (textStr.empty())
	{
		m_sharedTemplateValid = false;
	}
	else if (0 != textStr.find("object/"))
	{
		m_sharedTemplateValid = false;
	}
	else if ((textStr.size() - 4) != textStr.rfind(".iff"))
	{
		m_sharedTemplateValid = false;
	}
	else 
	{
		size_t const last_slashpos = textStr.rfind('/');
		if (std::string::npos == last_slashpos)
			m_sharedTemplateValid = false;
		else
		{
			if (last_slashpos != textStr.find("/shared_", last_slashpos))
				m_sharedTemplateValid = false;
		}
	}

	if (m_sharedTemplateValid)
	{
		m_creatingSharedTemplate = ObjectTemplateList::lookUp(Crc::normalizeAndCalculate(textStr.c_str())).isEmpty();
	}

	updateTextFieldValidity();
}

//----------------------------------------------------------------------

void TemplateNewDialog::onLineEditSharedBaseTemplateTextChanged(const QString & text)
{
	m_sharedBaseTemplateValid = true;
	std::string const & textStr = Unicode::toLower(QStringUtil::toString(text));
	if (textStr.empty())
	{
		m_sharedBaseTemplateValid  = false;
	}
	else if (0 != textStr.find("object/"))
	{
		m_sharedBaseTemplateValid  = false;
	}
	else if ((textStr.size() - 4) != textStr.rfind(".iff"))
	{
		m_sharedBaseTemplateValid  = false;
	}
	else 
	{
		size_t const last_slashpos = textStr.rfind('/');
		if (std::string::npos == last_slashpos)
			m_sharedBaseTemplateValid  = false;
		else
		{
			if (last_slashpos != textStr.find("/shared_", last_slashpos))
				m_sharedBaseTemplateValid  = false;
		}
	}

	if (m_sharedBaseTemplateValid)
	{
		m_sharedBaseTemplateValid = !ObjectTemplateList::lookUp(Crc::normalizeAndCalculate(textStr.c_str())).isEmpty();
	}

	updateTextFieldValidity();
}
//----------------------------------------------------------------------

void TemplateNewDialog::onLineEditBaseTemplateTextChanged(const QString & text)
{
	m_baseTemplateValid = true;
	std::string const & textStr = Unicode::toLower(QStringUtil::toString(text));
	if (textStr.empty())
	{
		m_baseTemplateValid = false;
	}
	else if (0 != textStr.find("object/"))
	{
		m_baseTemplateValid = false;
	}
	else if ((textStr.size() - 4) != textStr.rfind(".iff"))
	{
		m_baseTemplateValid = false;
	}
	else 
	{
		size_t const last_slashpos = textStr.rfind('/');
		if (std::string::npos == last_slashpos)
			m_baseTemplateValid = false;
		else
		{
			if (std::string::npos != textStr.find("/shared_", last_slashpos))
				m_baseTemplateValid = false;
		}
	}

	if (m_baseTemplateValid)
	{
		m_baseTemplateValid = (NULL != ShipComponentEditorServerTemplateManager::findTemplateByName(textStr));
	}

	updateTextFieldValidity();
}

//----------------------------------------------------------------------

void TemplateNewDialog::onLineEditNewTemplateTextChanged(const QString & text)
{
	std::string const & textStr = Unicode::toLower(QStringUtil::toString(text));
	m_newTemplateValid = true;
	m_creatingNewTemplate = false;

	if (textStr.empty())
	{
		m_newTemplateValid = false;
	}
	else if (0 != textStr.find("object/"))
	{
		m_newTemplateValid = false;
	}
	else if ((textStr.size() - 4) != textStr.rfind(".iff"))
	{
		m_newTemplateValid = false;
	}
	else 
	{
		size_t const last_slashpos = textStr.rfind('/');
		if (std::string::npos == last_slashpos)
			m_newTemplateValid = false;
		else
		{
			if (std::string::npos != textStr.find("/shared_", last_slashpos))
				m_newTemplateValid = false;
		}
	}
	
	if (m_newTemplateValid)
	{
		ShipComponentEditorServerTemplate const * const editorServerTemplate = ShipComponentEditorServerTemplateManager::findTemplateByName(textStr);
		m_creatingNewTemplate = editorServerTemplate == NULL;
		
		if (M_chassis == m_mode)
		{
			
			if (editorServerTemplate != NULL)
			{
				selectChassis(editorServerTemplate->chassisType);
			}
		}
	}

	updateTextFieldValidity();
}

//----------------------------------------------------------------------

void TemplateNewDialog::onPushButtonBrowseNewTemplateClicked()
{
	handleFileBrowsing(*m_lineEditNewTemplate, false, m_newTemplateValid, true);
}

//----------------------------------------------------------------------

void TemplateNewDialog::onPushButtonBrowseBaseTemplateClicked()
{
	handleFileBrowsing(*m_lineEditBaseTemplate, false, m_baseTemplateValid, true);
}

//----------------------------------------------------------------------

void TemplateNewDialog::onPushButtonBrowseSharedTemplateClicked()
{
	handleFileBrowsing(*m_lineEditSharedTemplate, false, m_sharedTemplateValid, false);
}

//----------------------------------------------------------------------

void TemplateNewDialog::onPushButtonBrowseSharedBaseTemplateClicked()
{
	handleFileBrowsing(*m_lineEditSharedBaseTemplate, false, m_sharedBaseTemplateValid, false);
}

//----------------------------------------------------------------------

void TemplateNewDialog::onPushButtonBrowseFileNewTemplateClicked()
{
	handleFileBrowsing(*m_lineEditNewTemplate, true, m_newTemplateValid, true);
}

//----------------------------------------------------------------------

void TemplateNewDialog::onPushButtonBrowseFileBaseTemplateClicked()
{
	handleFileBrowsing(*m_lineEditBaseTemplate, true, m_baseTemplateValid, true);
}

//----------------------------------------------------------------------

void TemplateNewDialog::onPushButtonBrowseFileSharedTemplateClicked()
{
	handleFileBrowsing(*m_lineEditSharedTemplate, true, m_sharedTemplateValid, false);
}

//----------------------------------------------------------------------

void TemplateNewDialog::onPushButtonBrowseFileSharedBaseTemplateClicked()
{
	handleFileBrowsing(*m_lineEditSharedBaseTemplate, true, m_sharedBaseTemplateValid, false);
}

//----------------------------------------------------------------------

void TemplateNewDialog::onPushButtonCreateTemplateClicked()
{
//	QMessageBox::information(NULL, "TODO", "Template Creation not implemented.");
	if (m_newTemplateValid && m_creatingNewTemplate && m_sharedTemplateValid)
	{
		m_resultTemplateName = QStringUtil::toString(m_lineEditNewTemplate->text());
		std::string const & baseTemplateName = QStringUtil::toString(m_lineEditBaseTemplate->text());
		std::string const & sharedTemplateName = QStringUtil::toString(m_lineEditSharedTemplate->text());
		std::string const & chassisName = QStringUtil::toString(m_comboChassis->text(m_comboChassis->currentItem()));
		
		//-- generate a set of ship templates
		if (M_chassis == m_mode)
		{
			if (chassisName.empty())
			{
				QMessageBox::warning(NULL, "Invalid Chassis Selection", "Invalid Chassis Selection");			
				return;
			}
			
			if (!generateServerShipTemplate(m_resultTemplateName, baseTemplateName, sharedTemplateName, chassisName))
			{
				QMessageBox::warning(NULL, "Error Generating Template", "An error occured while generating the server template.\nCheck DebugView warnings for more details.");
				return;
			}
			
			if (m_creatingSharedTemplate && m_sharedBaseTemplateValid)
			{
				std::string const & sharedBaseTemplateName = QStringUtil::toString(m_lineEditSharedBaseTemplate->text());
				
				if (!generateSharedShipTemplate(sharedTemplateName, sharedBaseTemplateName))
				{
					QMessageBox::warning(NULL, "Error Generating Template", "An error occured while generating the server template.\nCheck DebugView warnings for more details.");
					return;
				}			
			}
		}
		//-- generate a set of component templates
		else if (M_component == m_mode)
		{
			if (!generateServerComponentTemplate(m_resultTemplateName, baseTemplateName, sharedTemplateName))
			{
				QMessageBox::warning(NULL, "Error Generating Template", "An error occured while generating the server template.\nCheck DebugView warnings for more details.");
				return;
			}
			
			if (m_creatingSharedTemplate && m_sharedBaseTemplateValid)
			{
				std::string const & sharedBaseTemplateName = QStringUtil::toString(m_lineEditSharedBaseTemplate->text());
				
				if (!generateSharedComponentTemplate(sharedTemplateName, sharedBaseTemplateName))
				{
					QMessageBox::warning(NULL, "Error Generating Template", "An error occured while generating the server template.\nCheck DebugView warnings for more details.");
					return;
				}			
			}
		}
	}
	else
		m_resultTemplateName.clear();

	hide();
}

//----------------------------------------------------------------------
//-- End SLOTS
//----------------------------------------------------------------------

void TemplateNewDialog::handleFileBrowsing(QLineEdit & lineEdit, bool fileOrFolder, bool isValid, bool isServer)
{
	std::string pathPrefix;

	if (isServer)
		pathPrefix = ConfigShipComponentEditor::getServerPathData() + "/";
	else
		pathPrefix = ConfigShipComponentEditor::getSharedPathData() + "/";

	std::string pathToOpen = pathPrefix;

	std::string const & templateName = Unicode::toLower(QStringUtil::toString(lineEdit.text()));
	if (templateName.empty() || !isValid)
	{
		switch (m_mode)
		{
		case M_chassis:
			pathToOpen += "object/ship/";
			break;
		case M_component:
			pathToOpen += "object/tangible/ship/components/";
		}
	}
	else if (isValid)
	{
		pathToOpen += templateName;
	}


	QString qstringDirName;

	if (fileOrFolder)
	{
		if (pathToOpen.rfind(".iff") != pathToOpen.size() - 4)
			pathToOpen += "*.*";

		qstringDirName = QFileDialog::getOpenFileName(QString(pathToOpen.c_str()), QString::null, this, "fsel", "Select a File");
	}
	else
	{
		qstringDirName = QFileDialog::getExistingDirectory(QString(pathToOpen.c_str()), this, "Find a bzzz", "Select a Folder", false);
	}

	if (!qstringDirName.isEmpty())
	{
		std::string dirName = Unicode::toLower(QStringUtil::toString(qstringDirName));
		if (!_strnicmp(pathPrefix.c_str(), dirName.c_str(), pathPrefix.size()))
		{
			dirName = dirName.substr(pathPrefix.size());
			lineEdit.setText(dirName.c_str());
		}
		else
		{
			char buf[1024];
			size_t const buf_size = sizeof(buf);
			snprintf(buf, buf_size, 
				"You have selected an invalid path:\n"
				"%s\n"
				"The path for this template must be beneath:\n"
				"%s\n",
				dirName.c_str(),
				pathPrefix.c_str());
			QMessageBox::warning(NULL, "Invalid Path", buf);
		}
	}
}

//----------------------------------------------------------------------

void TemplateNewDialog::updateTextFieldValidity()
{
	//-- update new template field
	{
		std::string statusStr;
		QPalette pal (m_lineEditNewTemplate->palette());
		if (m_newTemplateValid)
		{
			if (m_creatingNewTemplate)
			{
				switch (m_mode)
				{
				case M_chassis:
					statusStr = "Creating NEW ship base template";
					break;
				case M_component:
					statusStr = "Creating NEW component base template";
					break;
				}
				
				pal.setColor(QColorGroup::Text, "blue");
			}
			else
			{
				switch (m_mode)
				{
				case M_chassis:
					statusStr = "Ship template ALREADY EXISTS";
					break;
				case M_component:
					statusStr = "Component template ALREADY EXISTS";
					break;
				}
				pal.setColor(QColorGroup::Text, "red");
			}
		}
		else
		{
			switch (m_mode)
			{
			case M_chassis:
				statusStr = "INVALID name for new ship template";
				break;
			case M_component:
				statusStr = "INVALID name for new component template";
				break;
			}
			
			pal.setColor(QColorGroup::Text, "red");
		}
		
		m_lineEditNewTemplate->setPalette(pal);
		m_textLabelStatusNewTemplate->setText(QString(statusStr.c_str()));
	}
	
	//-- auto populate the new base template field if needed
	
	if (m_newTemplateValid && !m_creatingNewTemplate)
	{
		m_lineEditBaseTemplate->setEnabled(false);
		std::string const & newTemplateName = Unicode::toLower(QStringUtil::toString(m_lineEditNewTemplate->text()));;
		
		ShipComponentEditorServerTemplate const * const editorServerTemplate = ShipComponentEditorServerTemplateManager::findTemplateByName(newTemplateName);
		std::string baseTemplateName;
		
		if (NULL != editorServerTemplate)
		{
			baseTemplateName = editorServerTemplate->baseServerTemplate;
			m_baseTemplateValid = true;
			
			//-- auto fill the shared template field
			m_lineEditSharedTemplate->setText(QString(editorServerTemplate->sharedTemplateName.c_str()));
			m_lineEditBaseTemplate->setText(QString(baseTemplateName.c_str()));
		}
	}
	else
	{
		m_lineEditBaseTemplate->setEnabled(true);
	}
	
	//-- update base template field
	{
		std::string statusStr;
		QPalette pal (m_lineEditBaseTemplate->palette());
		if (m_baseTemplateValid)
		{
			switch (m_mode)
			{
			case M_chassis:
				statusStr = "Using EXISTING ship base template";
				break;
			case M_component:
				statusStr = "Using EXISTING component base template";
				break;
			}
			pal.setColor(QColorGroup::Text, "black");
		}
		else
		{
			switch (m_mode)
			{
			case M_chassis:
				statusStr = "INVALID name for new ship base template";
				break;
			case M_component:
				statusStr = "INVALID name for new component base template";
				break;
			}
			pal.setColor(QColorGroup::Text, "red");
		}
		
		m_lineEditBaseTemplate->setPalette(pal);
		m_textLabelStatusBaseTemplate->setText(QString(statusStr.c_str()));
	}
	
	
	//----------------------------------------------------------------------
	
	
	//-- update shared template field
	{
		std::string statusStr;
		QPalette pal (m_lineEditSharedTemplate->palette());
		QPalette palStatus (m_textLabelStatusSharedTemplate->palette());
		if (m_sharedTemplateValid)
		{
			if (m_creatingSharedTemplate)
			{
				pal.setColor(QColorGroup::Text, "blue");
				palStatus.setColor(QColorGroup::Text, "blue");
				statusStr = "Creating NEW shared Template";
			}
			else
			{
				pal.setColor(QColorGroup::Text, "black");
				palStatus.setColor(QColorGroup::Text, "black");
				statusStr = "Using EXISTING shared Template";
			}
		}
		else
		{
			palStatus.setColor(QColorGroup::Text, "red");
			pal.setColor(QColorGroup::Text, "red");
			statusStr = "INVALID Name for shared Template";
		}
		
		m_lineEditSharedTemplate->setPalette(pal);
		
		m_textLabelStatusSharedTemplate->setPalette(palStatus);
		m_textLabelStatusSharedTemplate->setText(QString(statusStr.c_str()));
	}
	
	//-- auto populate the shared base template field if needed
	
	if (m_sharedTemplateValid && !m_creatingSharedTemplate)
	{
		m_lineEditSharedBaseTemplate->setEnabled(false);
		std::string const & sharedTemplateName = Unicode::toLower(QStringUtil::toString(m_lineEditSharedTemplate->text()));;
		ObjectTemplate const * const ot = ObjectTemplateList::fetch(sharedTemplateName);
		
		std::string sharedBaseTemplateName;
		
		if (NULL != ot)
		{
			ObjectTemplate const * const base_ot = ot->getBaseTemplate();
			if (NULL != base_ot)
			{
				sharedBaseTemplateName = base_ot->getName();
				m_sharedBaseTemplateValid = true;
			}
			ot->releaseReference();
		}
		
		m_lineEditSharedBaseTemplate->setText(QString(sharedBaseTemplateName.c_str()));
	}
	else
	{
		m_lineEditSharedBaseTemplate->setEnabled(true);
	}
	
	
	//-- update shared base template field
	{
		std::string statusStr;
		QPalette pal (m_lineEditSharedBaseTemplate->palette());
		if (m_sharedBaseTemplateValid)
		{
			statusStr = "Using EXISTING shared base template";
			pal.setColor(QColorGroup::Text, "black");
		}
		else
		{
			statusStr = "INVALID name for shared base template";
			pal.setColor(QColorGroup::Text, "red");
		}
		
		m_lineEditSharedBaseTemplate->setPalette(pal);
		m_textLabelStatusSharedBaseTemplate->setText(QString(statusStr.c_str()));
		
	}
	
	//-- update the create button
	
	bool const buttonCreateEnabled = m_newTemplateValid && m_creatingNewTemplate && m_baseTemplateValid && m_sharedTemplateValid && m_sharedBaseTemplateValid;
	
	m_pushButtonCreateTemplate->setEnabled(buttonCreateEnabled);
}

//----------------------------------------------------------------------

void TemplateNewDialog::populateChassisCombo()
{
	DEBUG_FATAL(M_chassis != m_mode, ("Invalid mode"));
	
	m_comboChassis->insertItem(QString("--- Select A Chassis ---"));
	
	typedef stdvector<std::string>::fwd StringVector;
	StringVector sv;
	{
		ShipChassis::PersistentCrcStringVector const & shipChassisNameVector = ShipChassis::getShipChassisCrcVector();
		for (ShipChassis::PersistentCrcStringVector::const_iterator it = shipChassisNameVector.begin(); it != shipChassisNameVector.end(); ++it)
		{
			PersistentCrcString const * const pcs = *it;
			sv.push_back(pcs->getString());
		}
		
		std::sort(sv.begin(), sv.end());
	}
	
	{
		for (StringVector::const_iterator it = sv.begin(); it != sv.end(); ++it)
		{
			std::string const & chassisName = *it;
			m_comboChassis->insertItem(chassisName.c_str());
		}
	}
	
	m_comboChassis->setCurrentItem(0);
}

//----------------------------------------------------------------------

void TemplateNewDialog::selectChassis(std::string const & selectChassisName)
{
	DEBUG_FATAL(M_chassis != m_mode, ("Invalid mode"));
	
	if (selectChassisName.empty())
	{
		m_comboChassis->setCurrentItem(0);
		return;
	}
	
	int selectedIndex = 0;
	int const numItems = m_comboChassis->count();
	for (int index = 0; index < numItems; ++index)
	{
		std::string const & chassisName = QStringUtil::toString(m_comboChassis->text(index));
		
		if (selectChassisName == chassisName)
		{
			selectedIndex = index;
		}
	}
	
	m_comboChassis->setCurrentItem(selectedIndex);
}

//----------------------------------------------------------------------

std::string const & TemplateNewDialog::showAndTell()
{
	exec();
	return m_resultTemplateName;
}

//======================================================================
