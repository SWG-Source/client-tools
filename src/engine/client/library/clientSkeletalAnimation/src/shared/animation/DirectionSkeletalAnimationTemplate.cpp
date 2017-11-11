// ======================================================================
//
// DirectionSkeletalAnimationTemplate.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/DirectionSkeletalAnimationTemplate.h"

#include "clientSkeletalAnimation/AnimationEnvironment.h"
#include "clientSkeletalAnimation/DirectionSkeletalAnimation.h"
#include "clientSkeletalAnimation/SkeletalAnimation.h"
#include "clientSkeletalAnimation/SkeletalAnimationTemplateList.h"
#include "clientSkeletalAnimation/SkeletalAnimationTemplateUiFactory.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h" // @todo remove this, here solely for the debug flag.  figure out better flag strategy.
#include "clientSkeletalAnimation/TransformNameMap.h"
#include "sharedFile/Iff.h"
#include "sharedFile/IndentedFileWriter.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedMath/Vector.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedObject/Object.h"
#include "sharedXml/XmlTreeNode.h"

#include <algorithm>
#include <limits>
#include <string>
#include <vector>

// ======================================================================

namespace DirectionSkeletalAnimationTemplateNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct ThetaMagnitudeData
	{
		float                                          m_yaw;
		DirectionSkeletalAnimationTemplate::Magnitude  m_xMagnitude; //lint -esym(754, ThetaMagnitudeData::m_xMagnitude) // @todo delete these if proven to be useless.
		DirectionSkeletalAnimationTemplate::Magnitude  m_zMagnitude; //lint -esym(754, ThetaMagnitudeData::m_zMagnitude) // @todo delete these if proven to be useless.
		int                                            m_encodedDirection;
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	const float  PI_UNDER_180 = 180.0f / PI;

#define SUPPORT_FULL_CIRCLE 1
#if SUPPORT_FULL_CIRCLE
	const float  cs_minYaw    = -180.0f * PI_OVER_180;
	const float  cs_maxYaw    =  180.0f * PI_OVER_180;
#else
	const float  cs_minYaw    = -165.0f * PI_OVER_180;
	const float  cs_maxYaw    =  165.0f * PI_OVER_180;
#endif

	const ThetaMagnitudeData  cs_thetaMagnitudeMap[] =
		{
			{ cs_minYaw           , DirectionSkeletalAnimationTemplate::M_negative, DirectionSkeletalAnimationTemplate::M_negative, 10 },
			{ -90.0f * PI_OVER_180, DirectionSkeletalAnimationTemplate::M_negative, DirectionSkeletalAnimationTemplate::M_zero,      2 },
			{ -45.0f * PI_OVER_180, DirectionSkeletalAnimationTemplate::M_negative, DirectionSkeletalAnimationTemplate::M_positive,  6 },
			{   0.0f * PI_OVER_180, DirectionSkeletalAnimationTemplate::M_zero,     DirectionSkeletalAnimationTemplate::M_positive,  4 },
			{  45.0f * PI_OVER_180, DirectionSkeletalAnimationTemplate::M_positive, DirectionSkeletalAnimationTemplate::M_positive,  5 },
			{  90.0f * PI_OVER_180, DirectionSkeletalAnimationTemplate::M_positive, DirectionSkeletalAnimationTemplate::M_zero,      1 },
			{ cs_maxYaw           , DirectionSkeletalAnimationTemplate::M_positive, DirectionSkeletalAnimationTemplate::M_negative,  9 }
		};

	const int cs_thetaMagnitudeEntryCount = static_cast<int>(sizeof(cs_thetaMagnitudeMap) / sizeof(cs_thetaMagnitudeMap[0]));

	enum SupportedDirection
	{
		SD_forwardLeft   = BINARY2(0000, 0001),
		SD_forward       = BINARY2(0000, 0010),
		SD_forwardRight  = BINARY2(0000, 0100),
		SD_right         = BINARY2(0000, 1000),
		SD_backwardRight = BINARY2(0001, 0000),
		SD_backward      = BINARY2(0010, 0000),
		SD_backwardLeft  = BINARY2(0100, 0000),
		SD_left          = BINARY2(1000, 0000)
	};

	// NOTE: this array must match one-for-one with the cms_supportedLocomotionDirectionMap
	char const *const  cs_directionNameArray[] =
		{
			NULL,
			"right",
			"left",
			NULL,
			"forward",
			"forwardRight",
			"forwardLeft",
			NULL,
			"backward",
			"backwardRight",
			"backwardLeft",
			NULL,
			NULL,
			NULL,
			NULL,
			NULL
		};
	int const cs_directionNameArrayLength = isizeof(cs_directionNameArray) / isizeof(cs_directionNameArray[0]);

	char const *const cs_directionAnimationElementName = "directionAnimation";
	char const *const cs_choiceElementName             = "choice";
	char const *const cs_directionNameAttributeName    = "directionName";

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	int findEncodedDirectionByName(char const *directionName);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	std::string  s_directionName;
}

