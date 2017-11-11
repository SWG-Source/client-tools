// ======================================================================
//
// VertexBufferFormat.cpp
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/VertexBufferFormat.h"

#include <string>

// ======================================================================

void VertexBufferFormat::formatFormat(std::string &result) const
{
	if (hasPosition())
		result += "position|";

	if (isTransformed())
		result += "transformed|";

	if (hasNormal())
		result += "normal|";

	if (hasPointSize())
		result += "pointSize|";

	if (hasColor0())
		result += "color0|";
	
	if (hasColor1())
		result += "color1|";

	int i;
	for (i = 0; i < getNumberOfTextureCoordinateSets(); ++i)
	{
		char buffer[10];
		result += _itoa(getTextureCoordinateSetDimension(i), buffer, 10);
		result += "|";
	}
}

// ======================================================================

