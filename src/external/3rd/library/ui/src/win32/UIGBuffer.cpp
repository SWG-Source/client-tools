#include "_precompile.h"
#include "UIGBuffer.h"
#include "UIGBuffer_Command.h"

#include "UICanvas.h"
#include "UILowerString.h"
#include "UIUtils.h"

#include <math.h>

#include <algorithm>

const char * UIGBuffer::TypeName = "GBuffer";

namespace UIGBufferNamespace
{
	UIFloatPoint s_UIGBufferNullUVs[4];

	class PointerDeleterPairSecond
	{
	public:
		template <typename FirstType, typename SecondType>
			void operator ()(std::pair<FirstType, SecondType> &pairArgument) const
		{
			delete pairArgument.second;
		}
	};
}

using namespace UIGBufferNamespace;

//======================================================================================
UIGBuffer::UIGBuffer() :
mCommandList()
{
}

//======================================================================================
UIGBuffer::~UIGBuffer()
{
	ClearBuffer();
}
												
//======================================================================================
bool UIGBuffer::IsA(const UITypeID Type) const
{
	return (Type == TUIGBuffer) || UIWidget::IsA(Type);
}

//======================================================================================
const char *UIGBuffer::GetTypeName(void) const
{
	return TypeName;
}

//======================================================================================
UIBaseObject *UIGBuffer::Clone(void) const
{
	return new UIGBuffer;
}


//======================================================================================
bool UIGBuffer::CanSelect(void) const
{
	return false;
}

//======================================================================================
bool UIGBuffer::WantsMessage(const UIMessage &) const
{
	return false;
}

//======================================================================================
bool UIGBuffer::ProcessMessage(const UIMessage &)
{
	return true;
}

//======================================================================================
void UIGBuffer::ClearBuffer()
{
	std::for_each(mCommandList.begin(), mCommandList.end(), PointerDeleterPairSecond());
	mCommandList.clear();
}

//======================================================================================
void UIGBuffer::Render(UICanvas & canvas) const
{
	for(UIGBuffer_Command::List::const_reverse_iterator itCommand = mCommandList.rbegin(); itCommand != mCommandList.rend(); ++itCommand)
	{
		itCommand->second->Render(canvas);
	}
}


//======================================================================================
void UIGBuffer::AddQuad(UIPoint const & p1, UIPoint const & p2, UIPoint const & p3, UIPoint const & p4, UIColor const & color,  float const opacity, float const buffer)
{
	UIGBuffer_Command * const cmd = new UIGBuffer_Command();

	cmd->SetType(UIGBuffer_Command::ID_Quad);
	cmd->SetPoints(p1, p2, p3, p4);
	cmd->SetColor(color);
	cmd->SetOpacity(opacity);

	mCommandList.insert(std::make_pair(buffer, cmd));
}

//======================================================================================
void UIGBuffer::AddLine(UIPoint const & p1, UIPoint const & p2, UIColor const & color,  float const opacity, float const buffer)
{
	UIGBuffer_Command * const cmd = new UIGBuffer_Command();
	
	cmd->SetType(UIGBuffer_Command::ID_Quad);
	UIPoint offset(1,1);
	cmd->SetPoints(p1, p1 + offset, p2 + offset, p2);
	cmd->SetColor(color);
	cmd->SetOpacity(opacity);
	
	mCommandList.insert(std::make_pair(buffer, cmd));
}

//======================================================================================
void UIGBuffer::AddWidget(UIWidget * widget, float buffer)
{
	if (widget)
	{
		UIGBuffer_Command * const cmd = new UIGBuffer_Command(widget);
		mCommandList.insert(std::make_pair(buffer, cmd));
	}
}

