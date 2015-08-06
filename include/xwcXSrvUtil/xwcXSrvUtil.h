/* Copyright (c) 2015, Anton Lantcevich
    
   This file is part of the xwinclone.
   <https://github.com/awpe/xwinclone>
  
   Xwinclone is released under the BSD 3-Clause License.
   You should have received a copy of the BSD 3-Clause
   License along with this software; if not see
   <https://github.com/awpe/xwinclone/blob/master/LICENSE.md>.  */

#ifndef XWCXSRVUTIL_H
#define	XWCXSRVUTIL_H

#include <defines.h>
#include <systemHeaders.h>
#include <xwcHeaders.h>

/*****************************************/
/*Redefine XComposite extension functions*/
/*****************************************/
typedef void (*XCompRedirWin_t)(Display *, Window, int) ;
extern XCompRedirWin_t redirWin;

typedef void (*XCompUnRedirWin_t)(Display *, Window, int) ;
extern XCompUnRedirWin_t unRedirWin;

typedef void (*XCompRedirSubWin_t)(Display *, Window, int) ;
extern XCompRedirSubWin_t redirSubWin;

typedef void (*XCompUnRedirSubWin_t)(Display *, Window, int) ;
extern XCompUnRedirSubWin_t unRedirSubWin;
/****************************************/


/** @var Bool X_ERROR
@brief Shows if there was an error on x server side
@warning Not thread safe!
 */
extern Bool X_ERROR;

/** @var Bool X_ERROR
@brief Contains the last x server error code. 
@warning Not thread safe!
 */
extern int  X_ERROR_CODE;

/**
 * Opens connection to default X display. Provides simple verbose.
 * @param[in] ctx Pointer to program's context structure
 * @return Xlib's True on success, False otherwise
 * @sa XOpenDisplay(), XCloseDisplay(), XSetErrorHandler()
 */
Bool
openDefaultDisplay (XWCContext * ctx);

/**
 * Custom Xlib error handler. Provides verbose for errors. And sets global 
 * error flag.
 * @param[in] display Pointer to Xlib's Display data struct.
 * @param[in] error   Pointer to Xlib's error event data  struct
 * @return an integer
 * @sa XSetErrorHandler()
 * @todo Add verbose to all types of possible errors. Seems it also need
 * to mutually assign error code somewhere...
 */
int
errorHandlerBasic (Display     * display,
                   XErrorEvent * error);

/**
 * Returns call-time state of X_ERROR variable. X_ERROR is always set 
 * to False.
 * @param[in] s Pointer to Xlib's Screen data struct
 * @return Xlib's Bool with call-time state of X_ERROR
 * @sa XCloseDisplay()
 */
Bool
getXErrState (XWCContext * ctx);

void
printDrawableInfo (Display  * xDpy,
                   Drawable   drw);

Bool
getVisOfScr (XWCContext  * ctx,
             XVisualInfo * xVisInfo);

/**
 * Gets default screen and default root window with attributes for established 
 * x server connection
 * @param[in] ctx Pointer to program context
 * @return Xlib's True on success, False otherwise
 */
Bool
getDefaultDisplayData (XWCContext * ctx);

/**
 * Checks if x server we have connection to has composite extension of 
 * supported version
 * @param[in] d Pointer to Xlib's Display data struct.
 * @return Xlib's True on success, False otherwise
 */
Bool
chkCompExt (Display * d);

Bool
parseColor (XWCContext * cfg);

/**
 * Searches for root window in Xlib's Screen struct.
 * @param[in] s Pointer to Xlib's Screen data struct
 * @return Xlib's window with root window xid for specified screen 
 * or Xlib's None macro in case of error
 */
Window
getRootWinOfScr (Screen * s);

Colormap
createColormap (XWCContext * ctx,
                Visual     * xVis);

Bool
getCtrlKeycodes (XWCContext * ctx);

#endif

