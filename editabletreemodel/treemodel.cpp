#include "treemodel.h"
#include "treeitem.h"

#include <QtWidgets>
#include <QtSql>
#include <QDebug>


TreeModel::TreeModel(const QVector<QString> &headers, QObject *parent)
    : QAbstractItemModel(parent)
{
    QVector<QVariant> rootData;
    for (const QString &header : headers)
        rootData << header;

    rootItem = new TreeItem(rootData);
    //table = "adr_officers";
    table = "tab";
    setupModelData(rootItem);
}


TreeModel::~TreeModel()
{
    delete rootItem;
}


int TreeModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return rootItem->columnCount();
}


QVariant TreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    TreeItem *item = getItem(index);

    return item->data(index.column());
}


Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
}


TreeItem *TreeModel::getItem(const QModelIndex &index) const
{
    if (index.isValid()) {
        TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
        if (item)
            return item;
    }
    return rootItem;
}


QVariant TreeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();
}


QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() != 0)
        return QModelIndex();

    TreeItem *parentItem = getItem(parent);
    if (!parentItem)
        return QModelIndex();

    TreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    return QModelIndex();
}


bool TreeModel::insertColumns(int position, int columns, const QModelIndex &parent)
{
    beginInsertColumns(parent, position, position + columns - 1);
    const bool success = rootItem->insertColumns(position, columns);
    endInsertColumns();

    return success;
}


bool TreeModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    TreeItem *parentItem = getItem(parent);
    if (!parentItem)
        return false;

    QSqlQuery query("select max(id) from " + table);
    query.next();
    int new_id = query.value(0).toInt() + 1;

    int new_level;
    if (parentItem->data(3).toString() == "level")
        new_level = 0;
    else
        new_level = parentItem->data(3).toInt() + 1;

    int new_left;
    int new_right;
    if (new_level == 0) {
        new_left = 1;
        new_right = 2;
    }
    else {
        new_left = parentItem->data(1).toInt() + 1;
        new_right = new_left + 1;
    }


    beginInsertRows(parent, position, position + rows - 1);
    const bool success = parentItem->insertChildren(position, rows, rootItem->columnCount());
    endInsertRows();


    parentItem->child(position)->setData(0, QString::number(new_id));
    parentItem->child(position)->setData(1, QString::number(new_left));
    parentItem->child(position)->setData(2, QString::number(new_right));
    parentItem->child(position)->setData(3, QString::number(new_level));


    QString sql_str = "update " + table + " set tr_right = tr_right + 2 where tr_right >= " + QString::number(new_left);
    if (!query.exec(sql_str))
        qDebug() << "1 ins gg";


    sql_str = "update " + table + " set tr_left = tr_left + 2 where tr_left >= " + QString::number(new_left);
    if (!query.exec(sql_str))
        qDebug() << "2 ins gg";


    sql_str = "insert into " + table + " (id, tr_left, tr_right, level) "
              "values(" + QString::number(new_id) + ", " + QString::number(new_left) + ", " + QString::number(new_right) + ", " + QString::number(new_level) +  ")";
    if (!query.exec(sql_str))
        qDebug() << "3 ins gg";


    rootItem->f(rootItem);

    return success;
}


QModelIndex TreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    TreeItem *childItem = getItem(index);
    TreeItem *parentItem = childItem ? childItem->parent() : nullptr;

    if (parentItem == rootItem || !parentItem)
        return QModelIndex();

    return createIndex(parentItem->childNumber(), 0, parentItem);
}


bool TreeModel::removeColumns(int position, int columns, const QModelIndex &parent)
{
    beginRemoveColumns(parent, position, position + columns - 1);
    const bool success = rootItem->removeColumns(position, columns);
    endRemoveColumns();

    if (rootItem->columnCount() == 0)
        removeRows(0, rowCount());

    return success;
}

// ыыыыыыыыыы
bool TreeModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    TreeItem *parentItem = getItem(parent);
    if (!parentItem)
        return false;


    int lft = parentItem->child(position)->data(1).toInt();
    int rgt = parentItem->child(position)->data(2).toInt();
    int width = rgt - lft + 1;
    QSqlQuery query;

    QString sql_str = "delete from " + table + " where tr_left between " + QString::number(lft) + " and " + QString::number(rgt);
    if (!query.exec(sql_str))
        qDebug() << "1 del gg";

    sql_str = "update " + table + " set tr_right = tr_right - " + QString::number(width) + " where tr_right > " + QString::number(rgt);
    if (!query.exec(sql_str))
        qDebug() << "2 del gg";

    sql_str = "update " + table + " set tr_left = tr_left - " + QString::number(width) + " where tr_left > " + QString::number(rgt);
    if (!query.exec(sql_str))
        qDebug() << "3 del gg";



    beginRemoveRows(parent, position, position + rows - 1);
    const bool success = parentItem->removeChildren(position, rows);
    endRemoveRows();

    rootItem->f(rootItem);

    return success;
}


int TreeModel::rowCount(const QModelIndex &parent) const
{
    const TreeItem *parentItem = getItem(parent);

    return parentItem ? parentItem->childCount() : 0;
}


bool TreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole)
        return false;

    TreeItem *item = getItem(index);
    bool result = item->setData(index.column(), value);

    if (result)
        emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});


    QSqlQuery query;

    int id = item->data(0).toInt();
    QString sql_str = "select * from " + table + " where id = " + QString::number(id);
    if (!query.exec(sql_str))
        qDebug() << "1 setdata gg";

    query.next();

    int column = index.column();
    if (query.value(column).toString() != item->data(column).toString())
    {
        sql_str = "update " + table + " set " + rootItem->data(column).toString() + " = '" + item->data(column).toString() + "' where id = " + QString::number(id);
        if (!query.exec(sql_str))
            qDebug() << "2 setdata gg" << query.lastError();
    }

    return result;
}


bool TreeModel::setHeaderData(int section, Qt::Orientation orientation,
                              const QVariant &value, int role)
{
    if (role != Qt::EditRole || orientation != Qt::Horizontal)
        return false;

    const bool result = rootItem->setData(section, value);

    if (result)
        emit headerDataChanged(orientation, section, section);

    return result;
}


//ok
void TreeModel::setupModelData(TreeItem *parent)
{
    QVector<TreeItem*> parents;
    QVector<int> indentations;
    parents << parent;
    indentations << 0;

    QSqlQuery query("select * from " + table + " order by tr_left");
    int cols = query.record().count();
    qDebug() << cols;

    while (query.next()) {
        int position = query.value(3).toInt();

        QVector<QVariant> columnData;
        columnData.reserve(cols);
        for (int i = 0; i < cols; i++){
            columnData << query.value(i).toString();
        }


        if (position > indentations.last()) {
            if (parents.last()->childCount() > 0) {
                parents << parents.last()->child(parents.last()->childCount()-1);
                indentations << position;
            }
        } else {
            while (position < indentations.last() && parents.count() > 0) {
                parents.pop_back();
                indentations.pop_back();
            }
        }

        TreeItem *parent = parents.last();
        parent->insertChildren(parent->childCount(), 1, rootItem->columnCount());
        for (int column = 0; column < columnData.size(); ++column)
            parent->child(parent->childCount() - 1)->setData(column, columnData[column]);
    };
}
