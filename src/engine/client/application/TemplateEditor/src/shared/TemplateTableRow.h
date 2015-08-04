// ============================================================================
//
//	TemplateRow.h
//
//	A row out of the template table, which contains two cells - a parameter
//	name (first column) and its value (second column).
//
//	This row is responsible for knowing a lot about template variables. It
//	needs to know which template file a value comes from (values are color
//	coded by which file they are defined in) as well as what type it is
//	(different variable types need to specify different modes of editing).
//
//	This class, ultimately, manages a TableItemColor (for this row's parameter
//	name) and a BaseEditTableItem (for this row's parameter value). It is
//	helped with this task by TemplateEditorManager.
//
//	See TableItemColor, TemplateData::Parameter
//
//	Used by TemplateEditorManager
//
//	Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_TemplateTableRow_H
#define INCLUDED_TemplateTableRow_H

#include "sharedTemplateDefinition/TemplateData.h"
#include "sharedTemplateDefinition/TpfFile.h"

class BoolParam;
class TableItemColor;
class DynamicVariableParam;
class TableItemBase;
class FloatParam;
class IntegerParam;
class StringIdParam;
class StringParam;
//class TemplateParameter;
class TemplateTable;
class TemplateTableRow;
class TpfFile;
class TpfTemplate;
class TriggerVolumeParam;
class VectorParam;

template <class ST> class StructParam;
class ObjectTemplate;
typedef StructParam<ObjectTemplate> StructParamOT;

//-----------------------------------------------------------------------------
class TemplateTableRow : public QWidget
{
Q_OBJECT

public:

	enum ColumnTyped
	{
		  CT_edited
		, CT_legend
		, CT_name
		, CT_value
		, CT_count
	};

	// Paramter Type enumeration
	//
	// Each table row can contain a varied amount of different data - not only
	// are many different data types available, but more complicated structures
	// (like lists and ranges, which are organizations around the simple data
	//  types) are also at a user's disposal.
	//
	// Note that:
	// * A templateTableRow's data type is not defined below. Rather, that can
	//   be found in TemplateData::ParamType (contained in the Parameter struct)
	// * Some data attributes are mutually exclusive (you can be a Range AND
	//   a Delta, but you can't be a ListItem AND a WeightedItem). This detail
	//   is taken care of in addParamterType(...).
	// * Some data attributes are data-type dependents (you can have an Integer
	//   range but not a String range). This detail is taken care of in
	//   TableItemBase::getSupportedValueModes().
	
	enum ParameterProperty
	{ 
		  PP_undefined    = 1 << 1   // The basic parameter type
		, PP_simple       = 1 << 2   // The basic parameter type
		, PP_float        = 1 << 3   // Row contains an item of straight data (ex: 1). See TemplateData::ParamType field in Parameter struct for more information at this point.
		, PP_int          = 1 << 4   // Row contains an item of straight data (ex: 1). See TemplateData::ParamType field in Parameter struct for more information at this point.
		, PP_bool         = 1 << 5   // Row contains an item of straight data (ex: 1). See TemplateData::ParamType field in Parameter struct for more information at this point.
		, PP_array        = 1 << 6   // Head of a fixed-length array (cell contains an open/close array button)
		, PP_arrayItem    = 1 << 7   // Item in the list
		, PP_list         = 1 << 8   // Head of a list (cell contains an open/close list button)
		, PP_listItem     = 1 << 9   // Item in the list
		, PP_weightedList = 1 << 10   // Head of a weighted list (cell contains an open/close list button)
		, PP_weightedItem = 1 << 11  // Row contains a weighted item (ex: "20 percent":20)
		, PP_randomRange  = 1 << 12  // Row contains a range (ex: 1..10)
		, PP_dieRoll      = 1 << 13  // Row contains a dice roll (ex: 1d6 + 20)
		, PP_positive     = 1 << 14
		, PP_negative     = 1 << 15
		, PP_percent      = 1 << 16
		, PP_derived      = 1 << 17  // This row is passing the responsibility of assignment to a child template
		, PP_useParent    = 1 << 18  // This row is just using its parents value
		, PP_count        = 17
		, PP_atBase       = 1 << 19

		// RowTypeFlag groups

		//, PP_array        = (PP_listArray | PP_arrayItem)
		, PP_valueList    = (PP_array | PP_arrayItem)
		//, PP_weightedList = (PP_weightedHead | PP_weightedItem)
		, PP_delta        = (PP_positive | PP_negative | PP_percent)
		, PP_invalid      = -1
	};

	enum SpecialRowType
	{
		BaseTemplateSpecifier,   // Contains special template @base field, which marks which Template File to inherit from
		ObjvarTypeSelector       // Contains a pull-down menu that specifies what type this objvar is
	};

public:

	TemplateTableRow(TemplateTable &parentTable, int const row, TpfTemplate &tpfTemplate, TemplateData::Parameter const &parameter);
	TemplateTableRow(TemplateTable &parentTable, int const row, TemplateData::ParamType const paramType, const char *name, const char *description = "");
	TemplateTableRow(TemplateTable &parentTable, int const row, TemplateTableRow const &parentRow, TemplateData::Parameter const &parameter, int const parameterIndex, int const weightedListIndex = 0);
	TemplateTableRow(TemplateTable &parentTable, int const row, SpecialRowType specialRowType);
	virtual ~TemplateTableRow();

