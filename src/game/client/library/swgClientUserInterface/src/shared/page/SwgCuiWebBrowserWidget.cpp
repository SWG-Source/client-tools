// ======================================================================
//
// SwgCuiWebBrowserWidget.cpp
// copyright (c) 2008 Sony Online Entertainment LLC
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiWebBrowserWidget.h"


#include "clientGraphics/Graphics.h"
#include "clientGraphics/StaticShader.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/DynamicVertexBuffer.h"
#include "clientGraphics/Texture.h"
#include "clientGraphics/Texture.def"
#include "clientGraphics/TextureList.h"
#include "clientUserInterface/CuiLayer_TextureCanvas.h"

#include "sharedFoundation/Os.h"

#include "swgClientUserInterface/SwgCuiChatWindow.h"
#include "swgClientUserInterface/SwgCuiHud.h"
#include "swgClientUserInterface/SwgCuiHudFactory.h"
#include "swgClientUserInterface/SwgCuiWebBrowserWindow.h"
#include "libMozilla/libMozilla.h"
#include "UnicodeUtils.h"

#include "UICanvas.h"
#include "UIImage.h"
#include "UIPage.h"
#include "UIMessage.h"

// ======================================================================

namespace browserNamespace
{
	std::string s_homePage = "www.StarWarsGalaxies.com";
}

using namespace browserNamespace;

class UIMozillaCallbacks : public libMozilla::ICallback
{
	SwgCuiWebBrowserWidget *m_MozillaWidget;

public:

	UIMozillaCallbacks( SwgCuiWebBrowserWidget *mozillaWidget ) :
	m_MozillaWidget( mozillaWidget )
	{

	}

	virtual void onURIChanged( libMozilla::Window * )
	{
		m_MozillaWidget->updateURI();
	}
	virtual void onProgressChanged( libMozilla::Window * )
	{
		m_MozillaWidget->updateProgress();
	}
	virtual void onStatusChanged( libMozilla::Window * )
	{
		m_MozillaWidget->updateStatus();
	}
	virtual bool doValidateURI( libMozilla::Window *, const char *pURI )
	{
		return m_MozillaWidget->validateURI( pURI );
	}
};

class UIMozillaCanvasBlitter : public libMozilla::IBlitter
{
	Texture *m_pTexture;

public:

	UIMozillaCanvasBlitter(Texture* texture)
		: m_pTexture( texture )
	{
	}

	void operator()( void *pSource, unsigned /*uSourceWidth*/, unsigned uSourceHeight, unsigned uSourceStride, unsigned /*uSourceBytesPerRow*/ )
	{
		Texture::LockData lockData(TF_XRGB_8888, 0, 0, 0, m_pTexture->getWidth(), m_pTexture->getHeight(), true);

		m_pTexture->lock(lockData);

		char* textureData = static_cast<char*>(lockData.getPixelData());

		if(uSourceHeight/uSourceStride == 4)
			memcpy(textureData, static_cast<char*>(pSource), uSourceStride * uSourceHeight );
		else
		{
			unsigned int destStride = lockData.getPitch();
			int copyLength = destStride < uSourceStride ? destStride : uSourceStride;
			for(unsigned int i = 0; i < uSourceHeight; ++i)
			{
				memcpy(textureData + i * destStride, static_cast<char*>(pSource) + i * uSourceStride, copyLength);
			}
		}

		m_pTexture->unlock(lockData);

	}

};

SwgCuiWebBrowserWidget::SwgCuiWebBrowserWidget()
: UIWidget(),
m_MozillaWindow(NULL),
m_Callbacks(NULL),
m_Texture(NULL),
m_Canvas(NULL),
m_Image(NULL),
m_Text(NULL),
m_Shader (NULL),
m_caratBlink (0.5f),
m_drawCarat (false)
{


	//shaderTemplate->release();

}

// ----------------------------------------------------------------------

SwgCuiWebBrowserWidget::~SwgCuiWebBrowserWidget()
{
	if(m_MozillaWindow)
	{
		m_MozillaWindow->setCallback(0);
		libMozilla::destroyWindow(m_MozillaWindow);
		m_MozillaWindow = NULL;
	}

	if(m_Callbacks)
	{
		delete m_Callbacks;
		m_Callbacks = NULL;
	}
}

void SwgCuiWebBrowserWidget::alter(float deltaTime)
{
	if(!m_Texture)
	{
		const TextureFormat  runtimeFormats[] = { TF_XRGB_8888};
		const int numberOfRuntimeFormats = sizeof(runtimeFormats) / sizeof(runtimeFormats[0]);

		m_Texture = const_cast<Texture*>(TextureList::fetch(static_cast<int>(0), GetWidth(), GetHeight(), 1, runtimeFormats, numberOfRuntimeFormats));
	}

	if(!m_MozillaWindow)
		m_MozillaWindow = getMozillaWindow();

	if(m_caratBlink.updateZero(deltaTime))
	{
		m_drawCarat = !m_drawCarat;
	}
}
// ----------------------------------------------------------------------

