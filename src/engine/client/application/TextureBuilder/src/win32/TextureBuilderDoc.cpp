// ======================================================================
//
// TextureBuilderDoc.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include "FirstTextureBuilder.h"
#include "TextureBuilderDoc.h"

#include "BlueprintWriter.h"
#include "ChildFrame.h"
#include "CommandGroupElement.h"
#include "ConstructionTreeView.h"
#include "DestinationTextureElement.h"
#include "DrawTextureCommandElement.h"
#include "Element.h"
#include "Exporter.h"
#include "HueModeElement.h"
#include "MainFrame.h"
#include "RegionElement.h"
#include "TextureArray1dElement.h"
#include "TextureBakeView.h"
#include "TextureBuilder.h"
#include "TextureGroupElement.h"
#include "TextureSingleElement.h"
#include "TextureSourceModeElement.h"
#include "TextureWriteModeElement.h"
#include "VariableEditView.h"
#include "VariableElement.h"
#include "clientGraphics/Texture.h"
#include "clientGraphics/TextureList.h"
#include "resource.h"
#include "sharedFile/Iff.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/CustomizationDataProperty.h"
#include "sharedObject/MemoryBlockManagedObject.h"

#include <algorithm>
#include <string>
#include <vector>

// ======================================================================

namespace
{
	const char *const    ms_directorySeparators              = "\\/";
	const char *const    ms_textureFileFilter                = "DDS Files (*.dds)|*.dds|Targa Files (*.tga)|*.tga|All Files (*.*)|*.*||";

	const float          ms_constructionViewWidthFraction    = 0.30f;
	const float          ms_constructionViewHeightFraction   = 0.70f;

	const int            ms_defaultTextureWidth              = 256;
	const int            ms_defaultTextureHeight             = 256;
	const int            ms_defaultMipMapLevelCount          = 9;

	const TextureFormat  ms_defaultRuntimeTextureFormats[]   =
		{
			TF_RGB_888,
			TF_XRGB_8888,
			TF_RGB_565,
			TF_RGB_555
		};
	const int            ms_defaultRuntimeTextureFormatCount = sizeof(ms_defaultRuntimeTextureFormats)/sizeof(ms_defaultRuntimeTextureFormats[0]);

	const int            ms_iffWriteBufferSize               = 128 * 1024;

	const Tag            TAG_TBWS                            = TAG(T,B,W,S);

	const float          ms_defaultCameraProjectionLength    = 1.0f;

}

// ======================================================================
// class TextureBuilderDoc
// ======================================================================

TextureBuilderDoc::TextureBuilderDoc()
:	CDocument(),
	m_projectPathname(new std::string()),
	m_exportPathname(new std::string()),
	m_elements(new ElementVector()),
	m_variableElements(new VariableElementVector()),
	m_destinationTextureElement(0),
	m_textureGroupElement(new TextureGroupElement()),
	m_commandGroupElement(new CommandGroupElement()),
	m_textureBakeRequested(false),
	m_texture(TextureList::fetch(0, ms_defaultTextureWidth, ms_defaultTextureHeight, ms_defaultMipMapLevelCount, ms_defaultRuntimeTextureFormats, ms_defaultRuntimeTextureFormatCount)),
	m_activeSelection(0)
{
	m_elements->push_back(m_textureGroupElement);
	m_elements->push_back(m_commandGroupElement);
}

// ----------------------------------------------------------------------

TextureBuilderDoc::~TextureBuilderDoc()
{
	m_activeSelection = 0;

	if (m_texture)
	{
		m_texture->release();
		m_texture = 0;
	}

	m_destinationTextureElement = 0; //lint !e423 // creation of memory leak // no, deleted with list
	m_commandGroupElement       = 0; //lint !e423 // creation of memory leak // no, deleted with list
	m_textureGroupElement       = 0; //lint !e423 // creation of memory leak // no, deleted with list

	std::for_each(m_variableElements->begin(), m_variableElements->end(), PointerDeleter());
	delete m_variableElements;

	std::for_each(m_elements->begin(), m_elements->end(), PointerDeleter());
	delete m_elements;
}

// ----------------------------------------------------------------------

IMPLEMENT_DYNCREATE(TextureBuilderDoc, CDocument)

BEGIN_MESSAGE_MAP(TextureBuilderDoc, CDocument)
	//{{AFX_MSG_MAP(TextureBuilderDoc)
	ON_COMMAND(ID_VIEW_BAKE_TEXTURE, OnViewBakeTexture)
	ON_COMMAND(ID_CNSTR_TSM_1TEXTURE, OnTextureSourceModeOneTexture)
	ON_COMMAND(ID_CNSTR_TSM_2TEXTURE_BLEND, OnTextureSourceModeTwoTextureBlend)
	ON_COMMAND(ID_CNSTR_TG_ADD_1D, OnAddTextureArray1d)
	ON_COMMAND(ID_CNSTR_TA1D_ADD, OnTextureArray1dAdd)
	ON_COMMAND(ID_CNSTR_TWM_OVERWRITE, OnTextureWriteModeOverwrite)
	ON_COMMAND(ID_CNSTR_TWM_ALPHA_BLEND, OnTextureWriteModeAlphaBlend)
	ON_COMMAND(ID_CNSTR_RE_SNAP_SIZE, OnRenderElementSnapSize)
	ON_COMMAND(ID_FILE_EXPORT_AS, OnFileExportAs)
	ON_COMMAND(ID_CNSTR_HM_NONE, OnHueModeNone)
	ON_COMMAND(ID_CNSTR_HM_ONE_COLOR, OnHueModeOneColor)
	ON_COMMAND(ID_CNSTR_TX1DEL_MOVE_UP, OnTexture1dElementMoveUp)
	ON_COMMAND(ID_CNSTR_TX1DEL_MOVE_DOWN, OnTexture1dElementMoveDown)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

