//======================================================================
//
// NebulaShellShaderPrimitive.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/NebulaShellShaderPrimitive.h"

#include "clientGame/NebulaVisualQuad.h"
#include "clientGame/NebulaVisualQuadShaderGroup.h"
#include "clientGraphics/Camera.h"
#include "clientGraphics/DebugPrimitive.h"
#include "clientGraphics/DynamicIndexBuffer.h"
#include "clientGraphics/DynamicVertexBuffer.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/RenderWorld.h"
#include "clientGraphics/ShaderPrimitive.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientGraphics/ShaderTemplate.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/StaticShader.h"
#include "clientGraphics/VertexBufferIterator.h"
#include "clientObject/SpriteAppearanceTemplate.h"
#include "sharedCollision/Extent.h"
#include "sharedCollision/ExtentList.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedMath/Plane.h"
#include <algorithm>
#include <map>
#include "sharedFoundation/MemoryBlockManager.h"

//======================================================================

namespace NebulaShellShaderPrimitiveNamespace
{
	bool s_showNebulaQuadFrames = false;
	bool s_nebulaQuadSolid = false;

	//--
	//-- Every nebula shader has a counterpart for rendering past the far plane
	//-- the counterpart far shader is a shader which does not do any z testing
	//--

	typedef stdmap<ShaderTemplate const *, ShaderTemplate const *>::fwd ShaderTemplateMap;
	ShaderTemplateMap s_shaderTemplateNoZMap;

	Shader const * const fetchFarShader(Shader const & nearShader)
	{
		ShaderTemplate const & shaderTemplateNear = nearShader.getShaderTemplate();

		ShaderTemplateMap::iterator const it = s_shaderTemplateNoZMap.find(&shaderTemplateNear);
		if (it != s_shaderTemplateNoZMap.end())
			return NON_NULL((*it).second)->fetchShader();

		shaderTemplateNear.fetch();
		std::string const & shaderTemplateNearName = shaderTemplateNear.getName().getString();

		size_t const dotpos = shaderTemplateNearName.find(".sht");

		WARNING(dotpos == std::string::npos, ("NebulaShellShaderPrimitive invalid near shader template name [%s]", shaderTemplateNearName.c_str()));

		std::string const & shaderTemplateFarName = shaderTemplateNearName.substr(0, dotpos) + "_no_z.sht";

		ShaderTemplate const * const shaderTemplateFar = ShaderTemplateList::fetch(shaderTemplateFarName.c_str());
		NOT_NULL(shaderTemplateFar);

		s_shaderTemplateNoZMap.insert(std::make_pair(&shaderTemplateNear, shaderTemplateFar));

		return shaderTemplateFar->fetchShader();
	}
}

using namespace NebulaShellShaderPrimitiveNamespace;

//----------------------------------------------------------------------

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITHOUT_INSTALL (NebulaShellShaderPrimitive, true, 0, 0, 0);

//----------------------------------------------------------------------

VertexBufferFormat NebulaShellShaderPrimitive::getVertexBufferFormat()
{
	VertexBufferFormat format;
	format.setPosition();
	format.setNormal();
	format.setColor0();
	format.setNumberOfTextureCoordinateSets(1);
	format.setTextureCoordinateSetDimension(0, 2);
	return format;
}

//----------------------------------------------------------------------

void NebulaShellShaderPrimitive::install()
{
	InstallTimer const installTimer("NebulaShellShaderPrimitive::install");

	DebugFlags::registerFlag(s_showNebulaQuadFrames, "ClientGame/NebulaManagerClient", "showNebulaQuadFrames");
	DebugFlags::registerFlag(s_nebulaQuadSolid, "ClientGame/NebulaManagerClient", "nebulaQuadSolid");

	installMemoryBlockManager ();
	ExitChain::add(NebulaShellShaderPrimitive::remove, "NebulaShellShaderPrimitive::remove");
}

//----------------------------------------------------------------------