void SwgCuiWebBrowserWidget::Render(UICanvas & canvas) const
{
	UNREF(canvas);
	
	UICanvas * sourceCanvas = &canvas;

	if (sourceCanvas && m_Texture)
	{
		const CuiLayer::TextureCanvas* constCanvas = safe_cast< const CuiLayer::TextureCanvas *>(m_Image->GetCanvas());

		CuiLayer::TextureCanvas * textureCanvas = const_cast<CuiLayer::TextureCanvas*>(constCanvas);

		UIMozillaCanvasBlitter blit(m_Texture);

		if(m_MozillaWindow && textureCanvas)
		{
			bool renderComplete = m_MozillaWindow->render(&blit);
			
			if(renderComplete)
			{

				const ShaderTemplate * const shaderTemplate = ShaderTemplateList::fetch("shader/uicanvas_filtered.sht");

				NOT_NULL(shaderTemplate);

				StaticShader * const newShader = safe_cast<StaticShader *>(NON_NULL (shaderTemplate->fetchModifiableShader ()));

				newShader->setTexture(TAG(M,A,I,N), *m_Texture);

				textureCanvas->SetSize(UISize(GetWidth(), GetHeight()));

				if(textureCanvas->getShader() != newShader)
					textureCanvas->SetShader(newShader);

				textureCanvas->SetTextureName(m_Texture->getName());

				canvas.BltFrom(textureCanvas, UIPoint::zero, UIPoint::zero, GetSize());

				if( IsSelected() && m_drawCarat)
				{
					int x, y, width, height;
					if( m_MozillaWindow->getCaret( x, y, width, height ) )
					{
						canvas.ClearTo( UIColor::black, UIRect( x, y, x + width, y + height ) );
					}
				}

				shaderTemplate->release();
			}
			else
			{
				canvas.BltFrom(0, UIPoint::zero, UIPoint::zero, GetSize());
			}

		}

	}

}

// ----------------------------------------------------------------------

UIBaseObject * SwgCuiWebBrowserWidget::Clone() const
{
	return new SwgCuiWebBrowserWidget;
}

// ----------------------------------------------------------------------

UIStyle * SwgCuiWebBrowserWidget::GetStyle() const
{
	return 0;
}

libMozilla::Window* SwgCuiWebBrowserWidget::getMozillaWindow()
{
	if(!m_MozillaWindow)
	{
		m_MozillaWindow = libMozilla::createWindow(GetWidth(), GetHeight());
		
		if(m_MozillaWindow)
		{
			if(!m_Callbacks)
				m_Callbacks = new UIMozillaCallbacks(this);

			m_MozillaWindow->setCallback(m_Callbacks);

			m_MozillaWindow->navigateTo(Unicode::narrowToWide(s_homePage).c_str());
		}

	}

	return m_MozillaWindow;
}

void SwgCuiWebBrowserWidget::updateProgress()
{

}

void SwgCuiWebBrowserWidget::updateStatus()
{

}

void SwgCuiWebBrowserWidget::updateURI()
{
	if(m_Text && m_MozillaWindow)
	{
		m_Text->SetLocalText(Unicode::narrowToWide(m_MozillaWindow->getURI()));
	}
}

bool SwgCuiWebBrowserWidget::validateURI(const char* /*uri*/)
{
	return true;
}

void SwgCuiWebBrowserWidget::setUIImage(UIImage* image)
{
	m_Image = image;
	image->SetVisible(false);
}

void SwgCuiWebBrowserWidget::debugOutput()
{
	if(!m_MozillaWindow)
		return;

	DEBUG_REPORT_LOG(true, ("Mozilla Browser Debug:\n\n"));
	bool loading;
	float progress;
	progress = m_MozillaWindow->getProgress(loading);

	DEBUG_REPORT_LOG(true, ("Current URI: %s\nLoading: %d\nProgress: %-3.1f\n", m_MozillaWindow->getURI(), static_cast<int>(loading), progress));
}

void SwgCuiWebBrowserWidget::setURL( std::string url, const char * postData, int postDataLength)
{
	if(!m_MozillaWindow)
		return;

	m_MozillaWindow->navigateTo(Unicode::narrowToWide(url).c_str(), postData, postDataLength);

}

