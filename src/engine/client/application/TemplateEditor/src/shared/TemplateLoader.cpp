// ============================================================================
//
// TemplateLoader.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstTemplateEditor.h"
#include "TemplateLoader.h"

#include "sharedCompression/SetupSharedCompression.h"
#include "sharedDebug/SetupSharedDebug.h"
#include "sharedFile/SetupSharedFile.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/PerThreadData.h"
#include "sharedFoundation/SetupSharedFoundation.h"
#include "sharedObject/SetupSharedObject.h"
#include "sharedRandom/SetupSharedRandom.h"
#include "sharedThread/SetupSharedThread.h"
#include "sharedTemplateDefinition/File.h"
#include "sharedTemplateDefinition/TemplateDefinitionFile.h"
#include "sharedTemplateDefinition/TpfFile.h"

// Template installs

#include "sharedTemplate/ServerUberObjectTemplate.h"
#include "sharedTemplate/SetupSharedTemplate.h"

///////////////////////////////////////////////////////////////////////////////

#include "TemplateEditorUtility.h"

// ============================================================================
//
// TemplateLoader
//
// ============================================================================

TpfFile *                     TemplateLoader::m_tpfFile = NULL;
TemplateDefinitionFile *      TemplateLoader::m_TemplateDefinitionFile = NULL;
QString                       TemplateLoader::m_tpfPath;
TemplateLoader::TpfFileVector TemplateLoader::m_tpfFileVector;

#include <direct.h>

//-----------------------------------------------------------------------------
void TemplateLoader::install()
{
	SetupSharedThread::install();
	SetupSharedDebug::install(4096);

	{
		SetupSharedFoundation::Data data(SetupSharedFoundation::Data::D_console);

		char currentWorkingDirectory[_MAX_PATH];
		getcwd(currentWorkingDirectory, sizeof(currentWorkingDirectory));
		
		if (strstr(currentWorkingDirectory, "win32") == NULL)
		{
			DEBUG_FATAL(true, ("Make sure the working directory is set correctly. Currently: %s", currentWorkingDirectory));
		}

		data.configFile = "tools.cfg";

		SetupSharedFoundation::install(data);
	}

	SetupSharedCompression::install();

	SetupSharedFile::install(false);

	TreeFile::addSearchAbsolute (20);

	// setup the random number generator
	SetupSharedRandom::install(static_cast<uint32>(time(NULL)));

	// install templates

	SetupSharedTemplate::install();
	ServerUberObjectTemplate::install();

#ifdef WIN32
	// find out what platform we are running on

	DWORD version = GetVersion();
	if (version & 0x80000000)
	{
		WindowsUnicode = false;
	}
	else
	{
		WindowsUnicode = true;
	}
#endif

	delete m_TemplateDefinitionFile;
	m_TemplateDefinitionFile = new TemplateDefinitionFile();
	delete m_tpfFile;
	m_tpfFile = new TpfFile();
}

//-----------------------------------------------------------------------------
void TemplateLoader::remove()
{
	SetupSharedFoundation::remove();
	PerThreadData::threadRemove();

	delete m_TemplateDefinitionFile;
	m_TemplateDefinitionFile = NULL;
	delete m_tpfFile;
	m_tpfFile = NULL;

	resetTpfFileVector();
}

//-----------------------------------------------------------------------------
void TemplateLoader::resetTpfFileVector()
{
	while (!m_tpfFileVector.empty())
	{
		TpfFile *tpfFile = m_tpfFileVector.back();

		m_tpfFileVector.pop_back();

		delete tpfFile;
	}
}

////-----------------------------------------------------------------------------
//bool TemplateLoader::loadTdf(QString const &path)
//{
//	File::setBasePath("");
//
//	File fileTdf(path, "rt");
//	
//	bool result = fileTdf.isOpened();
//
//	if (result)
//	{
//		result = (m_TemplateDefinitionFile->parse(fileTdf) == 0);
//
//		if (m_TemplateDefinitionFile->getTemplateName() == ROOT_TEMPLATE_NAME)
//		{
//			/*
//			fprintf(stderr, "Warning: will not generate C++ code for class %s, "
//				"invalid template name\n", TemplateDefinitionFile.getTemplateName().c_str());
//			*/
//			//result = AbstractTemplateLoaded;
//		}
//
//		fileTdf.close();
//	}
//
//	return result;
//}

//-----------------------------------------------------------------------------
bool TemplateLoader::loadTpf(QString const &path)
{
	Filename filenameTpf(NULL, NULL, path.latin1(), TEMPLATE_EXTENSION);
	int result = m_tpfFile->loadTemplate(filenameTpf);

	if (result == 0)
	{
		m_tpfPath = path;

		// Load all the inherited Tpf Files

		loadInheritedTpfFiles(path);
	}

	return (result == 0);
}

