// ======================================================================
//
// ApplicationWindow.h
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef ApplicationWindow_H
#define ApplicationWindow_H

// ======================================================================

class TextEdit;
class QPopupMenu;

#include <qmainwindow.h>
#include <vector>

// ======================================================================
/**
 * This class represents the main window (or Widget, in Qt-speak) of the application.
 */

class ApplicationWindow: public QMainWindow
{
	// necessary to make it a Qt object
	Q_OBJECT

public:

	enum MessageType
	{
		MT_normal,
		MT_error,
		MT_status
	};

	ApplicationWindow(QApplication &application);
	virtual ~ApplicationWindow();

	void appendMessage(MessageType type, QString message);

public slots:

	void clear();
	void sync();
	void previewSync();
	void forceResyncErrorFiles();

protected:

	void sync(bool preview, const QString &syncType);
	virtual void closeEvent(QCloseEvent*);

private:
	ApplicationWindow (const ApplicationWindow&);
	ApplicationWindow& operator= (const ApplicationWindow&);

private:
	QApplication         &m_application;
	TextEdit             *m_textEdit;
	QPopupMenu           *m_fileMenu;
	int                   m_forceResyncItemNumber;
	std::vector<QString>  m_errorFiles;
	bool                  m_summary;
	const char           *m_client;
};

// ======================================================================

#endif
