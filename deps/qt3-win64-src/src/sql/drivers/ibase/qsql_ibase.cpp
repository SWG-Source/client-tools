/****************************************************************************
**
** Implementation of Interbase driver classes.
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of the sql module of the Qt GUI Toolkit.
** EDITIONS: FREE, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qsql_ibase.h"

#include <qdatetime.h>
#include <private/qsqlextension_p.h>

#include <ibase.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>

#define QIBASE_DRIVER_NAME "QIBASE"

class QIBasePreparedExtension : public QSqlExtension
{
public:
    QIBasePreparedExtension(QIBaseResult *r)
	: result(r) {}

    bool prepare(const QString &query)
    {
	return result->prepare(query);
    }
    
    bool exec()
    {
	return result->exec();
    }

    QIBaseResult *result;
};

static bool getIBaseError(QString& msg, ISC_STATUS* status, long &sqlcode)
{
    if (status[0] != 1 || status[1] <= 0)
	return FALSE;
    
    sqlcode = isc_sqlcode(status);
    char buf[512];
    isc_sql_interprete(sqlcode, buf, 512);
    msg = QString::fromUtf8(buf);
    return TRUE;
}

static void createDA(XSQLDA *&sqlda)
{
    sqlda = (XSQLDA *) malloc(XSQLDA_LENGTH(1));
    sqlda->sqln = 1;
    sqlda->sqld = 0;
    sqlda->version = SQLDA_VERSION1;
    sqlda->sqlvar[0].sqlind = 0;
    sqlda->sqlvar[0].sqldata = 0;
}

static void enlargeDA(XSQLDA *&sqlda, int n)
{
    free(sqlda);
    sqlda = (XSQLDA *) malloc(XSQLDA_LENGTH(n));
    sqlda->sqln = n;
    sqlda->version = SQLDA_VERSION1;    
}

static void initDA(XSQLDA *sqlda)
{
    for (int i = 0; i < sqlda->sqld; ++i) {
	switch (sqlda->sqlvar[i].sqltype & ~1) {
	case SQL_INT64:
	case SQL_LONG:
	case SQL_SHORT:
	case SQL_FLOAT:
	case SQL_DOUBLE:
	case SQL_TIMESTAMP:
	case SQL_TYPE_TIME:
        case SQL_TYPE_DATE:
	case SQL_TEXT:
	case SQL_BLOB:
	    sqlda->sqlvar[i].sqldata = (char*)malloc(sqlda->sqlvar[i].sqllen);
	    break;
	case SQL_VARYING:
	    sqlda->sqlvar[i].sqldata = (char*)malloc(sqlda->sqlvar[i].sqllen + sizeof(short));
	    break;
	default:
	    // not supported - do not bind.
	    sqlda->sqlvar[i].sqldata = 0;
	    break;
	}
	if (sqlda->sqlvar[i].sqltype & 1) {
	    sqlda->sqlvar[i].sqlind = (short*)malloc(sizeof(short));
	    *(sqlda->sqlvar[i].sqlind) = 0;
	} else {
	    sqlda->sqlvar[i].sqlind = 0;
	}
    }
}

static void delDA(XSQLDA *&sqlda)
{
    if (!sqlda)
	return;
    for (int i = 0; i < sqlda->sqld; ++i) {
	free(sqlda->sqlvar[i].sqlind);
	free(sqlda->sqlvar[i].sqldata);
    }
    free(sqlda);
    sqlda = 0;    
}

static QVariant::Type qIBaseTypeName(int iType)
{
    switch (iType) {
    case blr_varying:
    case blr_varying2:
    case blr_text:
    case blr_cstring:
    case blr_cstring2:
	return QVariant::String;
    case blr_sql_time:
	return QVariant::Time;
    case blr_sql_date:
        return QVariant::Date;
    case blr_timestamp:
	return QVariant::DateTime;
    case blr_blob:
	return QVariant::ByteArray;
    case blr_quad:
    case blr_short:
    case blr_long:
	return QVariant::Int;
    case blr_int64:
	return QVariant::LongLong;
    case blr_float:
    case blr_d_float:
    case blr_double:
	return QVariant::Double;
    }
    return QVariant::Invalid;
}

static QVariant::Type qIBaseTypeName2(int iType)
{
    switch(iType & ~1) {
    case SQL_VARYING:
    case SQL_TEXT:
	return QVariant::String;
    case SQL_LONG:
    case SQL_SHORT:
	return QVariant::Int;
    case SQL_INT64:
	return QVariant::LongLong;
    case SQL_FLOAT:
    case SQL_DOUBLE:
	return QVariant::Double;
    case SQL_TIMESTAMP:
	return QVariant::DateTime;
    case SQL_TYPE_DATE:
        return QVariant::Date;
    case SQL_TYPE_TIME:
	return QVariant::Time;
    default:
	return QVariant::Invalid;
    }
}

static ISC_TIME toTime(const QTime &t)
{
    static const QTime midnight(0, 0, 0, 0);
    return (ISC_TIME)midnight.msecsTo(t) * 10;
}

static ISC_DATE toDate(const QDate &d)
{
    static const QDate basedate(1858, 11, 17);
    return (ISC_DATE)basedate.daysTo(d);
}

static ISC_TIMESTAMP toTimeStamp(const QDateTime &dt)
{
    ISC_TIMESTAMP ts;
    ts.timestamp_time = toTime(dt.time());
    ts.timestamp_date = toDate(dt.date());
    return ts;
}

static QTime toQTime(ISC_TIME time)
{
    // have to demangle the structure ourselves because isc_decode_time
    // strips the msecs
    static const QTime t;
    return t.addMSecs(time / 10);
}

static QDate toQDate(ISC_DATE d)
{
    static const QDate bd(1858, 11, 17);
    return bd.addDays(d);
}

static QDateTime toQDateTime(ISC_TIMESTAMP *ts)
{
    return QDateTime(toQDate(ts->timestamp_date), toQTime(ts->timestamp_time));
}

class QIBaseDriverPrivate
{
public:
    QIBaseDriverPrivate(QIBaseDriver *d): q(d)
    {
	ibase = 0;
	trans = 0;
    }
    
    bool isError(const QString &msg = QString::null, QSqlError::Type typ = QSqlError::Unknown) 
    {
	QString imsg;
	long sqlcode;
	if (!getIBaseError(imsg, status, sqlcode))
	    return FALSE;
	
	q->setLastError(QSqlError(msg, imsg, typ, (int)sqlcode));
	return TRUE;
    }

public:    
    QIBaseDriver* q;
    isc_db_handle ibase;
    isc_tr_handle trans;
    ISC_STATUS status[20];    
};

class QIBaseResultPrivate
{
public:
    QIBaseResultPrivate(QIBaseResult *d, const QIBaseDriver *ddb);
    ~QIBaseResultPrivate() { cleanup(); }
    
    void cleanup();
    bool isError(const QString &msg = QString::null, QSqlError::Type typ = QSqlError::Unknown) 
    {
	QString imsg;
	long sqlcode;
	if (!getIBaseError(imsg, status, sqlcode))
	    return FALSE;
	
	q->setLastError(QSqlError(msg, imsg, typ, (int)sqlcode));
	return TRUE;
    }    
    
    bool transaction();
    bool commit();

    bool isSelect();
    QVariant fetchBlob(ISC_QUAD *bId);
    void writeBlob(int i, const QByteArray &ba);

public:    
    QIBaseResult *q; 
    const QIBaseDriver *db;
    ISC_STATUS status[20];
    isc_tr_handle trans;
    //indicator whether we have a local transaction or a transaction on driver level    
    bool localTransaction;
    isc_stmt_handle stmt;
    isc_db_handle ibase;
    XSQLDA *sqlda; // output sqlda
    XSQLDA *inda; // input parameters
    int queryType;
};

QIBaseResultPrivate::QIBaseResultPrivate(QIBaseResult *d, const QIBaseDriver *ddb):
    q(d), db(ddb), trans(0), stmt(0), ibase(ddb->d->ibase), sqlda(0), inda(0), queryType(-1)
{
    localTransaction = (ddb->d->ibase == 0);
}

void QIBaseResultPrivate::cleanup()
{
    if (stmt) {
	isc_dsql_free_statement(status, &stmt, DSQL_drop);
	stmt = 0;
    }

    commit();
    if (!localTransaction)
	trans = 0;

    delDA(sqlda);
    delDA(inda);

    queryType = -1;
    q->cleanup();
}

void QIBaseResultPrivate::writeBlob(int i, const QByteArray &ba)
{
    isc_blob_handle handle = 0;
    ISC_QUAD *bId = (ISC_QUAD*)inda->sqlvar[i].sqldata;
    isc_create_blob2(status, &ibase, &trans, &handle, bId, 0, 0);
    if (!isError("Unable to create BLOB", QSqlError::Statement)) {
	uint i = 0;
	while (i < ba.size()) {
	    isc_put_segment(status, &handle, QMIN(ba.size() - i, SHRT_MAX), ba.data());
	    if (isError("Unable to write BLOB"))
		break;
	    i += SHRT_MAX;
	}
    }
    isc_close_blob(status, &handle);
}

QVariant QIBaseResultPrivate::fetchBlob(ISC_QUAD *bId)
{
    isc_blob_handle handle = 0;

    isc_open_blob2(status, &ibase, &trans, &handle, bId, 0, 0);
    if (isError("Unable to open BLOB", QSqlError::Statement))
	return QVariant();

    unsigned short len = 0;
    QByteArray ba(255);
    ISC_STATUS stat = isc_get_segment(status, &handle, &len, ba.size(), ba.data());
    while (status[1] == isc_segment) {
	uint osize = ba.size();
	// double the amount of data fetched on each iteration
	ba.resize(QMIN(ba.size() * 2, SHRT_MAX));
	stat = isc_get_segment(status, &handle, &len, osize, ba.data() + osize);
    }
    bool isErr = isError("Unable to read BLOB", QSqlError::Statement);
    isc_close_blob(status, &handle);
    if (isErr)
	return QVariant();

    if (ba.size() > 255)
	ba.resize(ba.size() / 2 + len);
    else
	ba.resize(len);

    return ba;
}

bool QIBaseResultPrivate::isSelect()
{
    char acBuffer[9];
    char qType = isc_info_sql_stmt_type;
    isc_dsql_sql_info(status, &stmt, 1, &qType, sizeof(acBuffer), acBuffer);
    if (isError("Could not get query info", QSqlError::Statement))
	return FALSE;    
    int iLength = isc_vax_integer(&acBuffer[1], 2);
    queryType = isc_vax_integer(&acBuffer[3], iLength);
    return (queryType == isc_info_sql_stmt_select);
}

bool QIBaseResultPrivate::transaction()
{
    if (trans)
	return TRUE;
    if (db->d->trans) {
	localTransaction = FALSE;
	trans = db->d->trans;
	return TRUE;
    }
    localTransaction = TRUE;
    
    isc_start_transaction(status, &trans, 1, &ibase, 0, NULL);
    if (isError("Could not start transaction", QSqlError::Statement))
	return FALSE;
    
    return TRUE;
}

// does nothing if the transaction is on the
// driver level
bool QIBaseResultPrivate::commit()
{
    if (!trans)
	return FALSE;
    // don't commit driver's transaction, the driver will do it for us
    if (!localTransaction)
	return TRUE;
    
    isc_commit_transaction(status, &trans);
    trans = 0;
    return !isError("Unable to commit transaction", QSqlError::Statement);
}

//////////

QIBaseResult::QIBaseResult(const QIBaseDriver* db):
    QtSqlCachedResult(db)
{
    d = new QIBaseResultPrivate(this, db);
    setExtension(new QIBasePreparedExtension(this));
}

QIBaseResult::~QIBaseResult()
{
    delete d;
}

bool QIBaseResult::prepare(const QString& query)
{
    //qDebug("prepare: %s", query.ascii());
    if (!driver() || !driver()->isOpen() || driver()->isOpenError())
	return FALSE;
    d->cleanup();
    setActive(FALSE);
    setAt(QSql::BeforeFirst);
    
    createDA(d->sqlda);
    createDA(d->inda);
    
    if (!d->transaction())
	return FALSE;
    
    isc_dsql_allocate_statement(d->status, &d->ibase, &d->stmt);
    if (d->isError("Could not allocate statement", QSqlError::Statement))
	return FALSE;
    isc_dsql_prepare(d->status, &d->trans, &d->stmt, 0, query.utf8().data(), 3, d->sqlda);
    if (d->isError("Could not prepare statement", QSqlError::Statement))
	return FALSE;
    
    isc_dsql_describe_bind(d->status, &d->stmt, 1, d->inda);
    if (d->isError("Could not describe input statement", QSqlError::Statement))
	return FALSE;
    if (d->inda->sqld > d->inda->sqln) {
	enlargeDA(d->inda, d->inda->sqld);

	isc_dsql_describe_bind(d->status, &d->stmt, 1, d->inda);
	if (d->isError("Could not describe input statement", QSqlError::Statement))
	    return FALSE;
    }
    initDA(d->inda);    
    if (d->sqlda->sqld > d->sqlda->sqln) {
	// need more field descriptors
	enlargeDA(d->sqlda, d->sqlda->sqld);

	isc_dsql_describe(d->status, &d->stmt, 1, d->sqlda);
	if (d->isError("Could not describe statement", QSqlError::Statement))
	    return FALSE;
    }
    initDA(d->sqlda);
 
    setSelect(d->isSelect());
    if (!isSelect()) {
	free(d->sqlda);
	d->sqlda = 0;
    }

    return TRUE;
}

bool QIBaseResult::exec()
{    
    if (!driver() || !driver()->isOpen() || driver()->isOpenError())
	return FALSE;
    setActive(FALSE);
    setAt(QSql::BeforeFirst);

    if (d->inda && extension()->index.count() > 0) {
	QMap<int, QString>::ConstIterator it;
	if ((int)extension()->index.count() > d->inda->sqld) {
	    qWarning("QIBaseResult::exec: Parameter mismatch, expected %d, got %d parameters", d->inda->sqld, extension()->index.count());
	    return FALSE;
	}
	int para = 0;
	for (it = extension()->index.constBegin(); it != extension()->index.constEnd(); ++it, ++para) {
	    if (para >= d->inda->sqld)
		break;
	    if (!d->inda->sqlvar[para].sqldata)
		continue;
	    const QVariant val(extension()->values[it.data()].value);
	    if (d->inda->sqlvar[para].sqltype & 1) {
		if (val.isNull()) {
		    // set null indicator
		    *(d->inda->sqlvar[para].sqlind) = 1;
		    // and set the value to 0, otherwise it would count as empty string.
		    *((short*)d->inda->sqlvar[para].sqldata) = 0;
		    continue;
		}
		// a value of 0 means non-null.
		*(d->inda->sqlvar[para].sqlind) = 0;
	    }
	    switch(d->inda->sqlvar[para].sqltype & ~1) {
	    case SQL_INT64:
		if (d->inda->sqlvar[para].sqlscale < 0)
		    *((Q_LLONG*)d->inda->sqlvar[para].sqldata) = Q_LLONG(val.toDouble() * 
									 pow(10.0, d->inda->sqlvar[para].sqlscale * -1));
		else
		    *((Q_LLONG*)d->inda->sqlvar[para].sqldata) = val.toLongLong();
		break;
	    case SQL_LONG:
		*((long*)d->inda->sqlvar[para].sqldata) = (long)val.toLongLong();
		break;
	    case SQL_SHORT:
		*((short*)d->inda->sqlvar[para].sqldata) = (short)val.toInt();
		break;
	    case SQL_FLOAT:
		*((float*)d->inda->sqlvar[para].sqldata) = (float)val.toDouble();
		break;
	    case SQL_DOUBLE:
		*((double*)d->inda->sqlvar[para].sqldata) = val.toDouble();
		break;
	    case SQL_TIMESTAMP:
		*((ISC_TIMESTAMP*)d->inda->sqlvar[para].sqldata) = toTimeStamp(val.toDateTime());
		break;
	    case SQL_TYPE_TIME:
		*((ISC_TIME*)d->inda->sqlvar[para].sqldata) = toTime(val.toTime());
		break;
            case SQL_TYPE_DATE:
		*((ISC_DATE*)d->inda->sqlvar[para].sqldata) = toDate(val.toDate());
		break;
	    case SQL_VARYING: {
		QCString str(val.toString().utf8()); // keep a copy of the string alive in this scope
		short buflen = d->inda->sqlvar[para].sqllen;
		if (str.length() < (uint)buflen)
		    buflen = str.length();
		*(short*)d->inda->sqlvar[para].sqldata = buflen; // first two bytes is the length
		memcpy(d->inda->sqlvar[para].sqldata + sizeof(short), str.data(), buflen);
		break; }
	    case SQL_TEXT: {
		QCString str(val.toString().utf8().leftJustify(d->inda->sqlvar[para].sqllen, ' ', TRUE));
		memcpy(d->inda->sqlvar[para].sqldata, str.data(), d->inda->sqlvar[para].sqllen);
		break; }
	case SQL_BLOB:
		d->writeBlob(para, val.toByteArray());
		break;
	    default:
		break;
	    }
	}
    }

    if (colCount()) {
	isc_dsql_free_statement(d->status, &d->stmt, DSQL_close);
	if (d->isError("Unable to close statement"))
	    return FALSE;
	cleanup();
    }
    if (d->sqlda)
	init(d->sqlda->sqld);
    isc_dsql_execute2(d->status, &d->trans, &d->stmt, 1, d->inda, 0);
    if (d->isError("Unable to execute query"))
	return FALSE;

    setActive(TRUE);
    return TRUE;
}

bool QIBaseResult::reset (const QString& query)
{
//    qDebug("reset: %s", query.ascii());
    if (!driver() || !driver()->isOpen() || driver()->isOpenError())
	return FALSE;
    d->cleanup();
    setActive(FALSE);
    setAt(QSql::BeforeFirst);
    
    createDA(d->sqlda);

    if (!d->transaction())
	return FALSE;
    
    isc_dsql_allocate_statement(d->status, &d->ibase, &d->stmt);
    if (d->isError("Could not allocate statement", QSqlError::Statement))
	return FALSE;
    isc_dsql_prepare(d->status, &d->trans, &d->stmt, 0, query.utf8().data(), 3, d->sqlda);
    if (d->isError("Could not prepare statement", QSqlError::Statement))
	return FALSE;
    
    if (d->sqlda->sqld > d->sqlda->sqln) {
	// need more field descriptors
	int n = d->sqlda->sqld;
	free(d->sqlda);
	d->sqlda = (XSQLDA *) malloc(XSQLDA_LENGTH(n));
	d->sqlda->sqln = n;
	d->sqlda->version = SQLDA_VERSION1;

	isc_dsql_describe(d->status, &d->stmt, 1, d->sqlda);
	if (d->isError("Could not describe statement", QSqlError::Statement))
	    return FALSE;
    }
    
    initDA(d->sqlda);

    setSelect(d->isSelect());
    if (isSelect()) {
	init(d->sqlda->sqld);
    } else {
	free(d->sqlda);
	d->sqlda = 0;
    }
    
    isc_dsql_execute(d->status, &d->trans, &d->stmt, 1, 0);
    if (d->isError("Unable to execute query"))
	return FALSE;

    // commit non-select queries (if they are local)
    if (!isSelect() && !d->commit())
	return FALSE;
    
    setActive(TRUE);
    return TRUE;
}

bool QIBaseResult::gotoNext(QtSqlCachedResult::RowCache* row)
{
    ISC_STATUS stat = isc_dsql_fetch(d->status, &d->stmt, 1, d->sqlda);
    
    if (stat == 100) {
	// no more rows
	setAt(QSql::AfterLast);
	return FALSE;
    }
    if (d->isError("Could not fetch next item", QSqlError::Statement))
	return FALSE;
    if (!row) // not interested in actual values
	return TRUE;

    Q_ASSERT(row);
    Q_ASSERT((int)row->size() == d->sqlda->sqld);
    for (int i = 0; i < d->sqlda->sqld; ++i) {
	char *buf = d->sqlda->sqlvar[i].sqldata;
	int size = d->sqlda->sqlvar[i].sqllen;
	Q_ASSERT(buf);
	
	if ((d->sqlda->sqlvar[i].sqltype & 1) && *d->sqlda->sqlvar[i].sqlind) {
	    // null value
	    QVariant v;
	    v.cast(qIBaseTypeName2(d->sqlda->sqlvar[i].sqltype));
	    (*row)[i] = v;
	    continue;
	}
    
	switch(d->sqlda->sqlvar[i].sqltype & ~1) {
	case SQL_VARYING:
	    // pascal strings - a short with a length information followed by the data
	    (*row)[i] = QString::fromUtf8(buf + sizeof(short), *(short*)buf);
	    break;
	case SQL_INT64:
	    if (d->sqlda->sqlvar[i].sqlscale < 0)
		(*row)[i] = *(Q_LLONG*)buf * pow(10.0, d->sqlda->sqlvar[i].sqlscale);
	    else
		(*row)[i] = QVariant(*(Q_LLONG*)buf);
	    break;
	case SQL_LONG:
	    if (sizeof(int) == sizeof(long)) //dear compiler: please optimize me out.
		(*row)[i] = QVariant((int)(*(long*)buf));
	    else
		(*row)[i] = QVariant((Q_LLONG)(*(long*)buf));
	    break;
	case SQL_SHORT:
	    (*row)[i] = QVariant((int)(*(short*)buf));
	    break;
	case SQL_FLOAT:
	    (*row)[i] = QVariant((double)(*(float*)buf));	    
	    break;
	case SQL_DOUBLE:
	    (*row)[i] = QVariant(*(double*)buf);
	    break;
	case SQL_TIMESTAMP:
	    (*row)[i] = toQDateTime((ISC_TIMESTAMP*)buf);
            break;
	case SQL_TYPE_TIME:
	    (*row)[i] = toQTime(*(ISC_TIME*)buf);
	    break;
        case SQL_TYPE_DATE:
	    (*row)[i] = toQDate(*(ISC_DATE*)buf);
            break;
	case SQL_TEXT:
	    (*row)[i] = QString::fromUtf8(buf, size);
	    break;
	case SQL_BLOB:
	    (*row)[i] = d->fetchBlob((ISC_QUAD*)buf);
	    break;
	default:
	    // unknown type - don't even try to fetch
	    (*row)[i] = QVariant();
	    break;
	}	
    }

    return TRUE;    
}

int QIBaseResult::size()
{
    static char sizeInfo[] = {isc_info_sql_records};
    char buf[33];
    
    if (!isActive() || !isSelect())
	return -1;
    
    isc_dsql_sql_info(d->status, &d->stmt, sizeof(sizeInfo), sizeInfo, sizeof(buf), buf);
    for (char* c = buf + 3; *c != isc_info_end; /*nothing*/) {
	char ct = *c++;
	short len = isc_vax_integer(c, 2);
	c += 2;
	int val = isc_vax_integer(c, len);
	c += len;
	if (ct == isc_info_req_select_count)
	    return val;
    }
    return -1;
}

