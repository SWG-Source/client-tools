// ======================================================================
//
// MayaShaderTemplateBuilder.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include "FirstMayaExporter.h"
#include "MayaShaderTemplateBuilder.h"

#include "MayaShaderTemplateWriter.h"
#include "Messenger.h"
#include "sharedFoundation/PointerDeleter.h"

#include <algorithm>
#include <string>
#include <vector>

// ======================================================================

class MayaShaderTemplateBuilder::WriterInfo
{
public:

	struct DescendingPriorityComparator
	{
		bool operator ()(const WriterInfo *lhs, const WriterInfo *rhs) const;
	};

public:

	WriterInfo(MayaShaderTemplateWriter *writer, int priority);
	~WriterInfo();

	MayaShaderTemplateWriter *getWriter() const;
	int                       getPriority() const;

private:

	// disabled
	WriterInfo();

private:

	MayaShaderTemplateWriter *m_writer;
	int                       m_priority;

};

// ======================================================================

Messenger                                   *MayaShaderTemplateBuilder::messenger;
bool                                         MayaShaderTemplateBuilder::ms_installed;
MayaShaderTemplateBuilder::WriterInfoVector  MayaShaderTemplateBuilder::ms_writerInfoVector;

// ======================================================================
// class MayaShaderTemplateBuilder::WriterInfo
// ======================================================================

MayaShaderTemplateBuilder::WriterInfo::WriterInfo(MayaShaderTemplateWriter *writer, int priority) :
	m_writer(writer),
	m_priority(priority)
{
	NOT_NULL(writer);
}

// ----------------------------------------------------------------------

MayaShaderTemplateBuilder::WriterInfo::~WriterInfo()
{
	// we own the writer
	delete m_writer;
}

// ----------------------------------------------------------------------
//lint -esym(1763, WriterInfo::getWriter) // member marked const indirectly modifies class // that's okay, class really acts as a struct

inline MayaShaderTemplateWriter *MayaShaderTemplateBuilder::WriterInfo::getWriter() const
{
	return m_writer;
}

// ----------------------------------------------------------------------

inline int MayaShaderTemplateBuilder::WriterInfo::getPriority() const
{
	return m_priority;
}

// ======================================================================
// struct MayaShaderTemplateBuilder::WriterInfo::DescendingPriorityComparator
// ======================================================================

inline bool MayaShaderTemplateBuilder::WriterInfo::DescendingPriorityComparator::operator ()(const WriterInfo *lhs, const WriterInfo *rhs) const
{
	NOT_NULL(lhs);
	NOT_NULL(rhs);

	// return true if lhs.priority is greater than rhs.priority
	return lhs->getPriority() > rhs->getPriority();
}

// ======================================================================
// class MayaShaderTemplateBuilder
// ======================================================================
/**
 * Install the module.
 */

void MayaShaderTemplateBuilder::install(Messenger *newMessenger)
{
	DEBUG_FATAL(ms_installed, ("MayaShaderTemplateBuilder already installed"));

	messenger    = newMessenger;
	ms_installed = true;
}

// ----------------------------------------------------------------------
/**
 * Uninstall the module.
 */

void MayaShaderTemplateBuilder::remove()
{
	DEBUG_FATAL(!ms_installed, ("MayaShaderTemplateBuilder not installed"));

	std::for_each(ms_writerInfoVector.begin(), ms_writerInfoVector.end(), PointerDeleter());
	ms_writerInfoVector.clear();

	messenger    = NULL;
	ms_installed = false;
}

// ======================================================================
/**
 * Register a MayaShaderTemplateWriter to handle writing data for a
 * specific ShaderTemplate-derived class.
 *
 * This function takes ownership of the MayaShaderTemplateWriter instance.
 * The caller should not try to delete this instance.
 *
 * The priority value indicates when the registered MayaShaderTemplateWriter
 * instance gets a crack at writing the ShaderTemplate data.  An instance
 * with a larger priority value will get a chance to indicate it can write
 * a ShaderTemplate prior to an instance with a smaller priority value.
 *
 * As currently implemented, this function does not check for registering
 * the same MayaShaderTemplateWriter multiple times.  Don't do it.
 *
 * @param writer    the MayaShaderTemplateWriter instance being registered.
 * @param priority  the priority value for the writer being registered.
 */
void MayaShaderTemplateBuilder::registerShaderTemplateWriter(MayaShaderTemplateWriter *writer, int priority)
{
	DEBUG_FATAL(!ms_installed, ("MayaShaderTemplateBuilder not installed"));
	NOT_NULL(writer);

	//-- add the writer.
	ms_writerInfoVector.push_back(new WriterInfo(writer, priority));

	//-- sort by descending priority.  We start checking for the writers from highest priority to least.
	std::sort(ms_writerInfoVector.begin(), ms_writerInfoVector.end(), WriterInfo::DescendingPriorityComparator());
}

// ----------------------------------------------------------------------
/**
 * Write a ShaderTemplate for the Maya shader data specified in the
 * given ShaderGroup node.
 *
 * This function will use the highest-priority MayaShaderTemplateWriter
 * that indicates it can and should write the given ShaderGroup
 * shader-related data.
 *
 * @todo document the parameters.
 *
 * @return  true if the ShaderTemplate data was successfully written to
 *          disk; false otherwise.
 */

bool MayaShaderTemplateBuilder::buildShaderTemplate(
		const std::string       &shaderTemplateWriteName, 
		const MObject           &shaderGroupObject, 
		bool                     hasVertexAlpha,
		TextureRendererVector   &referencedTextureRenderers, 
		const std::string       &textureReferenceDirectory,
		const std::string       &textureRendererReferenceDirectory,
		const std::string       &effectReferenceDirectory,
		MayaUtility::TextureSet &textureFilenames,
		bool                     hasDot3TextureCoordinate,
		int                      dot3TextureCoordinateIndex)
{
	DEBUG_FATAL(!ms_installed, ("MayaShaderTemplateBuilder not installed"));

	//-- find the first MayaShaderTemplateWriter that thinks it should write this shader.
	MayaShaderTemplateWriter *writer = 0;

	{
		const WriterInfoVector::iterator endIt = ms_writerInfoVector.end();
		for (WriterInfoVector::iterator it = ms_writerInfoVector.begin(); it != endIt; ++it)
		{
			NOT_NULL(*it);

			MayaShaderTemplateWriter *const testWriter = (*it)->getWriter();
			NOT_NULL(testWriter);

			if (testWriter->canWrite(shaderGroupObject))
			{
				//-- use this writer
				writer = testWriter;
				break;
			}
		}
	}

	MESSENGER_REJECT(!writer, ("none of the registered MayaShaderTemplateWriter instances thought they should write shader [%s].\n", shaderTemplateWriteName.c_str()));

	//-- write it
	const bool writeSuccess = writer->write(
		shaderTemplateWriteName, 
		shaderGroupObject, 
		hasVertexAlpha,
		referencedTextureRenderers, 
		textureReferenceDirectory,
		textureRendererReferenceDirectory,
		effectReferenceDirectory,
		textureFilenames,
		hasDot3TextureCoordinate,
		dot3TextureCoordinateIndex);

	MESSENGER_REJECT(!writeSuccess, ("MayaShaderTemplateWriter failed to write data for [%s].\n", shaderTemplateWriteName.c_str()));

	// success
	return true;
}

// ======================================================================
