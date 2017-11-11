#include "FirstTurf.h"
#include "HeightSampler.h"

#include "sharedTerrain/SamplerProceduralTerrainAppearance.h"
#include "sharedTerrain/SamplerProceduralTerrainAppearanceTemplate.h"

#include "sharedTerrain/TerrainObject.h"
#include "sharedTerrain/ProceduralTerrainAppearance.h"
#include "sharedTerrain/ProceduralTerrainAppearanceTemplate.h"

#include "sharedObject/Appearance.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/AppearanceTemplateList.h"

#include "sharedImage/Image.h"
#include "sharedImage/TargaFormat.h"

#include "sharedFile/Iff.h"

#include <malloc.h>
#include <math.h>
#include <io.h>
#include <map>

#define STARTX 0
#define STARTZ 0
#define ZSTEP 1
#define XSTEP 1

#define MAX_HEIGHT_ERROR (1)

namespace HeightSamplerNamespace
{
	struct TerrainSampleFileHeader
	{
		bool operator==(const TerrainSampleFileHeader &o) const
		{
			return
				mapWidthInMeters==o.mapWidthInMeters
			&& numberOfPoles==o.numberOfPoles
			&& tileWidthInMeters==o.tileWidthInMeters
			&& startX==o.startX
			&& startZ==o.startZ
			&& xStep==o.xStep
			&& zStep==o.zStep
			;
		}

		int     mapWidthInMeters;
		uint16  numberOfPoles;
		uint16  channel;
		float tileWidthInMeters;

		int startX;
		int startZ;
		int xStep;
		int zStep;

		float minHeight;
		float maxHeight;
		float avgHeight;
	};

	int getElementSize(HeightSampler::TERRAIN_CHANNEL c)
	{
		switch (c)
		{
		case HeightSampler::CH_HEIGHT: return 4;
		case HeightSampler::CH_COLOR: return 4;
		case HeightSampler::CH_SHADER: return sizeof(ShaderGroup::Info);
		case HeightSampler::CH_FLORA_STATIC_COLLIDABLE: return sizeof(FloraGroup::Info);
		case HeightSampler::CH_FLORA_STATIC_NON_COLLIDABLE: return sizeof(FloraGroup::Info);
		case HeightSampler::CH_FLORA_DYNAMIC_NEAR: return sizeof(RadialGroup::Info); 
		case HeightSampler::CH_FLORA_DYNAMIC_FAR: return sizeof(RadialGroup::Info); 
		case HeightSampler::CH_ENVIRONMENT: return sizeof(EnvironmentGroup::Info);
		case HeightSampler::CH_VERTEX_POSITION: return sizeof(Vector);
		case HeightSampler::CH_VERTEX_NORMAL: return sizeof(Vector);
		case HeightSampler::CH_EXCLUDE: return 1;
		case HeightSampler::CH_PASSABLE: return 1;
		default:
			return 4;
		}
	}

	bool debug_chunk=false;

	float bias;
	float scale;
}

using namespace HeightSamplerNamespace;

#pragma warning(disable:4189)
#pragma warning (disable: 4244)

