// ======================================================================
//
// SwgCuiTcgControl.cpp
// copyright (c) 2008 Sony Online Entertainment LLC
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiTcgControl.h"

#include "libEverQuestTCG/libEverQuestTCG.h"

#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/StaticShader.h"
#include "clientGraphics/Texture.def"
#include "clientGraphics/Texture.h"
#include "clientGraphics/TextureList.h"
#include "clientUserInterface/CuiLayer.h"
#include "clientUserInterface/CuiLayer_TextureCanvas.h"

#include "UICanvas.h"
#include "UIImage.h"
#include "UIMessage.h"

// ======================================================================

SwgCuiTcgControl::SwgCuiTcgControl()
: UIWidget()
, m_eqTcgWindow(0)
, m_image(0)
, m_texture(0)
{
}

// ----------------------------------------------------------------------

SwgCuiTcgControl::~SwgCuiTcgControl()
{
	m_eqTcgWindow = 0;
	m_image = 0;
	m_texture = 0;
}

// ----------------------------------------------------------------------

void SwgCuiTcgControl::alter(float deltaTime)
{
	UNREF(deltaTime);

	unsigned numWindows = libEverQuestTCG::getWindows(0, 0);

	if (m_eqTcgWindow && numWindows < 1)
	{
		m_eqTcgWindow = 0;
		libEverQuestTCG::release();
		return;
	}

	fetchTexture();
}

// ----------------------------------------------------------------------

void SwgCuiTcgControl::Render(UICanvas & canvas) const
{
	UNREF(canvas);

	if (!m_eqTcgWindow || !m_image || !m_texture)
	{
		DEBUG_REPORT_PRINT(!m_eqTcgWindow, ("SwgCuiTcgControl::Render() - m_eqTcgWindow is null\n"));
		DEBUG_REPORT_PRINT(!m_image, ("SwgCuiTcgControl::Render() - m_image is null\n"));
		DEBUG_REPORT_PRINT(!m_texture, ("SwgCuiTcgControl::Render() - m_texture is null\n"));
		return;
	}

	char * sourceBits = 0;
	unsigned sourceWidth = 0;
	unsigned sourceHeight = 0;
	unsigned sourceStride = 0;

	bool const result = m_eqTcgWindow->getWindowSurfaceData(reinterpret_cast<void **>(&sourceBits), &sourceWidth, &sourceHeight, &sourceStride);

	if (result)
	{
		Texture::LockData lockData(TF_XRGB_8888, 0, 0, 0, m_texture->getWidth(), m_texture->getHeight(), true);

		m_texture->lock(lockData);

		char * textureData = static_cast<char *>(lockData.getPixelData());

		if (sourceHeight/sourceStride == 4)
			memcpy(textureData, sourceBits, sourceStride * sourceHeight);
		else
		{
			unsigned int const destStride = lockData.getPitch();
			int const copyLength = destStride < sourceStride ? destStride : sourceStride;
			unsigned int const copyHeight = static_cast<unsigned int>(m_texture->getHeight()) < sourceHeight ? static_cast<unsigned int>(m_texture->getHeight()) : sourceHeight;
			for (unsigned int i = 0; i < copyHeight; ++i)
			{
				memcpy(textureData + i * destStride, sourceBits + i * sourceStride, copyLength);
			}
		}

		m_texture->unlock(lockData);

		CuiLayer::TextureCanvas const * constCanvas = safe_cast<CuiLayer::TextureCanvas const *>(m_image->GetCanvas());

		CuiLayer::TextureCanvas * textureCanvas = const_cast<CuiLayer::TextureCanvas *>(constCanvas);

		if (textureCanvas)
		{
			ShaderTemplate const * const shaderTemplate = ShaderTemplateList::fetch("shader/uicanvas_filtered.sht");

			NOT_NULL(shaderTemplate);

			if (!shaderTemplate)
				return;

			StaticShader * const newShader = safe_cast<StaticShader *>(NON_NULL(shaderTemplate->fetchModifiableShader()));

			if (!newShader)
				return;

			newShader->setTexture(TAG(M,A,I,N), *m_texture);

			textureCanvas->SetSize(UISize(GetWidth(), GetHeight()));

			if (textureCanvas->getShader() != newShader)
				textureCanvas->SetShader(newShader);

			textureCanvas->SetTextureName(m_texture->getName());

			canvas.BltFrom(textureCanvas, UIPoint::zero, UIPoint::zero, GetSize());

			shaderTemplate->release();
		}
	}
}

// ----------------------------------------------------------------------

UIBaseObject * SwgCuiTcgControl::Clone() const
{
	return new SwgCuiTcgControl;
}

