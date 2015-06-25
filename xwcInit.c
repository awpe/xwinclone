#include "xwc.h"

XWCContext *
init (int           argCnt,
      const char ** argArr)
{
    KeySym              exitKeySym;
    arguments         * args;
    char              * endPtr;
    int                 nextArgOffset, i, j, k, fr;
    unsigned long       srcid;
    char                buf[2048];
    XWCContext        * ctx;

    ctx = (XWCContext*) malloc (sizeof (XWCContext));

    if (ctx == NULL)
    {
        logCtr ("\tError allocating memory for options struct!", LOG_LVL_NO,
                True);
        return NULL;
    }

    memset (ctx, 0, sizeof (*ctx));

    ctx->bgColorStr      = NULL;
    ctx->bgImgFilePath   = NULL;
    ctx->cfgFPath        = NULL;
    ctx->exitKeyStr      = NULL;
    ctx->kbds            = NULL;
    ctx->lckFPath        = NULL;
    ctx->transCtrlKeyStr = NULL;
    ctx->xDpy            = NULL;
    ctx->xScr            = NULL;
    ctx->clMods          = NULL;
    ctx->exitMods        = NULL;

    /*Make a program to be portable to all locales*/
    setlocale (LC_ALL, "");

    if (( ctx->xDpy = openDefaultDisplay () )  == NULL)
    {
        freeXWCContext (ctx);
        return NULL;
    }

    if (chkCompExt (ctx->xDpy) == False)
    {
        XCloseDisplay (ctx->xDpy);
        freeXWCContext (ctx);
        return NULL;
    }

    if (chckXI2Ext (ctx) == False)
    {
        XCloseDisplay (ctx->xDpy);
        freeXWCContext (ctx);
        return NULL;
    }

    if (getMasterDevsList (ctx, XIMasterKeyboard) == False)
    {
        XCloseDisplay (ctx->xDpy);
        freeXWCContext (ctx);
        return NULL;
    }

    if ((ctx->rootW = getDefaultRootWindow (ctx->xDpy)) == None)
    {
        XCloseDisplay (ctx->xDpy);
        freeXWCContext (ctx);
        return NULL;
    }

    XGetWindowAttributes (ctx->xDpy, ctx->rootW, &ctx->rootWAttr);

    if (getXErrState () == True)
    {
        XCloseDisplay (ctx->xDpy);
        freeXWCContext (ctx);
        return NULL;
    }

    if (( ctx->xScr = getScreenByWindowAttr (ctx, &ctx->rootWAttr) ) == NULL)
    {
        XCloseDisplay (ctx->xDpy);
        freeXWCContext (ctx);
        return NULL;
    }

    logCtr ("Processing arguments and building configuration:", LOG_LVL_1,
            False);

    args = initArgs ();

    if (args == NULL)
    {
        logCtr ("\tCannot allocate structure to process arguments!",
                LOG_LVL_NO, True);
        XCloseDisplay (ctx->xDpy);
        freeXWCContext (ctx);
        return NULL;
    }

    if (   addArg (args, True,  C_STR, BGCOLOR,    "BGCOLOR",      1, "-bgclr")
        == False

        || addArg (args, True,  C_STR, BGIMAGE,    "BGIMAGE",      1, "-bgimg")
        == False

        || addArg (args, True,  INT,   FRAMERATE,  "FRAMERATE",    1, "-fr"   )
        == False

        || addArg (args, True,  INT,   FOCUSTIME,  "FOCUSTIME",    1, "-ft"   )
        == False

        || addArg (args, True,  INT,   TOPOFFSET,  "TOPOFFSET",    1, "-toff" )
        == False

        || addArg (args, False, C_STR, HELP,       "HELP",         3, "-h",
                   "-help",
                   "--help")
        == False

        || addArg (args, True,  INT,   AUTOCENTER, "AUTOCENTER",   1, "-ac"   )
        == False

        || addArg (args, True,  INT,   LOGLVL,     "LOGLEVEL",     1, "-ll"   )
        == False

        || addArg (args, True,  ULONG, SOURCEID,   "SOURCEID",     1, "-srcid")
        == False

        || addArg (args, True,  C_STR, LCKFPATH,   "LOCKFILEPATH", 1, "-lckf")
        == False

        || addArg (args, False, INT,   SINGLEINST, "SINGLEINST",   2, "-si",
                   "-single")
        == False

        || addArg (args, False, INT,   DAEMON,     "DAEMON",       2, "-d",
                   "-daemon")
        == False
        )
    {
        delArgs (args);
        logCtr ("\tCannot add new argument to list!\n", LOG_LVL_NO, True);
        XCloseDisplay (ctx->xDpy);
        freeXWCContext (ctx);
        delArgs (args);
        return NULL;
    }

    nextArgOffset = 2;

    for (i = 1; i < argCnt; i += nextArgOffset)
    {
        Bool argFound = False;
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
                        logCtr (buf, LOG_LVL_NO, True);
                        XCloseDisplay (ctx->xDpy);
                        freeXWCContext (ctx);
                        delArgs (args);
                        return NULL;
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
                            *( (int*) args->m_Args[j]->m_Value ) =
                                strtol (argArr[i + 1], &endPtr, 10);

                            if (endPtr == argArr[i + 1])
                            {
                                snprintf (buf, sizeof (buf), "\tError parsing "
                                          "%s argument!\n\n\t\tTry:\n\n\t%s "
                                          "[-help | -h | --help]\n",
                                          args->m_Args[j]->m_NameStr,
                                          PROGRAM_EXE_NAME_STR);
                                logCtr (buf, LOG_LVL_NO, True);
                                XCloseDisplay (ctx->xDpy);
                                freeXWCContext (ctx);
                                delArgs (args);
                                return NULL;
                            }
                            break;

                        case ULONG:
                            *( (unsigned long*) args->m_Args[j]->m_Value ) =
                                strtol (argArr[i + 1], &endPtr, 0);

                            if (endPtr == argArr[i + 1])
                            {
                                snprintf (buf, sizeof (buf), "\tError parsing "
                                          "%s argument!\n\nTry:\n\n\t%s [-help "
                                          "| -h | --help]\n",
                                          args->m_Args[j]->m_NameStr,
                                          PROGRAM_EXE_NAME_STR);
                                logCtr (buf, LOG_LVL_NO, True);
                                XCloseDisplay (ctx->xDpy);
                                freeXWCContext (ctx);
                                delArgs (args);
                                return NULL;
                            }
                            break;

                        case C_STR:
                            args->m_Args[j]->m_Value = (void*) argArr[i + 1];
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
            logCtr (buf, LOG_LVL_NO, True);
            XCloseDisplay (ctx->xDpy);
            freeXWCContext (ctx);
            delArgs (args);
            return NULL;
        }
        else
        {
            nextArgOffset = ( args->m_Args[j]->m_HasValue == True ) ? 2 : 1;
        }
    }

    /*boilerplate wa*/
    LOG_LVL = * ((int*)           args->m_Args[LOGLVL]->m_Value);
    fr      = * ((int*)           args->m_Args[FRAMERATE]->m_Value);
    srcid   = * ((unsigned long*) args->m_Args[SOURCEID]->m_Value);

    memset (&ctx->bgColor, 0, sizeof (ctx->bgColor ));

    ctx->focusDelay.tv_nsec     = 0;
    ctx->focusDelay.tv_sec      = * ((int*) args->m_Args[FOCUSTIME]->m_Value);
    ctx->frameDelay.tv_nsec     = ( 1.00000001 / fr) * 1000000000L;
    ctx->frameDelay.tv_sec      = 0;
    ctx->longDelay.tv_sec       = 0;
    ctx->longDelay.tv_nsec      = LONG_WAIT;
    ctx->autoCenter             = * ((int*) args->m_Args[AUTOCENTER]->m_Value);
    ctx->topOffset              = * ((int*) args->m_Args[TOPOFFSET]->m_Value);
    ctx->bgColorStr             = (const char*) args->m_Args[BGCOLOR]->m_Value;
    ctx->bgImgFilePath          = (const char*) args->m_Args[BGIMAGE]->m_Value;
    ctx->bgImgFileSet           = args->m_Args[BGIMAGE]->m_IsSet;
    ctx->bgImgStatus            = False;
    ctx->exitKeyStr             = EXIT_KEY;
    ctx->exitKeyMask            = EXIT_MASK;
    ctx->transCtrlKeyStr        = TRANSLATION_CTRL_KEY;
    ctx->cloneKeyMask           = TRANSLATION_CTRL_MASK;
    ctx->srcW                   = srcid;
    ctx->isDaemon               = args->m_Args[DAEMON]->m_IsSet;
    ctx->lckFPath               = args->m_Args[LCKFPATH]->m_Value;
    ctx->isSingleton            = args->m_Args[SINGLEINST]->m_IsSet;
    ctx->clickDelay.tv_sec      = 0;
    ctx->clickDelay.tv_nsec     = MOUSE_BTN_DELAY;

    ctx->isSingleton            = ctx->isSingleton || ctx->isDaemon;

    if (( exitKeySym = XStringToKeysym (ctx->exitKeyStr) ) == NoSymbol)
    {
        snprintf (buf, sizeof (buf), "\tError parsing exit key string (%s)",
                  ctx->exitKeyStr);
        logCtr (buf, LOG_LVL_NO, True);
        XCloseDisplay (ctx->xDpy);
        freeXWCContext (ctx);
        delArgs (args);
        return NULL;
    }

    if ((ctx->exitKeyCode = XKeysymToKeycode (ctx->xDpy, exitKeySym)) == 0)
    {
        snprintf (buf, sizeof (buf), "\tUnknown keycode %d", ctx->exitKeyCode);
        logCtr (buf, LOG_LVL_NO, True);
        XCloseDisplay (ctx->xDpy);
        freeXWCContext (ctx);
        delArgs (args);
        return NULL;
    }

    if (( exitKeySym = XStringToKeysym (ctx->transCtrlKeyStr) )
        == NoSymbol)
    {
        snprintf (buf, sizeof (buf), "\tError parsing exit key string (%s)",
                  ctx->transCtrlKeyStr);
        logCtr (buf, LOG_LVL_NO, True);
        XCloseDisplay (ctx->xDpy);
        freeXWCContext (ctx);
        delArgs (args);
        return NULL;
    }

    if (( ctx->cloneKeyCode = XKeysymToKeycode (ctx->xDpy, exitKeySym) ) == 0)
    {
        snprintf (buf, sizeof (buf), "\tUnknown keycode %d",
                  ctx->cloneKeyCode);
        logCtr (buf, LOG_LVL_NO, True);
        XCloseDisplay (ctx->xDpy);
        freeXWCContext (ctx);
        delArgs (args);
        return NULL;
    }

    if (args->m_Args[HELP]->m_IsSet == True)
    {
        printUsage (args);
        XCloseDisplay (ctx->xDpy);
        freeXWCContext (ctx);
        delArgs (args);
        return NULL;
    }

    if (ctx->isSingleton == True && ifSingleInst (ctx) == False)
    {
        XCloseDisplay (ctx->xDpy);
        freeXWCContext (ctx);
        delArgs (args);
        return NULL;
    }

    if (parseColor (ctx) == False)
    {
        XCloseDisplay (ctx->xDpy);
        freeXWCContext (ctx);
        delArgs (args);
        return NULL;
    }

    if ((ctx->keysGrabbed = grabAllKeys (ctx)) == False)
    {
        XCloseDisplay (ctx->xDpy);
        freeXWCContext (ctx);
        delArgs (args);
        return NULL;
    }

    if (LOG_LVL > LOG_LVL_NO)
    {
        printCurValues (args);
    }

    logCtr ("\tconfiguration successfully completed", LOG_LVL_1, True);

    delArgs (args);
    return ctx;
}

void
freeXWCContext (XWCContext * ctx)
{
    if (ctx->clMods != NULL)
    {
        free (ctx->clMods);
        free (ctx->exitMods);
    }
    
    
    if (ctx->kbds != NULL)
    {
        printf ("there are keyboards\n");
        if (ctx->kbds->devs != NULL)
        {
            printf ("there are devs\n");
            free (ctx->kbds->devs);
        }
        free (ctx->kbds);
    }
    free (ctx);
}
