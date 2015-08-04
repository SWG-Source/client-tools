// ======================================================================
//
// ProxySkeletalAnimationTemplate.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/ProxySkeletalAnimationTemplate.h"

#include "clientSkeletalAnimation/AnimationPriorityMap.h"
#include "clientSkeletalAnimation/MaskedPrioritySkeletalAnimation.h"
#include "clientSkeletalAnimation/SinglePrioritySkeletalAnimation.h"
#include "clientSkeletalAnimation/SkeletalAnimationTemplateList.h"
#include "clientSkeletalAnimation/SkeletalAnimationTemplateUiFactory.h"
#include "clientSkeletalAnimation/TransformMask.h"
#include "clientSkeletalAnimation/TransformMaskList.h"
#include "sharedFile/Iff.h"
#include "sharedFile/IndentedFileWriter.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/MemoryBlockManagerMacros.h"
#include "sharedXml/XmlTreeNode.h"

#include <stdio.h>
#include <string>

// ======================================================================

namespace ProxySkeletalAnimationTemplateNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	Tag const TAG_PPTR = TAG(P,P,T,R);
	Tag const TAG_PUNF = TAG(P,U,N,F);
	Tag const TAG_PXAT = TAG(P,X,A,T);

	char const *const cs_referenceAnimationElementName      = "referenceAnimation";
	char const *const cs_locationAttributeName              = "location";

	char const *const cs_uniformPriorityElementName         = "uniformPriority";
	char const *const cs_transformGroupNameAttributeName    = "transformGroupName";
	char const *const cs_locomotionGroupNameAttributeName   = "locomotionGroupName";

	char const *const cs_maskedPriorityElementName          = "maskedPriority";
	char const *const cs_transformMaskLocationAttributeName = "transformMaskLocation";
	char const *const cs_transformInGroupNameAttributeName  = "transformInGroupName";
	char const *const cs_transformOutGroupNameAttributeName = "transformOutGroupName";

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	std::string  s_temporaryString;
}

using namespace ProxySkeletalAnimationTemplateNamespace;

// ======================================================================

bool                                ProxySkeletalAnimationTemplate::ms_installed;
MemoryBlockManager   *ProxySkeletalAnimationTemplate::ms_memoryBlockManager;
SkeletalAnimationTemplateUiFactory *ProxySkeletalAnimationTemplate::ms_uiFactory;

// ======================================================================
/**
 * Provides a factory method to create a SkeletalAnimation-derived class
 * that layers on handling of transform and locomotion priority.
 */

class ProxySkeletalAnimationTemplate::PriorityAnimationFactory
{
public:

	static PriorityAnimationFactory *fetch_0000(Iff &iff);
	static PriorityAnimationFactory *fetchXml_1(XmlTreeNode const &treeNode);

	// Tool interface.
	virtual void write(Iff &iff) const = 0;
	virtual void writeXml(IndentedFileWriter &writer) const = 0;

public:

	virtual                   ~PriorityAnimationFactory();
	virtual SkeletalAnimation *fetch(const TransformNameMap &transformNameMap, SkeletalAnimation *baseAnimation) const = 0;

protected:

	PriorityAnimationFactory();

};

// ----------------------------------------------------------------------

class ProxySkeletalAnimationTemplate::MaskedPriorityAnimationFactory: public ProxySkeletalAnimationTemplate::PriorityAnimationFactory
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

public:

	MaskedPriorityAnimationFactory();
	virtual ~MaskedPriorityAnimationFactory();

	void load_0000(Iff &iff);
	void loadXml_1(XmlTreeNode const &treeNode);

	virtual SkeletalAnimation *fetch(const TransformNameMap &transformNameMap, SkeletalAnimation *baseAnimation) const;

	// Tool interface.
	MaskedPriorityAnimationFactory(const CrcLowerString &maskName, const CrcLowerString &inGroupTransformPriorityGroup, const CrcLowerString &outGroupTransformPriorityGroup, const CrcLowerString &locomotionPriorityGroup);

	const CrcLowerString &getTransformMaskName() const;
	const CrcLowerString &getInGroupTransformPriorityGroup() const;
	const CrcLowerString &getOutGroupTransformPriorityGroup() const;
	const CrcLowerString &getLocomotionPriorityGroup() const;

	virtual void          write(Iff &iff) const;
	virtual void          writeXml(IndentedFileWriter &writer) const;

private:

	// Disabled.
	MaskedPriorityAnimationFactory(const MaskedPriorityAnimationFactory&);             //lint -esym(754, MaskedPriorityAnimationFactory::MaskedPriorityAnimationFactory) // Defensive hiding.
	MaskedPriorityAnimationFactory &operator =(const MaskedPriorityAnimationFactory&); //lint -esym(754, MaskedPriorityAnimationFactory::operator=)                      // Defensive hiding.

private:

	const TransformMask *m_transformMask;
	int                  m_inGroupTransformPriority;
	int                  m_outGroupTransformPriority;
	int                  m_locomotionPriority;

	// @todo figure out a way to handle writing without needing these in the customer build.
	CrcLowerString      *m_transformMaskName;
	CrcLowerString      *m_inGroupTransformPriorityGroup;
	CrcLowerString      *m_outGroupTransformPriorityGroup;
	CrcLowerString      *m_locomotionPriorityGroup;

};

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(ProxySkeletalAnimationTemplate::MaskedPriorityAnimationFactory, true, 0, 0, 0);

