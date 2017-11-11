// ======================================================================
//
// Quest.h
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_Quest_H
#define INCLUDED_Quest_H

// ----------------------------------------------------------------------

#include <qdom.h>

// ----------------------------------------------------------------------

class Quest : public QDomDocument
{
public:
	Quest();
	~Quest();

	bool saveQuest(char const * const filename) const;
	bool loadQuest(char const * const filename = 0);

	int getUniqueTaskId() const;

	QDomNode getTasks() const;
	QDomNode getList() const;

protected:

private:
};

// ======================================================================

#endif // INCLUDED_Quest_H
