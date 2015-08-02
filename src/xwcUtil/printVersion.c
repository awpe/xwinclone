#include <xwcUtil.h>

void
printVersion (void)
{
    char buf[1024];
    snprintf (buf, sizeof (buf), "%s version %s", WM_CLASS_PRG_NAME_STR,
              XWINCLONE_VERSION_STR);
    logCtrl (buf, LOG_LVL_NO, False);
}
