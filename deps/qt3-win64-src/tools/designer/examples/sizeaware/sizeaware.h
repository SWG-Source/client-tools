#ifndef SIZEAWARE_H
#define SIZEAWARE_H
#include <qdialog.h>

class SizeAware : public QDialog
{
    Q_OBJECT
    Q_PROPERTY( QString company READ company WRITE setCompany )
    Q_PROPERTY( QString settingsFile READ settingsFile WRITE setSettingsFile )
public:
    SizeAware( QDialog *parent = 0, const char *name = 0, bool modal = FALSE );
    ~SizeAware();
    void setCompany( QString company ) { m_company = company; }
    QString company() const { return m_company; } 
    void setSettingsFile( QString settingsFile ) { m_settingsFile = settingsFile; }
    QString settingsFile() const { return m_settingsFile; }
public slots:
    void destroy();
private:
    QString m_company;
    QString m_settingsFile;
};
#endif
