#include <xwcWinUtil.h>

Window
createWindow (XWCContext           * ctx,
              Visual               * xVis,
              long long int          mask,
              XSetWindowAttributes * attr)
{
    Window w;
    w = XCreateWindow (ctx->xDpy, ctx->rootW, 0, 0, ctx->srcWAttr.width,
                       ctx->srcWAttr.height, 0, ctx->srcWAttr.depth,
                       InputOutput, xVis, mask, attr);
    XFlush (ctx->xDpy);
    return w;
}
