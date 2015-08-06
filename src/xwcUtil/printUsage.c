/* Copyright (c) 2015, Anton Lantcevich
    
   This file is part of the xwinclone.
   <https://github.com/awpe/xwinclone>
  
   Xwinclone is released under the BSD 3-Clause License.
   You should have received a copy of the BSD 3-Clause
   License along with this software; if not see
   <https://github.com/awpe/xwinclone/blob/master/LICENSE.md>.  */

#include <xwcUtil.h>

void
printUsage (arguments  * args)
{
    /*FIXME: This is shit, nice output format function needed!!!*/
    int  printedChars, i, j;
    char buf[1024];

    memset (buf, 0, sizeof (buf));

    printedChars = 0;

    if (args == NULL)
    {
        return;
    }

    printf ("\nNAME\n");

    printBlock (PROGRAM_BRIEF, "\t", PROGRAM_EXE_NAME_STR" - ", NULL, 0);

    printf ("\nSYNOPSIS\n\t%s [OPTIONS]\n\n", PROGRAM_EXE_NAME_STR);

    printedChars = 0;

    for (i = 0; i < args->m_ArgCnt; ++ i)
    {
        printedChars += snprintf (buf + printedChars,
                                  sizeof (buf) - printedChars, "%s",
                                  args->m_Args[i]->m_SynStrs[0]);

        if (args->m_Args[i]->m_HasValue == True)
        {
            printedChars += snprintf (buf + printedChars,
                                      sizeof (buf) - printedChars, " %s",
                                      args->m_Args[i]->m_NameStr);
        }

        if (i + 1 < args->m_ArgCnt)
        {
            printedChars += snprintf (buf + printedChars,
                                      sizeof (buf) - printedChars, " | ");
        }
    }

    printedChars += snprintf (buf + printedChars, sizeof (buf) - printedChars,
                              " }");

    buf[printedChars] = '\0';

    printBlock (buf, "\t", "OPTIONS := { ", NULL, 0);

    printf ("\nOPTIONS\n");

    for (i = 0; i < args->m_ArgCnt; ++ i)
    {
        printf ("\t");
        for (j = 0; j < args->m_Args[i]->m_SynCnt; ++ j)
        {
            printf ("%s", args->m_Args[i]->m_SynStrs[j]);

            if (j + 1 < args->m_Args[i]->m_SynCnt)
            {
                printf (", ");
            }
        }

        if (args->m_Args[i]->m_HasValue == True)
        {
            printf (" %s", args->m_Args[i]->m_NameStr);
        }

        printf ("\n");

        const char * comment = args->m_Args[i]->m_Comment;

        printBlock (comment, "\t\t", NULL, NULL, 0);

        if (i + 1 < args->m_ArgCnt)
        {
            printf ("\n");
        }
    }

    printf ("\nDESCRIPTION\n");
    
    printBlock (PROGRAM_DESCRIPTION, "\t", NULL, NULL, 0);

}
