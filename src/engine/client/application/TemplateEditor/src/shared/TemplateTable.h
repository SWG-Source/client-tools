// ============================================================================
//
//	TemplateTable.h
//
//	A wrapper around a QT Table that knows about Templates.
//
//	Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_TemplateTable_H
#define INCLUDED_TemplateTable_H

#include <map>

class TableItemBase;
class TableItemColor;
class TemplateData;
class TemplateEditorManager;
class TemplateTableRow;
class TpfTemplate;
class QFile;

//-----------------------------------------------------------------------------
class TemplateTable : public QTable
{
Q_OBJECT

public:

	TemplateTable(QWidget *parent = 0, const char *name = 0);
	virtual ~TemplateTable();

	// Template Functions
	///////////

	void                   load(TemplateData const &templateData, TpfTemplate *tpfTemplate, bool const quickLoad = false, bool const addBaseParameter = false);
	void                   write(QFile &file, bool const firstEntry = false);

	//void                   setParentParameterName(QString const &parameterName);
	//void                   setParentTemplateTable(TemplateTable const &templateTable);
	//void                   setParentTpfTemplate(TpfTemplate const &tpfTemplate);
	void                   setParentTemplateTableRow(TemplateTableRow const *templateTableRow);

	//TemplateTable const *   getParentTemplateTable() const;
	//QString const &         getParentParameterName() const;
	//TpfTemplate const *     getParentTpfTemplate() const;
	TemplateTableRow const *getParentTemplateTableRow() const;

	// Various setters and accessors
	//////////

	//void                   setNumTemplateTableRows(int numRows);
	static void            setTemplateEditorManager(TemplateEditorManager &manager);
	TemplateData const &   getTemplateData() const;

	TemplateTableRow *     getTemplateTableRow(int const row) const;
	//QTableItem *           item(int row, int col) const;

	void                   insertTemplateTableRow(int const row, TemplateTableRow *templateTableRow, QCheckTableItem *m_definedCheckTableItem, TableItemColor *m_legendColorTableItem, TableItemColor *m_nameColorTableItem, TableItemBase *m_valueBaseTableItem);

	void                   clearTemplateRow(int const row);
	//TemplateTableRow *     takeTemplateRow(int row);

	//QWidget *              cellWidget(int row, int col) const;
	//void                   insertWidget(int row, int col, QWidget *widget);
	//void                   clearCellWidget(int row, int col);

	void                   clearAll();
	void                   setCheckedAll(bool const checked);
	//void                   resizeData(int);   // Disabled so QTable can't manage me behind my back

	void                   setTemplateData(TemplateData const &templateData);
	static int             getGlobalCount();

	enum ParameterType
	{
		   PT_none
		,  PT_list
	};

	void                   setParameterType(ParameterType const parameterType);
	bool                   isParameterType(ParameterType const parameterType) const;
	bool                   getNeedsComma(TemplateTableRow const &templateTableRow) const;
	bool                   isLastElement(TemplateTableRow const &templateTableRow) const;

public slots:

	virtual void           setCurrentCell(int row, int col);
	void                   toggleEdit(int row);
	void                   endEditing();

protected:

	virtual void           mouseReleaseEvent(QMouseEvent *mouseEvent);

private slots:

	void                   execParameterPropertyPopupMenu(int row, int column, const QPoint &pos);

private:

	typedef std::map<int, TemplateTableRow *> IntToTemplateTableRowMap;
	//typedef std::map<int, QWidget *> IntToWidgetMap;

	void                   setNumRows(int numRows);   // Call setNumTemplateTableRows instead

	virtual QWidget *      beginEdit(int row, int column, bool replace);
	virtual void           endEdit(int row, int column, bool accept, bool replace);
	void                   writeSectionHeader(QTextStream &textStream, QString const &tdf, QString const &version);
	void                   setOptimizedRowInsertEnabled(bool const enabled);

	TpfTemplate *          getTpfTemplateForParameter(QString const &name);

	IntToTemplateTableRowMap m_intToTemplateTableRowMap;
	//IntToWidgetMap      m_intToWidgetMap;
	int                      m_previousRow;
	int                      m_previousColumn;
	TemplateData const *     m_templateData;
	ParameterType            m_parameterType;
	bool                     m_needsComma;
	TemplateTableRow const * m_parentTemplateTableRow;
	bool                     m_optimizatedRowInsert;
	int                      m_currentOptimizedRow;


	static int                    m_globalCount;
	static TemplateEditorManager *ms_templateEditorManager;
};

// ============================================================================

#endif // INCLUDED_TemplateTable_H