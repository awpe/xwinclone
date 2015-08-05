#include <xwcUtil.h>

Bool
mkdirRec (const char * dir)
{
    Bool retVal;
    int dirLen, pathPrevLen;
    char * pathCopy, * pathPrevTmp, * pathPrev, * pathCopyTmp;

    retVal      = False;
    pathCopy    = NULL;
    pathPrev    = NULL;
    pathPrevTmp = NULL;
    pathCopyTmp = NULL;

    if (dir == NULL)
    {
        return False;
    }

    if (   strcmp (dir, ".") == STR_EQUAL
        || strcmp (dir, "/") == STR_EQUAL)
    {
        return True;
    }

    dirLen = strlen (dir);

    pathCopy = (char *) malloc (sizeof (char) * (dirLen + 1));

    if (pathCopy == NULL)
    {
        retVal = False;
        goto ret;
    }

    memcpy (pathCopy, dir, dirLen);
    pathCopy[dirLen] = '\0';


    pathCopyTmp = (char *) malloc (sizeof (char) * (dirLen + 1));

    if (pathCopyTmp == NULL)
    {
        retVal = False;
        goto ret;
    }

    memcpy (pathCopyTmp, dir, dirLen);
    pathCopyTmp[dirLen] = '\0';

    pathPrevTmp = dirname (pathCopyTmp);

    if (pathPrevTmp == NULL)
    {
        retVal = False;
        goto ret;
    }

    pathPrevLen = strlen (pathPrevTmp);

    pathPrev = (char *) malloc (sizeof (char) * (pathPrevLen + 1));

    if (pathPrev == NULL)
    {
        retVal = False;
        goto ret;
    }

    memcpy (pathPrev, pathPrevTmp, pathPrevLen);
    pathPrev[pathPrevLen] = '\0';

    if (mkdirRec (pathPrev) == False && errno != EEXIST)
    {
        logCtrl (strerror (errno), LOG_LVL_NO, False);
        goto ret;
    }

    if (   mkdir (pathCopy, CONFIG_FILE_DIR_CREATE_MODE) == - 1
        && errno != EEXIST)
    {
        logCtrl (strerror (errno), LOG_LVL_NO, False);
        retVal = False;
    }
    else
    {
        retVal = True;
    }

ret:
    if (pathCopy != NULL)
    {
        free (pathCopy);
    }

    if (pathPrev != NULL)
    {
        free (pathPrev);
    }

    if (pathCopyTmp != NULL)
    {
        free (pathCopyTmp);
    }

    return retVal;
}
