
//======================================================================
//
// CuiChatHistory.cpp
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiChatHistory.h"

#include "Unicode.h"
#include "clientGame/Game.h"
#include "clientGraphics/Graphics.h"
#include "clientUserInterface/ConfigClientUserInterface.h"
#include "fileInterface/AbstractFile.h"
#include "fileInterface/StdioFile.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/Os.h"
#include <map>

//======================================================================

// This file is almost a complete copy of CuiSettings, but changed specifically
// to save player's chat history. The reason it is done this way is I just don't
// feel like rewriting our two incompatible and inflexible save systems into some-
// thing that's actually usable in multiple situations.

//======================================================================

namespace CuiChatHistoryNamespace
{
	struct Data
	{
		UISize  size;
		UIPoint location;
		bool    sizeOk;
		bool    locationOk;

		Data () : size (), location (), sizeOk (false), locationOk (false)
		{
		}

		typedef stdmap<std::string, Unicode::String>::fwd StringMap;
		typedef stdmap<std::string, std::string>::fwd     DataMap;
		typedef stdmap<std::string, int>::fwd             IntMap;

		StringMap strings;
		DataMap   data;
		IntMap    integers;

		bool findString  (const std::string & key, Unicode::String & value) const;
		void setString   (const std::string & key, const Unicode::String & value);

		bool findInteger (const std::string & key, int & value) const;
		void setInteger  (const std::string & key, const int value);

		bool findData    (const std::string & key, std::string & value) const;
		void setData     (const std::string & key, const std::string & value);
		
		template <typename T> bool findGeneric (const std::string & key, T & value, const std::map<std::string, T> & source) const
		{
			const std::map<std::string, T>::const_iterator it = source.find (key);
			if (it != source.end ())
			{
				value = (*it).second;
				return true;
			}
			return false;
		}

		template <typename T> void setGeneric (const std::string & key, const T & value, std::map<std::string, T> & source)
		{
			if (value.empty ())
				source.erase (key);
			else
				source [key] = value;
		}

		void setGeneric (const std::string & key, const int & value, std::map<std::string, int> & source)
		{
			source [key] = value;
		}

	};

	bool Data::findString (const std::string & key, Unicode::String & value) const
	{
		return findGeneric (key, value, strings);
	}
	
	void Data::setString  (const std::string & key, const Unicode::String & value)
	{
		setGeneric (key, value, strings);
	}

	bool Data::findData   (const std::string & key, std::string & value) const
	{
		return findGeneric (key, value, data);
	}

	void Data::setData    (const std::string & key, const std::string & value)
	{
		setGeneric (key, value, data);
	}

	bool Data::findInteger  (const std::string & key, int & value) const
	{
		return findGeneric (key, value, integers);
	}

	void Data::setInteger   (const std::string & key, const int value)
	{
		setGeneric (key, value, integers);
	}

	typedef stdmap<std::string, Data>::fwd DataMap;

	DataMap s_dataAvatar;
	DataMap s_dataCluster;
	DataMap s_dataAccount;
	DataMap s_dataGlobal;

	UISize  s_screenResolution;
	std::string s_prefixString;

	DataMap * s_dataMaps [] =
	{
		&s_dataAvatar,
		&s_dataCluster,
		&s_dataAccount,
		&s_dataGlobal
	};

	const int numDataMaps = static_cast<int>(sizeof (s_dataMaps) / sizeof (s_dataMaps [0]));

	Data * findData (const std::string & _owner)
	{
		std::string const & modifiedOwnerString = s_prefixString + _owner;

		for (int i = 0; i < numDataMaps; ++i)
		{
			DataMap & dataMap = *s_dataMaps [i];
			const DataMap::iterator it = dataMap.find (modifiedOwnerString);

			if (it != dataMap.end ())
				return &(*it).second;
		}

		return 0;
	}

