/* Copyright (c) 2015, Anton Lantcevich
    
   This file is part of the xwinclone.
   <https://github.com/awpe/xwinclone>
  
   Xwinclone is released under the BSD 3-Clause License.
   You should have received a copy of the BSD 3-Clause
   License along with this software; if not see
   <https://github.com/awpe/xwinclone/blob/master/LICENSE.md>.  */

#include <xwcArgs.h>
#include <xwcHeaders.h>

Bool
parseArgs (const char ** argArr,
           arguments  *  args,
           int           argCnt)
{
    int    nextArgOffset, i, j, k;
    char * endPtr, buf[2048];
    Bool   argFound;

    if (argArr == NULL)
    {
        logCtrl ("Cannot parse arguments: NULL pointer to user input array!",
                 LOG_LVL_NO, False);
        return False;
    }

    if (args == NULL)
    {
        logCtrl ("Cannot parse arguments: NULL pointer to arguments array!",
                 LOG_LVL_NO, False);
        return False;
    }

    if (argCnt < 1 || argCnt > (1 + OPTIONS_COUNT * 2))
    {
        logCtrl ("Cannot parse arguments: Bad argument count!",
                 LOG_LVL_NO, False);
        return False;
    }

    nextArgOffset = 2;

    for (i = 1; i < argCnt; i += nextArgOffset)
    {
        argFound = False;
        for (j = 0; j < OPTIONS_COUNT; ++ j)
        {
            for (k = 0; k < args->m_Args[j]->m_SynCnt; ++ k)
            {
                if (strcmp (argArr[i], args->m_Args[j]->m_SynStrs[k])
                    == STR_EQUAL)
                {
                    if (args->m_Args[j]->m_IsSet == True)
                    {
                        snprintf (buf, sizeof (buf), "Parameter %s has been "
                                  "set several times!",
                                  args->m_Args[j]->m_NameStr);
                        logCtrl (buf, LOG_LVL_NO, True);
                        return False;
                    }

                    args->m_Args[j]->m_IsSet = True;
                    argFound                 = True;

                    if ( args->m_Args[j]->m_HasValue == False)
                    {
                        break;
                    }
                    switch (args->m_Args[j]->m_Type)
                    {
                        case INT:
                            *( (int*) args->m_Args[j]->m_Val ) =
                                strtol (argArr[i + 1], &endPtr, 10);

                            if (endPtr == argArr[i + 1])
                            {
                                snprintf (buf, sizeof (buf), "\tError parsing "
                                          "%s argument!\n\n\t\tTry:\n\n\t%s "
                                          "[-help | -h | --help]\n",
                                          args->m_Args[j]->m_NameStr,
                                          PROGRAM_EXE_NAME_STR);
                                logCtrl (buf, LOG_LVL_NO, True);
                                return False;
                            }
                            break;

                        case ULONG:
                            *( (unsigned long*) args->m_Args[j]->m_Val ) =
                                strtol (argArr[i + 1], &endPtr, 0);

                            if (endPtr == argArr[i + 1])
                            {
                                snprintf (buf, sizeof (buf), "\tError parsing "
                                          "%s argument!\n\nTry:\n\n\t%s [-help "
                                          "| -h | --help]\n",
                                          args->m_Args[j]->m_NameStr,
                                          PROGRAM_EXE_NAME_STR);
                                logCtrl (buf, LOG_LVL_NO, True);
                                return False;
                            }
                            break;

                        case C_STR:
                            args->m_Args[j]->m_Val = (void*) argArr[i + 1];
                            break;

                        default:
                            break;
                    }
                    break;
                }
            }
            if (argFound == True)
            {
                break;
            }
        }
        if (argFound == False)
        {
            snprintf (buf, sizeof (buf), "\tUnknown argument specified!\n\nTry:"
                      "\n\n\t%s [-help | -h | --help]\n", PROGRAM_EXE_NAME_STR);
            logCtrl (buf, LOG_LVL_NO, True);
            return False;
        }
        else
        {
            nextArgOffset = ( args->m_Args[j]->m_HasValue == True ) ? 2 : 1;
        }
    }

    return True;
}