bool HeightSampler::sample(const char *i_terrain_file, const char *i_output_file, TERRAIN_CHANNEL i_channel)
{
	FILE *of = fopen(i_output_file, "wb");
	if (!of)
	{
		return false;
	}

	Iff iff;
	if (!iff.open(i_terrain_file, true))
	{
		FATAL(true, ("Could not open appearance file %s", i_terrain_file));
	}

	const Tag tag = iff.getCurrentName();

	AppearanceTemplate *const appearanceTemplate = SamplerProceduralTerrainAppearanceTemplate::create(i_terrain_file, &iff);
	if (!appearanceTemplate)
	{
		return false;
	}

	SamplerProceduralTerrainAppearanceTemplate *const samplerAppearanceTemplate = static_cast<SamplerProceduralTerrainAppearanceTemplate *>(appearanceTemplate);

	samplerAppearanceTemplate->setMapsToSample(TGM_height | TGM_vertexPosition | TGM_vertexNormal | TGM_floraStaticCollidable);

	Appearance * const appearance = appearanceTemplate->createAppearance();
	if (!appearance)
	{
		return false;
	}

	SamplerProceduralTerrainAppearance *const samplerAppearance = dynamic_cast<SamplerProceduralTerrainAppearance *>(appearance);

	const ProceduralTerrainAppearanceTemplate *proceduralTerrainAppearanceTemplate = static_cast<const ProceduralTerrainAppearanceTemplate *>(appearanceTemplate);
	const TerrainGenerator* const terrainGenerator = proceduralTerrainAppearanceTemplate->getTerrainGenerator ();

	const int originOffset=0;
	const int upperPad=0;
	const int numberOfTilesPerChunk = 16;

	const int numberOfPoles        = numberOfTilesPerChunk * 2;
	const int actualNumberOfPoles  = numberOfPoles + originOffset + upperPad;

	const int   mapWidthInMeters     = proceduralTerrainAppearanceTemplate->getMapWidthInMeters();
	const float tileWidthInMeters    = proceduralTerrainAppearanceTemplate->getTileWidthInMeters();
	const float distanceBetweenPoles = tileWidthInMeters * 0.5f;
	const float chunkWidthInMeters   = distanceBetweenPoles * float(numberOfPoles);
	const int   mapWidthInChunks     = mapWidthInMeters / chunkWidthInMeters;

	//---------------------------------------------------------
	TerrainSampleFileHeader fileHeader;
	memset(&fileHeader, 0, sizeof(fileHeader));

	fileHeader.mapWidthInMeters = mapWidthInMeters;
	fileHeader.numberOfPoles = static_cast<uint16>(numberOfPoles);
	fileHeader.channel = i_channel;
	fileHeader.tileWidthInMeters = tileWidthInMeters;

	fileHeader.startX = STARTX;
	fileHeader.startZ = STARTZ;
	fileHeader.xStep = XSTEP;
	fileHeader.zStep = ZSTEP;

	fileHeader.minHeight = FLT_MAX;
	fileHeader.maxHeight = -FLT_MAX;
	fileHeader.avgHeight=0;

	fwrite(&fileHeader, sizeof(fileHeader), 1, of);
	//---------------------------------------------------------

	//---------------------------------------------------------
	unsigned mapToSample;

	switch (i_channel)
	{
	case CH_HEIGHT:
	{
		mapToSample = TGM_height;
	} break;
	case CH_COLOR:
	{
		mapToSample = TGM_color;
	} break;
	case CH_SHADER:
	{
		mapToSample = TGM_shader;
	} break;
	case CH_FLORA_STATIC_COLLIDABLE:
	{
		mapToSample = TGM_floraStaticCollidable;
	} break;
	case CH_FLORA_STATIC_NON_COLLIDABLE:
	{
		mapToSample = TGM_floraStaticNonCollidable;
	} break;
	case CH_FLORA_DYNAMIC_NEAR:
	{
		mapToSample = TGM_floraDynamicNear;
	} break;
	case CH_FLORA_DYNAMIC_FAR:
	{
		mapToSample = TGM_floraDynamicFar;
	} break;
	case CH_ENVIRONMENT:
	{
		mapToSample = TGM_environment;
	} break;
	case CH_VERTEX_POSITION:
	{
		mapToSample = TGM_vertexPosition;
	} break;
	case CH_VERTEX_NORMAL:
	{
		mapToSample = TGM_vertexNormal;
	} break;
	case CH_EXCLUDE:
	{
		mapToSample = TGM_exclude;
	} break;
	case CH_PASSABLE:
	{
		mapToSample = TGM_passable;
	} break;

	default:
		mapToSample=unsigned(TGM_ALL);
	}
	samplerAppearanceTemplate->setMapsToSample(mapToSample);
	//---------------------------------------------------------

	//-- ask the generator to fill out this area
	TerrainGenerator::CreateChunkBuffer createChunkBuffer;
	TerrainGenerator::GeneratorChunkData generatorChunkData(proceduralTerrainAppearanceTemplate->getLegacyMode());

	createChunkBuffer.allocate(actualNumberOfPoles);

	generatorChunkData.shaderGroup          = &terrainGenerator->getShaderGroup();
	generatorChunkData.floraGroup           = &terrainGenerator->getFloraGroup ();
	generatorChunkData.radialGroup          = &terrainGenerator->getRadialGroup ();
	generatorChunkData.environmentGroup     = &terrainGenerator->getEnvironmentGroup ();
	generatorChunkData.fractalGroup         = &terrainGenerator->getFractalGroup ();
	generatorChunkData.bitmapGroup          = &terrainGenerator->getBitmapGroup ();
	generatorChunkData.heightMap                   = &createChunkBuffer.heightMap;
	generatorChunkData.colorMap                    = &createChunkBuffer.colorMap;
	generatorChunkData.shaderMap                   = &createChunkBuffer.shaderMap;
	generatorChunkData.floraStaticCollidableMap    = &createChunkBuffer.floraStaticCollidableMap;
	generatorChunkData.floraStaticNonCollidableMap = &createChunkBuffer.floraStaticNonCollidableMap;
	generatorChunkData.floraDynamicNearMap         = &createChunkBuffer.floraDynamicNearMap;
	generatorChunkData.floraDynamicFarMap          = &createChunkBuffer.floraDynamicFarMap;
	generatorChunkData.environmentMap              = &createChunkBuffer.environmentMap;
	generatorChunkData.vertexPositionMap           = &createChunkBuffer.vertexPositionMap;
	generatorChunkData.vertexNormalMap             = &createChunkBuffer.vertexNormalMap;
	generatorChunkData.excludeMap                  = &createChunkBuffer.excludeMap;
	generatorChunkData.passableMap                 = &createChunkBuffer.passableMap;
	generatorChunkData.originOffset                = originOffset;
	generatorChunkData.upperPad                    = upperPad;
	generatorChunkData.numberOfPoles               = actualNumberOfPoles;
	generatorChunkData.distanceBetweenPoles        = distanceBetweenPoles;

	REPORT_LOG_PRINT(true, ("\n\n*** Sampling %s to %s...\n\n", i_terrain_file, i_output_file));

	const int numberOfElements = numberOfPoles*numberOfPoles;
	const int actualNumberOfElements = actualNumberOfPoles*actualNumberOfPoles;

	double totalHeight=0;
	unsigned numberOfSamples=0;

	Vector start(0,0,0);
	for (int z=STARTZ;z<mapWidthInChunks;z+=ZSTEP)
	{
		REPORT_LOG_PRINT(true, ("%i/%i  ", z, mapWidthInChunks));
		start.z = float(z)*chunkWidthInMeters - mapWidthInMeters/2 - static_cast<float> (originOffset) * distanceBetweenPoles;
		for (int x=STARTX;x<mapWidthInChunks;x+=XSTEP)
		{
			start.x = float(x)*chunkWidthInMeters - mapWidthInMeters/2 - static_cast<float> (originOffset) * distanceBetweenPoles;

			debug_chunk=(z==STARTZ && x==STARTX);

			generatorChunkData.start                = start;
			terrainGenerator->generateChunk (generatorChunkData);
			numberOfSamples += numberOfElements;

			switch (i_channel)
			{
			case CH_HEIGHT:
			{
				const float *heightData = generatorChunkData.heightMap->getData();
				for (int i=0;i<numberOfPoles;i++)
				{
					const float *row = heightData + (i + originOffset)*actualNumberOfPoles + originOffset;
					for (int j=0;j<numberOfPoles;j++)
					{
						float h = heightData[i];
						totalHeight+=h;
						if (h>fileHeader.maxHeight)
						{
							fileHeader.maxHeight=h;
						}
						if (h<fileHeader.minHeight)
						{
							fileHeader.minHeight=h;
						}
					}
					fwrite(row, numberOfPoles*4, 1, of);
				}
			} break;
			case CH_COLOR:
			{
				const PackedRgb *data = generatorChunkData.colorMap->getData();
				for (int i=0;i<numberOfPoles;i++)
				{
					const PackedRgb *row = data + (i + originOffset)*actualNumberOfPoles + originOffset;
					fwrite(row, numberOfPoles*sizeof(*row), 1, of);
				}
			} break;
			case CH_SHADER:
			{
				const ShaderGroup::Info *data = generatorChunkData.shaderMap->getData();
				for (int i=0;i<numberOfPoles;i++)
				{
					const ShaderGroup::Info *row = data + (i + originOffset)*actualNumberOfPoles + originOffset;
					fwrite(row, numberOfPoles*sizeof(*row), 1, of);
				}
			} break;
			case CH_FLORA_STATIC_COLLIDABLE:
			{
				const FloraGroup::Info *data = generatorChunkData.floraStaticCollidableMap->getData();
				for (int i=0;i<numberOfPoles;i++)
				{
					const FloraGroup::Info *row = data + (i + originOffset)*actualNumberOfPoles + originOffset;
					fwrite(row, numberOfPoles*sizeof(*row), 1, of);
				}
			} break;
			case CH_FLORA_STATIC_NON_COLLIDABLE:
			{
				const FloraGroup::Info *data = generatorChunkData.floraStaticNonCollidableMap->getData();
				for (int i=0;i<numberOfPoles;i++)
				{
					const FloraGroup::Info *row = data + (i + originOffset)*actualNumberOfPoles + originOffset;
					fwrite(row, numberOfPoles*sizeof(*row), 1, of);
				}
			} break;
			case CH_FLORA_DYNAMIC_NEAR:
			{
				const RadialGroup::Info *data = generatorChunkData.floraDynamicNearMap->getData();
				for (int i=0;i<numberOfPoles;i++)
				{
					const RadialGroup::Info *row = data + (i + originOffset)*actualNumberOfPoles + originOffset;
					fwrite(row, numberOfPoles*sizeof(*row), 1, of);
				}
			} break;
			case CH_FLORA_DYNAMIC_FAR:
			{
				const RadialGroup::Info *data = generatorChunkData.floraDynamicFarMap->getData();
				for (int i=0;i<numberOfPoles;i++)
				{
					const RadialGroup::Info *row = data + (i + originOffset)*actualNumberOfPoles + originOffset;
					fwrite(row, numberOfPoles*sizeof(*row), 1, of);
				}
			} break;
			case CH_ENVIRONMENT:
			{
				const EnvironmentGroup::Info *data = generatorChunkData.environmentMap->getData();
				for (int i=0;i<numberOfPoles;i++)
				{
					const EnvironmentGroup::Info *row = data + (i + originOffset)*actualNumberOfPoles + originOffset;
					fwrite(row, numberOfPoles*sizeof(*row), 1, of);
				}
			} break;
			case CH_VERTEX_POSITION:
			{
				const Vector *data = generatorChunkData.vertexPositionMap->getData();
				for (int i=0;i<numberOfPoles;i++)
				{
					const Vector *row = data + (i + originOffset)*actualNumberOfPoles + originOffset;
					fwrite(row, numberOfPoles*sizeof(*row), 1, of);
				}
			} break;
			case CH_VERTEX_NORMAL:
			{
				const Vector *data = generatorChunkData.vertexNormalMap->getData();
				for (int i=0;i<numberOfPoles;i++)
				{
					const Vector *row = data + (i + originOffset)*actualNumberOfPoles + originOffset;
					fwrite(row, numberOfPoles*sizeof(*row), 1, of);
				}
			} break;
			case CH_EXCLUDE:
			{
				const bool *data = generatorChunkData.excludeMap->getData();
				for (int i=0;i<numberOfPoles;i++)
				{
					const bool *row = data + (i + originOffset)*actualNumberOfPoles + originOffset;
					fwrite(row, numberOfPoles*sizeof(*row), 1, of);
				}
			} break;
			case CH_PASSABLE:
			{
				const bool *data = generatorChunkData.passableMap->getData();
				for (int i=0;i<numberOfPoles;i++)
				{
					const bool *row = data + (i + originOffset)*actualNumberOfPoles + originOffset;
					fwrite(row, numberOfPoles*sizeof(*row), 1, of);
				}
			} break;
			}
		}
	}

	if (numberOfSamples && i_channel==CH_HEIGHT)
	{
		fileHeader.avgHeight = static_cast<float>(totalHeight / double(numberOfSamples));
		REPORT_LOG_PRINT(true, ("\nMin/Max/Avg: %g/%g/%g\n", double(fileHeader.minHeight), double(fileHeader.maxHeight), double(fileHeader.avgHeight)));
		fseek(of, 0, SEEK_SET);
		fwrite(&fileHeader, sizeof(fileHeader), 1, of);
	}

	fclose(of);
	REPORT_LOG_PRINT(true, ("\n\n"));
	delete appearance;
	return true;
}

