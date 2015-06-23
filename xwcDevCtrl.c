#include "xwc.h"

Bool
grabKeys (XWCContext * ctx)
{
    logCtr ("Trying to grab key combinations:", LOG_LVL_2, False);

    if (ctx == NULL)
    {
        logCtr ("\tCannot grab key combinations: null pointer to program"
                " context!", LOG_LVL_NO, True);
        return False;
    }

    if (ctx->xDpy == NULL)
    {
        logCtr ("\tCannot grab key combinations: null pointer to X "
                "connection!", LOG_LVL_NO, True);
        return False;
    }

    if (ctx->rootW == None)
    {
        logCtr ("\tCannot grab key combinations: no root window specified!",
                LOG_LVL_NO, True);
        return False;
    }

    logCtr ("\tTrying to grab exit key combination:", LOG_LVL_2, False);

    XGrabKey (ctx->xDpy, ctx->exitKeyCode, ctx->exitKeyMask, ctx->rootW, True,
              GrabModeAsync, GrabModeAsync);

    XSync (ctx->xDpy, 0);

    if (getXErrState () == True)
    {
        logCtr ("\t\tCannot grab exit key combination: XGrabKey error! (Another"
                " program may had already grabbed such a combination)",
                LOG_LVL_NO, True);
        return False;
    }

    logCtr ("\t\tsuccess", LOG_LVL_2, True);


    logCtr ("\tTrying to grab translation control key combination:", LOG_LVL_2,
            False);

    XGrabKey (ctx->xDpy, ctx->cloneKeyCode, ctx->cloneKeyMask,
              ctx->rootW, True,
              GrabModeAsync, GrabModeAsync);

    XSync (ctx->xDpy, 0);

    if (getXErrState () == True)
    {
        ungrabKeys (ctx);
        logCtr ("\t\tCannot grab translation control key combination: XGrabKey"
                " error! (Another program may had already grabbed such a"
                " combination)", LOG_LVL_NO, True);
        return False;
    }

    logCtr ("\t\tsuccess", LOG_LVL_2, True);

    return True;
}

void
ungrabKeys (XWCContext * ctx)
{

    if (ctx == NULL)
    {
        logCtr ("Cannot ungrab key combinations: invalid pointer to program"
                " context!", LOG_LVL_NO, False);
        return;
    }

    if (ctx->xDpy == NULL)
    {
        logCtr ("Cannot ungrab key combinations: null pointer to X "
                "connection!", LOG_LVL_NO, False);
        return;
    }


    if (ctx->rootW == None)
    {
        logCtr ("Cannot ungrab key combinations: no window specified!",
                LOG_LVL_NO, False);
        return;
    }

    XUngrabKey (ctx->xDpy, ctx->exitKeyCode, ctx->exitKeyMask, ctx->rootW);

    if (getXErrState () == True)
    {
        logCtr ("Cannot ungrab exit key combination: XUngrabKey error!",
                LOG_LVL_NO, False);
        return;
    }

    XUngrabKey (ctx->xDpy, ctx->cloneKeyCode, ctx->cloneKeyMask,
                ctx->rootW);

    if (getXErrState () == True)
    {
        logCtr ("Cannot ungrab translation control key combination: XUngrabKey"
                " error!", LOG_LVL_NO, False);
        return;
    }
}

int
mvPtr (XWCContext * ctx, int x, int y, int screen)
{
    int ret;

    ret = 0;
    //ret = XWarpPointer (ctx->xDpy, None, ctx->rootW, 0, 0, 0, 0, x, y);

    ret = XIWarpPointer ( ctx->xDpy, FIRST_POINTER_ID, None, ctx->rootW, 0, 0, 0, 0, x, y);
    ret = XIWarpPointer ( ctx->xDpy, SECOND_POINTER_ID, None, ctx->rootW, 0, 0, 0, 0, x, y);

    XFlush (ctx->xDpy);

    return ret == 0;
}

