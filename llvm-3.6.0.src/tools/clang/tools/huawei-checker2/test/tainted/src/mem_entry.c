#include <stdio.h>

#include "protype.h"


TRANS_MSG  g_stTaintedMem[RD_MSG_MAX] = 
{
 
    { PID_SAAL ,     SAAL_ProcMsg , SAAL_Timer1sec },        
    { PID_MTP3B ,    MTP3B_ProcMsg, MTP3B_Timer1sec},
    { PID_SCCP ,     SCCP_ProcMsg , SCCP_Timer1sec },
    { PID_QAAL2 ,    QAAL2_ProcMsg, QAAL2_Timer1sec }
       
};

UINT32 SAAL_ProcMsg(MSG_BLOCK *pstMsg, VOS_BLOCK *pstVos)
{
    if(pstMsg == NULL)
        return FAILURE;
    return PID_StaticMsgInit(pstMsg);

}

UINT32 MTP3B_ProcMsg(MSG_BLOCK *pstMsg, VOS_BLOCK *pstVos)
{
    if(pstMsg == NULL)
        return FAILURE;
    return PID_DosMsgInit(pstMsg);
}

UINT32 SCCP_ProcMsg(MSG_BLOCK *pstMsg, VOS_BLOCK *pstVos) 
{
    if(pstMsg == NULL)
        return FAILURE;

    CALD_StartUpFileReq( (void*) pstMsg);

    return SUCCESS;
}

UINT32 QAAL2_ProcMsg(MSG_BLOCK *pstMsg, VOS_BLOCK *pstVos)
{
    if(pstMsg == NULL)
        return FAILURE;

    CALD_StartDldFileReq( (void*) pstMsg);

    return SUCCESS;

}

UINT32 SAAL_Timer1sec(MSG_BLOCK *pstMsg, VOS_BLOCK *pstVos, UINT8 *pucMsg)
{
    if((pstMsg == NULL) || (pstVos == NULL) || (pucMsg == NULL))
        return FAILURE;
    
    ALG_HandleRawMrFreq((ABIS_MEASURE_REPORT *)pstMsg,  (MTLS_MSG_HEADER_STRU *)pstVos, pucMsg);
    return SUCCESS;
    
}

UINT32 MTP3B_Timer1sec(MSG_BLOCK *pstMsg, VOS_BLOCK *pstVos, UINT8 *pucMsg)
{
    UINT8 ucMsg[MAX_LEN_OF_MR_INFOR];
    
   
    VOS_MemSet(ucMsg, 0, MAX_LEN_OF_MR_INFOR);
    ALG_HandleMetaMrFreq((ABIS_MEASURE_REPORT *)pstMsg,  (MTLS_MSG_HEADER_STRU *)pstVos, ucMsg);
    return SUCCESS;
}

UINT32 SCCP_Timer1sec(MSG_BLOCK *pstMsg, VOS_BLOCK *pstVos, UINT8 *pucMsg)
{
    return SUCCESS;
}

UINT32 QAAL2_Timer1sec(MSG_BLOCK *pstMsg, VOS_BLOCK *pstVos, UINT8 *pucMsg)
{

    return SUCCESS;
}


