// ============================================================================
//
// TemplateEditorUtility.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstTemplateEditor.h"
#include "TemplateEditorUtility.h"

// ============================================================================
//
// TemplateEditorUtility
//
// ============================================================================

QTextEdit *TemplateEditorUtility::m_outputWindow = NULL;
char TemplateEditorUtility::ms_logPath[] = "TemplateEditor_Log.txt";
char TemplateEditorUtility::ms_successListPath[] = "TemplateEditor_SuccessList.txt";

//-----------------------------------------------------------------------------
void TemplateEditorUtility::setOutputWindow(QTextEdit &outputWindow)
{
	outputWindow.clear();
	outputWindow.setReadOnly(true);
	m_outputWindow = &outputWindow;

	// Create the error file

	QFile deleteFile(ms_logPath);
	deleteFile.remove();
}

//-----------------------------------------------------------------------------
void TemplateEditorUtility::report(QString const &text)
{
	NOT_NULL(m_outputWindow);

	if ((m_outputWindow != NULL) && !text.isNull())
	{
		//QString withTime(QTime::currentTime().toString(Qt::TextDate));
		QString withTime(QTime::currentTime().toString(Qt::LocalDate));
		withTime += " - " + text;
		m_outputWindow->append(withTime);

		// Log the error

		QFile writeFile(ms_logPath);
		bool const writeFileOpenResult = writeFile.open(IO_Append | IO_Translate | IO_WriteOnly);

		if (writeFileOpenResult)
		{
			QTextStream writeFileStream(&writeFile);

			writeFileStream << withTime;

			if (strchr(withTime.latin1(),'\n') == NULL)
			{
				writeFileStream << '\n';
			}

			writeFile.close();
		}
	}
}

//-----------------------------------------------------------------------------
void TemplateEditorUtility::saveWidget(QWidget const &widget)
{
   QSettings settings;
	settings.insertSearchPath(QSettings::Windows, getSearchPath());
   
	QWidget const *parent = (widget.parentWidget() == NULL) ? &widget : widget.parentWidget();

   int const x = parent->pos().x();
   int const y = parent->pos().y();
   int const w = widget.width();
   int const h = widget.height();

   char text[256];
   sprintf(text, "%s_PositionX", widget.name());
   settings.writeEntry(text, x);

   sprintf(text, "%s_PositionY", widget.name());
   settings.writeEntry(text, y);

   sprintf(text, "%s_Width", widget.name());
   settings.writeEntry(text, w);

   sprintf(text, "%s_Height", widget.name());
   settings.writeEntry(text, h);
}

//-----------------------------------------------------------------------------
void TemplateEditorUtility::loadWidget(QWidget &widget, int const defaultX, int const defaultY, int const defaultWidth, int const defaultHeight)
{
   QSettings settings;
	settings.insertSearchPath(QSettings::Windows, getSearchPath());

   char text[256];
   sprintf(text, "%s_PositionX", widget.name());
   int x = settings.readNumEntry(text, defaultX);
	x = (x > 10000) ? defaultX : x;

   sprintf(text, "%s_PositionY", widget.name());
   int y = settings.readNumEntry(text, defaultY);
	y = (y > 10000) ? defaultY : y;

   sprintf(text, "%s_Width", widget.name());
   int w = settings.readNumEntry(text, defaultWidth);
	w = (w > 10000) ? defaultWidth : w;

   sprintf(text, "%s_Height", widget.name());
   int h = settings.readNumEntry(text, defaultHeight);
	h = (h > 10000) ? defaultHeight : h;

   widget.move(x, y);
   widget.resize(w, h);
	widget.show();
}

//-----------------------------------------------------------------------------
const char *TemplateEditorUtility::getSearchPath()
{
	return "/SOE/SwgTemplateEditor";
}

//-----------------------------------------------------------------------------
const char *TemplateEditorUtility::getLogPath()
{
	return ms_logPath;
}

//-----------------------------------------------------------------------------
const char *TemplateEditorUtility::getSuccessListPath()
{
	return ms_successListPath;
}

//-----------------------------------------------------------------------------
int TemplateEditorUtility::validateLineEditInt(QLineEdit *lineEdit, int const min, int const max)
{
	return setLineEditInt(lineEdit, getInt(lineEdit), min, max);
}

