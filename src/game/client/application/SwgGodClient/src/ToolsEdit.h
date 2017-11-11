// ======================================================================
//
// ToolsEdit.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ToolsEdit_H
#define INCLUDED_ToolsEdit_H

// ======================================================================
class ToolsEdit : public QToolBar
{
	Q_OBJECT; //lint !e1516 !e19 !e1924 !e1762 various deficiencies in the Qt macro

public:
	QAction *             copy;
	QAction *             paste;
	QAction *             del;
	QAction *             cut;

private slots:
	void                  onDelete ();
	void                  onCopy   ();
	void                  onPaste  ();
	void                  onCut    ();

private:
	//disabled
	ToolsEdit(const ToolsEdit& rhs);
	ToolsEdit& operator= (const ToolsEdit& rhs);
};

// ======================================================================

#endif