int QIBaseResult::numRowsAffected()
{
    static char acCountInfo[] = {isc_info_sql_records};
    char cCountType;

    switch (d->queryType) {
    case isc_info_sql_stmt_select:
	cCountType = isc_info_req_select_count;
	break;
    case isc_info_sql_stmt_update:
	cCountType = isc_info_req_update_count;
	break;
    case isc_info_sql_stmt_delete:
	cCountType = isc_info_req_delete_count;
	break;
    case isc_info_sql_stmt_insert:
	cCountType = isc_info_req_insert_count;
	break;
    }

    char acBuffer[33];
    int iResult = -1;    
    isc_dsql_sql_info(d->status, &d->stmt, sizeof(acCountInfo), acCountInfo, sizeof(acBuffer), acBuffer);
    if (d->isError("Could not get statement info", QSqlError::Statement))
	return -1;
    for (char *pcBuf = acBuffer + 3; *pcBuf != isc_info_end; /*nothing*/) {
	char cType = *pcBuf++;
	short sLength = isc_vax_integer (pcBuf, 2);
	pcBuf += 2;
	int iValue = isc_vax_integer (pcBuf, sLength);
	pcBuf += sLength;
		
	if (cType == cCountType) {
	    iResult = iValue;
	    break;
	}
    }
    return iResult;    
}

