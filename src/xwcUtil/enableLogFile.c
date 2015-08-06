/* Copyright (c) 2015, Anton Lantcevich
    
   This file is part of the xwinclone.
   <https://github.com/awpe/xwinclone>
  
   Xwinclone is released under the BSD 3-Clause License.
   You should have received a copy of the BSD 3-Clause
   License along with this software; if not see
   <https://raw.githubusercontent.com/awpe/xwinclone/master/LICENSE.md>.  */

#include <xwcUtil.h>

Bool
enableLogFile (XWCContext * ctx)
{
    Bool fileLengthMatch;
    int  i;
    char buf[1024];

    logCtrl ("\tEnabling LOG file", LOG_LVL_1, False);

    if (ctx == NULL)
    {
        logCtrl ("\t\tError enabling LOG file: NULL program context pointer "
                 "received", LOG_LVL_NO, False);
        return False;
    }

    LOG_FILE = NULL;

    if (strncmp (ctx->logFileName, "stdout", LOG_FILE_NAME_MAX_LENGTH)
        == STR_EQUAL)
    {
        LOG_FILE = stdout;
    }

    if (strncmp (ctx->logFileName, "stderr", LOG_FILE_NAME_MAX_LENGTH)
        == STR_EQUAL)
    {
        LOG_FILE = stderr;
    }

    if (LOG_FILE != stdout && LOG_FILE != stderr)
    {
        fileLengthMatch = True;

        for (i = 1; i < LOG_FILE_NAME_MAX_LENGTH; ++ i)
        {
            if (*ctx->logFileName == '\0')
            {
                fileLengthMatch = False;
                break;
            }
        }

        if (fileLengthMatch == False)
        {
            logCtrl ("\t\tFile name is too long", LOG_LVL_NO, False);
            return False;
        }

        LOG_FILE = fopen (ctx->logFileName, "a");

        if (LOG_FILE == NULL)
        {
            snprintf (buf, sizeof (buf), "\t\tError opening log file %s!",
                      ctx->logFileName);
            logCtrl (buf, LOG_LVL_NO, False);
            return False;
        }
    }

    logCtrl ("\t\tsuccess", LOG_LVL_2, True);

    return True;
}