// ----------------------------------------------------------------------

class ProxySkeletalAnimationTemplate::UniformPriorityAnimationFactory: public ProxySkeletalAnimationTemplate::PriorityAnimationFactory
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

public:

	UniformPriorityAnimationFactory();
	virtual ~UniformPriorityAnimationFactory();

	void load_0000(Iff &iff);
	void loadXml_1(XmlTreeNode const &treeNode);

	virtual SkeletalAnimation *fetch(const TransformNameMap &transformNameMap, SkeletalAnimation *baseAnimation) const;

	// Tool interface.
	UniformPriorityAnimationFactory(const CrcLowerString &uniformTransformPriorityGroup, const CrcLowerString &locomotionPriorityGroup);

	const CrcLowerString &getUniformTransformPriorityGroup() const;
	const CrcLowerString &getLocomotionPriorityGroup() const;

	virtual void          write(Iff &iff) const;
	virtual void          writeXml(IndentedFileWriter &writer) const;

private:

	// Disabled.
	UniformPriorityAnimationFactory(const UniformPriorityAnimationFactory&);             //lint -esym(754, UniformPriorityAnimationFactory::UniformPriorityAnimationFactory) // unreferenced // Defensive hiding.
	UniformPriorityAnimationFactory &operator =(const UniformPriorityAnimationFactory&); //lint -esym(754, UniformPriorityAnimationFactory::operator=)                       // unreferenced // Defensive hiding.

private:

	int             m_uniformTransformPriority;
	int             m_locomotionPriority;

	// @todo figure out a way to handle writing without needing these in the customer build.
	CrcLowerString *m_uniformTransformPriorityGroup;
	CrcLowerString *m_locomotionPriorityGroup;

};

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(ProxySkeletalAnimationTemplate::UniformPriorityAnimationFactory, true, 0, 0, 0);

// ======================================================================
// class ProxySkeletalAnimationTemplate::PriorityAnimationFactory
// ======================================================================

ProxySkeletalAnimationTemplate::PriorityAnimationFactory *ProxySkeletalAnimationTemplate::PriorityAnimationFactory::fetch_0000(Iff &iff)
{
	//-- Create factory based on tag type.
	const Tag factoryType = iff.getCurrentName();
	switch (factoryType)
	{
		case TAG_PUNF:
			{
				UniformPriorityAnimationFactory *const factory = new UniformPriorityAnimationFactory();
				factory->load_0000(iff);

				return factory;
			}

		case TAG_PPTR:
			{
				MaskedPriorityAnimationFactory *const factory = new MaskedPriorityAnimationFactory();
				factory->load_0000(iff);

				return factory;
			}

		default:
			{
				char buffer[5];

				ConvertTagToString(factoryType, buffer);
				DEBUG_WARNING(true, ("unsupported PriorityAnimationFactory type [%s].", buffer));

				return 0;
			}
	}
}

// ----------------------------------------------------------------------

ProxySkeletalAnimationTemplate::PriorityAnimationFactory *ProxySkeletalAnimationTemplate::PriorityAnimationFactory::fetchXml_1(XmlTreeNode const &treeNode)
{
	char const *const nodeName = treeNode.getName();
	if (!_stricmp(nodeName, cs_uniformPriorityElementName))
	{
		UniformPriorityAnimationFactory *const factory = new UniformPriorityAnimationFactory();
		factory->loadXml_1(treeNode);
		return factory;
	}
	else if (!_stricmp(nodeName, cs_maskedPriorityElementName))
	{
		MaskedPriorityAnimationFactory *const factory = new MaskedPriorityAnimationFactory();
		factory->loadXml_1(treeNode);
		return factory;
	}
	else
	{
		FATAL(true, ("ProxySkeletalAnimationTemplate: unsupported entity name=[%s] specified where expecting a priority element type.", nodeName));
		return NULL; //lint !e527 // unreachable // needed for VS6.
	}
}

// ======================================================================

ProxySkeletalAnimationTemplate::PriorityAnimationFactory::PriorityAnimationFactory()
{
}

// ----------------------------------------------------------------------

ProxySkeletalAnimationTemplate::PriorityAnimationFactory::~PriorityAnimationFactory()
{
}

// ======================================================================
// class ProxySkeletalAnimationTemplate::MaskedPriorityAnimationFactory
// ======================================================================

ProxySkeletalAnimationTemplate::MaskedPriorityAnimationFactory::MaskedPriorityAnimationFactory() :
	PriorityAnimationFactory(),
	m_transformMask(0),
	m_inGroupTransformPriority(0),
	m_outGroupTransformPriority(0),
	m_locomotionPriority(0),
	m_transformMaskName(0),
	m_inGroupTransformPriorityGroup(0),
	m_outGroupTransformPriorityGroup(0),
	m_locomotionPriorityGroup(0)
{
}

// ----------------------------------------------------------------------