BOOL TextureBuilderDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	m_destinationTextureElement = new DestinationTextureElement(ms_defaultTextureWidth, ms_defaultTextureHeight);
	IGNORE_RETURN(m_elements->insert(m_elements->begin(), m_destinationTextureElement));

	displayTextureBakeView(true);
	displayVariableEditView(true);
	layoutViews();

	return TRUE;
}

// ----------------------------------------------------------------------

void TextureBuilderDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
} //lint !e1764 // Info -- Reference parameter 'ar' could be declared const ref // Part of contract.

// ----------------------------------------------------------------------

#ifdef _DEBUG
void TextureBuilderDoc::AssertValid() const
{
	CDocument::AssertValid();
}
#endif

// ----------------------------------------------------------------------

#ifdef _DEBUG
void TextureBuilderDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif

// ----------------------------------------------------------------------

void TextureBuilderDoc::getProjectDirectory(std::string &directory) const
{
	const size_t endOfDirIndex = m_projectPathname->find_last_of(ms_directorySeparators);
	if (static_cast<int>(endOfDirIndex) != m_projectPathname->npos)
	{
		// copy in the directory
		IGNORE_RETURN(directory.assign(*m_projectPathname, 0, endOfDirIndex+1));
	}
	else
	{
		// no directory in pathname
		directory = "";
	}
}

// ----------------------------------------------------------------------

void TextureBuilderDoc::getProjectRelativePath(const char *fullPathname, std::string &relativePathname) const
{
	// get project directory from project path
	std::string  projectDirectory;
	getProjectDirectory(projectDirectory);

	Os::buildRelativePath(projectDirectory.c_str(), fullPathname, relativePathname);
}

// ----------------------------------------------------------------------

void TextureBuilderDoc::viewAppendElementToParent(const Element &parent, Element &element, bool ensureVisible) const
{
	//-- find all construction tree views
	POSITION  viewPosition = GetFirstViewPosition();
	while (viewPosition != NULL)
	{
		ConstructionTreeView *const view = dynamic_cast<ConstructionTreeView*>(GetNextView(viewPosition));
		if (view)
			view->appendElementToParent(&parent, &element, ensureVisible);
	}
}

// ----------------------------------------------------------------------

void TextureBuilderDoc::viewUpdateElement(const Element &element, bool ensureVisible) const
{
	//-- find all construction tree views
	POSITION  viewPosition = GetFirstViewPosition();
	while (viewPosition != NULL)
	{
		ConstructionTreeView *const view = dynamic_cast<ConstructionTreeView*>(GetNextView(viewPosition));
		if (view)
			view->updateElement(&element, ensureVisible);
	}
}

// ----------------------------------------------------------------------

void TextureBuilderDoc::viewNotifyNewActiveSelection() const
{
	//-- find all texture bake views
	POSITION  viewPosition = GetFirstViewPosition();
	while (viewPosition != NULL)
	{
		TextureBakeView *const view = dynamic_cast<TextureBakeView*>(GetNextView(viewPosition));
		if (view)
			view->notifyNewActiveSelection();
	}
}

// ----------------------------------------------------------------------

void TextureBuilderDoc::viewNotifyNoActiveSelection() const
{
	//-- find all texture bake views
	POSITION  viewPosition = GetFirstViewPosition();
	while (viewPosition != NULL)
	{
		TextureBakeView *const view = dynamic_cast<TextureBakeView*>(GetNextView(viewPosition));
		if (view)
			view->notifyNoActiveSelection();
	}
}

// ----------------------------------------------------------------------

void TextureBuilderDoc::addSingleTexture(const std::string &relativePathname)
{
	//-- create it
	Element *newElement = new TextureSingleElement(relativePathname);

	//-- add it to list of textures
	m_textureGroupElement->getChildren().push_back(newElement);

	//-- add it to tree view
	viewAppendElementToParent(*m_textureGroupElement, *newElement, true);
} //lint !e429 // custodial pointer neither freed nor removed

// ----------------------------------------------------------------------

void TextureBuilderDoc::addDrawTextureCommand()
{
	//-- create it
	Element *newElement = new DrawTextureCommandElement();

	//-- add it to list of commands
	m_commandGroupElement->getChildren().push_back(newElement);

	//-- add it to tree view
	viewAppendElementToParent(*m_commandGroupElement, *newElement, true);
} //lint !e429 // custodial pointer leak // its okay, its in the list

// ----------------------------------------------------------------------

void TextureBuilderDoc::notifyElementDragAndDrop(Element &dragSourceElement, Element &dropTargetElement)
{
	dropTargetElement.dropElementHere(dragSourceElement);

	//-- update these elements in the views
	viewUpdateElement(dragSourceElement, true);
	viewUpdateElement(dropTargetElement, true);

	//-- rebake
	requestTextureBake();
}

// ----------------------------------------------------------------------

void TextureBuilderDoc::notifyElementLeftDoubleClick(Element &element)
{
	if (element.hasLeftDoubleClickAction())
	{
		const bool modified = element.doLeftDoubleClickAction(*this);
		if (!modified)
			return;

		//-- update construction view
		viewUpdateElement(element, true);

		//-- propagate any changes to available variables
		collectVariables();

		NOT_NULL(m_destinationTextureElement);
		if (m_destinationTextureElement->hasDescendant(element))
			createDestinationTexture();

		//-- rebake it
		requestTextureBake();
	}
}

// ----------------------------------------------------------------------

void TextureBuilderDoc::notifyVariableElementModified(const VariableElement &variableElement)
{
	UNREF(variableElement);

	//-- rebake it
	requestTextureBake();
}

