// ======================================================================
//
// LogicalAnimationTableTemplate.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/LogicalAnimationTableTemplate.h"

#include "clientSkeletalAnimation/AnimationStateHierarchyTemplate.h"
#include "clientSkeletalAnimation/AnimationStateHierarchyTemplateList.h"
#include "clientSkeletalAnimation/ConfigClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/LogicalAnimationTableTemplateList.h"
#include "clientSkeletalAnimation/SkeletalAnimationTemplate.h"
#include "clientSkeletalAnimation/SkeletalAnimationTemplateList.h"
#include "clientSkeletalAnimation/StateHierarchyAnimationController.h"
#include "sharedFile/Iff.h"
#include "sharedFile/IndentedFileWriter.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/MemoryBlockManagerMacros.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedXml/XmlTreeDocument.h"
#include "sharedXml/XmlTreeNode.h"

#include <algorithm>
#include <set>
#include <string>
#include <vector>

// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITHOUT_INSTALL(LogicalAnimationTableTemplate, true, 0, 0, 0);

// ======================================================================

const ConstCharCrcLowerString LogicalAnimationTableTemplate::cms_defaultEntryName("default");

// ======================================================================

namespace LogicalAnimationTableTemplateNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	Tag const TAG_ANIM = TAG(A,N,I,M);
	Tag const TAG_LATT = TAG(L,A,T,T);

	char const *const cs_logicalAnimationTableStartElementFormat = 
		"<logicalAnimationTable "
		"xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance' "
		"xsi:schemaLocation='http://namespaces.station.sony.com/animation/logical-animation-table-template/%d %slogical-animation-table-template-%d.xsd' "
		"xmlns='http://namespaces.station.sony.com/animation/logical-animation-table-template/%d' "
		"version='%d'>";

	/// The .LAT format version number that this code writes.
	int const cs_writeFormatVersion = 1;

	/// The directory where the .lat schema can be found, must include a trailing forward slash if non-empty.
	char const *const cs_localSchemaDirectoryPrefix = "";

	// Element and attribute constants.
	char const *const cs_logicalAnimationTableElementName = "logicalAnimationTable";
	char const *const cs_versionAttributeName             = "version";

	char const *const cs_ashFileReferenceElementName      = "ashFileReference";
	char const *const cs_locationAttributeName            = "location";

	char const *const cs_animationEntryElementName        = "animationEntry";
	char const *const cs_logicalNameAttributeName         = "logicalName";

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool         s_installed;
	std::string  s_animationName;
}

using namespace LogicalAnimationTableTemplateNamespace;

// ======================================================================
// class LogicalAnimationTableTemplate::AnimationEntry
// ======================================================================

class LogicalAnimationTableTemplate::AnimationEntry
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

