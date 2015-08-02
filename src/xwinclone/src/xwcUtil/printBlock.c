#include <xwcUtil.h>

void
printBlock (const char * str,
            const char * linePrefix,
            const char * blockPrefix,
            char       * extBuf,
            int          extBufSize)
{
    char bufLocal[1024], indentStr[1024];
    int  prefixLenCols, printedChars, tmp, i, extraCharCnt, blockPreLen;
    int  maxWordComplete, lineMaxLength, maxLen, strLen, prefixLenBytes;
    int  extBufPrintedChars;
    const char * tmpChar;

    extBufPrintedChars = 0;

    blockPreLen      = blockPrefix != NULL ? strlen (blockPrefix) : 1;
    prefixLenBytes   = strlen (linePrefix);
    prefixLenCols = prefixLenBytes;
    tmpChar          = linePrefix;
    while ((*tmpChar) != '\0')
    {
        if ((*tmpChar) == '\t')
        {
            prefixLenCols += 7;
        }

        tmpChar ++;
    }
    maxLen          = OUTPUT_MAX_COLUMNS - (blockPreLen + prefixLenCols) - 10;
    lineMaxLength   = OUTPUT_MAX_COLUMNS - 10;
    strLen          = strlen (str);
    maxWordComplete = 10;
    printedChars    = 0;

    snprintf (indentStr, sizeof (indentStr), "%%s%%%ds%%s", blockPreLen);

    i = 0;

    while (printedChars < strLen)
    {
        if (blockPrefix != NULL && i == 0)
        {
            tmp = snprintf (bufLocal, lineMaxLength, "%s%s%s", linePrefix,
                            blockPrefix, str);
        }
        else
        {
            if (*(str + printedChars) == ' ')
            {
                printedChars ++;
            }
            tmp = snprintf (bufLocal, lineMaxLength, indentStr, linePrefix, " ",
                            str + printedChars);
        }

        if (tmp > lineMaxLength)
        {
            printedChars += maxLen - 1;
            tmp = maxLen - 1 + blockPreLen + prefixLenBytes;
            extraCharCnt = 0;

            while (   (* (str + printedChars)) != '\0'
                   && (* (str + printedChars)) != ','
                   && (* (str + printedChars)) != ';'
                   && (* (str + printedChars)) != ' '
                   && (* (str + printedChars)) != '\t'
                   && (* (str + printedChars)) != '\n'
                   && extraCharCnt < maxWordComplete)
            {
                bufLocal[tmp + extraCharCnt] = * (str + printedChars);
                printedChars ++;
                extraCharCnt ++;
            }

            bufLocal[tmp + extraCharCnt] = * (str + printedChars);

            printedChars ++;

            extraCharCnt ++;

            bufLocal[tmp + extraCharCnt] = '\0';
        }
        else
        {
            printedChars += tmp - (blockPreLen + prefixLenBytes);
        }
        if (extBuf == NULL)
        {
            printf ("%s\n", bufLocal);
        }
        else
        {
            extBufPrintedChars += snprintf (extBuf + extBufPrintedChars,
                                            extBufSize, "%s\n", bufLocal);
        }
        i ++;
    }
}
