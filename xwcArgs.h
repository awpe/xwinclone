#ifndef XWCARGS_H
#define	XWCARGS_H

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
    SINGLEINST  = 9,
    BGIMAGE     = 10,
    LCKFPATH    = 11,
    PTRDEVNAME  = 12,
    LOGFNAME    = 13,
    RAISETIME   = 14,
    RESTORETIME = 15,
    CLICKTIME   = 16,
    LONGWAIT    = 17,
    /****************************************/
    /*Write count of possible arguments here*/
    OPTIONS_COUNT = 18
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
    Bool          m_IsSet;
    Bool          m_HasValue;
    void       *  m_Value;
    const char *  m_Comment;
} argument;

typedef struct arguments_
{
    int         m_ArgCnt;
    argument ** m_Args;
} arguments;


void
delArgs (arguments * args);

arguments *
initArgs ();

Bool
addArg (arguments  * args,
        Bool         hasValue,
        argTypes     type,
        argNames     name,
        const char * nameStr,
        const char * comment,
        int          argSynCnt, ...);

void
printCurValues (arguments  * args);

void
printUsage (arguments  * args);

#endif