public:

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class NameEqualPredicate
	{
	public:

		explicit NameEqualPredicate(const CrcLowerString *targetName);
		bool operator ()(const AnimationEntry *entry) const;

	private:

		// disabled
		NameEqualPredicate();

	private:

		const CrcLowerString *m_targetName;

	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class NameLessPredicate
	{
	public:

		bool operator ()(AnimationEntry const *lhs, AnimationEntry const *rhs) const;
		bool operator ()(AnimationEntry const *lhs, CrcString const &rhs) const;
		bool operator ()(CrcString const &lhs, AnimationEntry const *rhs) const;
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class AlphabeticalLessPredicate
	{
	public:

		bool operator ()(AnimationEntry const *lhs, AnimationEntry const *rhs) const;
		bool operator ()(AnimationEntry const *lhs, CrcString const &rhs) const;
		bool operator ()(CrcString const &lhs, AnimationEntry const *rhs) const;
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

public:

	AnimationEntry(const char *name, const SkeletalAnimationTemplate *animationTemplate);
	~AnimationEntry();

	const CrcLowerString            &getName() const;

	const SkeletalAnimationTemplate *getConstSkeletalAnimationTemplate() const;
	void                             setSkeletalAnimationTemplate(const SkeletalAnimationTemplate *animationTemplate);

	bool                             isWritable() const;
	void                             write(Iff &iff) const;
	void                             writeXml(IndentedFileWriter &writer) const;

	void                             garbageCollect();

private:

	// disabled
	AnimationEntry();
	AnimationEntry(const AnimationEntry &rhs);         //lint -esym(754, AnimationEntry::AnimationEntry) // not referenced // defensive hiding.
	AnimationEntry &operator =(const AnimationEntry&); //lint -esym(754, AnimationEntry::operator=) // not referenced // not implementable.

private:

	const CrcLowerString             m_name;             // wouldn't need to keep this in engine class if I didn't try to edit with this class.
	const SkeletalAnimationTemplate *m_animationTemplate;

};

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(LogicalAnimationTableTemplate::AnimationEntry, true, 0, 0, 0);

// ======================================================================

LogicalAnimationTableTemplate::AnimationEntry::AnimationEntry(const char *name, const SkeletalAnimationTemplate *animationTemplate) :
	m_name(name),
	m_animationTemplate(animationTemplate)
{
	//-- Grab local reference.
	if (m_animationTemplate)
		m_animationTemplate->fetch();
}

// ----------------------------------------------------------------------

LogicalAnimationTableTemplate::AnimationEntry::~AnimationEntry()
{
	//-- Release local reference.
	//lint -esym(1540, AnimationEntry::m_animationTemplate) // not freed or zero'ed // ok, ref counted & released.
	if (m_animationTemplate)
		m_animationTemplate->release();
}

// ----------------------------------------------------------------------

inline const CrcLowerString &LogicalAnimationTableTemplate::AnimationEntry::getName() const
{
	return m_name;
}

// ----------------------------------------------------------------------

const SkeletalAnimationTemplate *LogicalAnimationTableTemplate::AnimationEntry::getConstSkeletalAnimationTemplate() const
{
	return m_animationTemplate;
}

// ----------------------------------------------------------------------

void LogicalAnimationTableTemplate::AnimationEntry::setSkeletalAnimationTemplate(const SkeletalAnimationTemplate *animationTemplate)
{
	//-- Check for assignment of same instance.
	if (animationTemplate == m_animationTemplate)
		return;

	//-- Release existing animaiton template.
	if (m_animationTemplate)
		m_animationTemplate->release();

	//-- Fetch local reference to new animation template.
	if (animationTemplate)
		animationTemplate->fetch();

	//-- Make assignment.
	m_animationTemplate = animationTemplate;
}

// ----------------------------------------------------------------------

bool LogicalAnimationTableTemplate::AnimationEntry::isWritable() const
{
	return (m_animationTemplate && m_animationTemplate->supportsWriting());
}

// ----------------------------------------------------------------------

void LogicalAnimationTableTemplate::AnimationEntry::write(Iff &iff) const
{
	//-- Skip non-writable entries.
	if (!isWritable())
		return;

	//-- Write the entry.
	iff.insertForm(TAG_ANIM);

		iff.insertChunk(TAG_INFO);
			iff.insertChunkString(m_name.getString());
		iff.exitChunk(TAG_INFO);

		NOT_NULL(m_animationTemplate);
		m_animationTemplate->write(iff);

	iff.exitForm(TAG_ANIM);
}

// ----------------------------------------------------------------------

void LogicalAnimationTableTemplate::AnimationEntry::writeXml(IndentedFileWriter &writer) const
{
	//-- Skip non-writable entries.
	if (!m_animationTemplate || !isWritable())
		return;

	//-- Write the entry.
	writer.writeLineFormat("<%s %s='%s'>", cs_animationEntryElementName, cs_logicalNameAttributeName, m_name.getString());
	writer.indent();
	{
		m_animationTemplate->writeXml(writer);
	}
	writer.unindent();
	writer.writeLineFormat("</%s>", cs_animationEntryElementName);
}

// ----------------------------------------------------------------------

void LogicalAnimationTableTemplate::AnimationEntry::garbageCollect()
{
	if (m_animationTemplate)
		m_animationTemplate->garbageCollect();
} //lint !e1762 // function could be made const // I think of this as logically non-const.

// ======================================================================
// struct LogicalAnimationTableTemplate::AnimationEntry::NameEqualPredicate
// ======================================================================

inline LogicalAnimationTableTemplate::AnimationEntry::NameEqualPredicate::NameEqualPredicate(const CrcLowerString *targetName) :
	m_targetName(targetName)
{
	NOT_NULL(m_targetName);
}

// ----------------------------------------------------------------------

inline bool LogicalAnimationTableTemplate::AnimationEntry::NameEqualPredicate::operator ()(const AnimationEntry *entry) const
{
	NOT_NULL(entry);
	return entry->getName() == *m_targetName;
}

// ======================================================================
// struct LogicalAnimationTableTemplate::AnimationEntry::NameLessPredicate
// ======================================================================

inline bool LogicalAnimationTableTemplate::AnimationEntry::NameLessPredicate::operator ()(AnimationEntry const *lhs, AnimationEntry const *rhs) const
{
	NOT_NULL(lhs);
	NOT_NULL(rhs);

	return lhs->getName() < rhs->getName();
}

// ----------------------------------------------------------------------

inline bool LogicalAnimationTableTemplate::AnimationEntry::NameLessPredicate::operator ()(AnimationEntry const *lhs, CrcString const &rhs) const
{
	NOT_NULL(lhs);

	return lhs->getName() < rhs;
}

// ----------------------------------------------------------------------

inline bool LogicalAnimationTableTemplate::AnimationEntry::NameLessPredicate::operator ()(CrcString const &lhs, AnimationEntry const *rhs) const
{
	NOT_NULL(rhs);

	return lhs < rhs->getName();
}

// ----------------------------------------------------------------------

// ======================================================================
// struct LogicalAnimationTableTemplate::AnimationEntry::AlphabeticalLessPredicate
// ======================================================================

inline bool LogicalAnimationTableTemplate::AnimationEntry::AlphabeticalLessPredicate::operator ()(AnimationEntry const *lhs, AnimationEntry const *rhs) const
{
	NOT_NULL(lhs);
	NOT_NULL(rhs);

	return strcmp(lhs->getName().getString(), rhs->getName().getString()) < 0;
}

// ----------------------------------------------------------------------

inline bool LogicalAnimationTableTemplate::AnimationEntry::AlphabeticalLessPredicate::operator ()(AnimationEntry const *lhs, CrcString const &rhs) const
{
	NOT_NULL(lhs);

	return strcmp(lhs->getName().getString(), rhs.getString()) < 0;
}

// ----------------------------------------------------------------------

inline bool LogicalAnimationTableTemplate::AnimationEntry::AlphabeticalLessPredicate::operator ()(CrcString const &lhs, AnimationEntry const *rhs) const
{
	NOT_NULL(rhs);

	return strcmp(lhs.getString(), rhs->getName().getString()) < 0;
}

// ======================================================================
// class LogicalAnimationTableTemplate: public static member functions
// ======================================================================

void LogicalAnimationTableTemplate::install()
{
	DEBUG_FATAL(s_installed, ("LogicalAnimationTableTemplate already installed."));

	installMemoryBlockManager();
	LogicalAnimationTableTemplate::AnimationEntry::install();

	s_installed = true;
	ExitChain::add(remove, "LogicalAnimationTableTemplate");
}

// ======================================================================
// class LogicalAnimationTableTemplate: public member functions
// ======================================================================

void LogicalAnimationTableTemplate::release() const
{
	--m_referenceCount;
	if (m_referenceCount < 1)
	{
		DEBUG_WARNING(m_referenceCount < 0, ("bad reference counting logic [%d].", m_referenceCount));

		LogicalAnimationTableTemplateList::stopTracking(*this);
		delete const_cast<LogicalAnimationTableTemplate*>(this);
	}
}

// ----------------------------------------------------------------------

SkeletalAnimationTemplate const *LogicalAnimationTableTemplate::fetchConstAnimationTemplate(CrcString const &logicalAnimationName) const
{
	typedef std::pair<AnimationEntryVector::const_iterator, AnimationEntryVector::const_iterator> FindResult;
	
	const FindResult result = std::equal_range(m_animationEntries.begin(), m_animationEntries.end(), logicalAnimationName, AnimationEntry::NameLessPredicate()); //lint !e64 // type mismatch // I think lint is confused.
	if (result.first == result.second)
	{
		// No animation template mapping exists for the specified logical animation name.
		DEBUG_WARNING(ConfigClientSkeletalAnimation::getLogAnimationLookup(), ("LAT (%s): no animation for [%s], returning NULL animation template.", getName().getString(), logicalAnimationName.getString()));
		return 0;
	}
	else
	{
		// Retrieve the template from the mapping.
		SkeletalAnimationTemplate const * const saTemplate = (*result.first)->getConstSkeletalAnimationTemplate();
		if (saTemplate)
		{
			// Fetch reference for the caller.
			saTemplate->fetch();
		}

		return saTemplate;
	}
}

// ----------------------------------------------------------------------

const AnimationStateHierarchyTemplate *LogicalAnimationTableTemplate::fetchConstHierarchyTemplate() const
{
	if (m_hierarchyTemplate)
	{
		m_hierarchyTemplate->fetch();
		return m_hierarchyTemplate;
	}
	else
		return 0;
}

// ----------------------------------------------------------------------
/**
 * Create an animation controller suitable for the logical animation table.
 *
 * It is absolutely essential that the lifetime of the animationEnvironment
 * parameter be at least as long as the animation controller.  In practice
 * this is the case because the lifetime of the animationEnvironment is
 * the lifetime of the Appearance, which is also the lifetime of the
 * TransformAnimationResolver which contains the animation controller.
 *
 * @param animationEnvironment  the animation environment for the controller.
 */

TransformAnimationController *LogicalAnimationTableTemplate::createAnimationController(AnimationEnvironment &animationEnvironment, const TransformNameMap &transformNameMap, Appearance * /* ownerAppearance */, int channel, AnimationStatePath const &initialPath) const
{
	return new StateHierarchyAnimationController(animationEnvironment, transformNameMap, m_hierarchyTemplate, this, channel, initialPath);
}

// ----------------------------------------------------------------------

LogicalAnimationTableTemplate::LogicalAnimationTableTemplate(const AnimationStateHierarchyTemplate &hierarchyTemplate) :
	m_hierarchyTemplate(&hierarchyTemplate),
	m_animationEntries(),
	m_defaultAnimationEntryIndex(-1),
	m_referenceCount(0),
	m_name(CrcLowerString::empty)
{
	//-- Grab local reference.
	m_hierarchyTemplate->fetch();

	//-- Add default entry.
	m_defaultAnimationEntryIndex = static_cast<int>(m_animationEntries.size());
	m_animationEntries.push_back(new AnimationEntry(cms_defaultEntryName.getString(), 0));
}

// ----------------------------------------------------------------------

void LogicalAnimationTableTemplate::createUnspecifiedEntries()
{
	if (!m_hierarchyTemplate)
		return;

	AnimationEntryVector  newEntryVector;

	const int ashLogicalAnimationCount = m_hierarchyTemplate->getLogicalAnimationCount();
	for (int i = 0; i < ashLogicalAnimationCount; ++i)
	{
		//-- Get the Logical Animation Name from the ash.
		const CrcLowerString &logicalAnimationName = m_hierarchyTemplate->getLogicalAnimationName(i);

		//-- Check if this Lat has an entry for the name.
		const bool hasAnimationEntry = hasLogicalAnimationName(logicalAnimationName);
		if (!hasAnimationEntry)
		{
			// Lat doesn't have an entry, add one.
			newEntryVector.push_back(new AnimationEntry(logicalAnimationName.getString(), 0));
		}
	}

	//-- Add all new entries to the animation entry vector, then sort.
	m_animationEntries.insert(m_animationEntries.end(), newEntryVector.begin(), newEntryVector.end());
	std::sort(m_animationEntries.begin(), m_animationEntries.end(), AnimationEntry::NameLessPredicate());
}

// ----------------------------------------------------------------------
/**
 * Delete all lat table entries for logical animation names that are not
 * present in the associated ash file.
 *
 * A lat table can have entries that were defined for an earlier set of ash
 * logical animation names that no longer exist.  When this happens, the
 * lat is left with an orphan entry that will never be used by a lat controller.
 * This function removes all such orphan entries.  If the file is saved
 * after removing them, the orphan entries will not be saved.
 */

void LogicalAnimationTableTemplate::removeEntriesNotSpecifiedInAsh()
{
	if (!m_hierarchyTemplate)
		return;

	//-- Retrieve the set of referenced logical animation names from the ASH.
	AnimationStateHierarchyTemplate::CrcLowerStringSet ashLogicalAnimationNames;
	m_hierarchyTemplate->addReferencedLogicalAnimationNames(ashLogicalAnimationNames);

	//-- Check each LAT entry against the list of referenced logical animation entries.
	int deletedEntryCount = 0;

	for (AnimationEntryVector::iterator it = m_animationEntries.begin(); it != m_animationEntries.end(); )
	{
		CrcLowerString const &latLogicalAnimationName = (*it)->getName();
		const bool latEntryInAsh = (ashLogicalAnimationNames.find(latLogicalAnimationName) != ashLogicalAnimationNames.end());
		if (!latEntryInAsh && (latLogicalAnimationName != cms_defaultEntryName))
		{
			REPORT_LOG(true, ("removing stale entry from lat: logical animation name = [%s].\n", latLogicalAnimationName.getString()));

			// Delete the entry, get next iterator.
			delete *it;
			it = m_animationEntries.erase(it);
			++deletedEntryCount;
		}
		else
		{
			// Get next iterator.
			++it;
		}
	}

	REPORT_LOG(true, ("removed [%d] stale logical animation entries from lat [%s].\n", deletedEntryCount, getName().getString()));
}

// ----------------------------------------------------------------------

int LogicalAnimationTableTemplate::getAnimationCount() const
{
	return static_cast<int>(m_animationEntries.size());
}

// ----------------------------------------------------------------------

const CrcLowerString &LogicalAnimationTableTemplate::getLogicalAnimationName(int index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getAnimationCount());
	return m_animationEntries[static_cast<AnimationEntryVector::size_type>(index)]->getName();
}

// ----------------------------------------------------------------------

SkeletalAnimationTemplate *LogicalAnimationTableTemplate::fetchAnimationTemplateByIndex(int index)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getAnimationCount());
	const SkeletalAnimationTemplate *animationTemplate = m_animationEntries[static_cast<AnimationEntryVector::size_type>(index)]->getConstSkeletalAnimationTemplate();

	// @todo eliminate the need for the const cast.
	return const_cast<SkeletalAnimationTemplate*>(animationTemplate);
}