//-----------------------------------------------------------------------------
int TemplateEditorUtility::setLineEditInt(QLineEdit *lineEdit, int const value, int const min, int const max)
{
	int result = value;

	if (result > max)
	{
		result = max;
	}
	else if (result < min)
	{
		result = min;
	}

	char text[256];
	sprintf(text, "%d", result);
	lineEdit->setText(text);

	return result;
}

//-----------------------------------------------------------------------------
int TemplateEditorUtility::getInt(QLineEdit *lineEdit)
{
	bool valid = true;

	int result = lineEdit->text().toInt(&valid);

	// Just slam the value so we don't crash

	if (!valid)
	{
		result = 0;
	}

	return result;
}

// ============================================================================
//
// FilePath
//
// ============================================================================

//-----------------------------------------------------------------------------
FilePath::FilePath(std::string const &path)
{
	memset(&m_drive, 0, sizeof(m_drive));
	memset(&m_directory, 0, sizeof(m_directory));
	memset(&m_fileName, 0, sizeof(m_fileName));
	memset(&m_extension, 0, sizeof(m_extension));

	int const size = path.size();
	UNREF(size);

	if (path.size() > 2)
	{
		_splitpath(path.c_str(), m_drive, m_directory, m_fileName, m_extension);
	}
}

//-----------------------------------------------------------------------------
std::string FilePath::get(int flags) const
{
	std::string result("");

	if (flags & drive)
	{
		result += m_drive;
	}
	if (flags & directory)
	{
		result += m_directory;
	}
	if (flags & fileName)
	{
		result += m_fileName;
	}
	if (flags & extension)
	{
		result += m_extension;
	}
	
	return result;
}

//-----------------------------------------------------------------------------
std::string FilePath::getFullPath() const
{
	return get(drive | directory | fileName | extension);
}

//-----------------------------------------------------------------------------
void FilePath::swapChar(char const sourceChar, char const destChar)
{
	unsigned int i;

	for (i = 0; i < _MAX_DRIVE; ++i)
	{
		if (m_drive[i] == sourceChar)
		{
			m_drive[i] = destChar;
		}
	}

	for (i = 0; i < _MAX_DIR; ++i)
	{
		if (m_directory[i] == sourceChar)
		{
			m_directory[i] = destChar;
		}
	}

	for (i = 0; i < _MAX_FNAME; ++i)
	{
		if (m_fileName[i] == sourceChar)
		{
			m_fileName[i] = destChar;
		}
	}

	for (i = 0; i < _MAX_EXT; ++i)
	{
		if (m_extension[i] == sourceChar)
		{
			m_extension[i] = destChar;
		}
	}
}

//-----------------------------------------------------------------------------
bool FilePath::isReadable() const
{
	bool result = false;
	std::string fullPath(getFullPath());

	if (!fullPath.empty())
	{
		FILE *fp = fopen(getFullPath().c_str(), "r");

		result = (fp != NULL);

		if (fp != NULL)
		{
			fclose(fp);
		}
	}

	return result;
}

//-----------------------------------------------------------------------------
bool FilePath::isWritable() const
{
	bool result = false;
	std::string fullPath(getFullPath());

	if (!fullPath.empty())
	{
		FILE *fp = fopen(getFullPath().c_str(), "w");

		result = (fp != NULL);

		if (fp != NULL)
		{
			fclose(fp);
		}
	}

	return result;
}

//-----------------------------------------------------------------------------
bool FilePath::isIff() const
{
	bool result = false;
	FILE *fp = fopen(getFullPath().c_str(), "rt");

	if ((fp) && 
	    (getc(fp) == 'F') &&
	    (getc(fp) == 'O') &&
	    (getc(fp) == 'R') &&
	    (getc(fp) == 'M'))
	{
		result = true;
		fclose(fp);
	}

	return result;
}

//-----------------------------------------------------------------------------
char const *FilePath::getDrive() const
{
	return m_drive;
}

//-----------------------------------------------------------------------------
char const *FilePath::getDirectory() const
{
	return m_directory;
}

//-----------------------------------------------------------------------------
char const *FilePath::getFileName() const
{
	return m_fileName;
}

//-----------------------------------------------------------------------------
char const *FilePath::getExtension() const
{
	return m_extension;
}

//-----------------------------------------------------------------------------
std::string FilePath::getPath() const
{
	return get(drive | directory);
}

// ============================================================================
