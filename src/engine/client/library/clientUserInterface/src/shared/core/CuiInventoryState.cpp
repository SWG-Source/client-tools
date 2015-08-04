//======================================================================
//
// CuiInventoryState.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiInventoryState.h"

#include "clientGame/ClientObject.h"
#include "clientGame/Game.h"
#include "fileInterface/StdioFile.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/Os.h"
#include "sharedObject/CachedNetworkId.h"

//======================================================================

namespace
{
	namespace Tags
	{
		const Tag INVS = TAG (I,N,V,S);
	};

	typedef CuiInventoryState::ObjectWatcher         ObjectWatcher;
	typedef CuiInventoryState::CachedNetworkIdVector CachedNetworkIdVector;
	typedef CuiInventoryState::ObjectWatcherVector   ObjectWatcherVector;
	
	bool s_modified = false;
	
	class PlayerId
	{
	public:
		std::string loginId;
		std::string cluster;
		NetworkId   id;

		//----------------------------------------------------------------------

		PlayerId () :
		loginId (),
		cluster (),
		id ()
		{
		}

		//----------------------------------------------------------------------

		PlayerId (const std::string & _loginId, const std::string & _cluster, const NetworkId & _id) :
		loginId (Unicode::toLower (_loginId)),
		cluster (Unicode::toLower (_cluster)),
		id      (_id)
		{
		}

		//----------------------------------------------------------------------

		void makeSelf ()
		{
			Unicode::String dummy;

			if (Game::getPlayerPath (loginId, cluster, dummy, id))
			{
				const Object * const player = Game::getPlayer ();
				if (player)
					id = player->getNetworkId ();
			}

			loginId = Unicode::toLower (loginId);
			cluster = Unicode::toLower (cluster);
		}

		//----------------------------------------------------------------------

		bool getFilename (std::string & filename) const
		{
			if (loginId.empty () || cluster.empty () || id == NetworkId::cms_invalid)
				return false;

			static const std::string prefix = "profiles/";
			static const std::string slash = "/";
			static const std::string suffix = ".inv";

			filename = prefix + loginId + slash + cluster + slash + id.getValueString () + suffix;
			return true;
		}

		//----------------------------------------------------------------------

		const bool operator< (const PlayerId & rhs) const
		{
			if (loginId < rhs.loginId)
				return true;

			if (loginId == rhs.loginId)
			{
				if (cluster < rhs.cluster)
					return true;

				if (cluster == rhs.cluster)
					return id < rhs.id;
			}

			return false;
		}
	};

	//----------------------------------------------------------------------

	class InventoryState
	{
	public:
		bool                  modified;
		CachedNetworkIdVector objects;

		InventoryState () : 
		modified (false),
		objects  ()
		{}

		//----------------------------------------------------------------------

		void save (const PlayerId & pid)
		{
			std::string filename;
			if (!pid.getFilename (filename))
			{
				WARNING (true, ("CuiInventoryState unable to create filename for save."));
				return;
			}

			const size_t lastSlash = filename.rfind ('/');
			IGNORE_RETURN (Os::createDirectories (filename.substr (0, lastSlash).c_str ()));

			Iff iff (8196, true);
			if (!save_0000 (iff, pid))
				WARNING (true, ("Unable to create iff for %s", filename.c_str ()));
			else if (!iff.write (filename.c_str (), true))
				WARNING (true, ("Unable to write iff: %s", filename.c_str ()));
			else
				modified = false;
		}

		//----------------------------------------------------------------------

		void load (const PlayerId & pid)
		{
			std::string filename;
			if (!pid.getFilename (filename))
			{
				WARNING (true, ("CuiInventoryState unable to create filename for load."));
				return;
			}

			AbstractFile * const file = new StdioFile (filename.c_str(), "rb");
			
			if (file && file->isOpen ())
			{				
				if (!Iff::isValid(filename.c_str()))
				{
					WARNING (true, ("Data file %s is not valid.", filename.c_str ()));
					delete file;
					return;
				}
				
				Iff iff;
				iff.open (*file, filename.c_str());
				
				if (iff.enterForm (Tags::INVS, true))
				{
					switch (iff.getCurrentName ())
					{
					case TAG_0000:
						if (load_0000 (iff, pid))
							modified = false;
						break;
					default:
						WARNING (true, ("invalid CuiInventoryState"));
						break;
					}
					iff.exitForm (Tags::INVS);
				}
				
				REPORT_LOG_PRINT (true, ("loaded inventory state from %s\n", filename.c_str ()));
			}
			delete file;
		}

