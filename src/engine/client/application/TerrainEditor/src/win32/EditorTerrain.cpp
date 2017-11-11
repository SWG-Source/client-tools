//===================================================================
//
// EditorTerrain.cpp
// asommers 2001-01-17
//
// copyright 2001, verant interactive
//
//===================================================================

#include "FirstTerrainEditor.h"
#include "EditorTerrain.h"

#include "MapView.h"
#include "ProgressDialog.h"
#include "TerrainEditorDoc.h"
#include "TerrainGeneratorHelper.h"
#include "clientGraphics/DynamicVertexBuffer.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/StaticShader.h"
#include "clientGraphics/Texture.h"
#include "clientGraphics/TextureList.h"
#include "sharedDebug/PerformanceTimer.h"
#include "sharedDebug/VTune.h"
#include "sharedMath/Line2d.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedTerrain/AffectorColor.h"
#include "sharedTerrain/AffectorRibbon.h"
#include "sharedTerrain/AffectorRiver.h"
#include "sharedTerrain/AffectorRoad.h"
#include "sharedTerrain/Boundary.h"
#include "sharedUtility/BakedTerrain.h"

#include <map>

//===================================================================

namespace
{
	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	static unsigned long computeChunkMapKey (const int x, const int z)
	{
		return (x & 0x0FFFF) << 16 | (z & 0x0FFFF);
	}

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	static Shader* createInvalidShader (const int width)
	{
		const size_t size = static_cast<size_t> (width * width * 3);
		uint8* pixelData = new uint8 [size];
		memset (pixelData, 0, size);

		int i;
		for (i = 0; i < width; i++)
		{
			const int index = (i * 3 * width) + ((width - 1 - i) * 3);

			pixelData [index + 0] = static_cast<uint8> (0);
			pixelData [index + 1] = static_cast<uint8> (0);
			pixelData [index + 2] = static_cast<uint8> (255);
		}

		Shader *shader = ShaderTemplateList::fetchModifiableShader("shader\\terraineditor.sht");

		const TextureFormat  runtimeFormats[] = { TF_RGB_888, TF_XRGB_8888, TF_RGB_565 };
		const int numberOfRuntimeFormats = sizeof(runtimeFormats) / sizeof(runtimeFormats[0]);
		const Texture *texture = TextureList::fetch(pixelData, TF_RGB_888, width, width, runtimeFormats, numberOfRuntimeFormats);

		static_cast<StaticShader*>(shader)->setTexture(TAG(M,A,I,N), *texture);

		texture->release();

		delete [] pixelData;
		pixelData = 0;

		return shader;
	}

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
}

//===================================================================

EditorTerrain::Data::Data () :
	shaderFamilyName (),
	floraStaticCollidableFamilyName (),
	floraStaticNonCollidableFamilyName (),
	floraDynamicNearFamilyName (),
	floraDynamicFarFamilyName (),
	height (0)
{
}

//===================================================================
	
class EditorTerrain::Chunk
{
	friend class EditorTerrain;

public:

	Chunk (const EditorTerrain* newTerrain, int newChunkX, int newChunkZ, float chunkWidthInMeters);
	~Chunk ();

	void          create (const TerrainGenerator::GeneratorChunkData* generatorChunkData, bool noTexture);

	int           getChunkX () const;
	int           getChunkZ () const;
	float         getChunkWidthInMeters() const { return chunkWidthInMeters; }

	bool          isDirty () const;
	void          setDirty (bool newDirty);

	void          setTime (real newTime);
	real          getTime () const;

	bool          getDataAt (const Vector2d& position, Data& data, bool heightOnly=false) const;

	void          render (const HardwareVertexBuffer& vertexBuffer) const;
	void          setShaderTextureFactor (uint32 newDim, uint32 newMain);

	const Rectangle2d& getExtent () const;

private:

	Chunk ();
	Chunk (const Chunk& rhs);
	Chunk& operator= (const Chunk& rhs);

private:

	//--
	const EditorTerrain*       terrain;

	int                        chunkX;
	int                        chunkZ;
	float                      chunkWidthInMeters;

	Shader*                    shader;

	bool                       dirty;

	real                       time;

	Rectangle2d                extent;

	//-- copy of map for data
	Array2d<real>              heightMap;
	Array2d<Vector>            vertexNormalMap;
	Array2d<ShaderGroup::Info> shaderMap;
	Array2d<FloraGroup::Info>  staticNonCollidableMap;
	Array2d<FloraGroup::Info>  staticCollidableMap;
	Array2d<RadialGroup::Info> dynamicFarMap;
	Array2d<RadialGroup::Info> dynamicNearMap;
	Array2d<EnvironmentGroup::Info> environmentMap;
};

//===================================================================

EditorTerrain::Chunk::Chunk (const EditorTerrain* newTerrain, int newChunkX, int newChunkZ, float i_chunkWidthInMeters) :
	terrain (newTerrain),
	chunkX (newChunkX),
	chunkZ (newChunkZ),
	chunkWidthInMeters(i_chunkWidthInMeters),
	shader (0),
	dirty (false),
	time (0),
	extent (),
	heightMap (),
	vertexNormalMap (),
	staticNonCollidableMap (),
	staticCollidableMap (),
	dynamicFarMap (),
	dynamicNearMap (),
	environmentMap ()
{
}

//-------------------------------------------------------------------
	
EditorTerrain::Chunk::~Chunk ()
{
	terrain  = 0;

	if (shader)
	{
		shader->release();
		shader = 0;
	}
}

//-------------------------------------------------------------------

int EditorTerrain::Chunk::getChunkX () const
{
	return chunkX;
}

//-------------------------------------------------------------------

int EditorTerrain::Chunk::getChunkZ () const
{
	return chunkZ;
}

//-------------------------------------------------------------------

bool EditorTerrain::Chunk::isDirty () const
{
	return dirty;
}

//-------------------------------------------------------------------

void EditorTerrain::Chunk::setDirty (bool newDirty)
{
	dirty = newDirty;
}

//-------------------------------------------------------------------

real EditorTerrain::Chunk::getTime () const
{
	return time;
}

//-------------------------------------------------------------------

void EditorTerrain::Chunk::setTime (real newTime)
{
	time = newTime;
}

//-------------------------------------------------------------------

bool EditorTerrain::Chunk::getDataAt (const Vector2d& position, EditorTerrain::Data& data, bool heightOnly) const
{
	if (dirty)
		return false;
	
	DEBUG_FATAL (!extent.isWithin (position), (""));

	const real ratioX = (position.x - extent.x0) / extent.getWidth ();
	DEBUG_FATAL (!WithinRangeInclusiveInclusive (0.f, ratioX, 1.f), ("ratioX out of range"));
	const real ratioY = (position.y - extent.y0) / extent.getHeight ();
	DEBUG_FATAL (!WithinRangeInclusiveInclusive (0.f, ratioY, 1.f), ("ratioY out of range"));

	const int x = 1 + static_cast<int> (ratioX * (heightMap.getWidth () - 3));
	const int y = 1 + static_cast<int> (ratioY * (heightMap.getHeight () - 3));

	data.height = heightMap.getData (x, y);

#if 0
	{
		const real minimum = 2.f;
		UNREF (minimum);

		//-- verify height is somewhat correct
		const Vector p = vertexPositionMap.getData (x, y);
		UNREF (p);

		const Vector p2 = vertexPositionMap.getData (x - 1, y);
		UNREF (p2);

		DEBUG_FATAL (!WithinRangeInclusiveInclusive (p.x - minimum, position.x, p.x + minimum), (""));
		DEBUG_FATAL (!WithinRangeInclusiveInclusive (p.z - minimum, position.y, p.z + minimum), (""));
	}
#endif

	if (!heightOnly)
	{
		{
			const ShaderGroup::Info sgi = shaderMap.getData (x, y);
			if (sgi.getFamilyId ())
				data.shaderFamilyName = terrain->document->getTerrainGenerator ()->getShaderGroup ().getFamilyName (sgi.getFamilyId ());
		}

		{
			const FloraGroup::Info fgi = staticCollidableMap.getData (x, y);
			if (fgi.getFamilyId ())
				data.floraStaticCollidableFamilyName = terrain->document->getTerrainGenerator ()->getFloraGroup ().getFamilyName (fgi.getFamilyId ());
		}

		{
			const FloraGroup::Info fgi = staticNonCollidableMap.getData (x, y);
			if (fgi.getFamilyId ())
				data.floraStaticNonCollidableFamilyName = terrain->document->getTerrainGenerator ()->getFloraGroup ().getFamilyName (fgi.getFamilyId ());
		}

		{
			const RadialGroup::Info rgi = dynamicNearMap.getData (x, y);
			if (rgi.getFamilyId())
				data.floraDynamicNearFamilyName = terrain->document->getTerrainGenerator ()->getRadialGroup ().getFamilyName (rgi.getFamilyId());
		}

		{
			const RadialGroup::Info rgi = dynamicFarMap.getData (x, y);
			if (rgi.getFamilyId())
				data.floraDynamicFarFamilyName = terrain->document->getTerrainGenerator ()->getRadialGroup ().getFamilyName (rgi.getFamilyId());
		}

		{
			const EnvironmentGroup::Info egi = environmentMap.getData (x, y);
			if (egi.getFamilyId())
				data.environmentFamilyName = terrain->document->getTerrainGenerator ()->getEnvironmentGroup ().getFamilyName (egi.getFamilyId());
		}
	}

	return true;
}

