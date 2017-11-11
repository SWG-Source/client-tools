/****************************************************************************
** $Id: qt/qaxscript.h   3.3.4   edited Dec 3 2003 $
**
** Declaration of the QAxScriptEngine, QAxScript and QAxScriptManager classes
**
** Copyright (C) 2002-2003 Trolltech AS.  All rights reserved.
**
** This file is part of the Active Qt integration.
**
** Licensees holding valid Qt Enterprise Edition
** licenses for Windows may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QAXSCRIPT_H
#define QAXSCRIPT_H

#include <qaxobject.h>

class QAxBase;
class QAxScript;
class QAxScriptSite;
class QAxScriptEngine;
class QAxScriptManager;
class QAxScriptManagerPrivate;
struct IActiveScript;

class QAxScriptEngine : public QAxObject
{
public:
    enum State {
	Uninitialized = 0,
	Initialized = 5,
	Started = 1,
	Connected = 2,
	Disconnected = 3,
	Closed = 4
    };

    QAxScriptEngine(const QString &language, QAxScript *script);
    ~QAxScriptEngine();

    bool isValid() const;
    bool hasIntrospection() const;

    QString scriptLanguage() const;

    State state() const;
    void setState(State st);

    void addItem(const QString &name);

    long queryInterface( const QUuid &, void** ) const;

protected:
    bool initialize(IUnknown** ptr);

private:
    QAxScript *script_code;
    IActiveScript *engine;

    QString script_language;
};

class QAxScript : public QObject
{
    Q_OBJECT

public:
    enum FunctionFlags {
	FunctionNames = 0,
	FunctionSignatures	
    };

    QAxScript(const QString &name, QAxScriptManager *manager);
    ~QAxScript();

    bool load(const QString &code, const QString &language = QString::null);

    QStringList functions(FunctionFlags = FunctionNames) const;

    QString scriptCode() const;
    QString scriptName() const;
    QAxScriptEngine *scriptEngine() const;

    QVariant call(const QString &function, const QVariant &v1 = QVariant(), 
					   const QVariant &v2 = QVariant(),
					   const QVariant &v3 = QVariant(),
					   const QVariant &v4 = QVariant(),
					   const QVariant &v5 = QVariant(),
					   const QVariant &v6 = QVariant(),
					   const QVariant &v7 = QVariant(),
					   const QVariant &v8 = QVariant());
    QVariant call(const QString &function, QValueList<QVariant> &arguments);

signals:
    void entered();
    void finished();
    void finished(const QVariant &result);
    void finished(int code, const QString &source,const QString &description, const QString &help);
    void stateChanged(int state);
    void error(int code, const QString &description, int sourcePosition, const QString &sourceText);

private:
    friend class QAxScriptSite;
    friend class QAxScriptEngine;

    void updateObjects();
    QAxBase *findObject(const QString &name);

    QString script_name;
    QString script_code;
    QAxScriptManager *script_manager;
    QAxScriptEngine *script_engine;
    QAxScriptSite *script_site;
};

class QAxScriptManager : public QObject
{
    Q_OBJECT

public:
    QAxScriptManager( QObject *parent = 0, const char *name = 0 );
    ~QAxScriptManager();

    void addObject(QAxBase *object);
    void addObject(QObject *object);

    QStringList functions(QAxScript::FunctionFlags = QAxScript::FunctionNames) const;
    QStringList scriptNames() const;
    QAxScript *script(const QString &name) const;

    QAxScript* load(const QString &code, const QString &name, const QString &language);
    QAxScript* load(const QString &file, const QString &name);

    QVariant call(const QString &function, const QVariant &v1 = QVariant(), 
					   const QVariant &v2 = QVariant(),
					   const QVariant &v3 = QVariant(),
					   const QVariant &v4 = QVariant(),
					   const QVariant &v5 = QVariant(),
					   const QVariant &v6 = QVariant(),
					   const QVariant &v7 = QVariant(),
					   const QVariant &v8 = QVariant());
    QVariant call(const QString &function, QValueList<QVariant> &arguments);

    static bool registerEngine(const QString &name, const QString &extension, const QString &code = QString());
    static QString scriptFileFilter();

signals:
    void error(QAxScript *script, int code, const QString &description, int sourcePosition, const QString &sourceText);

private slots:
    void objectDestroyed(QObject *o);
    void scriptError(int code, const QString &description, int sourcePosition, const QString &sourceText);

private:
    friend class QAxScript;
    QAxScriptManagerPrivate *d;

    void updateScript(QAxScript*);
    QAxScript *scriptForFunction(const QString &function) const;
};


// QAxScript inlines

inline QString QAxScript::scriptCode() const
{
    return script_code;
}

inline QString QAxScript::scriptName() const
{
    return script_name;
}

inline QAxScriptEngine *QAxScript::scriptEngine() const
{
    return script_engine;
}

// QAxScriptEngine inlines

inline bool QAxScriptEngine::isValid() const
{
    return engine != 0;
}

inline QString QAxScriptEngine::scriptLanguage() const
{
    return script_language;
}

// QAxScriptManager inlines

inline void QAxScriptManager::addObject(QObject *object)
{
    extern QAxBase *qax_create_object_wrapper(QObject*);
    QAxBase *wrapper = qax_create_object_wrapper(object);
    addObject(wrapper);
}

#endif // QAXSCRIPT_H