bool HeightSampler::compareSamples(const char *i_original_sample_file, const char *i_new_sample_file)
{
	REPORT_LOG_PRINT(true, ("\n\n*** Comparing %s to %s...\n", i_new_sample_file, i_original_sample_file));

	FILE *of = fopen(i_original_sample_file, "rb");
	if (!of)
	{
		return false;
	}

	FILE *nf = fopen(i_new_sample_file, "rb");
	if (!nf)
	{
		fclose(of);
		return false;
	}

	bool ret_val=false;

	// read and compare headers
	TerrainSampleFileHeader fileHeader1, fileHeader2;
	memset(&fileHeader1, 0, sizeof(TerrainSampleFileHeader));
	memset(&fileHeader2, 0, sizeof(TerrainSampleFileHeader));
	fread(&fileHeader1, sizeof(TerrainSampleFileHeader), 1, of);
	fread(&fileHeader2, sizeof(TerrainSampleFileHeader), 1, nf);

	if (fileHeader1==fileHeader2)
	{
		const int   mapWidthInMeters     = fileHeader1.mapWidthInMeters;
		const int   numberOfPoles        = fileHeader1.numberOfPoles;
		const float tileWidthInMeters    = fileHeader1.tileWidthInMeters;
		const float distanceBetweenPoles = tileWidthInMeters * 0.5f;
		const float chunkWidthInMeters   = distanceBetweenPoles * float(numberOfPoles);
		const int   mapWidthInChunks     = mapWidthInMeters / chunkWidthInMeters;

		const int numberOfElements = numberOfPoles*numberOfPoles;

		float *orig_heights = (float *)_alloca(numberOfElements * 4);
		float *new_heights  = (float *)_alloca(numberOfElements * 4);

		Vector start(0,0,0);
		for (int z=fileHeader1.startZ;z<mapWidthInChunks;z+=fileHeader1.zStep)
		{
			REPORT_LOG_PRINT(true, ("%i/%i  ", z, mapWidthInChunks));
			start.z = float(z)*chunkWidthInMeters - mapWidthInMeters/2;

			double row_max_diff=0;

			for (int x=fileHeader1.startX;x<mapWidthInChunks;x+=fileHeader1.xStep)
			{
				start.x = float(x)*chunkWidthInMeters - mapWidthInMeters/2;

				fread(orig_heights, numberOfElements*4, 1, of);
				fread(new_heights, numberOfElements*4, 1, nf);

				double chunk_max_diff=0;
				int chunk_max_j=-1;
				int chunk_max_i=-1;
				double chunk_max_o=0, chunk_max_n=0;

				for (int j=0;j<numberOfPoles;j++)
				{
					for (int i=0;i<numberOfPoles;i++)
					{
						float oh = orig_heights[j*numberOfPoles + i];
						float nh = new_heights[j*numberOfPoles + i];

						double diff = fabs(oh - nh);
						if (diff>chunk_max_diff)
						{
							chunk_max_o=oh;
							chunk_max_n=nh;
							chunk_max_j=j;
							chunk_max_i=i;
							chunk_max_diff=diff;
						}
					}
				}

				//REPORT_LOG_PRINT(true, ("%g\n", chunk_max_diff));

				if (chunk_max_diff>row_max_diff)
				{
					row_max_diff=chunk_max_diff;
				}

				if (chunk_max_diff>MAX_HEIGHT_ERROR)
				{
					REPORT_LOG_PRINT(true, ("***Error: Overly large diff detected (%g):\n", chunk_max_diff));
					REPORT_LOG_PRINT(true, ("Chunk: (%i, %i) (%g, %g)\n", x, z, double(start.x), double(start.z)));
					REPORT_LOG_PRINT(true, ("Sample: (%i, %i) %g -> %g\n", chunk_max_i, chunk_max_j, chunk_max_o, chunk_max_n));
					break;
				}
			}

			if (row_max_diff>MAX_HEIGHT_ERROR)
			{
				break;
			}

			REPORT_LOG_PRINT(true, ("%g\n", row_max_diff));
		}

		ret_val=true;
	}
	else
	{
		REPORT_LOG_PRINT(true, ("File types incompatible.\n"));
	}

	REPORT_LOG_PRINT(true, ("\n\n"));

	fclose(of);
	fclose(nf);
	return ret_val;
}

