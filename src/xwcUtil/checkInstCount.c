#include <xwcUtil.h>

Bool
checkInstCount (XWCContext * ctx)
{
    char buf[1024];
    int  rc;

    logCtrl ("\tCreating lock file", LOG_LVL_1, False);

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
