// ============================================================================
//
// PlanetWatcherUtility.h
// copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_PlanetWatcherUtility_H
#define INCLUDED_PlanetWatcherUtility_H

#include "PlanetWatcherRenderer.h"

#include <string>

//-----------------------------------------------------------------------------
class PlanetWatcherUtility
{
public:
	static bool install();

	static void    writeEntry(QString const &location, QString const &value);
	static QString readEntry(QString const &location, QString const &defaultValue);
	static void    setOutputWindow(QTextEdit &outputWindow);
	static void    report(QString const &text, bool const showTime = true);
	static void    clearOutputTextWindow();
	static void    saveWidget(QWidget const &widget);
	static void    loadWidget(QWidget &widget, int const defaultX, int const defaultY, int const defaultWidth = 0, int const defaultHeight = 0);
	static const char * getObjectTypeName(int objectTypeTag);
	static bool    getObjectTypeTagFromName(const char * objectTypeName, int & objectTypeTag);
	static int     getRandomObjectType();
	static stdvector<std::string>::fwd getAllValidTagTypes();
	static char const * getServerObjectTemplateName(uint32 const serverObjectTemplateCrc);
	static char const * getServerObjectTemplateCrcStringTableName();
	static bool    serverTemplateCrcLookupsAreEnabled();
	static std::string dumpObjectInfo(PlanetWatcherRenderer::MiniMapObject const &object); 

private:
	static QTextEdit *m_outputWindow;
	static QString getSearchPath();
	static void remove();

private:

	// Disabled

	PlanetWatcherUtility();
	PlanetWatcherUtility(PlanetWatcherUtility const &);
	~PlanetWatcherUtility();
	PlanetWatcherUtility &operator =(PlanetWatcherUtility const &);
};

// ============================================================================

#endif // PlanetWatcherUtility_H
