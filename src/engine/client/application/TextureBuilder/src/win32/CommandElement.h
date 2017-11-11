// ======================================================================
//
// CommandElement.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef COMMAND_ELEMENT_H
#define COMMAND_ELEMENT_H

// ======================================================================

#include "VariableContainerElement.h"

class Exporter;
class Texture;
class CustomizationData;

// ======================================================================
/**
 * Common base class for all commands.
 */

class CommandElement: public VariableContainerElement
{
public:

	virtual void applyCommand(Texture &bakeTexture, const ElementVector &sourceTextures, CustomizationData const &variables) const = 0;

	virtual void exportTextureRenderer(Exporter &exporter) const = 0;

};

// ======================================================================

#endif