void NebulaShellShaderPrimitive::remove()
{
	removeMemoryBlockManager ();

	for (ShaderTemplateMap::iterator it = s_shaderTemplateNoZMap.begin(); it != s_shaderTemplateNoZMap.end(); ++it)
	{
		ShaderTemplate const * const shaderTemplateNear = (*it).first;
		ShaderTemplate const * const shaderTemplateFar = (*it).second;

		shaderTemplateNear->release();
		shaderTemplateFar->release();
	}
}

//===================================================================

/**
* @param shaderGroup must be pre-sorted
*/

NebulaShellShaderPrimitive::NebulaShellShaderPrimitive(NebulaVisualQuadShaderGroup const & shaderGroup, int const quadBegin, int const quadEnd, float const depthSquaredSortKey, bool const isOutsideFarPlane) :
ShaderPrimitive(),
m_shader(&shaderGroup.getShader()),
m_sphere(shaderGroup.getSphere()),
m_depthSquaredSortKey(depthSquaredSortKey),
m_nebulaVisualQuadVector(shaderGroup.getNebulaVisualQuads()),
m_quadBegin(quadBegin),
m_quadEnd(quadEnd),
m_isOutsideFarPlane(isOutsideFarPlane),
m_canDraw(false)
{
#ifdef _DEBUG
	int const size = static_cast<int>(m_nebulaVisualQuadVector.size());
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, quadBegin, size);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(quadBegin + 1, quadEnd, size + 1);
#endif

	if (m_isOutsideFarPlane)
	{
		m_shader = fetchFarShader(*m_shader);
	}
	else
		m_shader->fetch();
}

// ----------------------------------------------------------------------

NebulaShellShaderPrimitive::~NebulaShellShaderPrimitive()
{
	m_shader->release();
	m_shader = NULL;
}

// ----------------------------------------------------------------------

const Vector NebulaShellShaderPrimitive::getPosition_w() const
{
	return m_sphere.getCenter();
}

//-------------------------------------------------------------------

float NebulaShellShaderPrimitive::getDepthSquaredSortKey() const
{
	return m_depthSquaredSortKey;
}

// ----------------------------------------------------------------------

int NebulaShellShaderPrimitive::getVertexBufferSortKey() const
{
	return 0;
}

// ----------------------------------------------------------------------

const StaticShader &NebulaShellShaderPrimitive::prepareToView() const
{
	if (s_nebulaQuadSolid)
		return ShaderTemplateList::get3dVertexColorAZStaticShader().prepareToView();
	else
		return m_shader->prepareToView();
}

// ----------------------------------------------------------------------