	bool clearData(std::string const & owner)
	{
		std::string const & modifiedOwnerString = s_prefixString + owner;

		for (int i = 0; i < numDataMaps; ++i)
		{
			DataMap & dataMap = *s_dataMaps[i];
			const DataMap::iterator it = dataMap.find(modifiedOwnerString);

			if (it != dataMap.end())
			{
				dataMap.erase(it);
				return true;
			}
		}
		return false;
	}

	Data & findOrInsertData (const std::string & _owner)
	{
		std::string const & modifiedOwnerString = s_prefixString + _owner;

		DataMap & dataMap = *s_dataMaps [0];
		const DataMap::iterator it = dataMap.find (modifiedOwnerString);
		if (it != dataMap.end ())
			return (*it).second;

		else 
			return dataMap [modifiedOwnerString];
	}
	
	bool createFilename (std::string & filename)
	{
		std::string     loginId;
		std::string     clusterName;
		Unicode::String playerName;
		NetworkId       id;
		
		Game::getPlayerPath (loginId, clusterName, playerName, id);

		if (loginId.empty () || clusterName.empty () || id == NetworkId::cms_invalid)
			return false;
		
		static const std::string prefix = "profiles/";
		static const std::string slash = "/";
		static const std::string suffix = ".cht";
		
		filename = prefix + loginId + slash + clusterName + slash + id.getValueString () + suffix;
		return true;
	}

	std::string s_currentFilename;
	bool        s_resettingForPlayer = false;

	namespace Tags
	{
		const Tag UIST = TAG (U,I,S,T);  // ui settings file
		const Tag SIZE = TAG (S,I,Z,E);
		const Tag LOCA = TAG (L,O,C,A);  // location
		const Tag OWNE = TAG (O,W,N,E);  // owner entry
		const Tag STRS = TAG (S,T,R,S);  // string map
		const Tag DATS = TAG (D,A,T,S);  // data map
		const Tag INTS = TAG (I,N,T,S);  // integer map
		const Tag RESO = TAG (R,E,S,O);  // screen resolution
	}

	//----------------------------------------------------------------------
	
	bool load_owner_0003 (Iff & iff, std::string & owner, Data & data)
	{
		if (iff.enterForm (Tags::OWNE, true))
		{
			iff.enterChunk (TAG_NAME);
			{
				owner = iff.read_stdstring ();
			}
			iff.exitChunk (TAG_NAME);
			
			if (iff.enterChunk (Tags::SIZE, true))
			{
				data.size.x = iff.read_int32 ();
				data.size.y = iff.read_int32 ();
				data.sizeOk = true;
				iff.exitChunk (Tags::SIZE);
			}
			
			if (iff.enterChunk (Tags::LOCA, true))
			{
				data.location.x = iff.read_int32 ();
				data.location.y = iff.read_int32 ();
				data.locationOk = true;
				iff.exitChunk (Tags::LOCA);
			}

			//----------------------------------------------------------------------

			if (iff.enterForm (Tags::STRS, true))
			{
				while (iff.enterChunk (TAG_DATA, true))
				{
					const std::string & key       = iff.read_stdstring ();
					const Unicode::String & value = iff.read_unicodeString ();
					if (!data.strings.insert (std::make_pair (key, value)).second)
						WARNING (true, ("CuiChatHistory attempt to load duplicate string into [%s] map [%s]:[%s]", owner.c_str (), key.c_str (), Unicode::wideToNarrow (value).c_str ()));

					iff.exitChunk (TAG_DATA);
				}

				iff.exitForm (Tags::STRS);
			}

			//----------------------------------------------------------------------

			if (iff.enterForm (Tags::DATS, true))
			{
				while (iff.enterChunk (TAG_DATA, true))
				{
					const std::string & key   = iff.read_stdstring ();

					const int size = iff.read_int32 ();
					unsigned char * buf     = new unsigned char [size];
					iff.read_uint8 (size, buf);
					const std::string value (reinterpret_cast<char *>(buf), size);
					delete [] buf;

					if (!data.data.insert (std::make_pair (key, value)).second)
						WARNING (true, ("CuiChatHistory attempt to load duplicate data into [%s] map [%s]:[%s]", owner.c_str (), key.c_str (), value.c_str ()));

					iff.exitChunk (TAG_DATA);
				}

				iff.exitForm (Tags::DATS);
			}

			//----------------------------------------------------------------------

			if (iff.enterForm (Tags::INTS, true))
			{
				while (iff.enterChunk (TAG_DATA, true))
				{
					const std::string & key  = iff.read_stdstring ();
					const int32 value        = iff.read_int32 ();
					if (!data.integers.insert (std::make_pair (key, value)).second)
						WARNING (true, ("CuiChatHistory attempt to load duplicate integer into [%s] map [%s]:[%d]", owner.c_str (), key.c_str (), value));

					iff.exitChunk (TAG_DATA);
				}
			
				iff.exitForm (Tags::INTS);
			}
	
			//----------------------------------------------------------------------

			iff.exitForm (Tags::OWNE);
			
			return true;
		}
		return false;
	}
	