//-------------------------------------------------------------------

void EditorTerrain::Chunk::create (const TerrainGenerator::GeneratorChunkData* const generatorChunkData, const bool noTexture)
{
	NOT_NULL (generatorChunkData);
	NOT_NULL (terrain);

	//-- calculate texture width
	const int numberOfPoles = generatorChunkData->numberOfPoles;
	const int originOffset  = generatorChunkData->originOffset;
	const int upperPad      = generatorChunkData->upperPad;
	const int width         = numberOfPoles - originOffset - upperPad;
	DEBUG_FATAL (!IsPowerOfTwo (width), ("width is not a power of 2"));

	extent.x0 = generatorChunkData->start.x;
	extent.y0 = generatorChunkData->start.z;
	extent.x1 = generatorChunkData->start.x + generatorChunkData->distanceBetweenPoles * (width + 1);
	extent.y1 = generatorChunkData->start.z + generatorChunkData->distanceBetweenPoles * (width + 1);

	const Array2d<real>* gcd_heightMap = generatorChunkData->heightMap;
	NOT_NULL (gcd_heightMap);

	heightMap.allocate (generatorChunkData->heightMap->getWidth (), generatorChunkData->heightMap->getHeight ());
	heightMap.makeCopy (*gcd_heightMap);

	const Array2d<Vector>* gcd_vertexNormalMap = generatorChunkData->vertexNormalMap;
	NOT_NULL (gcd_vertexNormalMap);

	vertexNormalMap.allocate (generatorChunkData->vertexNormalMap->getWidth (), generatorChunkData->vertexNormalMap->getHeight ());
	vertexNormalMap.makeCopy (*gcd_vertexNormalMap);

	if (noTexture)
		return;

	const Array2d<PackedRgb>* colorMap  = generatorChunkData->colorMap;
	NOT_NULL (colorMap);

	const Array2d<ShaderGroup::Info>* gcd_shaderMap = generatorChunkData->shaderMap;
	NOT_NULL (gcd_shaderMap);

	shaderMap.allocate (generatorChunkData->shaderMap->getWidth (), generatorChunkData->shaderMap->getHeight ());
	shaderMap.makeCopy (*gcd_shaderMap);

	const Array2d<FloraGroup::Info>* gcd_floraStaticCollidableMap = generatorChunkData->floraStaticCollidableMap;
	NOT_NULL (gcd_floraStaticCollidableMap);

	staticCollidableMap.allocate (generatorChunkData->floraStaticCollidableMap->getWidth (), generatorChunkData->floraStaticCollidableMap->getHeight ());
	staticCollidableMap.makeCopy (*gcd_floraStaticCollidableMap);

	const Array2d<FloraGroup::Info>* gcd_floraStaticNonCollidableMap = generatorChunkData->floraStaticNonCollidableMap;
	NOT_NULL (gcd_floraStaticNonCollidableMap);

	staticNonCollidableMap.allocate (generatorChunkData->floraStaticNonCollidableMap->getWidth (), generatorChunkData->floraStaticNonCollidableMap->getHeight ());
	staticNonCollidableMap.makeCopy (*gcd_floraStaticNonCollidableMap);

	const Array2d<RadialGroup::Info>* gcd_floraDynamicNearMap = generatorChunkData->floraDynamicNearMap;
	NOT_NULL (gcd_floraDynamicNearMap);

	dynamicNearMap.allocate (generatorChunkData->floraDynamicNearMap->getWidth (), generatorChunkData->floraDynamicNearMap->getHeight ());
	dynamicNearMap.makeCopy (*gcd_floraDynamicNearMap);

	const Array2d<RadialGroup::Info>* gcd_floraDynamicFarMap = generatorChunkData->floraDynamicFarMap;
	NOT_NULL (gcd_floraDynamicFarMap);

	dynamicFarMap.allocate (generatorChunkData->floraDynamicFarMap->getWidth (), generatorChunkData->floraDynamicFarMap->getHeight ());
	dynamicFarMap.makeCopy (*gcd_floraDynamicFarMap);

	const Array2d<EnvironmentGroup::Info>* gcd_environmentMap = generatorChunkData->environmentMap;
	NOT_NULL (gcd_environmentMap);

	environmentMap.allocate (generatorChunkData->environmentMap->getWidth (), generatorChunkData->environmentMap->getHeight ());
	environmentMap.makeCopy (*gcd_environmentMap);

	const Array2d<Vector>* vertexPositionMap = generatorChunkData->vertexPositionMap;
	NOT_NULL (vertexPositionMap);

	const Array2d<bool>* excludeMap = generatorChunkData->excludeMap;
	NOT_NULL (excludeMap);

	const Array2d<bool>* passableMap = generatorChunkData->passableMap;
	NOT_NULL (passableMap);

	const MapView* mapView = terrain->mapView;  //lint !e578  //-- hides mapView
	NOT_NULL (mapView);

	const TerrainEditorDoc* document = terrain->document;  //lint !e578  //-- hides document
	NOT_NULL (document);

	const real distance = document->getWhiteHeight () - document->getBlackHeight ();

#if MAPVIEW_REVERSED
	Vector toLight (-1, 1, 1);
	toLight.normalize ();
#else
	Vector toLight (-1, 1, -1);
	toLight.normalize ();
#endif

	//-- create texture
	uint8* pixelData = new uint8 [static_cast<size_t> (width * width * 3)];

	int x;
	int z;
	for (z = 1; z < numberOfPoles - 2; z++)
	{
		for (x = 1; x < numberOfPoles - 2; x++)
		{
			bool set   = false;
			real r     = CONST_REAL (0);
			real g     = CONST_REAL (0);
			real b     = CONST_REAL (0);

			if (!excludeMap->getData (x, z))
			{
				//-- color is initially set to pink
				if (mapView->getShowHeightMap ())
				{
					if (mapView->getShowLighting ())
					{
						r = g = b = clamp (0.f, toLight.dot (gcd_vertexNormalMap->getData (x, z)), 1.f);
					}
					else
					{
						const real y = clamp (document->getBlackHeight (), gcd_heightMap->getData (x, z), CONST_REAL (document->getWhiteHeight ()));

						r = g = b = (distance == CONST_REAL (0)) ? CONST_REAL (0) : ((y - document->getBlackHeight ()) / distance);
					}

					set = true;
				}

				if (mapView->getShowColorMap () || mapView->getShowShaderMap ())
				{
					//-- brighten the height color before mixing in vertex color
					r = r * CONST_REAL (0.5) + CONST_REAL (0.5);
					g = g * CONST_REAL (0.5) + CONST_REAL (0.5);
					b = b * CONST_REAL (0.5) + CONST_REAL (0.5);
				}

				if (mapView->getShowColorMap ())
				{
					const PackedRgb color = colorMap->getData (x, z);

					if (set)
					{
						r *= static_cast<real> (color.r) * RECIP (255);
						g *= static_cast<real> (color.g) * RECIP (255);
						b *= static_cast<real> (color.b) * RECIP (255);
					}
					else
					{
						r = static_cast<real> (color.r) * RECIP (255);
						g = static_cast<real> (color.g) * RECIP (255);
						b = static_cast<real> (color.b) * RECIP (255);
					}

					set = true;
				}

				if (mapView->getShowPassableMap())
				{
					if (!passableMap->getData(x, z))
					{
						r *= 0.5f;
						g *= 0.5f;
						b *= 0.5f;
					}
				}

				if (mapView->getShowShaderMap ())
				{
					const int familyId = gcd_shaderMap->getData (x, z).getFamilyId ();

					if (familyId)
					{
						const PackedRgb color = document->getTerrainGenerator ()->getShaderGroup ().getFamilyColor (familyId);

						if (set)
						{
							r *= static_cast<real> (color.r) * RECIP (255);
							g *= static_cast<real> (color.g) * RECIP (255);
							b *= static_cast<real> (color.b) * RECIP (255);
						}
						else
						{
							r = static_cast<real> (color.r) * RECIP (255);
							g = static_cast<real> (color.g) * RECIP (255);
							b = static_cast<real> (color.b) * RECIP (255);
						}

						set = true;
					}
				}

				if (mapView->getShowRadialMap ())
				{
					const int familyId = gcd_floraDynamicFarMap->getData (x, z).getFamilyId();

					if (familyId)
					{
						const PackedRgb color = document->getTerrainGenerator ()->getRadialGroup ().getFamilyColor (familyId);

						r = static_cast<real> (color.r) * RECIP (255);
						g = static_cast<real> (color.g) * RECIP (255);
						b = static_cast<real> (color.b) * RECIP (255);
					}
				}

				if (mapView->getShowRadialMap ())
				{
					const int familyId = gcd_floraDynamicNearMap->getData (x, z).getFamilyId();

					if (familyId)
					{
						const PackedRgb color = document->getTerrainGenerator ()->getRadialGroup ().getFamilyColor (familyId);

						r = static_cast<real> (color.r) * RECIP (255);
						g = static_cast<real> (color.g) * RECIP (255);
						b = static_cast<real> (color.b) * RECIP (255);
					}
				}

				if (mapView->getShowFloraMap ())
				{
					const int familyId = gcd_floraStaticNonCollidableMap->getData (x, z).getFamilyId ();

					if (familyId)
					{
						const PackedRgb color = document->getTerrainGenerator ()->getFloraGroup ().getFamilyColor (familyId);

						r = static_cast<real> (color.r) * RECIP (255);
						g = static_cast<real> (color.g) * RECIP (255);
						b = static_cast<real> (color.b) * RECIP (255);
					}
				}

				if (mapView->getShowFloraMap ())
				{
					const int familyId = gcd_floraStaticCollidableMap->getData (x, z).getFamilyId ();

					if (familyId)
					{
						const PackedRgb color = document->getTerrainGenerator ()->getFloraGroup ().getFamilyColor (familyId);

						r = static_cast<real> (color.r) * RECIP (255);
						g = static_cast<real> (color.g) * RECIP (255);
						b = static_cast<real> (color.b) * RECIP (255);
					}
				}

				if (mapView->getShowEnvironmentMap ())
				{
					const int familyId = gcd_environmentMap->getData (x, z).getFamilyId();

					if (familyId)
					{
						const PackedRgb color = document->getTerrainGenerator ()->getEnvironmentGroup ().getFamilyColor (familyId);

						r = static_cast<real> (color.r) * RECIP (255);
						g = static_cast<real> (color.g) * RECIP (255);
						b = static_cast<real> (color.b) * RECIP (255);
					}
				}
			}

			if (mapView->getShowWater ())
			{
				bool showWater = false;

				{
					//-- handle local water tables
					const real worldX = generatorChunkData->start.x + (static_cast<real> (x) * generatorChunkData->distanceBetweenPoles);
					const real worldZ = generatorChunkData->start.z + (static_cast<real> (z) * generatorChunkData->distanceBetweenPoles);

					const real height = gcd_heightMap->getData (x, z);

					int i;
					for (i = 0; i < terrain->m_localWaterTableList.getNumberOfElements (); ++i)
					{
						if (terrain->m_localWaterTableList [i]->isWithin (worldX, worldZ) > 0)
						{
							if (terrain->m_localWaterTableList [i]->getType () == TGBT_polygon && static_cast<const BoundaryPolygon*> (terrain->m_localWaterTableList [i])->getLocalWaterTableHeight () > height)
							{
								showWater = true;

								break;
							}

							if (terrain->m_localWaterTableList [i]->getType () == TGBT_rectangle && static_cast<const BoundaryRectangle*> (terrain->m_localWaterTableList [i])->getLocalWaterTableHeight () > height)
							{
								showWater = true;

								break;
							}
						}
					}
					
					if(!showWater)
					{
						const Vector pos(worldX,0.0f,worldZ);
						for(i = 0; i < terrain->m_affectorRibbonList.getNumberOfElements();++i)
						{
							const AffectorRibbon* affectorRibbon = terrain->m_affectorRibbonList[i];
							if(affectorRibbon->getExtent().isWithin(worldX,worldZ))
							{
								// check endcap first
								if(affectorRibbon->getCapWidth() && affectorRibbon->getEndCapExtent().isWithin(worldX,worldZ))
								{
									const ArrayList<Vector2d>& endCapPointList = affectorRibbon->getEndCapPointList();
									const Vector p0 = Vector(endCapPointList[0].x,0.0f,endCapPointList[0].y);
									const Vector p1 = Vector(endCapPointList[1].x,0.0f,endCapPointList[1].y);
									const Vector p2 = Vector(endCapPointList[2].x,0.0f,endCapPointList[2].y);
									const Vector p3 = Vector(endCapPointList[3].x,0.0f,endCapPointList[3].y);
									const Vector p4 = Vector(endCapPointList[4].x,0.0f,endCapPointList[4].y);
									const Vector p5 = Vector(endCapPointList[5].x,0.0f,endCapPointList[5].y);
									const Vector p6 = Vector(endCapPointList[6].x,0.0f,endCapPointList[6].y);
									const Vector p7 = Vector(endCapPointList[7].x,0.0f,endCapPointList[7].y);
									// check the triangles in 2 fans
									if(
										pos.inPolygon(p0,p1,p2)
										|| pos.inPolygon(p0,p2,p3)
										|| pos.inPolygon(p0,p3,p4)
										|| pos.inPolygon(p4,p5,p6)
										|| pos.inPolygon(p4,p6,p7)
										|| pos.inPolygon(p4,p7,p0)
									)
									{
										if(affectorRibbon->getHeightList()[0] > height)
										{
											showWater = true;
											break;
										}

									}

								}

								// now check the strip
								ArrayList<AffectorRibbon::Quad> ribbonQuadList;
								affectorRibbon->createQuadList (ribbonQuadList); // JU_TODO: pregen this
								bool found = false;
								int j;
								for(j = 0; j < ribbonQuadList.size(); ++j)
								{
									const AffectorRibbon::Quad& ribbonQuad = (ribbonQuadList) [j];
									// check the 2 triangles on the quad
									const Vector p0 = Vector(ribbonQuad.points[0].x,0.0f,ribbonQuad.points[0].z);
									const Vector p1 = Vector(ribbonQuad.points[1].x,0.0f,ribbonQuad.points[1].z);
									const Vector p2 = Vector(ribbonQuad.points[2].x,0.0f,ribbonQuad.points[2].z);
									const Vector p3 = Vector(ribbonQuad.points[3].x,0.0f,ribbonQuad.points[3].z);

									if(pos.inPolygon(p0,p1,p2) || pos.inPolygon(p0,p2,p3))
									{

										// project the world point onto the line between the control points and interp for the height
			
										const Vector2d myPos(pos.x,pos.z);
										const Vector temp1 = (ribbonQuad.points[0] + ribbonQuad.points[1])/2.0f;
										const Vector temp2 = (ribbonQuad.points[2] + ribbonQuad.points[3])/2.0f;
										const Vector2d p1(temp1.x,temp1.z);
										const Vector2d p2(temp2.x,temp2.z);
										const float startHeight = temp1.y;
										const float endHeight = temp2.y;
										const float totalDistance = p1.magnitudeBetween(p2);
										
										
										const Line2d line (p1,p2);
										const float distToLine = line.computeDistanceTo(myPos);
										const float realDistToPoint1 = p1.magnitudeBetween(myPos);
										const float realDistToPoint2 = p2.magnitudeBetween(myPos);
										const float projectedDistToPoint1 = sqrt(sqr(realDistToPoint1) - sqr(distToLine));
										const float projectedDistToPoint2 = sqrt(sqr(realDistToPoint2) - sqr(distToLine));

										float newHeight;
										if(totalDistance != 0.0f && projectedDistToPoint1 <= totalDistance && projectedDistToPoint2 <= totalDistance)
										{
											newHeight = startHeight + (projectedDistToPoint1/totalDistance) * (endHeight - startHeight);
										}
										else
										{
											newHeight = startHeight;	
										}

										if(newHeight > height)
										{
											showWater = true;
											found = true;
											break;
										}
									}
								}
								if(found)
									break;
								
							}
						}
					}
				}

				if (showWater || (document->getUseGlobalWaterTable () && gcd_heightMap->getData (x, z) < document->getGlobalWaterTableHeight ()))
				{
					r = static_cast<real> (128) * RECIP (255);
					g = static_cast<real> (128) * RECIP (255);
					b = static_cast<real> (255) * RECIP (255);
				}
			}

			r = clamp (CONST_REAL (0), r * 255, CONST_REAL (255));
			g = clamp (CONST_REAL (0), g * 255, CONST_REAL (255));
			b = clamp (CONST_REAL (0), b * 255, CONST_REAL (255));

			pixelData [((z - 1) * width * 3) + ((x - 1) * 3) + 0] = static_cast<uint8> (b);
			pixelData [((z - 1) * width * 3) + ((x - 1) * 3) + 1] = static_cast<uint8> (g);
			pixelData [((z - 1) * width * 3) + ((x - 1) * 3) + 2] = static_cast<uint8> (r);
		}
	}

	const TextureFormat  runtimeFormats[]       = { TF_RGB_888, TF_XRGB_8888, TF_RGB_565 };
	const int            numberOfRuntimeFormats = sizeof(runtimeFormats) / sizeof(runtimeFormats[0]);

	shader = ShaderTemplateList::fetchModifiableShader("shader\\terraineditor.sht");
	const Texture *texture = TextureList::fetch(pixelData, TF_RGB_888, width, width, runtimeFormats, numberOfRuntimeFormats);
	static_cast<StaticShader*>(shader)->setTexture(TAG(M,A,I,N), *texture);
	texture->release();

	delete [] pixelData;
	pixelData = 0;
}

