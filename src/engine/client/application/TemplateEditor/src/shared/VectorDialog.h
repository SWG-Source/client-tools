// ============================================================================
//
// VectorDialog.h
//
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_VectorDialog_H
#define INCLUDED_VectorDialog_H

#include "StaticListDialog.h"

class TemplateTableRow;
struct VectorParamData;

//-----------------------------------------------------------------------------
class VectorDialog : public StaticListDialog
{
public:

	VectorDialog(QWidget *parent, const char *name, TemplateTableRow &parentRow);

	void    setValue(const VectorParamData &vectorParamData);

	QString getCoordXText() const;
	QString getCoordYText() const;
	QString getCoordZText() const;
	QString getRadiusText() const;

	bool    ignoreIndexY() const;

private:

	int m_indexX;
	int m_indexY;
	int m_indexZ;
	int m_indexRadius;
	int m_ignoreIndexY;

private:

	// Disabled

	VectorDialog();
	VectorDialog(VectorDialog const &);
	VectorDialog &operator=(VectorDialog const &);
};

// ============================================================================

#endif // INCLUDED_VectorDialog_H
