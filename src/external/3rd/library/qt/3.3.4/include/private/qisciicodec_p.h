#ifndef QISCIICODEC_H
#define QISCIICODEC_H

#ifndef QT_H
#include "qtextcodec.h"
#endif // QT_H

#ifndef QT_NO_CODECS

class QIsciiCodec : public QTextCodec {
public:
    QIsciiCodec(int i);

    virtual int         mibEnum() const;
    virtual const char* mimeName () const;
    const   char*       name() const;

#if !defined(Q_NO_USING_KEYWORD)
    using QTextCodec::fromUnicode;
#endif

    QCString fromUnicode(const QString& uc, int& len_in_out) const;
    QString  toUnicode(const char* chars, int len) const;

    int heuristicContentMatch(const char* chars, int len) const;
    int heuristicNameMatch(const char* hint) const;

private:
    int idx;
};

#endif // QT_NO_CODECS
#endif // QISCIIDEVCODEC_H