	//----------------------------------------------------------------------
	
	void load_0003 (Iff & iff)
	{
		iff.enterForm (TAG_0003);
		{			
			bool localSizeLocationOk = true;
			
			const UISize currentResolution (Graphics::getCurrentRenderTargetWidth (), Graphics::getCurrentRenderTargetHeight ());
			
			if (iff.enterChunk (Tags::RESO, true))
			{
				s_screenResolution.x = iff.read_int32 ();
				s_screenResolution.y = iff.read_int32 ();
				
				localSizeLocationOk = (currentResolution.x == s_screenResolution.x && currentResolution.y == s_screenResolution.y);

				if (!localSizeLocationOk)
				{
					REPORT_LOG(true, ("Resetting UI due to screen resolution change from (%dx%d) to (%dx%d)\n", s_screenResolution.x, s_screenResolution.y, currentResolution.x, currentResolution.y));
				}
				
				iff.exitChunk (Tags::RESO);
			}
			else
			{
				s_screenResolution = currentResolution;
			}
			
			for (;;)
			{
				std::string owner;
				Data data;
				
				if (!load_owner_0003 (iff, owner, data))
					break;

				data.sizeOk     = data.sizeOk     && localSizeLocationOk;
				data.locationOk = data.locationOk && localSizeLocationOk;
				
				(*s_dataMaps [0])[owner] = data;
			}
		}
		
		iff.exitForm (TAG_0003);
	}

	//----------------------------------------------------------------------
	
	void save_owner_0003 (Iff & iff, const std::string & owner, const Data & data)
	{
		iff.insertForm (Tags::OWNE);
		{			
			iff.insertChunk (TAG_NAME);
			{
				iff.insertChunkString (owner.c_str ());
			}
			iff.exitChunk (TAG_NAME);
			
			if (data.sizeOk)
			{
				iff.insertChunk (Tags::SIZE);
				{
					iff.insertChunkData (static_cast<int32>(data.size.x));
					iff.insertChunkData (static_cast<int32>(data.size.y));
				}
				iff.exitChunk (Tags::SIZE);
			}
			
			if (data.locationOk)
			{
				iff.insertChunk (Tags::LOCA);
				{
					iff.insertChunkData (static_cast<int32>(data.location.x));
					iff.insertChunkData (static_cast<int32>(data.location.y));
				}
				iff.exitChunk (Tags::LOCA);
			}
		
			//----------------------------------------------------------------------

			iff.insertForm (Tags::STRS);
			{
				for (Data::StringMap::const_iterator it = data.strings.begin (); it != data.strings.end (); ++it)
				{
					iff.insertChunk (TAG_DATA);
					{
						const std::string & key       = (*it).first;
						const Unicode::String & value = (*it).second;

						iff.insertChunkString (key.c_str ());
						iff.insertChunkString (value);
					}
					iff.exitChunk (TAG_DATA);
				}
			}
			iff.exitForm (Tags::STRS);

			//----------------------------------------------------------------------

			iff.insertForm (Tags::DATS);
			{
				for (Data::DataMap::const_iterator it = data.data.begin (); it != data.data.end (); ++it)
				{
					iff.insertChunk (TAG_DATA);
					{
						const std::string & key   = (*it).first;
						const std::string & value = (*it).second;

						iff.insertChunkString (key.c_str ());
						const int32 size = static_cast<int32>(value.size ());
						iff.insertChunkData  (size);
						iff.insertChunkArray (reinterpret_cast<const uint8 *>(value.data ()), size);
					}
					iff.exitChunk (TAG_DATA);
				}
			}
			iff.exitForm (Tags::DATS);

			//----------------------------------------------------------------------
			
			iff.insertForm (Tags::INTS);
			{
				for (Data::IntMap::const_iterator it = data.integers.begin (); it != data.integers.end (); ++it)
				{
					iff.insertChunk (TAG_DATA);
					{
						const std::string & key     = (*it).first;
						const int32 value           = static_cast<int32>((*it).second);
						
						iff.insertChunkString (key.c_str ());
						iff.insertChunkData   (value);
					}
					iff.exitChunk (TAG_DATA);
				}
			}
			iff.exitForm (Tags::INTS);
		}
		iff.exitForm (Tags::OWNE);
	}