ProxySkeletalAnimationTemplate::MaskedPriorityAnimationFactory::~MaskedPriorityAnimationFactory()
{
	if (m_transformMask)
	{
		m_transformMask->release();
		m_transformMask = 0;
	}

	delete m_locomotionPriorityGroup;
	delete m_outGroupTransformPriorityGroup;
	delete m_inGroupTransformPriorityGroup;
	delete m_transformMaskName;
}

// ----------------------------------------------------------------------

void ProxySkeletalAnimationTemplate::MaskedPriorityAnimationFactory::load_0000(Iff &iff)
{
	//-- Validate preconditions.
	IS_NULL(m_locomotionPriorityGroup);
	IS_NULL(m_transformMaskName);
	IS_NULL(m_inGroupTransformPriorityGroup);
	IS_NULL(m_outGroupTransformPriorityGroup);

	char buffer[512];

	iff.enterChunk(TAG_PPTR);

		//-- Get locomotion priority.
		iff.read_string(buffer, sizeof(buffer) - 1);
		m_locomotionPriorityGroup = new CrcLowerString(buffer);
		m_locomotionPriority      = AnimationPriorityMap::getPriorityValue(*m_locomotionPriorityGroup);

		//-- Get TransformMask instance.
		iff.read_string(buffer, sizeof(buffer) - 1);

		m_transformMaskName = new CrcLowerString(buffer);
		m_transformMask     = TransformMaskList::fetch(*m_transformMaskName);		

		//-- Get mask "in" group transform priority.
		iff.read_string(buffer, sizeof(buffer) - 1);
		m_inGroupTransformPriorityGroup = new CrcLowerString(buffer);
		m_inGroupTransformPriority      = AnimationPriorityMap::getPriorityValue(*m_inGroupTransformPriorityGroup);

		//-- Get mask "out" group transform priority.
		iff.read_string(buffer, sizeof(buffer) - 1);
		m_outGroupTransformPriorityGroup = new CrcLowerString(buffer);
		m_outGroupTransformPriority      = AnimationPriorityMap::getPriorityValue(*m_outGroupTransformPriorityGroup);

	iff.exitChunk(TAG_PPTR);
}

// ----------------------------------------------------------------------

void ProxySkeletalAnimationTemplate::MaskedPriorityAnimationFactory::loadXml_1(XmlTreeNode const &treeNode)
{
	//-- Validate preconditions.
	IS_NULL(m_locomotionPriorityGroup);
	IS_NULL(m_transformMaskName);
	IS_NULL(m_inGroupTransformPriorityGroup);
	IS_NULL(m_outGroupTransformPriorityGroup);
	treeNode.assertIsElement(cs_maskedPriorityElementName);

	//-- Get the transform mask location.
	treeNode.getElementAttributeAsString(cs_transformMaskLocationAttributeName, s_temporaryString);
	m_transformMaskName = new CrcLowerString(s_temporaryString.c_str());
	m_transformMask     = TransformMaskList::fetch(*m_transformMaskName);		

	//-- Get transform mask in-group priority.
	treeNode.getElementAttributeAsString(cs_transformInGroupNameAttributeName, s_temporaryString);
	m_inGroupTransformPriorityGroup = new CrcLowerString(s_temporaryString.c_str());
	m_inGroupTransformPriority      = AnimationPriorityMap::getPriorityValue(*m_inGroupTransformPriorityGroup);
	
	//-- Get transform mask out-group priority.
	treeNode.getElementAttributeAsString(cs_transformOutGroupNameAttributeName, s_temporaryString);
	m_outGroupTransformPriorityGroup = new CrcLowerString(s_temporaryString.c_str());
	m_outGroupTransformPriority      = AnimationPriorityMap::getPriorityValue(*m_inGroupTransformPriorityGroup);

	//-- Get locomotion priority.
	treeNode.getElementAttributeAsString(cs_locomotionGroupNameAttributeName, s_temporaryString);
	m_locomotionPriorityGroup = new CrcLowerString(s_temporaryString.c_str());
	m_locomotionPriority      = AnimationPriorityMap::getPriorityValue(*m_locomotionPriorityGroup);
}

// ----------------------------------------------------------------------

SkeletalAnimation *ProxySkeletalAnimationTemplate::MaskedPriorityAnimationFactory::fetch(const TransformNameMap &transformNameMap, SkeletalAnimation *baseAnimation) const
{
	//-- Ignore null base animations.
	if (!baseAnimation)
	{
		DEBUG_WARNING(true, ("caller passed null baseAnimation."));
		return 0;
	}

	//-- Create the MaskedPrioritySkeletalAnimation.
	NOT_NULL(m_transformMask);

	SkeletalAnimation *const animation = new MaskedPrioritySkeletalAnimation(*baseAnimation, transformNameMap, *m_transformMask, m_inGroupTransformPriority, m_outGroupTransformPriority, m_locomotionPriority);
	animation->fetch();

	return animation;
}

// ----------------------------------------------------------------------

