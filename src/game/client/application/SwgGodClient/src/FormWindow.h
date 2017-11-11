// ======================================================================
//
// FormWindow.h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_FormWindow_H
#define INCLUDED_FormWindow_H

// ======================================================================

#include "BaseFormWindow.h"

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/NetworkId.h"
#include "sharedGame/FormManager.h"

// ======================================================================

class Object;
class QLabel;
class QLineEdit;
class QWidget;
class Vector;

// ======================================================================

class FormWindow : public BaseFormWindow
{
	Q_OBJECT; //lint !e1516 !e19 !e1924 !e1762 various deficiencies in the Qt macro

public:
	explicit FormWindow(QWidget * theParent = 0, const char * theName = 0);
	virtual ~FormWindow();

	void populatePage();
	void populatePage(FormManager::UnpackedFormData const & unpackedData);

	void setDisplayReason(FormManager::Command const & reason);
	void setCurrentForm(FormManager::Form const & form);
	void setServerObjectTemplate(std::string const & templateName);
	void setCreatePosition(Vector const & pos, NetworkId const & cell);
	void setObject(Object const * const object);

public slots:
	virtual void onOkPressed();
	virtual void onCancelPressed();

private:
	//disabled
	FormWindow (FormWindow const & rhs);
	FormWindow & operator= (FormWindow const & rhs);

private:
	void buildUI(FormManager::Form const & form);
	void buildUI(FormManager::Form const & form, FormManager::UnpackedFormData const & unpackedData);
	int addField(FormManager::Field const & field, int currentY, std::string const & value);
	bool gatherAndSendData();
	std::string getFieldValue(QWidget const * const widget) const;
	bool validateData(stdmap<std::string, std::string>::fwd const & pendingData);
	bool validateField(FormManager::Field const & field, std::string const & value, stdmap<std::string, std::string>::fwd const & pendingData, std::string & errorString /*OUT*/) const;
	bool validateFieldAsInt(FormManager::Field const & field, std::string const & value, stdmap<std::string, std::string>::fwd const & pendingData, std::string & errorString /*OUT*/) const;
	bool validateFieldAsFloat(FormManager::Field const & field, std::string const & value, stdmap<std::string, std::string>::fwd const & pendingData, std::string & errorString /*OUT*/) const;
	bool validateFieldAsString(FormManager::Field const & field, std::string const & value, stdmap<std::string, std::string>::fwd const & pendingData, std::string & errorString /*OUT*/) const;

private:
	FormManager::Command m_displayReason;
	FormManager::Form const * m_currentForm;
	stdmap<FormManager::Field const *, QWidget *>::fwd * m_fieldMap;
	stdvector<FormManager::Field const *>::fwd * m_orderedFieldList;
	std::string m_serverTemplateName;
	Vector * m_cursorPositionInWorld;
	NetworkId m_cellIdPositionInWorld; 
	Object const * m_object;
};

// ======================================================================

#endif
