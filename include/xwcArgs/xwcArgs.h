/* Copyright (c) 2015, Anton Lantcevich
    
   This file is part of the xwinclone.
   <https://github.com/awpe/xwinclone>
  
   Xwinclone is released under the BSD 3-Clause License.
   You should have received a copy of the BSD 3-Clause
   License along with this software; if not see
   <https://raw.githubusercontent.com/awpe/xwinclone/master/LICENSE.md>.  */

#ifndef XWCARGS_H
#define	XWCARGS_H

#include <defines.h>
#include <systemHeaders.h>

typedef enum argNames_
{
    HELP        = 0,
    FRAMERATE   = 1,
    BGCOLOR     = 2,
    AUTOCENTER  = 3,
    FOCUSTIME   = 4,
    TOPOFFSET   = 5,
    LOGLVL      = 6,
    SOURCEID    = 7,
    DAEMON      = 8,
    MULTIINST   = 9,
    BGIMAGE     = 10,
    LCKFPATH    = 11,
    PTRDEVNAME  = 12,
    LOGFNAME    = 13,
    RAISETIME   = 14,
    RESTORETIME = 15,
    CLICKTIME   = 16,
    LONGWAIT    = 17,
    PROCBTNEV   = 18,
    CONFFILE    = 19,
    MKCONFIG    = 20,
    EXITKEY     = 21,
    CLONEKEY    = 22,
    CHECKARGS   = 23,
    /****************************************/
    /*Write count of possible arguments here*/
    OPTIONS_COUNT = 24
    /****************************************/
} argNames;

typedef enum argTypes_
{
    C_STR, INT, ULONG
} argTypes;

typedef struct argument_
{
    int           m_SynCnt;
    const char ** m_SynStrs;
    argTypes      m_Type;
    argNames      m_Name;
    const char *  m_NameStr;
    Bool          m_IsSet; //i.e. by user or config
    Bool          m_ConfFAllow; //i.e. if it CAN be set in config
    Bool          m_HasValue;
    Bool          m_NeedFree;
    Bool          m_SetInConf;
    void       *  m_Val;
    const char *  m_Comment;
    Bool          m_IsInConf;
} argument;

typedef struct arguments_
{
    int         m_ArgCnt;
    argument ** m_Args;
} arguments;


void
delArgs (arguments * args);

arguments *
initArgs (void);

Bool
addArg (arguments  * args,
        Bool         hasValue,
        Bool         usedInConfig,
        argTypes     type,
        argNames     name,
        const char * nameStr,
        const char * comment,
        int          argSynCnt, ...);

Bool
populateArgs (arguments * args);

Bool
parseArgs (const char ** argArr,
           arguments  *  args,
           int           argCnt);

Bool
argSetDefaultVal (argument * arg);

#endif