//-------------------------------------------------------------------

void EditorTerrain::Chunk::render (const HardwareVertexBuffer& vertexBuffer) const
{  
	Graphics::setVertexBuffer (vertexBuffer);  

	NOT_NULL (shader);
	const StaticShader& staticShader = shader->prepareToView ();

	const int numberOfPasses = staticShader.getNumberOfPasses ();
	int i;
	for (i = 0; i < numberOfPasses; ++i)
	{
		Graphics::setStaticShader (staticShader, i);
		Graphics::drawTriangleFan ();
	}

	if (dirty)
	{
		Graphics::setStaticShader (ShaderTemplateList::get2dVertexColorAStaticShader ());
		Graphics::drawTriangleFan ();
	}
}

//===================================================================

EditorTerrain::EditorTerrain (TerrainEditorDoc* newDocument, const MapView* newMapView) :
	document (newDocument),
	mapView (newMapView),
	createChunkBuffer (),
	bakedCreateChunkBuffer (),
	chunkWidthInMeters_w (0.f),
	distanceBetweenPoles_w (0.f),
	originOffset(1),
	upperPad(2),
	numberOfPoles(16 + originOffset + upperPad),
	m_chunkMap (NON_NULL (new ChunkMap)),
	invalidShader (0),
	m_localWaterTableList (),
	m_affectorRibbonList (),
	m_heightDataChunkMap (NON_NULL (new ChunkMap)),
	m_resolutionType (RT_medium)
{
	setResolutionType (RT_medium);

	invalidShader = createInvalidShader (static_cast<int> (chunkWidthInMeters_w));
	static_cast<StaticShader*> (invalidShader)->setTextureFactor (TAG3 (D,I,M),  PackedRgb::solidWhite.asUint32 ());
	static_cast<StaticShader*> (invalidShader)->setTextureFactor (TAG (M,A,I,N), PackedRgb::solidBlack.asUint32 ());
}

