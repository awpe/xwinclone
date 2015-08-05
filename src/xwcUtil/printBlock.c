#include <xwcUtil.h>

void
printBlock (const char * text,
            const char * blPreIn,
            const char * lnPreIn,
            char       * extBuf,
            int          extBufLen)
{
    int charAtCutPos, maxLineLenCols, res, textLen, blckPreLenB;
    int lnPreLenB, extBufBCnt, lnPrefixEn, blckPrefixEn;
    int i, j, printedChars, newLine, line, lineAvailableTextCharCnt;
    int bytesToPrint, actuallyPrintedBytes, bytesFromText, cutPos, bufCutPos;
    char buf[256], blckPre[256], lnPre[256], format[256], formatFL[256];
    char ftmp[256];
    const char*t;

    textLen      = strlen (text);
    
    blckPreLenB  = blPreIn     != NULL ? strlen (blPreIn) : 0;
    lnPreLenB    = lnPreIn     != NULL ? strlen (lnPreIn) : 0;
    lnPrefixEn   = lnPreLenB   != 0;
    blckPrefixEn = blckPreLenB != 0;

    if (blckPreLenB > 128 || lnPreLenB > 128 || text == NULL || textLen == 0)
    {
        return;
    }

    extBufBCnt     = 0;
    maxLineLenCols = OUTPUT_MAX_COLUMNS;
    charAtCutPos   = 1;
    printedChars   = 0;
    newLine        = 0;
    line           = 0;

    if (blPreIn != NULL)
    {
        t = blPreIn;
        i = 0;
        while (*t != '\0')
        {
            if (*t == '\t')
            {
                for (j = 0; j < 8; ++ j)
                {
                    blckPre[i] = ' ';
                    i ++;
                }
            }
            else
            {
                blckPre[i] = * t;
                i ++;
            }
            t ++;
        }
        blckPre[i] = '\0';
        blckPreLenB = strlen (blckPre);
    }

    if (lnPreIn != NULL)
    {
        t = lnPreIn;
        i = 0;
        while (*t != '\0')
        {
            if (*t == '\t')
            {
                for (j = 0; j < 8; ++ j)
                {
                    lnPre[i] = ' ';
                    i ++;
                }
            }
            else
            {
                lnPre[i] = * t;
                i ++;
            }
            t ++;
        }
        lnPre[i] = '\0';
        lnPreLenB = strlen (lnPre);
    }

    if (lnPrefixEn && blckPrefixEn)
    {
        snprintf (formatFL, sizeof (formatFL), "%s%s%%s", blckPre, lnPre);
        snprintf (ftmp, sizeof (ftmp), "%%s%%%ds%%%%s", lnPreLenB);
        snprintf (format, sizeof (format), ftmp, blckPre, " ");
    }
    else if (! lnPrefixEn && blckPrefixEn)
    {
        snprintf (formatFL, sizeof (formatFL), "%s%%s", blckPre);
        snprintf (format, sizeof (format), "%s%%s", blckPre);
    }
    else if  (lnPrefixEn && ! blckPrefixEn)
    {
        snprintf (formatFL, sizeof (formatFL), "%s%%s", lnPre);
        snprintf (ftmp, sizeof (ftmp), "%%%ds%%%%s", lnPreLenB);
        snprintf (format, sizeof (format), ftmp, " ");
    }
    else
    {
        snprintf (formatFL, sizeof (formatFL), "%%s");
        snprintf (format, sizeof (format), "%%s");
    }

    while (printedChars < textLen && * (text + printedChars + 1) != '\0')
    {
        t = text + printedChars;

        while (*t == ' ')
        {
            printedChars ++;
            t ++;
        }

        t = text + printedChars;

        lineAvailableTextCharCnt = maxLineLenCols - blckPreLenB - lnPreLenB;

        bytesToPrint = 0;
        newLine = 0;
        while (   *t != '\0'
               && bytesToPrint <= lineAvailableTextCharCnt
               && bytesToPrint < textLen)
        {
            if (*t == '\n')
            {
                newLine = 1;
                break;
            }
            bytesToPrint ++ ;
            t ++;
        }

        if (bytesToPrint > lineAvailableTextCharCnt)
        {
            bytesToPrint = lineAvailableTextCharCnt;
        }

        bytesToPrint += blckPreLenB + lnPreLenB + 1;

        if (line == 0)
        {
            res = snprintf (buf, bytesToPrint, formatFL, text + printedChars);
        }
        else
        {
            res = snprintf (buf, bytesToPrint, format, text + printedChars);
        }

        if (res >= bytesToPrint)
        {
            actuallyPrintedBytes = bytesToPrint - 1;

            if (newLine == 0)
            {
                bytesFromText = actuallyPrintedBytes - lnPreLenB - blckPreLenB;

                for (i = bytesFromText - 1; i >= 0; -- i)
                {
                    t = text + printedChars + i;
                    if (   * t == '\n'
                        || * t == '\0'
                        || * t == '.'
                        || * t == ';'
                        || * t == ' '
                        || * t == '\t'
                        || * t == ','
                        || * t == ':'
                        || * t == '|')
                    {
                        break;
                    }
                }
                cutPos = i;
                if (cutPos != 0 && cutPos != bytesFromText)
                {
                    bufCutPos = cutPos + lnPreLenB + blckPreLenB;

                    buf[bufCutPos] = '\0';

                    actuallyPrintedBytes = bufCutPos + charAtCutPos;
                }
            }
        }
        else
        {
            actuallyPrintedBytes = res;
        }

        bytesFromText = actuallyPrintedBytes - lnPreLenB - blckPreLenB;
        printedChars += bytesFromText + newLine;

        if (extBuf == NULL || extBufLen == 0)
        {
            printf ("%s\n", buf);
        }
        else
        {
            snprintf (extBuf, extBufLen - extBufBCnt, "%s\n", buf );
            extBufBCnt += actuallyPrintedBytes + newLine;
            extBuf += actuallyPrintedBytes + newLine;
        }

        line ++;
    }
}