using namespace DirectionSkeletalAnimationTemplateNamespace;

// ======================================================================

const uint32 DirectionSkeletalAnimationTemplate::cms_supportedLocomotionDirectionMap[16] =
	{
		                                   // value  Z MAG,      X MAG
		0,                                 // 0      M_zero,     M_zero
		static_cast<int>(SD_right),        // 1      M_zero,     M_positive
		static_cast<int>(SD_left),         // 2      M_zero,     M_negative
		0,                                 // 3      M_zero,     <invalid>
		static_cast<int>(SD_forward),      // 4      M_positive, M_zero
		static_cast<int>(SD_forwardRight), // 5      M_positive, M_positive
		static_cast<int>(SD_forwardLeft),  // 6      M_positive, M_negative
		0,                                 // 7      M_positive, <invalid>
		static_cast<int>(SD_backward),     // 8      M_negative, M_zero
		static_cast<int>(SD_backwardRight),// 9      M_negative, M_positive
		static_cast<int>(SD_backwardLeft), // 10     M_negative, M_negative
		0,                                 // 11     <invalid>,  <invalid>
		0,                                 // 12     <invalid>,  M_zero
		0,                                 // 13     <invalid>,  M_positive
		0,                                 // 14     <invalid>,  M_negative
		0,                                 // 15     <invalid>,  <invalid>
	};

bool                                DirectionSkeletalAnimationTemplate::ms_installed;
MemoryBlockManager   *DirectionSkeletalAnimationTemplate::ms_memoryBlockManager;
SkeletalAnimationTemplateUiFactory *DirectionSkeletalAnimationTemplate::ms_uiFactory;

// ======================================================================

const Tag TAG_DIR  = TAG3(D,I,R);
const Tag TAG_DRAT = TAG(D,R,A,T);

// ======================================================================
// namespace DirectionSkeletalAnimationTemplateNamespace
// ======================================================================

int DirectionSkeletalAnimationTemplateNamespace::findEncodedDirectionByName(char const *directionName)
{
	for (int i = 0; i < cs_directionNameArrayLength; ++i)
	{
		if (cs_directionNameArray[i] && !_stricmp(directionName, cs_directionNameArray[i]))
		{
			// found a match.
			return i;
		}
	}

	FATAL(true, ("failed to find an encoded direction match for directionName [%s].", directionName));
	return -1; //lint !e527 // unreachable // needed for VS.
}

// ======================================================================
// class DirectionSkeletalAnimationTemplate: public static member functions
// ======================================================================

