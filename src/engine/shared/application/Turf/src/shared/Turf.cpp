// TerrainViewer.cpp : Defines the class behaviors for the application.
//

#include "FirstTurf.h"
#include "Turf.h"

#include "sharedCollision/SetupSharedCollision.h"
#include "sharedCollision/CollisionWorld.h"
#include "sharedCompression/SetupSharedCompression.h"
#include "sharedDebug/SetupSharedDebug.h"
#include "sharedDebug/PerformanceTimer.h"
#include "sharedFile/SetupSharedFile.h"
#include "sharedFile/TreeFile.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ApplicationVersion.h"
#include "sharedFoundation/SetupSharedFoundation.h"
#include "sharedImage/SetupSharedImage.h"
#include "sharedMath/SetupSharedMath.h"
#include "sharedObject/SetupSharedObject.h"
#include "sharedRandom/SetupSharedRandom.h"
#include "sharedTerrain/SetupSharedTerrain.h"
#include "sharedThread/SetupSharedThread.h"
#include "sharedUtility/SetupSharedUtility.h"
#include "sharedObject/CellProperty.h"
#include "sharedRandom/RandomGenerator.h"
#include "sharedRandom/FastRandomGenerator.h"

#include "sharedTerrain/TerrainObject.h"
#include "sharedTerrain/ProceduralTerrainAppearance.h"
#include "sharedTerrain/ProceduralTerrainAppearanceTemplate.h"
#include "sharedTerrain/SamplerProceduralTerrainAppearanceTemplate.h"
#include "sharedTerrain/CoordinateHash.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/ObjectNotification.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedDebug/PerformanceTimer.h"
#include "sharedImage/Image.h"
#include "sharedImage/TargaFormat.h"

#include "HeightSampler.h"

#include <assert.h>
#include <stdio.h>
#include <malloc.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma warning (disable: 4189)


//===================================================================

//===================================================================

/////////////////////////////////////////////////////////////////////////////
// TurfApp construction

TurfApp::TurfApp()
: m_samplerAppearanceTemplate(0)
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only TurfApp object

TurfApp theApp;

/////////////////////////////////////////////////////////////////////////////
// TurfApp initialization
static bool _extractPlanetName(char *o_buffer, const char *i_filename)
{
	o_buffer[0]=0;
	if (!i_filename || !*i_filename || i_filename[0]=='.')
	{
		return false;
	}

	const char *iter;

	iter=strrchr(i_filename, 0);
	while (iter>i_filename)
	{
		if (  iter[-1]=='/' 
			|| iter[-1]=='\\'
			|| iter[-1]==':'
			)
		{
			break;
		}
		iter--;
	}

	// iter should now point to beginning of planet name
	char *oiter=o_buffer;
	while (*iter && *iter!='.')
	{
		*oiter++=*iter++;
	}
	*oiter++=0;

	return true;
}

static void _sampleTest(const char *i_filename, HeightSampler::TERRAIN_CHANNEL channel)
{
	char planetname[256];

	strcpy(planetname, "terrain\\");
	if (!_extractPlanetName(strrchr(planetname, 0), i_filename))
	{
		return;
	}
	strcat(planetname, ".trn");

	HeightSampler::sample(planetname, i_filename, channel);
	REPORT_LOG_PRINT(true, ("\n***Finished sampling.\n"));
}

static void _compareTest(const char *i_original_sample_file, const char *i_new_sample_file)
{
	HeightSampler::compareSamples(i_original_sample_file, i_new_sample_file);
	REPORT_LOG_PRINT(true, ("\n***Finished comparing.\n"));
}

int main(int argc, char *argv[])
{
	int ret_val=-1;
	if (theApp.InitInstance())
	{
		theApp.run(argc, argv);
		ret_val=theApp.ExitInstance();
	}
	return ret_val;
}

