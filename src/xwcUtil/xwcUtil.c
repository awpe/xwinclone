#include <xwcUtil.h>

/*Global X_ERROR initialization*/
Bool X_ERROR      = False;
int  X_ERROR_CODE = 0;

XCompRedirWin_t      redirWin         = & XCompositeRedirectWindow;
XCompUnRedirWin_t    unRedirWin       = & XCompositeUnredirectWindow;
XCompRedirSubWin_t   redirSubWin      = & XCompositeRedirectSubwindows;
XCompUnRedirSubWin_t unRedirSubWin    = & XCompositeUnredirectSubwindows;

/*Global LOG_LVL initialization*/
int LOG_LVL = DEFAULT_LOG_LVL;

FILE * LOG_FILE = NULL;

void
printVersion (void)
{
    char buf[1024];
    snprintf (buf, sizeof (buf), "%s version %s", WM_CLASS_PRG_NAME_STR,
              XWINCLONE_VERSION_STR);
    logCtrl (buf, LOG_LVL_NO, False);
}

Bool
ifSingleInst (XWCContext * ctx)
{
    char buf[1024];
    int  rc;

    logCtrl ("\tCreating lock file:", LOG_LVL_1, False);

    if (ctx == NULL)
    {
        logCtrl ("\t\tError: null pointer to context!", LOG_LVL_NO, True);
    }

    ctx->lckFD = open (ctx->lckFPath, O_CREAT | O_RDWR, 0666);

    if (ctx == NULL)
    {
        snprintf (buf, sizeof (buf), "\t\tError: cannot create lock file %s!",
                  ctx->lckFPath);
        logCtrl (buf, LOG_LVL_NO, True);
    }

    rc = flock (ctx->lckFD, LOCK_EX | LOCK_NB);

    if (rc != 0 && EWOULDBLOCK == errno)
    {

        snprintf (buf, sizeof (buf), "\t\tFile %s seems to be already created "
                  "and locked,\n\t\tassuming other instance of this program "
                  "is running", ctx->lckFPath);
        logCtrl (buf, LOG_LVL_NO, False);
        return False;
    }

    logCtrl ("\t\tsuccess", LOG_LVL_2, True);

    return True;
}

Bool
checkFileMode (const char * fname,
               int          mode)
{
    return access (fname, mode) != - 1 ? True : False;
}

Bool
getUserDir (XWCContext * ctx)
{
    char buf[1024];

    if ((ctx->userDir = getenv ("HOME")) == NULL)
    {
        ctx->userDir = getpwuid (getuid ())->pw_dir;
    }

    if (ctx->userDir == NULL)
    {
        snprintf (buf, sizeof (buf), "Cannot determine user directory to "
                  "search for config file '%s'!", ctx->confFileName);
        logCtrl (buf, LOG_LVL_1, True);
        //        XCloseDisplay (ctx->xDpy);
        //        freeXWCContext (ctx);
        //        delArgs (args);
        //        return NULL;
    }
    return True;
}