// ----------------------------------------------------------------------

UIStyle * SwgCuiTcgControl::GetStyle() const
{
	return 0;
}

// ----------------------------------------------------------------------

void SwgCuiTcgControl::setImage(UIImage * image)
{
	m_image = image;

	if (m_image)
		m_image->SetVisible(false);
}

// ----------------------------------------------------------------------

void SwgCuiTcgControl::OnSizeChanged(const UISize &newSize, const UISize &oldSize)
{
	UIWidget::OnSizeChanged(newSize, oldSize);

	if (m_eqTcgWindow)
	{
		m_eqTcgWindow->setSize(GetWidth(), GetHeight());
	}

	if (m_image)
		m_image->SetSize(UIPoint(GetWidth(), GetHeight()));

	if (m_texture)
	{
		m_texture->release();
		m_texture = NULL;
	}
	fetchTexture();
}

// ----------------------------------------------------------------------

void SwgCuiTcgControl::initializeEqTcgWindow()
{
	if (m_eqTcgWindow)
		return;

	// use the first window
	unsigned numWindows = libEverQuestTCG::getWindows(0, 0);

	libEverQuestTCG::Window ** windows = 0;

	if (numWindows > 0)
	{
		windows = static_cast<libEverQuestTCG::Window **>(alloca(numWindows * sizeof(libEverQuestTCG::Window *)));
		libEverQuestTCG::getWindows(windows, numWindows);

		if (windows && windows[0])
		{
			m_eqTcgWindow = windows[0];
			m_eqTcgWindow->setSize(GetSize().x, GetSize().y);

			alter(0);
		}
	}
}

// ----------------------------------------------------------------------

