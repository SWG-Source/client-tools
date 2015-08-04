#include "FirstUiBuilder.h"
#include "SimpleSoundCanvas.h"

#include "FileLocator.h"

#include <cstdio>

SimpleSoundCanvas::SimpleSoundCanvas() :
UISoundCanvas (),
m_soundMap ()
{

}

SimpleSoundCanvas::~SimpleSoundCanvas()
{

	for (SoundMap_t::iterator iter = m_soundMap.begin (); iter != m_soundMap.end (); ++iter)
	{
		delete (*iter).second;
		(*iter).second = 0;
	}

	m_soundMap.clear ();

}

void SimpleSoundCanvas::Play( const char *name )
{

	assert (name);

	SoundMap_t::const_iterator find_iter = m_soundMap.find (name);

	if (find_iter == m_soundMap.end ())
	{
		UINarrowString filename (name);
		filename += ".wav";

		UINarrowString result;
		
		if ( FileLocator::gFileLocator ().findFile (filename.c_str (), result) == false)
			return;
		
		FILE * fl = fopen (result.c_str (), "rb");
		
		if (fl == 0)
			return;
		
		fseek (fl, 0, SEEK_END);
		long len = ftell (fl);
		fseek (fl, 0, SEEK_SET);
		
		char * buf = new char [len];
		
		if (!fread (buf, len, 1, fl))
		{
			delete[] buf;
			return;
		}

		fclose (fl);

		std::pair<SoundMap_t::const_iterator, bool> retval = m_soundMap.insert (SoundMap_t::value_type (name, buf));

		find_iter = retval.first;
	}

	PlaySound ((*find_iter).second, NULL, SND_MEMORY | SND_ASYNC | SND_NOWAIT | SND_NOSTOP);
}