//-------------------------------------------------------------------
	
EditorTerrain::~EditorTerrain ()
{
	if (invalidShader)
	{
		invalidShader->release();
		invalidShader = 0;
	}

	clearChunks ();

	delete m_chunkMap;
	m_chunkMap = 0;

	delete m_heightDataChunkMap;
	m_heightDataChunkMap = 0;

	mapView  = 0;
	document = 0;
}

//-------------------------------------------------------------------

void EditorTerrain::update (bool showProgress)
{
	NOT_NULL (mapView);

	CRect rect;
	mapView->GetClientRect (&rect);

	//-- find out which chunks are visible and build them
	const Vector2d topLeft     = mapView->convertScreenToWorld (rect.TopLeft ());
	const Vector2d bottomRight = mapView->convertScreenToWorld (rect.BottomRight ());

	CRect chunkRect;
#if MAPVIEW_REVERSED
	chunkRect.top    = calculateChunkZ (topLeft) + 1;
	chunkRect.bottom = calculateChunkZ (bottomRight) - 1;
	std::swap (chunkRect.top, chunkRect.bottom);
#else
	chunkRect.top    = calculateChunkZ (topLeft) - 1;
	chunkRect.bottom = calculateChunkZ (bottomRight) + 1;
#endif
	chunkRect.left   = calculateChunkX (topLeft) - 1;
	chunkRect.right  = calculateChunkX (bottomRight) + 1;

	CProgressDlg* dlg = 0;
	
	if (showProgress)
	{
		dlg = new CProgressDlg ();
		IGNORE_RETURN (dlg->Create ());
		dlg->SetRange (0, chunkRect.Height ());
		IGNORE_RETURN (dlg->SetStep (1));
		IGNORE_RETURN (dlg->SetPos (0));
	}

	//-- update list of local water tables
	{
		TerrainGeneratorHelper::buildLocalWaterTableList (document->getTerrainGenerator (), m_localWaterTableList);
		TerrainGeneratorHelper::buildAffectorRibbonList (document->getTerrainGenerator (), m_affectorRibbonList);
	}

	bool quit = false;
	int x;
	int z;
#if MAPVIEW_REVERSED
	for (z = chunkRect.bottom; !quit && z >= chunkRect.top; z--)
#else
	for (z = chunkRect.top; !quit && z <= chunkRect.bottom; z++)
#endif
	{
		for (x = chunkRect.left; !quit && x <= chunkRect.right; x++)
		{
			Chunk* chunk = findChunk (x, z);
			
			if (chunk && chunk->isDirty ())
				removeChunk (x, z);

			//-- make sure indices are valid
			if (!areValidChunkIndices (x, z))
				continue;

			//-- see if the chunk already exists
			if (hasChunk (x, z))
				continue;

// JU_TODO: display road affector effect
#if 1
			// orig - doesn't display roads
			chunk = createChunk (x, z, true, false);
#else
			// will fail if the road affector doesn't have height data yet - todo : force an affector update
			chunk = createChunk (x, z, false, false);

#endif
// JU_TODO: end display

			NOT_NULL (chunk);
			m_chunkMap->insert (ChunkMap::value_type (computeChunkMapKey (x, z), chunk));

			const_cast<MapView*> (mapView)->Invalidate (false);
		}

		if (showProgress)
		{
			NOT_NULL (dlg);
			IGNORE_RETURN (dlg->StepIt ());

			if (dlg->CheckCancelButton ())
				quit = true;
		}
	}

	if (showProgress)
	{
		delete dlg;
		dlg = 0;
	}

	//-- find minimum and maximum
	real minimum        = REAL_MAX;
	real maximum        = -REAL_MAX;
	real sum            = 0;
	int  numberOfChunks = 0;

	for (ChunkMap::iterator i = m_chunkMap->begin (); i != m_chunkMap->end (); ++i)
	{
		const Chunk* const chunk = i->second;

		if (!chunk->isDirty ())
		{
			minimum = min (chunk->getTime (), minimum);
			maximum = max (chunk->getTime (), maximum);
			
			sum += chunk->getTime ();
			++numberOfChunks;
		}
	}

	const real average = (numberOfChunks > 0) ? (sum / static_cast<real> (numberOfChunks)) : 0.f;

	document->setLastChunkGenerationTimes (sum, average, minimum, maximum);
}

//-------------------------------------------------------------------

#define NEW_DRAWING_SCHEME 0

