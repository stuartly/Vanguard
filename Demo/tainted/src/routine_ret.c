#include "type.h"



UINT32 devGetDfrInfo(MSG* pMsg)
{
    return pMsg->RecvPid;
}

void SET_BulkCfgStartW(void*  pstrMuxStartMsg)
{
    UINT32 dfrInfo ;
    BULKCFGMSG    * pBulkStartMsg;

    pBulkStartMsg  =  (BULKCFGMSG *) pstrMuxStartMsg ;
    dfrInfo = devGetDfrInfo(pBulkStartMsg->pMsg);
    pBulkStartMsg->usCpuNo[dfrInfo ]  = 1;  //noncompliant, dfrInfo could be out of bound
}


void SET_BulkCfgStart(void*  pstrMuxStartMsg)
{
    UINT32 dfrInfo ;
    BULKCFGMSG    * pBulkStartMsg;

    pBulkStartMsg  =  (BULKCFGMSG *) pstrMuxStartMsg ;
    dfrInfo = devGetDfrInfo(pBulkStartMsg->pMsg);
    if(dfrInfo >= MAX_CPU)
    {
        return;
    }
    pBulkStartMsg->usCpuNo[dfrInfo ]  = 1;  //compliant, dfrInfo is valid
}


void SET_BulkCfgStartMod(void*  pstrMuxStartMsg)
{
    UINT32 dfrInfo ;
    BULKCFGMSG    * pBulkStartMsg;

    pBulkStartMsg  =  (BULKCFGMSG *) pstrMuxStartMsg ;
    dfrInfo = devGetDfrInfo(pBulkStartMsg->pMsg);

    dfrInfo %= MAX_CPU;
	
    pBulkStartMsg->usCpuNo[dfrInfo ]  = 1;  //compliant, dfrInfo is valid
}




