//======================================================================
//
// CuiLayer_TextureCanvas.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiLayer_TextureCanvas_H
#define INCLUDED_CuiLayer_TextureCanvas_H

#include "clientGraphics/StaticShader.h"
#include "clientGraphics/Texture.h"
#include "clientUserInterface/CuiLayer.h"
#include "UICanvas.h"

//======================================================================

class CuiLayer::TextureCanvas : public UICanvas
{
public:
	
	explicit           TextureCanvas   (const std::string & shaderName, const std::string & textureName);
	~TextureCanvas   ();
	
	void                 GetSize         (UISize &) const;
	long                 GetWidth        () const;
	long                 GetHeight       () const;
	
	
	UIBaseObject *       Clone           () const { return 0; };
	const StaticShader * getShader       () const;
	
	void                 unloadShader     ();
	float                getLastTimeUsed  ();
	int                  getLastFrameUsed ();
	
	static void          setCurrentTime   (float time, int frame);
	
	bool                 isInitialized   () const;
	
	virtual void Reload(UINarrowString const & newTextureName);

	void				 SetShader        (const StaticShader* newShader);
	void				 SetTextureName   (std::string textureName);


private:
	
	TextureCanvas   (const TextureCanvas &);
	TextureCanvas &    operator=       (const TextureCanvas &);
	void               initShader      () const;
	
private:
	
	std::string                  m_shaderName;
	std::string                  m_textureName;
	
	mutable const StaticShader * m_shader;
	mutable bool                 m_initialized;
	mutable float                m_lastTimeUsed;
	mutable int                  m_lastFrameUsed;
	
	static float                 ms_currentTime;
	static int                   ms_currentFrame;
};

//-----------------------------------------------------------------

inline const StaticShader* CuiLayer::TextureCanvas::getShader (void) const
{
	if (!m_initialized)
		initShader ();

	m_lastTimeUsed  = ms_currentTime;
	m_lastFrameUsed = ms_currentFrame;
	return m_shader;

}

//-----------------------------------------------------------------

inline void CuiLayer::TextureCanvas::GetSize( UISize & s) const
{
	if (!m_initialized)
		initShader ();

	UICanvas::GetSize (s);
}

//-----------------------------------------------------------------

inline long CuiLayer::TextureCanvas::GetWidth( void ) const
{
	if (!m_initialized)
		initShader ();

	return UICanvas::GetWidth ();
}

//-----------------------------------------------------------------

inline long CuiLayer::TextureCanvas::GetHeight( void ) const
{
	if (!m_initialized)
		initShader ();

	return UICanvas::GetHeight ();
}

//-----------------------------------------------------------------

inline float CuiLayer::TextureCanvas::getLastTimeUsed ()
{
	return m_lastTimeUsed;
}

//----------------------------------------------------------------------

inline int CuiLayer::TextureCanvas::getLastFrameUsed ()
{
	return m_lastFrameUsed;
}

//-----------------------------------------------------------------

inline bool CuiLayer::TextureCanvas::isInitialized () const
{
	return m_initialized;
}

inline void CuiLayer::TextureCanvas::SetShader(const StaticShader *newShader)
{
	if(m_shader)
		m_shader->release();

	m_shader = newShader;
	m_shaderName = newShader->getName();
	m_initialized = true;
}

inline void CuiLayer::TextureCanvas::SetTextureName(std::string textureName)
{
	m_textureName = textureName;
}
//======================================================================

#endif
