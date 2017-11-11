#include "_precompile.h"

#include "UICanvasGenerator.h"

UICanvasGenerator::UICanvasGenerator()
{
}

UICanvasGenerator::~UICanvasGenerator()
{
}

bool UICanvasGenerator::IsA( const UITypeID Type ) const
{
	return (Type == TUICanvasGenerator) || UIBaseObject::IsA( Type );
}
