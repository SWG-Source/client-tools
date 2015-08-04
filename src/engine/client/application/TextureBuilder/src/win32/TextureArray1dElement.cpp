// ======================================================================
//
// TextureArray1dElement.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include "FirstTextureBuilder.h"
#include "TextureArray1dElement.h"

#include "BlueprintWriter.h"
#include "clientGraphics/Texture.h"
#include "clientGraphics/TextureList.h"
#include "DialogEnterName.h"
#include "ElementTypeIndex.h"
#include "Exporter.h"
#include "sharedFile/Iff.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/PointerDeleter.h"
#include "TextureBuilder.h"

#include <algorithm>
#include <string>
#include <vector>

// ======================================================================

namespace
{
	const char *const ms_textureFileFilter = "DDS Files (*.dds)|*.dds|Targa Files (*.tga)|*.tga|All Files (*.*)|*.*||";

	const Tag         TAG_ETX1             = TAG(E,T,X,1);
}

// ======================================================================

class TextureArray1dElement::TextureInfo
{
public:

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class NameEqualsTarget
	{
	public:

		explicit NameEqualsTarget(const std::string &targetPathname);

		bool operator ()(const TextureInfo *textureInfo) const;

	private:

		const std::string &m_targetPathname;

	private:
		// disabled
		NameEqualsTarget();
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

public:

	TextureInfo();
	explicit TextureInfo(const std::string &projectRelativePathname);
	TextureInfo(const TextureInfo &rhs);
	~TextureInfo();

	const std::string &getPathname() const;
	void               setPathname(const std::string &pathname);

	bool               canGetTexture() const;
	const Texture     &getTexture() const;

private:

	std::string            m_pathname;
	mutable const Texture *m_texture;

};

// ======================================================================
// class TextureArray1dElement::TextureInfo
// ======================================================================


TextureArray1dElement::TextureInfo::TextureInfo()
:	m_pathname(),
	m_texture(0)
{
}

// ----------------------------------------------------------------------

TextureArray1dElement::TextureInfo::TextureInfo(const std::string &projectRelativePathname)
:	m_pathname(projectRelativePathname),
	m_texture(0)
{
}

// ----------------------------------------------------------------------

TextureArray1dElement::TextureInfo::TextureInfo(const TextureInfo &rhs) :
	m_pathname(rhs.m_pathname),
	m_texture(rhs.m_texture) //lint !e1554 // (Warning -- Direct pointer copy of member 'TextureInfo::m_texture' within copy constructor) // Yep, used for container of non-pointer container manipulation.
{
	if (m_texture)
		m_texture->fetch();
}

// ----------------------------------------------------------------------

TextureArray1dElement::TextureInfo::~TextureInfo()
{
	if (m_texture)
	{
		m_texture->release();
		m_texture = 0;
	}
}

// ----------------------------------------------------------------------

inline const std::string &TextureArray1dElement::TextureInfo::getPathname() const
{
	return m_pathname;
}

// ----------------------------------------------------------------------

inline void TextureArray1dElement::TextureInfo::setPathname(const std::string &pathname)
{
	m_pathname = pathname;
}

// ----------------------------------------------------------------------

inline bool TextureArray1dElement::TextureInfo::canGetTexture() const
{
	return (m_pathname.length() != 0);
}

// ----------------------------------------------------------------------

const Texture &TextureArray1dElement::TextureInfo::getTexture() const
{
	if (!m_texture)
	{
		//-- load the texture
		FATAL(!canGetTexture(), ("tried to getTexture() when canGetTexture() failed"));

		const std::string textureReferenceName = TextureElement::getReferencePathname(m_pathname);
		m_texture = TextureList::fetch(textureReferenceName.c_str());
	}

	NOT_NULL(m_texture);
	return *m_texture;
}

// ======================================================================
// class TextureArray1dElement::TextureInfo::NameEqualsTarget
// ======================================================================

TextureArray1dElement::TextureInfo::NameEqualsTarget::NameEqualsTarget(const std::string &targetPathname)
: m_targetPathname(targetPathname)
{
}

// ----------------------------------------------------------------------

inline bool TextureArray1dElement::TextureInfo::NameEqualsTarget::operator ()(const TextureInfo *textureInfo) const
{
	NOT_NULL(textureInfo);
	return textureInfo->getPathname() == m_targetPathname;
}

// ======================================================================
// class TextureArray1dElement::ArrayElement
// ======================================================================

TextureArray1dElement::ArrayElement::ArrayElement(TextureArray1dElement &array, int index)
:	Element(),
	m_array(array),
	m_index(index)
{
}

// ----------------------------------------------------------------------

TextureArray1dElement::ArrayElement::~ArrayElement()
{
}

// ----------------------------------------------------------------------

std::string TextureArray1dElement::ArrayElement::getLabel() const
{
	char conversionBuffer[32];
	IGNORE_RETURN(_itoa(m_index, conversionBuffer, 10));

	std::string  result(m_array.getPathname(m_index));
	result += " [";
	result += conversionBuffer;
	result += "]";

	return result;
}

// ----------------------------------------------------------------------

unsigned int TextureArray1dElement::ArrayElement::getTypeIndex() const
{
	return ETI_textureArray1dElement;
}

// ----------------------------------------------------------------------

bool TextureArray1dElement::ArrayElement::hasLeftDoubleClickAction() const
{
	return true;
}

// ----------------------------------------------------------------------

bool TextureArray1dElement::ArrayElement::doLeftDoubleClickAction(TextureBuilderDoc & /* document */)
{
	//-- pop up a dialog to retrieve texture name
	TextureBuilderApp &app                 = TextureBuilderApp::getApp();
	const std::string &lastTexturePathname = app.getLastTexturePathname();

	bool  done;
	bool  returnValue = true;

	do
	{
		done = false;

		CFileDialog dlg(TRUE, "dds", lastTexturePathname.c_str(), 0, ms_textureFileFilter, NULL);
		if (dlg.DoModal() != IDOK)
		{
			//-- user canceled, not modified
			done        = true;
			returnValue = false;
		}
		else
		{
			// selected a texture filename

			//-- Retrieve the texture path relative to the TreeFile paths.
			const CString fullPathName = dlg.GetPathName();
			std::string   treeFilePathName;

			const bool fsToTreeSuccess = TreeFile::stripTreeFileSearchPathFromFile(std::string(fullPathName), treeFilePathName);
			if (!fsToTreeSuccess)
			{
				//-- Indicate the specified filename was not in the TreeFile path.
				char buffer[1024];

				sprintf(buffer, "The specified file [%s] is not within the TreeFile search path, ignoring.", static_cast<const char*>(fullPathName));
				MessageBox(NULL, buffer, "File Not in TreeFile Path", MB_OK | MB_ICONSTOP);
			}
			else if (treeFilePathName == m_array.getPathname(m_index))
			{
				//-- user selected same pathname, return with no modification
				done        = true;
				returnValue = false;
			}
			else if (m_array.hasPathname(treeFilePathName))
			{
				//-- user already entered this pathname
				std::string  message("Texture ");
				message += treeFilePathName;
				message += "already assigned.";

				IGNORE_RETURN(MessageBox(NULL, message.c_str(), "Duplicate texture entry", MB_ICONSTOP | MB_OK));
			}
			else
			{
				//-- we can change this one
				m_array.setPathname(m_index, treeFilePathName);
				done        = true;
				returnValue = true;

				//-- remember this texture name
				app.setLastTexturePathname(fullPathName);
			}
		}
	} while (!done);

	return returnValue;
}

// ----------------------------------------------------------------------

bool TextureArray1dElement::ArrayElement::userCanDelete() const
{
	return true;
}

// ----------------------------------------------------------------------

void TextureArray1dElement::ArrayElement::doPreDeleteTasks()
{
	DEBUG_REPORT_LOG(true, ("doPreDeleteTasks(): for 1-d texture component [index=%d, label=%s].\n", m_index, getLabel().c_str()));
	m_array.removeTextureByIndex(m_index);
}

// ======================================================================
// class TextureArray1dElement
// ======================================================================

bool TextureArray1dElement::isPersistedNext(const Iff &iff)
{
	return (iff.isCurrentForm() && (iff.getCurrentName() == TAG_ETX1));
}

// ======================================================================

TextureArray1dElement::TextureArray1dElement()
:	TextureElement(),
	m_arrayName(new std::string()),
	m_textureInfoVector(new TextureInfoVector())
{
}

// ----------------------------------------------------------------------

TextureArray1dElement::TextureArray1dElement(Iff &iff)
:	TextureElement(),
	m_arrayName(new std::string()),
	m_textureInfoVector(new TextureInfoVector())
{
	iff.enterForm(TAG_ETX1);
		
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

	iff.exitForm(TAG_ETX1);
}

// ----------------------------------------------------------------------

TextureArray1dElement::TextureArray1dElement(const std::string &arrayName)
:	TextureElement(),
	m_arrayName(new std::string(arrayName)),
	m_textureInfoVector(new TextureInfoVector())
{
}

// ----------------------------------------------------------------------

TextureArray1dElement::~TextureArray1dElement()
{
	std::for_each(m_textureInfoVector->begin(), m_textureInfoVector->end(), PointerDeleter());
	delete m_textureInfoVector;

	delete m_arrayName;
}

// ----------------------------------------------------------------------

bool TextureArray1dElement::writeForWorkspace(Iff &iff) const
{
	iff.insertForm(TAG_ETX1);
		iff.insertForm(TAG_0000);
			iff.insertChunk(TAG_INFO);
			{
				iff.insertChunkString(m_arrayName->c_str());
				iff.insertChunkData(static_cast<int32>(m_textureInfoVector->size()));

				const TextureInfoVector::const_iterator itEnd = m_textureInfoVector->end();
				for (TextureInfoVector::const_iterator it = m_textureInfoVector->begin(); it != itEnd; ++it)
				{
					iff.insertChunkString((*it)->getPathname().c_str());
				}
			}
			iff.exitChunk(TAG_INFO);
		iff.exitForm(TAG_0000);
	iff.exitForm(TAG_ETX1);

	return true;
}

// ----------------------------------------------------------------------

std::string TextureArray1dElement::getLabel() const
{
	if (!m_arrayName->empty())
		return *m_arrayName;
	else
		return "<unnamed>";
}

// ----------------------------------------------------------------------

unsigned int TextureArray1dElement::getTypeIndex() const
{
	return ETI_textureArray1d;
}

// ----------------------------------------------------------------------

std::string TextureArray1dElement::getShortLabel() const
{
	return TextureArray1dElement::getLabel();
}

// ----------------------------------------------------------------------

int TextureArray1dElement::getArraySubscriptCount() const
{
	return 1;
}

// ----------------------------------------------------------------------

int TextureArray1dElement::getArraySubscriptUpperBound(int subscriptIndex) const
{
	FATAL(subscriptIndex != 0, ("bad subscript %d for TextureArray1dElement, must be zero", subscriptIndex));
	return getTextureCount();
}

// ----------------------------------------------------------------------

const int TextureArray1dElement::getTextureCount() const
{
	return static_cast<int>(m_textureInfoVector->size());
}

// ----------------------------------------------------------------------

const std::string &TextureArray1dElement::getPathname(int index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, static_cast<int>(m_textureInfoVector->size()));
	return (*m_textureInfoVector)[static_cast<size_t>(index)]->getPathname();
}

