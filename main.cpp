/*
 * Mainy entry for testing functions of Uconfig parser
 * See individual test file for more details.
 */

#include <QApplication>
#include <QDesktopWidget>
#include "editor/uconfigeditor.h"


// ./test/testbasic.cpp
void testBasic();

// ./test/testparser.cpp
void testParser();

// ./test/testconversion.cpp
void testConversion();

void testEditor(int argc, char* argv[])
{
    QApplication a(argc, argv);

    UconfigEditor w;
    w.move((a.desktop()->width() - w.width()) / 2,
           (a.desktop()->height() - w.height()) / 2);
    w.show();

    a.exec();
}

int main(int argc, char *argv[])
{
    testBasic();
    testParser();
    testConversion();
    testEditor(argc, argv);

    return 0;
}