bool SwgCuiTcgControl::ProcessMessage(const UIMessage & msg)
{
	if (m_eqTcgWindow)
	{
		if (IsSelected())
			m_eqTcgWindow->setFocus(true);

		unsigned int flags = 0;

		if (msg.Modifiers.isAltDown())
			flags |= libEverQuestTCG::Window::ALT;
		if (msg.Modifiers.isControlDown())
			flags |= libEverQuestTCG::Window::CONTROL;
		if (msg.Modifiers.isShiftDown())
			flags |= libEverQuestTCG::Window::SHIFT;
		if (msg.Modifiers.LeftMouseDown)
			flags |= libEverQuestTCG::Window::LeftButton;
		if (msg.Modifiers.RightMouseDown)
			flags |= libEverQuestTCG::Window::RightButton;
		if (msg.Modifiers.MiddleMouseDown)
			flags |= libEverQuestTCG::Window::MiddleButton;

		int const x = msg.MouseCoords.x;
		int const y = msg.MouseCoords.y;
		UIPoint gpos = GetWorldLocation();
		int const gx = x + gpos.x;
		int const gy = y + gpos.y;

#ifdef _DEBUG
		bool const enablePrintLog = false;
#endif

		DEBUG_REPORT_PRINT(enablePrintLog, ("mouse.x = %d, mouse.y = %d\n", x, y));
		DEBUG_REPORT_PRINT(enablePrintLog, ("gpos.x = %d, gpos.y = %d\n", gpos.x, gpos.y));
		DEBUG_REPORT_PRINT(enablePrintLog, ("gx = %d, gy = %d\n", gx, gy));

		switch (msg.Type)
		{
		case UIMessage::MouseMove:
			{
				DEBUG_REPORT_LOG(enablePrintLog, ("%s got a Mouse Move Down Message. Params: X = %i Y= %i gx = %i gy = %i\n", GetName().c_str(), x, y, gx, gy));
				m_eqTcgWindow->onMouseMove(x, y, gx, gy, flags);
				return true;
			}
			break;
		case UIMessage::LeftMouseDown:
			{
				DEBUG_REPORT_LOG(enablePrintLog, ("%s got a Left Mouse Down Message. Params: X = %i Y= %i gx = %i gy = %i\n", GetName().c_str(), x, y, gx, gy));
				m_eqTcgWindow->onLeftMouseDown(x, y, gx, gy, flags);
				return true;
			}
			break;
		case UIMessage::LeftMouseUp:
			{
				m_eqTcgWindow->onLeftMouseUp(x, y, gx, gy, flags);
				return true;
			}
			break;
		case UIMessage::MiddleMouseDown:
			{
				m_eqTcgWindow->onMiddleMouseDown(x, y, gx, gy, flags);
				return true;
			}
			break;
		case UIMessage::MiddleMouseUp:
			{
				m_eqTcgWindow->onMiddleMouseUp(x, y, gx, gy, flags);
				return true;
			}
			break;
		case UIMessage::RightMouseDown:
			{
				DEBUG_REPORT_LOG(enablePrintLog, ("%s got a Right Mouse Down Message. Params: X = %i Y= %i gx = %i gy = %i\n", GetName().c_str(), x, y, gx, gy));
				m_eqTcgWindow->onRightMouseDown(x, y, gx, gy, flags);
				return true;
			}
			break;
		case UIMessage::RightMouseUp:
			{
				m_eqTcgWindow->onRightMouseUp(x, y, gx, gy, flags);
				return true;
			}
			break;
		case UIMessage::MouseWheel:
			{
				m_eqTcgWindow->onMouseWheel(x, y, gx, gy, msg.Data * 16, flags);
				return true;
			}
			break;
		case UIMessage::LeftMouseDoubleClick:
			{
				m_eqTcgWindow->onLeftMouseDoubleClick(x, y, gx, gy, flags);
				return true;
			}
			break;
		case UIMessage::RightMouseDoubleClick:
			{
				m_eqTcgWindow->onRightMouseDoubleClick(x, y, gx, gy, flags);
				return true;
			}
			break;
		case UIMessage::MiddleMouseDoubleClick:
			{
				m_eqTcgWindow->onMiddleMouseDoubleClick(x, y, gx, gy, flags);
				return true;
			}
			break;
		case UIMessage::KeyUp:
		case UIMessage::KeyDown:
		case UIMessage::KeyRepeat:
		case UIMessage::Character:
			{
				int key = msg.Keystroke;

				if (key == UIMessage::Escape)
					key = libEverQuestTCG::Key_Escape;
				else if (key == UIMessage::Tab)
					key = libEverQuestTCG::Key_Tab;
				else if (key == UIMessage::BackSpace)
					key = libEverQuestTCG::Key_Backspace;
				else if (key == UIMessage::Enter)
					key = libEverQuestTCG::Key_Enter;
				else if (key == UIMessage::Insert)
					key = libEverQuestTCG::Key_Insert;     
				else if (key == UIMessage::Delete)
					key = libEverQuestTCG::Key_Delete;
				else if (key == UIMessage::BackSpace)
					key = libEverQuestTCG::Key_Clear;      
				else if (key == UIMessage::Home)
					key = libEverQuestTCG::Key_Home;       
				else if (key == UIMessage::End)
					key = libEverQuestTCG::Key_End;        
				else if (key == UIMessage::LeftArrow)
					key = libEverQuestTCG::Key_Left;       
				else if (key == UIMessage::UpArrow)
					key = libEverQuestTCG::Key_Up;         
				else if (key == UIMessage::RightArrow)
					key = libEverQuestTCG::Key_Right;      
				else if (key == UIMessage::DownArrow)
					key = libEverQuestTCG::Key_Down;       
				else if (key == UIMessage::PageUp)
					key = libEverQuestTCG::Key_PageUp;     
				else if (key == UIMessage::PageDown)
					key = libEverQuestTCG::Key_PageDown;

				if (msg.Type == UIMessage::KeyUp)
					m_eqTcgWindow->onKeyUp(key, flags, 0, 0, 0);
				else
				{
					bool const accepted = m_eqTcgWindow->onKeyDown(key, flags, 0, 0, 0);
					UNREF(accepted);

					DEBUG_REPORT_LOG(enablePrintLog, ("onKeyDown - [%s]\n", accepted ? "accepted" : "not accepted"));
				}

				return true;
			}
		default:
			return UIWidget::ProcessMessage(msg);
		}

		return true;
	}

	return UIWidget::ProcessMessage(msg);
}

// ----------------------------------------------------------------------

void SwgCuiTcgControl::SetSelected(const bool selected)
{		
	if (IsSelected() != selected && m_eqTcgWindow && m_eqTcgWindow->canGetFocus())
	{
		UIWidget::SetSelected(selected);
		m_eqTcgWindow->setFocus(selected);
	}
}

// ----------------------------------------------------------------------

void SwgCuiTcgControl::fetchTexture()
{
	if (!m_texture)
	{
		TextureFormat const runtimeFormats[] = {TF_XRGB_8888};
		int const numberOfRuntimeFormats = sizeof(runtimeFormats) / sizeof(runtimeFormats[0]);

		m_texture = const_cast<Texture *>(TextureList::fetch(static_cast<int>(0), GetWidth(), GetHeight(), 1, runtimeFormats, numberOfRuntimeFormats));
	}
}

// ----------------------------------------------------------------------

void SwgCuiTcgControl::OnLocationChanged(const UIPoint &newLocation, const UIPoint &oldLocation)
{
	UIWidget::OnLocationChanged(newLocation, oldLocation);
}

// ======================================================================
