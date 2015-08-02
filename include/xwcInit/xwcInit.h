#ifndef XWCINIT_H
#define	XWCINIT_H

#include <defines.h>
#include <systemHeaders.h>

struct DevList_
{
    int * devs;
    int   nDevs;
} ;

typedef struct DevList_ DevList;

/**
 * Actual structure to hold various program options. Maybe you want to use 
 * type definition `XWCOptions` instead.
 * @sa options
 */
struct XWCContext_
{
    int               autoCenter;  /**< whether to enable autocentering of 
                                      * source window int the xwinclone window 
                                      * (in case source window is smaller than 
                                      * xwinclone's window).\n 1 - enabled, 
                                      * 0 -  disabled, other - enabled*/
    int               topOffset;   /**< Source window top offset (pixels)*/
    XColor            bgColor;     /**< Background color data*/
    const char      * bgColorStr;  /**< Background color string (#rrggbb)*/
    Bool              bgColorNeedFree;
    const char      * bgImgFilePath; /**< Background image file path*/
    Bool              bgImgFileSet; /**< Shows if background image file path
                                      *  was specified*/
    Bool              bgImgNeedFree;
    Bool              bgImgStatus; /**< Shows if background image file has
                                     *  been loaded to x server's pixmap*/
    struct timespec   focusDelay;  /**< how long to wait, before you focuse 
                                      * on window you want to be cloned*/
    struct timespec   frameDelay;  /**< how often should xwinclone refresh 
                                      * its content (frames per second)*/
    struct timespec   raiseDelay;   /**< Used for waiting in case of unmapped
                                    *  windows*/
    struct timespec   clickDelay;
    
    struct timespec   longWait;
    
    struct timespec   restoreDelay;
    
    const char      * exitKeyStr;  /**< string representing exit key 
                                      * (keysymdef.h)*/
    Bool              exitKeyStrNeedFree;
    
    const char      * transCtrlKeyStr;  /**< string representing translation
                                         *  control key (keysymdef.h)*/
    Bool              transCtrlKeyNeedFree;
    KeyCode           exitKeyCode; /**< result of exit key string parsing*/
    int               exitKeyMask; /**< Exit key modifier according to X.h*/
    KeyCode           cloneKeyCode; /**< result of translation control key
                                     *  string parsing*/
    int               cloneKeyMask; /**< Translation control key modifier
                                     *  according to X.h*/
    int               isDaemon;    /**< If program considered to be run in 
                                    * daemon mode*/
    int               multiInst; /**< Allow only one instance of program
                                    *  to be run, automatically set in
                                    *  daemon mode*/
    Display         * xDpy;
    Screen          * xScr;
    Window            rootW;
    XWindowAttributes rootWAttr;
    Window            srcW; /**< Default window id to be used as 
                             * source*/
    XWindowAttributes srcWAttr;
    Window            trgW;
    XWindowAttributes trgWAttr;

    int               lckFD;

    const char      * lckFPath;
    Bool              lckFileNameNeedFree;
   

    DevList         * kbds; /**< List of master keyboard devices*/

    int               xiOp; /**< XInput 2 extension opcode */

    int               nMods; /**< Number of modifiers for control keys*/

    XIGrabModifiers * clMods; /**< Modifiers for clone key */
    XIGrabModifiers * exitMods; /**< Modifiers for exit key */

    Bool              devsAcquired;

    int               slavePtrDevId; /**< Used to select events*/
    int               masterPtrDevId; /**< Used to manipulate pointer*/

    const char      * ptrDevName;
    Bool              ptrDevNameNeedFree;
    
    const char      * logFileName;
    Bool              logFileNameNeedFree;
    
    Bool              procBtnEv;
    
    const char      * confFileName;
    
    const char      * userDir;
} ;

/** 
 * @typedef XWCOptions
 * @brief This is a type definition for struct _XWCOptions. Struct with 
 * various program options.
 * @var XWCOptions.focusTime 
 * Field 'focusTime' stores amount of time to wait, before you focuse on 
 * window you want to be cloned (in seconds)
 * @var XWCOptions.frameRate 
 * Field 'frameRate' sets frequency of content refreshing 
 * @var XWCOptions.autoCenter 
 * Field 'autoCenter' controls autocentering(0 - disabled, other - enabled)
 * @var XWCOptions.topOffset 
 * Field 'topOffset' sets top offset for source window (in pixels)
 * @var XWCOptions.bgColor 
 * Field 'bgColor' color data struct used for background color manipulation
 * @var XWCOptions.exitKeyStr 
 * Field 'exitKeyStr' exit key string (keysymdef.h)
 * @var XWCOptions.exitKeyCode 
 * Field 'exitKeyCode' result of exit key string parsing
 * @var XWCOptions.exitKeyMask 
 * Field 'exitKeyMask' defines exit key modifier according to X.h
 * @var XWCOptions.srcWinId 
 * Field 'srcWinId' defines window id to be used instead of focused
 */
typedef struct XWCContext_ XWCContext;


/**
 * Processes string arguments. Allocates and fills up `XWCOptions` struct. 
 * Returns Null on error.
 * @param[in] argCnt Number of strings in arguments array.
 * @param[in] argArr Pointer to array of arguments's strings.
 * @param[in] d Pointer to Xlib's Display struct.
 * @return Pointer to `XWCOptions` or NULL in case of error.
 * @todo Add some heuristics to argument processing.
 */
XWCContext *
init (int           argCnt,
      const char ** argArr);


void
freeXWCContext (XWCContext * ctx);

#endif
