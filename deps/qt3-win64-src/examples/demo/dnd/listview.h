#include <qlistview.h>

class ListViewItem : public QListViewItem
{
public:
    ListViewItem ( QListView * parent, const QString& name, const QString& tag )
        : QListViewItem( parent, name ), _tag( tag ) {}
    ListViewItem ( QListView * parent, QListViewItem * after, const QString& name, const QString& tag )
        : QListViewItem( parent, after, name ), _tag( tag ) {}
    virtual ~ListViewItem() {}

    QString tag() { return _tag; }

private:
    QString _tag;
};

class ListView : public QListView
{
    Q_OBJECT

public:
    ListView( QWidget* parent = 0, const char* name = 0 );
    ~ListView();

    void dragEnterEvent( QDragEnterEvent * );
    void dropEvent( QDropEvent * );
    void contentsMousePressEvent( QMouseEvent * );
    void contentsMouseMoveEvent( QMouseEvent * );
    void contentsMouseReleaseEvent( QMouseEvent * );

private:
    QPoint pressPos;
    bool dragging;
};
