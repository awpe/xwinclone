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
    printf ("\n\tCurrent values (default value if no corresponding prompt "
            "arg provided):\n\n");

    for (int i = 0; i < args->m_ArgCnt; ++ i)
    {
        if (args->m_Args[i]->m_HasValue == True)
        {
            if (args->m_Args[i]->m_IsSet == True)
            {
                printf ("\t\t%-10s\t%-10s\t", args->m_Args[i]->m_NameStr,
                        "is set");
            }
            else
            {
                printf ("\t\t%-10s\t%-10s\t", args->m_Args[i]->m_NameStr,
                        "default");
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
    if (args == NULL)
    {
        return;
    }

    printf ("\nUSAGE:\n\n\t%s ", PROGRAM_EXE_NAME_STR);

    for (int i = 0; i < args->m_ArgCnt; ++ i)
    {
        printf ("[");
        for (int j = 0; j < args->m_Args[i]->m_SynCnt; ++ j)
        {
            printf ("%s", args->m_Args[i]->m_SynStrs[j]);

            if (j + 1 < args->m_Args[i]->m_SynCnt)
            {
                printf (" | ");
            }
        }
        if (args->m_Args[i]->m_HasValue == True)
        {
            printf (" %s", args->m_Args[i]->m_NameStr);
        }
        printf ("] ");
    }

    printCurValues (args);
}