// --------------------------------------------------------------------

void _diffChunks(
	float *diffChunk,
	const float *chunk1,
	const float *chunk2,
	const int numberOfPoles
	)
{
	const int numberOfElements = numberOfPoles*numberOfPoles;
	for (int i=0;i<numberOfElements;i++)
	{
		diffChunk[i] = chunk1[i]-chunk2[i];
	}
}

// --------------------------------------------------------------------

template <class T>
void _t_diffChunks(
	bool *diffChunk,
	const T *chunk1,
	const T *chunk2,
	const int numberOfPoles
	)
{
	const int numberOfElements = numberOfPoles*numberOfPoles;
	for (int i=0;i<numberOfElements;i++)
	{
		diffChunk[i] = memcmp(chunk1+i, chunk2+i, sizeof(T))!=0;
	}
}


// --------------------------------------------------------------------

void _processDiffChunk(
	const float *diffChunk,
	int numberOfPoles,
	uint8 *pixelChunk,
	int imageWidthInBytes,
	int chunkWidthInPixels,
	int pixelWidthInSamples
)
{
	for (int z = 0;z<chunkWidthInPixels;z++)
	{
		uint8 *pixelRow = pixelChunk + z*imageWidthInBytes;
		const float *chunkRow = diffChunk + z*pixelWidthInSamples*numberOfPoles;

		for (int x = 0;x<chunkWidthInPixels;x++)
		{
			uint8 *pixel = pixelRow + x*3;
			const float *pixelChunk = chunkRow + x*pixelWidthInSamples;

			float maxDiff=0;
			float minDiff=0;

			for (int i=0;i<pixelWidthInSamples;i++)
			{
				for (int j=0;j<pixelWidthInSamples;j++)
				{
					const float diff = pixelChunk[i*numberOfPoles + j];
					if (diff>maxDiff)
					{
						maxDiff=diff;
					}
					else if (diff<minDiff)
					{
						minDiff=diff;
					}
				}
			}

			if (maxDiff<0) maxDiff=-maxDiff;
			if (minDiff<0) minDiff=-minDiff;

			if (minDiff>maxDiff) maxDiff=minDiff;

			maxDiff *= 100.0f;

			if (maxDiff>255.0f)
			{
				maxDiff=255.0f;
			}
			if (maxDiff<0.0f)
			{
				maxDiff=0.0f;
			}
			uint8 b = static_cast<uint8>(maxDiff);

			/*
			const float power = 1.0f;
			if (maxDiff>=0)
			{
				if (maxDiff>1.0f)
				{
					maxDiff=1.0f;
				}

				maxDiff = pow(maxDiff, power);
			}
			else
			{
				if (maxDiff<-1.0f)
				{
					maxDiff=-1.0f;
				}
				maxDiff = -pow(-maxDiff, power);
			}

			uint8 b = static_cast<uint8>(128.0f + maxDiff*127.0f);
			*/

			pixel[0]=pixel[1]=pixel[2]=b;
		}
	}
}