ProxySkeletalAnimationTemplate::MaskedPriorityAnimationFactory::MaskedPriorityAnimationFactory(const CrcLowerString &maskName, const CrcLowerString &inGroupTransformPriorityGroup, const CrcLowerString &outGroupTransformPriorityGroup, const CrcLowerString &locomotionPriorityGroup) :
	PriorityAnimationFactory(),
	m_transformMask(TransformMaskList::fetch(maskName)),
	m_inGroupTransformPriority(AnimationPriorityMap::getPriorityValue(inGroupTransformPriorityGroup)),
	m_outGroupTransformPriority(AnimationPriorityMap::getPriorityValue(outGroupTransformPriorityGroup)),
	m_locomotionPriority(AnimationPriorityMap::getPriorityValue(locomotionPriorityGroup)),
	m_transformMaskName(new CrcLowerString(maskName)),
	m_inGroupTransformPriorityGroup(new CrcLowerString(inGroupTransformPriorityGroup)),
	m_outGroupTransformPriorityGroup(new CrcLowerString(outGroupTransformPriorityGroup)),
	m_locomotionPriorityGroup(new CrcLowerString(locomotionPriorityGroup))
{
}

// ----------------------------------------------------------------------

inline const CrcLowerString &ProxySkeletalAnimationTemplate::MaskedPriorityAnimationFactory::getTransformMaskName() const
{
	NOT_NULL(m_transformMaskName);
	return *m_transformMaskName;
}

// ----------------------------------------------------------------------

inline const CrcLowerString &ProxySkeletalAnimationTemplate::MaskedPriorityAnimationFactory::getInGroupTransformPriorityGroup() const
{
	NOT_NULL(m_inGroupTransformPriorityGroup);
	return *m_inGroupTransformPriorityGroup;
}

// ----------------------------------------------------------------------

inline const CrcLowerString &ProxySkeletalAnimationTemplate::MaskedPriorityAnimationFactory::getOutGroupTransformPriorityGroup() const
{
	NOT_NULL(m_outGroupTransformPriorityGroup);
	return *m_outGroupTransformPriorityGroup;
}

// ----------------------------------------------------------------------

inline const CrcLowerString &ProxySkeletalAnimationTemplate::MaskedPriorityAnimationFactory::getLocomotionPriorityGroup() const
{
	NOT_NULL(m_locomotionPriorityGroup);
	return *m_locomotionPriorityGroup;
}

// ----------------------------------------------------------------------

void ProxySkeletalAnimationTemplate::MaskedPriorityAnimationFactory::write(Iff &iff) const
{
	iff.insertChunk(TAG_PPTR);

		iff.insertChunkString(NON_NULL(m_locomotionPriorityGroup)->getString());
		iff.insertChunkString(NON_NULL(m_transformMaskName)->getString());
		iff.insertChunkString(NON_NULL(m_inGroupTransformPriorityGroup)->getString());
		iff.insertChunkString(NON_NULL(m_outGroupTransformPriorityGroup)->getString());

	iff.exitChunk(TAG_PPTR);
}

// ----------------------------------------------------------------------

void ProxySkeletalAnimationTemplate::MaskedPriorityAnimationFactory::writeXml(IndentedFileWriter &writer) const
{
	NOT_NULL(m_transformMaskName);
	NOT_NULL(m_inGroupTransformPriorityGroup);
	NOT_NULL(m_outGroupTransformPriorityGroup);
	NOT_NULL(m_locomotionPriorityGroup);

	writer.writeLineFormat(
		"<%s %s='%s' %s='%s' %s='%s' %s='%s'/>",
		cs_maskedPriorityElementName,
		cs_transformMaskLocationAttributeName,
		m_transformMaskName->getString(),
		cs_transformInGroupNameAttributeName,
		m_inGroupTransformPriorityGroup->getString(),
		cs_transformOutGroupNameAttributeName,
		m_outGroupTransformPriorityGroup->getString(),
		cs_locomotionGroupNameAttributeName,
		m_locomotionPriorityGroup->getString());
}

// ======================================================================
// class ProxySkeletalAnimationTemplate::UniformPriorityAnimationFactory
// ======================================================================

ProxySkeletalAnimationTemplate::UniformPriorityAnimationFactory::UniformPriorityAnimationFactory() :
	PriorityAnimationFactory(),
	m_uniformTransformPriority(0),
	m_locomotionPriority(0),
	m_uniformTransformPriorityGroup(0),
	m_locomotionPriorityGroup(0)
{
}

// ----------------------------------------------------------------------

ProxySkeletalAnimationTemplate::UniformPriorityAnimationFactory::~UniformPriorityAnimationFactory()
{
	delete m_locomotionPriorityGroup;
	delete m_uniformTransformPriorityGroup;
}

// ----------------------------------------------------------------------

void ProxySkeletalAnimationTemplate::UniformPriorityAnimationFactory::load_0000(Iff &iff)
{
	//-- Verify preconditions.
	IS_NULL(m_locomotionPriorityGroup);
	IS_NULL(m_uniformTransformPriorityGroup);

	char buffer[512];

	iff.enterChunk(TAG_PUNF);

		//-- Get locomotion priority.
		iff.read_string(buffer, sizeof(buffer) - 1);
		m_locomotionPriorityGroup = new CrcLowerString(buffer);
		m_locomotionPriority      = AnimationPriorityMap::getPriorityValue(*m_locomotionPriorityGroup);

		//-- Get uniform transform priority.
		iff.read_string(buffer, sizeof(buffer) - 1);
		m_uniformTransformPriorityGroup = new CrcLowerString(buffer);
		m_uniformTransformPriority      = AnimationPriorityMap::getPriorityValue(*m_uniformTransformPriorityGroup);

	iff.exitChunk(TAG_PUNF);
}

