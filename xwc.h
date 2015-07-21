#ifndef XWCLIB_H
#define	XWCLIB_H

#include "defines.h"
#include "headers.h"

/****************************************/
/*Redefine imlib2 functions*/
/****************************************/
typedef
Imlib_Image (*imgLibCreateCrSc_t)(int, int, int, int, int, int) ;
/****************************************/

/** @var int LOG_LVL
@brief Sets logging level 0, 1, 2
 */
extern int LOG_LVL;

extern FILE * LOG_FILE;

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
 * Prints out program version
 */
void
printVersion (void);

/**
 * Checks if no other instance of this program is running.
 */
Bool
ifSingleInst (XWCContext * ctx);

Bool
bgImgPrepare (XWCContext        * ctx,
              Pixmap            * bgImgPm,
              unsigned int      * bgImgWidth,
              unsigned int      * bgImgHeight);

#endif

