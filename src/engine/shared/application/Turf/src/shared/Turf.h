// TerrainViewer.h : main header file for the TERRAINVIEWER application
//

#ifndef __TURF_H
#define __TURF_H

class ProceduralTerrainAppearance;
class TerrainObject;
class Object;

#include "sharedMath/Rectangle2d.h"

#include <set>
#include <string>
#include <list>

/////////////////////////////////////////////////////////////////////////////

class SamplerProceduralTerrainAppearanceTemplate;

class TurfApp
{
public:
	TurfApp();

	bool InitInstance();
	void run(int argc, char *argv[]);
	int ExitInstance();

	void _destroyTemplate();
	bool _loadTemplate(const char *i_terrain_file);
	bool _saveTemplate(const char *i_terrain_file);
	bool _importSampleFile(const char *i_sample_file);
	bool _commitFloraSamples();
	bool _writeFloraSampleFile(const char *i_sample_file);

	bool _processRectParam(const char *i_param);

	std::list<std::string> m_files;
	Rectangle2d            m_clipRectangle;

	SamplerProceduralTerrainAppearanceTemplate *m_samplerAppearanceTemplate;
};


/////////////////////////////////////////////////////////////////////////////

#endif
