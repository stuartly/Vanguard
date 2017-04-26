
#include "protype.h"

typedef struct tagstDbaObjDsBuf
{
    UINT16  usObjNum;  
    UINT16  uspad;     
    UINT32  ulBufLen;  
    UINT32  ulActLen;  
    UINT16* pusAVLenList;
    UINT8*  pucData;  
} MIT_CFGDATA_STRU;


extern UINT32 CDM_QueryObj(UINT32 ulCid, UINT32 bIsNewDb, void * pstCond, MIT_CFGDATA_STRU * pstCfgDataList);

UINT32 CC_UpgCheckRrDataVerIsExist(UINT32 ulNewDbFlag)
{
    UINT32 ulResult;
    UINT32 ulRet = FAILURE;

    MIT_CFGDATA_STRU stRrDataVerList;

    VOS_MemSet(&stRrDataVerList, 0x00, sizeof(MIT_CFGDATA_STRU));

    ulResult = CDM_QueryObj(CID_RR_RRDATAVER,  ulNewDbFlag, NULL, &stRrDataVerList);
    if ((SUCCESS == ulResult) && (1 == stRrDataVerList.usObjNum))
    {
        ulRet = SUCCESS;
    }

    if (NULL != stRrDataVerList.pucData)
    {
        VOS_MemFree(CID_RR_RRDATAVER, stRrDataVerList.pucData);  //compliant
        stRrDataVerList.pucData = NULL;
    }

    return ulRet;
}


UINT32 CC_UpgCheckRrData(UINT32 ulNewDbFlag)
{
    UINT32 ulResult;
    UINT32 ulRet = FAILURE;

    MIT_CFGDATA_STRU stRrDataVerList;

    VOS_MemSet(&stRrDataVerList, 0x00, sizeof(MIT_CFGDATA_STRU));

    ulResult = CDM_QueryObj(CID_RR_RRDATAVER,  ulNewDbFlag, NULL, &stRrDataVerList);
    if ((SUCCESS == ulResult) && (1 == stRrDataVerList.usObjNum))
    {
        ulRet = SUCCESS;
    }

    if (NULL != stRrDataVerList.pucData)
    {
        VOS_MemFree(CID_RR_RRDATAVER, stRrDataVerList.pucData);  //non-compliant
    }

    return ulRet;
}

MIT_CFGDATA_STRU g_stRrDataVerList;
UINT32 A1_DanglingPointerGlobalData(UINT32 ulNewDbFlag)
{
    UINT32 ulResult;
    UINT32 ulRet = FAILURE;

    ulResult = CDM_QueryObj(CID_RR_RRDATAVER,  ulNewDbFlag, NULL, &g_stRrDataVerList);
    if ((SUCCESS == ulResult) && (1 == g_stRrDataVerList.usObjNum))
    {
        ulRet = SUCCESS;
    }

    if (NULL != g_stRrDataVerList.pucData)
    {
        VOS_MemFree(CID_RR_RRDATAVER, g_stRrDataVerList.pucData);  //compliant
        g_stRrDataVerList.pucData = NULL;
    }

    return ulRet;
}

UINT32 A1_DanglingPointerGlobalDataUnrel(UINT32 ulNewDbFlag)
{
    UINT32 ulResult;
    UINT32 ulRet = FAILURE;

    ulResult = CDM_QueryObj(CID_RR_RRDATAVER,  ulNewDbFlag, NULL, &g_stRrDataVerList);
    if ((SUCCESS == ulResult) && (1 == g_stRrDataVerList.usObjNum))
    {
        ulRet = SUCCESS;
    }

    if (NULL != g_stRrDataVerList.pucData)
    {
        VOS_MemFree(CID_RR_RRDATAVER, g_stRrDataVerList.pucData);  //non-compliant
    }

    return ulRet;
}