/*********************************/

QIBaseDriver::QIBaseDriver(QObject * parent, const char * name)
    : QSqlDriver(parent, name ? name : QIBASE_DRIVER_NAME)
{
    d = new QIBaseDriverPrivate(this);
}

QIBaseDriver::QIBaseDriver(void *connection, QObject *parent, const char *name)
    : QSqlDriver(parent, name ? name : QIBASE_DRIVER_NAME)
{
    d = new QIBaseDriverPrivate(this);
    d->ibase = (isc_db_handle)connection;
    setOpen(TRUE);
    setOpenError(FALSE);
}

QIBaseDriver::~QIBaseDriver()
{
    delete d;
}

bool QIBaseDriver::hasFeature(DriverFeature f) const
{
    switch (f) {
    case Transactions:
//    case QuerySize:
    case PreparedQueries:
    case PositionalPlaceholders:
    case Unicode:
    case BLOB:
        return TRUE;
    default:
        return FALSE;
    }
}

bool QIBaseDriver::open(const QString & db,
	  const QString & user,
	  const QString & password,
	  const QString & host,
	  int /*port*/,
	  const QString & /* connOpts */)
{
    if (isOpen())
        close();
    
    static const char enc[8] = "UTF_FSS";
    QCString usr = user.local8Bit();
    QCString pass = password.local8Bit();
    usr.truncate(255);
    pass.truncate(255);

    QByteArray ba(usr.length() + pass.length() + sizeof(enc) + 6);
    int i = -1;
    ba[++i] = isc_dpb_version1;
    ba[++i] = isc_dpb_user_name;
    ba[++i] = usr.length();
    memcpy(&ba[++i], usr.data(), usr.length());
    i += usr.length();
    ba[i] = isc_dpb_password;
    ba[++i] = pass.length();
    memcpy(&ba[++i], pass.data(), pass.length());
    i += pass.length();
    ba[i] = isc_dpb_lc_ctype;
    ba[++i] = sizeof(enc) - 1;
    memcpy(&ba[++i], enc, sizeof(enc) - 1);
    i += sizeof(enc) - 1;

    QString ldb;
    if (!host.isEmpty())
	ldb += host + ":";
    ldb += db;
    isc_attach_database(d->status, 0, (char*)ldb.latin1(), &d->ibase, i, ba.data());
    if (d->isError("Error opening database", QSqlError::Connection)) {
	setOpenError(TRUE);
	return FALSE;
    }
    
    setOpen(TRUE);
    return TRUE;
}

