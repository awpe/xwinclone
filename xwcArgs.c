#include "xwc.h"

void
delArgs (arguments * args)
{
    if (args == NULL)
    {
        return;
    }

    if ( args->m_Args != NULL)
    {
        argument * arg;
        for (int i = 0; i < args->m_ArgCnt; ++ i)
        {
            if ((arg = args->m_Args[i]) != NULL)
            {
                if (arg->m_Value != NULL)
                {
                    if (arg->m_Type == INT)
                    {
                        free (arg->m_Value);
                    }

                    if (arg->m_Type == ULONG)
                    {
                        free (arg->m_Value);
                    }
                }

                if ( arg->m_SynStrs != NULL)
                {
                    free (arg->m_SynStrs);
                }

                free (arg);
            }
        }
        free (args->m_Args);
    }
    free (args);
}

arguments *
initArgs ()
{
    arguments * args = (arguments*) malloc (sizeof (arguments ));

    if (args == NULL)
    {
        return NULL;
    }

    args->m_ArgCnt = OPTIONS_COUNT;
    args->m_Args   = (argument**) malloc (sizeof (argument ) * args->m_ArgCnt);

    for (int i = 0; i < args->m_ArgCnt; ++ i)
    {
        args->m_Args[i] = (argument*) malloc (sizeof (argument ));
        memset (args->m_Args[i], 0, sizeof (*args->m_Args[i] ));
        args->m_Args[i]->m_NameStr  = NULL;
        args->m_Args[i]->m_Comment  = NULL;
        args->m_Args[i]->m_SynStrs  = NULL;
        args->m_Args[i]->m_Value    = NULL;
        args->m_Args[i]->m_HasValue = False;
    }

    return args;
}

Bool
addArg (arguments  * args,
        Bool         hasValue,
        argTypes     type,
        argNames     name,
        const char * nameStr,
        const char * comment,
        int          argSynCnt, ...)
{
    if (args == NULL)
    {
        return False;
    }

    va_list argStrList;

    va_start (argStrList, argSynCnt);

    argument * arg = args->m_Args[name];

    if (arg == NULL)
    {
        va_end (argStrList);
        return False;
    }

    arg->m_IsSet    = False;
    arg->m_SynCnt   = argSynCnt;
    arg->m_Type     = type;
    arg->m_Name     = name;
    arg->m_HasValue = hasValue;
    arg->m_NameStr  = nameStr;
    arg->m_Comment  = comment;
    arg->m_SynStrs  = (const char **) malloc (sizeof (const char*) * argSynCnt);

    if (arg->m_SynStrs == NULL)
    {
        va_end (argStrList);
        return False;
    }

    for (int i = 0; i < argSynCnt; ++ i)
    {
        arg->m_SynStrs[i] = va_arg (argStrList, const char *);
    }

    if (arg->m_HasValue == True)
    {
        switch (arg->m_Type)
        {
            case INT:
                arg->m_Value = (int*) malloc (sizeof (int ));
                break;

            case ULONG:
                arg->m_Value = (unsigned long*) malloc (sizeof (unsigned long));
                break;

            case C_STR:
                //arg->m_Value = (char**) malloc (sizeof (char*));
                break;

            default:
                logCtr ("Error adding argument, bad type specified!",
                        LOG_LVL_NO, False);
                free (arg->m_SynStrs);
                va_end (argStrList);
                return False;
        }
    }

    switch (arg->m_Name)
    {
        case HELP:
            break;

        case DAEMON:
            break;

        case SINGLEINST:
            break;

        case AUTOCENTER:
            *( (int*) arg->m_Value )           = AUTOCENTERING;
            break;

        case TOPOFFSET:
            *( (int*) arg->m_Value )           = TOP_OFFSET;
            break;

        case FOCUSTIME:
            *( (int*) arg->m_Value )           = TIME_TO_CHANGE_FOCUS_SEC;
            break;

        case BGCOLOR:
            arg->m_Value                       = (void*) DEFAULT_BG_COLOR;
            break;

        case LCKFPATH:
            arg->m_Value                       = (void*) LOCK_FILE_PATH;
            break;

        case BGIMAGE:
            arg->m_Value                       = (void*) DEFAULT_BGIMAGE_PATH;
            break;

        case FRAMERATE:
            *( (int*) arg->m_Value )           = FRAMERATE_FPS;
            break;

        case LOGLVL:
            *( (int*) arg->m_Value )           = DEFAULT_LOG_LVL;
            break;

        case SOURCEID:
            *( (unsigned long*) arg->m_Value ) = None;
            break;

        case PTRDEVNAME:
            arg->m_Value                       = (void*) TRACKED_PTR_NAME;
            break;

        case LOGFNAME:
            arg->m_Value                       = (void*) LOG_FILE_NAME;
            break;

        case RAISETIME:
            *( (int*) arg->m_Value )           = RAISE_SOURCE_DELAY;
            break;

        case RESTORETIME:
            *( (int*) arg->m_Value )           = RESTORE_SOURCE_DELAY;
            break;

        case CLICKTIME:
            *( (int*) arg->m_Value )           = BTN_CLICK_DELAY;
            break;

        case LONGWAIT:
            *( (int*) arg->m_Value )           = LONG_WAIT;
            break;

        default:
            logCtr ("Unknown argument type detected while creating option!",
                    LOG_LVL_NO, False);
            va_end (argStrList);
            return False;
    }

    va_end (argStrList);

    return True;
}

