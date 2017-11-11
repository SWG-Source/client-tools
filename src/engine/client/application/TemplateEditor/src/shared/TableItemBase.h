// ============================================================================
//
//	TableItemBase.h
//
//	This class extends the functionality of a QTableItem. It is a base class
//	that sets up the ability to have a customizable edit cell. That is, integer
//	table cells have different editing properties than boolean, enum, or file
//	cells.
//
//	See also QT's QTableItem
//
//	Used by TemplateTableRow
//
//	Inherited by Edit*TableItem, DHERMAN expand this list
//
//	Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_TableItemBase_H
#define INCLUDED_TableItemBase_H

#include "TemplateTableRow.h"

class BaseDialogEdit;
class BoolParam;
class DataTypeEdit;
class DynamicVariableParamData;
class FloatParam;
class IntegerParam;
class MustDeriveEdit;
class StringIdParam;
class StringParam;
class TemplateTable;
class TemplateTableRow;
class TpfTemplate;
class TriggerVolumeParam;
class VectorParam;
class WeightedListDialogEdit;

template <class ST> class StructParam;
class ObjectTemplate;
typedef StructParam<ObjectTemplate> StructParamOT;

// DHERMAN When done testing, make this class abstract
//-----------------------------------------------------------------------------
class TableItemBase : public QTableItem
{
public:

	TableItemBase(TemplateTableRow &tableRow);
	//virtual ~TableItemBase();

	void              update();

	virtual int       alignment() const;    // All TableItemBases will be left justified

	virtual QWidget * createEditor() const;
	virtual void      setContentFromEditor(QWidget *widget);

	// Public accessors
	////////////////

	TemplateTableRow &getTemplateTableRow() const;
	TemplateTable &   getParentTable() const;
	virtual QString   getTypeString() const = 0;
	virtual int       getSupportedParameterProperties() const = 0;

	int               getWeight() const; // If this is a weighted item, what is its weighted chance?

	// Optional initializers, provided for interface completion. Children should overload if they want 'em

	virtual void      initDefault();
	virtual void      init(TpfTemplate *tpfTemplate, QString const &parameterName, int const parameterIndex, int const weightedListIndex, TemplateTableRow::ParameterProperty const parameterProperty) = 0;
	virtual void      init(IntegerParam const *integerParam, int const parameterIndex, int const weightedListIndex, TemplateTableRow::ParameterProperty const parameterProperty);
	virtual void      init(FloatParam const *floatParam, int const parameterIndex, int const weightedListIndex, TemplateTableRow::ParameterProperty const parameterProperty);
	//virtual void                    init(const BoolParam *parameterData) { UNREF(parameterData); }
	virtual void      init(StringParam const *stringParam, int const parameterIndex, int const weightedListIndex);
	//virtual void                    init(const VectorParam *parameterData) { UNREF(parameterData); }
	//virtual void      init(StringIdParam const *stringIdParam);
	//virtual void                    init(const TriggerVolumeParam *parameterData) { UNREF(parameterData); }
	//virtual void      init(std::vector<DynamicVariableParamData *> *dynamicVariableParamDataVector);
	virtual void      init(DynamicVariableParamData const *dynamicVariableParamData);
	//virtual void                    init(const StructParamOT *parameterData) { UNREF(parameterData); }

protected:

	DataTypeEdit *m_dataTypeEdit;

	void                    initializeEditor();

	int                     getParameterIndex() const;
	QString const &         getParameterName() const;
	TpfTemplate *           getTemplate() const;
	WeightedListDialogEdit *getWeightedListDialogEdit() const;

	void initAsArray();
	void initAsDerived();
	void initAsList();

private:

	TemplateTableRow &m_templateTableRow;

private:

	// Disabled

	TableItemBase();
	TableItemBase(TableItemBase const &);
	TableItemBase &operator=(TableItemBase const &);
};

// ============================================================================

#endif // INCLUDED_TableItemBase_H