void QIBaseDriver::close()
{
    if (isOpen()) {
	isc_detach_database(d->status, &d->ibase);
	d->ibase = 0;
	setOpen(FALSE);
	setOpenError(FALSE);
    }
}

QSqlQuery QIBaseDriver::createQuery() const
{
    return QSqlQuery(new QIBaseResult(this));
}

bool QIBaseDriver::beginTransaction()
{
    if (!isOpen() || isOpenError())
	return FALSE;
    if (d->trans)
	return FALSE;
    
    isc_start_transaction(d->status, &d->trans, 1, &d->ibase, 0, NULL);    
    return !d->isError("Could not start transaction", QSqlError::Transaction);
}

bool QIBaseDriver::commitTransaction()
{
    if (!isOpen() || isOpenError())
	return FALSE;
    if (!d->trans)
	return FALSE;

    isc_commit_transaction(d->status, &d->trans);
    d->trans = 0;
    return !d->isError("Unable to commit transaction", QSqlError::Transaction);
}

bool QIBaseDriver::rollbackTransaction()
{
    if (!isOpen() || isOpenError())
	return FALSE;
    if (!d->trans)
	return FALSE;
    
    isc_rollback_transaction(d->status, &d->trans);
    d->trans = 0;
    return !d->isError("Unable to rollback transaction", QSqlError::Transaction);
}