int
mvPtrWRel (XWCContext * ctx, Window window, int x, int y)
{
    Window n;
    int    rX, rY;

    XTranslateCoordinates (ctx->xDpy, window, ctx->rootW, x, y, &rX, &rY, &n);

    return mvPtr (ctx, rX, rY, XScreenNumberOfScreen (ctx->xScr));
}

int
getMouseLoc (XWCContext * ctx,
             int        * xR,
             int        * yR,
             int        * scrNumR,
             Window     * wR)
{
    int          ret, x, y, screen_num, dummy_int;
    Window       window;
    Window       root;
    unsigned int dummy_uint;
    //char         buf[1024];

    ret         = False;
    window      = None;
    root        = None;

    ret = XQueryPointer (ctx->xDpy, ctx->rootW, &root, &window, &x, &y,
                         &dummy_int, &dummy_int, &dummy_uint);
    if (ret == True)
    {
        screen_num = XScreenNumberOfScreen (ctx->xScr);
    }
    else
    {
        logCtr ("Cannot get mouse location", LOG_LVL_NO, False);
        return False;
    }

    if (wR != NULL)
    {
        /* Find the client window if we are not root. */
        if (window != root && window != 0)
        {
            int findret;
            Window client = 0;

            /* Search up the stack for a client window for this window */
            findret = findWClient (ctx, window, &client, FIND_PARENTS);
            if (findret == False)
            {
                /* If no client found, search down the stack */
                findret = findWClient (ctx, window, &client, FIND_CHILDREN);
            }

            //fprintf (stderr, "%lX, %lX, %lX, %X\n", window, root, client, findret);

            if (findret == True)
            {
                window = client;
            }
        }
        else
        {
            window = root;
        }
    }

    //printf ("mouseloc root: %lX\n", root);
    //printf ("mouseloc window: %lX\n", window);

    if (ret == True)
    {
        if (xR != NULL)
        {
            *xR = x;
        }
        if (yR != NULL)
        {
            *yR = y;
        }
        if (scrNumR != NULL)
        {
            *scrNumR = screen_num;
        }
        if (wR != NULL)
        {
            *wR = window;
        }
    }

    return ret;
}

unsigned int
getInSt (XWCContext * ctx)
{
    Window       dummy;
    int          rX, rY, wX, wY;
    unsigned int mask;

    XQueryPointer (ctx->xDpy, ctx->rootW, &dummy, &dummy, &rX, &rY, &wX, &wY, &mask);

    return mask;
}

int
mouseBtnCtrl (XWCContext * ctx, int button, int is_press)
{
    int          res;
    int          screen;
    XButtonEvent xbpe;
    Window       z;

    memset (&xbpe, 0, sizeof (xbpe));

    z = ctx->srcW;

    getMouseLoc (ctx, &xbpe.x_root, &xbpe.y_root, &screen, &z);

    //printf ("\tmouse is in %d, %d relative to root\n", xbpe.x_root, xbpe.y_root);

    xbpe.window      = ctx->srcW;
    xbpe.button      = button;
    xbpe.display     = ctx->xDpy;
    xbpe.root        = ctx->rootW;
    xbpe.same_screen = True;
    xbpe.state       = getInSt (ctx);
    xbpe.subwindow   = None;
    xbpe.time        = CurrentTime;
    xbpe.type        = (is_press ? ButtonPress : ButtonRelease);

    XTranslateCoordinates (ctx->xDpy, ctx->rootW, ctx->srcW, xbpe.x_root,
                           xbpe.y_root, &xbpe.x, &xbpe.y, &xbpe.subwindow);

    //printf ("\t1relative to source it is %d, %d\n", xbpe.x, xbpe.y);

    //printf ("\t1src is %lX, subw is %lX\n", ctx->srcW, xbpe.subwindow);

    if (xbpe.subwindow != None)
    {
        XTranslateCoordinates (ctx->xDpy, ctx->rootW, xbpe.subwindow,
                               xbpe.x_root, xbpe.y_root, &xbpe.x, &xbpe.y, &xbpe.subwindow);

        //printf ("\t1relative to source it is %d, %d\n", xbpe.x, xbpe.y);

        //printf ("\t1src is %lX, subw is %lX\n", ctx->srcW, xbpe.subwindow);
    }

    if (is_press == False)
    {
        xbpe.state |= Button1MotionMask;
    }

    res = XSendEvent (ctx->xDpy, ctx->srcW, True, ButtonPressMask,
                      (XEvent *) & xbpe);
    XFlush (ctx->xDpy);

    if (res == 0 || getXErrState () == True)
    {
        logCtr ("Error sending click event", LOG_LVL_NO, False);
        return False;
    }

    return True;
}