		//----------------------------------------------------------------------

		bool save_0000 (Iff & iff, const PlayerId & pid)
		{
			iff.insertForm (Tags::INVS);
			iff.insertForm (TAG_0000);

			iff.insertChunk (TAG_DATA);
			{
				iff.insertChunkString (pid.loginId.c_str ());
				iff.insertChunkString (pid.cluster.c_str ());

				iff.insertChunkString (pid.id.getValueString ().c_str ());
				for (CachedNetworkIdVector::const_iterator it = objects.begin (); it != objects.end (); ++it)
				{
					const CachedNetworkId & id = *it;
					iff.insertChunkString (id.getValueString ().c_str ());
				}
			}
			iff.exitChunk (TAG_DATA);
			iff.exitForm  (TAG_0000);
			iff.exitForm  (Tags::INVS);

			return true;
		}

		//----------------------------------------------------------------------

		bool load_0000 (Iff & iff, const PlayerId & pid)
		{
			bool ret = true;

			iff.enterForm (TAG_0000);
			iff.enterChunk (TAG_DATA);
			{
				const std::string & loginId = iff.read_stdstring ();
				const std::string & cluster = iff.read_stdstring ();
				const NetworkId & playerId  = NetworkId (iff.read_stdstring ());

				if (Unicode::toLower (loginId) != pid.loginId || Unicode::toLower (cluster) != pid.cluster || playerId != pid.id)
				{
					ret = false;
				}
				else
				{
					while (iff.getChunkLengthLeft () > 0)
					{
						const CachedNetworkId id (iff.read_stdstring ());
						objects.push_back (id);
					}
				}
			}
			iff.exitChunk (TAG_DATA);
			iff.exitForm  (TAG_0000);

			return ret;
		}
	};

	//----------------------------------------------------------------------

	typedef stdmap<PlayerId, InventoryState>::fwd InventoryStateMap;
	InventoryStateMap inventoryStates;

	//----------------------------------------------------------------------

	void convert (const ObjectWatcherVector & owv, CachedNetworkIdVector & cniv)
	{
		cniv.clear ();
		cniv.reserve (owv.size ());

		for (ObjectWatcherVector::const_iterator it = owv.begin (); it != owv.end (); ++it)
		{
			ClientObject * const clientObject = (*it).getPointer ();
			if (clientObject)
				cniv.push_back (CachedNetworkId (*clientObject));
			else
				WARNING (true, ("CuiInventoryState converting watcher->id vector with null pointer"));
		}
	}

	//----------------------------------------------------------------------

	void convert (const CachedNetworkIdVector & cniv, ObjectWatcherVector & owv)
	{
		owv.clear ();
		owv.reserve (cniv.size ());

		for (CachedNetworkIdVector::const_iterator it = cniv.begin (); it != cniv.end (); ++it)
		{
			const CachedNetworkId & id = *it;

			ClientObject * const clientObject = safe_cast<ClientObject *>(id.getObject ());
			if (clientObject)
				owv.push_back (ObjectWatcher (clientObject));
			else
				WARNING (true, ("CuiInventoryState converting (id obj [%s])->watcher vector with null pointer", id.getValueString ().c_str ()));
		}
	}
}


//----------------------------------------------------------------------

void CuiInventoryState::setInventoryState  (const ObjectWatcherVector & owv)
{
	PlayerId pid;
	pid.makeSelf ();

	s_modified = true;

	InventoryState & is = inventoryStates [pid];
	is.modified = true;
	convert (owv, is.objects);
}

//----------------------------------------------------------------------

void CuiInventoryState::getInventoryState  (ObjectWatcherVector & owv)
{
	PlayerId pid;
	pid.makeSelf ();

	const InventoryStateMap::iterator it = inventoryStates.find (pid);

	if (it != inventoryStates.end ())
	{
		const InventoryState & is = (*it).second;
		convert (is.objects, owv);
	}
	else
	{
		InventoryState is;
		is.load (pid);
		inventoryStates.insert (std::make_pair (pid, is));
		convert (is.objects, owv);
	}
}

//----------------------------------------------------------------------

void CuiInventoryState::saveInventoryState ()
{
	if (s_modified)
	{
		s_modified = false;
		for (InventoryStateMap::iterator it = inventoryStates.begin (); it != inventoryStates.end (); ++it)
		{
			const PlayerId & pid = (*it).first;
			InventoryState & is  = (*it).second;
			
			if (is.modified)
			{
				is.save (pid);
			}
		}
	}
}

//======================================================================