// ----------------------------------------------------------------------

const Texture *TextureArray1dElement::getTexture(int index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, static_cast<int>(m_textureInfoVector->size()));

	const TextureInfo *textureInfo = (*m_textureInfoVector)[static_cast<size_t>(index)];
	NOT_NULL(textureInfo);

	if (textureInfo->canGetTexture())
		return &(textureInfo->getTexture());
	else
		return 0;
}

// ----------------------------------------------------------------------

bool TextureArray1dElement::hasLeftDoubleClickAction() const
{
	return true;
}

// ----------------------------------------------------------------------

bool TextureArray1dElement::doLeftDoubleClickAction(TextureBuilderDoc&)
{
	DialogEnterName dlg(*m_arrayName);

	if (dlg.DoModal() == IDOK)
	{
		//-- name was entered
		dlg.m_name.MakeLower();
		IGNORE_RETURN(m_arrayName->assign(dlg.m_name));

		//-- indicate we're modified
		return true;
	}
	else
	{
		// indicate we did not modify this element
		return false;
	}
}

// ----------------------------------------------------------------------

bool TextureArray1dElement::userCanDelete() const
{
	return true;
}

// ----------------------------------------------------------------------

bool TextureArray1dElement::hasPathname(const std::string &pathname)
{
	TextureInfoVector::iterator it = std::find_if(m_textureInfoVector->begin(), m_textureInfoVector->end(), TextureInfo::NameEqualsTarget(pathname));
	if (it != m_textureInfoVector->end())
		return true;
	else
		return false;
}