	//----------------------------------------------------------------------

	void save_0003 (Iff & iff)
	{
		iff.insertForm (Tags::UIST);
		iff.insertForm (TAG_0003);

		const UISize currentResolution (Graphics::getCurrentRenderTargetWidth (), Graphics::getCurrentRenderTargetHeight ());

		iff.insertChunk (Tags::RESO);
		{
			iff.insertChunkData (static_cast<int32>(currentResolution.x));
			iff.insertChunkData (static_cast<int32>(currentResolution.y));	
			iff.exitChunk (Tags::RESO);
		}

		DataMap & dataMap = *s_dataMaps [0];
		for (DataMap::const_iterator it = dataMap.begin (); it != dataMap.end (); ++it)
		{
			const std::string & owner = (*it).first;
			const Data & data         = (*it).second;

			save_owner_0003 (iff, owner, data);
		}

		iff.exitForm (TAG_0003);
		iff.exitForm (Tags::UIST);
	}

	int                    s_debugCounter         = 0;
	bool                   s_debugSettings        = false;
	bool                   ms_dirty               = false;
	float s_elapsedSecondsSinceLastSave = 0.0f;
	bool s_isNewPlayer = true;
	bool s_enableAutomaticSave = false;
}

using namespace CuiChatHistoryNamespace;

//----------------------------------------------------------------------

void CuiChatHistory::install ()
{
	DebugFlags::registerFlag (s_debugSettings,       "ClientUserInterface", "settings");
	setDirty (false);
}

//----------------------------------------------------------------------

void CuiChatHistory::remove  ()
{
	CuiChatHistory::clear ();
}

//----------------------------------------------------------------------

void CuiChatHistory::clear ()
{
	for (int i = 0; i < numDataMaps; ++i)
	{
		DataMap & dataMap = *s_dataMaps [i];
		dataMap.clear ();
	}
	s_currentFilename.clear ();
	setDirty (false);
}

//----------------------------------------------------------------------

void CuiChatHistory::load()
{
	s_isNewPlayer = true;

	clear();
	
	std::string filename;
	if (!createFilename (filename))
		return;
	
	s_resettingForPlayer = false;

	if (!ConfigClientUserInterface::getSettingsEnabled ())
		return;

	AbstractFile * const file = new StdioFile (filename.c_str(), "rb");
	
	s_currentFilename = filename;

	if (!file)
		return;

	if (file->isOpen ())
	{
		if (!Iff::isValid(filename.c_str()))
			WARNING (true, ("Data file %s is not valid.", filename.c_str ()));
		else
		{		
			Iff iff;
			iff.open (*file, filename.c_str());
			
			if (iff.enterForm (Tags::UIST, true))
			{
				switch (iff.getCurrentName ())
				{
				case TAG_0000:
				case TAG_0001:
				case TAG_0002:
					WARNING (true, ("CuiChatHistory version < 0003 no longer supported"));
					iff.exitForm (Tags::UIST, true);
					break;
				case TAG_0003:
					load_0003 (iff);
					iff.exitForm (Tags::UIST);
					setDirty (false);
					s_isNewPlayer = false;
					break;
				default:
					WARNING (true, ("invalid CuiChatHistory"));
					iff.exitForm (Tags::UIST, true);
					break;
				}
				REPORT_LOG_PRINT (true, ("CuiChatHistory loaded inventory state from %s\n", filename.c_str ()));
			}
		}
	}

	delete file;
}