bool TurfApp::InitInstance()
{
	//
	//-- install the engine
	//

	//-- setup shared
	{
		//-- thread
		SetupSharedThread::install();

		//-- debug
		SetupSharedDebug::install(4096);

		//-- foundation
		SetupSharedFoundation::Data setupFoundationData(SetupSharedFoundation::Data::D_console);
		setupFoundationData.configFile                = "client.cfg";
        setupFoundationData.writeMiniDumps            = ApplicationVersion::isBootlegBuild();
	    SetupSharedFoundation::install(setupFoundationData);

		SetupSharedCompression::install();

		//-- file
		SetupSharedFile::install(false);

		//-- math
		SetupSharedMath::install();

		//-- utility
		SetupSharedUtility::Data setupUtilityData;
		SetupSharedUtility::setupGameData (setupUtilityData);
		SetupSharedUtility::install (setupUtilityData);

		//-- random
		SetupSharedRandom::install(static_cast<uint32>(time(NULL)));

		//-- image
		SetupSharedImage::Data setupImageData;
		SetupSharedImage::setupDefaultData (setupImageData);
		SetupSharedImage::install (setupImageData);

		//-- collision
		{
			SetupSharedCollision::Data scData;

			scData.installExtents = true;
			scData.installCollisionWorld = true;
			//scData.isPlayerHouse = &isPlayerHouseHook;
			scData.serverSide = false;

			SetupSharedCollision::install(scData);
		}

		//-- object
		SetupSharedObject::Data setupObjectData;
		SetupSharedObject::setupDefaultMFCData (setupObjectData);
		SetupSharedObject::install (setupObjectData);

		//-- terrain
		SetupSharedTerrain::Data setupSharedTerrainData;
		SetupSharedTerrain::setupToolData (setupSharedTerrainData);
		SetupSharedTerrain::install (setupSharedTerrainData);
	}


	// -qq- don't like these being hard-coded
	TreeFile::addSearchAbsolute (0);

	// CG: This line inserted by 'Tip of the Day' component.
	//ShowTipAtStartup();

	m_clipRectangle.x0 = -16384.0f;
	m_clipRectangle.y0 = -16384.0f;
	m_clipRectangle.x1 =  16384.0f;
	m_clipRectangle.y1 =  16384.0f;

	return true;
}

