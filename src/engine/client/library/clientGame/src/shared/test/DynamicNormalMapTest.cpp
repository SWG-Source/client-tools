//===================================================================
//
// DynamicNormalMapTest.cpp
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/DynamicNormalMapTest.h"

#include "sharedDebug/DebugFlags.h"
#include "sharedMath/PackedRgb.h"
#include "sharedMath/VectorArgb.h"
#include "sharedRandom/Random.h"
#include "clientGraphics/DynamicVertexBuffer.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/Shader.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/StaticShader.h"
#include "clientGraphics/Texture.h"
#include "clientGraphics/TextureList.h"
#include "clientObject/ShaderAppearance.h"
#include "clientTerrain/SkyBoxAppearance.h"
#include "sharedObject/Object.h"

//===================================================================

class DynamicHeightMap
{
	friend class DynamicNormalMap;

public:

	explicit DynamicHeightMap (int size);
	~DynamicHeightMap ();

	int          getSize () const;
	const float* getData () const;
	void         ripple (int x, int y);
	void         update (float elapsedTime);

private:

	float        getData (const float* data, int x, int y) const;
	float        getData (int x, int y) const;
	void         setData (float* data, int x, int y, float value) const;

private:

	int          m_size;
	int          m_sizeSquared;
	int          m_mask;
	float*       m_source;
	float*       m_destination;

private:

	DynamicHeightMap ();
	DynamicHeightMap (const DynamicHeightMap&);
	DynamicHeightMap& operator= (const DynamicHeightMap&);
};

//===================================================================

inline int DynamicHeightMap::getSize () const
{
	return m_size;
}

//--------------------------------------------------------------------

inline const float* DynamicHeightMap::getData () const
{
	return m_source;
}

//--------------------------------------------------------------------

inline float DynamicHeightMap::getData (const float* data, int x, int y) const
{
	x &= m_mask;
	y &= m_mask;

	return data [y * m_size + x];
}

//--------------------------------------------------------------------

inline float DynamicHeightMap::getData (int x, int y) const
{
	return getData (getData (), x, y);
}

//--------------------------------------------------------------------

inline void DynamicHeightMap::setData (float* data, int x, int y, float value) const
{
	x &= m_mask;
	y &= m_mask;

	data [y * m_size + x] = value;
}

//--------------------------------------------------------------------

DynamicHeightMap::DynamicHeightMap (const int size) :
	m_size (size),
	m_sizeSquared (sqr (m_size)),
	m_mask (m_size - 1),
	m_source (new float [static_cast<size_t> (m_sizeSquared)]),
	m_destination (new float [static_cast<size_t> (m_sizeSquared)])
{
	DEBUG_WARNING (!IsPowerOfTwo (m_size), ("DynamicHeightMap not a power of 2"));
	memset (m_source, 0, sizeof (float) * static_cast<size_t> (m_sizeSquared));
	memset (m_destination, 0, sizeof (float) * static_cast<size_t> (m_sizeSquared));
}

//--------------------------------------------------------------------

DynamicHeightMap::~DynamicHeightMap ()
{
	delete [] m_source;
	m_source = 0;

	delete [] m_destination;
	m_destination = 0;
}

//--------------------------------------------------------------------

void DynamicHeightMap::ripple (const int x, const int y)
{
	setData (m_source, x,     y,     8.f);
	setData (m_source, x - 1, y,     8.f);
	setData (m_source, x + 1, y,     8.f);
	setData (m_source, x,     y - 1, 8.f);
	setData (m_source, x,     y + 1, 8.f);
}

//--------------------------------------------------------------------

void DynamicHeightMap::update (const float elapsedTime)
{
	const float dampener = 0.25f * elapsedTime;

	int x;
	int y;
	for (y = 0; y < m_size; ++y)
		for (x = 0; x < m_size; ++x)
		{
			float destination = 0.f;
			destination += getData (m_source, x - 1, y);
			destination += getData (m_source, x + 1, y);
			destination += getData (m_source, x,     y - 1);
			destination += getData (m_source, x,     y + 1);
			destination *= 0.5f;
			destination -= getData (m_destination, x, y);
			destination -= destination * dampener;

			setData (m_destination, x, y, destination);
		}

	std::swap (m_source, m_destination);
}

//===================================================================

class DynamicNormalMap
{
public:

	explicit DynamicNormalMap (int size);
	~DynamicNormalMap ();

	int           getSize () const;
	const Vector* getData () const;

