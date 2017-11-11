// ============================================================================
//
//	TemplateEditorManager.h
//
//	TemplateEditorManager is a class that sits atop of the Template Editor and
//	manipulates the various components of its UI. Unlike the TemplateEditor,
//	the manager is completely aware of the Template Loader system all Template
//	components. I could have made the Template Editor aware of this as well, but
//	I thought it would be easier to read if the Template Editor just dealt with
//	initializing and organizing its UI pieces, while a separate class dealt with
//	the Template data management.
//
//	Some of this class' responsibilities include:
//	
//	* Taking care of the (Color Scheme --> Template File) mapping
//
//	* Loading/unloading TdfFiles and TpfFiles (DHERMAN TpfFile will soon be eliminated)
//
//	* Taking raw TdfFile and TpfFile (DHERMAN TpfFile will soon be eliminated)
//	  structures apart and inserting their data to the appropriate editor's
//	  Tables and Lists.
//	
//	See BaseTemplateEditor, TemplateEditor, TemplateTableRow,
//	TdfFile, TemplateData, TpfFile (this needs to become TemplateDatabaseRow
//	or something like that)
//
//	Used by TemplateEditor
//
//	Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef TEMPLATEEDITORMANAGER_H
#define TEMPLATEEDITORMANAGER_H

#include "sharedTemplateDefinition/templateData.h"	// Include necessary for access to nested struct Parameter

class TemplateEditor;
class TemplateTable;
class TemplateTableRow;

//-----------------------------------------------------------------------------
class TemplateEditorManager : public QObject
{
	Q_OBJECT

public:

	TemplateEditorManager(TemplateEditor &templateEditor);
	~TemplateEditorManager();

	void           describeRow(TemplateTableRow const *templateTableRow) const;
	TemplateTable *getTemplateTable() const;
	//void           initializeWithTDF(QString const &path);
	void           initializeWithTemplate(QString const &path);
	void           initializeTemplateTable(bool const quickLoad = false);
	void           autoSave();
	
private:

	TemplateEditor *m_templateEditor;

private:

	// Disabled

	TemplateEditorManager();
	TemplateEditorManager(TemplateEditorManager const &);
	TemplateEditorManager &operator=(TemplateEditorManager const &);
};

// ============================================================================

#endif // TEMPLATEEDITORMANAGER_H
