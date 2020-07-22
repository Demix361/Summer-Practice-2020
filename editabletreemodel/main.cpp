#include "mainwindow.h"

#include <QApplication>
#include <QtSql>
#include <QDebug>


int main(int argc, char *argv[])
{
    //Q_INIT_RESOURCE(editabletreemodel);

    QSqlDatabase sdb = QSqlDatabase::addDatabase("QSQLITE");
    sdb.setDatabaseName("testdb.db");
    if (!sdb.open()) {
        qDebug() << sdb.lastError().text();
    }

    QApplication app(argc, argv);
    MainWindow window;
    window.show();
    return app.exec();
}
