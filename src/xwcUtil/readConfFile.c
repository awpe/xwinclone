/* Copyright (c) 2015, Anton Lantcevich
    
   This file is part of the xwinclone.
   <https://github.com/awpe/xwinclone>
  
   Xwinclone is released under the BSD 3-Clause License.
   You should have received a copy of the BSD 3-Clause
   License along with this software; if not see
   <https://github.com/awpe/xwinclone/blob/master/LICENSE.md>.  */

#include <xwcUtil.h>

Bool
readConfFile (XWCContext * ctx,
              arguments  * args)
{
    FILE * config;
    char buf[4096], fnameTmp[1024], c, argBuf[128], valBuf[256], * tmp;
    int  argNamePos, argValPos, linePos, a, b, d, e;
    argNames argNameTmp;
    Bool errRead, skipLine;

    logCtrl ("\tParsing config file\n", LOG_LVL_1, False);

    ctx->confFileName = (const char *) args->m_Args[CONFFILE]->m_Val;

    if (*ctx->confFileName == '~')
    {
        logCtrl ("\t\tgetting user directory", LOG_LVL_2, True);

        if (ctx->userDir == NULL)
        {
            logCtrl ("\t\tError: Cannot determine user home directory!",
                     LOG_LVL_NO, False);
            return False;
        }
        else
        {
            logCtrl ("\t\t\tsuccess", LOG_LVL_2, True);
        }

        snprintf (fnameTmp, sizeof (fnameTmp), "%s%s", ctx->userDir,
                  ctx->confFileName + 1);
    }
    else
    {
        snprintf (fnameTmp, sizeof (fnameTmp), "%s", ctx->confFileName);
    }

    if (checkFileMode (fnameTmp, F_OK) == False)
    {
        snprintf (buf, sizeof (buf), "\t\tFile %s doesn't exist or you don't "
                  "have access to specified directory!", fnameTmp);
        logCtrl (buf, LOG_LVL_NO, False);
        return False;
    }

    if (checkFileMode (fnameTmp, R_OK) == False)
    {
        snprintf (buf, sizeof (buf), "No read permission for file %s",
                  fnameTmp);
        logCtrl (buf, LOG_LVL_NO, False);
        return False;
    }

    if ((config = fopen (fnameTmp, "r")) == NULL)
    {
        snprintf (buf, sizeof (buf), "Error opening file '%s' for reading "
                  "configuration!\nError: %s\n", fnameTmp, strerror (errno));
        logCtrl (buf, LOG_LVL_NO, False);
        return False;
    }
    else
    {
        snprintf (buf, sizeof (buf), "\twill read config from %s\n", fnameTmp);
        logCtrl (buf, LOG_LVL_1, False);
    }

    /**************************************************************************/
    /*WELCOME TO HELL*/
    /**************************************************************************/
    /*I don't have time to think about better solution, forgive me :( */

    /**************************************************************************/

    enum
    {
        FIND_CHAR,
        READING_WORD,
        FIND_EQ
    } st;

    enum
    {
        NO_QST,
        SQ_OPENED,
        DQ_OPENED,
        SQ_CLOSED,
        DQ_CLOSED
    } qst;

    enum
    {
        FIND_ARG_NAME,
        FIND_ARG_VALUE,
        BREAK
    } argSt;

    errRead  = False;

    do
    {
        argNamePos = 0;
        argValPos  = 0;
        linePos    = 0;
        c          = 0;
        argSt      = FIND_ARG_NAME;
        st         = FIND_CHAR;
        skipLine   = False;
        qst        = NO_QST;
        do
        {
            c = fgetc (config);
            linePos ++;

            if (c == EOF || ferror (config))
            {
                break;
            }

            if (c == '#')
            {
                skipLine = True;
                while (c != '\n' && c != EOF && !ferror (config))
                {
                    c = fgetc (config);
                }
                break;
            }

            switch (argSt)
            {
                case FIND_ARG_NAME:
                    switch (st)
                    {
                        case FIND_CHAR:
                            if (isalpha (c))
                            {
                                st = READING_WORD;
                                argBuf[argNamePos] = c;
                                argNamePos++;
                            }
                            break;

                        case READING_WORD:
                            if (isalpha (c))
                            {
                                argBuf[argNamePos] = c;
                                argNamePos++;
                            }
                            else
                            {
                                argBuf[argNamePos] = '\0';
                                if (getArgName (args,
                                                argBuf,
                                                &argNameTmp) == False)
                                {
                                    snprintf (buf, sizeof (buf),
                                              "\tError parsing config file "
                                              "%s\n\tCannot find argument name "
                                              "%s in current argument list",
                                              fnameTmp, argBuf);
                                    logCtrl (buf, LOG_LVL_NO, False);
                                    errRead = True;
                                }
                                else if (args->m_Args[argNameTmp]->m_IsInConf
                                         == False)
                                {
                                    snprintf (buf, sizeof (buf),
                                              "\tError parsing config file "
                                              "%s\n\tArgument name %s is not "
                                              "meant to be put into config "
                                              "file", fnameTmp, argBuf);
                                    logCtrl (buf, LOG_LVL_NO, False);
                                }
                                else
                                {
                                    argSt = FIND_ARG_VALUE;
                                    st = FIND_EQ;
                                }
                            }
                            break;

                        default:
                            break;
                    }
                    break;

                case FIND_ARG_VALUE:
                    switch (st)
                    {
                        case FIND_CHAR:
                            if (isalpha (c) || isdigit (c))
                            {
                                st = READING_WORD;
                                valBuf[argValPos] = c;
                                argValPos++;
                            }
                            else if (c == '"')
                            {
                                st  = READING_WORD;
                                qst = DQ_OPENED;
                            }
                            else if (c == '\'')
                            {
                                st  = READING_WORD;
                                qst = SQ_OPENED;
                            }
                            break;

                        case FIND_EQ:
                            if (c == '=')
                            {
                                st = FIND_CHAR;
                            }
                            break;

                        case READING_WORD:

                            if (c == '"')
                            {
                                switch (qst)
                                {
                                    case DQ_OPENED:
                                        a = valBuf[argValPos - 1] != '\\';
                                        b = (c = fgetc (config)) == EOF;
                                        d = ferror (config);
                                        e = c != '\n';

                                        if ( a && (!b || !d) && e )
                                        {
                                            errRead = True;
                                        }
                                        else
                                        {
                                            qst = DQ_CLOSED;
                                            valBuf[argValPos] = '\0';
                                        }
                                        break;

                                    case SQ_OPENED:
                                        valBuf[argValPos] = c;
                                        argValPos++;
                                        break;

                                    default:
                                        break;
                                }
                            }
                            else if (c == '\'')
                            {
                                switch (qst)
                                {
                                    case DQ_OPENED:
                                        valBuf[argValPos] = c;
                                        argValPos++;
                                        break;

                                    case SQ_OPENED:
                                        if (valBuf[argValPos - 1] != '\\' &&
                                            !((c = fgetc (config)) == EOF &&
                                            !ferror (config))
                                            && c != '\n' )
                                        {
                                            errRead = True;
                                        }
                                        else
                                        {
                                            qst = SQ_CLOSED;
                                            valBuf[argValPos] = '\0';
                                        }
                                        break;

                                    default:
                                        break;
                                }
                            }
                            else if (c != '\n')
                            {
                                if (!isspace (c) || c == ' ')
                                {
                                    valBuf[argValPos] = c;
                                    argValPos++;
                                }
                            }
                            else
                            {
                                valBuf[argValPos] = '\0';
                            }
                            break;

                        default:
                            break;
                    }
                    break;

                default:
                    break;
            }
        }
        while (c != EOF && c != '\n' && !ferror (config) && errRead == False
               && skipLine == False && linePos < MAX_FILE_LINE_LENGTH);

        if (   errRead == False
            && !ferror (config)
            && skipLine == False
            && (argValPos != 0 && argNamePos != 0 ))
        {
            char * endPtr;
            int s;
            unsigned long l;
            if (args->m_Args[argNameTmp]->m_HasValue == False)
            {
                if (args->m_Args[argNameTmp]->m_IsSet == False)
                {
                    snprintf (buf, sizeof (buf), "\t\tReaded arg %-12s with "
                              "val %s", argBuf, valBuf);
                    logCtrl (buf, LOG_LVL_2, True);
                    args->m_Args[argNameTmp]->m_SetInConf = True;

                    tmp = valBuf;

                    for ( ; *tmp; ++tmp)
                    {
                        *tmp = tolower (*tmp);
                    }

                    if (   strcmp (valBuf, "y")    == STR_EQUAL
                        || strcmp (valBuf, "yes")  == STR_EQUAL
                        || strcmp (valBuf, "1")    == STR_EQUAL
                        || strcmp (valBuf, "true") == STR_EQUAL)
                    {
                        args->m_Args[argNameTmp]->m_IsSet = True;
                    }
                    else if (   strcmp (valBuf, "n")     == STR_EQUAL
                             || strcmp (valBuf, "no")    == STR_EQUAL
                             || strcmp (valBuf, "0")     == STR_EQUAL
                             || strcmp (valBuf, "false") == STR_EQUAL)
                    {
                        args->m_Args[argNameTmp]->m_IsSet = False;
                    }
                    else
                    {
                        snprintf (buf, sizeof (buf), "\t\tCannot parse value "
                                  "%s for boolean argument %s", valBuf, argBuf);
                        logCtrl (buf, LOG_LVL_NO, False);
                        errRead = True;
                    }
                }
                else
                {
                    snprintf (buf, sizeof (buf), "\t\tCannot assign value "
                              "%s to arg %s: Argument is set from command "
                              "prompt", valBuf, argBuf);
                    logCtrl (buf, LOG_LVL_NO, True);
                }
                continue;
            }

            switch (args->m_Args[argNameTmp]->m_Type)
            {
                case C_STR:
                    snprintf (buf, sizeof (buf), "\t\tReaded arg %-12s with "
                              "val %s", argBuf, valBuf);
                    logCtrl (buf, LOG_LVL_2, True);
                    if (args->m_Args[argNameTmp]->m_IsSet == False)
                    {
                        args->m_Args[argNameTmp]->m_NeedFree = True;
                        s = strlen (valBuf);
                        tmp = (char *) malloc (sizeof (char) * (s + 1));
                        memcpy (tmp, valBuf, s);
                        tmp[s] = '\0';
                        args->m_Args[argNameTmp]->m_Val = tmp;
                        args->m_Args[argNameTmp]->m_SetInConf = True;
                    }
                    else
                    {
                        snprintf (buf, sizeof (buf), "\t\tCannot assign value "
                                  "%s to arg %s: Argument is set from command "
                                  "prompt", valBuf, argBuf);
                        logCtrl (buf, LOG_LVL_NO, True);
                    }
                    break;

                case INT:
                    s = strtol (valBuf, &endPtr, 10);
                    if (endPtr == valBuf)
                    {
                        snprintf (buf, sizeof (buf), "\t\tError parsing value "
                                  "%s for argument %s!", valBuf, argBuf);
                        logCtrl (buf, LOG_LVL_NO, True);
                        errRead = True;
                    }
                    snprintf (buf, sizeof (buf), "\t\tReaded arg %-12s with "
                              "val %d", argBuf, s);
                    logCtrl (buf, LOG_LVL_2, True);
                    if (args->m_Args[argNameTmp]->m_IsSet == False)
                    {
                        *((int*) args->m_Args[argNameTmp]->m_Val) = s;
                        args->m_Args[argNameTmp]->m_SetInConf = True;
                    }
                    else
                    {
                        snprintf (buf, sizeof (buf), "\t\tCannot assign value "
                                  "%s to arg %s: Argument is set from command "
                                  "prompt", valBuf, argBuf);
                        logCtrl (buf, LOG_LVL_NO, True);
                    }
                    break;

                case ULONG:
                    l = strtol (valBuf, &endPtr, 0);
                    if (endPtr == valBuf)
                    {
                        snprintf (buf, sizeof (buf), "\t\tError parsing value "
                                  "%s for arguement %s!", valBuf, argBuf);
                        logCtrl (buf, LOG_LVL_NO, True);
                        errRead = True;
                    }
                    snprintf (buf, sizeof (buf), "\t\tReaded arg %-12s with "
                              "val %ld", argBuf, l);
                    logCtrl (buf, LOG_LVL_2, True);
                    if (args->m_Args[argNameTmp]->m_IsSet == False)
                    {
                        *((unsigned long*) args->m_Args[argNameTmp]->m_Val) = l;
                        args->m_Args[argNameTmp]->m_SetInConf = True;
                    }
                    else
                    {
                        snprintf (buf, sizeof (buf), "\t\tCannot assign value "
                                  "%s to arg %s: Argument is set from command "
                                  "prompt", valBuf, argBuf);
                        logCtrl (buf, LOG_LVL_NO, True);
                    }
                    break;

                default:
                    break;
            }
        }
    }
    while (c != EOF && !ferror (config) && errRead == False);

    if ((c == EOF && ferror (config)) || errRead == True)
    {
        fclose (config);
        return False;
    }

    fclose (config);

    logCtrl ("\t\tsuccess", LOG_LVL_2, True);

    return True;
}
