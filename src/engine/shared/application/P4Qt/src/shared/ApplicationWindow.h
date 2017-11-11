// ======================================================================
//
// ApplicationWindow.h
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef ApplicationWindow_H
#define ApplicationWindow_H

// ======================================================================

class DepotListViewItem;
class ClientUser;
class QTextEdit;
class QListView;
class QListViewItem;

#include <qmainwindow.h>
#include <qstring.h>
#include <qpixmap.h>
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

	void p4(bool tags, const std::vector<const char *> &command, ClientUser &clientUser);

	void addDepot(QString depotName);
	void addOpenedFile(QString changelist, QString file, bool binary);
	void outputMessage(MessageType type, QString message);

public slots:

	// slots dealing with the depot window
	void depots();
	void previewSync();
	void sync();
	void edit();

	// slots dealing with the changelist window
	void opened();
	void revert();
	void submit();

	// slots dealing with the output window
	void clear();

protected slots:

	void depotContextMenu(QListViewItem *, const QPoint &, int);
	void changelistContextMenu(QListViewItem *, const QPoint &, int);

protected:

	virtual void closeEvent(QCloseEvent*);

private:

	ApplicationWindow (const ApplicationWindow&);
	ApplicationWindow& operator= (const ApplicationWindow&);

	void getSelectedDepotFiles(QListViewItem *item, std::vector<const char *> &files);
	void getSelectedDepotFiles(std::vector<const char *> &files);

	void getSelectedChangelistsFiles(QListViewItem *item, std::vector<const char *> &files);
	void getSelectedChangelistsFiles(std::vector<const char *> &files);

private:
	typedef std::vector<QString>  ErrorList;
	
private:
	QApplication         &m_application;
	QListView            *m_depot;
	QListView            *m_changelists;
	QTextEdit            *m_output;
	bool                  m_summary;
	std::vector<QString>  m_errors;
};

// ======================================================================

#endif