void EditorTerrain::draw () const
{
	NOT_NULL (document);
	NOT_NULL (mapView);

	//-- get window size.
	CRect rect;
	mapView->GetClientRect (&rect);

	//-- find out which chunks are visible and draw them
	CRect chunkRect;

	{
		const Vector2d topLeft     = mapView->convertScreenToWorld(rect.TopLeft ());
		const Vector2d bottomRight = mapView->convertScreenToWorld(rect.BottomRight ());

#if MAPVIEW_REVERSED
		chunkRect.top    = calculateChunkZ (topLeft) + 1;
		chunkRect.bottom = calculateChunkZ (bottomRight) - 1;
		std::swap (chunkRect.top, chunkRect.bottom);
#else
		chunkRect.top    = calculateChunkZ (topLeft) - 1;
		chunkRect.bottom = calculateChunkZ (bottomRight) + 1;
#endif
		chunkRect.left   = calculateChunkX (topLeft) - 1;
		chunkRect.right  = calculateChunkX (bottomRight) + 1;
	}
	
	Graphics::setStaticShader (ShaderTemplateList::get2dVertexColorStaticShader ());

	const real chunkWidthInMeters = chunkWidthInMeters_w / mapView->getZoomLevel ();

	bool quit = false;
	int x;
	int z;
#if MAPVIEW_REVERSED
	for (z = chunkRect.bottom; !quit && z >= chunkRect.top; z--)
#else
	for (z = chunkRect.top; !quit && z <= chunkRect.bottom; z++)
#endif
	{
		for (x = chunkRect.left; !quit && x <= chunkRect.right; x++)
		{
			const Vector2d topLeft (x * chunkWidthInMeters, z * chunkWidthInMeters);
			const Vector2d bottomRight (topLeft.x + chunkWidthInMeters, topLeft.y + chunkWidthInMeters);

			const CPoint topLeftScreen     = mapView->convertWorldToScreen (topLeft);
			const CPoint bottomRightScreen = mapView->convertWorldToScreen (bottomRight);

			VertexBufferFormat format;
			format.setPosition();
			format.setTransformed();
			format.setColor0();
			format.setNumberOfTextureCoordinateSets(1);
			format.setTextureCoordinateSetDimension(0, 2);
				
			DynamicVertexBuffer vertexBuffer (format);

			const VectorArgb color (CONST_REAL (0.5), CONST_REAL (0), CONST_REAL (0), CONST_REAL (0));

			vertexBuffer.lock(4);


				VertexBufferWriteIterator v = vertexBuffer.begin();

#if MAPVIEW_REVERSED
				v.setPosition (Vector (static_cast<real> (topLeftScreen.x),     static_cast<real> (bottomRightScreen.y), CONST_REAL (0)));
				v.setOoz  (CONST_REAL (1));
				v.setColor0 ( color);
				v.setTextureCoordinates  (0, CONST_REAL (0), CONST_REAL (1));
				++v;

				v.setPosition (Vector (static_cast<real> (bottomRightScreen.x), static_cast<real> (bottomRightScreen.y), CONST_REAL (0)));
				v.setOoz  (CONST_REAL (1));
				v.setColor0 (color);
				v.setTextureCoordinates  (0, CONST_REAL (1), CONST_REAL (1));
				++v;

				v.setPosition (Vector (static_cast<real> (bottomRightScreen.x), static_cast<real> (topLeftScreen.y),     CONST_REAL (0)));
				v.setOoz  (CONST_REAL (1));
				v.setColor0 (color);
				v.setTextureCoordinates  (0, CONST_REAL (1), CONST_REAL (0));
				++v;

				v.setPosition (Vector (static_cast<real> (topLeftScreen.x),     static_cast<real> (topLeftScreen.y),     CONST_REAL (0)));
				v.setOoz  (CONST_REAL (1));
				v.setColor0 (color);
				v.setTextureCoordinates  (0, CONST_REAL (0), CONST_REAL (0));
#else
				v.setPosition (Vector (static_cast<real> (topLeftScreen.x),     static_cast<real> (topLeftScreen.y),     CONST_REAL (0)));
				v.setOoz  (CONST_REAL (1));
				v.setColor0 (color);
				v.setTextureCoordinates  (0, CONST_REAL (0), CONST_REAL (0));
				++v;

				v.setPosition (Vector (static_cast<real> (bottomRightScreen.x), static_cast<real> (topLeftScreen.y),     CONST_REAL (0)));
				v.setOoz  (CONST_REAL (1));
				v.setColor0 (color);
				v.setTextureCoordinates  (0, CONST_REAL (1), CONST_REAL (0));
				++v;

				v.setPosition (2, Vector (static_cast<real> (bottomRightScreen.x), static_cast<real> (bottomRightScreen.y), CONST_REAL (0)));
				v.setOoz  (i, CONST_REAL (1));
				v.setColor0 (i, color);
				v.setTextureCoordinates  (2, 0, CONST_REAL (1), CONST_REAL (1));
				++v;

				v.setPosition (3, Vector (static_cast<real> (topLeftScreen.x),     static_cast<real> (bottomRightScreen.y), CONST_REAL (0)));
				v.setOoz  (i, CONST_REAL (1));
				v.setColor0 (i, color);
				v.setTextureCoordinates  (3, 0, CONST_REAL (0), CONST_REAL (1));
#endif

			vertexBuffer.unlock();

			Graphics::setStaticShader (invalidShader->prepareToView());
			Graphics::setVertexBuffer (vertexBuffer);
			Graphics::drawTriangleFan ();
		}
	}

	for (ChunkMap::iterator i = m_chunkMap->begin (); i != m_chunkMap->end (); ++i)
	{
		const Chunk* const chunk = i->second;

#if NEW_DRAWING_SCHEME
		const Vector2d topLeft (chunk->getChunkX () * chunk->getChunkWidthInMeters(), chunk->getChunkZ () * chunk->getChunkWidthInMeters());
		const Vector2d bottomRight (topLeft.x + chunk->getChunkWidthInMeters(), topLeft.y + chunk->getChunkWidthInMeters());
		const CPoint topLeftScreen     = mapView->convertWorldToScreen (topLeft);
		const CPoint bottomRightScreen = mapView->convertWorldToScreen (bottomRight);
		if (  topLeftScreen.x    <rect.right
			&& bottomRightScreen.y<rect.bottom
			&& bottomRightScreen.x>0
			&& topLeftScreen.y    >0
			)
#else
		if (chunkRect.PtInRect (CPoint (chunk->getChunkX (), chunk->getChunkZ ())))
#endif
		{
#if !NEW_DRAWING_SCHEME
			const Vector2d topLeft (chunk->getChunkX () * chunkWidthInMeters, chunk->getChunkZ () * chunkWidthInMeters);
			const Vector2d bottomRight (topLeft.x + chunkWidthInMeters, topLeft.y + chunkWidthInMeters);

			const CPoint topLeftScreen     = mapView->convertWorldToScreen (topLeft);
			const CPoint bottomRightScreen = mapView->convertWorldToScreen (bottomRight);
#endif

			VertexBufferFormat format;
			format.setPosition ();
			format.setTransformed ();
			format.setColor0 ();
			format.setNumberOfTextureCoordinateSets (1);
			format.setTextureCoordinateSetDimension (0, 2);
			DynamicVertexBuffer vertexBuffer (format);

			const VectorArgb color (CONST_REAL (0.5), CONST_REAL (0), CONST_REAL (0), CONST_REAL (0));

			vertexBuffer.lock(4);

				VertexBufferWriteIterator v = vertexBuffer.begin();

#if MAPVIEW_REVERSED
				v.setPosition (Vector (static_cast<real> (topLeftScreen.x),     static_cast<real> (bottomRightScreen.y), CONST_REAL (0)));
				v.setOoz  (CONST_REAL (1));
				v.setColor0 (color);
				v.setTextureCoordinates  (0, CONST_REAL (0), CONST_REAL (1));
				++v;

				v.setPosition (Vector (static_cast<real> (bottomRightScreen.x), static_cast<real> (bottomRightScreen.y), CONST_REAL (0)));
				v.setOoz  (CONST_REAL (1));
				v.setColor0 (color);
				v.setTextureCoordinates  (0, CONST_REAL (1), CONST_REAL (1));
				++v;

				v.setPosition (Vector (static_cast<real> (bottomRightScreen.x), static_cast<real> (topLeftScreen.y),     CONST_REAL (0)));
				v.setOoz  (CONST_REAL (1));
				v.setColor0 (color);
				v.setTextureCoordinates  (0, CONST_REAL (1), CONST_REAL (0));
				++v;

				v.setPosition (Vector (static_cast<real> (topLeftScreen.x),     static_cast<real> (topLeftScreen.y),     CONST_REAL (0)));
				v.setOoz  (CONST_REAL (1));
				v.setColor0 (color);
				v.setTextureCoordinates  (0, CONST_REAL (0), CONST_REAL (0));
#else
				v.setPosition (Vector (static_cast<real> (topLeftScreen.x),     static_cast<real> (topLeftScreen.y),     CONST_REAL (0)));
				v.setOoz  (CONST_REAL (1));
				v.setColor0 (color);
				v.setTextureCoordinates  (0, CONST_REAL (0), CONST_REAL (0));
				++v;

				v.setPosition (Vector (static_cast<real> (bottomRightScreen.x), static_cast<real> (topLeftScreen.y),     CONST_REAL (0)));
				v.setOoz  (CONST_REAL (1));
				v.setColor0 (color);
				v.setTextureCoordinates  (0, CONST_REAL (1), CONST_REAL (0));
				++v;

				v.setPosition (Vector (static_cast<real> (bottomRightScreen.x), static_cast<real> (bottomRightScreen.y), CONST_REAL (0)));
				v.setOoz  (CONST_REAL (1));
				v.setColor0 (color);
				v.setTextureCoordinates  (0, CONST_REAL (1), CONST_REAL (1));
				++v;

				v.setPosition (Vector (static_cast<real> (topLeftScreen.x),     static_cast<real> (bottomRightScreen.y), CONST_REAL (0)));
				v.setOoz  (CONST_REAL (1));
				v.setColor0 (color);
				v.setTextureCoordinates  (0, CONST_REAL (0), CONST_REAL (1));
#endif

			vertexBuffer.unlock();

			chunk->render (vertexBuffer);
		}
	}
}

//-------------------------------------------------------------------

bool EditorTerrain::areValidChunkIndices (const int x, const int z) const
{
	const int maximumNumberOfChunksAlongSide = static_cast<int> ((document->getMapWidthInMeters () / (CONST_REAL (2) * chunkWidthInMeters_w)) * mapView->getZoomLevel ());

	return (x >= -maximumNumberOfChunksAlongSide && 
			z >= -maximumNumberOfChunksAlongSide && 
			x < maximumNumberOfChunksAlongSide && 
			z < maximumNumberOfChunksAlongSide);
}

//-------------------------------------------------------------------

int EditorTerrain::calculateChunkX (const Vector2d& position) const
{
	NOT_NULL (document);

	const int chunkX = static_cast<int> ((position.x >= 0.f) ? floorf (position.x / (chunkWidthInMeters_w / mapView->getZoomLevel ())) : ceilf (position.x / (chunkWidthInMeters_w / mapView->getZoomLevel ())));

	//-- zero is not a valid chunk index
	return (position.x < CONST_REAL (0)) ? chunkX - 1 : chunkX;
}

//-------------------------------------------------------------------

int EditorTerrain::calculateChunkZ (const Vector2d& position) const
{
	NOT_NULL (document);

	const int chunkZ = static_cast<int> ((position.y >= 0.f) ? floorf (position.y / (chunkWidthInMeters_w / mapView->getZoomLevel ())) : ceilf (position.y / (chunkWidthInMeters_w / mapView->getZoomLevel ())));
   
   	//-- zero is not a valid chunk index
   	return (position.y < CONST_REAL (0)) ? chunkZ - 1 : chunkZ;
}

//-------------------------------------------------------------------

EditorTerrain::Chunk* EditorTerrain::findChunk (const int x, const int z)
{
	ChunkMap::iterator i = m_chunkMap->find (computeChunkMapKey (x, z));

	return i != m_chunkMap->end () ? i->second : 0;
}

//-------------------------------------------------------------------

EditorTerrain::Chunk* EditorTerrain::findChunk (const Vector2d& position)
{
	//-- where is this position in the world in reference to the terrain?
	const int chunkX = calculateChunkX (position);
	const int chunkZ = calculateChunkZ (position);

	return findChunk (chunkX, chunkZ);
}

//-------------------------------------------------------------------

const EditorTerrain::Chunk* EditorTerrain::findChunk (const int x, const int z) const
{
	ChunkMap::iterator i = m_chunkMap->find (computeChunkMapKey (x, z));

	return i != m_chunkMap->end () ? i->second : 0;
}

//-------------------------------------------------------------------

const EditorTerrain::Chunk* EditorTerrain::findChunk (const Vector2d& position) const
{
	//-- where is this position in the world in reference to the terrain?
	const int chunkX = calculateChunkX (position);
	const int chunkZ = calculateChunkZ (position);

	return findChunk (chunkX, chunkZ);
}

//-------------------------------------------------------------------

bool EditorTerrain::hasChunk (const int x, const int z) const
{
	return findChunk (x, z) != 0;
}

//-------------------------------------------------------------------

bool EditorTerrain::hasChunk (const Vector2d& position) const
{
	//-- where is this position in the world in reference to the terrain?
	const int chunkX = calculateChunkX (position);
	const int chunkZ = calculateChunkZ (position);

	return hasChunk (chunkX, chunkZ);
}

//-------------------------------------------------------------------

