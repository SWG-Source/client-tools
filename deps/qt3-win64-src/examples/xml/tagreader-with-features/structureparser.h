/*
$Id: structureparser.h 1439 2006-05-05 18:17:01Z chehrlic $
*/

#ifndef STRUCTUREPARSER_H
#define STRUCTUREPARSER_H

#include <qxml.h>
#include <qptrstack.h>

class QListView;
class QListViewItem;
class QString;

class StructureParser: public QXmlDefaultHandler
{
public:
    StructureParser( QListView * );
    bool startElement( const QString&, const QString&, const QString& ,
                       const QXmlAttributes& );
    bool endElement( const QString&, const QString&, const QString& );

    void setListView( QListView * );

private:
    QPtrStack<QListViewItem> stack;
    QListView * table;
};

#endif