// ----------------------------------------------------------------------

void TextureArray1dElement::addTexture(const std::string &projectRelativePathname)
{
	if (hasPathname(projectRelativePathname))
	{
		// texture array already contains this texture
		REPORT_LOG(true, ("skipped adding duplicate texture [%s] to 1d texture array [%s]\n", projectRelativePathname.c_str(), m_arrayName->c_str()));
		return;
	}

	//-- append new texture
	const int newIndex = static_cast<int>(m_textureInfoVector->size());

	m_textureInfoVector->push_back(new TextureInfo(projectRelativePathname));
	getChildren().push_back(new ArrayElement(*this, newIndex));
}

// ----------------------------------------------------------------------

void TextureArray1dElement::setPathname(int index, const std::string &projectRelativePathname)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, static_cast<int>(m_textureInfoVector->size()));
	(*m_textureInfoVector)[static_cast<size_t>(index)]->setPathname(projectRelativePathname);
}

// ----------------------------------------------------------------------
/**
 * Take the texture array element at elementIndex and switch it with the
 * entry that is one closer to the front.
 *
 * This function ignores the request if the specified elementIndex is already
 * at the front of the list.
 *
 * If the caller is maintaining a visual hierarchical tree of data containing
 * this element, all data rooted at this element should be redrawn as the
 * ordering may change.
 *
 * @param elementIndex  0-based index of the texture element to move.
 */