QStringList QIBaseDriver::tables(const QString& typeName) const
{
    QStringList res;
    if (!isOpen())
	return res;

    int type = typeName.isEmpty() ? (int)QSql::Tables | (int)QSql::Views : typeName.toInt();
    QString typeFilter;

    if (type == (int)QSql::SystemTables) {
	typeFilter += "RDB$SYSTEM_FLAG != 0";
    } else if (type == ((int)QSql::SystemTables | (int)QSql::Views)) {
	typeFilter += "RDB$SYSTEM_FLAG != 0 OR RDB$VIEW_BLR NOT NULL";
    } else {
	if (!(type & (int)QSql::SystemTables))
	    typeFilter += "RDB$SYSTEM_FLAG = 0 AND ";
	if (!(type & (int)QSql::Views))
	    typeFilter += "RDB$VIEW_BLR IS NULL AND ";
	if (!(type & (int)QSql::Tables))
	    typeFilter += "RDB$VIEW_BLR IS NOT NULL AND ";
	if (!typeFilter.isEmpty())
	    typeFilter.truncate(typeFilter.length() - 5);
    }
    if (!typeFilter.isEmpty())
	typeFilter.prepend("where ");
    
    QSqlQuery q = createQuery();
    q.setForwardOnly(TRUE);
    if (!q.exec("select rdb$relation_name from rdb$relations " + typeFilter))
	return res;
    while(q.next())
	    res << q.value(0).toString().stripWhiteSpace();

    return res;
}

