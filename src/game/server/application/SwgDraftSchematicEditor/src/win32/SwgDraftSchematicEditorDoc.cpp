// ======================================================================
//
// SwgDraftSchematicEditorDoc.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "FirstSwgDraftSchematicEditor.h"
#include "SwgDraftSchematicEditorDoc.h"

#include "Configuration.h"
#include "MainFrame.h"
#include "Resource.h"

// ======================================================================

namespace SwgDraftSchematicEditorDocNamespace
{
	bool isInvalidStringId (CString const & stringId)
	{
		if (stringId.IsEmpty ())
			return true;

		if (stringId [0] < 'a' || stringId [0] > 'z')
			return true;

		for (int i = 1; i < stringId.GetLength (); ++i)
		{
			if (stringId [i] >= 'a' && stringId [i] <= 'z')
				continue;
			
			if (isdigit (stringId [i]))
				continue;
			
			if (stringId [i] == '_')
				continue;

			return true;
		}

		return false;
	}
}

using namespace SwgDraftSchematicEditorDocNamespace;

// ======================================================================

SwgDraftSchematicEditorDoc::ItemData::ItemData () :
	m_type (T_unknown),
	m_void (0)
{
}

// ----------------------------------------------------------------------

SwgDraftSchematicEditorDoc::ItemData::~ItemData ()
{
}

// ----------------------------------------------------------------------

CString SwgDraftSchematicEditorDoc::ItemData::getName () const
{
	switch (m_type)
	{
	case T_unknown:
	default:
		break;

	case T_attribute:
		return m_attribute->getName ();

	case T_slot:
		return m_slot->getName ();
	}

	return "";
}

// ======================================================================

IMPLEMENT_DYNCREATE(SwgDraftSchematicEditorDoc, CDocument)

BEGIN_MESSAGE_MAP(SwgDraftSchematicEditorDoc, CDocument)
	//{{AFX_MSG_MAP(SwgDraftSchematicEditorDoc)
	ON_COMMAND(ID_BUTTON_COMPILE, OnButtonCompile)
	ON_COMMAND(ID_BUTTON_P4EDIT, OnButtonP4edit)
	ON_COMMAND(ID_BUTTON_SCAN, OnButtonScan)
	ON_COMMAND(ID_BUTTON_SAVETEST, OnButtonSavetest)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

SwgDraftSchematicEditorDoc::SwgDraftSchematicEditorDoc()
{
	// TODO: add one-time construction code here

}

// ----------------------------------------------------------------------

SwgDraftSchematicEditorDoc::~SwgDraftSchematicEditorDoc()
{
}

// ----------------------------------------------------------------------

BOOL SwgDraftSchematicEditorDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	m_draftSchematic.setBaseServerObjectTemplate (Configuration::getDefaultBaseServerDraftSchematicObjectTemplate ());
	m_draftSchematic.setOverrideCraftingTypeValue (Configuration::getDefaultCraftingType ());
	m_draftSchematic.setOverrideXpTypeValue (Configuration::getDefaultXpType ());
	m_draftSchematic.setBaseSharedObjectTemplate (DraftSchematic::createSharedObjectTemplateName (Configuration::getDefaultBaseServerDraftSchematicObjectTemplate ()));

	DraftSchematic::Attribute attribute;
	DraftSchematic::createDefaultAttribute (attribute);
	attribute.m_nameStringTable = "crafting";
	attribute.m_nameStringId = "complexity";
	m_draftSchematic.addAttribute (new DraftSchematic::Attribute (attribute));

	attribute.m_nameStringTable = "crafting";
	DraftSchematic::createDefaultAttribute (attribute);
	attribute.m_nameStringId = "xp";
	m_draftSchematic.addAttribute (new DraftSchematic::Attribute (attribute));

	DraftSchematic::Slot slot;
	DraftSchematic::createDefaultSlot (slot);
	m_draftSchematic.addSlot (new DraftSchematic::Slot (slot));

	DraftSchematic::createDefaultSlot (slot);
	m_draftSchematic.addSlot (new DraftSchematic::Slot (slot));

	return TRUE;
}