void TextureArray1dElement::moveElementTowardFront(int elementIndex)
{
	//-- Validate index.
	if (elementIndex < 1)
	{
		DEBUG_WARNING(true, ("requested to move texture 1d-array element [index %d] closer to front, cannot do.", elementIndex));
		return;
	}

	if (elementIndex >= static_cast<int>(m_textureInfoVector->size()))
	{
		DEBUG_WARNING(true, ("requested to move non-existent texture 1d-array element [index %d] closer to front, cannot do.", elementIndex));
		return;
	}

	//-- Move the child entry toward front.
	ElementVector &elements = getChildren();

	const ElementVector::size_type  newIndex = static_cast<ElementVector::size_type>(elementIndex - 1);
	const ElementVector::size_type  oldIndex = static_cast<ElementVector::size_type>(elementIndex);

	Element *const tempElement = elements[newIndex];
	elements[newIndex]         = elements[oldIndex];
	elements[oldIndex]         = tempElement;

	NON_NULL(dynamic_cast<ArrayElement*>(elements[oldIndex]))->setIndex(static_cast<int>(oldIndex));
	NON_NULL(dynamic_cast<ArrayElement*>(elements[newIndex]))->setIndex(static_cast<int>(newIndex));

	//-- Move TextureInfo entry toward front.
	TextureInfo *const tempInfo      = (*m_textureInfoVector)[newIndex];
	(*m_textureInfoVector)[newIndex] = (*m_textureInfoVector)[oldIndex];
	(*m_textureInfoVector)[oldIndex] = tempInfo;
}

// ----------------------------------------------------------------------

void TextureArray1dElement::moveElementTowardBack(int elementIndex)
{
	const int entryCount = static_cast<int>(m_textureInfoVector->size());

	//-- Validate index.
	if (elementIndex >= entryCount)
	{
		DEBUG_WARNING(true, ("requested to move texture 1d-array element [index %d] closer to back, cannot do, at back.", elementIndex));
		return;
	}

	if (elementIndex < 0)
	{
		DEBUG_WARNING(true, ("requested to move non-existent texture 1d-array element [index %d] closer to back, cannot do.", elementIndex));
		return;
	}

	//-- Move the child entry toward back.
	ElementVector &elements = getChildren();

	const ElementVector::size_type  newIndex = static_cast<ElementVector::size_type>(elementIndex + 1);
	const ElementVector::size_type  oldIndex = static_cast<ElementVector::size_type>(elementIndex);

	Element *const tempElement = elements[newIndex];
	elements[newIndex]         = elements[oldIndex];
	elements[oldIndex]         = tempElement;

	NON_NULL(dynamic_cast<ArrayElement*>(elements[oldIndex]))->setIndex(static_cast<int>(oldIndex));
	NON_NULL(dynamic_cast<ArrayElement*>(elements[newIndex]))->setIndex(static_cast<int>(newIndex));

	//-- Move TextureInfo entry toward front.
	TextureInfo *const tempInfo      = (*m_textureInfoVector)[newIndex];
	(*m_textureInfoVector)[newIndex] = (*m_textureInfoVector)[oldIndex];
	(*m_textureInfoVector)[oldIndex] = tempInfo;
}

