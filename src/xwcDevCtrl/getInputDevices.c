/* Copyright (c) 2015, Anton Lantcevich
    
   This file is part of the xwinclone.
   <https://github.com/awpe/xwinclone>
  
   Xwinclone is released under the BSD 3-Clause License.
   You should have received a copy of the BSD 3-Clause
   License along with this software; if not see
   <https://github.com/awpe/xwinclone/blob/master/LICENSE.md>.  */

#include <xwcDevCtrl.h>

Bool
getInputDevices (XWCContext * ctx)
{
    XIDeviceInfo * allDevsInfo, * dev;
    char           buf[1024];
    int            i, * kbdIds, masterKbdCnt, nAllDevs;

    logCtrl ("\tBuiding list of input devices", LOG_LVL_1, False);

    if (ctx == NULL)
    {
        logCtrl ("\t\tCannot get list of all devices: NULL pointer to program"
                 " context received!", LOG_LVL_NO, False);
        return False;
    }

    nAllDevs    = 0;
    kbdIds      = NULL;
    allDevsInfo = XIQueryDevice (ctx->xDpy, XIAllDevices, &nAllDevs);

    if (allDevsInfo != NULL)
    {
        kbdIds = (int*) malloc (sizeof (int) * nAllDevs);

        if (kbdIds == NULL)
        {
            logCtrl ("\t\tCannot get list of all devices: cannot allocate "
                     "memory for device's id array!", LOG_LVL_NO, False);
            XIFreeDeviceInfo (allDevsInfo);
            return False;
        }

        masterKbdCnt = 0;

        for (i = 0; i < nAllDevs; ++ i)
        {
            dev = & allDevsInfo[i];

            switch (dev->use)
            {
                case XIMasterKeyboard:
                    kbdIds[masterKbdCnt] = dev->deviceid;
                    ++ masterKbdCnt;
                    break;

                case XISlavePointer:
                    if (ctx->procBtnEv == True)
                    {
                        if (strncmp (dev->name, ctx->ptrDevName,
                                     MAX_POINTER_DEV_NAME_LENGTH) == STR_EQUAL)
                        {
                            ctx->slavePtrDevId  = dev->deviceid;
                            ctx->masterPtrDevId = dev->attachment;
                        }
                    }
                    break;

                default:
                    break;
            }
        }

        XIFreeDeviceInfo (allDevsInfo);

        if (ctx->slavePtrDevId == NO_DEVICE && ctx->procBtnEv == True)
        {
            snprintf (buf, sizeof (buf), "\t\tCannot get list of all devices: "
                      "no slave pointer with name('%s') found, see '$ xinput "
                      "list' to find propriate pointer name for slave pointer!",
                      ctx->ptrDevName);
            logCtrl (buf, LOG_LVL_NO, False);
            free (kbdIds);
            return False;
        }

        ctx->kbds = (DevList *) malloc (sizeof (DevList));

        if (ctx->kbds == NULL)
        {
            logCtrl ("\t\tCannot get list of all devices: Cannot allocate "
                     "memory for device list!", LOG_LVL_NO, False);
            free (kbdIds);
            return False;
        }

        ctx->kbds->devs  = kbdIds;
        ctx->kbds->nDevs = masterKbdCnt;
    }
    else
    {
        logCtrl ("\t\tCannot get list of all devices: XIQueryDevice error!",
                 LOG_LVL_NO, False);
        return False;
    }

    logCtrl ("\t\tsuccess",
             LOG_LVL_2, True);

    return True;
}