void
printCurValues (arguments  * args)
{
    printf ("\nCURRENT VALUES\n");

    printBlock ("default value shown if no corresponding prompt arg provided, "
                "you can specify option of interest with -h option and you "
                "will see how program understood it in this section",
                "\t",
                "Notice: ");

    printf ("\n\tValues:\n");

    for (int i = 0; i < args->m_ArgCnt; ++ i)
    {
        if (args->m_Args[i]->m_HasValue == True)
        {
            printf ("\t%-8s%-12s\t", " ", args->m_Args[i]->m_NameStr);

            if (args->m_Args[i]->m_IsSet == True)
            {
                printf ("%-12s\t", "is set");
            }
            else
            {
                printf ("%-12s\t", "default");
            }

            switch (args->m_Args[i]->m_Type)
            {

                case INT:
                    printf ("%d\n", *( (int*) args->m_Args[i]->m_Value ));
                    break;

                case C_STR:
                    printf ("%s\n", (const char*) args->m_Args[i]->m_Value );
                    break;

                case ULONG:
                    printf ("0x%lx\n",
                            *( (unsigned long*) args->m_Args[i]->m_Value ));
                    break;

                default:
                    logCtr ("Unknown argument type detected during arguments "
                            "list traversing!\n", LOG_LVL_NO, False);
                    break;
            }
        }
    }

    printf ("\n\tpress %s to exit program\n", EXIT_STR);

    printf ("\n\tuse %s combination for translation control\n\n",
            TRANSLATION_CTRL_STR);
}

void
printUsage (arguments  * args)
{
    /*FIXME: This is shit, normal formatted output function needed!!!*/
    int  printedChars, i;
    char buf[1024];

    memset (buf, 0, sizeof (buf));

    printedChars = 0;

    if (args == NULL)
    {
        return;
    }

    printf ("\nNAME\n");

    printBlock (PROGRAM_BRIEF, "\t", PROGRAM_EXE_NAME_STR" - ");

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

    printBlock (buf, "\t", "OPTIONS := { ");

    printf ("\nOPTIONS\n");

    for (int i = 0; i < args->m_ArgCnt; ++ i)
    {
        printf ("\t");
        for (int j = 0; j < args->m_Args[i]->m_SynCnt; ++ j)
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

        printBlock (comment, "\t\t", NULL);

        if (i + 1 < args->m_ArgCnt)
        {
            printf ("\n");
        }
    }
    
    printCurValues (args);
}
