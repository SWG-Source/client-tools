// ======================================================================
//
// CuiWidgetGroundRadar.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiWidgetGroundRadar.h"

#include "UIImage.h"
#include "UIMessage.h"
#include "UIPalette.h"
#include "UITextStyle.h"
#include "UITextStyleManager.h"
#include "UnicodeUtils.h"
#include "clientGame/ClientObject.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGraphics/Camera.h"
#include "clientGraphics/DynamicVertexBuffer.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/Shader.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/StaticShader.h"
#include "clientGraphics/Texture.h"
#include "clientGraphics/TextureList.h"
#include "clientGraphics/VertexBuffer.h"
#include "clientTerrain/ClientProceduralTerrainAppearance.h"
#include "clientUserInterface/CuiLayerRenderer.h"
#include "clientUserInterface/CuiUtils.h"
#include "sharedObject/Object.h"
#include <cstdio>

// ======================================================================

const int CuiWidgetGroundRadar::CP_N = 0;
const int CuiWidgetGroundRadar::CP_E = 1;
const int CuiWidgetGroundRadar::CP_S = 2;
const int CuiWidgetGroundRadar::CP_W = 3;

static const char s_dirChars[4] = { 'N', 'E', 'S', 'W' };

const UILowerString CuiWidgetGroundRadar::PropertyName::TextStyle = UILowerString ("TextStyle");
const char * const CuiWidgetGroundRadar::TypeName                = "CuiWidgetGroundRadar";

//-----------------------------------------------------------------

CuiWidgetGroundRadar::CuiWidgetGroundRadar () :
UIWidget           (),
m_textStyle        (0),
m_angle            (0.0f),
m_lastSize         (),
m_radarShaderInfo  (),
m_clipToCircle     (true),
m_drawBackground   (true),
m_shaderIsCleared  (false),
m_phonyShaderTexture(0)
{
	m_compassText [CP_N].assign (1, s_dirChars[CP_N]);
	m_compassText [CP_E].assign (1, s_dirChars[CP_E]);
	m_compassText [CP_S].assign (1, s_dirChars[CP_S]);
	m_compassText [CP_W].assign (1, s_dirChars[CP_W]);

	//--
	const TextureFormat runtimeTextureFormat = TF_ARGB_8888;
	m_phonyShaderTexture = TextureList::fetch (0, 128, 128, 1, &runtimeTextureFormat, 1);
}

//-----------------------------------------------------------------

CuiWidgetGroundRadar::~CuiWidgetGroundRadar ()
{
	m_textStyle = 0;

	m_phonyShaderTexture->release();
	m_phonyShaderTexture = 0;
}

//-----------------------------------------------------------------

void CuiWidgetGroundRadar::GetPropertyNames( UIPropertyNameVector &In, bool forCopy ) const
{
	In.push_back ( PropertyName::TextStyle );
	UIWidget::GetPropertyNames( In, forCopy );
}

//-----------------------------------------------------------------

bool CuiWidgetGroundRadar::SetProperty( const UILowerString & Name, const UIString &Value )
{
	if (Name == PropertyName::TextStyle)
	{
		m_textStyle = UITextStyleManager::GetInstance()->GetFontForLogicalFont(Value);
		UIWidget::SetProperty (Name, Value);
		if (m_textStyle)
		{
			return true;
		}
	}

	return UIWidget::SetProperty (Name, Value);
}

//-----------------------------------------------------------------

bool CuiWidgetGroundRadar::GetProperty( const UILowerString & Name, UIString &Value ) const
{
	if(Name == PropertyName::TextStyle )
	{
		if( m_textStyle )
		{
			Value = m_textStyle->GetLogicalName();
			return true;
		}
	}

	return UIWidget::GetProperty( Name, Value );
}

//-----------------------------------------------------------------

