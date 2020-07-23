#include "treeitem.h"
#include <QDebug>
#include <QtSql>


TreeItem::TreeItem(const QVector<QVariant> &data, TreeItem *parent)
    : itemData(data),
      parentItem(parent)
{
    //table = "adr_officers";
    table = "tab";
}



TreeItem::~TreeItem()
{
    qDeleteAll(childItems);
}




// Обновляет данные в дереве из бд
void TreeItem::f(TreeItem *parent)
{
    int cnt = parent->childCount();

    if (parent->data(0).toString() != "id")
    {
        QSqlQuery query;
        int id = parent->data(0).toInt();

        QString sql_str = "select * from " + table + " where id = " + QString::number(id);
        if (!query.exec(sql_str))
            qDebug() << "aaaaaaaaaaaaaa";
        else
        {
            query.next();

            int left = query.value(1).toInt();
            int right = query.value(2).toInt();
            int level = query.value(3).toInt();

            parent->setData(0, id);
            parent->setData(1, left);
            parent->setData(2, right);
            parent->setData(3, level);
        }

    }

    if (parent->childCount() > 0)
        for (int i = 0; i < cnt; i++)
        {
            f(parent->child(i));
        }
}







TreeItem *TreeItem::child(int number)
{
    if (number < 0 || number >= childItems.size())
        return nullptr;
    return childItems.at(number);
}


//! [3]
int TreeItem::childCount() const
{
    return childItems.count();
}
//! [3]

//! [4]
int TreeItem::childNumber() const
{
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<TreeItem*>(this));
    return 0;
}
//! [4]

//! [5]
int TreeItem::columnCount() const
{
    return itemData.count();
}
//! [5]

//! [6]
QVariant TreeItem::data(int column) const
{
    if (column < 0 || column >= itemData.size())
        return QVariant();
    return itemData.at(column);
}
//! [6]

//! [7]
bool TreeItem::insertChildren(int position, int count, int columns)
{
    if (position < 0 || position > childItems.size())
        return false;

    for (int row = 0; row < count; ++row) {
        QVector<QVariant> data(columns);
        TreeItem *item = new TreeItem(data, this);
        childItems.insert(position, item);
    }

    return true;
}
//! [7]

//! [8]
bool TreeItem::insertColumns(int position, int columns)
{
    if (position < 0 || position > itemData.size())
        return false;

    for (int column = 0; column < columns; ++column)
        itemData.insert(position, QVariant());

    for (TreeItem *child : qAsConst(childItems))
        child->insertColumns(position, columns);

    return true;
}
//! [8]

//! [9]
TreeItem *TreeItem::parent()
{
    return parentItem;
}
//! [9]

//! [10]
bool TreeItem::removeChildren(int position, int count)
{
    if (position < 0 || position + count > childItems.size())
        return false;

    for (int row = 0; row < count; ++row)
        delete childItems.takeAt(position);

    return true;
}
//! [10]

bool TreeItem::removeColumns(int position, int columns)
{
    if (position < 0 || position + columns > itemData.size())
        return false;

    for (int column = 0; column < columns; ++column)
        itemData.remove(position);

    for (TreeItem *child : qAsConst(childItems))
        child->removeColumns(position, columns);

    return true;
}

//! [11]
bool TreeItem::setData(int column, const QVariant &value)
{
    if (column < 0 || column >= itemData.size())
        return false;

    itemData[column] = value;

    /*
    QSqlQuery query;

    int id = parent()->child(this->childNumber())->data(0).toInt();
    QString sql_str = "select * from " + table + " where id = " + QString::number(id);
    if (!query.exec(sql_str))
        qDebug() << "1 setdata gg";

    query.next();
    qDebug() << query.isNull(0);
    qDebug() << query.size() << "\n";
    */

    /*
    if (query.value(column).toString() != parent()->child(this->childNumber())->data(column).toString())
    {
        qDebug() << "tree:" << parent()->child(this->childNumber())->data(column).toString();
        qDebug() << "db:" << query.value(column).toString();
        qDebug() << "column:" << column;
    }
    */

    return true;
}
//! [11]
