// ============================================================================
//
// TemplateEditorUtility.h
//
// A useful collection of functions for Template Editor UI components.
//
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_TemplateEditorUtility_H
#define INCLUDED_TemplateEditorUtility_H

class QWidget;

class QTextEdit;
class QWidget;

// Reference SoundEditorUtility for more useful functions to copy

//-----------------------------------------------------------------------------
class TemplateEditorUtility
{
public:

	static void        setOutputWindow(QTextEdit &outputWindow);
	static void        report(QString const &text);
	static void        saveWidget(QWidget const &widget);
	static void        loadWidget(QWidget &widget, int const defaultX, int const defaultY, int const defaultWidth = 0, int const defaultHeight = 0);
	static int         validateLineEditInt(QLineEdit *lineEdit, int const min, int const max);
	static int         setLineEditInt(QLineEdit *lineEdit, int const value, int const min, int const max);
	static int         getInt(QLineEdit *lineEdit);
	static const char *getSearchPath();
	static const char *getLogPath();
	static const char *getSuccessListPath();

private:

	static QTextEdit *m_outputWindow;
	static char ms_logPath[];
	static char ms_successListPath[];

private:

	// Disabled

	TemplateEditorUtility();
	~TemplateEditorUtility();
};

// This needs to be moved because I use it in Audio, the Sound Editor, and the Particle Editor, but I need it here now
//-----------------------------------------------------------------------------

///@deprecated, use sharedFile/FileNameUtils instead
class FilePath
{
public:

	enum
	{
		drive =     1L << 0,
		directory = 1L << 1,
		fileName =  1L << 2,
		extension = 1L << 3
	};

	explicit FilePath(std::string const &path);

	std::string get(int flags) const;
	std::string getFullPath() const;
	std::string getPath() const;
	char const *getDrive() const;
	char const *getDirectory() const;
	char const *getFileName() const;
	char const *getExtension() const;
	void        swapChar(char const sourceChar, char const destChar);
	bool        isReadable() const;
	bool        isWritable() const;
	bool        isIff() const;

private:

	char m_drive[_MAX_DRIVE];
	char m_directory[_MAX_DIR];
	char m_fileName[_MAX_FNAME];
	char m_extension[_MAX_EXT];

private:

	FilePath();
	FilePath &operator =(FilePath const &);
};

// ============================================================================

#endif // INCLUDED_TemplateEditorUtility_H