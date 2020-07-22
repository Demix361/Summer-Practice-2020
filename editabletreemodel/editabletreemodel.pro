QT += widgets
QT += sql
requires(qtConfig(treeview))

FORMS       = mainwindow.ui
HEADERS     = mainwindow.h \
              treeitem.h \
              treemodel.h
RESOURCES   =
SOURCES     = mainwindow.cpp \
              treeitem.cpp \
              treemodel.cpp \
              main.cpp

# install
target.path = $$[QT_INSTALL_EXAMPLES]/widgets/itemviews/editabletreemodel
INSTALLS += target
