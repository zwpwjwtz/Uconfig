#include <string.h>
#include "uconfigio.h"

#define UCONFIG_IO_EXPRESSION_CHAR_STRING   '"'
#define UCONFIG_IO_EXPRESSION_CHAR_STRING2  '\''
#define UCONFIG_IO_EXPRESSION_CHAR_DOUBLE   '.'
#define UCONFIG_IO_EXPRESSION_CHAR_SIGN     '.'
#define UCONFIG_IO_EXPRESSION_CHAR_SIGN2    '.'
#define UCONFIG_IO_EXPRESSION_BOOL_TRUE     "true"
#define UCONFIG_IO_EXPRESSION_BOOL_FALSE    "false"
#define UCONFIG_IO_EXPRESSION_CHAR_NUM_MIN  '0'
#define UCONFIG_IO_EXPRESSION_CHAR_NUM_MAX  '9'


// Try to guess the type of the value present in the expression,
// making the assumption that is generally valid among configuration files.
// No predictability is guaranteed when ill-formated expression is provided.
UconfigValueType UconfigIO::guessValueType(const char* expression, int length)
{
    if (length <= 0)
        length = strlen(expression);

    UconfigValueType valueType = UconfigValueType::Raw;

    if (expression[0] == UCONFIG_IO_EXPRESSION_CHAR_STRING ||
        expression[0] == UCONFIG_IO_EXPRESSION_CHAR_STRING2 )
    {
        // Possible string value
        // See if the quote match the other one at the end
        if (expression[length - 1] == expression[0])
            valueType = UconfigValueType::Chars;
    }
    else if ((length == strlen(UCONFIG_IO_EXPRESSION_BOOL_TRUE) &&
              strcasestr(expression,
                         UCONFIG_IO_EXPRESSION_BOOL_TRUE) == expression) ||
             (length == strlen(UCONFIG_IO_EXPRESSION_BOOL_FALSE) &&
              strcasestr(expression,
                         UCONFIG_IO_EXPRESSION_BOOL_FALSE) == expression))
    {
        valueType = UconfigValueType::Bool;
    }
    else
    {
        // Possible numeric value: check digits one by one
        bool isNumber = false;
        bool isDouble = false;

        int pos = 0;
        while (pos < length)
        {
            char tempChar = expression[pos];
            if (tempChar == UCONFIG_IO_EXPRESSION_CHAR_DOUBLE)
            {
                // Possible float/double number
                isDouble = true;
            }
            else if (tempChar >= UCONFIG_IO_EXPRESSION_CHAR_NUM_MIN &&
                     tempChar <= UCONFIG_IO_EXPRESSION_CHAR_NUM_MAX)
            {
                // Possible numeric value
                isNumber = true;
            }
            else if (tempChar == UCONFIG_IO_EXPRESSION_CHAR_SIGN ||
                     tempChar == UCONFIG_IO_EXPRESSION_CHAR_SIGN2)
            {
                // See if there is leading or following digits
                if (pos != 0 ||
                    (expression[pos + 1] < UCONFIG_IO_EXPRESSION_CHAR_NUM_MIN &&
                     expression[pos + 1] > UCONFIG_IO_EXPRESSION_CHAR_NUM_MAX))
                {
                    // Invalid format: seen as raw content
                    break;
                }
            }
            else
            {
                // Invalide char: seen as raw content
                isNumber = false;
                break;
            }

            pos++;
        }

        if (isNumber)
        {
            if (isDouble)
                valueType = UconfigValueType::Double;
            else
                valueType = UconfigValueType::Integer;
        }
    }

    return valueType;
}