// ----------------------------------------------------------------------

bool LogicalAnimationTableTemplate::hasLogicalAnimationName(const CrcLowerString &logicalAnimationName) const
{
	return std::binary_search(m_animationEntries.begin(), m_animationEntries.end(), logicalAnimationName, AnimationEntry::NameLessPredicate());
}

// ----------------------------------------------------------------------

void LogicalAnimationTableTemplate::setAnimationTemplate(const CrcLowerString &logicalAnimationName, const SkeletalAnimationTemplate *animationTemplate)
{
	//-- Check if item already exists in the list.
	typedef std::pair<AnimationEntryVector::iterator, AnimationEntryVector::iterator> FindResult;
	
	const FindResult result = std::equal_range(m_animationEntries.begin(), m_animationEntries.end(), logicalAnimationName, AnimationEntry::NameLessPredicate()); //lint !e64 // type mismatch // I think lint is confused.
	if (result.first == result.second)
	{
		// Entry doesn't exist for the specified name.
		if (animationTemplate != NULL)
		{
			// @todo convert this into a log n op instead of an n log n op.

			// Entry doesn't exist, add to end.
			m_animationEntries.push_back(new AnimationEntry(logicalAnimationName.getString(), animationTemplate));

			// Maintain sort order.
			std::sort(m_animationEntries.begin(), m_animationEntries.end(), AnimationEntry::NameLessPredicate());
		}
	}
	else
	{
		// Entry exists for the specified name.

		if (animationTemplate == NULL)
		{
			// Delete the existing entry.
			delete *result.first;
			IGNORE_RETURN(m_animationEntries.erase(result.first));
		}
		else
		{
			// Set the new animation template for the logical animation name.
			AnimationEntry *const animationEntry = *result.first;
			NOT_NULL(animationEntry);

			animationEntry->setSkeletalAnimationTemplate(animationTemplate);
		}
	}
}

