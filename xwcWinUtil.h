#ifndef XWCWINUTIL_H
#define	XWCWINUTIL_H

#include "xwcInit.h"


typedef enum triStateLogic_
{
    UNDEFINED = INT_MIN, //True != INT_MIN && False != INT_MIN
    TRUE      = True, //Xlib's Bool compilant
    FALSE     = False //Xlib's Bool compilant
} triState;

/**
 * Tries to get currently focused window. It may NOT be the id of window 
 * you really see on the screen, it depends on toolkit beign used 
 * (GTK/QT/...)
 * @param[in] d Pointer to Xlib's Display data struct.
 * @return Window XID or 0 (Xlib's `None` macro) if no focused window found
 * or error occured.
 * @sa XGetInputFocus()
 */
Window
getFocusedWindow (XWCContext * ctx);

/**
 * Tries to find top (child-of-root) window which is the parent to 
 * specified one (thus in first in the chain).
 * @param[in] d Pointer to Xlib's Display data struct.
 * @param[in] start Window xid of window which parent to look for
 * @return Window XID or 0 (Xlib's `None` macro) if no window found
 * or error occured.
 * @sa XQueryTree()
 */
Window
getTopWindow (XWCContext * ctx,
              Window       start);

/**
 * Tries to find window, at or below the specified window, that has a 
 * WM_STATE property.
 * @param[in] ctx Pointer to XWCOptions struct
 * @param[in] start Window xid of window where search begins
 * @return Window XID in case of success or if if no window found, 
 * or 0 (Xlib's `None` macro) if error occured.
 * @sa XmuClientWindow()
 */
Window
getNamedWindow (XWCContext * ctx,
                Window       start);

/**
 * Tries to get currently active window. Uses different approaches to get
 * proper active window xid that can be used to aquire window's image.
 * @param[in] ctx Pointer to XWCOptions struct
 * @param[in] implicitW Window xid of implicitly specified window to be checked
 * @return Window XID or 0 (Xlib's `None` macro) if no active window found
 * or error occured.
 * @sa getFocusedWindow(), getTopWindow(), getNamedWindow()
 */
Window
getActiveWindow (XWCContext * ctx, 
                 Window       implicitW);

/**
 * Prints window name as reported to window manager (ICCC WM_NAME).
 * @param[in] d Pointer to Xlib's Display data struct.
 * @param[in] w Window xid of window which WM_CLASS should be printed
 * @todo Add return value for result checking
 * @sa XAllocClassHint(), XGetClassHint()
 */
void
printWindowName (XWCContext * ctx,
                 Window       w);
/**
 * Prints window class as reported to window manager (ICCC WM_CLASS).
 * @param[in] ctx Pointer to XWCOptions struct
 * @param[in] w Window xid of window which WM_CLASS should be printed
 * @todo Add return value for result checking
 * @sa XAllocClassHint(), XGetClassHint()
 */
void
printWindowClass (XWCContext * ctx,
                  Window       w);

/**
 * Prints window info (class and name).
 * @param[in] ctx Pointer to XWCOptions struct
 * @param[in] w Window xid of window which info should be printed
 * @param[in] xWinAttr Pointer to XWindowAttributes data struct
 * @todo Add return value for result checking
 * @sa printWindowName(), printWindowClass(), XWindowAttributes
 */
void
printWindowInfo (XWCContext        * ctx,
                 Window              w,
                 XWindowAttributes * xWinAttr);

/**
 * Sets window name (ICCC WM_NAME). Usually seen in titlebar.
 * @param[in] ctx Pointer to XWCOptions struct
 * @param[in] WID Window xid of window which name will be changed
 * @param[in] name Null-terminated array of chars with window name
 * @return Xlib's True on success, False otherwise
 * @sa XSetWMName(), XStringListToTextProperty()
 */
Bool
setWinTitlebar (XWCContext * ctx,
                Window       WID,
                const char * name);
/**
 * Sets window class properties (ICCC WM_CLASS) which have two c-strings
 * first with permanent window name and second with the name of class.
 * @param[in] ctx Pointer to XWCOptions struct
 * @param[in] WID Window xid of window which name will be changed
 * @param[in] permNameStr Null-terminated array of chars with permanent 
 * window name
 * @param[in] classStr Null-terminated array of chars with class name
 * @return Xlib's True on success, False otherwise
 * @sa XChangeProperty(), XStringListToTextProperty(), XA_WM_NAME, 
 * PropModeReplace
 */
Bool
setWindowClass (XWCContext * ctx,
                Window       WID,
                const char * permNameStr,
                const char * classStr);

Window
createWindow (XWCContext           * ctx,
              Visual               * xVis,
              long long int          mask,
              XSetWindowAttributes * attr);

Bool
createTrgWindow (XWCContext * ctx);

unsigned char *
getWPrprtByAtom (XWCContext * ctx,
                 Window       window,
                 Atom         atom,
                 long       * nitems,
                 Atom       * type,
                 int        * size);

int
findWClient (XWCContext * ctx,
             Window       window,
             Window     * window_ret,
             int          direction);

/**
 * if window ioWin is not focused, raises it and sets input focus to that window
 * then sets ioWin to be last focused window
 * @param[in] ctx Pointer to XWCOptions struct
 * @param[in/out] ioWin Window xid of window to be raised
 * @return Xlib's True on success, False otherwise
 * @sa XRaiseWindow(), XSetInputFocus()
 * @todo check for minimized windows and maximize them if needed
 * @todo check for desired coords as it may not be needed to do 
 * minimize/maximize routine
 */
Bool
wRaiseCtrl (XWCContext * ctx,
            Window     * ioWin);

triState
isWinVis (XWCContext * ctx,
          Window       checkW);

Bool
toggleHiddenState (XWCContext * ctx,
                   Window       ioWin);

Bool
getWAttr (XWCContext        * ctx,
          Window            * win,
          XWindowAttributes * wa);

#endif
