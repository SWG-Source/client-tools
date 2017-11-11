// ======================================================================
//
// RegionElement.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include "FirstTextureBuilder.h"
#include "RegionElement.h"

#include "ElementTypeIndex.h"
#include "sharedFile/Iff.h"

#include <string>

// ======================================================================

namespace
{
	const Tag TAG_ERGN = TAG(E,R,G,N);
}

// ======================================================================

bool RegionElement::isPersistedNext(Iff &iff)
{
	return (iff.isCurrentForm() && (iff.getCurrentName() == TAG_ERGN));
}

// ======================================================================

RegionElement::RegionElement(Iff &iff)
:	Element(),
	m_regionName(new std::string()),
	m_x0(0.0),
	m_y0(0.0),
	m_x1(1.0),
	m_y1(1.0)
{
	iff.enterForm(TAG_ERGN);

		switch (iff.getCurrentName())
		{
			case TAG_0000:
				load_0000(iff);
				break;
			default:
				{
					char version[5];
					ConvertTagToString(iff.getCurrentName(), version);
					FATAL(true, ("unsupported version [%s]", version));
				}
		}

	iff.exitForm(TAG_ERGN);
}

// ----------------------------------------------------------------------

RegionElement::RegionElement(const std::string &regionName, float x0, float y0, float x1, float y1)
:	Element(),  //lint !e578 // y0 hides bogus global math.h y0
	m_regionName(new std::string(regionName)),
	m_x0(x0),
	m_y0(y0),
	m_x1(x1),
	m_y1(y1)
{
}

// ----------------------------------------------------------------------

RegionElement::~RegionElement()
{
	delete m_regionName;
}

// ----------------------------------------------------------------------

void RegionElement::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);
		iff.enterChunk(TAG_INFO);
		{
			char buffer[1024];
			iff.read_string(buffer, sizeof(buffer)-1);
			IGNORE_RETURN(m_regionName->assign(buffer));

			m_x0 = iff.read_float();
			m_y0 = iff.read_float();
			m_x1 = iff.read_float();
			m_y1 = iff.read_float();
		}
		iff.exitChunk(TAG_INFO);
	iff.exitForm(TAG_0000);
}

// ----------------------------------------------------------------------

bool RegionElement::writeForWorkspace(Iff &iff) const
{
	iff.insertForm(TAG_ERGN);	
		iff.insertForm(TAG_0000);
			iff.insertChunk(TAG_INFO);
			{
				iff.insertChunkString(m_regionName->c_str());

				iff.insertChunkData(static_cast<float>(m_x0));
				iff.insertChunkData(static_cast<float>(m_y0));
				iff.insertChunkData(static_cast<float>(m_x1));
				iff.insertChunkData(static_cast<float>(m_y1));
			}
			iff.exitChunk(TAG_INFO);
		iff.exitForm(TAG_0000);
	iff.exitForm(TAG_ERGN);	

	return true;
}

// ----------------------------------------------------------------------

std::string RegionElement::getLabel() const
{
	char conversionBuffer[128];
	sprintf(conversionBuffer, ": (%.03f,%.03f) - (%.03f,%.03f)", m_x0, m_y0, m_x1, m_y1);

	std::string result(*m_regionName);
	result += conversionBuffer;

	return result;
}

// ----------------------------------------------------------------------

unsigned int RegionElement::getTypeIndex() const
{
	return ETI_region;
}

// ======================================================================