// --------------------------------------------------------------------

void _processDiffChunk(
	const bool *diffChunk,
	int numberOfPoles,
	uint8 *pixelChunk,
	int imageWidthInBytes,
	int chunkWidthInPixels,
	int pixelWidthInSamples
)
{
	for (int z = 0;z<chunkWidthInPixels;z++)
	{
		uint8 *pixelRow = pixelChunk + z*imageWidthInBytes;
		const bool *chunkRow = diffChunk + z*pixelWidthInSamples*numberOfPoles;

		for (int x = 0;x<chunkWidthInPixels;x++)
		{
			uint8 *pixel = pixelRow + x*3;
			const bool *pixelChunk = chunkRow + x*pixelWidthInSamples;

			bool isDifferent=false;

			float maxDiff=0;
			float minDiff=0;

			for (int i=0;i<pixelWidthInSamples;i++)
			{
				for (int j=0;j<pixelWidthInSamples;j++)
				{
					if (pixelChunk[i*numberOfPoles + j])
					{
						isDifferent=true;
						break;
					}
				}
			}


			uint8 b = isDifferent ? 255 : 0;

			pixel[0]=pixel[1]=pixel[2]=b;
		}
	}
}

// --------------------------------------------------------------------

void _processHeightChunk(
	const float *heightChunk,
	int numberOfPoles,
	uint8 *pixelChunk,
	int imageWidthInBytes,
	int chunkWidthInPixels,
	int pixelWidthInSamples
)
{
	for (int z = 0;z<chunkWidthInPixels;z++)
	{
		uint8 *pixelRow = pixelChunk + z*imageWidthInBytes;
		const float *chunkRow = heightChunk + z*pixelWidthInSamples*numberOfPoles;

		for (int x = 0;x<chunkWidthInPixels;x++)
		{
			uint8 *pixel = pixelRow + x*3;
			const float *pixelChunk = chunkRow + x*pixelWidthInSamples;

			float maxHeight=-1000000;
			float minHeight=1000000;

			for (int i=0;i<pixelWidthInSamples;i++)
			{
				for (int j=0;j<pixelWidthInSamples;j++)
				{
					const float height = pixelChunk[i*numberOfPoles + j];
					if (height>maxHeight)
					{
						maxHeight=height;
					}

					if (height<minHeight)
					{
						minHeight=height;
					}
				}
			}

			const float power = 0.6f;

			maxHeight = (maxHeight + bias) * scale;

			if (maxHeight>=0)
			{
				if (maxHeight>1.0f)
				{
					maxHeight=1.0f;
				}

				maxHeight = pow(maxHeight, power);
			}
			else
			{
				if (maxHeight<-1.0f)
				{
					maxHeight=-1.0f;
				}

				maxHeight = -pow(-maxHeight, power);
			}

			uint8 b = static_cast<uint8>(128.0f + maxHeight*127.0f);

			pixel[0]=pixel[1]=pixel[2]=b;
		}
	}
}

// --------------------------------------------------------------------

void _processFloraChunk(
	const FloraGroup::Info *chunk,
	int numberOfPoles,
	uint8 *pixelChunk,
	int imageWidthInBytes,
	int chunkWidthInPixels,
	int pixelWidthInSamples
)
{
	for (int z = 0;z<chunkWidthInPixels;z++)
	{
		uint8 *pixelRow = pixelChunk + z*imageWidthInBytes;
		const FloraGroup::Info *chunkRow = chunk + z*pixelWidthInSamples*numberOfPoles;

		for (int x = 0;x<chunkWidthInPixels;x++)
		{
			uint8 *pixel = pixelRow + x*3;
			const FloraGroup::Info *pixelChunk = chunkRow + x*pixelWidthInSamples;

			FloraGroup::Info flora = pixelChunk[0];

			pixel[0]=static_cast<uint8>(flora.getFamilyId());
			pixel[1]=static_cast<uint8>(flora.getChildChoice());
			pixel[2]=0;
		}
	}
}

// --------------------------------------------------------------------

template <class T> void _defaultDiffer(
	bool *diffChunk, 
	const T *chunk1, 
	const T *chunk2, 
	int numberOfPoles,
	uint8 *pixelChunk,
	int imageWidthInBytes,
	int chunkWidthInPixels,
	int pixelWidthInSamples
)
{
	_t_diffChunks(diffChunk, chunk1, chunk2, numberOfPoles);
	_processDiffChunk(diffChunk, numberOfPoles, pixelChunk, imageWidthInBytes, chunkWidthInPixels, pixelWidthInSamples);
}

// --------------------------------------------------------------------