void DirectionSkeletalAnimationTemplate::install()
{
	DEBUG_FATAL(ms_installed, ("DirectionSkeletalAnimationTemplate already installed."));

	ms_memoryBlockManager = new MemoryBlockManager("DirectionSkeletalAnimationTemplate", true, sizeof(DirectionSkeletalAnimationTemplate), 0, 0, 0);

	//-- Register IFF reader.
	const bool success = SkeletalAnimationTemplateList::registerCreateFunction(TAG_DRAT, create);
	DEBUG_FATAL(!success, ("DirectionSkeletalAnimationTemplate failed to register create function."));
	UNREF(success);

	//-- Register XML reader.
	SkeletalAnimationTemplateList::registerXmlCreateFunction(cs_directionAnimationElementName, xmlCreate);

	ms_installed = true;
	ExitChain::add(remove, "DirectionSkeletalAnimationTemplate");
}

// ----------------------------------------------------------------------

void *DirectionSkeletalAnimationTemplate::operator new(size_t size)
{
	DEBUG_FATAL(!ms_installed, ("DirectionSkeletalAnimationTemplate not installed."));
	DEBUG_FATAL(size != sizeof(DirectionSkeletalAnimationTemplate), ("tried to allocate object of wrong size."));
	UNREF(size);

	return ms_memoryBlockManager->allocate();
}

// ----------------------------------------------------------------------

void DirectionSkeletalAnimationTemplate::operator delete(void *data)
{
	DEBUG_FATAL(!ms_installed, ("DirectionSkeletalAnimationTemplate not installed."));

	if (data)
		ms_memoryBlockManager->free(data);
}

// ======================================================================

void DirectionSkeletalAnimationTemplate::setUiFactory(SkeletalAnimationTemplateUiFactory *uiFactory)
{
	DEBUG_FATAL(!ms_installed, ("DirectionSkeletalAnimationTemplate not installed."));

	//-- Check for assignment to self.
	if (ms_uiFactory == uiFactory)
		return;

	//-- Delete existing uiFactory.
	delete ms_uiFactory;

	//-- Assign new uiFactory.
	ms_uiFactory = uiFactory;
}

// ----------------------------------------------------------------------

template <typename T>
inline T AbsoluteValue(T value)
{
	if (value < static_cast<T>(0))
		return -value;
	else
		return value;
}

// ======================================================================
// class DirectionSkeletalAnimationTemplate: public member functions
// ======================================================================

SkeletalAnimation *DirectionSkeletalAnimationTemplate::fetchSkeletalAnimation(AnimationEnvironment &animationEnvironment, const TransformNameMap &transformNameMap) const
{
	SkeletalAnimation *const newAnimation = new DirectionSkeletalAnimation(*this, animationEnvironment, transformNameMap);
	newAnimation->fetch();

	return newAnimation;
}

// ----------------------------------------------------------------------

