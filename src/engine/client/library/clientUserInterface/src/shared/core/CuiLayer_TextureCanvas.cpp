//======================================================================
//
// CuiLayer_TextureCanvas.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiLayer_TextureCanvas.h"

#include "clientGraphics/Graphics.h"
#include "clientGraphics/Shader.h"
#include "clientGraphics/ShaderTemplate.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/StaticShader.h"
#include "clientGraphics/Texture.h"
#include "clientGraphics/TextureList.h"
#include "clientUserInterface/CuiLayerRenderer.h"
#include "sharedDebug/DebugFlags.h"

//======================================================================

namespace
{	
	bool                   s_debugReportCanvases     = false;
	bool                   s_debugReportCanvasesInit = false;

	const char * const UICANVAS_SHT            = "shader/uicanvas_filtered.sht";

	inline const StaticShader * loadUIShader (const std::string & shaderName, const std::string & textureName, UISize & size)
	{
		std::string fname;
		
		const StaticShader * shader = 0;
		const Texture * texture = 0;	
		
		const ShaderTemplate * const shaderTemplate = ShaderTemplateList::fetch (shaderName.empty () ? UICANVAS_SHT : shaderName.c_str ());
		
		if (shaderTemplate)
		{
			StaticShader * const modShader = safe_cast<StaticShader *>(NON_NULL (shaderTemplate->fetchModifiableShader ()));
			
			if (!textureName.empty ())
			{
				texture = TextureList::fetch (textureName.c_str ());
				
				if (texture)
				{
					modShader->setTexture(TAG(M,A,I,N), *texture); //lint !e1774
					shader = modShader;
				}
				else
					WARNING (true, ("No such UI texture: '%s'", textureName.c_str ()));
			}
			else
			{
				shader = modShader;
				modShader->getTexture(TAG (M,A,I,N), texture);
				if (texture)
					texture->fetch ();
			}

			shaderTemplate->release ();
		}
		else
			WARNING (true, ("No such UI shader template: '%s'", shaderName.c_str ()));

		if (texture)
		{
			//-- extract size
			size.x = texture->getWidth ();
			size.y = texture->getHeight ();			
			texture->release();
		}
		else
		{
			WARNING (true, ("UI shader with no texture: %s:%s", textureName.c_str (), shaderName.c_str ()));
		}

		return shader;
	}
}

//----------------------------------------------------------------------

float CuiLayer::TextureCanvas::ms_currentTime  = 0.0f;
int   CuiLayer::TextureCanvas::ms_currentFrame = 0;

//-----------------------------------------------------------------
// TextureCanvas
//-----------------------------------------------------------------

CuiLayer::TextureCanvas::TextureCanvas (const std::string & shaderName, const std::string & textureName) :
UICanvas       (UISize ()),
m_shaderName   (shaderName),
m_textureName  (textureName),
m_shader       (0),
m_initialized  (false),
m_lastTimeUsed (0.0f),
m_lastFrameUsed (0)
{
	if (!s_debugReportCanvasesInit)
	{
		s_debugReportCanvasesInit = true;
		DebugFlags::registerFlag(s_debugReportCanvases, "ClientUserInterface", "canvases");
	}

	if (!m_shaderName.empty ())
		m_shaderName = "shader/" + m_shaderName + ".sht";

	if (!m_textureName.empty ())
		m_textureName = "texture/" + m_textureName + ".dds";
}

//-------------------------------------------------------------------

CuiLayer::TextureCanvas::~TextureCanvas ()
{
	if (m_shader)
	{
		CuiLayerRenderer::flushRenderQueueIfCurShader (*m_shader);
		m_shader->release();
		m_shader = 0;
	}
}

//-------------------------------------------------------------------

/**
* initShader will simply re-initialize the shader if called more than once.
* It will properly release the old shader.
*/
void CuiLayer::TextureCanvas::initShader () const
{
	if (m_shader)
	{
		CuiLayerRenderer::flushRenderQueueIfCurShader (*m_shader);
		m_shader->release();
		m_shader = 0;
	}

	UISize s;
	m_shader = loadUIShader (m_shaderName, m_textureName, s);

	if (m_shader)
		const_cast<TextureCanvas *>(this)->SetSize (s);

	m_initialized = true;
	REPORT_LOG_PRINT (s_debugReportCanvases, (":: Cui initialized shader %s:%s\n", m_shaderName.c_str (), m_textureName.c_str ()));
}

//-----------------------------------------------------------------

/**
* releases the shader, sets the initialized flag to false
*/
void CuiLayer::TextureCanvas::unloadShader ()
{
	if (m_shader)
	{
		CuiLayerRenderer::flushRenderQueueIfCurShader (*m_shader);
		m_shader->release();
		m_shader = 0;
	}

	m_initialized = false;
	REPORT_LOG_PRINT (s_debugReportCanvases, (":: Cui unloaded shader %s:%s\n", m_shaderName.c_str (), m_textureName.c_str ()));
}

//-----------------------------------------------------------------

void CuiLayer::TextureCanvas::setCurrentTime (float time, int frame)
{
	ms_currentTime  = time;
	ms_currentFrame = frame;
}

//-----------------------------------------------------------------

void CuiLayer::TextureCanvas::Reload(UINarrowString const & newTextureName)
{
	if(m_textureName != newTextureName)
	{
		m_textureName = newTextureName;
		initShader();

	}
}
//======================================================================