bool _writeTGA(
	const char *o_tgaFile,
	int ulChunkX,
	int ulChunkZ,
	int imageWidth, 
	int metersPerPixel, 
	FILE *f1, 
	FILE *f2
	)
{
	//////////////////////////////////////////////////////////////////////////////
	Image *image = new Image();
	image->setPixelInformation(0x00ff0000,0x0000ff00,0x000000ff,0x00000000);
	image->setDimensions(imageWidth, imageWidth, 24, 3);
	uint8* imageData = image->lock();
	if (!imageData)
	{
		delete image;
		return false;
	}
	//////////////////////////////////////////////////////////////////////////////

	fseek(f1, 0, SEEK_SET);
	if (f2)
	{
		fseek(f2, 0, SEEK_SET);
	}

	TerrainSampleFileHeader fileHeader1;
	memset(&fileHeader1, 0, sizeof(TerrainSampleFileHeader));
	fread(&fileHeader1, sizeof(TerrainSampleFileHeader), 1, f1);
	const int filePixelOffset = ftell(f1);
	if (f2)
	{
		fseek(f2, filePixelOffset, SEEK_SET);
	}

	const int channel = fileHeader1.channel!=0 ? fileHeader1.channel : HeightSampler::CH_HEIGHT;

	const int   mapWidthInMeters     = fileHeader1.mapWidthInMeters;
	const int   numberOfPoles        = fileHeader1.numberOfPoles;
	const float tileWidthInMeters    = fileHeader1.tileWidthInMeters;
	const float distanceBetweenPoles = tileWidthInMeters * 0.5f;
	const float chunkWidthInMeters   = distanceBetweenPoles * float(numberOfPoles);
	const int   mapWidthInChunks     = mapWidthInMeters / chunkWidthInMeters;
	const int numberOfElements = numberOfPoles*numberOfPoles;

	const int imageWidthInMeters = imageWidth * metersPerPixel;
	const int imageWidthInChunks = imageWidthInMeters / chunkWidthInMeters;
	const int imageWidthInBytes = imageWidth*3;

	const int elementSize=getElementSize(HeightSampler::TERRAIN_CHANNEL(fileHeader1.channel));
	const int chunkSizeInBytes = numberOfElements * elementSize;

	const int chunkWidthInPixels = imageWidth / imageWidthInChunks;
	const int pixelWidthInSamples = numberOfPoles / chunkWidthInPixels;

	void *heights1 = (void *)_alloca(numberOfElements * elementSize);
	void *heights2 = (f2) ? (void *)_alloca(numberOfElements * elementSize) : (float *)0;

	const int elementDiffSize = (channel==HeightSampler::CH_HEIGHT) ? 4 : 1;
	void *diffChunk = (f2) ? (void *)_alloca(numberOfElements * elementDiffSize) : (float *)0;

	bias = -fileHeader1.avgHeight;
	const float max1 = fabs(fileHeader1.maxHeight-fileHeader1.avgHeight);
	const float max2 = fabs(fileHeader1.avgHeight-fileHeader1.minHeight);
	scale = 1.0f / ((max1>max2) ? max1 : max2);

	for (int z = ulChunkZ;z<ulChunkZ+imageWidthInChunks;z++)
	{
		const int fileRowOffset = z * mapWidthInChunks * chunkSizeInBytes + filePixelOffset;
		const int imageZ = (z - ulChunkZ) * chunkWidthInPixels;
		uint8 *const pixelRow = imageData + imageZ*imageWidthInBytes;

		for (int x = ulChunkX;x<ulChunkX+imageWidthInChunks;x++)
		{
			const int fileOffset = fileRowOffset + x*chunkSizeInBytes;

			fseek(f1, fileOffset, SEEK_SET);
			if (f2)
			{
				fseek(f2, fileOffset, SEEK_SET);
			}

			fread(heights1, chunkSizeInBytes, 1, f1);
			if (heights2)
			{
				fread(heights2, chunkSizeInBytes, 1, f2);
			}

			const int imageX = (x - ulChunkX) * chunkWidthInPixels;
			uint8 *pixelChunk = pixelRow + 3*imageX;

			//--------------------------------------------
			if (diffChunk)
			{
				switch (channel)
				{
				case HeightSampler::CH_HEIGHT: 
				{
					_diffChunks((float *)diffChunk, (float *)heights1, (float *)heights2, numberOfPoles);
					_processDiffChunk((float *)diffChunk, numberOfPoles, pixelChunk, imageWidthInBytes, chunkWidthInPixels, pixelWidthInSamples);
				} break;
				case HeightSampler::CH_COLOR:
				{
					_defaultDiffer((bool *)diffChunk, (PackedRgb *)heights1, (PackedRgb *)heights2, numberOfPoles, pixelChunk, imageWidthInBytes, chunkWidthInPixels, pixelWidthInSamples);
				} break;
				case HeightSampler::CH_SHADER:
				{
					_defaultDiffer((bool *)diffChunk, (ShaderGroup::Info *)heights1, (ShaderGroup::Info *)heights2, numberOfPoles, pixelChunk, imageWidthInBytes, chunkWidthInPixels, pixelWidthInSamples);
				} break;
				case HeightSampler::CH_FLORA_STATIC_COLLIDABLE:
				case HeightSampler::CH_FLORA_STATIC_NON_COLLIDABLE:
				{
					_defaultDiffer((bool *)diffChunk, (FloraGroup::Info *)heights1, (FloraGroup::Info *)heights2, numberOfPoles, pixelChunk, imageWidthInBytes, chunkWidthInPixels, pixelWidthInSamples);
				} break;
				case HeightSampler::CH_FLORA_DYNAMIC_NEAR:
				case HeightSampler::CH_FLORA_DYNAMIC_FAR:
				{
					_defaultDiffer((bool *)diffChunk, (RadialGroup::Info *)heights1, (RadialGroup::Info *)heights2, numberOfPoles, pixelChunk, imageWidthInBytes, chunkWidthInPixels, pixelWidthInSamples);
				} break;
				case HeightSampler::CH_ENVIRONMENT:
				{
					_defaultDiffer((bool *)diffChunk, (EnvironmentGroup::Info *)heights1, (EnvironmentGroup::Info *)heights2, numberOfPoles, pixelChunk, imageWidthInBytes, chunkWidthInPixels, pixelWidthInSamples);
				} break;
				case HeightSampler::CH_VERTEX_POSITION:
				{
					_defaultDiffer((bool *)diffChunk, (Vector *)heights1, (Vector *)heights2, numberOfPoles, pixelChunk, imageWidthInBytes, chunkWidthInPixels, pixelWidthInSamples);
				} break;
				case HeightSampler::CH_VERTEX_NORMAL:
				{
					_defaultDiffer((bool *)diffChunk, (Vector *)heights1, (Vector *)heights2, numberOfPoles, pixelChunk, imageWidthInBytes, chunkWidthInPixels, pixelWidthInSamples);
				} break;
				case HeightSampler::CH_EXCLUDE:
				{
					_defaultDiffer((bool *)diffChunk, (bool *)heights1, (bool *)heights2, numberOfPoles, pixelChunk, imageWidthInBytes, chunkWidthInPixels, pixelWidthInSamples);
				} break;
				case HeightSampler::CH_PASSABLE:
				{
					_defaultDiffer((bool *)diffChunk, (bool *)heights1, (bool *)heights2, numberOfPoles, pixelChunk, imageWidthInBytes, chunkWidthInPixels, pixelWidthInSamples);
				} break;
				default:;
				}
			}
			else
			{
				switch (channel)
				{
				case HeightSampler::CH_HEIGHT:
					_processHeightChunk((float *)heights1, numberOfPoles, pixelChunk, imageWidthInBytes, chunkWidthInPixels, pixelWidthInSamples);
					break;

				case HeightSampler::CH_FLORA_STATIC_COLLIDABLE:
				case HeightSampler::CH_FLORA_STATIC_NON_COLLIDABLE:
					_processFloraChunk((FloraGroup::Info *)heights1, numberOfPoles, pixelChunk, imageWidthInBytes, chunkWidthInPixels, pixelWidthInSamples);
					break;

				default:
					break;
				}
			}
		}
	}


	//////////////////////////////////////////////////////////////////////////////
	image->unlock();
	TargaFormat().saveImage(*image, o_tgaFile);
	delete image;
	return true;
}