// ----------------------------------------------------------------------

void LogicalAnimationTableTemplate::write(Iff &iff) const
{
	if (!m_hierarchyTemplate)
		return;

	iff.insertForm(TAG_LATT);
		iff.insertForm(TAG_0000);

			iff.insertChunk(TAG_INFO);

				//-- Ensure ash reference name is a TreeFile-relative path.
				std::string const ashReferenceName = getProperAshReferenceName();

				// Write ash file name.
				iff.insertChunkString(ashReferenceName.c_str());

				// Write animation entry count.
				const int writableAnimationEntryCount = countWritableAnimationEntries();
				iff.insertChunkData(static_cast<int16>(writableAnimationEntryCount));

			iff.exitChunk(TAG_INFO);

			//-- Write each logical animation name and entry
			const AnimationEntryVector::const_iterator endIt = m_animationEntries.end();
			for (AnimationEntryVector::const_iterator it = m_animationEntries.begin(); it != endIt; ++it)
			{
				const AnimationEntry *const entry = *it;
				NOT_NULL(entry);

				if (entry->isWritable())
				{
					entry->write(iff);
				}
			}

		iff.exitForm(TAG_0000);
	iff.exitForm(TAG_LATT);
}

// ----------------------------------------------------------------------

bool LogicalAnimationTableTemplate::writeXml(char const *filename) const
{
	//-- Ensure we have an ASH.
	if (!m_hierarchyTemplate)
	{
		DEBUG_WARNING(true, ("Failed to write LAT as XML: this .LAT data does not specify an associated .ASH file."));
		return false;
	}

	//-- Create the file writing object.
	IndentedFileWriter *const writer = IndentedFileWriter::createWriter(filename);
	if (!writer)
	{
		WARNING(true, ("writeXml(): failed to create writer for file [%s], aborting LAT write.", filename));
		return false;
	}

	//-- Write XML header.
	writer->writeLine("<?xml version='1.0'?>");

	//-- Write hierarchy element.
	writer->writeLineFormat(
		cs_logicalAnimationTableStartElementFormat,
		cs_writeFormatVersion,
		cs_localSchemaDirectoryPrefix,
		cs_writeFormatVersion,
		cs_writeFormatVersion,
		cs_writeFormatVersion);
	
	//-- Write LAT contents.
	writer->indent();
	{
		// Write ash reference name.
		std::string const ashReferenceName = getProperAshReferenceName();
		writer->writeLineFormat("<%s location='%s'/>", cs_ashFileReferenceElementName, ashReferenceName.c_str());

		// Write animations.
		int const writableAnimationCount = countWritableAnimationEntries();
		if (writableAnimationCount > 0)
		{
			writer->writeLine("<animations>");
			writer->indent();
			{
				//-- Copy and sort animation entries by ABC order (not CRC order) to make it easier to work with text XML version.
				AnimationEntryVector  sortedEntryVector(m_animationEntries);
				std::sort(sortedEntryVector.begin(), sortedEntryVector.end(), AnimationEntry::AlphabeticalLessPredicate());

				//-- Write each logical animation name and entry.
				const AnimationEntryVector::const_iterator endIt = sortedEntryVector.end();
				for (AnimationEntryVector::const_iterator it = sortedEntryVector.begin(); it != endIt; ++it)
				{
					const AnimationEntry *const entry = *it;
					DEBUG_WARNING(!entry, ("Null animation entry in LAT [%s].", getName().getString()));

					if (entry && entry->isWritable())
						entry->writeXml(*writer);
				}
			}
			writer->unindent();
			writer->writeLine("</animations>");
		}
	}
	writer->unindent();

	//-- Close table.
	writer->writeLineFormat("</%s>", cs_logicalAnimationTableElementName);
	delete writer;

	// Success.
	return true;
}

