#include <xwcXSrvUtil.h>

void
printDrawableInfo (Display  * xDpy,
                   Drawable   drw)
{
    Window       r;
    Status       st;
    int          x, y;
    unsigned int w, h, b, d;
    char         buf[1024];

    st = XGetGeometry (xDpy, drw, &r, &x, &y, &w, &h, &b, &d);

    if (st == 0 )
    {
        logCtrl ("XGetGeometry: Cannot get parameters of specified drawable!",
                 LOG_LVL_NO, False);
        return;
    }

    snprintf (buf, sizeof (buf), "Drawble information:\n\tWidth:\t"
              "%d\n\tHeight:\t%d\n\tDepth:\t%u\n\troot win:\t%lX", w, h, d, r);

    logCtrl (buf, LOG_LVL_1, False);
}