bool HeightSampler::writeTGA(const char *o_tga_base_path, const char *i_sample_file1, const char *i_diff_file)
{
	UNREF(o_tga_base_path);

	FILE *f1=0, *f2=0;

	if (i_sample_file1 && *i_sample_file1)
	{
		f1 = fopen(i_sample_file1, "rb");
		if (!f1)
		{
			REPORT_LOG_PRINT(true, ("Error! Could not open file: %s\n", i_sample_file1));
			return false;
		}

		if (i_diff_file && *i_diff_file)
		{
			f2 = fopen(i_diff_file, "rb");
			if (!f2)
			{
				REPORT_LOG_PRINT(true, ("Error! Could not open file: %s\n", i_diff_file));
				fclose(f1);
				return false;
			}
		}
	}
	else
	{
		REPORT_LOG_PRINT(true, ("Error! Nothing to do!\n"));
		return false;
	}

	bool ret_val=false;

	///////////////////////////////////////////////////////////////////////////////////////////
	// read and compare headers
	TerrainSampleFileHeader fileHeader1, fileHeader2;
	memset(&fileHeader1, 0, sizeof(TerrainSampleFileHeader));
	memset(&fileHeader2, 0, sizeof(TerrainSampleFileHeader));
	fread(&fileHeader1, sizeof(TerrainSampleFileHeader), 1, f1);
	if (f2)
	{
		fread(&fileHeader2, sizeof(TerrainSampleFileHeader), 1, f2);
	}

	if (!f2 || fileHeader1==fileHeader2)
	{
		const int   mapWidthInMeters     = fileHeader1.mapWidthInMeters;
		const int   numberOfPoles        = fileHeader1.numberOfPoles;
		const float tileWidthInMeters    = fileHeader1.tileWidthInMeters;
		const float distanceBetweenPoles = tileWidthInMeters * 0.5f;
		const float chunkWidthInMeters   = distanceBetweenPoles * float(numberOfPoles);
		const int   mapWidthInChunks     = mapWidthInMeters / chunkWidthInMeters;

		const int   channel = fileHeader1.channel!=0 ? fileHeader1.channel : HeightSampler::CH_HEIGHT;
		const int   diffWidth = (channel==CH_HEIGHT) ? 1024 : 8192;

		const int   tgaWidth = (f2) ? diffWidth : 8192;
		const int   mapWidthInImages = 1;

		const int   mapWidthInPixels = tgaWidth * mapWidthInImages;
		const int   metersPerPixel = mapWidthInMeters / mapWidthInPixels;

/*
bool _writeTGA(
	const char *o_tgaFile,
	int ulChunkX,
	int ulChunkZ,
	int imageWidth, 
	int metersPerPixel, 
	FILE *f1, 
	FILE *f2
	)
*/

		// UL image
		_writeTGA(
			o_tga_base_path,
			0,
			0,
			tgaWidth,
			metersPerPixel,
			f1,
			f2
		);
/*
		// UR image
		_writeTGA(
			"testfile2.tga",
			mapWidthInChunks/2,
			0,
			tgaWidth,
			metersPerPixel,
			f1,
			f2
		);

		// LL image
		_writeTGA(
			"testfile3.tga",
			mapWidthInChunks/2,
			mapWidthInChunks/2,
			tgaWidth,
			metersPerPixel,
			f1,
			f2
		);

		// LR image
		_writeTGA(
			"testfile4.tga",
			0,
			mapWidthInChunks/2,
			tgaWidth,
			metersPerPixel,
			f1,
			f2
		);
*/

		ret_val=true;
	}
	else
	{
		REPORT_LOG_PRINT(true, ("File types incompatible.\n"));
	}

	///////////////////////////////////////////////////////////////////////////////////////////
	fclose(f1);
	if (f2)
	{
		fclose(f2);
	}

	return ret_val;
}