	void          update (const DynamicHeightMap* dynamicHeightMap);

private:

	int           m_size;
	Vector*       m_data;

private:

	DynamicNormalMap ();
	DynamicNormalMap (const DynamicNormalMap&);
	DynamicNormalMap& operator= (const DynamicNormalMap&);
};

//--------------------------------------------------------------------

DynamicNormalMap::DynamicNormalMap (int size) :
	m_size (size),
	m_data (new Vector [static_cast<size_t> (m_size * m_size)])
{
	DEBUG_WARNING (!IsPowerOfTwo (m_size), ("DynamicNormalMap not a power of 2"));
}

//--------------------------------------------------------------------

DynamicNormalMap::~DynamicNormalMap ()
{
	delete [] m_data;
	m_data = 0;
}

//--------------------------------------------------------------------

inline int DynamicNormalMap::getSize () const
{
	return m_size;
}

//--------------------------------------------------------------------

inline const Vector* DynamicNormalMap::getData () const
{
	return m_data;
}

//--------------------------------------------------------------------

void DynamicNormalMap::update (const DynamicHeightMap* const dynamicHeightMap)
{
	const float offset = 16.f;

	//-- forward, left, center, right, backward
	Vector vList [5] = 
	{
		Vector ( 0.f,    0.f,  offset),
		Vector (-offset, 0.f,  0.f),
		Vector ( 0.f,    0.f,  0.f),
		Vector ( offset, 0.f,  0.f),
		Vector ( 0.f,    0.f, -offset)
	};

	//-- compute normal per pole
	int x;
	int y;
	for (y = 0; y < dynamicHeightMap->getSize (); ++y)
		for (x = 0; x < dynamicHeightMap->getSize (); ++x)
		{
			vList [0].y = dynamicHeightMap->getData (x,     y + 1);
			vList [1].y = dynamicHeightMap->getData (x - 1, y);
			vList [2].y = dynamicHeightMap->getData (x,     y);
			vList [3].y = dynamicHeightMap->getData (x + 1, y);
			vList [4].y = dynamicHeightMap->getData (x,     y - 1);

			Vector& normal = m_data [y * m_size + x];

			//-- reset
			normal.makeZero ();

			//-- add triangle 1, 2 1 0, (a - c).cross (b - a)
			normal += (vList [2] - vList [0]).cross (vList [1] - vList [2]);

			//-- add triangle 2, 2 0 3, (a - c).cross (b - a)
			normal += (vList [2] - vList [3]).cross (vList [0] - vList [2]);

			//-- add triangle 3, 2 3 4, (a - c).cross (b - a)
			normal += (vList [2] - vList [4]).cross (vList [3] - vList [2]);

			//-- add triangle 4, 2 4 1, (a - c).cross (b - a)
			normal += (vList [2] - vList [1]).cross (vList [4] - vList [2]);

			//-- normalize
			IGNORE_RETURN (normal.approximateNormalize ());
		}
}

//===================================================================

namespace DynamicNormalMapNamespace
{
	static bool s_debugReport;
	static bool s_renderMap;
	static bool s_renderHeightMap;

	const int   s_textureSize = 128;

	inline int log (int value)
	{
		int count = 0;
		while (value > 0)
		{
			value >>= 1;
			++count;
		}

		return count;
	}

	const int s_cubeTextureSize = 32;

	const Tag TAG_MAIN = TAG (M,A,I,N);
	const Tag TAG_NRML = TAG (N,R,M,L);
	const Tag TAG__SKY = TAG (_,S,K,Y);
}

using namespace DynamicNormalMapNamespace;

//===================================================================

DynamicNormalMapTest::DynamicNormalMapTest () : 
	TestIoWin ("DynamicNormalMapTest"),
	m_dynamicShader (ShaderTemplateList::fetchModifiableShader ("shader/uicanvas_filtered.sht")),
	m_dynamicTexture (0),
	m_waterHeightMap (new DynamicHeightMap (s_textureSize)),
	m_waterNormalMap (new DynamicNormalMap (s_textureSize)),
	m_timer (0.2f),
	m_gradientSkyTexture (0),
	m_dynamicCubeTexture (0),
	m_dynamicCubeTimer (10.f)
{
	DebugFlags::registerFlag (s_debugReport, "Test", "debugReport");
	DebugFlags::registerFlag (s_renderMap, "Test", "renderMap");

	const TextureFormat runtimeFormats [] = { TF_ARGB_8888 };
	const int numberOfRuntimeFormats = sizeof (runtimeFormats) / sizeof (runtimeFormats [0]);
	m_dynamicTexture = TextureList::fetch (0, s_textureSize, s_textureSize, log (s_textureSize), runtimeFormats, numberOfRuntimeFormats);

	safe_cast<StaticShader*>(m_dynamicShader)->setTexture (TAG_MAIN, *m_dynamicTexture);

	{
		m_gradientSkyTexture = TextureList::fetch ("texture/grad_sky_tato.dds");
		m_dynamicCubeTexture = TextureList::fetch (TCF_cubeMap, s_cubeTextureSize, s_cubeTextureSize, 1, runtimeFormats, numberOfRuntimeFormats);
	}
}