void TurfApp::run(int argc, char *argv[])
{
	REPORT_LOG_PRINT(true, ("Turf - terrain processing utility.\n\n"));

	/*
	{
		REPORT_PRINT(true, ("%i params.\n", argc));
		for (int i=0;i<argc;i++)
		{
			REPORT_PRINT(true, ("%s\n", argv[i]));
		}
	}
	*/

	for (int i=1;i<argc;i++)
	{
		const char *arg = argv[i];
		if (*arg=='/')
		{
			arg++;
			switch (toupper(arg[0]))
			{
			case 'C':
				if (m_files.size()>1)
				{
					std::string fn1 = m_files.front();
					m_files.pop_front();
					std::string fn2 = m_files.front();
					m_files.pop_front();

					SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_IDLE);
					_compareTest(fn1.c_str(), fn2.c_str());
					SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
				}
				break;
			case 'F':
				if (!arg[1])
				{
					if (!m_files.empty())
					{
						// capital 'F' means full priority
						if (arg[0]!='F')
						{
							REPORT_PRINT(true, ("Shifting into low-priority mode.\n"));
							SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_IDLE);
						}

						////////////////////////////////////////////////////////////////

						char planetname[256];
						strcpy(planetname, "terrain\\");
						if (!_extractPlanetName(strrchr(planetname, 0), m_files.front().c_str()))
						{
							return;
						}
						strcat(planetname, ".trn");

						HeightSampler::createStaticFloraList(planetname, m_files.front().c_str(), m_clipRectangle);
						REPORT_LOG_PRINT(true, ("\n***Finished sampling.\n"));

						////////////////////////////////////////////////////////////////

						if (arg[0]!='F')
						{
							SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
						}

						m_files.pop_front();
					}
				}
				else if (toupper(arg[1])=='C')
				{
					REPORT_PRINT(true, ("Committing flora samples\n"));
					_commitFloraSamples();
				}
				else if (toupper(arg[1])=='I')
				{
					if (!m_files.empty() && m_samplerAppearanceTemplate)
					{
						REPORT_PRINT(true, ("Importing flora sample file: %s\n", m_files.front().c_str()));
						_importSampleFile(m_files.front().c_str());
						m_files.pop_front();
					}
				}
				else if (toupper(arg[1])=='W')
				{
					if (!m_files.empty() && m_samplerAppearanceTemplate)
					{
						REPORT_PRINT(true, ("Writing flora sample file: %s\n", m_files.front().c_str()));
						_writeFloraSampleFile(m_files.front().c_str());
						m_files.pop_front();
					}
				}
				break;
			case 'H':
				if (!m_files.empty())
				{
					std::string fn1 = m_files.front();
					m_files.pop_front();
					std::string fn2;
					if (!m_files.empty())
					{
						fn2 = m_files.front();
						m_files.pop_front();
					}

					char planetname[256];
					if (!_extractPlanetName(planetname, fn1.c_str()))
					{
						strcpy(planetname, "planetimage");
					}
					strcat(planetname, ".tga");

					SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_IDLE);
					HeightSampler::writeTGA(planetname, fn1.c_str(), fn2.c_str());
					SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
				}
				break;
			case 'L':
				if (!m_files.empty())
				{
					REPORT_PRINT(true, ("Loading %s\n", m_files.front().c_str()));
					_loadTemplate(m_files.front().c_str());
					m_files.pop_front();
				}
				break;
			case 'R':
				if (arg[1]=='(' || arg[1]=='"')
				{
					_processRectParam(arg+2);
				}
				else if (arg[1])
				{
					_processRectParam(arg+1);
				}
				break;
			case 'S':
				if (!m_files.empty())
				{
					HeightSampler::TERRAIN_CHANNEL channel = HeightSampler::CH_HEIGHT;
					switch (arg[1])
					{
					case '1': channel = HeightSampler::CH_COLOR; break;
					case '2': channel = HeightSampler::CH_SHADER; break;
					case '3': channel = HeightSampler::CH_FLORA_STATIC_COLLIDABLE; break;
					case '4': channel = HeightSampler::CH_FLORA_STATIC_NON_COLLIDABLE; break;
					case '5': channel = HeightSampler::CH_FLORA_DYNAMIC_NEAR; break;
					case '6': channel = HeightSampler::CH_FLORA_DYNAMIC_FAR; break;
					case '7': channel = HeightSampler::CH_ENVIRONMENT; break;
					case '8': channel = HeightSampler::CH_VERTEX_POSITION; break;
					case '9': channel = HeightSampler::CH_VERTEX_NORMAL; break;
					case '!': channel = HeightSampler::CH_EXCLUDE; break;
					case '@': channel = HeightSampler::CH_PASSABLE; break;
					default:;
					}
					// capital 'S' means full priority
					if (arg[0]!='S')
					{
						SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_IDLE);
					}
					_sampleTest(m_files.front().c_str(), channel);
					if (arg[0]!='S')
					{
						SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
					}

					m_files.pop_front();
				}
				break;
			case 'T':
				//_hash_test();
				//NoiseGenerator::test();
				break;
			case 'W':
				if (!m_files.empty())
				{
					_saveTemplate(m_files.front().c_str());
					m_files.pop_front();
				}
				break;
			}
		}
		else
		{
			m_files.push_front(std::string(arg));
		}
	}
}