// ----------------------------------------------------------------------

void ProxySkeletalAnimationTemplate::UniformPriorityAnimationFactory::loadXml_1(XmlTreeNode const &treeNode)
{
	//-- Verify preconditions.
	IS_NULL(m_locomotionPriorityGroup);
	IS_NULL(m_uniformTransformPriorityGroup);
	treeNode.assertIsElement(cs_uniformPriorityElementName);

	//-- Get locomotion priority.
	treeNode.getElementAttributeAsString(cs_locomotionGroupNameAttributeName, s_temporaryString);
	m_locomotionPriorityGroup = new CrcLowerString(s_temporaryString.c_str());
	m_locomotionPriority      = AnimationPriorityMap::getPriorityValue(*m_locomotionPriorityGroup);

	//-- Get uniform transform priority.
	treeNode.getElementAttributeAsString(cs_transformGroupNameAttributeName, s_temporaryString);
	m_uniformTransformPriorityGroup = new CrcLowerString(s_temporaryString.c_str());
	m_uniformTransformPriority      = AnimationPriorityMap::getPriorityValue(*m_uniformTransformPriorityGroup);
}

// ----------------------------------------------------------------------

SkeletalAnimation *ProxySkeletalAnimationTemplate::UniformPriorityAnimationFactory::fetch(const TransformNameMap & /*transformNameMap*/, SkeletalAnimation *baseAnimation) const
{
	//-- Ignore null base animations.
	if (!baseAnimation)
	{
		DEBUG_WARNING(true, ("caller passed null baseAnimation."));
		return 0;
	}

	//-- Create the SinglePrioritySkeletalAnimation.
	SkeletalAnimation *const animation = new SinglePrioritySkeletalAnimation(*baseAnimation, m_uniformTransformPriority, m_locomotionPriority);
	animation->fetch();

	return animation;
}

// ----------------------------------------------------------------------

ProxySkeletalAnimationTemplate::UniformPriorityAnimationFactory::UniformPriorityAnimationFactory(const CrcLowerString &uniformTransformPriorityGroup, const CrcLowerString &locomotionPriorityGroup) :
	PriorityAnimationFactory(),
	m_uniformTransformPriority(AnimationPriorityMap::getPriorityValue(uniformTransformPriorityGroup)),
	m_locomotionPriority(AnimationPriorityMap::getPriorityValue(locomotionPriorityGroup)),
	m_uniformTransformPriorityGroup(new CrcLowerString(uniformTransformPriorityGroup)),
	m_locomotionPriorityGroup(new CrcLowerString(locomotionPriorityGroup))
{
}

// ----------------------------------------------------------------------

inline const CrcLowerString &ProxySkeletalAnimationTemplate::UniformPriorityAnimationFactory::getUniformTransformPriorityGroup() const
{
	NOT_NULL(m_uniformTransformPriorityGroup);
	return *m_uniformTransformPriorityGroup;
}

// ----------------------------------------------------------------------

inline const CrcLowerString &ProxySkeletalAnimationTemplate::UniformPriorityAnimationFactory::getLocomotionPriorityGroup() const
{
	NOT_NULL(m_locomotionPriorityGroup);
	return *m_locomotionPriorityGroup;
}

// ----------------------------------------------------------------------

void ProxySkeletalAnimationTemplate::UniformPriorityAnimationFactory::write(Iff &iff) const
{
	iff.insertChunk(TAG_PUNF);

		iff.insertChunkString(NON_NULL(m_locomotionPriorityGroup)->getString());
		iff.insertChunkString(NON_NULL(m_uniformTransformPriorityGroup)->getString());

	iff.exitChunk(TAG_PUNF);
}

// ----------------------------------------------------------------------

void ProxySkeletalAnimationTemplate::UniformPriorityAnimationFactory::writeXml(IndentedFileWriter &writer) const
{
	NOT_NULL(m_uniformTransformPriorityGroup);
	NOT_NULL(m_locomotionPriorityGroup);

	writer.writeLineFormat(
		"<%s %s='%s' %s='%s'/>",
		cs_uniformPriorityElementName,
		cs_transformGroupNameAttributeName,
		m_uniformTransformPriorityGroup->getString(),
		cs_locomotionGroupNameAttributeName,
		m_locomotionPriorityGroup->getString());
}

// ======================================================================
// class ProxySkeletalAnimationTemplate: public static member functions
// ======================================================================

void ProxySkeletalAnimationTemplate::install()
{
	DEBUG_FATAL(ms_installed, ("ProxySkeletalAnimationTemplate already installed"));

	ms_memoryBlockManager = new MemoryBlockManager("ProxySkeletalAnimationTemplate", true, sizeof(ProxySkeletalAnimationTemplate), 0, 0, 0);

	//-- Register IFF reader.
	const bool success = SkeletalAnimationTemplateList::registerCreateFunction(TAG_PXAT, create);
	DEBUG_FATAL(!success, ("failed to register ProxySkeletalAnimationTemplate."));
	UNREF(success);

	//-- Register XML reader.
	SkeletalAnimationTemplateList::registerXmlCreateFunction(cs_referenceAnimationElementName, xmlCreate);

	MaskedPriorityAnimationFactory::install();
	UniformPriorityAnimationFactory::install();

	ms_installed = true;
	ExitChain::add(remove, "ProxySkeletalAnimationTemplate");
}