//-------------------------------------------------------------------

DynamicNormalMapTest::~DynamicNormalMapTest ()
{
	DebugFlags::unregisterFlag (s_debugReport);

	m_dynamicShader->release ();
	m_dynamicShader = 0;

	m_dynamicTexture->release ();
	m_dynamicTexture = 0;

	delete m_waterHeightMap;
	m_waterHeightMap = 0;

	delete m_waterNormalMap;
	m_waterNormalMap = 0;

	{
		m_gradientSkyTexture->release ();
		m_dynamicCubeTexture->release ();
	}
}

//-------------------------------------------------------------------

IoResult DynamicNormalMapTest::processEvent (IoEvent* event)
{
	const IoResult result = TestIoWin::processEvent (event);

	switch (event->type)
	{
	case IOET_WindowKill:
		return IOR_PassKillMe;

	case IOET_Character:
		{
			if (tolower (event->arg2) == 't')
				s_renderHeightMap = !s_renderHeightMap;
		}
		break;

	case IOET_Update:
		{
			const float elapsedTime = event->arg3;

			m_dynamicCubeTimer.updateSubtract (elapsedTime);

			if (m_timer.updateSubtract (elapsedTime))
				m_waterHeightMap->ripple (Random::random (0, 255), Random::random (0, 255));

			//-- update the water
			m_waterHeightMap->update (event->arg3);
			m_waterNormalMap->update (m_waterHeightMap);

			//-- render heightmap
			const int numberOfMipMaps = log (s_textureSize);
			int i;
			for (i = 0; i < numberOfMipMaps; ++i)
			{
				const int mipMapSize = s_textureSize >> i;
				const int offset     = 1 << i;

				TextureGraphicsData::LockData lockData (TF_ARGB_8888, i, 0, 0, mipMapSize, mipMapSize, true);
				m_dynamicTexture->lock (lockData);

				if (s_renderHeightMap)
				{
					const float* sourceData     = m_waterHeightMap->getData ();
					uint8* const pixelData      = reinterpret_cast<uint8*> (lockData.getPixelData ());
					uint8*       destinationRow = pixelData;

					int dy;
					for (dy = 0; dy < lockData.getHeight (); ++dy, destinationRow += lockData.getPitch ())
					{
						const float* source      = sourceData;
						uint8*       destination = destinationRow;

						int dx;
						for (dx = 0; dx < lockData.getWidth (); ++dx)
						{
							const uint8 color = static_cast<uint8> (clamp (0.f, *source + 127.f, 255.f));

							*destination++ = color;
							*destination++ = color;
							*destination++ = color;
							*destination++ = 0;

							source += offset;
						}

						sourceData += offset * m_waterHeightMap->getSize ();
					}
				}
				else
				{
					const Vector* sourceData     = m_waterNormalMap->getData ();
					uint8* const  pixelData      = reinterpret_cast<uint8*> (lockData.getPixelData ());
					uint8*        destinationRow = pixelData;

					int dy;
					for (dy = 0; dy < lockData.getHeight (); ++dy, destinationRow += lockData.getPitch ())
					{
						const Vector* source      = sourceData;
						uint8*        destination = destinationRow;

						int dx;
						for (dx = 0; dx < lockData.getWidth (); ++dx)
						{
							//-- convert normal to color
							const Vector& normal = *source;

							//-- b (y)
							*destination++ = static_cast<uint8> (255.f * (normal.y * 0.5f + 0.5f));

							//-- g (-z)
							*destination++ = static_cast<uint8> (255.f * (-normal.z * 0.5f + 0.5f));

							//-- r (x)
							*destination++ = static_cast<uint8> (255.f * (normal.x * 0.5f + 0.5f));

							//-- a
							*destination++ = 0;

							source += offset;
						}

						sourceData += offset * m_waterHeightMap->getSize ();
					}
				}

				m_dynamicTexture->unlock (lockData);
			}

			//--
			{
				updateDynamicCubeTexture ();
			}

			//-- debug report
			if (s_debugReport)
			{
				DEBUG_REPORT_PRINT (true, ("Q      quit\n\n"));
				DEBUG_REPORT_PRINT (true, ("m0 = %i\n", m_button0 ? 1 : 0));
				DEBUG_REPORT_PRINT (true, ("m1 = %i\n", m_button1 ? 1 : 0));
				DEBUG_REPORT_PRINT (true, ("m2 = %i\n", m_button2 ? 1 : 0));
			}
		}
		break;

	default:
		break;
	}

	return result;
}

