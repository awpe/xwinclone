#include <xwcWinUtil.h>

void
printWindowInfo (XWCContext        * ctx,
                 Window              w,
                 XWindowAttributes * xWinAttr)
{

    char buf[1024];

    if (ctx == NULL)
    {
        logCtrl ("Error cannot print window information: "
                 "Bad context pointer", LOG_LVL_NO, False);
        return;
    }

    if (w == None)
    {
        logCtrl ("Error cannot print window information: No window specified",
                 LOG_LVL_NO, False);
        return;
    }

    if (xWinAttr == NULL)
    {
        logCtrl ("Error cannot print window information: "
                 "No window attributes data specified", LOG_LVL_NO, False);
        return;
    }

    printWindowName (ctx, w);
    printWindowClass (ctx, w);

    snprintf (buf, sizeof (buf), "Window info:\n\tWidth:\t%d\n\tHeight:\t%d\n\t"
              "Depth:\t%d",
              xWinAttr->width, xWinAttr->height, xWinAttr->depth);

    logCtrl (buf, LOG_LVL_1, False);
}
