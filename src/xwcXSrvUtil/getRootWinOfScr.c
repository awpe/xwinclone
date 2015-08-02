#include <xwcXSrvUtil.h>

Window
getRootWinOfScr (Screen * s)
{
    logCtrl ("Getting root window of screen:", LOG_LVL_1, False);
    if (s == NULL)
    {
        logCtrl ("\tError getting root window of screen: "
                 "Invalid pointer to Screen data struct!", LOG_LVL_NO, True);
        return None;
    }

    if (s->root == None)
    {
        logCtrl ("\tError getting root window of screen: "
                 "No root wondow specified for given screen!", LOG_LVL_NO,
                 True);
    }
    logCtrl ("\tsuccess", LOG_LVL_1, True);
    return s->root;
}