//----------------------------------------------------------------------

void CuiChatHistory::save()
{
	if (isDirty() || s_resettingForPlayer) 
	{
		if (s_currentFilename.empty())
			return;
		
		if (!ConfigClientUserInterface::getSettingsEnabled())
			return;
		
		if (s_resettingForPlayer)
		{
			//-- open for writing, zeroing the file
			AbstractFile * const file = new StdioFile (s_currentFilename.c_str(), "wb");
			
			if (!file)
				return;
			
			delete file;
		}
		else
		{
			DEBUG_REPORT_LOG (s_debugSettings, ("CuiChatHistory::%-30s [%03d] [%30s]\n", "save", s_debugCounter++, s_currentFilename.c_str ()));
			
			const size_t lastSlash = s_currentFilename.rfind ('/');
			IGNORE_RETURN (Os::createDirectories (s_currentFilename.substr (0, lastSlash).c_str ()));
			Iff iff (8196, true);
			
			save_0003 (iff);
			if (!iff.write (s_currentFilename.c_str (), true))
				WARNING (true, ("Unable to write iff: %s", s_currentFilename.c_str ()));
		}

		setDirty(false);
	}
}

//----------------------------------------------------------------------

void CuiChatHistory::saveSize     (const std::string & owner, const UISize & size)
{
	DEBUG_REPORT_LOG (s_debugSettings, ("CuiChatHistory::%-30s [%03d] [%30s] [%d,%d]\n", "saveSize", s_debugCounter++, owner.c_str (), size.x, size.y));
	Data & data = findOrInsertData (owner);
	data.size           = size;
	data.sizeOk         = true;
}

//----------------------------------------------------------------------

void CuiChatHistory::saveLocation (const std::string & owner, const UIPoint & location)
{
	DEBUG_REPORT_LOG (s_debugSettings, ("CuiChatHistory::%-30s [%03d] [%30s] [%d,%d]\n", "saveLocation", s_debugCounter++, owner.c_str (), location.x, location.y));
	Data & data = findOrInsertData (owner);
	data.location = location;
	data.locationOk = true;
	setDirty (true);
}

//----------------------------------------------------------------------

void CuiChatHistory::saveString   (const std::string & owner, const std::string & key, const Unicode::String & value)
{
	Data & data = findOrInsertData (owner);
	data.setString (key, value);
	setDirty (true);
}

//----------------------------------------------------------------------

void CuiChatHistory::saveData     (const std::string & owner, const std::string & key, const std::string & value)
{
	Data & data = findOrInsertData (owner);
	data.setData (key, value);
}

//----------------------------------------------------------------------

void CuiChatHistory::saveInteger  (const std::string & owner, const std::string & key, const int value)
{
	Data & data = findOrInsertData (owner);
	data.setInteger (key, value);
	setDirty (true);
}

//----------------------------------------------------------------------

void CuiChatHistory::saveBoolean (const std::string & owner, const std::string & key, const bool value)
{
	Data & data = findOrInsertData (owner);
	data.setInteger (key, value ? 1 : 0);
	setDirty (true);
}

//----------------------------------------------------------------------

bool CuiChatHistory::loadSize     (const std::string & owner, UISize & size)
{
	const Data * const data = findData (owner);
	if (data)
	{
		if (data->sizeOk)
		{
			size = data->size;
			DEBUG_REPORT_LOG (s_debugSettings, ("CuiChatHistory::%-30s [%03d] [%30s] [%d,%d]\n", "loadSize", s_debugCounter++, owner.c_str (), size.x, size.y));
			return true;
		}
	}
	DEBUG_REPORT_LOG (s_debugSettings, ("CuiChatHistory::%-30s [%03d] [%30s] failed\n", "loadSize", s_debugCounter++, owner.c_str ()));
	return false;
}