// ----------------------------------------------------------------------

void TextureBuilderDoc::displayTextureBakeView(bool visible)
{
	if (visible)
	{
		
		CMultiDocTemplate &docTemplate = TextureBuilderApp::getApp().getTextureBakeViewDocTemplate();

		// create the view
		ChildFrame *const viewFrame = dynamic_cast<ChildFrame*>(docTemplate.CreateNewFrame(this, NULL));
		if (viewFrame)
		{
			docTemplate.InitialUpdateFrame(viewFrame, this);

			// set the title for the window
			std::string windowTitle = "Texture Bake View";
			viewFrame->SetWindowText(windowTitle.c_str());
		}
	}
	else
	{
		POSITION  viewPosition = GetFirstViewPosition();
		while (viewPosition != NULL)
		{
			TextureBakeView *const view = dynamic_cast<TextureBakeView*>(GetNextView(viewPosition));
			if (view)
			{
				CFrameWnd *frameWindow = view->GetParentFrame();
				IGNORE_RETURN(frameWindow->DestroyWindow());
			}
		}
	}
}

// ----------------------------------------------------------------------

void TextureBuilderDoc::displayVariableEditView(bool visible)
{
	if (visible)
	{
		
		CMultiDocTemplate &docTemplate = TextureBuilderApp::getApp().getVariableEditViewDocTemplate();

		// create the view
		ChildFrame *const viewFrame = dynamic_cast<ChildFrame*>(docTemplate.CreateNewFrame(this, NULL));
		if (viewFrame)
		{
			docTemplate.InitialUpdateFrame(viewFrame, this);

			// set the title for the window
			std::string windowTitle = "Variable Edit View";
			viewFrame->SetWindowText(windowTitle.c_str());
		}
	}
	else
	{
		POSITION  viewPosition = GetFirstViewPosition();
		while (viewPosition != NULL)
		{
			VariableEditView *const view = dynamic_cast<VariableEditView *>(GetNextView(viewPosition));
			if (view)
			{
				CFrameWnd *frameWindow = view->GetParentFrame();
				IGNORE_RETURN(frameWindow->DestroyWindow());
			}
		}
	}
}

// ----------------------------------------------------------------------

ConstructionTreeView *TextureBuilderDoc::getConstructionTreeView()
{
	POSITION  viewPosition = GetFirstViewPosition();
	while (viewPosition != NULL)
	{
		ConstructionTreeView *const view = dynamic_cast<ConstructionTreeView*>(GetNextView(viewPosition));
		if (view)
			return view;
	}

	//-- didn't find it
	return 0;
} //lint !e1762 // Info -- Member function could be made const // logically non-const.

// ----------------------------------------------------------------------

TextureBakeView *TextureBuilderDoc::getTextureBakeView()
{
	POSITION  viewPosition = GetFirstViewPosition();
	while (viewPosition != NULL)
	{
		TextureBakeView *const view = dynamic_cast<TextureBakeView*>(GetNextView(viewPosition));
		if (view)
			return view;
	}

	//-- didn't find it
	return 0;
} //lint !e1762 // Info -- Member function could be made const // logically non-const.

// ----------------------------------------------------------------------

VariableEditView *TextureBuilderDoc::getVariableEditView()
{
	POSITION  viewPosition = GetFirstViewPosition();
	while (viewPosition != NULL)
	{
		VariableEditView *const view = dynamic_cast<VariableEditView*>(GetNextView(viewPosition));
		if (view)
			return view;
	}

	//-- didn't find it
	return 0;
} //lint !e1762 // Info -- Member function could be made const // logically non-const.

// ----------------------------------------------------------------------

CommandElement *TextureBuilderDoc::whichCommandHasDescendant(const Element &element)
{
	//-- this function is not needed if we keep parent pointers in elements

	ElementVector &commands = m_commandGroupElement->getChildren();

	const ElementVector::iterator itEnd = commands.end();
	for (ElementVector::iterator it = commands.begin(); it != itEnd; ++it)
	{
		if ((*it)->hasDescendant(element))
		{
			CommandElement *commandElement = dynamic_cast<CommandElement*>(*it);
			if (commandElement)
				return commandElement;
		}
	}

	//-- didn't find it
	return NULL;
}

// ----------------------------------------------------------------------

namespace
{
	inline bool VariableElementLess(const VariableElement *lhs, const VariableElement *rhs)
	{
		NOT_NULL(lhs);
		NOT_NULL(rhs);
		return lhs->getVariableName() < rhs->getVariableName();
	}
}

// ----------------------------------------------------------------------

