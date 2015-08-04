// ======================================================================
//
// PropertyFileItem.h
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_PropertyFileItem_H
#define INCLUDED_PropertyFileItem_H

// ----------------------------------------------------------------------

#include "PropertyItem.h"

// ----------------------------------------------------------------------

class QHBox;
class QLineEdit;
class QPushButton;

// ----------------------------------------------------------------------

class PropertyFileItem : public QObject, public PropertyItem
{
	Q_OBJECT

public:
	PropertyFileItem(PropertyList *l, PropertyItem *after, PropertyItem *prop,
		const QString &propName, const QString& defaultDirectory, const QString& extensionFilter);
	~PropertyFileItem();

	virtual void showEditor();
	virtual void hideEditor();

	virtual void setValue(const QVariant &v);

protected:
	virtual bool validate(QString &);

private slots:
	void getFile();
	void setValue();

private:
    QLineEdit *lined();

private:
	QString m_defaultDirectory;
	QString m_extensionFilter;

	QPushButton * m_browseButton;
    QGuardedPtr<QLineEdit> m_lin;
    QGuardedPtr<QHBox> m_hbox;
};

// ======================================================================

#endif // INCLUDED_PropertyFileItem_H
