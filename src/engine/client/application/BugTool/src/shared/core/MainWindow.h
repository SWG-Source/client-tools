// ======================================================================
//
// MainWindow.h
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_MainWindow_H
#define INCLUDED_MainWindow_H

// ======================================================================

#include "BaseMainWindow.h"

// ======================================================================

class MainWindow: public BaseMainWindow
{
public:
	MainWindow(QWidget* newParent = 0, const char* newName = 0, WFlags fl = WType_TopLevel);
	~MainWindow();
	void setStationId(const QString& id);

public:
	static MainWindow &getInstance();

//slots (inherited from BaseMainWindow)
public slots:
	virtual void addAttachment();
	virtual void deleteAttachment(QListViewItem* item, const QPoint& pt, int);
	virtual void quitApplication();
	virtual void sendBug();
	virtual void setAttachmentSelection(QListViewItem* item);
	virtual void onDeleteAttachment();

private:
	void updateTitleBar();
	char* convertToSystemPath(const std::string & path);

private:
	//grab the default values from the designer, rather than hardcode them here
	QString m_defaultDescriptionString;
	QString m_defaultTitleBar;
	//the StationId the user (comes as the command line parameter)
	QString m_stationId;
	QListViewItem* m_selectedAttachment;
	QString m_currentDir;

private:
	static MainWindow *ms_theMainWindow;
	static const std::string ms_smtpServer;
	static const std::string ms_smtpPort;
	static const std::string ms_to;

};

// ======================================================================

#endif