void CuiWidgetGroundRadar::Render (UICanvas & ) const
{
	const UISize & widgetSize = GetSize ();

	/*
	const Camera * const camera = Game::getCamera ();

	if (camera)
	{
		const real theta = camera->getObjectFrameK_w ().theta ();
		if (theta != m_angle)
			const_cast<CuiWidgetGroundRadar *>(this)->setAngle (theta, false);
	}
*/

	//-- regenerate vertexes if the size changes
	if (m_lastSize != widgetSize)
	{
		const_cast<CuiWidgetGroundRadar *>(this)->setAngle (m_angle, true);
		m_lastSize = widgetSize;
	}

	CuiLayerRenderer::flushRenderQueue ();

	const UIPoint & point = GetWorldLocation  ();

	if (m_radarShaderInfo.m_shader)
	{
		const UIPoint & center = point + (widgetSize / 2);
		Graphics::setStaticShader (m_radarShaderInfo.m_shader->prepareToView ());

		//-- build vertexarray
		VertexBufferFormat format;
		format.setPosition    ();
		format.setTransformed ();
		format.setNumberOfTextureCoordinateSets (2);
		format.setTextureCoordinateSetDimension (0, 2);
		format.setTextureCoordinateSetDimension (1, 2);
		format.setColor0      ();

		DynamicVertexBuffer vertexBuffer (format);
		vertexBuffer.lock(4);

		static const UIFloatPoint normal_uvs [4] =
		{
			UIFloatPoint (0.0f, 0.0f),
			UIFloatPoint (1.0f, 0.0f),
			UIFloatPoint (1.0f, 1.0f),
			UIFloatPoint (0.0f, 1.0f)
		};

		VertexBufferWriteIterator v = vertexBuffer.begin();
		for (size_t i = 0; i < 4; ++i)
		{
			v.setPosition (m_quadPoints [i].x + center.x, m_quadPoints [i].y + center.y, 1.0f);
			v.setOoz (1.0f);
			v.setTextureCoordinates (0, m_radarShaderInfo.m_quadUVs[i].x, m_radarShaderInfo.m_quadUVs[i].y);
			v.setTextureCoordinates (1, normal_uvs [i].x, normal_uvs [i].y);
			v.setColor0(PackedArgb::solidWhite);
			++v;
		}

		vertexBuffer.unlock();

		Graphics::setVertexBuffer(vertexBuffer);
		Graphics::drawTriangleFan ();
	}
	else if(m_drawBackground)
	{
		const StaticShader * shader = safe_cast<const StaticShader *>(ShaderTemplateList::fetchModifiableShader("shader/uicanvas_radar.sht"));

		const_cast<StaticShader *>(shader)->setTexture(TAG(M,A,I,N), *m_phonyShaderTexture);					

		// Draw a simple black circle
		const UIPoint & center = point + (widgetSize / 2);
		//Graphics::setStaticShader (ShaderTemplateList::get2dVertexColorStaticShader ());
		Graphics::setStaticShader (*shader);
		//-- build vertexarray
		VertexBufferFormat format;
		format.setPosition    ();
		format.setTransformed ();
		format.setNumberOfTextureCoordinateSets (2);
		format.setTextureCoordinateSetDimension (0, 2);
		format.setTextureCoordinateSetDimension (1, 2);
		format.setColor0      ();

		DynamicVertexBuffer vertexBuffer (format);
		vertexBuffer.lock(4);

		static const UIFloatPoint normal_uvs [4] =
		{
			UIFloatPoint (0.0f, 0.0f),
			UIFloatPoint (1.0f, 0.0f),
			UIFloatPoint (1.0f, 1.0f),
			UIFloatPoint (0.0f, 1.0f)
		};

		VertexBufferWriteIterator v = vertexBuffer.begin();
		for (size_t i = 0; i < 4; ++i)
		{
			v.setPosition (m_quadPoints [i].x + center.x, m_quadPoints [i].y + center.y, 1.0f);
			v.setOoz (1.0f);
			v.setTextureCoordinates (0, normal_uvs[i].x, normal_uvs[i].y);
			v.setTextureCoordinates (1, normal_uvs [i].x, normal_uvs [i].y);
			v.setColor0(PackedArgb::solidBlack);
			++v;
		}

		vertexBuffer.unlock();

		Graphics::setVertexBuffer(vertexBuffer);
		Graphics::drawTriangleFan ();
		shader->release();
	}
}

