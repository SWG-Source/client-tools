#ifndef CHARTFORM_H
#define CHARTFORM_H

#include "element.h"

#include <qmainwindow.h>
#include <qstringlist.h>


class CanvasView;

class QAction;
class QCanvas;
class QFont;
class QPrinter;
class QString;


class ChartForm: public QMainWindow
{
    Q_OBJECT
public:
    enum { MAX_ELEMENTS = 100 };
    enum { MAX_RECENTFILES = 9 }; // Must not exceed 9
    enum ChartType { PIE, VERTICAL_BAR, HORIZONTAL_BAR };
    enum AddValuesType { NO, YES, AS_PERCENTAGE };

    ChartForm( const QString& filename );
    ~ChartForm();

    int chartType() { return m_chartType; }
    void setChanged( bool changed = TRUE ) { m_changed = changed; }
    void drawElements();

    QPopupMenu *optionsMenu; // Why public? See canvasview.cpp

protected:
    virtual void closeEvent( QCloseEvent * );

private slots:
    void fileNew();
    void fileOpen();
    void fileOpenRecent( int index );
    void fileSave();
    void fileSaveAs();
    void fileSaveAsPixmap();
    void filePrint();
    void fileQuit();
    void optionsSetData();
    void updateChartType( QAction *action );
    void optionsSetFont();
    void optionsSetOptions();
    void helpHelp();
    void helpAbout();
    void helpAboutQt();
    void saveOptions();

private:
    void init();
    void load( const QString& filename );
    bool okToClear();
    void drawPieChart( const double scales[], double total, int count );
    void drawVerticalBarChart( const double scales[], double total, int count );
    void drawHorizontalBarChart( const double scales[], double total, int count );

    QString valueLabel( const QString& label, double value, double total );
    void updateRecentFiles( const QString& filename );
    void updateRecentFilesMenu();
    void setChartType( ChartType chartType );

    QPopupMenu *fileMenu;
    QAction *optionsPieChartAction;
    QAction *optionsHorizontalBarChartAction;
    QAction *optionsVerticalBarChartAction;


    QString m_filename;
    QStringList m_recentFiles;
    QCanvas *m_canvas;
    CanvasView *m_canvasView;
    bool m_changed;
    ElementVector m_elements;
    QPrinter *m_printer;
    ChartType m_chartType;
    AddValuesType m_addValues;
    int m_decimalPlaces;
    QFont m_font;
};

#endif
