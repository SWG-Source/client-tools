#include <qapplication.h>
#include <qcheckbox.h>
#include <qclipboard.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <qstatusbar.h>
#include <qtable.h>

#include "regexptester.h"


RegexpTester::RegexpTester(QWidget* parent, const char* name, bool modal,
			   WFlags f)
    : QDialog(parent, name, modal, f)
{
    regexLabel = new QLabel(this);
    regexComboBox = new QComboBox(this);
    regexComboBox->setEditable(true);
    regexComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    regexLabel->setBuddy(regexComboBox);
    textLabel = new QLabel(this);
    textComboBox = new QComboBox(this);
    textComboBox->setEditable(true);
    textComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    textLabel->setBuddy(textComboBox);
    caseSensitiveCheckBox = new QCheckBox(this);
    caseSensitiveCheckBox->setChecked(true);
    minimalCheckBox = new QCheckBox(this);
    wildcardCheckBox = new QCheckBox(this);
    resultTable = new QTable(3, 3, this);
    resultTable->verticalHeader()->hide();
    resultTable->setLeftMargin(0);
    resultTable->horizontalHeader()->hide();
    resultTable->setTopMargin(0);
    resultTable->setReadOnly(true);
    executePushButton = new QPushButton(this);
    executePushButton->setDefault(true);
    copyPushButton = new QPushButton(this);
    quitPushButton = new QPushButton(this);
    statusBar = new QStatusBar(this);

    QGridLayout *gridLayout = new QGridLayout(2, 2, 6);
    gridLayout->addWidget(regexLabel, 0, 0);
    gridLayout->addWidget(regexComboBox, 0, 1);
    gridLayout->addWidget(textLabel, 1, 0);
    gridLayout->addWidget(textComboBox, 1, 1);
    QHBoxLayout *checkboxLayout = new QHBoxLayout(0, 6, 6);
    checkboxLayout->addWidget(caseSensitiveCheckBox);
    checkboxLayout->addWidget(minimalCheckBox);
    checkboxLayout->addWidget(wildcardCheckBox);
    checkboxLayout->addStretch(1);
    QVBoxLayout *buttonLayout = new QVBoxLayout(0, 6, 6);
    buttonLayout->addWidget(executePushButton);
    buttonLayout->addWidget(copyPushButton);
    buttonLayout->addWidget(quitPushButton);
    buttonLayout->addStretch(1);
    QHBoxLayout *middleLayout = new QHBoxLayout(0, 6, 6);
    middleLayout->addWidget(resultTable);
    middleLayout->addLayout(buttonLayout);
    QVBoxLayout *mainLayout = new QVBoxLayout(this, 6, 6);
    mainLayout->addLayout(gridLayout);
    mainLayout->addLayout(checkboxLayout);
    mainLayout->addLayout(middleLayout);
    mainLayout->addWidget(statusBar);

    resize(QSize(500, 350).expandedTo(minimumSizeHint()));

    languageChange();

    connect(copyPushButton, SIGNAL(clicked()), this, SLOT(copy()));
    connect(executePushButton, SIGNAL(clicked()), this, SLOT(execute()));
    connect(quitPushButton, SIGNAL(clicked()), this, SLOT(accept()));

    execute();
}

void RegexpTester::execute()
{
    QString regex = regexComboBox->currentText();
    QString text = textComboBox->currentText();
    if (!regex.isEmpty() && !text.isEmpty()) {
	QRegExp re(regex);
	re.setCaseSensitive(caseSensitiveCheckBox->isChecked());
	re.setMinimal(minimalCheckBox->isChecked());
	bool wildcard = wildcardCheckBox->isChecked();
	re.setWildcard(wildcard);
	if (!re.isValid()) {
	    statusBar->message(tr("Invalid regular expression: %1")
				.arg(re.errorString()));
	    return;
	}
	int offset = re.search(text);
	int captures = re.numCaptures();
	int row = 0;
	const int OFFSET = 5;
	resultTable->setNumRows(0);
	resultTable->setNumRows(captures + OFFSET);
	resultTable->setText(row, 0, tr("Regex"));
	QString escaped = regex;
	escaped = escaped.replace("\\", "\\\\");
	resultTable->setText(row, 1, escaped);
	resultTable->item(row, 1)->setSpan(1, 2);
	if (offset != -1) {
	    ++row;
	    resultTable->setText(row, 0, tr("Offset"));
	    resultTable->setText(row, 1, QString::number(offset));
	    resultTable->item(row, 1)->setSpan(1, 2);
	    if (!wildcard) {
		++row;
		resultTable->setText(row, 0, tr("Captures"));
		resultTable->setText(row, 1, QString::number(captures));
		resultTable->item(row, 1)->setSpan(1, 2);
		++row;
		resultTable->setText(row, 1, tr("Text"));
		resultTable->setText(row, 2, tr("Characters"));
	    }
	    ++row;
	    resultTable->setText(row, 0, tr("Match"));
	    resultTable->setText(row, 1, re.cap(0));
	    resultTable->setText(row, 2, QString::number(re.matchedLength()));
	    if (!wildcard) {
		for (int i = 1; i <= captures; ++i) {
		    resultTable->setText(row + i, 0, tr("Capture #%1").arg(i));
		    resultTable->setText(row + i, 1, re.cap(i));
		    resultTable->setText(row + i, 2,
					QString::number(re.cap(i).length()));
		}
	    }
	    else
		resultTable->setNumRows(3);
	}
	else {
	    resultTable->setNumRows(2);
	    ++row;
	    resultTable->setText(row, 0, tr("No matches"));
	    resultTable->item(row, 0)->setSpan(1, 3);
	}
	resultTable->adjustColumn(0);
	resultTable->adjustColumn(1);
	resultTable->adjustColumn(2);
	statusBar->message(tr("Executed \"%1\" on \"%2\"")
				.arg(escaped).arg(text));
    }
    else
	statusBar->message(tr("A regular expression and a text must be given"));
}

void RegexpTester::copy()
{
    QString escaped = regexComboBox->currentText();
    if (!escaped.isEmpty()) {
	escaped = escaped.replace("\\", "\\\\");
	QClipboard *cb = QApplication::clipboard();
	cb->setText(escaped, QClipboard::Clipboard);
	if (cb->supportsSelection())
	    cb->setText(escaped, QClipboard::Selection);
	statusBar->message(tr("Copied \"%1\" to the clipboard")
				.arg(escaped));
    }
}

void RegexpTester::languageChange()
{
    setCaption(tr("Regex Tester"));
    regexLabel->setText(tr("&Regex:"));
    regexComboBox->insertItem(tr("[A-Z]+=(\\d+):(\\d*)"));
    textLabel->setText(tr("&Text:"));
    textComboBox->insertItem(tr("ABC=12:3456"));
    caseSensitiveCheckBox->setText(tr("Case &Sensitive"));
    minimalCheckBox->setText(tr("&Minimal"));
    wildcardCheckBox->setText(tr("&Wildcard"));
    copyPushButton->setText(tr("&Copy"));
    executePushButton->setText(tr("&Execute"));
    quitPushButton->setText(tr("&Quit"));
}

