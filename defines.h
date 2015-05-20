#ifndef DEFINES_H
#define	DEFINES_H

#ifdef	__cplusplus
extern "C" {
#endif

#define _POSIX_C_SOURCE                              200112L
#define FRAMERATE_FPS                                60  //fps
#define TIME_TO_CHANGE_FOCUS_SEC                     2   //s
#define TIME_TO_WAIT_FOR_SRCWIN_TO_BECOME_VISIBLE_MS 500 //ms
#define DEFAULT_BG_COLOR                             "#0000ff" //rgb color
#define EXIT_KEY                                     XK_c // from keysymdef.h
#define EXIT_MASK                                    ControlMask // from X.h
#define DEF_EXIT_KOMBINATION_STR                     "CTRL + c"
#define WM_CLASS_PRG_NAME_STR                        "XWinClone"
#define WM_CLASS_CLASS_NAME_STR                      "xwinclone"
#define AUTOCENTERING                                1 //1=true; 0=false
#define TOP_OFFSET                                   71 // in pixels
#define PROGRAM_NAME_STR                             "XWinClone"

#ifdef	__cplusplus
}
#endif

#endif