int
setMouseUp (XWCContext * ctx,
            int          button)
{
    return mouseBtnCtrl (ctx, button, False);
}

int
setMouseDown (XWCContext * ctx,
              int          button)
{
    return mouseBtnCtrl (ctx, button, True);
}

int
clickW (XWCContext * ctx,
        int          button)
{
    int ret;

    ret = setMouseDown (ctx, button);

    if (ret != True)
    {
        logCtr ("Click failed!", LOG_LVL_NO, False);
        return ret;
    }

    nanosleep (&ctx->clickDelay, NULL);

    ret = setMouseUp (ctx, button);

    return ret;
}

int
getPressedComb (XWCContext * ctx)
{
    XEvent       xEvent;
    Bool         res;
    Window       rRetW, chRetW;
    int          rX, rY, trgX, trgY;
    unsigned int mask_return;
    char         buf[1024];

    while (XPending (ctx->xDpy) != 0)
    {
        XNextEvent (ctx->xDpy, &xEvent);
        switch (xEvent.type)
        {
            case ButtonRelease:

                if (xEvent.xbutton.button != Button1)
                {
                    continue;
                }

                logCtr ("Got click!", LOG_LVL_2, False);

                res = XQueryPointer (ctx->xDpy, ctx->trgW, &rRetW, &chRetW,
                                     &rX, &rY, &trgX, &trgY, &mask_return);

                if (res == False)
                {
                    logCtr ("Error getting pointer coordinates!\n", LOG_LVL_NO,
                            False);
                    continue;
                }

                mvPtrWRel (ctx, ctx->srcW, trgX, trgY);

                clickW (ctx, Button1);

                mvPtrWRel (ctx, ctx->trgW, trgX, trgY);

                return NO_KEY_PRESSED;
                break;

            case KeyPress:
                snprintf (buf, sizeof (buf), "Got key combination\n\tkeycode:"
                          "\t%d\n\tkey state:\t%d\n\texit key code:\t%d\n\texit"
                          " key mask:\t%d\n\ttrans key code:\t%d\n\ttrans key "
                          "mask:%d\n\txEvent.xkey.state ^ cfg->exitKeyMask:\t%d"
                          "\n\txEvent.xkey.state ^ cfg->translationCtrlKeyMask:"
                          "\t%d", xEvent.xkey.keycode, xEvent.xkey.state,
                          ctx->exitKeyCode, ctx->exitKeyMask,
                          ctx->cloneKeyCode,
                          ctx->cloneKeyMask,
                          xEvent.xkey.state ^ ctx->exitKeyMask,
                          xEvent.xkey.state ^ ctx->cloneKeyMask);
                logCtr (buf, LOG_LVL_2, False);

                if (xEvent.xkey.keycode == ctx->exitKeyCode
                    && (xEvent.xkey.state ^ ctx->exitKeyMask) == 0)
                {
                    logCtr ("Exit key sequence received!", LOG_LVL_NO, False);
                    return EXIT_COMBINATION;
                }
                else if (xEvent.xkey.keycode
                         == ctx->cloneKeyCode
                         && (xEvent.xkey.state ^ ctx->cloneKeyMask) == 0
                         && ctx->isDaemon == True)
                {
                    logCtr ("Grab window key sequence received!", LOG_LVL_NO,
                            False);
                    return TRANSLATION_COMB;
                }
                else
                {
                    XAllowEvents (ctx->xDpy, ReplayKeyboard, xEvent.xkey.time);
                    XFlush (ctx->xDpy);
                }
                break;

            default:
                break;
        }
    }
    return NO_KEY_PRESSED;
}