void TextureBuilderDoc::collectVariables()
{
	//-- Create a CustomizationData instance.
	Object            *object              = new MemoryBlockManagedObject;
	CustomizationData *savedVariableValues = new CustomizationData(*object);
	savedVariableValues->fetch();

	//-- keep track of the value of the old variable elements
	{
		const VariableElementVector::const_iterator itEnd = m_variableElements->end();
		for (VariableElementVector::const_iterator it = m_variableElements->begin(); it != itEnd; ++it)
		{
			const VariableElement *const variableElement = *it;
			variableElement->saveToCustomizationData(*savedVariableValues);
		}
	}

	//-- delete existing variable elements
	std::for_each(m_variableElements->begin(), m_variableElements->end(), PointerDeleter());
	m_variableElements->clear();

	//-- reconstruct the vector of variable elements from the commands
	{
		const ElementVector                 &commands = m_commandGroupElement->getChildren();
		const ElementVector::const_iterator  itEnd    = commands.end();
		for (ElementVector::const_iterator  it = commands.begin(); it != itEnd; ++it)
			(*it)->createHierarchyVariableElements(*m_variableElements);
	}

	//-- remove duplicate variable elements
	std::sort(m_variableElements->begin(), m_variableElements->end(), VariableElementLess);
	{
		// -TRF- once we start using ref counted smart pointers, this can be turned into a call to unique.  for now
		//       we have ugliness.  One alternative is to use a map instead, but this complicates all the elements.
		for (VariableElementVector::iterator it = m_variableElements->begin(); it != m_variableElements->end();)
		{
			if ((it != m_variableElements->begin()) && ((*it)->getVariableName() == (*(it-1))->getVariableName()))
			{
				//-- delete the duplicate.  note we assume that two variables of the same name are also of the same type and same bounds.
				//   if this is not the case, the user should create a different variable.
				delete *it;
				it = m_variableElements->erase(it);
			}
			else
				++it;
		}
	}

	//-- initialize variable values from saved values
	{
		const VariableElementVector::iterator itEnd = m_variableElements->end();
		for (VariableElementVector::iterator it = m_variableElements->begin(); it != itEnd; ++it)
			(*it)->loadFromCustomizationData(*savedVariableValues);
	}

	//-- Relesae local references.
	savedVariableValues->release();
	savedVariableValues = 0;

	delete object;

	//-- rebuild variable edit view
	VariableEditView *view = getVariableEditView();
	view->replaceVariableElements(*m_variableElements);
}

// ----------------------------------------------------------------------

void TextureBuilderDoc::layoutViews()
{
	//-- get main frame client area
	RECT  mainFrameRect;

	MainFrame &frame = TextureBuilderApp::getApp().getMainFrame();
	frame.getClientRect(mainFrameRect);

	//-- calculate construction tree view rect
	RECT ctvRect;
	ctvRect       = mainFrameRect;
	ctvRect.right = static_cast<long>((mainFrameRect.right - mainFrameRect.left) * ms_constructionViewWidthFraction);

	//-- calculate variable edit view rect
	RECT vevRect   = ctvRect;
	vevRect.top    = static_cast<long>((mainFrameRect.bottom - mainFrameRect.top) * ms_constructionViewHeightFraction);
	ctvRect.bottom = vevRect.top;

	//-- calculate texture bake view rect
	RECT tbvRect;
	tbvRect       = mainFrameRect;
	tbvRect.left  = ctvRect.right;

	const UINT setPositionFlags = SWP_DRAWFRAME | SWP_SHOWWINDOW;

	//-- place the construction tree view
	ConstructionTreeView *const ctv = getConstructionTreeView();
	if (ctv)
	{
		CWnd *childFrame = ctv->GetParent();
		FATAL(!childFrame, ("failed to get view frame"));

		IGNORE_RETURN(childFrame->SetWindowPos(&CWnd::wndTop, ctvRect.left, ctvRect.top, ctvRect.right-ctvRect.left, ctvRect.bottom-ctvRect.top, setPositionFlags));
	}

	//-- place the variable edit view
	VariableEditView *const vev = getVariableEditView();
	if (vev)
	{
		CWnd *childFrame = vev->GetParent();
		FATAL(!childFrame, ("failed to get view frame"));

		IGNORE_RETURN(childFrame->SetWindowPos(&CWnd::wndTop, vevRect.left, vevRect.top, vevRect.right-vevRect.left, vevRect.bottom-vevRect.top, setPositionFlags));
	}

	//-- place the texture bake view
	TextureBakeView *const tbv = getTextureBakeView();
	if (tbv)
	{
		CWnd *childFrame = tbv->GetParent();
		FATAL(!childFrame, ("failed to get view frame"));

		IGNORE_RETURN(childFrame->SetWindowPos(&CWnd::wndTop, tbvRect.left, tbvRect.top, tbvRect.right-tbvRect.left, tbvRect.bottom-tbvRect.top, setPositionFlags));
	}
}

// ----------------------------------------------------------------------

void TextureBuilderDoc::bakeTexture() const
{
	//-- Create a CustomizationData instance.
	Object            *object              = new MemoryBlockManagedObject;
	CustomizationData *customizationData   = new CustomizationData(*object);
	customizationData->fetch();

	//-- construct the CustomizationData object from the VariableElement objects
	{
		const VariableElementVector::const_iterator itEnd = m_variableElements->end();
		for (VariableElementVector::const_iterator it = m_variableElements->begin(); it != itEnd; ++it)
		{
			const VariableElement *const variableElement = *it;
			variableElement->saveToCustomizationData(*customizationData);
		}
	}

	//-- apply all commands to the texture
	{
		Element::ElementVector &commandChildren      = m_commandGroupElement->getChildren();
		Element::ElementVector::const_iterator itEnd = commandChildren.end();
		for (Element::ElementVector::const_iterator it = commandChildren.begin(); it != itEnd; ++it)
		{
			const CommandElement *commandElement = dynamic_cast<const CommandElement*>(*it);
			DEBUG_FATAL(!commandElement, ("child of command group element not derived from CommandElement!"));

			commandElement->applyCommand(*m_texture, m_textureGroupElement->getChildren(), *customizationData);
		}
	}

	static int callCount = 0;
	++callCount;
	DEBUG_REPORT_LOG(true, ("bakeTexture [%d]\n", callCount));

	//-- Release local references.
	customizationData->release();
	delete object;
}

// ----------------------------------------------------------------------

void TextureBuilderDoc::OnViewBakeTexture()
{
	bakeTexture();

	//-- update the views
	UpdateAllViews(NULL);
}

// ----------------------------------------------------------------------