//-------------------------------------------------------------------

void DynamicNormalMapTest::draw () const
{
	TestIoWin::draw ();

	if (s_renderMap)
	{
		Graphics::setStaticShader (m_dynamicShader->prepareToView ());

		VertexBufferFormat format;
		format.setTransformed ();
		format.setPosition ();
		format.setNumberOfTextureCoordinateSets (1);
		format.setTextureCoordinateSetDimension (0, 2);

		DynamicVertexBuffer vertexBuffer (format);

		vertexBuffer.lock (4);

			VertexBufferWriteIterator dv = vertexBuffer.begin ();

				dv.setPosition (0.f, 0.f, 0.f);
				dv.setOoz (1.f);
				dv.setTextureCoordinates (0, 0.0f, 0.0f);
				++dv;

				dv.setPosition (256.f, 0.f, 0.f);
				dv.setOoz (1.f);
				dv.setTextureCoordinates (0, 1.0f, 0.0f);
				++dv;

				dv.setPosition (256.f, 256.f, 0.f);
				dv.setOoz (1.f);
				dv.setTextureCoordinates (0, 1.0f, 1.0f);
				++dv;

				dv.setPosition (0.f, 256.f, 0.f);
				dv.setOoz (1.f);
				dv.setTextureCoordinates (0, 0.0f, 1.0f);
				++dv;

		vertexBuffer.unlock ();

		Graphics::setVertexBuffer (vertexBuffer);
		Graphics::drawTriangleFan ();
	}

	{
		const Vector ms_vertices [8] =
		{
			Vector (-1.f,  1.f,  1.f),
			Vector ( 1.f,  1.f,  1.f),
			Vector ( 1.f,  1.f, -1.f),
			Vector (-1.f,  1.f, -1.f),
			Vector (-1.f, -1.f,  1.f),
			Vector ( 1.f, -1.f,  1.f),
			Vector ( 1.f, -1.f, -1.f),
			Vector (-1.f, -1.f, -1.f)
		};

		const int lines [24] =
		{
			0, 1,
			1, 2,
			2, 3,
			3, 0,
			4, 5,
			5, 6,
			6, 7,
			7, 4,
			0, 4,
			1, 5,
			2, 6,
			3, 7
		};

		Graphics::setStaticShader (ShaderTemplateList::get3dVertexColorStaticShader ());
		Graphics::setObjectToWorldTransformAndScale (Transform::identity, Vector::xyz111);

		int i;
		for (i = 0; i < 12; ++i)
			Graphics::drawLine (ms_vertices [lines [i * 2 + 0]], ms_vertices [lines [i * 2 + 1]], VectorArgb::solidWhite);
	}
}

//===================================================================

void DynamicNormalMapTest::createFloor ()
{
}

//-------------------------------------------------------------------

Object* DynamicNormalMapTest::createObjectAndAppearance ()
{
	Graphics::setGlobalTexture (TAG__SKY, *m_dynamicCubeTexture);

	Object* const object = new Object ();
#if 1
	object->setScale (Vector::xyz111 * 10.f);
	object->setAppearance (new SkyBoxAppearance (m_dynamicCubeTexture));
	object->getAppearance ()->setAlpha (true, 1.f, true, 1.f);
//	object->setAppearance (new SkyBoxAppearance ("shader/skybox.sht"));
#else
	Shader* const shader = ShaderTemplateList::fetchModifiableShader ("shader/water_test.sht");
	safe_cast<StaticShader*>(shader)->setTexture (TAG_NRML, *m_dynamicTexture);
	object->setAppearance (new ShaderAppearance (10.f, 10.f, shader, VectorArgb::solidWhite, 5.f));
#endif

	return object;
}

//===================================================================

