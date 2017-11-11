// ======================================================================
//
// QuestInputStringValidtor.h
// Copyright 2006, Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_QuestInputStringValidator_H
#define INCLUDED_QuestInputStringValidator_H

// ======================================================================

#include <qvalidator.h>

// ----------------------------------------------------------------------

class QuestInputStringValidator : public QValidator
{
	Q_OBJECT

public:
	QuestInputStringValidator(QObject * parent, char const * name = 0, bool allowSpaces = true);
	~QuestInputStringValidator();

	virtual QValidator::State validate(QString & input, int & pos) const;
	virtual void fixup(QString & input) const;

	bool setAllowSpaces(bool allowSpaces);

protected:
	virtual bool isValidCharacter(QChar c) const;

private:
	bool m_allowSpaces;
};

// ----------------------------------------------------------------------

inline bool QuestInputStringValidator::setAllowSpaces(bool allowSpaces)
{
	m_allowSpaces = allowSpaces;
}

// ======================================================================

#endif // INCLUDED_QuestInputStringValidator_H