void DirectionSkeletalAnimationTemplate::getClosestAvailableEncodedDirections(float targetYaw, int &index1, int &index2, float &blendFactor) const
{
	// @todo I don't handle "not-pointing" directions with this through this function as currently stands.
	//       Currently this isn't an issue because directions are now "pointing direction", which is always set to
	//       targetYaw = 0 when no target is selected.

	//-- Clamp yaw to valid range.
	targetYaw = clamp(cs_minYaw, targetYaw, cs_maxYaw);

	//-- Lookup the closest yaw for which we have an animation.
	float closestDeltaYaw         = std::numeric_limits<float>::max();
	int   closestEncodedDirection = 0;
	int   closestEntryIndex       = 0;

	{
		for (int i = 0; i < cs_thetaMagnitudeEntryCount; ++i)
		{
			const ThetaMagnitudeData &data = cs_thetaMagnitudeMap[i];

			const float deltaYaw = AbsoluteValue(targetYaw - data.m_yaw);
			if ((deltaYaw < closestDeltaYaw) && ((*m_directionTemplates)[static_cast<TemplateVector::size_type>(data.m_encodedDirection)] != 0))
			{
				closestDeltaYaw         = deltaYaw;
				closestEncodedDirection = data.m_encodedDirection;
				closestEntryIndex       = i;
			}
		}
	}

	const float closestYaw                    = cs_thetaMagnitudeMap[closestEntryIndex].m_yaw;
	int         secondClosestEntryIndex       = -1;
	int         secondClosestEncodedDirection = closestEncodedDirection;

	if (closestYaw > targetYaw)
	{
		// Second yaw blend should straddle on lower side.

		//-- Find the first available direction on the more-negative-angle side.
		for (int i = closestEntryIndex - 1; i >= 0; --i)
		{
			const ThetaMagnitudeData &data = cs_thetaMagnitudeMap[i];
			if ((*m_directionTemplates)[static_cast<TemplateVector::size_type>(data.m_encodedDirection)] != 0)
			{
				secondClosestEntryIndex = i;
				break;
			}
		}
	}
	else
	{
		// Second yaw blend should straddle on upper side.
		for (int i = closestEntryIndex + 1; i < cs_thetaMagnitudeEntryCount; ++i)
		{
			const ThetaMagnitudeData &data = cs_thetaMagnitudeMap[i];
			if ((*m_directionTemplates)[static_cast<TemplateVector::size_type>(data.m_encodedDirection)] != 0)
			{
				secondClosestEntryIndex = i;
				break;
			}
		}
	}
	
	if (secondClosestEntryIndex < 0)
	{
		// Only one blend.
		blendFactor = 0.0f;
		secondClosestEncodedDirection = closestEncodedDirection;

		DEBUG_REPORT_PRINT(SkeletalAppearance2::getRenderTargetDirection(), ("dirsel:target(%.1f),use1(%.1f),use2(XX),blend(0.0f)\n", targetYaw * PI_UNDER_180, closestYaw * PI_UNDER_180));
	}
	else
	{
		// Collect info on second yaw target.
		const float secondYaw             = cs_thetaMagnitudeMap[secondClosestEntryIndex].m_yaw;
		secondClosestEncodedDirection     = cs_thetaMagnitudeMap[secondClosestEntryIndex].m_encodedDirection;

		const float secondClosestDeltaYaw = AbsoluteValue(targetYaw - secondYaw);

		// Calculate blend.
		const float denominator           = (closestDeltaYaw + secondClosestDeltaYaw);
		if (denominator <= 0.0f)
			blendFactor = 0.0f;
		else
		{
			blendFactor = closestDeltaYaw / denominator;
		}

#ifdef _DEBUG
		if (SkeletalAppearance2::getRenderTargetDirection())
		{
			FATAL(secondClosestEntryIndex == -1, ("secondClosestEntryIndex == -1"));

			const float result = (1.0f - blendFactor) * closestYaw * PI_UNDER_180 + blendFactor * (cs_thetaMagnitudeMap[secondClosestEntryIndex].m_yaw * PI_UNDER_180);
			DEBUG_REPORT_PRINT(true, ("dirsel:target(%.1f),use1(%.1f),use2(%.1f),blend(%.1f),res(%.1f)\n", targetYaw * PI_UNDER_180, closestYaw * PI_UNDER_180, cs_thetaMagnitudeMap[secondClosestEntryIndex].m_yaw * PI_UNDER_180, blendFactor, result));
		}
#endif
	}

	index1 = closestEncodedDirection;
	index2 = secondClosestEncodedDirection;
}

// ----------------------------------------------------------------------

const SkeletalAnimationTemplate *DirectionSkeletalAnimationTemplate::fetchDirectionalAnimationTemplate(int encodedDirection) const
{
	//-- Find direction.
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, encodedDirection, static_cast<int>(m_directionTemplates->size()));

	const SkeletalAnimationTemplate *const saTemplate = (*m_directionTemplates)[static_cast<TemplateVector::size_type>(encodedDirection)];
	if (saTemplate)
		saTemplate->fetch();

	return saTemplate;
}

// ----------------------------------------------------------------------