// ----------------------------------------------------------------------

void LogicalAnimationTableTemplate::garbageCollect()
{
	AnimationEntryVector::iterator const endIt = m_animationEntries.end();
	for (AnimationEntryVector::iterator it = m_animationEntries.begin(); it != endIt; ++it)
	{
		if (*it)
			(*it)->garbageCollect();
	}
}

// ======================================================================
// class LogicalAnimationTableTemplate: PRIVATE STATIC
// ======================================================================

void LogicalAnimationTableTemplate::remove()
{
	DEBUG_FATAL(!s_installed, ("LogicalAnimationTableTemplate not installed."));
	s_installed = false;

	removeMemoryBlockManager();
}

// ======================================================================
// class LogicalAnimationTableTemplate: private member functions
// ======================================================================

LogicalAnimationTableTemplate::LogicalAnimationTableTemplate(const CrcString &name, Iff &iff) :
	m_hierarchyTemplate(0),
	m_animationEntries(),
	m_defaultAnimationEntryIndex(-1),
	m_referenceCount(0),
	m_name(name)
{
	//-- Load from Iff.
	iff.enterForm(TAG_LATT);
		
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
					DEBUG_FATAL(true, ("unsupported LogicalAnimationTableTemplate version [%s].", buffer));
				}
		}

	iff.exitForm(TAG_LATT);
}