EditorTerrain::Chunk* EditorTerrain::createChunk (const int x, const int z, const bool noRiversOrRoads, const bool noTexture)
{
	NOT_NULL (document);
	NOT_NULL (mapView);

	//-- chunk does not exist -- it needs to be created. find out what map data i'll need to ask the generator for
	const real   zoom                 = mapView->getZoomLevel ();

	const real   distanceBetweenPoles = distanceBetweenPoles_w / zoom;
	const real   chunkWidthInMeters   = chunkWidthInMeters_w / zoom;

	const bool legacyMode = true;

	const Vector start(
		static_cast<real>(x)*chunkWidthInMeters - static_cast<float>(originOffset)*distanceBetweenPoles, 
		CONST_REAL (0), 
		static_cast<real>(z)*chunkWidthInMeters - static_cast<float>(originOffset)*distanceBetweenPoles
	);


	//-- setup scratch buffer

	createChunkBuffer.allocate (numberOfPoles);

	//-- create chunk
	Chunk* chunk = new Chunk (this, x, z, chunkWidthInMeters);

	//-- ask the generator to fill out this area
	TerrainGenerator::GeneratorChunkData generatorChunkData(legacyMode);

	generatorChunkData.heightMap            = &createChunkBuffer.heightMap;
	generatorChunkData.colorMap             = &createChunkBuffer.colorMap;
	generatorChunkData.shaderMap            = &createChunkBuffer.shaderMap;
	generatorChunkData.floraStaticCollidableMap    = &createChunkBuffer.floraStaticCollidableMap;
	generatorChunkData.floraStaticNonCollidableMap = &createChunkBuffer.floraStaticNonCollidableMap;
	generatorChunkData.floraDynamicNearMap  = &createChunkBuffer.floraDynamicNearMap;
	generatorChunkData.floraDynamicFarMap   = &createChunkBuffer.floraDynamicFarMap;
	generatorChunkData.vertexPositionMap    = &createChunkBuffer.vertexPositionMap;
	generatorChunkData.vertexNormalMap      = &createChunkBuffer.vertexNormalMap;
	generatorChunkData.environmentMap       = &createChunkBuffer.environmentMap;
	generatorChunkData.excludeMap           = &createChunkBuffer.excludeMap;
	generatorChunkData.passableMap          = &createChunkBuffer.passableMap;
	generatorChunkData.start                = start;
	generatorChunkData.originOffset         = originOffset;
	generatorChunkData.numberOfPoles        = numberOfPoles;
	generatorChunkData.upperPad             = upperPad;
	generatorChunkData.distanceBetweenPoles = distanceBetweenPoles;
	generatorChunkData.shaderGroup          = &document->getTerrainGenerator ()->getShaderGroup ();
	generatorChunkData.floraGroup           = &document->getTerrainGenerator ()->getFloraGroup ();
	generatorChunkData.radialGroup          = &document->getTerrainGenerator ()->getRadialGroup ();
	generatorChunkData.environmentGroup     = &document->getTerrainGenerator ()->getEnvironmentGroup ();
	generatorChunkData.fractalGroup         = &document->getTerrainGenerator ()->getFractalGroup ();
	generatorChunkData.bitmapGroup          = &document->getTerrainGenerator ()->getBitmapGroup ();

		PerformanceTimer timer;
		timer.start ();

	//-- disable roads/rivers
	if (noRiversOrRoads)
	{
		AffectorRoad::disable ();
		AffectorRiver::disable ();
	}

	TerrainGenerator *generator = document->getTerrainGenerator ();
	generator->setMapsToSample(unsigned(TGM_ALL));
	generator->generateChunk(generatorChunkData);

	//-- enable roads/rivers
	if (noRiversOrRoads)
	{
		AffectorRoad::enable ();
		AffectorRiver::enable ();
	}

	//-- create the chunk using the data the generator created
	chunk->create (&generatorChunkData, noTexture);

	chunk->setShaderTextureFactor (PackedRgb::solidWhite.asUint32 (), 0);

		timer.stop ();
		chunk->setTime (timer.getElapsedTime ());

	return chunk;
}  

//-------------------------------------------------------------------

EditorTerrain::Chunk* EditorTerrain::createChunk (const Vector2d& position, const bool noRiversOrRoads, const bool noTexture)
{
	//-- where is this position in the world in reference to the terrain?
	const int chunkX = calculateChunkX (position);
	const int chunkZ = calculateChunkZ (position);

	//-- make sure indices are valid
	if (!areValidChunkIndices (chunkX, chunkZ))
		return 0;

	//-- see if the chunk already exists
	if (hasChunk (chunkX, chunkZ))
		return 0;

	return createChunk (chunkX, chunkZ, noRiversOrRoads, noTexture);
}

//-------------------------------------------------------------------

void EditorTerrain::removeChunk (const int x, const int z)
{
	ChunkMap::iterator i = m_chunkMap->find (computeChunkMapKey (x, z));
	if (i != m_chunkMap->end ())
	{
		delete i->second;

		m_chunkMap->erase (i);
	}
}

//-------------------------------------------------------------------

void EditorTerrain::removeChunk (const Vector2d& position)
{
	//-- where is this position in the world in reference to the terrain?
	const int chunkX = calculateChunkX (position);
	const int chunkZ = calculateChunkZ (position);

	removeChunk (chunkX, chunkZ);
}

//-------------------------------------------------------------------

void EditorTerrain::markDirty ()
{
	for (ChunkMap::iterator i = m_chunkMap->begin (); i != m_chunkMap->end (); ++i)
		i->second->setDirty (true);
}

//-------------------------------------------------------------------

bool EditorTerrain::getDataAt (const Vector2d& position, EditorTerrain::Data& data, bool heightOnly) const
{
	const Chunk* chunk = findChunk (position);
	
	return chunk ? chunk->getDataAt (position, data, heightOnly) : 0;
}

//-------------------------------------------------------------------

void EditorTerrain::showProfile (bool newShowProfile)
{
	//-- clear out the profile data
	for (ChunkMap::iterator i = m_chunkMap->begin (); i != m_chunkMap->end (); ++i)
		i->second->setShaderTextureFactor (PackedRgb::solidWhite.asUint32 (), 0);

	const real average = document->getLastAverageChunkGenerationTime ();
	const real minimum = document->getLastMinimumChunkGenerationTime ();
	const real maximum = document->getLastMaximumChunkGenerationTime ();

	//-- 
	if (newShowProfile)
	{
		//-- set texture factors based on relative times
		for (ChunkMap::iterator i = m_chunkMap->begin (); i != m_chunkMap->end (); ++i)
		{
			Chunk* const chunk = i->second;

			PackedRgb color = PackedRgb::solidBlack;

#if 0
			color.r = static_cast<uint8> (255.f * ((chunk->getTime () - minimum) / (maximum - minimum)));
#endif

#if 1
			UNREF (minimum);

			if (chunk->getTime () > average)
			{
				color.r = static_cast<uint8> (255.f * ((chunk->getTime () - average) / (maximum - average)));
			}
#endif

#if 0
			if (chunk->getTime () > average)
			{
				if ((average + average - minimum) < maximum)
				{
					//-- lerp average to average + average - min
					if (chunk->getTime () < average + (average - minimum))
					{
						color.r = static_cast<uint8> (255.f * ((chunk->getTime () - average) / (average + average - minimum)));
					}
					else
					{
						//-- is red!
						color.r = 255;
					}
				}
				else
				{
					//-- lerp average to max
					color.r = static_cast<uint8> (255.f * ((chunk->getTime () - average) / (maximum - average)));
				}
			}
#endif

			chunk->setShaderTextureFactor (PackedRgb::solidGray.asUint32 (), color.asUint32 ());
		}
	}
}

//-------------------------------------------------------------------

void EditorTerrain::Chunk::setShaderTextureFactor (uint32 newDim, uint32 newMain)
{
	if (shader)
	{
		static_cast<StaticShader*> (shader)->setTextureFactor (TAG (M,A,I,N), newMain);
		static_cast<StaticShader*> (shader)->setTextureFactor (TAG3 (D,I,M), newDim);
	}
}

//-------------------------------------------------------------------

const Rectangle2d& EditorTerrain::Chunk::getExtent () const
{
	return extent;
}

//-------------------------------------------------------------------

void EditorTerrain::clearHeightDataChunkMap ()
{
	for (ChunkMap::iterator i = m_heightDataChunkMap->begin (); i != m_heightDataChunkMap->end (); ++i)
		delete i->second;

	m_heightDataChunkMap->clear ();
}

//-------------------------------------------------------------------

const EditorTerrain::Chunk* EditorTerrain::findHeightDataChunk (int x, int z) const
{
	ChunkMap::iterator i = m_heightDataChunkMap->find (computeChunkMapKey (x, z));

	return i != m_heightDataChunkMap->end () ? i->second : 0;
}

//-------------------------------------------------------------------

const EditorTerrain::Chunk* EditorTerrain::findHeightDataChunk (const Vector2d& position) const
{
	//-- where is this position in the world in reference to the terrain?
	const int chunkX = calculateChunkX (position);
	const int chunkZ = calculateChunkZ (position);

	return findHeightDataChunk (chunkX, chunkZ);
}

//-------------------------------------------------------------------

