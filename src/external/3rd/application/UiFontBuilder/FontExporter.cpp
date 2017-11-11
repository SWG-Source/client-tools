// ======================================================================
//
// FontExporter.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "StdAfx.h"
#include "FontExporter.h"

// ======================================================================
FontExporter::FontExporter (const FontImageExporter & imexp, const ExporterInfo & info) :
m_imageExporter (imexp),
m_info (info)
{

}

//-----------------------------------------------------------------

FontExporter::~FontExporter ()
{
}

// ======================================================================
