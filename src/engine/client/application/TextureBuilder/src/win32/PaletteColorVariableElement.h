// ======================================================================
//
// PaletteColorVariableElement.h
// copyright 2003 Sony Online Entertainment, Inc.
// All rights reserved.
// 
// ======================================================================

#ifndef INCLUDED_PaletteColorVariableElement_h
#define INCLUDED_PaletteColorVariableElement_h

// ======================================================================

class PaletteArgb;

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/PersistentCrcString.h"
#include "IntVariableElement.h"

#include <string>

// ======================================================================

class PaletteColorVariableElement: public IntVariableElement
{
public:

	PaletteColorVariableElement(char const *variableName, char const *paletteFileName, int paletteIndex);
	virtual ~PaletteColorVariableElement();

	virtual void          saveToCustomizationData(CustomizationData &customizationData) const;
	virtual void          loadFromCustomizationData(CustomizationData const &customizationData);

	virtual bool          isBounded() const;
	virtual int           getLowerBoundInclusive() const;
	virtual int           getUpperBoundInclusive() const;

	virtual int           getValue() const;
	virtual void          setValue(int value);

private:

	PersistentCrcString  m_paletteFileName;
	PaletteArgb const   *m_palette;
	int                  m_value;

private:
	// disabled
	PaletteColorVariableElement();
	PaletteColorVariableElement(const PaletteColorVariableElement&);
	PaletteColorVariableElement &operator =(const PaletteColorVariableElement&);
};

// ======================================================================

#endif