QSqlRecord QIBaseDriver::record(const QString& tablename) const
{
    QSqlRecord rec;
    if (!isOpen())
	return rec;
	
    QSqlQuery q = createQuery();
    q.setForwardOnly(TRUE);
    
    q.exec("SELECT a.RDB$FIELD_NAME, b.RDB$FIELD_TYPE "
	   "FROM RDB$RELATION_FIELDS a, RDB$FIELDS b "
	   "WHERE b.RDB$FIELD_NAME = a.RDB$FIELD_SOURCE "
	   "AND a.RDB$RELATION_NAME = '" + tablename.upper()+ "' "
	   "ORDER BY RDB$FIELD_POSITION");
    while (q.next()) {
	QSqlField field(q.value(0).toString().stripWhiteSpace(), qIBaseTypeName(q.value(1).toInt()));
	rec.append(field);
   }

    return rec;
}

QSqlRecordInfo QIBaseDriver::recordInfo(const QString& tablename) const
{
    QSqlRecordInfo rec;
    if (!isOpen())
	return rec;
	
    QSqlQuery q = createQuery();
    q.setForwardOnly(TRUE);

    q.exec("SELECT a.RDB$FIELD_NAME, b.RDB$FIELD_TYPE, b.RDB$FIELD_LENGTH, b.RDB$FIELD_SCALE, "
	   "b.RDB$FIELD_PRECISION, a.RDB$NULL_FLAG "
	   "FROM RDB$RELATION_FIELDS a, RDB$FIELDS b "
	   "WHERE b.RDB$FIELD_NAME = a.RDB$FIELD_SOURCE "
	   "AND a.RDB$RELATION_NAME = '" + tablename.upper() + "' "
	   "ORDER BY a.RDB$FIELD_POSITION");

    while (q.next()) {
	QVariant::Type type = qIBaseTypeName(q.value(1).toInt());
	QSqlFieldInfo field(q.value(0).toString().stripWhiteSpace(), type, q.value(5).toInt(),
			     q.value(2).toInt(), q.value(4).toInt(), QVariant());
		
	rec.append(field);
    }

    return rec;
}

