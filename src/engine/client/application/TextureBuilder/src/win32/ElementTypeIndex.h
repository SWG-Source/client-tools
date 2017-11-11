// ======================================================================
//
// ElementTypeIndex.h
// Copyright 2001, 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ElementTypeIndex_H
#define INCLUDED_ElementTypeIndex_H

// ======================================================================

enum // ElementTypeIndex
{
	ETI_unknown,
	ETI_textureGroup,
	ETI_textureSingle,
	ETI_textureArray1d,
	ETI_textureArray2d,
	ETI_commandGroup,
	ETI_commandClearScreen,
	ETI_commandDrawTexture,
	ETI_imageSlot,
	ETI_destinationTexture,
	ETI_componentAttribute,
	ETI_variableInt,
	ETI_textureSourceMode,
	ETI_textureWriteMode,
	ETI_variableDefinition,
	ETI_textureArray1dElement,
	ETI_region,
	ETI_bool,
	ETI_hue,
	ETI_hueMode,
	ETI_path
};

// ======================================================================

#endif
