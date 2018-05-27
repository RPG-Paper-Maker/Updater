#-------------------------------------------------
#
# Project created by QtCreator 2018-05-25T20:27:54
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Updater
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += \
    ../RPG-Paper-Maker/Singletons \
    ../RPG-Paper-Maker/Enums \
    ../RPG-Paper-Maker/Dialogs

SOURCES += \
    main.cpp \
    dialogengineupdate.cpp \
    widgetreleasenotes.cpp \
    engineupdater.cpp \
    ../RPG-Paper-Maker/Singletons/common.cpp \
    ../RPG-Paper-Maker/Dialogs/dialogprogress.cpp

HEADERS += \
    dialogengineupdate.h \
    widgetreleasenotes.h \
    engineupdater.h \
    ../RPG-Paper-Maker/Singletons/common.h \
    ../RPG-Paper-Maker/Enums/engineupdatefilekind.h \
    ../RPG-Paper-Maker/Dialogs/dialogprogress.h

FORMS += \
    dialogengineupdate.ui \
    ../RPG-Paper-Maker/Dialogs/dialogprogress.ui

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
