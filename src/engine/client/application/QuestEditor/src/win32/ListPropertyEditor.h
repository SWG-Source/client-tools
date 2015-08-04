// ======================================================================
//
// ListPropertyEditor.h
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ListPropertyEditor_H
#define INCLUDED_ListPropertyEditor_H

// ----------------------------------------------------------------------

#include "ElementPropertyEditor.h"
#include "QuestEditorConfig.h"

// ----------------------------------------------------------------------

#include <qdom.h>

// ----------------------------------------------------------------------

class ListPropertyEditor : public ElementPropertyEditor
{
	Q_OBJECT //lint !e830 !e1511 !e1516 !e1924

public:
	explicit ListPropertyEditor(QWidget * listPropertyEditorParent);
	~ListPropertyEditor();

	void setList(QDomNode list);

	void clearList();

	virtual void setupProperties();
	void setSelectedField(QString const & field);

public slots:

private slots:

signals:

protected:

private:
	
private: //-- disabled
	ListPropertyEditor(ListPropertyEditor const &);
	ListPropertyEditor &operator=(ListPropertyEditor const &);
	ListPropertyEditor();
};

// ======================================================================

#endif // INCLUDED_ListPropertyEditor_H
