#-------------------------------------------------
#
# Project created by QtCreator 2018-05-25T20:27:54
#
#-------------------------------------------------

CONFIG += c++11

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

RC_ICONS = icon.ico
TARGET = RPG-Paper-Maker
win32{
    TARGET = "RPG Paper Maker"
    RC_FILE = "RPG Paper Maker.rc"
}
TEMPLATE = app

INCLUDEPATH += \
    ../RPG-Paper-Maker/Editor/Singletons \
    ../RPG-Paper-Maker/Editor/Enums \
    ../RPG-Paper-Maker/Editor/Dialogs

SOURCES += \
    main.cpp \
    dialogengineupdate.cpp \
    widgetreleasenotes.cpp \
    engineupdater.cpp \
    ../RPG-Paper-Maker/Editor/Singletons/common.cpp \
    ../RPG-Paper-Maker/Editor/Dialogs/dialogprogress.cpp

HEADERS += \
    dialogengineupdate.h \
    widgetreleasenotes.h \
    engineupdater.h \
    ../RPG-Paper-Maker/Editor/Singletons/common.h \
    ../RPG-Paper-Maker/Editor/Enums/engineupdatefilekind.h \
    ../RPG-Paper-Maker/Editor/Dialogs/dialogprogress.h

FORMS += \
    dialogengineupdate.ui \
    ../RPG-Paper-Maker/Editor/Dialogs/dialogprogress.ui

#-------------------------------------------------
# Copy Content directory in build folder
#-------------------------------------------------

FROM = \"$$shell_path($$PWD\\Content)\"
DEST = \"$$shell_path($$OUT_PWD)\"
win32{
    CONFIG(debug, debug|release) {
        VARIANT = debug
    } else {
        VARIANT = release
    }
    DEST = \"$$shell_path($$OUT_PWD\\$$VARIANT\\Content)\"
    DESTDIR = $$OUT_PWD\\$$VARIANT
}

!equals(PWD, $${OUT_PWD}) {
    copyBR.commands = $(COPY_DIR) $$FROM $$DEST
    first.depends = $(first) copyBR
    export(first.depends)
    export(copyBR.commands)
    QMAKE_EXTRA_TARGETS += first copyBR
}