QSqlIndex QIBaseDriver::primaryIndex(const QString &table) const
{
    QSqlIndex index(table);
    if (!isOpen())
	return index;

    QSqlQuery q = createQuery();
    q.setForwardOnly(TRUE);
    q.exec("SELECT a.RDB$INDEX_NAME, b.RDB$FIELD_NAME, d.RDB$FIELD_TYPE "
	   "FROM RDB$RELATION_CONSTRAINTS a, RDB$INDEX_SEGMENTS b, RDB$RELATION_FIELDS c, RDB$FIELDS d "
	   "WHERE a.RDB$CONSTRAINT_TYPE = 'PRIMARY KEY' "
	   "AND a.RDB$RELATION_NAME = '" + table.upper() + "' "
	   "AND a.RDB$INDEX_NAME = b.RDB$INDEX_NAME "
	   "AND c.RDB$RELATION_NAME = a.RDB$RELATION_NAME "
	   "AND c.RDB$FIELD_NAME = b.RDB$FIELD_NAME "
	   "AND d.RDB$FIELD_NAME = c.RDB$FIELD_SOURCE "
	   "ORDER BY b.RDB$FIELD_POSITION");

    while (q.next()) {
	QSqlField field(q.value(1).toString().stripWhiteSpace(), qIBaseTypeName(q.value(2).toInt()));
	index.append(field); //TODO: asc? desc?
	index.setName(q.value(0).toString());
    }

    return index;
}