bool EditorTerrain::buildHeightData (const ArrayList<MetaData>& metaDataList)
{
	PerformanceTimer performanceTimer;
	performanceTimer.start ();

	CProgressDlg* dlg = new CProgressDlg (CG_IDS_PROGRESS_CAPTION2);
	IGNORE_RETURN (dlg->Create ());
	dlg->SetRange (0, metaDataList.getNumberOfElements ());
	IGNORE_RETURN (dlg->SetStep (1));
	IGNORE_RETURN (dlg->SetPos (0));

	clearHeightDataChunkMap ();

	const float edgeLength = numberOfPoles * distanceBetweenPoles_w;

	bool quit = false;

	int i;
	for (i = 0; i < metaDataList.getNumberOfElements (); ++i)
	{
		const MetaData& metaData = metaDataList [i];

		CString tmp;

		if (metaDataList.getNumberOfElements () > 1)
			tmp.Format ("%i of %i: ", i + 1, metaDataList.getNumberOfElements ());

		tmp += metaData.name;
		dlg->setInfo (tmp);

		const Rectangle2d extent = metaData.extent;

		const Vector2d topLeft (extent.x0, extent.y1);
		const Vector2d bottomRight (extent.x1, extent.y0);

		CRect chunkRect;
#if MAPVIEW_REVERSED
		chunkRect.top    = calculateChunkZ (topLeft) + 1;
		chunkRect.bottom = calculateChunkZ (bottomRight) - 1;
		std::swap (chunkRect.top, chunkRect.bottom);
#else
		chunkRect.top    = calculateChunkZ (topLeft) - 1;
		chunkRect.bottom = calculateChunkZ (bottomRight) + 1;
#endif
		chunkRect.left   = calculateChunkX (topLeft) - 1;
		chunkRect.right  = calculateChunkX (bottomRight) + 1;

		dlg->SetRange (0, abs (chunkRect.bottom - chunkRect.top));
		IGNORE_RETURN (dlg->SetStep (1));
		IGNORE_RETURN (dlg->SetPos (0));

		int x;
		int z;
#if MAPVIEW_REVERSED
		for (z = chunkRect.bottom; !quit && z >= chunkRect.top; z--)
#else
		for (z = chunkRect.top; !quit && z <= chunkRect.bottom; z++)
#endif
		{
			for (x = chunkRect.left; !quit && x <= chunkRect.right; x++)
			{
				const Vector start (static_cast<real> (x) * chunkWidthInMeters_w - distanceBetweenPoles_w, CONST_REAL (0), static_cast<real> (z) * chunkWidthInMeters_w - distanceBetweenPoles_w);

				const Vector extents3d [4] =
				{
					Vector (start.x, 0.f, start.z), 
					Vector (start.x + edgeLength, 0.f, start.z),
					Vector (start.x + edgeLength, 0.f, start.z + edgeLength),
					Vector (start.x, 0.f, start.z + edgeLength)
				};

				//-- see if the chunk extents are close enough to the plane
				bool shouldBuild = false;

				int j;
				for (j = 0; j < metaData.planeList.getNumberOfElements (); ++j)
				{
					int k;
					for (k = 0; k < 4; ++k)
					{
						if (metaData.planeList [j].computeDistanceTo (extents3d [k]) < fabsf (metaData.width))
						{
							shouldBuild = true;
							break;
						}
					}
				}

				//-- see if the chunk exists
				if (!shouldBuild || findHeightDataChunk (x, z))
					continue;

				//-- chunk doesn't exist, so build it
				Chunk* chunk = createChunk (x, z, true, true);
				NOT_NULL (chunk);
				m_heightDataChunkMap->insert (ChunkMap::value_type (computeChunkMapKey (x, z), chunk));

				quit = dlg->CheckCancelButton () == TRUE;
			}

			IGNORE_RETURN (dlg->StepIt ());
		}
	}

	delete dlg;
	dlg = 0;

	if (quit)
	{
		MessageBox (mapView->m_hWnd, "You cancelled building the rivers/roads meta-data. Your terrain file may not produce desired results.", "Warning", MB_ICONWARNING);

		return false;
	}
	else
	{
		performanceTimer.stop ();

#ifdef _DEBUG
		CString s;
		s.Format ("Build time = %1.2f seconds", performanceTimer.getElapsedTime ());

		MessageBox (mapView->m_hWnd, s, "Information", MB_ICONINFORMATION);
#endif
	}

	return true;
}

//-------------------------------------------------------------------

real EditorTerrain::getHeightDataHeightAt (const Vector2d& position) const
{
	const Chunk* chunk = findHeightDataChunk (position);
	NOT_NULL (chunk);
	if (chunk)
	{
		Data data;
		if (chunk->getDataAt (position, data, true))
			return data.height;
	}
	DEBUG_FATAL (true, ("couldn't find height"));

	return 0;
}

//-------------------------------------------------------------------

void EditorTerrain::setResolutionType (ResolutionType resolutionType)
{
	m_resolutionType = resolutionType;

	switch (m_resolutionType)
	{
	case RT_low:
		{
			chunkWidthInMeters_w = 64.f;
			distanceBetweenPoles_w = 4.f;
		}
		break;

	case RT_medium:
		{
			chunkWidthInMeters_w = 32.f;
			distanceBetweenPoles_w = 2.f;
		}
		break;

	case RT_high:
		{
			chunkWidthInMeters_w = 16.f;
			distanceBetweenPoles_w = 1.f;
		}
		break;
	}

	clearChunks ();
}

//-------------------------------------------------------------------

EditorTerrain::ResolutionType EditorTerrain::getResolutionType () const
{
	return m_resolutionType;
}

//-------------------------------------------------------------------

void EditorTerrain::clearChunks ()
{
	{
		for (ChunkMap::iterator i = m_chunkMap->begin (); i != m_chunkMap->end (); ++i)
			delete i->second;

		m_chunkMap->clear ();
	}

	{
		for (ChunkMap::iterator i = m_heightDataChunkMap->begin (); i != m_heightDataChunkMap->end (); ++i)
			delete i->second;

		m_heightDataChunkMap->clear ();
	}
}

//-------------------------------------------------------------------

void EditorTerrain::createBakedChunk (const int x, const int z, float& maximumHeight, float& chunkHeight, bool& hasWater)
{
	const float bakedDistanceBetweenPoles = 4.f;
	const float bakedChunkWidthInMeters = 8.f;

	const int bakedNumberOfPoles = 3;

	const int bakedOriginOffset = 1;
	const int bakedUpperPad = 1;

	const int bakedActualNumberOfPoles = bakedNumberOfPoles + bakedOriginOffset + bakedUpperPad;

	//const int bakedActualNumberOfPoles = bakedNumberOfPoles + static_cast<int> (bakedChunkWidthInMeters / bakedDistanceBetweenPoles);

	const Vector start(
		static_cast<float>(x)*bakedChunkWidthInMeters - static_cast<float>(bakedOriginOffset)*bakedDistanceBetweenPoles, 
		0.f, 
		static_cast<float>(z)*bakedChunkWidthInMeters - static_cast<float>(bakedOriginOffset)*bakedDistanceBetweenPoles
	);

	//-- setup scratch buffer
	bakedCreateChunkBuffer.allocate (bakedActualNumberOfPoles);

	//-- ask the generator to fill out this area
	const bool legacyMode = false;
	TerrainGenerator::GeneratorChunkData generatorChunkData(legacyMode);

	generatorChunkData.heightMap                   = &bakedCreateChunkBuffer.heightMap;
	generatorChunkData.colorMap                    = &bakedCreateChunkBuffer.colorMap;
	generatorChunkData.shaderMap                   = &bakedCreateChunkBuffer.shaderMap;
	generatorChunkData.floraStaticCollidableMap    = &bakedCreateChunkBuffer.floraStaticCollidableMap;
	generatorChunkData.floraStaticNonCollidableMap = &bakedCreateChunkBuffer.floraStaticNonCollidableMap;
	generatorChunkData.floraDynamicNearMap         = &bakedCreateChunkBuffer.floraDynamicNearMap;
	generatorChunkData.floraDynamicFarMap          = &bakedCreateChunkBuffer.floraDynamicFarMap;
	generatorChunkData.vertexPositionMap           = &bakedCreateChunkBuffer.vertexPositionMap;
	generatorChunkData.vertexNormalMap             = &bakedCreateChunkBuffer.vertexNormalMap;
	generatorChunkData.environmentMap              = &bakedCreateChunkBuffer.environmentMap;
	generatorChunkData.excludeMap                  = &bakedCreateChunkBuffer.excludeMap;
	generatorChunkData.passableMap                 = &bakedCreateChunkBuffer.passableMap;
	generatorChunkData.start                       = start;
	generatorChunkData.originOffset                = bakedOriginOffset;
	generatorChunkData.numberOfPoles               = bakedActualNumberOfPoles;
	generatorChunkData.upperPad                    = bakedUpperPad;
	generatorChunkData.distanceBetweenPoles        = bakedDistanceBetweenPoles;
	generatorChunkData.shaderGroup                 = &document->getTerrainGenerator ()->getShaderGroup ();
	generatorChunkData.floraGroup                  = &document->getTerrainGenerator ()->getFloraGroup ();
	generatorChunkData.radialGroup                 = &document->getTerrainGenerator ()->getRadialGroup ();
	generatorChunkData.environmentGroup            = &document->getTerrainGenerator ()->getEnvironmentGroup ();
	generatorChunkData.fractalGroup                = &document->getTerrainGenerator ()->getFractalGroup ();
	generatorChunkData.bitmapGroup                 = &document->getTerrainGenerator ()->getBitmapGroup ();

	TerrainGenerator *generator = document->getTerrainGenerator ();

	generator->setMapsToSample(TGM_height);
	generator->generateChunk(generatorChunkData);

	//
	float minimumHeight = FLT_MAX;
	maximumHeight = -FLT_MAX;
	hasWater = false;

	{
		for (int j = 0; j < bakedNumberOfPoles; ++j)
		{
			const float worldZ = float(z)*bakedChunkWidthInMeters + float(j)*bakedDistanceBetweenPoles;

			for (int i = 0; i < bakedNumberOfPoles; ++i)
			{
				const float worldY = bakedCreateChunkBuffer.heightMap.getData(i+bakedOriginOffset, j+bakedOriginOffset);

				if (worldY<minimumHeight)
				{
					minimumHeight=worldY;
				}
				if (worldY>maximumHeight)
				{
					maximumHeight=worldY;
				}

				if (!hasWater)
				{
					const float worldX = float(x)*bakedChunkWidthInMeters + float(i)*bakedDistanceBetweenPoles;
					if (isBelowWater(Vector(worldX, worldY, worldZ)))
					{
						hasWater = true;
					}
				}
			}
		}
	}

	chunkHeight = maximumHeight - minimumHeight;
}

