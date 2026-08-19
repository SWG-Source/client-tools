#ifndef REGEXPTESTER_H
#define REGEXPTESTER_H

#include <qdialog.h>

class QCheckBox;
class QComboBox;
class QLabel;
class QPushButton;
class QStatusBar;
class QTable;

class RegexpTester : public QDialog
{
    Q_OBJECT

public:
    RegexpTester(QWidget* parent=0, const char* name=0, bool modal=false,
		 WFlags f=0);

    QLabel *regexLabel;
    QComboBox *regexComboBox;
    QLabel *textLabel;
    QComboBox *textComboBox;
    QCheckBox *caseSensitiveCheckBox;
    QCheckBox *minimalCheckBox;
    QCheckBox *wildcardCheckBox;
    QTable *resultTable;
    QPushButton *executePushButton;
    QPushButton *copyPushButton;
    QPushButton *quitPushButton;
    QStatusBar *statusBar;

public slots:
    void copy();
    void execute();

private:
    void languageChange();
};

#endif // REGEXPTESTER_H