// ----------------------------------------------------------------------

void SwgDraftSchematicEditorDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

// ----------------------------------------------------------------------

#ifdef _DEBUG
void SwgDraftSchematicEditorDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void SwgDraftSchematicEditorDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

// ----------------------------------------------------------------------

BOOL SwgDraftSchematicEditorDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	if (!m_draftSchematic.load (lpszPathName))
		return FALSE;
	
	updateTpfTabs (lpszPathName);

	return TRUE;
}

// ----------------------------------------------------------------------

BOOL SwgDraftSchematicEditorDoc::OnSaveDocument(LPCTSTR lpszPathName) 
{
	if (!m_draftSchematic.save (lpszPathName))
		return FALSE;

	SetModifiedFlag (false);

	updateTpfTabs (lpszPathName);
	safe_cast<MainFrame *> (AfxGetMainWnd ())->refreshDirectory ();

	return TRUE;
}

// ----------------------------------------------------------------------

void SwgDraftSchematicEditorDoc::SetModifiedFlag (BOOL const bModified)
{
	CDocument::SetModifiedFlag (bModified);

	if (bModified)
		updateTpfTabs (GetPathName ());
}

// ----------------------------------------------------------------------

void SwgDraftSchematicEditorDoc::OnButtonCompile() 
{
	CString pathName = GetPathName ();
	pathName.MakeLower ();
	if (pathName.GetLength () == 0)
	{
		MessageBox (0, "Please save the conversation before editing/adding to Perforce.", AfxGetApp ()->m_pszAppName, MB_OK);
		return;
	}

	OnSaveDocument (GetPathName ());

	CString const serverObjectTemplateName (DraftSchematic::createServerObjectTemplateName (pathName));
	CONSOLE_EXECUTE ("templatecompiler -compileeditor " + serverObjectTemplateName);

	CString const sharedObjectTemplateName (DraftSchematic::createSharedObjectTemplateName (pathName));
	CONSOLE_EXECUTE ("templatecompiler -compileeditor " + sharedObjectTemplateName);
}

// ----------------------------------------------------------------------

void SwgDraftSchematicEditorDoc::OnButtonP4edit() 
{
	CString pathName = GetPathName ();
	pathName.MakeLower ();
	if (pathName.GetLength () == 0)
	{
		MessageBox (0, "Please save the conversation before editing/adding to Perforce.", AfxGetApp ()->m_pszAppName, MB_OK);
		return;
	}

	OnSaveDocument (GetPathName ());

	//-- server tpf
	CString serverObjectTemplateName (DraftSchematic::createServerObjectTemplateName (pathName));
	CONSOLE_EXECUTE ("p4 edit " + serverObjectTemplateName);
	CONSOLE_EXECUTE ("p4 add " + serverObjectTemplateName);

	//-- server iff
	serverObjectTemplateName.Replace ("/dsrc/", "/data/");
	serverObjectTemplateName.Replace (".tpf", ".iff");	
	CONSOLE_EXECUTE ("p4 edit " + serverObjectTemplateName);
	CONSOLE_EXECUTE ("p4 add " + serverObjectTemplateName);

	//-- shared tpf
	CString sharedObjectTemplateName (DraftSchematic::createSharedObjectTemplateName (pathName));
	CONSOLE_EXECUTE ("p4 edit " + sharedObjectTemplateName);
	CONSOLE_EXECUTE ("p4 add " + sharedObjectTemplateName);

	//-- shared iff
	sharedObjectTemplateName.Replace ("/dsrc/", "/data/");
	sharedObjectTemplateName.Replace (".tpf", ".iff");	
	CONSOLE_EXECUTE ("p4 edit " + sharedObjectTemplateName);
	CONSOLE_EXECUTE ("p4 add " + sharedObjectTemplateName);
}

