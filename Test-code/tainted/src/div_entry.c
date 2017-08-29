#include "protype.h"

RD_COMM_MSG  g_stTaintedDiv[RD_MSG_MAX] = 
{
    { PID_PCU ,    PCU_ProcMsg , PCU_Timer1Sec },
    { PID_BPU ,    BPU_ProcMsg , BPU_Timer1Sec },
    { PID_TBM ,    TBM_ProcMsg , TBM_Timer1Sec }
};

UINT32 gul_PID_active;


UINT32 PCU_ProcMsg(MSG_BLOCK *pstMsg)
{
    UINT32 ulSenderId;
    UINT32 ulSenderPid;

    ulSenderId    = pstMsg->ulSenderCpuId;
    ulSenderPid   = pstMsg->ulSenderPid;
    DIV_quot_o2W(ulSenderId, ulSenderPid);
    return SUCCESS;
}

UINT32 PCU_Timer1Sec(MSG_BLOCK *pstMsg) 
{
    UINT32 ulSenderId;
    UINT32 ulSenderPid;

    ulSenderId    = pstMsg->ulSenderCpuId;
    ulSenderPid   = pstMsg->ulSenderPid;
    DIV_quot_o2(ulSenderId, ulSenderPid);    
    return SUCCESS;
}

UINT32 BPU_ProcMsg(MSG_BLOCK *pstMsg)
{
    UINT32 ulSenderId;
    MSG *pMsg ;
    
    ulSenderId    = pstMsg->ulSenderCpuId;
    pMsg = (MSG *)pstMsg->aucValue;    
    DIV_quot_msgW(ulSenderId, pMsg);    
    return SUCCESS;
}

UINT32 BPU_Timer1Sec(MSG_BLOCK *pstMsg)    
{
    UINT32 ulSenderId;
    MSG *pMsg ;
    
    ulSenderId    = pstMsg->ulSenderCpuId;
    pMsg = (MSG *)pstMsg->aucValue;    
    DIV_quot_msg(ulSenderId, pMsg);    
    return SUCCESS;
}

UINT32 TBM_ProcMsg(MSG_BLOCK *pstMsg)    
{
    UINT32 ulLgcPort = pstMsg->ulSenderCpuId;
    UINT32 *plForwardDelayAverageBase = &pstMsg->ulSenderPid;
    UINT32 *plForwardJitterMean = &pstMsg->ulReceiverPid;
    
    DIV_SetDelayW(ulLgcPort, plForwardDelayAverageBase, plForwardJitterMean);
    return SUCCESS;
}

UINT32 TBM_Timer1Sec(MSG_BLOCK *pstMsg)    
{
    UINT32 ulLgcPort = pstMsg->ulSenderCpuId;
    UINT32 *plForwardDelayAverageBase = &pstMsg->ulSenderPid;
    UINT32 *plForwardJitterMean = &pstMsg->ulReceiverPid;
    
    DIV_SetDelay(ulLgcPort, plForwardDelayAverageBase, plForwardJitterMean);
    return SUCCESS;
}