QSqlRecord QIBaseDriver::record(const QSqlQuery& query) const
{
    QSqlRecord rec;
    if (query.isActive() && query.driver() == this) {
	QIBaseResult* result = (QIBaseResult*)query.result();
	if (!result->d->sqlda)
	    return rec;
	XSQLVAR v;
	for (int i = 0; i < result->d->sqlda->sqld; ++i) {
	    v = result->d->sqlda->sqlvar[i];
	    QSqlField f(QString::fromLatin1(v.sqlname, v.sqlname_length).stripWhiteSpace(),
			qIBaseTypeName2(result->d->sqlda->sqlvar[i].sqltype));
	    rec.append(f);
	}
    }
    return rec;
}

QSqlRecordInfo QIBaseDriver::recordInfo(const QSqlQuery& query) const
{
    QSqlRecordInfo rec;
    if (query.isActive() && query.driver() == this) {
	QIBaseResult* result = (QIBaseResult*)query.result();
	if (!result->d->sqlda)
	    return rec;
	XSQLVAR v;
	for (int i = 0; i < result->d->sqlda->sqld; ++i) {
	    v = result->d->sqlda->sqlvar[i];
	    QSqlFieldInfo f(QString::fromLatin1(v.sqlname, v.sqlname_length).stripWhiteSpace(), 
			    qIBaseTypeName2(result->d->sqlda->sqlvar[i].sqltype),
			    -1, v.sqllen, QABS(v.sqlscale), QVariant(), v.sqltype);
	    rec.append(f);
	}
    }
    return rec;
}

QString QIBaseDriver::formatValue(const QSqlField* field, bool trimStrings) const
{
    switch (field->type()) {
    case QVariant::DateTime: {
	QDateTime datetime = field->value().toDateTime();
	if (datetime.isValid())
	    return "'" + QString::number(datetime.date().year()) + "-" +
		QString::number(datetime.date().month()) + "-" +
		QString::number(datetime.date().day()) + " " +
		QString::number(datetime.time().hour()) + ":" +
		QString::number(datetime.time().minute()) + ":" +
		QString::number(datetime.time().second()) + "." +
		QString::number(datetime.time().msec()).rightJustify(3, '0', TRUE) + "'";
	else
	    return "NULL";
    }
    case QVariant::Time: {
	QTime time = field->value().toTime();
	if (time.isValid())
	    return "'" + QString::number(time.hour()) + ":" +
		QString::number(time.minute()) + ":" +
		QString::number(time.second()) + "." +
		QString::number(time.msec()).rightJustify(3, '0', TRUE) + "'";
	else
	    return "NULL";
    }
    case QVariant::Date: {
	QDate date = field->value().toDate();
	if (date.isValid())
	    return "'" + QString::number(date.year()) + "-" +
		QString::number(date.month()) + "-" +
		QString::number(date.day()) + "'";
	    else
		return "NULL";
    }
    default:
        return QSqlDriver::formatValue(field, trimStrings);
    }	
}
