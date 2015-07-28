#include "xwc.h"

/*Global X_ERROR initialization*/
Bool X_ERROR      = False;
int  X_ERROR_CODE = 0;

imgLibCreateCrSc_t   imgLibCreateCrSc = & imlib_create_cropped_scaled_image;

XCompRedirWin_t      redirWin         = & XCompositeRedirectWindow;
XCompUnRedirWin_t    unRedirWin       = & XCompositeUnredirectWindow;
XCompRedirSubWin_t   redirSubWin      = & XCompositeRedirectSubwindows;
XCompUnRedirSubWin_t unRedirSubWin    = & XCompositeUnredirectSubwindows;

/*Global LOG_LVL initialization*/
int LOG_LVL = DEFAULT_LOG_LVL;

FILE * LOG_FILE = NULL;

void
printVersion (void)
{
    char buf[1024];
    snprintf (buf, sizeof (buf), "%s version %s", WM_CLASS_PRG_NAME_STR,
              XWINCLONE_VERSION_STR);
    logCtr (buf, LOG_LVL_NO, False);
}

void
logCtr (const char * msg,
        int          lvl,
        Bool         sequenced)
{
    if (msg == NULL)
    {
        if (LOG_FILE != NULL)
        {
            fprintf (LOG_FILE, "Tryng to log NULL msg!\n");
        }
        else
        {
            printf ("Tryng to log NULL msg!\n");
        }
        return;
    }

    if (lvl < 0)
    {
        if (LOG_FILE != NULL)
        {
            fprintf (LOG_FILE,  "Tryng to log with unknown lvl!\n");
        }
        else
        {
            printf ( "Tryng to log with unknown lvl!\n");
        }
        return;
    }

    if (LOG_LVL >= lvl)
    {
        if (sequenced == False)
        {
            if (LOG_FILE != NULL)
            {
                fprintf (LOG_FILE, "\n%s\n", msg);
            }
            else
            {
                printf ( "\n%s\n", msg);
            }
        }
        else
        {
            if (LOG_FILE != NULL)
            {
                fprintf (LOG_FILE, "%s\n", msg);
            }
            else
            {
                printf ( "%s\n", msg);
            }
        }
    }
}

Bool
ifSingleInst (XWCContext * ctx)
{
    char buf[1024];
    int  rc;

    logCtr ("Creating lock file:", LOG_LVL_1, False);

    if (ctx == NULL)
    {
        logCtr ("\tError: null pointer to context!", LOG_LVL_NO, True);
    }

    ctx->lckFD = open (ctx->lckFPath, O_CREAT | O_RDWR, 0666);

    if (ctx == NULL)
    {
        snprintf (buf, sizeof (buf), "\tError: cannot create lock file %s!",
                  ctx->lckFPath);
        logCtr (buf, LOG_LVL_NO, True);
    }

    rc = flock (ctx->lckFD, LOCK_EX | LOCK_NB);

    if (rc != 0 && EWOULDBLOCK == errno)
    {

        snprintf (buf, sizeof (buf), "File %s seems to be already created and"
                  " locked,\nassuming other instance of this program is"
                  " running", ctx->lckFPath);
        logCtr (buf, LOG_LVL_NO, False);
        return False;
    }

    return True;
}