// ----------------------------------------------------------------------

LogicalAnimationTableTemplate::LogicalAnimationTableTemplate(const CrcString &name, XmlTreeDocument const &xmlTreeDocument) :
	m_hierarchyTemplate(0),
	m_animationEntries(),
	m_defaultAnimationEntryIndex(-1),
	m_referenceCount(0),
	m_name(name)
{
	//-- Load from Xml.

	// Ensure the root is a hierarchy element.
	XmlTreeNode  rootTreeNode = xmlTreeDocument.getRootTreeNode();

	FATAL(!rootTreeNode.isElement(), ("LogicalAnimationTableTemplate(): document root should be an element."));
	FATAL(_stricmp(cs_logicalAnimationTableElementName, rootTreeNode.getName()), ("LogicalAnimationTableTemplate(): expecting root node to be named [%s] but found [%s].", cs_logicalAnimationTableElementName, rootTreeNode.getName()));

	int versionNumber = -1;
	rootTreeNode.getElementAttributeAsInt(cs_versionAttributeName, versionNumber);

	switch (versionNumber)
	{
		case 1:
			loadXml_1(rootTreeNode.getFirstChildElementNode());
			break;

		default:
			FATAL(true, ("Unsupported LogicalAnimationTableTemplate version [%d] from file [%s].", versionNumber, name.getString()));
	}
}

// ----------------------------------------------------------------------