// ----------------------------------------------------------------------

void SwgDraftSchematicEditorDoc::OnButtonScan() 
{
	CONSOLE_PRINT ("---------- Scanning ----------\r\n");
	CString result;

	if (m_draftSchematic.getComplexity () == 0)
		result += "Complexity is 0\r\n";

	if (m_draftSchematic.getItemsPerContainer () == 0)
		result += "Items per container is 0\r\n";

	if (m_draftSchematic.getCraftedServerObjectTemplate ().IsEmpty ())
		result += "Server crafted object template is empty\r\n";

	if (m_draftSchematic.getCraftedSharedObjectTemplate ().IsEmpty ())
		result += "Shared crafted object template is empty\r\n";

	if (m_draftSchematic.getManufactureScript ().IsEmpty ())
		result += "Shared crafted object template is empty\r\n";

	{
		for (int i = 0; i < m_draftSchematic.getNumberOfAttributes (); ++i)
		{
			DraftSchematic::Attribute const * const attribute = m_draftSchematic.getAttribute (i);
			if (atoi (attribute->m_minimumValue) > atoi (attribute->m_maximumValue))
				result += attribute->getName () + " has minimumValue > maximumValue\r\n";

			if (attribute->m_nameStringTable.IsEmpty () || attribute->m_nameStringId.IsEmpty ())
				result += "Found attribute with no name\r\n";
			else
				if (attribute->m_nameStringTable == "crafting" && attribute->m_nameStringId == "attribute")
					result += "Found attribute '" + attribute->getName () + "' with default name\r\n";
				else
				{
					if (isInvalidStringId (attribute->m_nameStringId))
						result += "Found attribute '" + attribute->getName () + "' with invalid string id\r\n";

					if (!attribute->m_experimentStringId.IsEmpty () && isInvalidStringId (attribute->m_experimentStringId))
						result += "Found attribute '" + attribute->getName () + "' with invalid string id\r\n";
				}
		}
	}

	{
		for (int i = 0; i < m_draftSchematic.getNumberOfSlots (); ++i)
		{
			DraftSchematic::Slot const * const slot = m_draftSchematic.getSlot (i);

			if (slot->m_nameStringTable.IsEmpty () || slot->m_nameStringId.IsEmpty ())
				result += "Found slot with no name\r\n";
			else
				if (slot->m_nameStringTable == "crafting" && slot->m_nameStringId == "slot")
					result += "Found slot with default name " + slot->getName () + "\r\n";
				else
					if (isInvalidStringId (slot->m_nameStringId))
						result += "Found slot '" + slot->getName () + "' with invalid string id\r\n";
		}
	}

	result += "Scan complete.\r\n";

	CONSOLE_PRINT (result);
}

// ----------------------------------------------------------------------

void SwgDraftSchematicEditorDoc::OnButtonSavetest() 
{
	m_draftSchematic.save ("c:/test.tpf");
}

// ----------------------------------------------------------------------

void SwgDraftSchematicEditorDoc::updateTpfTabs (char const * const pathName)
{
	CString const serverObjectTemplateName (DraftSchematic::createServerObjectTemplateName (pathName));
	CString const sharedObjectTemplateName (DraftSchematic::createSharedObjectTemplateName (pathName));
	CString const oldServerText = m_draftSchematic.getOldServerObjectTemplate ();
	CString const newServerText = m_draftSchematic.generateServerObjectTemplate (serverObjectTemplateName, false);
	CString const oldSharedText = m_draftSchematic.getOldSharedObjectTemplate ();
	CString const newSharedText = m_draftSchematic.generateSharedObjectTemplate (serverObjectTemplateName, false);
	safe_cast<MainFrame *> (AfxGetMainWnd ())->updateTpfTabs (oldServerText, newServerText, oldSharedText, newSharedText);
}

// ======================================================================