void SwgCuiWebBrowserWidget::OnSizeChanged(const UISize &newSize, const UISize &oldSize)
{
	UIWidget::OnSizeChanged(newSize, oldSize);

	if(m_MozillaWindow)
		m_MozillaWindow->setSize(GetWidth(), GetHeight());

	if(m_Image)
		m_Image->SetSize(UIPoint(GetWidth(), GetHeight()));

	if(m_Texture)
	{
		m_Texture->release();
		m_Texture = NULL;
	}

	if(!m_Texture)
	{
		const TextureFormat  runtimeFormats[] = { TF_XRGB_8888};
		const int numberOfRuntimeFormats = sizeof(runtimeFormats) / sizeof(runtimeFormats[0]);

		m_Texture = const_cast<Texture*>(TextureList::fetch(static_cast<int>(0), GetWidth(), GetHeight(), 1, runtimeFormats, numberOfRuntimeFormats));
	}

}

bool SwgCuiWebBrowserWidget::ProcessMessage(const UIMessage & msg)
{
	if (UIWidget::ProcessMessage( msg ))
		return true;

	if(m_MozillaWindow)
	{
		unsigned int Flags = 0;

		if(msg.Modifiers.isAltDown())
			Flags |= libMozilla::Window::ALT;
		if(msg.Modifiers.isControlDown())
			Flags |= libMozilla::Window::CONTROL;
		if(msg.Modifiers.isShiftDown())
			Flags |= libMozilla::Window::SHIFT;

		switch(msg.Type)
		{
		case UIMessage::MouseMove:
			{
				m_MozillaWindow->onMouseMove(msg.MouseCoords.x, msg.MouseCoords.y, Flags);
				return true;
			}
			break;
		case UIMessage::LeftMouseDown:
			{
				m_MozillaWindow->onLeftMouseDown(msg.MouseCoords.x, msg.MouseCoords.y, Flags);
				return true;
			}
			break;
		case UIMessage::LeftMouseUp:
			{
				m_MozillaWindow->onLeftMouseUp(msg.MouseCoords.x, msg.MouseCoords.y, Flags);
				return true;
			}
			break;
		case UIMessage::MiddleMouseDown:
			{
				m_MozillaWindow->onMiddleMouseDown(msg.MouseCoords.x, msg.MouseCoords.y, Flags);
				return true;
			}
			break;
		case UIMessage::MiddleMouseUp:
			{
				m_MozillaWindow->onMiddleMouseUp(msg.MouseCoords.x, msg.MouseCoords.y, Flags);
				return true;
			}
			break;
		case UIMessage::RightMouseDown:
			{
				m_MozillaWindow->onRightMouseDown(msg.MouseCoords.x, msg.MouseCoords.y, Flags);
				return true;
			}
			break;
		case UIMessage::RightMouseUp:
			{
				m_MozillaWindow->onRightMouseUp(msg.MouseCoords.x, msg.MouseCoords.y, Flags);
				return true;
			}
			break;
		case UIMessage::MouseWheel:
			{
				m_MozillaWindow->onMouseWheel(msg.Data, Flags);
				return true;
			}
			break;

		case UIMessage::KeyUp:
			return true;

		case UIMessage::KeyDown:
		case UIMessage::KeyRepeat:
			{
				m_MozillaWindow->onKeyPress( 0, 0xFFFF & msg.Keystroke, Flags );
				return true;
			}
		default:
			break;
		}
	}

	return true;
}

void SwgCuiWebBrowserWidget::SetSelected(const bool selected)
{		
	if(IsSelected() != selected)
	{
		UIWidget::SetSelected(selected);

		if(m_MozillaWindow)
			m_MozillaWindow->setFocus(selected);

		if(!selected)
			::SetFocus(Os::getWindow());
	}
}

void SwgCuiWebBrowserWidget::NavigateForward()
{
	if(m_MozillaWindow && m_MozillaWindow->canNavigateForward())
		m_MozillaWindow->navigateForward();
		
}

void SwgCuiWebBrowserWidget::NavigateBack()
{
	if(m_MozillaWindow && m_MozillaWindow->canNavigateBack())
		m_MozillaWindow->navigateBack();
}

void SwgCuiWebBrowserWidget::NavigateStop()
{
	if(m_MozillaWindow)
		m_MozillaWindow->navigateStop();
}

void SwgCuiWebBrowserWidget::createMozillaWindow()
{
   if(!m_MozillaWindow)
	   m_MozillaWindow = getMozillaWindow();
}

void SwgCuiWebBrowserWidget::RefreshPage()
{
	if(m_MozillaWindow)
		m_MozillaWindow->reload();
}

void SwgCuiWebBrowserWidget::setHomePage(std::string const & home)
{
	s_homePage = home;
}

std::string SwgCuiWebBrowserWidget::getCurrentURL() const
{
	if(m_MozillaWindow)
	{
		return std::string(m_MozillaWindow->getURI());
	}

	return std::string();
}
// ======================================================================