LogicalAnimationTableTemplate::~LogicalAnimationTableTemplate()
{
	//-- Release local referenced.
	if (m_hierarchyTemplate)
	{
		m_hierarchyTemplate->release();
		m_hierarchyTemplate = 0;
	}

	std::for_each(m_animationEntries.begin(), m_animationEntries.end(), PointerDeleter());
}

// ----------------------------------------------------------------------

int LogicalAnimationTableTemplate::countWritableAnimationEntries() const
{
	int count = 0;

	const AnimationEntryVector::const_iterator endIt = m_animationEntries.end();
	for (AnimationEntryVector::const_iterator it = m_animationEntries.begin(); it != endIt; ++it)
	{
		NOT_NULL(*it);
		if ((*it)->isWritable())
			++count;
	}

	return count;
}

// ----------------------------------------------------------------------

void LogicalAnimationTableTemplate::load_0000(Iff &iff)
{
	char buffer[2048];

	iff.enterForm(TAG_0000);

		iff.enterChunk(TAG_INFO);
			
			//-- Load the animation state hierarchy template.
			// Get animation state hierarchy template name.
			iff.read_string(buffer, sizeof(buffer) - 1);

			// Load the hierarchy template.
			m_hierarchyTemplate = AnimationStateHierarchyTemplateList::fetch(TemporaryCrcString(buffer, true));
			WARNING(!m_hierarchyTemplate, ("Lat file (%s): associated hierarchy file [%s] failed to load.", getName().getString(), buffer));

			//-- Get # animation entries.
			const int animationEntryCount = static_cast<int>(iff.read_int16());
			WARNING_STRICT_FATAL(animationEntryCount < 0, ("bad animation entry count [%d].", animationEntryCount));

		iff.exitChunk(TAG_INFO);

		m_animationEntries.reserve(static_cast<AnimationEntryVector::size_type>(animationEntryCount));
		for (int i = 0; i < animationEntryCount; ++i)
		{
			iff.enterForm(TAG_ANIM);
			
				iff.enterChunk(TAG_INFO);
					// Get logical animation name.
					iff.read_string(buffer, sizeof(buffer) - 1);
				iff.exitChunk(TAG_INFO);

				//-- Strip off trailing spaces.
				int scanChar = static_cast<int>(strlen(buffer)) - 1;
				while ((scanChar >= 0) && (buffer[scanChar] == ' '))
					--scanChar;
				buffer[scanChar + 1] = 0;

				// @todo incorporate this into the file format so we don't need to scan for it.
				if (strcmp(buffer, cms_defaultEntryName.getString()) == 0)
					m_defaultAnimationEntryIndex = i;

				// Get SkeletalAnimationTemplate.
				const SkeletalAnimationTemplate *const animationTemplate = SkeletalAnimationTemplateList::fetch(iff);
				if (!animationTemplate)
				{
					WARNING_STRICT_FATAL(true, ("failed to load SkeletalAnimationTemplate straight from iff."));
				}
				else
				{
					// Add the animation entry.
					m_animationEntries.push_back(new AnimationEntry(buffer, animationTemplate));

					// Release local reference.
					animationTemplate->release();
				}

			iff.exitForm(TAG_ANIM);

		}

	iff.exitForm(TAG_0000);

	//-- Create default animation entry if it doesn't exist.
	if (m_defaultAnimationEntryIndex < 0)
	{
		m_defaultAnimationEntryIndex = static_cast<int>(m_animationEntries.size());
		m_animationEntries.push_back(new AnimationEntry(cms_defaultEntryName.getString(), 0));
	}

	//-- Version 0000 does not guarantee that AnimationEntry instances are sorted by CRC value on disk.
	//   Sort these now so we can assume m_animationEntries is sorted by CRC on the logical animation name.
	std::sort(m_animationEntries.begin(), m_animationEntries.end(), AnimationEntry::NameLessPredicate());

#if 0
	debugDump();
#endif
}

// ----------------------------------------------------------------------

