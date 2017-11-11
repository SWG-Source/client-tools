// ======================================================================
//
// QuestInputStringValidator.cpp
// Copyright 2006, Sony Online Entertainment
//
// ======================================================================

#include "FirstQuestEditor.h"

// ----------------------------------------------------------------------

#include "QuestInputStringValidator.h"

// ----------------------------------------------------------------------

#include "QuestInputStringValidator.moc"

// ----------------------------------------------------------------------

QuestInputStringValidator::QuestInputStringValidator(QObject * parent, const char * name, bool allowSpaces)
: QValidator(parent, name)
, m_allowSpaces(allowSpaces)
{
}

// ----------------------------------------------------------------------

QuestInputStringValidator::~QuestInputStringValidator()
{
}

// ----------------------------------------------------------------------

QValidator::State QuestInputStringValidator::validate(QString & input, int & pos) const
{
	fixup(input);

	int length = static_cast<int>(input.length());
	
	// adjust the cursor position if the fix removed characters
	if (pos > length)
		pos = length;

	return QValidator::Acceptable;
}

// ----------------------------------------------------------------------

void QuestInputStringValidator::fixup(QString & input) const
{
	QString fixed = "";

	for (int i = 0; i < static_cast<int>(input.length()); ++i)
	{
		if (isValidCharacter(input[i]))
			fixed += input[i];
		else
			QApplication::beep();
	}

	input = fixed;
}

// ----------------------------------------------------------------------

bool QuestInputStringValidator::isValidCharacter(QChar c) const
{
	// make sure the character is in the alphanumeric range
	if (c < ' ' || c > '~')
		return false;

	if (!m_allowSpaces && c == ' ')
		return false;

	return true;
}

// ======================================================================