// ----------------------------------------------------------------------

void *ProxySkeletalAnimationTemplate::operator new(size_t size)
{
	DEBUG_FATAL(!ms_installed, ("ProxySkeletalAnimationTemplate not installed"));
	DEBUG_FATAL(size != sizeof(ProxySkeletalAnimationTemplate), ("trying to use operator new for wrong-sized object."));
	UNREF(size);

	return ms_memoryBlockManager->allocate();
}

// ----------------------------------------------------------------------

void ProxySkeletalAnimationTemplate::operator delete(void *data)
{
	DEBUG_FATAL(!ms_installed, ("ProxySkeletalAnimationTemplate not installed"));

	// Skip case of delete NULL.
	if (!data)
		return;

	// Delete the memory.
	ms_memoryBlockManager->free(data);
}

// ----------------------------------------------------------------------

void ProxySkeletalAnimationTemplate::setUiFactory(SkeletalAnimationTemplateUiFactory *uiFactory)
{
	//-- Check for assignment to self.
	if (ms_uiFactory == uiFactory)
		return;

	//-- Delete old ui factory.
	delete ms_uiFactory;

	//-- Assign new ui factory.
	ms_uiFactory = uiFactory;
}

// ======================================================================
// class ProxySkeletalAnimationTemplate: public member functions
// ======================================================================

SkeletalAnimation *ProxySkeletalAnimationTemplate::fetchSkeletalAnimation(AnimationEnvironment &animationEnvironment, const TransformNameMap &transformNameMap) const
{
	SkeletalAnimation *resultAnimation = 0;

	//-- Forward this call on to the target SkeletalAnimationTemplate.
	// Get the target template.
	const SkeletalAnimationTemplate *const targetTemplate = getTargetSkeletalAnimationTemplate();

	// Forward the call.
	if (targetTemplate)
	{
		//-- Create the base animation from the target SkeletalAnimationTemplate.
		resultAnimation = targetTemplate->fetchSkeletalAnimation(animationEnvironment, transformNameMap);

		//-- Layer on animation transform and locomotion priority if specified.
		if (m_priorityAnimationFactory && resultAnimation)
		{
			// Keep track of old animation.
			SkeletalAnimation *const oldAnimation = resultAnimation;

			// Fetch new animation.
			resultAnimation = m_priorityAnimationFactory->fetch(transformNameMap, resultAnimation);

			// Release reference to old animation.
			oldAnimation->release();
		}
	}

	//-- Return resulting animation to caller.
	return resultAnimation;
}

// ----------------------------------------------------------------------

const SkeletalAnimationTemplateUiFactory *ProxySkeletalAnimationTemplate::getConstUiFactory() const
{
	return ms_uiFactory;
}

// ----------------------------------------------------------------------

ProxySkeletalAnimationTemplate::ProxySkeletalAnimationTemplate() :
	SkeletalAnimationTemplate(CrcLowerString::empty),
	m_targetSkeletalAnimationTemplateName(new CrcLowerString(CrcLowerString::empty)),
	m_targetSkeletalAnimationTemplate(0),
	m_priorityAnimationFactory(0)
{
	DEBUG_FATAL(!ms_installed, ("ProxySkeletalAnimationTemplate not installed"));
}

// ----------------------------------------------------------------------

const CrcLowerString &ProxySkeletalAnimationTemplate::getTargetAnimationTemplateName() const
{
	return *m_targetSkeletalAnimationTemplateName;
}

// ----------------------------------------------------------------------

void ProxySkeletalAnimationTemplate::setTargetAnimationTemplateName(const CrcLowerString &pathName)
{
	*m_targetSkeletalAnimationTemplateName = pathName;
}

// ----------------------------------------------------------------------

void ProxySkeletalAnimationTemplate::setUniformPriority(const CrcLowerString &uniformTransformPriorityGroup, const CrcLowerString &locomotionPriorityGroup)
{
	//-- Clear out existing factory.
	if (m_priorityAnimationFactory)
		delete m_priorityAnimationFactory;

	//-- Create new factory.
	m_priorityAnimationFactory = new UniformPriorityAnimationFactory(uniformTransformPriorityGroup, locomotionPriorityGroup);
}

// ----------------------------------------------------------------------

bool ProxySkeletalAnimationTemplate::hasUniformPriority() const
{
	return (dynamic_cast<const UniformPriorityAnimationFactory*>(m_priorityAnimationFactory) != 0);
}

// ----------------------------------------------------------------------

void ProxySkeletalAnimationTemplate::getUniformPriority(CrcLowerString &uniformTransformPriorityGroup, CrcLowerString &locomotionPriorityGroup) const
{
	DEBUG_FATAL(!hasUniformPriority(), ("proxy template doesn't have uniform transform priority information."));

	//-- Get the factory.
	const UniformPriorityAnimationFactory *const factory = safe_cast<const UniformPriorityAnimationFactory*>(m_priorityAnimationFactory);

	//-- Return the values.
	uniformTransformPriorityGroup = factory->getUniformTransformPriorityGroup();
	locomotionPriorityGroup       = factory->getLocomotionPriorityGroup();
}

