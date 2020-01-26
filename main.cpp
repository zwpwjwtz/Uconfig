/*
 * Mainy entry for testing functions of Uconfig parser
 * See individual test file for more details.
 */

// ./test/testbasic.cpp
void testBasic();

// ./test/testparser.cpp
void testParser();

// ./test/testconversion.cpp
void testConversion();

int main(int argc, char *argv[])
{
    testBasic();
    testParser();
    testConversion();

    return 0;
}