void DynamicNormalMapTest::updateDynamicCubeTexture ()
{
	PackedRgb colorRamp [s_cubeTextureSize];

	//-- copy the strip from the environment texture into the color ramp
	{
		const Texture* const texture = m_gradientSkyTexture;

		TextureGraphicsData::LockData lockData (TF_ARGB_8888, 0, 0, 0, texture->getWidth (), texture->getHeight (), false);
		texture->lockReadOnly (lockData);

			const int strip = static_cast<int> (linearInterpolate (0, lockData.getWidth () - 1, m_dynamicCubeTimer.getElapsedRatio ()));
			const uint8* const pixelData = reinterpret_cast<const uint8*> (lockData.getPixelData ());
			const int dy = lockData.getHeight () / s_cubeTextureSize;

			int y;
			for (y = 0; y < s_cubeTextureSize; ++y)
			{
				const uint8* destination = pixelData + (y * dy * lockData.getPitch ()) + (strip * 4);

				colorRamp [y].b = *destination++;
				colorRamp [y].g = *destination++;
				colorRamp [y].r = *destination++;
			}

		texture->unlock (lockData);
	}

	//-- create +y
	{
		Texture::LockData lockData (TF_ARGB_8888, CF_positiveY, 0, 0, 0, 0, s_cubeTextureSize, s_cubeTextureSize, true);
		m_dynamicCubeTexture->lock (lockData);

			uint8* const pixelData = reinterpret_cast<uint8*> (lockData.getPixelData ());

			const PackedRgb& color = colorRamp [0];

			int y;
			for (y = 0; y < s_cubeTextureSize; ++y)
			{
				uint8* destination = pixelData + (y * lockData.getPitch ());

				int x;
				for (x = 0; x < s_cubeTextureSize; ++x)
				{
					*destination++ = color.b;
					*destination++ = color.g;
					*destination++ = color.r;
					*destination++ = 255;
				}
			}

		m_dynamicCubeTexture->unlock (lockData);
	}

	//-- create -y
	{
		Texture::LockData lockData (TF_ARGB_8888, CF_negativeY, 0, 0, 0, 0, s_cubeTextureSize, s_cubeTextureSize, true);
		m_dynamicCubeTexture->lock (lockData);

			uint8* const pixelData = reinterpret_cast<uint8*> (lockData.getPixelData ());

			const PackedRgb& color = colorRamp [s_cubeTextureSize - 1];

			int y;
			for (y = 0; y < s_cubeTextureSize; ++y)
			{
				uint8* destination = pixelData + (y * lockData.getPitch ());

				int x;
				for (x = 0; x < s_cubeTextureSize; ++x)
				{
					*destination++ = color.b;
					*destination++ = color.g;
					*destination++ = color.r;
					*destination++ = 255;
				}
			}

		m_dynamicCubeTexture->unlock (lockData);
	}

	//-- create +x, -x, +z, -z
	{
		int i;
		for (i = 0; i < CF_none; ++i)
		{
			if (i == CF_positiveY || i == CF_negativeY)
				continue;

			Texture::LockData lockData (TF_ARGB_8888, static_cast<CubeFace> (i), 0, 0, 0, 0, s_cubeTextureSize, s_cubeTextureSize, true);
			m_dynamicCubeTexture->lock (lockData);

				uint8* const pixelData = reinterpret_cast<uint8*> (lockData.getPixelData ());

				int y;
				for (y = 0; y < s_cubeTextureSize / 2; ++y)
				{
					const float dy = sqr (static_cast<float> (y) / (s_cubeTextureSize / 2));
					const PackedRgb& color = colorRamp [static_cast<int> (dy * s_cubeTextureSize)];

					uint8* destination = pixelData + (y * lockData.getPitch ());

					int x;
					for (x = 0; x < s_cubeTextureSize; ++x)
					{
						*destination++ = color.b;
						*destination++ = color.g;
						*destination++ = color.r;
						*destination++ = 255;
					}
				}

				for (y = s_cubeTextureSize / 2; y < s_cubeTextureSize; ++y)
				{
					const PackedRgb& color = colorRamp [s_cubeTextureSize - 1];

					uint8* destination = pixelData + (y * lockData.getPitch ());

					int x;
					for (x = 0; x < s_cubeTextureSize; ++x)
					{
						*destination++ = color.b;
						*destination++ = color.g;
						*destination++ = color.r;
						*destination++ = 255;
					}
				}

			m_dynamicCubeTexture->unlock (lockData);
		}
	}
}

//-------------------------------------------------------------------