void DirectionSkeletalAnimationTemplate::garbageCollect() const
{
	//-- Forward on garbage collect call to all subordinate templates.
	NOT_NULL(m_directionTemplates);

	TemplateVector::iterator const endIt = m_directionTemplates->end();
	for (TemplateVector::iterator it = m_directionTemplates->begin(); it != endIt; ++it)
	{
		if (*it)
			(*it)->garbageCollect();
	}
}

// ----------------------------------------------------------------------

DirectionSkeletalAnimationTemplate::DirectionSkeletalAnimationTemplate() :
	SkeletalAnimationTemplate(CrcLowerString::empty),
	m_directionTemplates(new TemplateVector(16, static_cast<const SkeletalAnimationTemplate*>(0))),
	m_supportedLocomotionDirections(0)
{
}

// ----------------------------------------------------------------------

bool DirectionSkeletalAnimationTemplate::supportsWriting() const
{
	return true;
}

// ----------------------------------------------------------------------

void DirectionSkeletalAnimationTemplate::write(Iff &iff) const
{
	iff.insertForm(TAG_DRAT);
		iff.insertForm(TAG_0000);

			// Write # direction entries.
			iff.insertChunk(TAG_INFO);
				iff.insertChunkData(static_cast<int8>(getWritableDirectionalTemplateCount()));
			iff.exitChunk(TAG_INFO);

			// Write direction entries.
			int directionIndex = 0;
			const TemplateVector::const_iterator endIt = m_directionTemplates->end();
			for (TemplateVector::const_iterator it = m_directionTemplates->begin(); it != endIt; ++it, ++directionIndex)
			{
				const SkeletalAnimationTemplate *const saTemplate = *it;
				if (saTemplate && saTemplate->supportsWriting())
				{
					iff.insertForm(TAG_DIR);
						
						// Insert direction.
						iff.insertChunk(TAG_INFO);
							iff.insertChunkData(static_cast<int8>(directionIndex));
						iff.exitChunk(TAG_INFO);

						// Insert SkeletalAnimationTemplate.
						saTemplate->write(iff);

					iff.exitForm(TAG_DIR);
				}
			}

		iff.exitForm(TAG_0000);
	iff.exitForm(TAG_DRAT);
}

// ----------------------------------------------------------------------

void DirectionSkeletalAnimationTemplate::writeXml(IndentedFileWriter &writer) const
{
	writer.writeLineFormat("<%s>", cs_directionAnimationElementName);
	writer.indent();
	{
		//-- Write direction entries.
		int directionIndex = 0;
		const TemplateVector::const_iterator endIt = m_directionTemplates->end();
		for (TemplateVector::const_iterator it = m_directionTemplates->begin(); it != endIt; ++it, ++directionIndex)
		{
			const SkeletalAnimationTemplate *const saTemplate = *it;
			if (saTemplate && saTemplate->supportsWriting())
			{
				writer.writeLineFormat("<%s %s='%s'>", cs_choiceElementName, cs_directionNameAttributeName, cs_directionNameArray[directionIndex]);
				writer.indent();
				{
					saTemplate->writeXml(writer);
				}
				writer.unindent();
				writer.writeLineFormat("</%s>", cs_choiceElementName);
			}
		}
	}
	writer.unindent();
	writer.writeLineFormat("</%s>", cs_directionAnimationElementName);
}

// ----------------------------------------------------------------------

const SkeletalAnimationTemplateUiFactory *DirectionSkeletalAnimationTemplate::getConstUiFactory() const
{
	return ms_uiFactory;
}

// ----------------------------------------------------------------------

void DirectionSkeletalAnimationTemplate::setDirectionalAnimationTemplate(Magnitude xMagnitude, Magnitude zMagnitude, const SkeletalAnimationTemplate *animationTemplate)
{
	//-- Get encoded direction.
	const int encodedDirection = encodeDirectionInInt(xMagnitude, zMagnitude);

	//-- Get pointer entry.
	const SkeletalAnimationTemplate *&saTemplateEntry = (*m_directionTemplates)[static_cast<TemplateVector::size_type>(encodedDirection)];

	//-- Fetch new animationTemplate.
	if (animationTemplate)
		animationTemplate->fetch();

	//-- Release old.
	if (saTemplateEntry)
		saTemplateEntry->release();

	//-- Assign new.
	saTemplateEntry = animationTemplate;

	//-- Recalculate supported locomotion directions.
	calculateSupportedLocomotionDirections();
}

