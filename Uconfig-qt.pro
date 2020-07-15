#-------------------------------------------------
#
# Project created by QtCreator 2018-01-26T16:28:25
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Uconfig
TEMPLATE = app

VER_MAJ = 1
VER_MIN = 0
VER_PAT = 0
VERSION = 1.0.0

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS APP_VERSION=\\\"$$VERSION\\\"

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp \
    parser/uconfigfile.cpp \
    parser/uconfigentryobject.cpp \
    parser/uconfigini.cpp \
    parser/utils.cpp \
    parser/uconfig2dtable.cpp \
    parser/uconfigkeyvalue.cpp \
    parser/uconfigjson.cpp \
    parser/uconfigxml.cpp \
    parser/uconfigio.cpp \
    test/testbasic.cpp \
    test/testparser.cpp \
    test/testconversion.cpp \
    editor/uconfigeditor.cpp \
    parser/uconfigcsv.cpp \
    editor/qhexedit2/commands.cpp \
    editor/qhexedit2/qhexedit.cpp \
    editor/qhexedit2/chunks.cpp \
    editor/hexeditdialog.cpp

HEADERS  += \
    parser/uconfigentry.h \
    parser/uconfigfile.h \
    parser/uconfigentryobject.h \
    parser/uconfigini.h \
    parser/uconfigio.h \
    parser/uconfigfile_metadata.h \
    parser/utils.h \
    parser/uconfig2dtable.h \
    parser/uconfigkeyvalue.h \
    parser/uconfigkeyvalue_p.h \
    parser/uconfigini_p.h \
    parser/uconfigjson.h \
    parser/uconfigjson_p.h \
    parser/uconfigxml.h \
    parser/uconfigxml_p.h \
    editor/uconfigeditor.h \
    parser/uconfigcsv.h \
    editor/qhexedit2/qhexedit.h \
    editor/qhexedit2/commands.h \
    editor/qhexedit2/chunks.h \
    editor/hexeditdialog.h

target.path = $${PREFIX}/bin/

INSTALLS += target

FORMS += \
    editor/uconfigeditor.ui \
    editor/hexeditdialog.ui

DISTFILES += \
    editor/qhexedit2/license.txt

RESOURCES += \
    editor/icons.qrc
