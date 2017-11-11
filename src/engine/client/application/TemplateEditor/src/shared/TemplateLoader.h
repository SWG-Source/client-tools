// ============================================================================
//
// TemplateLoader.h
//
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_TemplateLoader_H
#define INCLUDED_TemplateLoader_H

class File;
class TemplateDefinitionFile;
class TemplateData;
class TpfFile;
class TpfTemplate;

//------------------------------------------------------------------------------
class TemplateLoader
{
public:
	
	static void           install();
	static void           remove();

	//static bool           loadTdf(QString const &path);
	static bool           loadTpf(QString const &path);

	static TemplateData & getTemplateData();
	static TpfTemplate &  getTpfTemplate();
	static QString const &getTpfPath();
	static TpfFile *      getTpfFileForParameter(QString const &parameterName);

private:

	static void loadInheritedTpfFiles(QString const &path);
	static void resetTpfFileVector();

	typedef std::vector<TpfFile *> TpfFileVector;

	static TpfFileVector m_tpfFileVector;
	static TemplateDefinitionFile *     m_TemplateDefinitionFile;
	static TpfFile *     m_tpfFile;
	static QString       m_tpfPath;

private:

	// Disabled

	TemplateLoader();
	TemplateLoader(const TemplateLoader &);
	TemplateLoader &operator=(const TemplateLoader &);
};

// ============================================================================

#endif // INCLUDED_TemplateLoader_H
