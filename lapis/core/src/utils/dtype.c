#include "dtype.h"

lp_dtype detect_type(char *input)
{
    int hasDecimal = 0;

    for (int i = 0; input[i] != '\0'; i++)
    {
        if (i == 0 && (input[i] == '-' || input[i] == '+'))
        {
            continue;
        }
        if (input[i] == '.' && !hasDecimal)
        {
            hasDecimal = 1;
        }
        else if (!isdigit(input[i]))
        {
            return LP_STRING;
        }
    }

    if (hasDecimal)
    {
        return LP_FLOAT;
    }
    else
    {
        return LP_INT;
    }
}