// ----------------------------------------------------------------------

void ProxySkeletalAnimationTemplate::setMaskedPriority(const CrcLowerString &maskName, const CrcLowerString &inGroupTransformPriorityGroup, const CrcLowerString &outGroupTransformPriorityGroup, const CrcLowerString &locomotionPriorityGroup)
{
	//-- Clear out existing factory.
	if (m_priorityAnimationFactory)
		delete m_priorityAnimationFactory;

	//-- Create new factory.
	m_priorityAnimationFactory = new MaskedPriorityAnimationFactory(maskName, inGroupTransformPriorityGroup, outGroupTransformPriorityGroup, locomotionPriorityGroup);
}

// ----------------------------------------------------------------------

bool ProxySkeletalAnimationTemplate::hasMaskedPriority() const
{
	return (dynamic_cast<const MaskedPriorityAnimationFactory*>(m_priorityAnimationFactory) != 0);
}

// ----------------------------------------------------------------------

void ProxySkeletalAnimationTemplate::getMaskedPriority(CrcLowerString &maskName, CrcLowerString &inGroupTransformPriorityGroup, CrcLowerString &outGroupTransformPriorityGroup, CrcLowerString &locomotionPriorityGroup)
{
	DEBUG_FATAL(!hasMaskedPriority(), ("proxy template doesn't have masked transform priority information."));

	//-- Get the factory.
	const MaskedPriorityAnimationFactory *const factory = safe_cast<const MaskedPriorityAnimationFactory*>(m_priorityAnimationFactory);

	//-- Return the values.
	maskName                       = factory->getTransformMaskName();
	inGroupTransformPriorityGroup  = factory->getInGroupTransformPriorityGroup();
	outGroupTransformPriorityGroup = factory->getOutGroupTransformPriorityGroup();
	locomotionPriorityGroup        = factory->getLocomotionPriorityGroup();
}

// ----------------------------------------------------------------------

void ProxySkeletalAnimationTemplate::setHasNoAssignedPriority()
{
	//-- Clear out existing factory.
	if (m_priorityAnimationFactory)
	{
		delete m_priorityAnimationFactory;
		m_priorityAnimationFactory = 0;
	}
}

// ----------------------------------------------------------------------

bool ProxySkeletalAnimationTemplate::hasNoAssignedPriority() const
{
	return (m_priorityAnimationFactory == NULL);
}

// ----------------------------------------------------------------------

bool ProxySkeletalAnimationTemplate::supportsWriting() const
{
	return true;
}

// ----------------------------------------------------------------------

void ProxySkeletalAnimationTemplate::write(Iff &iff) const
{
	iff.insertForm(TAG_PXAT);
		iff.insertForm(TAG_0000);

			iff.insertChunk(TAG_INFO);

				iff.insertChunkString(m_targetSkeletalAnimationTemplateName->getString());

			iff.exitChunk(TAG_INFO);

			if (m_priorityAnimationFactory)
				m_priorityAnimationFactory->write(iff);

		iff.exitForm(TAG_0000);
	iff.exitForm(TAG_PXAT);
}

// ----------------------------------------------------------------------

void ProxySkeletalAnimationTemplate::writeXml(IndentedFileWriter &writer) const
{
	//-- Write location of source animation.
	writer.writeLineFormat("<%s %s='%s'>", cs_referenceAnimationElementName, cs_locationAttributeName, m_targetSkeletalAnimationTemplateName->getString());

	//-- Write out optional priority information.
	if (m_priorityAnimationFactory)
	{
		writer.indent();
		{
			m_priorityAnimationFactory->writeXml(writer);
		}
		writer.unindent();
	}

	//-- Close up the reference animation.
	writer.writeLineFormat("</%s>", cs_referenceAnimationElementName);
}

// ----------------------------------------------------------------------

void ProxySkeletalAnimationTemplate::garbageCollect() const
{
	//-- Call parent class.
	SkeletalAnimationTemplate::garbageCollect();

	//-- Throw out our reference to the real template
	if (m_targetSkeletalAnimationTemplate)
	{
		m_targetSkeletalAnimationTemplate->release();
		m_targetSkeletalAnimationTemplate = NULL;
	}
}

// ======================================================================
// class ProxySkeletalAnimationTemplate: private static member functions
// ======================================================================

void ProxySkeletalAnimationTemplate::remove()
{
	DEBUG_FATAL(!ms_installed, ("ProxySkeletalAnimationTemplate not installed"));
	ms_installed = false;

	delete ms_memoryBlockManager;
	ms_memoryBlockManager = 0;

	delete ms_uiFactory;
	ms_uiFactory = 0;

	const bool success = SkeletalAnimationTemplateList::deregisterCreateFunction(TAG_PXAT);
	DEBUG_WARNING(!success, ("failed to deregister ProxySkeletalAnimationTemplate."));
	UNREF(success);

	SkeletalAnimationTemplateList::deregisterXmlCreateFunction(cs_referenceAnimationElementName);
}

// ----------------------------------------------------------------------