bool TurfApp::_processRectParam(const char *param)
{
	if (!param || !*param)
	{
		return false;
	}

	float newXmin, newZmin, newXmax, newZmax;
	if (sscanf(param, " %f , %f , %f , %f", &newXmin, &newZmin, &newXmax, &newZmax)==4)
	{
		REPORT_PRINT(true, ("Rect: %f, %f, %f, %f\n", newXmin, newZmin, newXmax, newZmax));

		m_clipRectangle.x0 = newXmin;
		m_clipRectangle.y0 = newZmin;
		m_clipRectangle.x1 = newXmax;
		m_clipRectangle.y1 = newZmax;

		return true;
	}
	else
	{
		REPORT_PRINT(true, ("Rect failed: %s\n", param));

		return false;
	}
}

int TurfApp::ExitInstance() 
{
	_destroyTemplate();

	SetupSharedFoundation::remove();

	SetupSharedThread::remove();
	
	return 0;
}

void TurfApp::_destroyTemplate()
{
	if (m_samplerAppearanceTemplate)
	{
		delete m_samplerAppearanceTemplate;
		m_samplerAppearanceTemplate=0;
	}
}

bool TurfApp::_loadTemplate(const char *i_terrain_file)
{
	_destroyTemplate();

	Iff iff;
	if (!iff.open(i_terrain_file, true))
	{
		// ERROR
		return false;
	}

	const Tag tag = iff.getCurrentName();

	AppearanceTemplate *const appearanceTemplate = SamplerProceduralTerrainAppearanceTemplate::create(i_terrain_file, &iff);
	if (!appearanceTemplate)
	{
		return false;
	}

	m_samplerAppearanceTemplate = static_cast<SamplerProceduralTerrainAppearanceTemplate *>(appearanceTemplate);

	return true;
}

bool TurfApp::_saveTemplate(const char *i_terrain_file)
{
	if (!m_samplerAppearanceTemplate)
	{
		// ERROR
		return false;
	}

	ProceduralTerrainAppearanceTemplate::WriterData data;

	m_samplerAppearanceTemplate->prepareWriterData(data);

	Iff iff(10000);
	ProceduralTerrainAppearanceTemplate::write(iff, data);
	iff.write(i_terrain_file);

	return true;
}

bool TurfApp::_importSampleFile(const char *i_sample_file)
{
	if (!m_samplerAppearanceTemplate)
	{
		// ERROR
		return false;
	}

	return m_samplerAppearanceTemplate->loadStaticCollidableFloraFile(i_sample_file);
}

bool TurfApp::_commitFloraSamples()
{
	if (!m_samplerAppearanceTemplate)
	{
		// ERROR
		return false;
	}

	m_samplerAppearanceTemplate->commitStaticCollidableFloraSamples(0);
	return true;
}

bool TurfApp::_writeFloraSampleFile(const char *i_sample_file)
{
	if (!m_samplerAppearanceTemplate)
	{
		// ERROR
		return false;
	}

	const float floraTileWidthInMeters = m_samplerAppearanceTemplate->getFloraTileWidthInMeters();
	const int   mapWidthInFlora        = m_samplerAppearanceTemplate->getMapWidthInFlora();
	const int   floraTileCenterOffset  = mapWidthInFlora / 2;

	int startTileX = clamp(-floraTileCenterOffset, int(floor(m_clipRectangle.x0 / floraTileWidthInMeters)), floraTileCenterOffset);
	int startTileZ = clamp(-floraTileCenterOffset, int(floor(m_clipRectangle.y0 / floraTileWidthInMeters)), floraTileCenterOffset);
	int stopTileX  = clamp(-floraTileCenterOffset, int( ceil(m_clipRectangle.x1 / floraTileWidthInMeters)), floraTileCenterOffset);
	int stopTileZ  = clamp(-floraTileCenterOffset, int( ceil(m_clipRectangle.y1 / floraTileWidthInMeters)), floraTileCenterOffset);

	int tileBounds[4] = { 
		startTileX+floraTileCenterOffset, 
		startTileZ+floraTileCenterOffset, 
		 stopTileX+floraTileCenterOffset, 
		 stopTileZ+floraTileCenterOffset 
	};
	return m_samplerAppearanceTemplate->writeStaticCollidableFloraFile(i_sample_file, tileBounds);
}

//===================================================================