//-----------------------------------------------------------------

void CuiWidgetGroundRadar::setClipToCircle (bool clip)
{
	m_clipToCircle = clip;
}

//-----------------------------------------------------------------

void CuiWidgetGroundRadar::setAngle (float angle, bool force)
{
	// m_angle is in radians

	if (force == false && angle == m_angle)
		return;

	m_angle = angle;

	UISize halfSize      = GetSize () / 2L;
	halfSize.x = halfSize.y = std::min (halfSize.x, halfSize.y);

	const float radius = static_cast<float>(halfSize.x);

	const float cos_angle = cos (-m_angle);
	const float sin_angle = sin (-m_angle);

	const int r_cos_angle = static_cast<int>(radius * cos_angle);
	const int r_sin_angle = static_cast<int>(radius * sin_angle);

	//--
	//-- Nx = Ey = -Sx = -Wy =   r * sin -theta
	//-- Ny = Wx = -Sy = -Ex = - r * cos -theta
	//--

	m_compassPoints [CP_N].x = m_compassPoints [CP_E].y =   r_sin_angle;
	m_compassPoints [CP_S].x = m_compassPoints [CP_W].y = - r_sin_angle;

	m_compassPoints [CP_N].y = m_compassPoints [CP_W].x = - r_cos_angle;
	m_compassPoints [CP_S].y = m_compassPoints [CP_E].x =   r_cos_angle;

	const Vector qPoint (-static_cast<float>(halfSize.x), -static_cast<float>(halfSize.y), 0.0f);

	m_quadPoints[0].x = qPoint.x * cos_angle - qPoint.y * sin_angle;
	m_quadPoints[0].y = qPoint.x * sin_angle + qPoint.y * cos_angle;

	m_quadPoints[3].x =   m_quadPoints [0].y;
	m_quadPoints[3].y = - m_quadPoints [0].x;

	m_quadPoints[2].x = - m_quadPoints [0].x;
	m_quadPoints[2].y = - m_quadPoints [0].y;

	m_quadPoints[1].x = - m_quadPoints [0].y;
	m_quadPoints[1].y =   m_quadPoints [0].x;
}

//-----------------------------------------------------------------

CuiWidgetGroundRadar::RadarShaderInfo::RadarShaderInfo () :
m_shader       (0),
m_shaderSize   (128),
m_range        (0),
m_origin       (),
m_worldSize    (0.0f),
m_center       (),
m_rangeString  ()
{
}

//-----------------------------------------------------------------

CuiWidgetGroundRadar::RadarShaderInfo::~RadarShaderInfo ()
{

	if (m_shader)
	{
		m_shader->release ();
		m_shader = 0;
	}
}

//-----------------------------------------------------------------

void      CuiWidgetGroundRadar::RadarShaderInfo::updateUVs ()
{
	if (m_worldSize)
	{
		const Vector transformedCenter (m_center.x - m_origin.x,
			0.0f,
			m_center.z - m_origin.z);

		// the southwest corner of the texture is 0,0
		// so we need to flip the texture vertically

		m_quadUVs [0].x = (transformedCenter.x - m_range) / m_worldSize;
		m_quadUVs [0].y = (transformedCenter.z + m_range) / m_worldSize;
		m_quadUVs [1].x = (transformedCenter.x + m_range) / m_worldSize;
		m_quadUVs [1].y = m_quadUVs[0].y;
		m_quadUVs [2].x = m_quadUVs[1].x;
		m_quadUVs [2].y = (transformedCenter.z - m_range) / m_worldSize;
		m_quadUVs [3].x = m_quadUVs[0].x;
		m_quadUVs [3].y = m_quadUVs[2].y;
	}
}

