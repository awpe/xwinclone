#ifndef XWCLIB_H
#define	XWCLIB_H

#ifdef	__cplusplus
extern "C"
{
#endif

#include "defines.h"
#include "headers.h"


    /*Redefine imlib2 functions*/
    typedef
    Imlib_Image (*imgLibCreateCrSc_t)(int, int, int, int, int, int) ;

    /****************************************/

    typedef enum argNames_
    {
        HELP          = 0,
        FRAMERATE     = 1,
        BGCOLOR       = 2,
        AUTOCENTER    = 3,
        FOCUSTIME     = 4,
        TOPOFFSET     = 5,
        LOGLVL        = 6,
        SOURCEID      = 7,
        DAEMON        = 8,
        SINGLEINST    = 9,
        BGIMAGE       = 10,
        /****************************************/
        /*Write count of possible arguments here*/
        OPTIONS_COUNT = 11
        /****************************************/
    } argNames;

    typedef enum argTypes_
    {
        C_STR, INT, ULONG
    } argTypes;

    typedef struct argument_
    {
        int            m_SynCnt;
        const char **  m_SynStrs;
        argTypes       m_Type;
        argNames       m_Name;
        const char *   m_NameStr;
        Bool           m_IsSet;
        Bool           m_HasValue;
        void       *   m_Value;
    } argument;

    typedef struct arguments_
    {
        int         m_ArgCnt;
        argument ** m_Args;
    } arguments;

    /**
     * Actual structure to hold various program options. Maybe you want to use 
     * type definition `XWCOptions` instead.
     * @sa options
     */
    struct XWCOptions_
    {
        int               autoCenter;  /**< whether to enable autocentering of 
                                        * source window int the xwinclone window 
                                        * (in case source window is smaller than 
                                        * xwinclone's window).\n 1 - enabled, 
                                        * 0 -  disabled, other - enabled*/
        int               topOffset;   /**< Source window top offset (pixels)*/
        XColor            bgColor;     /**< Background color data*/
        const char      * bgColorStr;  /**< Background color string (#rrggbb)*/
        const char      * bgImgFileStr; /**< Background image file path*/
        Bool              bgImgFileSet; /**< Shows if background image file path
                                        *  was specified*/
        Bool              bgImgStatus; /**< Shows if background image file has
                                        *  been loaded to x server's pixmap*/
        struct timespec   focusDelay;  /**< how long to wait, before you focuse 
                                        * on window you want to be cloned*/
        struct timespec   frameDelay;  /**< how often should xwinclone refresh 
                                        * its content (frames per second)*/
        const char      * exitKeyStr;  /**< string representing exit key 
                                        * (keysymdef.h)*/
        const char      * translationCtrlKeyStr;  /**< string representing
                                                   *  translation control key
                                        * (keysymdef.h)*/
        KeyCode           exitKeyCode; /**< result of exit key string parsing*/
        int               exitKeyMask; /**< Exit key modifier according to X.h*/
        KeyCode           translationCtrlKeyCode; /**< result of translation
                                                   *  control key string
                                                   *  parsing*/
        int               translationCtrlKeyMask; /**< Translation control key
                                                   *  modifier according to
                                                   *  X.h*/
        Window            srcWinId;    /**< Default window id to be used as 
                                        * source*/
        int               isDaemon;    /**< If program considered to be run in 
                                        * daemon mode*/
        int               isSingleton; /**< Allow only one instance of program
                                        *  to be run, automatically set in
                                        *  daemon mode*/
        Display         * xDpy;
        Screen          * xScr;
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
    typedef struct XWCOptions_ XWCOptions;

    /** @var Bool X_ERROR
    @brief Contains the last Xlib's error code. 
    @warning Not thread safe!
     */
    extern Bool X_ERROR;

    /** @var int LOG_LVL
    @brief Sets logging level 0, 1, 2
     */
    extern int LOG_LVL;

    /**
     * Returns call-time state of X_ERROR variable. X_ERROR is always set 
     * to False.
     * @return Xlib's Bool with call-time state of X_ERROR
     * @sa XCloseDisplay()
     */
    Bool
    getXErrState (void);

    /**
     * Opens connection to default X display. Provides simple verbose.
     * @return A pointer to Xlib's Display struct or NULL if fails
     * @sa XCloseDisplay()
     */
    Display *
    openDefaultDisplay (void);

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
     * Tries to get currently focused window. It may NOT be the id of window 
     * you really see on the screen, it depends on toolkit beign used 
     * (GTK/QT/...)
     * @param[in] d Pointer to Xlib's Display data struct.
     * @return Window XID or 0 (Xlib's `None` macro) if no focused window found
     * or error occured.
     * @sa XGetInputFocus()
     */
    Window
    getFocusedWindow (Display * d);

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
    getTopWindow (Display * d,
                  Window    start);

    /**
     * Tries to find window, at or below the specified window, that has a 
     * WM_STATE property.
     * @param[in] d Pointer to Xlib's Display data struct.
     * @param[in] start Window xid of window where search begins
     * @return Window XID in case of success or if if no window found, 
     * or 0 (Xlib's `None` macro) if error occured.
     * @sa XmuClientWindow()
     */
    Window
    getNamedWindow (Display * d,
                    Window    start);
    /**
     * Prints window name as reported to window manager (ICCC WM_NAME).
     * @param[in] d Pointer to Xlib's Display data struct.
     * @param[in] w Window xid of window which WM_NAME should be printed
     * @sa XGetWMName(), XmbTextPropertyToTextList()
     */

    /**
     * Tries to get currently active window. Uses different approaches to get
     * proper active window xid that can be used to aquire window's image.
     * @param[in] d Pointer to Xlib's Display data struct.
     * @return Window XID or 0 (Xlib's `None` macro) if no active window found
     * or error occured.
     * @sa getFocusedWindow(), getTopWindow(), getNamedWindow()
     */
    Window
    getActiveWindow (Display    * d,
                     XWCOptions * prgCfg);

    /**
     * Prints window name as reported to window manager (ICCC WM_NAME).
     * @param[in] d Pointer to Xlib's Display data struct.
     * @param[in] w Window xid of window which WM_CLASS should be printed
     * @todo Add return value for result checking
     * @sa XAllocClassHint(), XGetClassHint()
     */
    void
    printWindowName (Display * d,
                     Window    w);
    /**
     * Prints window class as reported to window manager (ICCC WM_CLASS).
     * @param[in] d Pointer to Xlib's Display data struct.
     * @param[in] w Window xid of window which WM_CLASS should be printed
     * @todo Add return value for result checking
     * @sa XAllocClassHint(), XGetClassHint()
     */
    void
    printWindowClass (Display * d,
                      Window    w);

    /**
     * Prints window info (class and name).
     * @param[in] d Pointer to Xlib's Display data struct.
     * @param[in] w Window xid of window which info should be printed
     * @param[in] xWinAttr Pointer to XWindowAttributes data struct
     * @todo Add return value for result checking
     * @sa printWindowName(), printWindowClass(), XWindowAttributes
     */
    void
    printWindowInfo (Display           * d,
                     Window              w,
                     XWindowAttributes * xWinAttr);

    /**
     * Sets window name (ICCC WM_NAME). Usually seen in titlebar.
     * @param[in] d Pointer to Xlib's Display data struct.
     * @param[in] WID Window xid of window which name will be changed
     * @param[in] name Null-terminated array of chars with window name
     * @return Xlib's True on success, False otherwise
     * @sa XSetWMName(), XStringListToTextProperty()
     */
    Bool
    setWinTitlebar (Display    * d,
                    Window       WID,
                    const char * name);
    /**
     * Sets window class properties (ICCC WM_CLASS) which have two c-strings
     * first with permanent window name and second with the name of class.
     * @param[in] d Pointer to Xlib's Display data struct.
     * @param[in] WID Window xid of window which name will be changed
     * @param[in] permNameStr Null-terminated array of chars with permanent 
     * window name
     * @param[in] classStr Null-terminated array of chars with class name
     * @return Xlib's True on success, False otherwise
     * @sa XChangeProperty(), XStringListToTextProperty(), XA_WM_NAME, 
     * PropModeReplace
     */
    Bool
    setWindowClass (Display    * d,
                    Window       WID,
                    const char * permNameStr,
                    const char * classStr);

    /**
     * Prints out program version
     */
    void
    printVersion (void);

    /**
     * Processes string arguments. Allocates and fills up `XWCOptions` struct. 
     * Returns Null on error.
     * @param[in] argCnt Number of strings in arguments array.
     * @param[in] argArr Pointer to array of arguments's strings.
     * @param[in] d Pointer to Xlib's Display struct.
     * @return Pointer to `XWCOptions` or NULL in case of error.
     * @todo Add some heuristics to argument processing.
     */
    XWCOptions *
    processArgs (Display    *  d,
                 int           argCnt,
                 const char ** argArr);

    /**
     * Extracts pointer to screen where window belongs.
     * @param[in] d Pointer to Xlib's Display data struct.
     * @param[in] WID Window xid of window which name will be changed
     * @return Pointer to Xlib's Screen structure or NULL in case of error.
     */
    Screen *
    getScreenByWindowAttr (Display           * d,
                           XWindowAttributes * winAttr);

    /**
     * Searches for root window in Xlib's Screen struct.
     * @param[in] s Pointer to Xlib's Screen data struct
     * @return Xlib's window with root window xid for specified screen 
     * or Xlib's None macro in case of error
     */
    Window
    getRootWinOfScr (Screen * s);

    /**
     * Registers exit key combination for a given window.
     * @param[in] d Pointer to Xlib's Display data struct.
     * @param[in] WID Window xid of window where exit event will be grabbed
     * @param[in] prgCfg Data struct with program's configuration
     * @return Xlib's True on success, False otherwise
     */
    Bool
    grabExitKey (Display    * d,
                 Window       WID,
                 XWCOptions * prgCfg);

    /**
     * Registers translation control key combination for a given window.
     * @param[in] d Pointer to Xlib's Display data struct.
     * @param[in] WID Window xid of window where translation control event will 
     * be grabbed
     * @param[in] prgCfg Data struct with program's configuration
     * @return Xlib's True on success, False otherwise
     */
    Bool
    grabTranslationCtrlKey (Display    * d,
                            Window       WID,
                            XWCOptions * prgCfg);

    /**
     * Deregisters exit key combination for a given window.
     * @param[in] d Pointer to Xlib's Display data struct.
     * @param[in] WID Window xid of window where exit event was grabbed
     * @param[in] prgCfg Data struct with program's configuration
     */
    void
    ungrabExitKey (Display    * d,
                   Window       grabWin,
                   XWCOptions * prgCfg);

    /**
     * Deregisters translation control key combination for a given window.
     * @param[in] d Pointer to Xlib's Display data struct.
     * @param[in] WID Window xid of window where translation control event 
     * was grabbed
     * @param[in] prgCfg Data struct with program's configuration
     */
    void
    ungrabTranslationCtrlKey (Display    * d,
                              Window       grabWin,
                              XWCOptions * prgCfg);

    /**
     * Checks if x server we have connection to has composite extension of 
     * supported version
     * @param[in] d Pointer to Xlib's Display data struct.
     * @return Xlib's True on success, False otherwise
     */
    Bool
    chkCompExt (Display * d);

    /**
     * Tries to parse color string and allocate color struct.
     * @param[in] d Pointer to Xlib's Display data struct.
     * @param[in] prgCfg Data struct with program's configuration 
     * @param[in] s Pointer to Xlib's Screen data struct
     * @return Xlib's True on success, False otherwise
     */
    Bool
    parseColor (Display    * d,
                XWCOptions * prgCfg,
                Screen     * s);

    /**
     * Processes log messages according to current logging level.
     * @param[in] msg Pointer to null-terminated C-string with message.
     * @param[in] lvl Desired log level of message
     * @param[in] sequenced if this log msg is a part of sequence
     */
    void
    logCtr (const char * msg,
            int          lvl,
            Bool         sequenced);

    /**
     * Returns default root window of display
     * @param[in] d Pointer to Xlib's Display data struct.
     */
    Window
    getDefaultRootWindow (Display * d);

    /**
     * Checks if no other instance of this program is running.
     */
    Bool
    ifSingleInst (void);

    int
    getPressedComb (Display    * xDpy,
                    XWCOptions * cfg);

    Bool
    getVisualOfScr (Screen      * xScr,
                    int           depth,
                    XVisualInfo * xVisInfo);

    Bool
    bgImgPrepare (Display           * xDpy,
                  XWCOptions        * cfg,
                  Pixmap            * bgImgPm,
                  unsigned int      * bgImgWidth,
                  unsigned int      * bgImgHeight,
                  Window              bgImgRootWin,
                  XWindowAttributes * bgImgRootWinAttr,
                  XWindowAttributes * rootWinAttr);

    void
    printDrawableInfo (Display  * xDpy,
                       Drawable   drw);

#ifdef	__cplusplus
}
#endif

#endif