void TextureBuilderDoc::setTextureSourceMode(int value)
{
	//-- get the selected element from the construction tree view
	ConstructionTreeView *view = getConstructionTreeView();
	NOT_NULL(view);

	Element *selectedElement = view->getSelectedElement();
	DEBUG_FATAL(!selectedElement, ("setTextureSourceMode() but no element is selected in tree view"));

	TextureSourceModeElement *tsmElement = dynamic_cast<TextureSourceModeElement*>(selectedElement);
	DEBUG_FATAL(!tsmElement, ("setTextureSourceMode() but selected element is not a TextureSourceModeElement object"));

	DrawTextureCommandElement &dtcElement = tsmElement->getDrawTextureCommandElement();

	//-- set source mode
	dtcElement.setSourceTextureMode(static_cast<DrawTextureCommandElement::SourceTextureMode>(value));

	//-- update this element
	viewUpdateElement(dtcElement, true);

	//-- propagate any changes to available variables
	collectVariables();

	//-- request bake
	requestTextureBake();
}

// ----------------------------------------------------------------------

void TextureBuilderDoc::OnTextureSourceModeOneTexture() 
{
	setTextureSourceMode(DrawTextureCommandElement::STM_texture1);
}

// ----------------------------------------------------------------------

void TextureBuilderDoc::OnTextureSourceModeTwoTextureBlend() 
{
	setTextureSourceMode(DrawTextureCommandElement::STM_textureBlend2);
}

// ----------------------------------------------------------------------

void TextureBuilderDoc::setTextureWriteMode(int value)
{
	//-- get the selected element from the construction tree view
	ConstructionTreeView *view = getConstructionTreeView();
	NOT_NULL(view);

	Element *selectedElement = view->getSelectedElement();
	DEBUG_FATAL(!selectedElement, ("setTextureWriteMode() but no element is selected in tree view"));

	TextureWriteModeElement *twmElement = dynamic_cast<TextureWriteModeElement*>(selectedElement);
	DEBUG_FATAL(!twmElement, ("setTextureWriteMode() but selected element is not a TextureWriteModeElement object"));

	DrawTextureCommandElement &dtcElement = twmElement->getDrawTextureCommandElement();

	//-- set source mode
	dtcElement.setWriteTextureMode(static_cast<DrawTextureCommandElement::WriteTextureMode>(value));

	//-- update this element
	viewUpdateElement(dtcElement, true);

	//-- request bake
	requestTextureBake();
}

// ----------------------------------------------------------------------

void TextureBuilderDoc::OnTextureWriteModeOverwrite() 
{
	setTextureWriteMode(DrawTextureCommandElement::WTM_overwrite);
}

// ----------------------------------------------------------------------

void TextureBuilderDoc::OnTextureWriteModeAlphaBlend() 
{
	setTextureWriteMode(DrawTextureCommandElement::WTM_alphaBlend);
}

// ----------------------------------------------------------------------

void TextureBuilderDoc::requestTextureBake()
{
	m_textureBakeRequested = true;

	//-- update the views.  this will (eventually) cause the TextureBakeView to ask for
	//   the texture, which will force the texture to be baked.
	const_cast<TextureBuilderDoc*>(this)->UpdateAllViews(NULL);
}

// ----------------------------------------------------------------------

Texture &TextureBuilderDoc::getTexture()
{
	//-- perform lazy texture baking: only do it before we need to.
	//   this helps speed up the user interface when the user drags the
	//   slider quickly.

	if (m_textureBakeRequested)
	{
		bakeTexture();
		m_textureBakeRequested = false;
	}

	return *NON_NULL(m_texture);
}

// ----------------------------------------------------------------------

const Texture &TextureBuilderDoc::getTexture() const
{
	//-- perform lazy texture baking: only do it before we need to.
	//   this helps speed up the user interface when the user drags the
	//   slider quickly.

	if (m_textureBakeRequested)
	{
		bakeTexture();
		m_textureBakeRequested = false;
	}

	return *NON_NULL(m_texture);
}

// ----------------------------------------------------------------------

int TextureBuilderDoc::getTextureWidth() const
{
	return m_texture->getWidth();
}

// ----------------------------------------------------------------------

int TextureBuilderDoc::getTextureHeight() const
{
	return m_texture->getHeight();
}

// ----------------------------------------------------------------------

void TextureBuilderDoc::OnAddTextureArray1d() 
{
	//-- create it
	Element *newElement = new TextureArray1dElement();

	//-- add it to list of textures
	m_textureGroupElement->getChildren().push_back(newElement);

	//-- add it to tree view
	viewAppendElementToParent(*m_textureGroupElement, *newElement, true);
} //lint !e429 // custodial pointer newElement not freed or returned

// ----------------------------------------------------------------------

void TextureBuilderDoc::OnTextureArray1dAdd() 
{
	//-- get the selected element
	Element *const selectedElement = NON_NULL(getConstructionTreeView())->getSelectedElement();
	FATAL(!selectedElement, ("how did this get called?"));

	TextureArray1dElement *const arrayElement = dynamic_cast<TextureArray1dElement*>(selectedElement);
	FATAL(!arrayElement, ("how did this get called?"));

	//-- pop up a dialog to retrieve texture name
	TextureBuilderApp &app                 = TextureBuilderApp::getApp();
	const std::string &lastTexturePathname = app.getLastTexturePathname();

	bool  done;

	do
	{
		done = false;

		CFileDialog dlg(TRUE, "dds", lastTexturePathname.c_str(), 0, ms_textureFileFilter, NULL);
		if (dlg.DoModal() != IDOK)
		{
			//-- user canceled, not modified
			done = true;
		}
		else
		{
			//-- Get the full pathname to the file.
			const CString fullPathName = dlg.GetPathName();

			//-- Convert full pathname to TreeFile-relative pathname.
			std::string  treeFilePathName;

			const bool fsToTreeSuccess = TreeFile::stripTreeFileSearchPathFromFile(std::string(fullPathName), treeFilePathName);
			if (fsToTreeSuccess)
			{
				if (arrayElement->hasPathname(treeFilePathName))
				{
					//-- User already entered this pathname.
					std::string  message("Texture ");
					message += treeFilePathName;
					message += "already assigned.";

					IGNORE_RETURN(MessageBox(NULL, message.c_str(), "Duplicate texture entry", MB_ICONSTOP | MB_OK));
				}
				else
				{
					//-- I can change this one.
					arrayElement->addTexture(treeFilePathName);
					done = true;

					//-- Update the element
					viewUpdateElement(*arrayElement, true);

					//-- Remember this texture filesystem pathname.
					app.setLastTexturePathname(fullPathName);
				}
			}
			else
			{
				//-- Indicate the specified filename was not in the TreeFile path.
				char buffer[1024];

				sprintf(buffer, "The specified file [%s] is not within the TreeFile search path, ignoring.", static_cast<const char*>(fullPathName));
				MessageBox(NULL, buffer, "File Not in TreeFile Path", MB_OK | MB_ICONSTOP);
			}
		}
	} while (!done);
}