//-----------------------------------------------------------------

bool CuiWidgetGroundRadar::RadarShaderInfo::update (const Vector & center, float range, const ClientProceduralTerrainAppearance * cmtat, bool force, bool clip, const VectorArgb & clearColor, bool drawTerrain)
{
	if (m_range == range)
	{
		// see if the circle centered at @center with radius @range lies within
		// our valid size

		if (force == false)
		{
			if (!((center.x - range) < m_origin.x ||
				(center.z - range) < m_origin.z ||
				(center.x + range) >= (m_origin.x + m_worldSize) ||
				(center.z + range) >= (m_origin.z + m_worldSize)))
			{
				if (m_center != center)
				{
					m_center = center;
					updateUVs ();
				}
				else
				{
					m_center = center;
				}

				// no need to create a new texture
				return false;
			}
		}
	}

	char buf [32];
	_snprintf (buf, 32, "%2.0f m", range);
	m_rangeString = Unicode::narrowToWide (buf);

	// we must create a new texture

	ClientProceduralTerrainAppearance::RadarShaderInfo rinfo;
	rinfo.m_shader = 0;

	const float requestedRange = range * 1.1f;

	if (cmtat)
		cmtat->createRadarShader (center, requestedRange, m_shaderSize, rinfo, clip, clearColor, drawTerrain);

	if (m_shader)
	{
		m_shader->release ();
		m_shader = 0;
	}

	m_shader       = rinfo.m_shader;
	m_range        = range;
	m_origin       = rinfo.m_origin;
	m_worldSize    = requestedRange * 2.0f;
	m_center       = center;

	updateUVs ();

	return true;
}

//----------------------------------------------------------------------

float CuiWidgetGroundRadar::getPixelsToWorldRatio () const
{
	if (m_radarShaderInfo.m_range == 0.0f)
		return 0.0f;

	return static_cast<float>(GetWidth ()) / (m_radarShaderInfo.m_range * 2.0f);
}

//----------------------------------------------------------------------

bool CuiWidgetGroundRadar::updateRadarShader (const Vector & center, float range, const ClientProceduralTerrainAppearance * cmtat, bool force)
{
	static const UILowerString palette_entry = UILowerString ("back2");

	if (!IsEnabled () && m_shaderIsCleared)
	{
		m_radarShaderInfo.m_origin.x  = center.x - range;
		m_radarShaderInfo.m_origin.z  = center.z - range;
		m_radarShaderInfo.m_center    = center;
		m_radarShaderInfo.m_range     = range;
		return false;
	}

	VectorArgb color = VectorArgb::solidMagenta;
	{
		const UIPalette * const pal = UIPalette::GetInstance ();
		if (pal)
		{
			UIColor uiColor;
			if (pal->FindColor (palette_entry, uiColor))
				color = CuiUtils::convertToVectorArgb (uiColor);
		}
	}

	color = CuiUtils::convertToVectorArgb(UIColor::black);
	if (IsEnabled ())
	{
		m_shaderIsCleared = false;
		return m_radarShaderInfo.update (center, range, cmtat, force, m_clipToCircle, color, true);
	}
	else if (!m_shaderIsCleared)
	{
		m_shaderIsCleared = true;
		return m_radarShaderInfo.update (center, range, cmtat, true, m_clipToCircle, color, false);
	}

	return false;
}

//----------------------------------------------------------------------

void  CuiWidgetGroundRadar::SetEnabled( bool const NewEnabled )
{
	UIWidget::SetEnabled(NewEnabled);
	m_shaderIsCleared = false;
}

//----------------------------------------------------------------------

void  CuiWidgetGroundRadar::setDrawBackground( bool drawBackground )
{
	m_drawBackground = drawBackground;
}

// ======================================================================