void NebulaShellShaderPrimitive::prepareToDraw() const
{
	m_canDraw = false;

	Camera const & camera = ShaderPrimitiveSorter::getCurrentCamera();
	Vector const & cameraPos_w = camera.getPosition_w();
	Vector const & cameraFrameK = camera.getObjectFrameK_p();

	float const farPlane = camera.getFarPlane();
	float const nearPlane = camera.getFarPlane();
	float const farPlaneRatio = farPlane / 32768.0f;
	float const nearPlaneRatio = nearPlane / 32768.0f;
	float const viewingDistanceRatio = farPlaneRatio - nearPlaneRatio;
	float const farScaleFactor = nearPlaneRatio + viewingDistanceRatio;

	int vertexCount = 0;
	int indexCount = 0;
	{
		NebulaVisualQuadVector::const_iterator end = m_nebulaVisualQuadVector.begin() + m_quadEnd;
		for (NebulaVisualQuadVector::const_iterator it = m_nebulaVisualQuadVector.begin() + m_quadBegin; it != end; ++it)
		{
			NebulaVisualQuad const & quad = **it;

			//-- Quad starts out as not rendered
			quad.setRenderThisFrame(false);

			Sphere const & sphere = quad.getSphere();

			//-- quads in this shader primitive are assumed to be pre-culled against the frustum
			Vector const & quadPos_w = sphere.getCenter();

			Transform t(Transform::identity);

			bool const quadIsOriented = quad.isOriented();

			Vector const cameraPosFromQuad_w = cameraPos_w - quadPos_w;

			float facingAlphaFactor = 1.0f;
			Vector cameraToQuadVector(-cameraPosFromQuad_w);
			cameraToQuadVector.approximateNormalize();

			//-- oriented quads are in a fixed rotation relative to world space
			if (quadIsOriented)
			{
				Vector const & rotation = quad.getRotation();
				t.yaw_l(rotation.x);
				t.pitch_l(rotation.y);
				t.roll_l(rotation.z);
			}

			//- these are camera facing quads
			else
			{
				//-- use the vector from the quad to the camera as the transform frame k
				Vector frameK = cameraPosFromQuad_w;

				//-- failure to normalize probaly means we are too close to properly rotate
				if (!frameK.approximateNormalize())
					continue;

				Vector facingFrameK = Vector::linearInterpolate(frameK, -cameraFrameK, 0.5f);

				if (!facingFrameK.approximateNormalize())
					continue;

				static float const s_zeroEpsilon = 0.0001f;
				Vector const & frameJ = quad.getRotation();

				float const frameJCameraDot = frameJ.dot(cameraToQuadVector);

				facingAlphaFactor = sqr(clamp(0.0f, (1.0f - fabsf(frameJCameraDot)), 1.0f));

				if (facingFrameK.cross(frameJ).withinEpsilon(Vector::zero, s_zeroEpsilon))
					continue;

				t.setLocalFrameKJ_p(facingFrameK, frameJ);
			}

			t.setPosition_p(quadPos_w);

			float const cameraDot = cameraToQuadVector.dot(t.getLocalFrameK_p());
			bool const showFront = cameraDot < 0.0f;

			//-- skip this quad if we are neither able to show the front nor the quad is oriented
			//-- if the quad is oriented, it is 2-sided, so it always renders
			if (!showFront && !quadIsOriented)
				continue;

			//-- fade the quads based on the dot product of the quad normal and the camera to quad vector
			//-- edge-on quads get faded out via vertex alpha
			float cameraDotAlphaFactor = (clamp(0.0f, fabsf(cameraDot), 1.0f) * (s_nebulaQuadSolid ? 0.5f : 1.0f)) * (facingAlphaFactor);

			//-- quad is too faint, don't render it
			if (cameraDotAlphaFactor <= 0.01f)
				continue;

			if (showFront)
				indexCount += 6;

			//-- oriented quads have backsides
			if (quad.isOriented())
				indexCount += 6;

			//-- render closer and within the far plane (as part of the environment)
			if (isOutsideFarPlane())
				t.move_p(cameraPosFromQuad_w * (1.0f - farScaleFactor));

			vertexCount += 4;

#ifdef _DEBUG
			if (s_showNebulaQuadFrames)
				ShaderPrimitiveSorter::getCurrentCamera().addDebugPrimitive(new FrameDebugPrimitive(FrameDebugPrimitive::S_z, t, sphere.getRadius()));
#endif

			quad.setRenderThisFrame(true);
			quad.setRenderTransform(t);
			quad.setRenderAlpha(cameraDotAlphaFactor);
			quad.setRenderShowFront(showFront);
		}
	}

	if (vertexCount > 0 && indexCount > 0)
	{
		m_canDraw = true;

		Vector corners[4];

		int const cornerIndices[6] =
		{
			0, 1, 3,
			1, 2, 3
		};

		//-- Fill out vertex and index buffer
		DynamicVertexBuffer vertexBuffer(getVertexBufferFormat());
		DynamicIndexBuffer indexBuffer;

		vertexBuffer.lock(vertexCount);
		indexBuffer.lock(indexCount);

		VertexBufferWriteIterator vi = vertexBuffer.begin();
		Index * ii = indexBuffer.begin();

		int vertexIndex = 0;

		NebulaVisualQuadVector::const_iterator end = m_nebulaVisualQuadVector.begin() + m_quadEnd;
		for (NebulaVisualQuadVector::const_iterator it = m_nebulaVisualQuadVector.begin() + m_quadBegin; it != end; ++it)
		{
			NebulaVisualQuad const * const quad = *it;
			if (!quad->getRenderThisFrame())
				continue;

			if (quad->getRenderShowFront())
			{
				*ii++ = static_cast<uint16>(cornerIndices[0] + vertexIndex);
				*ii++ = static_cast<uint16>(cornerIndices[1] + vertexIndex);
				*ii++ = static_cast<uint16>(cornerIndices[2] + vertexIndex);
				*ii++ = static_cast<uint16>(cornerIndices[3] + vertexIndex);
				*ii++ = static_cast<uint16>(cornerIndices[4] + vertexIndex);
				*ii++ = static_cast<uint16>(cornerIndices[5] + vertexIndex);
			}

			//-- oriented quads have backsides
			if (quad->isOriented())
			{
				*ii++ = static_cast<uint16>(cornerIndices[5] + vertexIndex);
				*ii++ = static_cast<uint16>(cornerIndices[4] + vertexIndex);
				*ii++ = static_cast<uint16>(cornerIndices[3] + vertexIndex);
				*ii++ = static_cast<uint16>(cornerIndices[2] + vertexIndex);
				*ii++ = static_cast<uint16>(cornerIndices[1] + vertexIndex);
				*ii++ = static_cast<uint16>(cornerIndices[0] + vertexIndex);
			}

			vertexIndex += 4;

			float const radius = quad->getSphere().getRadius();
			corners[0] = Vector(-radius, radius, 0.0f);
			corners[1] = Vector(-radius, -radius, 0.0f);
			corners[2] = Vector(radius, -radius, 0.0f);
			corners[3] = Vector(radius, radius, 0.0f);

			//-- render closer and within the far plane (as part of the environment)
			if (isOutsideFarPlane())
			{
				corners[0] *= farScaleFactor;
				corners[1] *= farScaleFactor;
				corners[2] *= farScaleFactor;
				corners[3] *= farScaleFactor;
			}

			Transform const & renderTransform = quad->getRenderTransform();
			Vector const & frameK = renderTransform.getLocalFrameK_p();

			renderTransform.rotateTranslate_l2p(corners, corners, 4);

			//-- vertex colors are returned as an array of 7 color values
			VectorArgb const * const vertexColors = quad->getVertexColors();

			//-- uvs are returned as an array of 8 float values.
			float const * const uvs = quad->getCornerUvsForStyle();

			float const renderAlpha = quad->getRenderAlpha();
			int uvIndex = 0;
			for (int i = 0; i < 4; ++i, ++vi)
			{
				vi.setPosition(corners[i]);

				vi.setNormal(frameK);

				VectorArgb color = vertexColors[i];
				color.a *= renderAlpha;
				vi.setColor0(color);

				float const u = uvs[uvIndex++];
				float const v = uvs[uvIndex++];
				vi.setTextureCoordinates(0, u, v);
			}
		}


		DEBUG_WARNING(vertexIndex != vertexCount, ("NebulaShellShaderPrimitive mismatch vertex counts. buffer has %d, we filled %d.", vertexIndex, vertexCount));

		vertexBuffer.unlock();
		indexBuffer.unlock();

		Graphics::setObjectToWorldTransformAndScale(Transform::identity, Vector::xyz111);
		Graphics::setVertexBuffer(vertexBuffer);
		Graphics::setIndexBuffer(indexBuffer);
	}
}

// ----------------------------------------------------------------------

void NebulaShellShaderPrimitive::draw() const
{
	NP_PROFILER_AUTO_BLOCK_DEFINE("NebulaShellShaderPrimitive::draw");

	if (m_canDraw)
		Graphics::drawIndexedTriangleList();
}

//======================================================================
