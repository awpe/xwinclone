#include <xwcInit.h>
#include <xwcHeaders.h>

XWCContext *
init (int           argCnt,
      const char ** argArr)
{
    arguments         * args;
    int                 fr, raiseT, clickT, resT, longWait, focusTime;
    unsigned long       srcid;
    const char        * ptrDevName;
    XWCContext        * ctx;

    /*Make a program to be portable to all locales*/
    setlocale (LC_ALL, "");

    logCtrl ("Processing arguments and building configuration", LOG_LVL_1,
             False);

    ctx = (XWCContext*) malloc (sizeof (XWCContext));

    if (ctx == NULL)
    {
        logCtrl ("\tError allocating memory for options struct!", LOG_LVL_NO,
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

    args = initArgs ();

    if (args == NULL)
    {
        logCtrl ("\tCannot allocate structure to process arguments!",
                 LOG_LVL_NO, True);
        freeXWCContext (ctx);
        return NULL;
    }

    if (populateArgs (args) == False)
    {
        logCtrl ("\tCannot add argument to list!\n", LOG_LVL_NO, True);
        freeXWCContext (ctx);
        delArgs (args);
        return NULL;
    }

    if (parseArgs (argArr, args, argCnt) == False)
    {
        freeXWCContext (ctx);
        delArgs (args);
        return NULL;
    }

    /*boilerplate wa*/
    LOG_LVL     = * ((int*)           args->m_Args[LOGLVL]->m_Value);
    fr          = * ((int*)           args->m_Args[FRAMERATE]->m_Value);
    srcid       = * ((unsigned long*) args->m_Args[SOURCEID]->m_Value);
    ptrDevName  =   (const char*)     args->m_Args[PTRDEVNAME]->m_Value;
    raiseT      = * ((int*)           args->m_Args[RAISETIME]->m_Value);
    clickT      = * ((int*)           args->m_Args[CLICKTIME]->m_Value);
    resT        = * ((int*)           args->m_Args[RESTORETIME]->m_Value);
    longWait    = * ((int*)           args->m_Args[LONGWAIT]->m_Value);
    focusTime   = * ((int*)           args->m_Args[FOCUSTIME]->m_Value);

    if (longWait == 0)
    {
        longWait = LONG_WAIT;
    }

    memset (&ctx->bgColor, 0, sizeof (ctx->bgColor ));

    ctx->focusDelay.tv_sec    = focusTime == 0 ? 0 : focusTime / 1000;
    ctx->focusDelay.tv_nsec   = focusTime == 0 ? 0 : focusTime % 1000;
    ctx->frameDelay.tv_sec    = fr == 1 ? 1 : 0;
    ctx->frameDelay.tv_nsec   = fr == 1 ? 0 : (1.00000001 / fr) * 1000000000L;
    ctx->longWait.tv_sec      = longWait / 1000;
    ctx->longWait.tv_nsec     = longWait % 1000;
    ctx->raiseDelay.tv_sec    = raiseT == 0 ? 0 :  raiseT / 1000;
    ctx->raiseDelay.tv_nsec   = raiseT == 0 ? 0 : (raiseT % 1000) * 1000000L;
    ctx->restoreDelay.tv_sec  = resT   == 0 ? 0 :  resT   / 1000;
    ctx->restoreDelay.tv_nsec = resT   == 0 ? 0 : (resT   % 1000) * 1000000L;
    ctx->clickDelay.tv_sec    = clickT == 0 ? 0 :  clickT / 1000;
    ctx->clickDelay.tv_nsec   = clickT == 0 ? 0 : (clickT % 1000) * 1000000L;
    ctx->autoCenter           = * ((int*) args->m_Args[AUTOCENTER]->m_Value);
    ctx->topOffset            = * ((int*) args->m_Args[TOPOFFSET]->m_Value);
    ctx->bgColorStr           = (const char*) args->m_Args[BGCOLOR]->m_Value;
    ctx->bgImgFilePath        = (const char*) args->m_Args[BGIMAGE]->m_Value;
    ctx->bgImgFileSet         = args->m_Args[BGIMAGE]->m_IsSet;
    ctx->bgImgStatus          = False;
    ctx->exitKeyStr           = args->m_Args[EXITKEY]->m_Value;
    ctx->exitKeyMask          = EXIT_MASK;
    ctx->transCtrlKeyStr      = args->m_Args[CLONEKEY]->m_Value;
    ctx->cloneKeyMask         = TRANSLATION_CTRL_MASK;
    ctx->srcW                 = srcid;
    ctx->isDaemon             = args->m_Args[DAEMON]->m_IsSet;
    ctx->lckFPath             = args->m_Args[LCKFPATH]->m_Value;
    ctx->multiInst            = args->m_Args[MULTIINST]->m_IsSet;
    ctx->multiInst            = ctx->isDaemon == False ? ctx->multiInst : True;
    ctx->slavePtrDevId        = NO_DEVICE;
    ctx->masterPtrDevId       = NO_DEVICE;
    ctx->ptrDevName           = ptrDevName;
    ctx->logFileName          = (const char*) args->m_Args[LOGFNAME]->m_Value;
    ctx->translateOnly        = args->m_Args[TRANSONLY]->m_IsSet;
    ctx->confFileName         = (const char*) args->m_Args[CONFFILE]->m_Value;
    ctx->userDir              = NULL;

    if (enableLogFile (ctx) == False)
    {
        freeXWCContext (ctx);
        delArgs (args);
        return NULL;
    }

    if (   (ctx->multiInst == False || ctx->isDaemon == True)
        && ifSingleInst (ctx) == False)
    {
        freeXWCContext (ctx);
        delArgs (args);
        return NULL;
    }

    if (openDefaultDisplay (ctx)  == False)
    {
        freeXWCContext (ctx);
        delArgs (args);
        return NULL;
    }

    if (chkCompExt (ctx->xDpy) == False)
    {
        XCloseDisplay (ctx->xDpy);
        freeXWCContext (ctx);
        delArgs (args);
        return NULL;
    }

    if (chckXI2Ext (ctx) == False)
    {
        XCloseDisplay (ctx->xDpy);
        freeXWCContext (ctx);
        delArgs (args);
        return NULL;
    }

    if (getDefaultDisplayData (ctx) == False)
    {
        XCloseDisplay (ctx->xDpy);
        freeXWCContext (ctx);
        delArgs (args);
        return NULL;
    }

    if (getUserDir (ctx) == False)
    {
        XCloseDisplay (ctx->xDpy);
        freeXWCContext (ctx);
        delArgs (args);
        return NULL;
    }

    if (getInputDevices (ctx) == False)
    {
        XCloseDisplay (ctx->xDpy);
        freeXWCContext (ctx);
        delArgs (args);
        return NULL;
    }

    if (getCtrlKeycodes (ctx) == False)
    {
        XCloseDisplay (ctx->xDpy);
        freeXWCContext (ctx);
        delArgs (args);
        return NULL;
    }

    if ((ctx->devsAcquired = grabAllKeys (ctx)) == False)
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

    if (args->m_Args[HELP]->m_IsSet == True)
    {
        printUsage (args);
        XCloseDisplay (ctx->xDpy);
        freeXWCContext (ctx);
        delArgs (args);
        return NULL;
    }

    if (args->m_Args[MKCONFIG]->m_IsSet == True)
    {
        printCurValues (args);
        if (createConfFile (ctx, args) == False)
        {
            logCtrl ("\tError creating config file!", LOG_LVL_NO, False);
        }
        XCloseDisplay (ctx->xDpy);
        freeXWCContext (ctx);
        delArgs (args);
        return NULL;
    }

    logCtrl ("\n\tconfiguration successfully completed", LOG_LVL_1, True);

    if (LOG_LVL > LOG_LVL_1)
    {
        printCurValues (args);
    }

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
        if (ctx->kbds->devs != NULL)
        {
            free (ctx->kbds->devs);
        }
        free (ctx->kbds);
    }

    free (ctx);
}