void LogicalAnimationTableTemplate::loadXml_1(XmlTreeNode const &treeNode)
{
	//-- Bail out if there is no root node.
	if (treeNode.isNull())
		return;

	//-- Load the associated ASH
	{
		char const *const nodeName = treeNode.getName();
		FATAL(!nodeName || _stricmp(nodeName, cs_ashFileReferenceElementName), ("loadXml_1(): expecting element [%s], found [%s].\n", cs_ashFileReferenceElementName, nodeName));

		std::string  ashPathName;
		treeNode.getElementAttributeAsString(cs_locationAttributeName, ashPathName);

		m_hierarchyTemplate = AnimationStateHierarchyTemplateList::fetch(TemporaryCrcString(ashPathName.c_str(), true));
		WARNING(!m_hierarchyTemplate, ("Lat file (%s): associated hierarchy file [%s] failed to load.", getName().getString(), ashPathName.c_str()));
	}

	//-- Read animation entries.
	XmlTreeNode animationsNode = treeNode.getNextSiblingElementNode();
	if (animationsNode.isNull())
		return;

	int const versionNumber = 1;

	int entryIndex = 0;
	for (XmlTreeNode animationEntryNode = animationsNode.getFirstChildElementNode(); !animationEntryNode.isNull(); animationEntryNode = animationEntryNode.getNextSiblingElementNode(), ++entryIndex)
	{
		//-- Validate entity name/type.
		char const *const nodeName = animationEntryNode.getName();
		FATAL(_stricmp(nodeName, cs_animationEntryElementName), ("LAT XML file expecting element [%s], found entity named [%s] instead.", cs_animationEntryElementName, nodeName));
		FATAL(!animationEntryNode.isElement(), ("The animation entry must be an element."));

		//-- Get logical animation name.
		animationEntryNode.getElementAttributeAsString(cs_logicalNameAttributeName, s_animationName);

		// Strip off trailing spaces in animation name.  This fixed a bug where the editor allowed the user to
		// enter names ending in spaces, which now appear erroneously in the data.
		while (!s_animationName.empty() && (s_animationName[s_animationName.length() - 1] == ' '))
			IGNORE_RETURN(s_animationName.erase(s_animationName.length() - 1));

		FATAL(s_animationName.empty(), ("LAT [%s] had an empty animation entry name.", getName().getString()));

		//-- Keep track of the default animation entry.
		if (!_stricmp(s_animationName.c_str(), cms_defaultEntryName.getString()))
			m_defaultAnimationEntryIndex = entryIndex;

		//-- Load the template from the XML data.
		const SkeletalAnimationTemplate *const animationTemplate = SkeletalAnimationTemplateList::fetch(animationEntryNode.getFirstChildElementNode(), versionNumber);
		if (!animationTemplate)
			WARNING(true, ("failed to load SkeletalAnimationTemplate from XML data."));
		else
		{
			// Add the animation entry.
			m_animationEntries.push_back(new AnimationEntry(s_animationName.c_str(), animationTemplate));

			// Release local reference.
			animationTemplate->release();
		}
	}

	//-- We write the XML versions out in ABC order but internally we need CRC order.  Fix this up now.
	std::sort(m_animationEntries.begin(), m_animationEntries.end(), AnimationEntry::NameLessPredicate());
}

// ----------------------------------------------------------------------

void LogicalAnimationTableTemplate::debugDump() const
{
	DEBUG_REPORT_LOG(true, ("LAT [%s]: dumping [%d] entry CRC values, should be in increasing order.\n", getName().getString(), static_cast<int>(m_animationEntries.size())));

	uint32 lastCrc = 0;
	const AnimationEntryVector::const_iterator endIt = m_animationEntries.end();
	for (AnimationEntryVector::const_iterator it = m_animationEntries.begin(); it != endIt; ++it)
	{
		NOT_NULL(*it);

		const uint32 crc = (*it)->getName().getCrc();
		DEBUG_REPORT_LOG(true, ("  [%10u], [%s].\n", crc, (*it)->getName().getString()));

		DEBUG_REPORT_LOG(crc <= lastCrc, ("duplicate or decreasing crc values!\n"));
		lastCrc = crc;
	}
}

// ----------------------------------------------------------------------

std::string LogicalAnimationTableTemplate::getProperAshReferenceName() const
{
	if (!m_hierarchyTemplate)
		return "<NULL hierarchy template>";

	//-- Try to convert from an absolute filename to a TreeFile-relative filename.
	std::string ashName = TreeFile::getShortestExistingPath(m_hierarchyTemplate->getName().getString());
	if (ashName.empty())
		ashName = m_hierarchyTemplate->getName().getString();

	//-- If the filename contains .xml, remove the .xml portion.  This will force
	//   ash references to always be IFF-relative.  The load code always looks for
	//   .XML files first and falls back to IFF if the original filename was IFF-based
	//   (i.e. .ash but not .ash.xml).
	std::string::size_type const dotXmlStartPos = ashName.find(".xml");
	if (static_cast<int>(dotXmlStartPos) != static_cast<int>(std::string::npos))
		IGNORE_RETURN(ashName.erase(dotXmlStartPos, 4));

	return ashName;
}

// ======================================================================