// ----------------------------------------------------------------------

void TextureBuilderDoc::getActiveSelection(Selection &selection) const
{
	FATAL(!m_activeSelection, ("no active selection, call hasActiveSelection()"));

	selection.m_x0 = m_activeSelection->getX0();
	selection.m_y0 = m_activeSelection->getY0();
	selection.m_x1 = m_activeSelection->getX1();
	selection.m_y1 = m_activeSelection->getY1();
}

// ----------------------------------------------------------------------

void TextureBuilderDoc::setActiveSelection(const Selection &selection)
{
	FATAL(!m_activeSelection, ("set active selection region when doc has no active selection"));

	m_activeSelection->setX0(selection.m_x0);
	m_activeSelection->setY0(selection.m_y0);
	m_activeSelection->setX1(selection.m_x1);
	m_activeSelection->setY1(selection.m_y1);

	viewUpdateElement(*m_activeSelection, true);
}

// ----------------------------------------------------------------------

void TextureBuilderDoc::notifyElementSelected(Element &element)
{
	//-- check for a region element
	RegionElement *const regionElement = dynamic_cast<RegionElement*>(&element);
	if (regionElement)
	{
		//-- check if its the same as previously selected
		if (regionElement != m_activeSelection)
		{
			// set the active selection
			m_activeSelection = regionElement;

			// notify views of new selection
			viewNotifyNewActiveSelection();
		}
	}
	else
	{
		//-- if we had a region selected, notify views that we no longer have a region selected
		if (m_activeSelection)
		{
			viewNotifyNoActiveSelection();
			m_activeSelection = 0;
		}
	}
}

// ----------------------------------------------------------------------

BOOL TextureBuilderDoc::OnSaveDocument(LPCTSTR lpszPathName) 
{
	//-- create the iff memory image
	Iff iff(ms_iffWriteBufferSize);

	iff.insertForm(TAG_TBWS);
		iff.insertForm(TAG_0001);
		{
			//-- write general texture builder doc info
			iff.insertChunk(TAG_INFO);

				iff.insertChunkString(m_exportPathname->c_str());

			iff.exitChunk(TAG_INFO);

			//-- write destination texture
			NOT_NULL(m_destinationTextureElement);
			if (!m_destinationTextureElement->writeForWorkspace(iff))
				return false;

			//-- write source textures
			{
				ElementVector &children = m_textureGroupElement->getChildren();
				const ElementVector::const_iterator itEnd = children.end();
				for (ElementVector::const_iterator it = children.begin(); it != itEnd; ++it)
				{
					if (!(*it)->writeForWorkspace(iff))
						return false;
				}
			}

			//-- write commands
			{
				ElementVector &children = m_commandGroupElement->getChildren();
				const ElementVector::const_iterator itEnd = children.end();
				for (ElementVector::const_iterator it = children.begin(); it != itEnd; ++it)
				{
					if (!(*it)->writeForWorkspace(iff))
						return false;
				}
			}
		}
		iff.exitForm(TAG_0001);
	iff.exitForm(TAG_TBWS);

	//-- write iff to disk
	return iff.write(lpszPathName, true) ? TRUE : FALSE;
}

// ----------------------------------------------------------------------

bool TextureBuilderDoc::loadWorkspace_0000(Iff &iff)
{
	if (!iff.enterForm(TAG_0000, true))
		return false;
	{
		while (!iff.atEndOfForm())
		{
			if (DestinationTextureElement::isPersistedNext(iff))
			{
				FATAL(m_destinationTextureElement, ("m_destinationTextureElement not null, unexpected"));
				m_destinationTextureElement = new DestinationTextureElement(iff);
				IGNORE_RETURN(m_elements->insert(m_elements->begin(), m_destinationTextureElement));
			}
			else if (TextureSingleElement::isPersistedNext(iff))
			{
				ElementVector &children = m_textureGroupElement->getChildren();
				children.push_back(new TextureSingleElement(iff));
			}
			else if (TextureArray1dElement::isPersistedNext(iff))
			{
				ElementVector &children = m_textureGroupElement->getChildren();
				children.push_back(new TextureArray1dElement(iff));
			}
			else if (DrawTextureCommandElement::isPersistedNext(iff))
			{
				ElementVector &children = m_commandGroupElement->getChildren();
				children.push_back(new DrawTextureCommandElement(iff, m_textureGroupElement->getChildren()));
			}
		}
	}
	iff.exitForm(TAG_0000);

	return true;
}

// ----------------------------------------------------------------------

