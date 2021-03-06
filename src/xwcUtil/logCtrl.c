/* Copyright (c) 2015, Anton Lantcevich
    
   This file is part of the xwinclone.
   <https://github.com/awpe/xwinclone>
  
   Xwinclone is released under the BSD 3-Clause License.
   You should have received a copy of the BSD 3-Clause
   License along with this software; if not see
   <https://raw.githubusercontent.com/awpe/xwinclone/master/LICENSE.md>.  */

#include <xwcUtil.h>

/*Global LOG_LVL initialization*/
int LOG_LVL = DEFAULT_LOG_LVL;

FILE * LOG_FILE = NULL;

void
logCtrl (const char * msg,
         int          lvl,
         Bool         sequenced)
{
    if (msg == NULL)
    {
        if (LOG_FILE != NULL)
        {
            fprintf (LOG_FILE, "Tryng to log NULL msg!\n");
        }
        else
        {
            printf ("Tryng to log NULL msg!\n");
        }
        return;
    }

    if (lvl < 0)
    {
        if (LOG_FILE != NULL)
        {
            fprintf (LOG_FILE,  "Tryng to log with unknown lvl!\n");
        }
        else
        {
            printf ( "Tryng to log with unknown lvl!\n");
        }
        return;
    }

    if (LOG_LVL >= lvl)
    {
        if (sequenced == False)
        {
            if (LOG_FILE != NULL)
            {
                fprintf (LOG_FILE, "\n%s\n", msg);
            }
            else
            {
                printf ( "\n%s\n", msg);
            }
        }
        else
        {
            if (LOG_FILE != NULL)
            {
                fprintf (LOG_FILE, "%s\n", msg);
            }
            else
            {
                printf ( "%s\n", msg);
            }
        }
    }
}
