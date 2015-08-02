#include <xwcArgs.h>
#include <xwcHeaders.h>

arguments *
initArgs ()
{
    arguments * args = (arguments*) malloc (sizeof (arguments ));

    if (args == NULL)
    {
        return NULL;
    }

    args->m_ArgCnt = OPTIONS_COUNT;
    args->m_Args   = (argument**) malloc (sizeof (argument ) * args->m_ArgCnt);

    for (int i = 0; i < args->m_ArgCnt; ++ i)
    {
        args->m_Args[i] = (argument*) malloc (sizeof (argument ));
        memset (args->m_Args[i], 0, sizeof (*args->m_Args[i] ));
        args->m_Args[i]->m_NameStr   = NULL;
        args->m_Args[i]->m_Comment   = NULL;
        args->m_Args[i]->m_SynStrs   = NULL;
        args->m_Args[i]->m_Val       = NULL;
        args->m_Args[i]->m_HasValue  = False;
        args->m_Args[i]->m_IsInConf  = False;
        args->m_Args[i]->m_SetInConf = False;
    }

    return args;
}
