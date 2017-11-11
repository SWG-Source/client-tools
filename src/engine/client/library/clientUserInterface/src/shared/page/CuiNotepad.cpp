//======================================================================
//
// CuiNotepad.cpp
// copyright(c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiNotepad.h"

#include "fileInterface/StdioFile.h"

#include "sharedFoundation/NetworkId.h"
#include "clientGame/Game.h"
#include "clientUserInterface/CuiManager.h"

#include "StringId.h"
#include "UIManager.h"
#include "UIText.h"

// =====================================================================

namespace CuiNotepadNamespace
{
	const std::string cs_defaultFileName  = "notes.txt";
	std::string s_userSpecifiedFileName;
	const int         cs_maxLength = 1024 * 1024; 

	std::string getDefaultFilePath ()
	{
		std::string     loginId;
		std::string     cluster;
		Unicode::String playerName;
		NetworkId       id;
		
		if (!Game::getPlayerPath (loginId, cluster, playerName, id))
			return std::string ();
		
		return std::string ("profiles/") + loginId + std::string("/") + cs_defaultFileName;
	}

	std::string getUserSpecifiedFilePath ()
	{
		std::string     loginId;
		std::string     cluster;
		Unicode::String playerName;
		NetworkId       id;

		if (s_userSpecifiedFileName.empty())
			return std::string ();

		if (!Game::getPlayerPath (loginId, cluster, playerName, id))
			return std::string ();

		return std::string ("profiles/") + loginId + std::string("/") + s_userSpecifiedFileName + std::string(".txt");
	}
}

using namespace CuiNotepadNamespace;

// =====================================================================

CuiNotepad::CuiNotepad (UIPage & page)
: CuiMediator      ("CuiNotepad", page),
  UIEventCallback  (),
  m_noteText       (0),
  m_windowTitle    (0),
  m_currentFilePath()
{
	IGNORE_RETURN(setState(MS_closeable));

	getCodeDataObject(TUIText,   m_noteText, "text");
	m_noteText->Clear();

	getCodeDataObject(TUIText,   m_windowTitle, "bg.caption.text");
}

//-----------------------------------------------------------------

void CuiNotepad::performActivate()
{
	CuiManager::requestPointer (true);

	loadFromFile();
}

//-----------------------------------------------------------------

void CuiNotepad::performDeactivate()
{
	CuiManager::requestPointer (false);

	saveToFile();
}

//-----------------------------------------------------------------

void CuiNotepad::loadFromFile()
{
	// don't need to do anything if currently displaying the file 
	if (!m_currentFilePath.empty())
	{
		std::string fileName = getUserSpecifiedFilePath();
		if (fileName.empty())
			fileName = getDefaultFilePath();

		if (fileName == m_currentFilePath)
			return;

		m_currentFilePath = fileName;
	}
	else
	{
		m_currentFilePath = getUserSpecifiedFilePath();
		if (m_currentFilePath.empty())
			m_currentFilePath = getDefaultFilePath();
	}

	StdioFileFactory fact;
	AbstractFile* f = NULL;

	if (!m_currentFilePath.empty())
		f = fact.createFile(m_currentFilePath.c_str(), "r");

	if(f && f->isOpen())
	{
		int length = f->length();
		if(length > cs_maxLength * sizeof(unsigned short) + 1)
			length = cs_maxLength * sizeof(unsigned short) + 1;

		char* const buffer = new char[static_cast<unsigned int>(length+1)];
		memset(buffer, 0, static_cast<unsigned int>(length+1));
		IGNORE_RETURN(f->read(buffer, length));
		f->close();

		bool m_unicodeMode = false;
		{
			//detect mode
			if(buffer[0] == '\377')
				m_unicodeMode = true;
		}			
		Unicode::String str;
		if(m_unicodeMode)
		{
			//This reinterpret_cast is neccessary to interpret the char* data read from the file
			unsigned short *unicodeBuffer = reinterpret_cast<unsigned short *>(buffer + 1);
			str = Unicode::String(unicodeBuffer);
		}
		else
		{				
			str = Unicode::narrowToWide(buffer);
		}

		m_noteText->Clear();
		m_noteText->SetLocalText(str);
		delete[] buffer;
	}
	else
	{
		m_noteText->Clear();
	}

	delete f;

	Unicode::String defaultTitle(StringId("ui_notepad", "title").localize());
	Unicode::String title(defaultTitle);
	if (!m_currentFilePath.empty())
	{
		title = Unicode::narrowToWide(m_currentFilePath);
		title += Unicode::narrowToWide(" - ");
		title += defaultTitle;
	}

	if (m_windowTitle)
		m_windowTitle->SetLocalText(title);
}

//-----------------------------------------------------------------

void CuiNotepad::saveToFile()
{
	if (m_currentFilePath.empty())
		return;

	StdioFileFactory fact;
	AbstractFile* f = fact.createFile(m_currentFilePath.c_str(), "w");
	if(f && f->isOpen())
	{
		UIString const & localText = m_noteText->GetLocalText();
		bool isUnicode = Unicode::isUnicode(localText);
		if(isUnicode)
		{			
			int length = static_cast<int>(localText.length()) * sizeof(unsigned short);
			if(length > cs_maxLength * sizeof(unsigned short))
				length = cs_maxLength * sizeof(unsigned short);
			char tmp = '\377';
			IGNORE_RETURN(f->write(1, &tmp));
			IGNORE_RETURN(f->write(length, reinterpret_cast<const char *>(localText.c_str())));
			f->close();
		}
		else
		{
			std::string const & localTextNarrow = Unicode::wideToNarrow(localText);
			int length = static_cast<int>(localTextNarrow.length());
			if(length > cs_maxLength)
				length = cs_maxLength;
			IGNORE_RETURN(f->write(length, localTextNarrow.c_str()));
			f->close();
		}
	}

	delete f;
}

//-----------------------------------------------------------------

void CuiNotepad::setUserSpecifiedFileName(const std::string & fileName)
{
	// replace \ with /
	std::string editedFileName = fileName;
	std::string::size_type backSlashPos = editedFileName.find('\\');
	while (backSlashPos != std::string::npos)
	{
		editedFileName[backSlashPos] = '/';
		backSlashPos = editedFileName.find('\\');
	}

	s_userSpecifiedFileName = editedFileName;
}

//=================================================================