//-------------------------------------------------------------------

void EditorTerrain::bakeTerrain ()
{
#ifdef _DEBUG
	VTune::resume ();
#endif

	//-- build local water table list
	TerrainGeneratorHelper::buildLocalWaterTableList (document->getTerrainGenerator (), m_localWaterTableList);
	TerrainGeneratorHelper::buildAffectorRibbonList (document->getTerrainGenerator (), m_affectorRibbonList);

	//-- disable rivers and roads
	AffectorRoad::disable ();
	AffectorRiver::disable ();

	{
		const float mapWidthInMeters = document->getMapWidthInMeters ();
		const float chunkWidthInMeters = document->getChunkWidthInMeters ();
		BakedTerrain* const bakedTerrain = document->getBakedTerrain ();
		bakedTerrain->setMap (mapWidthInMeters, chunkWidthInMeters);
		
		PerformanceTimer performanceTimer;
		performanceTimer.start ();

		const int numberOfChunks = static_cast<int> (mapWidthInMeters / chunkWidthInMeters);
		const int chunkStart = numberOfChunks / 2;

		CProgressDlg* const dlg = new CProgressDlg (CG_IDS_PROGRESS_CAPTION2);
		IGNORE_RETURN (dlg->Create ());

		dlg->SetRange (0, numberOfChunks);

		IGNORE_RETURN (dlg->SetStep (1));
		IGNORE_RETURN (dlg->SetPos (0));

		bool quit = false;

		int cx;
		int cz;
		for (cz = -chunkStart; !quit && cz < chunkStart; ++cz)
		{
			for (cx = -chunkStart; !quit && cx < chunkStart; ++cx)
			{
				float maximumHeight = 0.f;
				float chunkHeight = 0.f;
				bool hasWater = false;
				createBakedChunk (cx, cz, maximumHeight, chunkHeight, hasWater);
				bakedTerrain->setWater (cx, cz, hasWater);
				bakedTerrain->setSlope (cx, cz, chunkHeight > 4.f);
			}

			if (dlg->CheckCancelButton ())
				quit = true;

			IGNORE_RETURN (dlg->StepIt ());
		}

		delete dlg;

		if (quit)
			MessageBox (mapView->m_hWnd, "You cancelled terrain baking.  Your terrain file may not produce desired results.", "Warning", MB_ICONWARNING);
		else
		{
			performanceTimer.stop ();

#ifdef _DEBUG
			CString s;
			s.Format ("Build time = %1.2f seconds", performanceTimer.getElapsedTime ());

			MessageBox (mapView->m_hWnd, s, "Information", MB_ICONINFORMATION);
#endif
		}
	}

	//-- enable rivers and roads
	AffectorRoad::enable ();
	AffectorRiver::enable ();

#ifdef _DEBUG
	VTune::pause ();
#endif
}

//===================================================================

bool EditorTerrain::isBelowWater (const Vector& position) const
{
	if (document->getUseGlobalWaterTable () && position.y < document->getGlobalWaterTableHeight ())
		return true;
	
	int i;
	for (i = 0; i < m_localWaterTableList.getNumberOfElements (); ++i)
	{
		if (m_localWaterTableList [i]->isWithin (position.x, position.z) > 0)
		{
			if (m_localWaterTableList [i]->getType () == TGBT_polygon)
			{
				BoundaryPolygon const * const boundaryPolygon = safe_cast<BoundaryPolygon const *> (m_localWaterTableList [i]);

				if (position.y < boundaryPolygon->getLocalWaterTableHeight ())
					return true;
			}

			if (m_localWaterTableList [i]->getType () == TGBT_rectangle)
			{
				BoundaryRectangle const * const boundaryRectangle = safe_cast<BoundaryRectangle const *> (m_localWaterTableList [i]);

				if (position.y < boundaryRectangle->getLocalWaterTableHeight ())
					return true;
			}
		}
	}

					
	const Vector pos(position.x,0.0f,position.z);
	const float heightToCheck = position.y;
	for(i = 0; i < m_affectorRibbonList.getNumberOfElements();++i)
	{
		const AffectorRibbon* affectorRibbon = m_affectorRibbonList[i];
		if(affectorRibbon->getExtent().isWithin(pos.x,pos.z))
		{
			// check endcap first
			if(affectorRibbon->getCapWidth() && affectorRibbon->getEndCapExtent().isWithin(pos.x,pos.z))
			{
				const ArrayList<Vector2d>& endCapPointList = affectorRibbon->getEndCapPointList();
				const Vector p0 = Vector(endCapPointList[0].x,0.0f,endCapPointList[0].y);
				const Vector p1 = Vector(endCapPointList[1].x,0.0f,endCapPointList[1].y);
				const Vector p2 = Vector(endCapPointList[2].x,0.0f,endCapPointList[2].y);
				const Vector p3 = Vector(endCapPointList[3].x,0.0f,endCapPointList[3].y);
				const Vector p4 = Vector(endCapPointList[4].x,0.0f,endCapPointList[4].y);
				const Vector p5 = Vector(endCapPointList[5].x,0.0f,endCapPointList[5].y);
				const Vector p6 = Vector(endCapPointList[6].x,0.0f,endCapPointList[6].y);
				const Vector p7 = Vector(endCapPointList[7].x,0.0f,endCapPointList[7].y);
				// check the triangles in 2 fans
				if(
					pos.inPolygon(p0,p1,p2)
					|| pos.inPolygon(p0,p2,p3)
					|| pos.inPolygon(p0,p3,p4)
					|| pos.inPolygon(p4,p5,p6)
					|| pos.inPolygon(p4,p6,p7)
					|| pos.inPolygon(p4,p7,p0)
				)
				{
					if(affectorRibbon->getHeightList()[0] > heightToCheck)
					{
						return true;
					}
				}
			}
		

			// now check the strip
			const Vector2d myPos(pos.x,pos.z);
			ArrayList<AffectorRibbon::Quad> ribbonQuadList;
			affectorRibbon->createQuadList (ribbonQuadList); 
			int j;
			for(j = 0; j < ribbonQuadList.size(); ++j)
			{
				const AffectorRibbon::Quad& ribbonQuad = (ribbonQuadList) [j];
				// check the 2 triangles on the quad
				const Vector p0 = Vector(ribbonQuad.points[0].x,0.0f,ribbonQuad.points[0].z);
				const Vector p1 = Vector(ribbonQuad.points[1].x,0.0f,ribbonQuad.points[1].z);
				const Vector p2 = Vector(ribbonQuad.points[2].x,0.0f,ribbonQuad.points[2].z);
				const Vector p3 = Vector(ribbonQuad.points[3].x,0.0f,ribbonQuad.points[3].z);

				if(pos.inPolygon(p0,p1,p2) || pos.inPolygon(p0,p2,p3))
				{
					// project the world point onto the line between the control points and interp for the height
					const Vector temp1 = (ribbonQuad.points[0] + ribbonQuad.points[1])/2.0f;
					const Vector temp2 = (ribbonQuad.points[2] + ribbonQuad.points[3])/2.0f;
					const Vector2d p1(temp1.x,temp1.z);
					const Vector2d p2(temp2.x,temp2.z);
					const float startHeight = temp1.y;
					const float endHeight = temp2.y;
					const float totalDistance = p1.magnitudeBetween(p2);						
											
					const Line2d line (p1,p2);
					const float distToLine = line.computeDistanceTo(myPos);
					const float realDistToPoint1 = p1.magnitudeBetween(myPos);
					const float realDistToPoint2 = p2.magnitudeBetween(myPos);
					const float projectedDistToPoint1 = sqrt(sqr(realDistToPoint1) - sqr(distToLine));
					const float projectedDistToPoint2 = sqrt(sqr(realDistToPoint2) - sqr(distToLine));

					float newHeight;
					if(totalDistance != 0.0f && projectedDistToPoint1 <= totalDistance && projectedDistToPoint2 <= totalDistance)
					{
						newHeight = startHeight + (projectedDistToPoint1/totalDistance) * (endHeight - startHeight);
					}
					else
					{
						newHeight = startHeight;	
					}

					if(newHeight > heightToCheck)
					{
						return true;
					}
				}
			}
		}
	}
					
	return false;
}

//===================================================================