	// Public accessors
	///////////////

	QString const &                getComment() const;
	QCheckTableItem *              getColumnEdited() const;
	QTableItem *                   getColumnLegend() const;
	QTableItem *                   getColumnName() const;
	TableItemBase *                getColumnValue() const;
	QString                        getDecoratedName() const;
	static int                     getGlobalCount();
	ParameterProperty              getListType() const;
	TemplateData::Parameter const *getParameter() const;
	int                            getParameterIndex() const;
	QString                        getParameterName() const;
	//int                            getParameterPropertys() const;
	TemplateTable &                getParentTable() const;
	int                            getRow() const;
	TpfTemplate *                  getTpfTemplate() const;
	QString                        getType() const;
	TemplateData::ParamType        getTemplateDataParamType() const;

	bool                           isChecked() const;
	bool                           isCheckEnabled() const;
	bool                           isSingle() const;
	bool                           isParameterProperty(ParameterProperty const parameterProperty) const;
	bool                           isParameterDerived() const;

	void                           execParameterPropertyPopupMenu(QPoint const &popupPosition);
	void                           setChildTable(TemplateTable &childTable);
	void                           write(QFile &file);
	void                           insertTableItems(int const row, TemplateTable *templateTable);

	void                           setChecked(bool const checked);
	void                           setCheckEnabled(bool const enabled);
	void                           setDecoratedName(QString const &name);
	void                           setName(QString const &name);

	void                           addParameterProperty(ParameterProperty const parameterProperty);

	bool                           isDeltaPositive() const;
	bool                           isDeltaNegative() const;
	bool                           isDeltaPercent() const;
	void                           setDeltaPositive(bool const enabled);
	void                           setDeltaNegative(bool const enabled);
	void                           setDeltaPercent(bool const enabled);

public slots:

	void                   reload();  // This row changed its type/contents
	void                   refresh(); // See if this cell's contents have changed since being loaded

private slots:

	void                   goToFile();

private:

	// Initiailizes the data in this row

	void                   load(QString const &parameterName, int const parameterIndex, int const weightedListIndex = 0, bool const changeCheckedState = true);

	// Row type flag functionality and helper functions
	////////////

	void                   clearParameterProperty(ParameterProperty const parameterProperty);
	void                   checkParameterProperties();
	bool                   isSupported(ParameterProperty const parameterProperty) const;

	void                   clearParameterBasic();
	void                   clearParameterListHead();
	void                   clearParameterListItem();
	void                   clearParameterDeltaType();
	void                   clearParameterDeltaPercent();
	void                   clearParameterAll();

	void                   setParameterPropertysFromParameter(const TemplateData::Parameter &parameter);

	// Various internal helper functions

	//TpfTemplate *          getParentParameter(TpfFile &tpfFile, TpfTemplate &tpfTemplate, QString const &path) const;

	void                   addPropertyToPopupMenu(QPopupMenu &popupMenu, ParameterProperty const parameterProperty, QString const &typeName);

	void                   setParameterPropertys(IntegerParam *param);
	void                   setParameterPropertys(FloatParam *param);
	void                   setParameterPropertys(BoolParam *param);
	void                   setParameterPropertys(StringParam *param);
	void                   setParameterPropertys(VectorParam *param);
	void                   setParameterPropertys(StringIdParam *param);
	void                   setParameterPropertys(TriggerVolumeParam *param);
	void                   setParameterPropertys(DynamicVariableParam *param);
	void                   setParameterPropertys(StructParamOT *param);

	// TableItemBase factory functions

	TableItemBase *        createTableItemFromParamType(TemplateData::ParamType paramType);
	TableItemBase *        createTableItemFromSpecialType(SpecialRowType const specialRowType);

	TemplateTable &                m_parentTemplateTable;
	TemplateTable *                m_childTable;         // A sub-template table, used for lists, arrays, and structs
	QString                        m_parameterName;
	QString                        m_description;
	TemplateData::Parameter const *m_parameter;
	TemplateData::ParamType        m_templateDataParamType;
	ParameterProperty              m_parameterProperty;      // Is this a single value, a dynamic list, or an array? This is the only unchangeable row type flag
	int                            m_parameterProperties;
	int                            m_parameterIndex;
	bool                           m_single;
	TpfTemplate *                  m_tpfTemplate;

	QCheckTableItem *              m_definedCheckTableItem;
	TableItemColor *               m_legendTableItemColor;
	TableItemColor *               m_nameTableItemColor;
	TableItemBase *                m_valueTableItemBase;

	static int m_globalCount; // Number of alive objects

private:

	// Disabled

	TemplateTableRow();
	TemplateTableRow(TemplateTableRow const &);
	TemplateTableRow &operator=(TemplateTableRow const &);
};

// ============================================================================

#endif // INCLUDED_TemplateTableRow_H
