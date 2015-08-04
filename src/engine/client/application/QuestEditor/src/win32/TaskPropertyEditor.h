// ======================================================================
//
// TaskPropertyEditor.h
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_TaskPropertyEditor_H
#define INCLUDED_TaskPropertyEditor_H

// ----------------------------------------------------------------------

#include "ElementPropertyEditor.h"
#include "QuestEditorConfig.h"

// ----------------------------------------------------------------------

#include <qdom.h>

// ----------------------------------------------------------------------

class TaskPropertyEditor : public ElementPropertyEditor
{
	Q_OBJECT //lint !e830 !e1511 !e1516 !e1924

public:
	explicit TaskPropertyEditor(QWidget * taskPropertyEditorParent);
	~TaskPropertyEditor();

	void setTask(QDomNode task);

	void clearTask();

	virtual void setupProperties();
	void setSelectedField(QString const & field);

public slots:

private slots:

signals:

protected:

private:
	
private: //-- disabled
	TaskPropertyEditor(TaskPropertyEditor const &);
	TaskPropertyEditor &operator=(TaskPropertyEditor const &);
	TaskPropertyEditor();
};

// ======================================================================

#endif // INCLUDED_TaskPropertyEditor_H