//-----------------------------------------------------------------------------
TemplateData &TemplateLoader::getTemplateData()
{
	int const version = m_tpfFile->getTemplateDefinitionFile()->getHighestVersion();
	TemplateData *templateData = m_tpfFile->getTemplateDefinitionFile()->getTemplateData(version);

	NOT_NULL(templateData);

	return *templateData;
}

//-----------------------------------------------------------------------------
TpfTemplate &TemplateLoader::getTpfTemplate()
{
	TpfTemplate *tpfTemplate = m_tpfFile->getTemplate();
	NOT_NULL(tpfTemplate);

	return *tpfTemplate;
}

//-----------------------------------------------------------------------------
QString const &TemplateLoader::getTpfPath()
{
	return m_tpfPath;
}

//-----------------------------------------------------------------------------
TpfFile *TemplateLoader::getTpfFileForParameter(QString const &parameterName)
{
	TpfFile *result = NULL;

	if (parameterName == "@base")
	{
		result = (*m_tpfFileVector.begin());
	}
	else
	{
		TpfFileVector::const_iterator iterTpfFileVector = m_tpfFileVector.begin();

		for (; iterTpfFileVector != m_tpfFileVector.end(); ++iterTpfFileVector)
		{
			TpfFile *tpfFileCurrent = (*iterTpfFileVector);
			TpfTemplate *tpfTemplateCurrent = tpfFileCurrent->getTemplate();

			// If the parameter is loaded or if the parameter is defined as pure
			// virtual (@derived), then this is the tpf file we need.

			bool const paramLoaded = tpfTemplateCurrent->isParamLoaded(parameterName.latin1(), false);
			bool const paramPureVirtual = tpfTemplateCurrent->isParamPureVirtual(parameterName.latin1(), false);

			if (paramLoaded || paramPureVirtual)
			{
				result = tpfFileCurrent;
				break;
			}
		}

		// See if we need to handle bad parameter data

		if (iterTpfFileVector == m_tpfFileVector.end())
		{
			QString text;
			text.sprintf("Error loading parameter (%s). The parameter is not defined in the tpf hierarchy for: (%s) Defaulting parameter to @derived.", parameterName.latin1(), FilePath(TemplateLoader::getTpfPath().latin1()).get(FilePath::fileName | FilePath::extension));

			TemplateEditorUtility::report(text);

			if (m_tpfFileVector.size() > 0)
			{
				result = (*m_tpfFileVector.begin());
			}
		}
	}

	return result;
}

//-----------------------------------------------------------------------------
void TemplateLoader::loadInheritedTpfFiles(QString const &path)
{
	// Reset the tpf file vector we are about to recreate

	resetTpfFileVector();

	// Start checking the parent's parameters

	FilePath filePath(path.latin1());

	QDir dir(filePath.get(FilePath::drive | FilePath::directory).c_str());
	QString baseTemplateName(path);

	// Insert the initial tpf file

	TpfFile *tpfFile = new TpfFile();

	bool done = false;

	while (!done)
	{
		if (!baseTemplateName.isNull())
		{
			Filename fileNameTpf(NULL, dir.absPath(), baseTemplateName.latin1(), TEMPLATE_EXTENSION);

			QFileInfo fileInfo(fileNameTpf.getFullFilename().c_str());
			bool cdUp = false;

			if (fileInfo.isFile() && fileInfo.exists())
			{
				int result = tpfFile->loadTemplate(fileNameTpf);

				if (result == 0)
				{
					// Save this tpf file since it loaded

					m_tpfFileVector.push_back(tpfFile);

					TpfTemplate *tpfTemplate = tpfFile->getTemplate();

					if (tpfTemplate != NULL)
					{
						// Get the new base template name

						baseTemplateName = tpfFile->getTemplate()->getBaseTemplateName().c_str();

						// Create a new tpf file

						tpfFile = new TpfFile();

						// Reset the directory path

						dir = filePath.get(FilePath::drive | FilePath::directory).c_str();
					}
					else
					{
						delete tpfFile;
						done = true;
					}
				}
				else
				{
					cdUp = true;
				}
			}
			else
			{
				cdUp = true;
			}

			if (cdUp)
			{
				// Move up a directory

				bool result = dir.cdUp();

				if (!result || baseTemplateName.isEmpty())
				{
					// We are done looking for the base template

					delete tpfFile;
					done = true;
				}
			}
		}
		else
		{
			delete tpfFile;
			done = true;
		}
	}
}

// ============================================================================