//----------------------------------------------------------------------

bool CuiChatHistory::loadLocation (const std::string & owner, UIPoint & location)
{
	const Data * const data = findData (owner);
	if (data)
	{
		if (data->locationOk)
		{
			location = data->location;
			DEBUG_REPORT_LOG (s_debugSettings, ("CuiChatHistory::%-30s [%03d] [%30s] [%d,%d]\n", "loadLocation", s_debugCounter++, owner.c_str (), location.x, location.y));
			return true;
		}
	}
	DEBUG_REPORT_LOG (s_debugSettings, ("CuiChatHistory::%-30s [%03d] [%30s] failed\n", "loadLocation", s_debugCounter++, owner.c_str ()));
	return false;
}

//----------------------------------------------------------------------

bool CuiChatHistory::loadString   (const std::string & owner, const std::string & key, Unicode::String & value)
{
	const Data * const data = findData (owner);
	if (data)
		return data->findString (key, value);
	return false;
}

//----------------------------------------------------------------------

bool CuiChatHistory::loadData     (const std::string & owner, const std::string & key, std::string & value)
{
	const Data * const data = findData (owner);
	if (data)
		return data->findData (key, value);
	return false;
}

//----------------------------------------------------------------------

bool CuiChatHistory::loadInteger  (const std::string & owner, const std::string & key, int & value)
{
	const Data * const data = findData (owner);
	if (data)
		return data->findInteger (key, value);
	return false;
}

//----------------------------------------------------------------------

bool CuiChatHistory::loadBoolean (const std::string & owner, const std::string & key, bool & value)
{
	const Data * const data = findData (owner);
	if (data)
	{
		int ivalue = 0;
		if (data->findInteger (key, ivalue))
		{
			value = ivalue != 0;
			return true;
		}
	}
	return false;
}

//----------------------------------------------------------------------

void CuiChatHistory::clearDataForOwner(std::string const & owner)
{
	if (clearData(owner))
	{
		setDirty(true);
	}
}

//----------------------------------------------------------------------

void CuiChatHistory::resetForPlayer ()
{
	if (!s_currentFilename.empty())
	{
		s_resettingForPlayer = true;
		save();
	}
}

//----------------------------------------------------------------------

bool CuiChatHistory::isResettingForPlayer ()
{
	return s_resettingForPlayer;
}

//----------------------------------------------------------------------

void CuiChatHistory::update(float deltaTimeSecs)
{
	float const numberOfSecondsUntilCuiSave = ConfigClientUserInterface::getNumberOfSecondsUntilCuiSave();

	if (s_enableAutomaticSave && ms_dirty && (numberOfSecondsUntilCuiSave > 0.0f))
	{
		s_elapsedSecondsSinceLastSave += deltaTimeSecs;

		if (s_elapsedSecondsSinceLastSave > numberOfSecondsUntilCuiSave)
		{
			s_elapsedSecondsSinceLastSave = 0.0f;
			CuiChatHistory::save();
		}
	}
	else
	{
		s_elapsedSecondsSinceLastSave = 0.0f;
	}
}

//----------------------------------------------------------------------

void CuiChatHistory::setDirty(bool dirty)
{
	ms_dirty = dirty;
}

//----------------------------------------------------------------------

bool CuiChatHistory::isDirty()
{
	return ms_dirty;
}

//----------------------------------------------------------------------

void CuiChatHistory::setPrefixString(std::string const & prefixString)
{
	s_prefixString = prefixString;
}

//----------------------------------------------------------------------

bool CuiChatHistory::isNewPlayer()
{
	return s_isNewPlayer;
}

//----------------------------------------------------------------------

void CuiChatHistory::enableAutomaticSave(bool enable)
{
	s_enableAutomaticSave = enable;
}

//======================================================================