SkeletalAnimationTemplate *ProxySkeletalAnimationTemplate::create(const CrcLowerString &name, Iff &iff)
{
	DEBUG_FATAL(!ms_installed, ("ProxySkeletalAnimationTemplate not installed"));

	return new ProxySkeletalAnimationTemplate(name, iff);
}

// ----------------------------------------------------------------------

SkeletalAnimationTemplate *ProxySkeletalAnimationTemplate::xmlCreate(const CrcString &name, XmlTreeNode const &treeNode, int versionNumber)
{
	DEBUG_FATAL(!ms_installed, ("ProxySkeletalAnimationTemplate not installed"));
	return new ProxySkeletalAnimationTemplate(name, treeNode, versionNumber);
}

// ======================================================================
// class ProxySkeletalAnimationTemplate: private member functions
// ======================================================================

ProxySkeletalAnimationTemplate::ProxySkeletalAnimationTemplate(const CrcLowerString &name, Iff &iff) :
	SkeletalAnimationTemplate(name),
	m_targetSkeletalAnimationTemplateName(0),
	m_targetSkeletalAnimationTemplate(0),
	m_priorityAnimationFactory(0)
{
	iff.enterForm(TAG_PXAT);

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
					DEBUG_FATAL(true, ("unsupported ProxySkeletalAnimationTemplate format [%s].", buffer));
				}
		}

	iff.exitForm(TAG_PXAT);
}

// ----------------------------------------------------------------------

ProxySkeletalAnimationTemplate::ProxySkeletalAnimationTemplate(const CrcString &name, XmlTreeNode const &treeNode, int versionNumber) :
	SkeletalAnimationTemplate(name),
	m_targetSkeletalAnimationTemplateName(0),
	m_targetSkeletalAnimationTemplate(0),
	m_priorityAnimationFactory(0)
{
	switch (versionNumber)
	{
		case 1:
			loadXml_1(treeNode, versionNumber);
			break;

		default:
			FATAL(true, ("ProxySkeletalAnimationTemplate: tried to load unsupported version %d.", versionNumber));
	}
}

// ----------------------------------------------------------------------

ProxySkeletalAnimationTemplate::~ProxySkeletalAnimationTemplate()
{
	DEBUG_FATAL(!ms_installed, ("ProxySkeletalAnimationTemplate not installed"));

	if (m_targetSkeletalAnimationTemplate)
	{
		m_targetSkeletalAnimationTemplate->release();
		m_targetSkeletalAnimationTemplate = 0;
	}

	delete m_priorityAnimationFactory;
	delete m_targetSkeletalAnimationTemplateName;
}

// ----------------------------------------------------------------------

const SkeletalAnimationTemplate *ProxySkeletalAnimationTemplate::getTargetSkeletalAnimationTemplate() const
{
	NOT_NULL(m_targetSkeletalAnimationTemplateName);

	if (!m_targetSkeletalAnimationTemplate)
	{
		if (*m_targetSkeletalAnimationTemplateName->getString() && TreeFile::exists(m_targetSkeletalAnimationTemplateName->getString()))
			m_targetSkeletalAnimationTemplate = SkeletalAnimationTemplateList::fetch(*m_targetSkeletalAnimationTemplateName);
	}

	return m_targetSkeletalAnimationTemplate;
}

// ----------------------------------------------------------------------

void ProxySkeletalAnimationTemplate::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);

		//-- Load name of SkeletalAnimationTemplate for which this is a proxy.
		iff.enterChunk(TAG_INFO);
			
			char buffer[2048];
			iff.read_string(buffer, sizeof(buffer) - 1);

			delete m_targetSkeletalAnimationTemplateName;
			m_targetSkeletalAnimationTemplateName = new CrcLowerString(buffer);
#if 0
			DEBUG_REPORT_LOG(true, ("PROXY: mem [0x%08x] for anim [%s].\n", m_targetSkeletalAnimationTemplateName, buffer));
#endif

		iff.exitChunk(TAG_INFO);

		//-- Create factory for layered-on priority animation.
		if (!iff.atEndOfForm())
		{
			if (m_priorityAnimationFactory)
				delete m_priorityAnimationFactory;

			m_priorityAnimationFactory = PriorityAnimationFactory::fetch_0000(iff);
		}

	iff.exitForm(TAG_0000);
}

// ----------------------------------------------------------------------

void ProxySkeletalAnimationTemplate::loadXml_1(XmlTreeNode const &treeNode, int versionNumber)
{
	UNREF(versionNumber);

	IS_NULL(m_targetSkeletalAnimationTemplateName);
	IS_NULL(m_priorityAnimationFactory);
	treeNode.assertIsElement(cs_referenceAnimationElementName);

	//-- Get the location of the IFF.
	treeNode.getElementAttributeAsString(cs_locationAttributeName, s_temporaryString);
	m_targetSkeletalAnimationTemplateName = new CrcLowerString(s_temporaryString.c_str());

	//-- Handle animation priority.
	XmlTreeNode const priorityNode = treeNode.getFirstChildElementNode();
	if (!priorityNode.isNull())
		m_priorityAnimationFactory = PriorityAnimationFactory::fetchXml_1(priorityNode);
}

// ======================================================================
