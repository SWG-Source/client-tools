//
// UILayer.cpp
// asommers 1-10-2001
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiLayer_TextureCanvas.h"
#include "clientUserInterface/CuiLayer_EngineCanvas.h"
#include "clientUserInterface/CuiLayer_Loader.h"

#include "StringId.h"
#include "UnicodeUtils.h"
#include "clientAudio/Audio.h"
#include "clientAudio/SoundTemplate.h"
#include "clientAudio/SoundTemplateList.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/Shader.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/VertexBuffer.h"
#include "clientUserInterface/CuiLayerRenderer.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiSoundManager.h"
#include "clientUserInterface/CuiSounds.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFile/TreeFile.h"
#include "sharedMath/PackedArgb.h"

#include <algorithm>
#include <map>

//----------------------------------------------------------------------

namespace
{	
	bool                   s_installed            = false;
	bool                   s_debugReportUi        = false;
	bool                   s_debugReportStrings   = false;


	typedef stdvector<StringId>::fwd StringIdVector;
	StringIdVector         s_stringList;
	bool                   s_generateStringList = false;
}

//-----------------------------------------------------------------

void  CuiLayer::install ()
{
	DEBUG_FATAL (s_installed, ("already installed\n"));

	InstallTimer const installTimer("CuiLayer");

	CuiLayerRenderer::install ();

	s_installed      = true;

	DebugFlags::registerFlag(s_debugReportUi,       "ClientUserInterface", "ui");
	DebugFlags::registerFlag(s_debugReportStrings,  "ClientUserInterface", "strings");

	Loader::install ();
}

//-----------------------------------------------------------------

void  CuiLayer::remove ()
{
	DEBUG_FATAL (!s_installed, ("not installed.\n"));
	
	CuiLayerRenderer::remove ();

	s_installed      = false;
}

//-----------------------------------------------------------------

UICanvas *CuiLayer::TextureCanvasFactory::CreateCanvas (const std::string & shaderName, const std::string & textureName) const
{
	UNREF (shaderName);
	return  new CuiLayer::TextureCanvas (shaderName, textureName);
}


//-------------------------------------------------------------------
//
// SoundCanvas
//-----------------------------------------------------------------

CuiLayer::SoundCanvas::SoundCanvas () :
UISoundCanvas    (),
m_soundTemplates (new SoundTemplateVector)
{
}

//-----------------------------------------------------------------

CuiLayer::SoundCanvas::~SoundCanvas ()
{
	for (SoundTemplateVector::const_iterator it = m_soundTemplates->begin (); it != m_soundTemplates->end (); ++it)
	{
		const SoundTemplate * const st = (*it).second;
		if (st)
			SoundTemplateList::release (st);
	}

	delete m_soundTemplates;
	m_soundTemplates = 0;
}

//-----------------------------------------------------------------

void CuiLayer::SoundCanvas::Play(char const * const filenameIn)
{
	NOT_NULL (filenameIn);

	char const * filename = filenameIn;
	std::string path;
	
	if (strstr(filename, "sound\\"))
	{
		filename += strlen("sound\\");
	}

	if (!strstr(filename, "sound/"))
	{
		path += std::string("sound/");
	}

	path += filename;
	
	if (!strstr(filename, ".snd"))
	{
		path += ".snd";
	}

	const SoundTemplateVector::const_iterator it = m_soundTemplates->find (path);
	if (it == m_soundTemplates->end ())
	{
		const SoundTemplate * const st = SoundTemplateList::fetch (path.c_str ());
		m_soundTemplates->insert (std::make_pair (path, st));
	}

	Audio::playSound (path.c_str (), NULL);
}

//----------------------------------------------------------------------

void CuiLayer::SoundCanvas::PlaySoundGenericNegative () const
{
	CuiSoundManager::play (CuiSounds::negative);
}

//-----------------------------------------------------------------

CuiLayer::StringFactory::StringFactory () : UILocalizedStringFactory ()
{
}

//-----------------------------------------------------------------

bool CuiLayer::StringFactory::GetLocalizedString (const std::string & name, Unicode::String & dest) const
{
	dest.clear ();

	static const std::string default_table_name = "ui";

	StringId id;
	static Unicode::String tmp;

	size_t pos       = 0;
	
	while (pos != std::string::npos)
	{
		const size_t space_pos = name.find_first_of (" \t\n", pos);
		
		const std::string & token = space_pos == std::string::npos ? (pos == 0 ? name : name.substr (pos)) : name.substr (pos, space_pos - pos);
		
		if (token.empty ())
			break;
		
		if (token [0] == '@')
		{
			const size_t colon_pos = token.find (':');
			
			if (colon_pos != name.npos)
			{
				id.setTable (token.substr (1, colon_pos - 1));
				id.setText  (token.substr (colon_pos + 1));
			}
			else
			{
				id.setTable (default_table_name);
				id.setText  (token.substr (1));
			}
			
			if (s_generateStringList)
				s_stringList.push_back (id);
			
			tmp.clear ();
			id.localize (tmp);

			dest += tmp;
		}
		else
			dest += Unicode::narrowToWide (token);
		
		if (space_pos != std::string::npos)
		{
			const size_t non_space_pos = name.find_first_not_of (" \t\n", space_pos);
			if (non_space_pos != std::string::npos)
			{
				dest += Unicode::narrowToWide (name.substr (space_pos, non_space_pos - space_pos));
				pos = non_space_pos;
				continue;
			}
			else
			{
				dest += Unicode::narrowToWide (name.substr (space_pos));
			}
		}

		break;
	}
	
	return true;
}

//----------------------------------------------------------------------

void CuiLayer::StringFactory::generateStringList ()
{
	s_stringList.clear ();
	s_stringList.reserve (1024);
	s_generateStringList = true;
}

//----------------------------------------------------------------------

const CuiLayer::StringFactory::StringIdVector & CuiLayer::StringFactory::getStringList  ()
{
	std::sort (s_stringList.begin (), s_stringList.end ());
	s_stringList.erase (std::unique (s_stringList.begin (), s_stringList.end ()), s_stringList.end ());
	return s_stringList;
}

//-----------------------------------------------------------------