bool TextureBuilderDoc::loadWorkspace_0001(Iff &iff)
{
	if (!iff.enterForm(TAG_0001, true))
		return false;

	{
		iff.enterChunk(TAG_INFO);
		{
			char buffer[1024];

			iff.read_string(buffer, sizeof(buffer)-1);
			IGNORE_RETURN(m_exportPathname->assign(buffer));
		}
		iff.exitChunk(TAG_INFO);

		while (!iff.atEndOfForm())
		{
			if (DestinationTextureElement::isPersistedNext(iff))
			{
				FATAL(m_destinationTextureElement, ("m_destinationTextureElement not null, unexpected"));
				m_destinationTextureElement = new DestinationTextureElement(iff);
				IGNORE_RETURN(m_elements->insert(m_elements->begin(), m_destinationTextureElement));
			}
			else if (TextureSingleElement::isPersistedNext(iff))
			{
				ElementVector &children = m_textureGroupElement->getChildren();
				children.push_back(new TextureSingleElement(iff));
			}
			else if (TextureArray1dElement::isPersistedNext(iff))
			{
				ElementVector &children = m_textureGroupElement->getChildren();
				children.push_back(new TextureArray1dElement(iff));
			}
			else if (DrawTextureCommandElement::isPersistedNext(iff))
			{
				ElementVector &children = m_commandGroupElement->getChildren();
				children.push_back(new DrawTextureCommandElement(iff, m_textureGroupElement->getChildren()));
			}
		}
	}
	iff.exitForm(TAG_0001);

	return true;
}

// ----------------------------------------------------------------------

BOOL TextureBuilderDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	//-- open the iff

	// make sure it exists
	if (!TreeFile::exists(lpszPathName))
		return FALSE;

	// load the iff
	Iff iff(lpszPathName, true);
	if (!iff.getRawDataSize())
	{
		//-- assume failed to open
		return FALSE;
	}

	if (!iff.enterForm(TAG_TBWS, true))
		return FALSE;
	{
		bool versionLoadResult;

		switch (iff.getCurrentName())
		{
			case TAG_0000:
				versionLoadResult = loadWorkspace_0000(iff);
				break;
			case TAG_0001:
				versionLoadResult = loadWorkspace_0001(iff);
				break;
			default:
				versionLoadResult = false;
				break;
		}

		if (!versionLoadResult)
			return FALSE;
	}

	iff.exitForm(TAG_TBWS);

	//-- display and layout views
	displayTextureBakeView(true);
	displayVariableEditView(true);
	layoutViews();

	//-- propagate any changes to available variables
	collectVariables();

	//-- Create the bake's destination texture with the appropriate attributes.
	createDestinationTexture();

	//-- request bake
	requestTextureBake();
	
	return TRUE;
}

// ----------------------------------------------------------------------

void TextureBuilderDoc::OnRenderElementSnapSize() 
{
	// snap the region's size to that of the underlying texture.
	// what this means is that the selection region will be set
	// to a size with dimensions set to do a 1-1 map from source
	// texture to our destination texture.

	//-- find region's draw command
	// get the region
	Element *const selectedElement = getConstructionTreeView()->getSelectedElement();
	DEBUG_FATAL(!selectedElement, ("No selected element, how did we get here?"));

	RegionElement *const regionElement = dynamic_cast<RegionElement*>(selectedElement);
	DEBUG_FATAL(!regionElement, ("Selected element is not a region, how did we get here?"));

	CommandElement *const commandElement = whichCommandHasDescendant(*regionElement);
	DEBUG_FATAL(!commandElement, ("No command has the region element as a descendant"));

	DrawTextureCommandElement *const drawTextureCommandElement = dynamic_cast<DrawTextureCommandElement*>(commandElement);
	DEBUG_FATAL(!drawTextureCommandElement, ("Containing command is not a draw texture command"));

	//-- get size of destination texture
	NOT_NULL(m_destinationTextureElement);

	const int destWidth  = m_destinationTextureElement->getWidth();
	const int destHeight = m_destinationTextureElement->getHeight();

	//-- retrieve the texture width and height of the first filled image slot in the draw command
	int imageWidth = destWidth;
	int imageHeight = destHeight;
	
	drawTextureCommandElement->getFirstFilledImageSlotDimensions(imageWidth, imageHeight);

	const float regionWidth  = static_cast<float>(imageWidth) / static_cast<float>(destWidth);
	const float regionHeight = static_cast<float>(imageHeight) / static_cast<float>(destHeight);

	//-- keep same x0, y0, simply modify width and height
	regionElement->setX1(regionElement->getX0() + regionWidth);
	regionElement->setY1(regionElement->getY0() + regionHeight);

	//-- update the element, ask for a rebake
	viewUpdateElement(*regionElement, true);

	requestTextureBake();

	TextureBakeView *const view = getTextureBakeView();
	view->notifyNewActiveSelection();
}

// ----------------------------------------------------------------------