//-------------------------------------------------------------------

bool HeightSampler::createStaticFloraList(const char *i_terrain_file, const char *o_flora_file, const Rectangle2d &clipRectangle)
{
	if (!i_terrain_file || !*i_terrain_file)
	{
		return false;
	}
	if (!o_flora_file || !*o_flora_file)
	{
		return false;
	}

	if (  _access(o_flora_file, 00)==0 // exists
		&& _access(o_flora_file, 02)!=0 // not writable
		)
	{
		REPORT_LOG_PRINT(true, ("Can't write to output file %s\n", o_flora_file));
		return false;
	}

	Iff iff;
	if (!iff.open(i_terrain_file, true))
	{
		FATAL(true, ("Could not open appearance file %s", i_terrain_file));
	}

	const Tag tag = iff.getCurrentName();

	AppearanceTemplate *const appearanceTemplate = SamplerProceduralTerrainAppearanceTemplate::create(i_terrain_file, &iff);
	if (!appearanceTemplate)
	{
		return false;
	}

	SamplerProceduralTerrainAppearanceTemplate *const samplerAppearanceTemplate = static_cast<SamplerProceduralTerrainAppearanceTemplate *>(appearanceTemplate);

	samplerAppearanceTemplate->setMapsToSample(TGM_height | TGM_vertexPosition | TGM_vertexNormal | TGM_floraStaticCollidable);

	Appearance * const appearance = appearanceTemplate->createAppearance();
	if (!appearance)
	{
		return false;
	}

	SamplerProceduralTerrainAppearance *const samplerAppearance = dynamic_cast<SamplerProceduralTerrainAppearance *>(appearance);

	const ProceduralTerrainAppearanceTemplate *proceduralTerrainAppearanceTemplate = static_cast<const ProceduralTerrainAppearanceTemplate *>(appearanceTemplate);
	const TerrainGenerator* const terrainGenerator = proceduralTerrainAppearanceTemplate->getTerrainGenerator ();

	const int   mapWidthInMeters     = proceduralTerrainAppearanceTemplate->getMapWidthInMeters();
	const float chunkWidthInMeters   = proceduralTerrainAppearanceTemplate->getChunkWidthInMeters();
	const int   mapWidthInChunks     = mapWidthInMeters / chunkWidthInMeters;
	const int   mapChunkOffset       = -mapWidthInChunks/2;

	//---------------------------------------------------------

	const float floraTileWidthInMeters = proceduralTerrainAppearanceTemplate->getFloraTileWidthInMeters();
	const int   mapWidthInFlora        = proceduralTerrainAppearanceTemplate->getMapWidthInFlora();
	const int   floraTileCenterOffset  = mapWidthInFlora / 2;

	int startTileX = clamp(-floraTileCenterOffset, int(floor(clipRectangle.x0 / floraTileWidthInMeters)), floraTileCenterOffset);
	int startTileZ = clamp(-floraTileCenterOffset, int(floor(clipRectangle.y0 / floraTileWidthInMeters)), floraTileCenterOffset);
	int stopTileX  = clamp(-floraTileCenterOffset, int( ceil(clipRectangle.x1 / floraTileWidthInMeters)), floraTileCenterOffset);
	int stopTileZ  = clamp(-floraTileCenterOffset, int( ceil(clipRectangle.y1 / floraTileWidthInMeters)), floraTileCenterOffset);

	int startX = clamp(0, int(startTileX * floraTileWidthInMeters / chunkWidthInMeters) - mapChunkOffset, mapWidthInChunks);
	int startZ = clamp(0, int(startTileZ * floraTileWidthInMeters / chunkWidthInMeters) - mapChunkOffset, mapWidthInChunks);
	int stopX  = clamp(0, int( stopTileX * floraTileWidthInMeters / chunkWidthInMeters) - mapChunkOffset, mapWidthInChunks);
	int stopZ  = clamp(0, int( stopTileZ * floraTileWidthInMeters / chunkWidthInMeters) - mapChunkOffset, mapWidthInChunks);

	//---------------------------------------------------------

	REPORT_LOG_PRINT(true, ("\n\n*** Sampling %s flora to %s...\n\n", i_terrain_file, o_flora_file));

	samplerAppearanceTemplate->beginStaticCollidableFloraSampling();

	for (int z=startZ;z<stopZ;z+=1)
	{
		REPORT_LOG_PRINT(true, ("%i/%i  ", z, stopZ-startZ));

		const int cz = z+mapChunkOffset;
		for (int x=startX;x<stopX;x+=1)
		{
			const int cx = x+mapChunkOffset;

			if (samplerAppearance->isStaticCollidableFloraChunk(cx, cz, 1))
			{
				samplerAppearance->createChunk(cx, cz, 1);
				samplerAppearance->purgeChunks();
			}
		}
		REPORT_LOG_PRINT(true, ("%i flora.\n", samplerAppearanceTemplate->getNumberOfSampledFlora()));
	}

	REPORT_LOG_PRINT(true, ("\n\n"));

	//---------------------------------------------------------

	int tileBounds[4] = { 
		startTileX+floraTileCenterOffset, 
		startTileZ+floraTileCenterOffset, 
		 stopTileX+floraTileCenterOffset, 
		 stopTileZ+floraTileCenterOffset 
	};
	samplerAppearanceTemplate->writeStaticCollidableFloraFile(o_flora_file, tileBounds);

	//---------------------------------------------------------

	delete appearance;
	delete appearanceTemplate;
	return true;
}