Bool
bgImgPrepare (XWCContext        * ctx,
              Pixmap            * bgImgPm,
              unsigned int      * bgImgWidth,
              unsigned int      * bgImgHeight)
{
    Imlib_Image imgSrc, imgScaled;
    char buf[1024];

    if (ctx->bgImgFileSet == True)
    {
        logCtr ("Reading background image file:", LOG_LVL_NO, False);
    }
    else
    {
        logCtr ("Reading background image file:", LOG_LVL_1, False);
    }

    imgSrc = imlib_load_image (ctx->bgImgFilePath);

    if (imgSrc == NULL)
    {
        snprintf (buf, sizeof (buf), "\tcannot load background image file"
                  " '%s'!", ctx->bgImgFilePath);

        if (ctx->bgImgFileSet == True)
        {
            logCtr (buf, LOG_LVL_NO, True);
            return False;
        }
        else
        {
            logCtr (buf, LOG_LVL_1, True);
        }
    }
    else
    {
        imlib_context_set_image (imgSrc);
        *bgImgWidth  = imlib_image_get_width ();
        *bgImgHeight = imlib_image_get_height ();

        if (   *bgImgWidth  > ctx->rootWAttr.width
            || * bgImgHeight > ctx->rootWAttr.height)
        {
            float scaleFactor = (float) *bgImgWidth / (float) *bgImgHeight;

            int newWidth  = ctx->rootWAttr.width;
            int newHeight = (float) newWidth / scaleFactor;

            snprintf (buf, sizeof (buf), "Image scaled to:\n\twidth:\t%d\n\t"
                      "height:\t%d", newWidth, newHeight);
            logCtr (buf, LOG_LVL_1, True);

            imgScaled = imgLibCreateCrSc (0, 0, *bgImgWidth, *bgImgHeight,
                                          newWidth, newHeight);
            //imlib_free_image_and_decache ();
            imlib_free_image ();
            imlib_context_set_image (imgScaled);
            *bgImgWidth  = imlib_image_get_width ();
            *bgImgHeight = imlib_image_get_height ();
        }
        else
        {
            imgScaled = imgSrc;
        }


        *bgImgPm = XCreatePixmap (ctx->xDpy, ctx->trgW, *bgImgWidth,
                                  *bgImgHeight, ctx->trgWAttr.depth);

        imlib_context_set_display (ctx->xDpy);
        imlib_context_set_visual (ctx->trgWAttr.visual);
        imlib_context_set_colormap (ctx->trgWAttr.colormap);
        imlib_context_set_drawable (*bgImgPm);

        imlib_render_image_on_drawable (0, 0);

        //imlib_free_image_and_decache ();
        imlib_free_image ();

        if (getXErrState (ctx) == True)
        {
            return False;
        }

        ctx->bgImgStatus = True;

        if (ctx->bgImgFileSet == True)
        {
            logCtr ("\tsuccess", LOG_LVL_NO, True);
        }
        else
        {
            logCtr ("\tsuccess", LOG_LVL_1, True);
        }
    }
    return True;
}

void
printBlock (const char * str,
            const char * linePrefix,
            const char * blockPrefix)
{
    char buf[1024], indentStr[1024];
    int  prefixLengthCols, printedChars, tmp, i, extraCharCnt, blockPreLen;
    int  maxWordComplete, lineMaxLength, maxLen, strLen, prefixLenBytes;
    const char * tmpChar;

    blockPreLen      = blockPrefix != NULL ? strlen (blockPrefix) : 1;
    prefixLenBytes   = strlen (linePrefix);
    prefixLengthCols = prefixLenBytes;
    tmpChar          = linePrefix;
    while ((*tmpChar) != '\0')
    {
        if ((*tmpChar) == '\t')
        {
            prefixLengthCols += 7;
        }

        tmpChar ++;
    }
    maxLen          = OUTPUT_MAX_COLUMNS - (blockPreLen + prefixLengthCols) - 10;
    lineMaxLength   = OUTPUT_MAX_COLUMNS - 10;
    strLen          = strlen (str);
    maxWordComplete = 10;
    printedChars    = 0;

    snprintf (indentStr, sizeof (indentStr), "%%s%%%ds%%s", blockPreLen);
    i = 0;
    while (printedChars < strLen)
    {
        if (blockPrefix != NULL && i == 0)
        {
            tmp = snprintf (buf, lineMaxLength, "%s%s%s", linePrefix,
                            blockPrefix, str);
        }
        else
        {
            if (*(str + printedChars) == ' ')
            {
                printedChars ++;
            }
            tmp = snprintf (buf, lineMaxLength, indentStr, linePrefix, " ",
                            str + printedChars);
        }

        if (tmp > lineMaxLength)
        {
            printedChars += maxLen - 1;
            tmp = maxLen - 1 + blockPreLen + prefixLenBytes;
            extraCharCnt = 0;

            while (   (* (str + printedChars)) != '\0'
                   && (* (str + printedChars)) != ','
                   && (* (str + printedChars)) != ';'
                   && (* (str + printedChars)) != ' '
                   && (* (str + printedChars)) != '\t'
                   && (* (str + printedChars)) != '\n'
                   && extraCharCnt < maxWordComplete)
            {
                buf[tmp + extraCharCnt] = * (str + printedChars);
                printedChars ++;
                extraCharCnt ++;
            }

            buf[tmp + extraCharCnt] = * (str + printedChars);

            printedChars ++;

            extraCharCnt ++;

            buf[tmp + extraCharCnt] = '\0';
        }
        else
        {
            printedChars += tmp - (blockPreLen + prefixLenBytes);
        }

        printf ("%s\n", buf);
        i ++;
    }
}