// ======================================================================
// class DirectionSkeletalAnimationTemplate: private member functions
// ======================================================================

void DirectionSkeletalAnimationTemplate::remove()
{
	DEBUG_FATAL(!ms_installed, ("DirectionSkeletalAnimationTemplate not installed."));

	IGNORE_RETURN(SkeletalAnimationTemplateList::deregisterCreateFunction(TAG_DRAT));
	SkeletalAnimationTemplateList::deregisterXmlCreateFunction(cs_directionAnimationElementName);

	delete ms_uiFactory;
	ms_uiFactory = 0;

	delete ms_memoryBlockManager;
	ms_memoryBlockManager = 0;

	ms_installed = false;
}

// ----------------------------------------------------------------------

SkeletalAnimationTemplate *DirectionSkeletalAnimationTemplate::create(const CrcLowerString &name, Iff &iff)
{
	return new DirectionSkeletalAnimationTemplate(name, iff);
}

// ----------------------------------------------------------------------

SkeletalAnimationTemplate *DirectionSkeletalAnimationTemplate::xmlCreate(const CrcString &name, XmlTreeNode const &treeNode, int versionNumber)
{
	return new DirectionSkeletalAnimationTemplate(name, treeNode, versionNumber);
}

// ======================================================================
// class DirectionSkeletalAnimationTemplate: private member functions
// ======================================================================

DirectionSkeletalAnimationTemplate::DirectionSkeletalAnimationTemplate(const CrcLowerString &name, Iff &iff) :
	SkeletalAnimationTemplate(name),
	m_directionTemplates(new TemplateVector(16, static_cast<const SkeletalAnimationTemplate*>(0))),
	m_supportedLocomotionDirections(0)
{
	iff.enterForm(TAG_DRAT);

		const Tag version = iff.getCurrentName();
		switch (version)
		{
			case TAG_0000:
				load_0000(iff);
				break;

			default:
				{
					char buffer[5];

					ConvertTagToString(version, buffer);
					DEBUG_FATAL(true, ("unsupported DirectionSkeletalAnimationTemplate version [%s].", buffer));
					break;
				}
		}

	iff.exitForm(TAG_DRAT);

	calculateSupportedLocomotionDirections();
}

// ----------------------------------------------------------------------

DirectionSkeletalAnimationTemplate::DirectionSkeletalAnimationTemplate(const CrcString &name, XmlTreeNode const &treeNode, int versionNumber) :
	SkeletalAnimationTemplate(name),
	m_directionTemplates(new TemplateVector(16, static_cast<const SkeletalAnimationTemplate*>(0))),
	m_supportedLocomotionDirections(0)
{
	switch (versionNumber)
	{
		case 1:
			loadXml_1(treeNode, versionNumber);
			break;

		default:
			FATAL(true, ("Attempted to load unsupported DirectionSkeletalAnimationTemplate version number %d", versionNumber));
	}

	calculateSupportedLocomotionDirections();
}

// ----------------------------------------------------------------------

DirectionSkeletalAnimationTemplate::~DirectionSkeletalAnimationTemplate()
{
	const TemplateVector::const_iterator endIt = m_directionTemplates->end();
	for (TemplateVector::const_iterator it = m_directionTemplates->begin(); it != endIt; ++it)
	{
		if (*it)
			(*it)->release();
	}

	delete m_directionTemplates;
}

// ----------------------------------------------------------------------

