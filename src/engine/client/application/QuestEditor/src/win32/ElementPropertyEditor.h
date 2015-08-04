// ======================================================================
//
// ElementPropertyEditor.h
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ElementPropertyEditor_H
#define INCLUDED_ElementPropertyEditor_H

// ----------------------------------------------------------------------

#include "QuestEditorConfig.h"
#include "PropertyList/PropertyList.h"

// ----------------------------------------------------------------------

#include <qdom.h>

// ----------------------------------------------------------------------

class ElementPropertyEditor : public PropertyList
{
	Q_OBJECT //lint !e830 !e1511 !e1516 !e1924

public:
	explicit ElementPropertyEditor(QWidget * elementPropertyEditorParent);
	~ElementPropertyEditor();

	void setElement(QDomElement element);
	bool addElementProperty(PropertyItem *& item, PropertyItem * parentItem, char const * const displayName, char const * const valueName, QDomElement configInputElement);

	void clearElement();

	virtual void valueChanged(PropertyItem *i);
	virtual void setupProperties() = 0;
	virtual QVariant getResetValue(PropertyItem *item);
	void setupInput(PropertyItem *& item, QDomNode configDataNode, PropertyItem * parentItem = 0);

	QDomElement getElement() const;
	void openParentCompoundInput(QDomElement configDataElement);

public slots:

private slots:

signals:
	void dirty();

protected:

private:
	QDomElement m_element;
	
private: //-- disabled
	ElementPropertyEditor();
	ElementPropertyEditor(ElementPropertyEditor const &);
	ElementPropertyEditor &operator=(ElementPropertyEditor const &);
};

// ----------------------------------------------------------------------

inline QDomElement ElementPropertyEditor::getElement() const
{
	return m_element;
}

// ======================================================================

#endif // INCLUDED_ElementPropertyEditor_H