// ----------------------------------------------------------------------
/** 
 * Note: this function removes the texture choice element but does not delete it.
 */

void TextureArray1dElement::removeTextureByIndex(int elementIndex)
{
	ElementVector &elements = getChildren();

	//-- Ensure index is within valid range.
	if ((elementIndex < 0) || (elementIndex >= static_cast<int>(elements.size())))
	{
		DEBUG_WARNING(true, ("deleteTextureByIndex(%d): index not within valid range [0..%d).\n", elementIndex, static_cast<int>(elements.size())));
		return;
	}

	//-- Remove the element from the list of children.
	{
		ElementVector::iterator eraseIt = elements.begin();
		std::advance(eraseIt, static_cast<ElementVector::size_type>(elementIndex));

		IGNORE_RETURN(elements.erase(eraseIt));
	}

	//-- Remove the element from the texture info vector.
	{
		TextureInfoVector::iterator eraseIt = m_textureInfoVector->begin();
		std::advance(eraseIt, static_cast<TextureInfoVector::size_type>(elementIndex));

		delete *eraseIt;
		IGNORE_RETURN(m_textureInfoVector->erase(eraseIt));
	}

	//-- Renumber texture elements.
	renumberTextureElements();
}

// ----------------------------------------------------------------------

void TextureArray1dElement::exportTextureRenderer(Exporter &exporter) const
{
	if (m_textureInfoVector->empty())
		return;

	BlueprintWriter &writer = exporter.getWriter();

	//-- get index of first texture added
	const TextureInfoVector::const_iterator itEnd = m_textureInfoVector->end();
	TextureInfoVector::const_iterator       it    = m_textureInfoVector->begin();

	const int textureIndex = writer.addTexture(getReferencePathname((*it)->getPathname()));
	exporter.setTextureIndex(*this, textureIndex);

	//-- add rest of textures to the writer
	for (++it; it != itEnd; ++it)
	{
		const TextureInfo *const textureInfo = NON_NULL(*it);
		IGNORE_RETURN(writer.addTexture(getReferencePathname(textureInfo->getPathname())));
	}
}

// ======================================================================
// class TextureArray1dElement: PRIVATE FUNCTIONS
// ======================================================================

void TextureArray1dElement::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);
		iff.enterChunk(TAG_INFO);
		{
			char buffer[1024];

			iff.read_string(buffer, sizeof(buffer)-1);
			IGNORE_RETURN(m_arrayName->assign(buffer));

			ElementVector &children = getChildren();
			const int entryCount    = static_cast<int>(iff.read_int32());

			children.reserve(static_cast<size_t>(entryCount));
			m_textureInfoVector->reserve(static_cast<size_t>(entryCount));

			for (int i = 0; i < entryCount; ++i)
			{
				iff.read_string(buffer, sizeof(buffer)-1);
				m_textureInfoVector->push_back(new TextureInfo(buffer));
				children.push_back(new ArrayElement(*this, i));
			}
		}
		iff.exitChunk(TAG_INFO);
	iff.exitForm(TAG_0000);
}

// ----------------------------------------------------------------------
/** 
 * Reset the index values for each of the texture choice elements for this
 * 1-d texture array.
 */

void TextureArray1dElement::renumberTextureElements()
{
	int index = 0;

	ElementVector &children = getChildren();
	ElementVector::iterator const endIt = children.end();
	for (ElementVector::iterator it = children.begin(); it != endIt; ++it, ++index)
		NON_NULL(dynamic_cast<ArrayElement*>(*it))->setIndex(index);
	
}

// ======================================================================
