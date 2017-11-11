//======================================================================
//
// TemplateNewDialog.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_TemplateNewDialog_H
#define INCLUDED_TemplateNewDialog_H

//======================================================================

#include "BaseTemplateNewDialog.h"


//-----------------------------------------------------------------------------

class TemplateNewDialog : public BaseTemplateNewDialog
{
	Q_OBJECT //lint !e1924 !e1511 !e1516

public:

	TemplateNewDialog(QWidget *parent, char const *name, std::string const & selectChassisName, std::string const & selectTemplateName);
	TemplateNewDialog(QWidget *parent, char const *name, std::string const & selectTemplateName);
	~TemplateNewDialog();

public slots:

	void onLineEditSharedTemplateTextChanged(const QString &);
	void onLineEditSharedBaseTemplateTextChanged(const QString &);
	void onLineEditBaseTemplateTextChanged(const QString &);
	void onLineEditNewTemplateTextChanged(const QString &);

	void onPushButtonBrowseNewTemplateClicked();
	void onPushButtonBrowseBaseTemplateClicked();
	void onPushButtonBrowseSharedTemplateClicked();
	void onPushButtonBrowseSharedBaseTemplateClicked();

	void onPushButtonBrowseFileNewTemplateClicked();
	void onPushButtonBrowseFileBaseTemplateClicked();
	void onPushButtonBrowseFileSharedTemplateClicked();
	void onPushButtonBrowseFileSharedBaseTemplateClicked();

	void onPushButtonCreateTemplateClicked();

	void handleFileBrowsing(QLineEdit & lineEdit, bool fileOrFolder, bool isValid, bool isServer);

	std::string const & showAndTell();

signals:

protected:

private:

	TemplateNewDialog();

private:

	void updateTextFieldValidity();
	void populateChassisCombo();
	void selectChassis(std::string const & selectChassisName);
	void setupSignals();

private:
	bool m_newTemplateValid;
	bool m_baseTemplateValid;
	bool m_sharedTemplateValid;
	bool m_sharedBaseTemplateValid;
	bool m_creatingNewTemplate;
	bool m_creatingSharedTemplate;

	enum Mode
	{
		M_chassis,
		M_component
	};

	Mode m_mode;

	std::string m_resultTemplateName;
};

//======================================================================

#endif