int DirectionSkeletalAnimationTemplate::getWritableDirectionalTemplateCount() const
{
	int count = 0;

	// Count writable templates.
	const TemplateVector::const_iterator endIt = m_directionTemplates->end();
	for (TemplateVector::const_iterator it = m_directionTemplates->begin(); it != endIt; ++it)
	{
		if (*it && (*it)->supportsWriting())
			++count;
	}

	return count;
}

// ----------------------------------------------------------------------

void DirectionSkeletalAnimationTemplate::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);
		
		// Read general info.
		iff.enterChunk(TAG_INFO);
			const int directionCount = static_cast<int>(iff.read_int8());
			DEBUG_FATAL(directionCount < 0, ("invalid direction count [%d].", directionCount));
		iff.exitChunk(TAG_INFO);

		m_directionTemplates->reserve(static_cast<TemplateVector::size_type>(directionCount));
		
		for (int i = 0; i < directionCount; ++i)
		{
			iff.enterForm(TAG_DIR);

				// Read direction for animation template.
				iff.enterChunk(TAG_INFO);
					const int encodedDirection = static_cast<int>(iff.read_int8());
					VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, encodedDirection, static_cast<int>(m_directionTemplates->size()));
				iff.exitChunk(TAG_INFO);

				const SkeletalAnimationTemplate *animationTemplate = 0;
				if (!iff.atEndOfForm())
				{
					animationTemplate = SkeletalAnimationTemplateList::fetch(iff);
					WARNING_STRICT_FATAL(!animationTemplate, ("failed to load animation template for direction [0x%x].", encodedDirection));

					//-- Add DirectionalTemplate entry.
					(*m_directionTemplates)[static_cast<TemplateVector::size_type>(encodedDirection)] = animationTemplate;
				}

			iff.exitForm(TAG_DIR);
		}

	iff.exitForm(TAG_0000);
}

// ----------------------------------------------------------------------

void DirectionSkeletalAnimationTemplate::loadXml_1(XmlTreeNode const &treeNode, int versionNumber)
{
	//-- Validate animation element name.
	char const *const nodeName = treeNode.getName();
	FATAL(!treeNode.isElement() || _stricmp(nodeName, cs_directionAnimationElementName), ("expecting element [%s], found entity [%s].", cs_directionAnimationElementName, nodeName));

	for (XmlTreeNode choiceNode = treeNode.getFirstChildElementNode(); !choiceNode.isNull(); choiceNode = choiceNode.getNextSiblingElementNode())
	{
		//-- Validate choice element name.
		char const *const choiceNodeName = choiceNode.getName();
		FATAL(!treeNode.isElement() || _stricmp(choiceNodeName, cs_choiceElementName), ("expecting element [%s], found [%s].", cs_choiceElementName, choiceNodeName));

		//-- Get direction index from name.
		choiceNode.getElementAttributeAsString(cs_directionNameAttributeName, s_directionName);
		FATAL(s_directionName.empty(), ("empty [%s] attribute found.", cs_directionNameAttributeName));

		int const encodedDirection = findEncodedDirectionByName(s_directionName.c_str());
		(*m_directionTemplates)[static_cast<TemplateVector::size_type>(encodedDirection)] = SkeletalAnimationTemplateList::fetch(choiceNode.getFirstChildElementNode(), versionNumber);
	}
}

// ----------------------------------------------------------------------

void DirectionSkeletalAnimationTemplate::calculateSupportedLocomotionDirections()
{
	m_supportedLocomotionDirections = 0;

	//-- Loop over each existing template entry and add its direction to the list of supported directions.
	int encodedDirection = 0;
	const TemplateVector::const_iterator endIt = m_directionTemplates->end();
	for (TemplateVector::const_iterator it = m_directionTemplates->begin(); it != endIt; ++it, ++encodedDirection)
	{
		m_supportedLocomotionDirections |= cms_supportedLocomotionDirectionMap[encodedDirection];
	}
}

// ----------------------------------------------------------------------

uint32 DirectionSkeletalAnimationTemplate::getNumDirectionTemplates() const
{
	return m_directionTemplates->size();
}

// ======================================================================