void TextureBuilderDoc::exportTextureRenderer(const std::string &pathname) const
{
	Exporter  exporter;

	//-- fill writer with data
	BlueprintWriter &writer = exporter.getWriter();

	//-- setup variable type
	// writer.setVariableOwnership(static_cast<uint>(AvailableVariables::O_asset));
	writer.setVariableOwnership(1);

	//-- setup dest texture

	// -TRF- setup through interface
	std::vector<TextureFormat>  textureFormats;

	textureFormats.reserve(4);
	textureFormats.push_back(TF_RGB_888);
	textureFormats.push_back(TF_XRGB_8888);
	textureFormats.push_back(TF_RGB_565);
	textureFormats.push_back(TF_RGB_555);
	
	NOT_NULL(m_destinationTextureElement);
	writer.setDestTextureAttributes(m_destinationTextureElement->getWidth(), m_destinationTextureElement->getHeight(), textureFormats);

	//-- setup camera
	writer.setCamera(ms_defaultCameraProjectionLength);

	//-- write texture data
	{
		const ElementVector &textures = m_textureGroupElement->getChildren();
		const ElementVector::const_iterator itEnd = textures.end();
		for (ElementVector::const_iterator it = textures.begin(); it != itEnd; ++it)
		{
			const TextureElement *const textureElement = dynamic_cast<TextureElement*>(*it);
			DEBUG_FATAL(!textureElement, ("child of texture group element not a TextureElement descendant"));

			textureElement->exportTextureRenderer(exporter);
		}
	}

	//-- write commands
	{
		const ElementVector &commands = m_commandGroupElement->getChildren();
		const ElementVector::const_iterator itEnd = commands.end();
		for (ElementVector::const_iterator it = commands.begin(); it != itEnd; ++it)
		{
			const CommandElement *const commandElement = dynamic_cast<CommandElement*>(*it);
			DEBUG_FATAL(!commandElement, ("child of command group element not a CommandElement descendant"));

			commandElement->exportTextureRenderer(exporter);
		}
	}

	//-- write to iff
	Iff  iff(ms_iffWriteBufferSize);

	writer.write(iff);

	//-- save to file
	IGNORE_RETURN(iff.write(pathname.c_str()));
}

// ----------------------------------------------------------------------

void TextureBuilderDoc::OnFileExportAs() 
{
	CFileDialog  dlg(FALSE, ".trt", NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "Texture Renderer Template Files (*.trt)|*.trt||", NULL);

	if (dlg.DoModal() == IDOK)
	{
		*(m_exportPathname.get()) = static_cast<const char*>(dlg.GetPathName());
		exportTextureRenderer(*m_exportPathname);
	}
}

// ----------------------------------------------------------------------

void TextureBuilderDoc::setHueMode(int value)
{
	//-- Get the selected element from the construction tree view.
	ConstructionTreeView *view = getConstructionTreeView();
	NOT_NULL(view);

	Element *selectedElement = view->getSelectedElement();
	DEBUG_FATAL(!selectedElement, ("setTextureSourceMode() but no element is selected in tree view"));

	//-- Ensure it is a HueModeElement.
	HueModeElement *hmElement = dynamic_cast<HueModeElement*>(selectedElement);
	DEBUG_FATAL(!hmElement, ("SetHueMode() called but selected element is not a HueModeElement instance."));

	DrawTextureCommandElement &dtcElement = hmElement->getDrawTextureCommandElement();

	//-- Set hue mode.
	dtcElement.setHueMode(static_cast<DrawTextureCommandElement::HueMode>(value));

	//-- Update this element.
	viewUpdateElement(dtcElement, true);

	//-- Propagate any changes to available variables
	collectVariables();

	//-- Request bake.
	requestTextureBake();
}

// ----------------------------------------------------------------------

void TextureBuilderDoc::OnHueModeNone() 
{
	setHueMode(DrawTextureCommandElement::HM_none);
}

// ----------------------------------------------------------------------

void TextureBuilderDoc::OnHueModeOneColor() 
{
	setHueMode(DrawTextureCommandElement::HM_oneColor);
}

// ----------------------------------------------------------------------

void TextureBuilderDoc::OnTexture1dElementMoveUp() 
{
	//-- Get the parent of the selected element from the construction tree view.
	ConstructionTreeView *view = getConstructionTreeView();
	NOT_NULL(view);

	Element *parentElement = view->getSelectedElementParent();
	DEBUG_FATAL(!parentElement, ("OnTexture1dElementMoveUp() called but no element is selected or element has no parent."));

	//-- Convert parent to 1-d texture array element.
	TextureArray1dElement *const arrayElement = dynamic_cast<TextureArray1dElement*>(parentElement);
	NOT_NULL(arrayElement);

	//-- Retrieve the index of the selected element.
	const int childIndex = view->getSelectedElementChildIndex();

	//-- Move the texture array's child texture.
	arrayElement->moveElementTowardFront(childIndex);

	//-- Render.
	viewUpdateElement(*arrayElement, true);
}

// ----------------------------------------------------------------------

void TextureBuilderDoc::OnTexture1dElementMoveDown() 
{
	//-- Get the parent of the selected element from the construction tree view.
	ConstructionTreeView *view = getConstructionTreeView();
	NOT_NULL(view);

	Element *parentElement = view->getSelectedElementParent();
	DEBUG_FATAL(!parentElement, ("OnTexture1dElementMoveUp() called but no element is selected or element has no parent."));

	//-- Convert parent to 1-d texture array element.
	TextureArray1dElement *const arrayElement = dynamic_cast<TextureArray1dElement*>(parentElement);
	NOT_NULL(arrayElement);

	//-- Retrieve the index of the selected element.
	const int childIndex = view->getSelectedElementChildIndex();

	//-- Move the texture array's child texture.
	arrayElement->moveElementTowardBack(childIndex);

	//-- Render.
	viewUpdateElement(*arrayElement, true);
}

// ----------------------------------------------------------------------

void TextureBuilderDoc::createDestinationTexture()
{
	//-- Release existing texture.
	if (m_texture)
	{
		m_texture->release();
		m_texture = 0;
	}

	//-- Get desired texture attributes from destination texture element.
	const int textureWidth     = m_destinationTextureElement->getWidth();
	const int textureHeight    = m_destinationTextureElement->getHeight();
	const int mipmapLevelCount = GetFirstBitSet(std::max(textureWidth, textureHeight)) + 1;

	m_texture = TextureList::fetch(0, textureWidth, textureHeight, mipmapLevelCount, ms_defaultRuntimeTextureFormats, ms_defaultRuntimeTextureFormatCount);
}

// ======================================================================
