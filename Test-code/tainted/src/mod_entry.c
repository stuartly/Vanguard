#include "protype.h"

RD_COMM_MSG  g_stTaintedfor[RD_MSG_MAX] = 
{
    { PID_MTP ,    MTP_ProcMsg , MTP_Timer1Sec },
    { PID_CIE ,    CIE_ProcMsg , CIE_Timer1Sec },        

    { PID_PIM ,    PIM_ProcMsg , PIM_Timer1Sec },
    { PID_PIE ,    PIE_ProcMsg , PIE_Timer1Sec },
    { PID_BIM ,    BIM_ProcMsg , BIM_Timer1Sec },
    { PID_BIE ,    BIE_ProcMsg , BIE_Timer1Sec },
        
    { PID_TIM ,    TIM_ProcMsg , TIM_Timer1Sec },
    { PID_TIE ,    TIE_ProcMsg , TIE_Timer1Sec },
};


UINT32 MTP_ProcMsg(MSG_BLOCK *pstMsg) 
{
    return SUCCESS;
}

UINT32 MTP_Timer1Sec(MSG_BLOCK *pstMsg) 
{
    return SUCCESS;
}

UINT32 CIE_ProcMsg(MSG_BLOCK *pstMsg) 
{
    return SUCCESS;
}

UINT32 CIE_Timer1Sec(MSG_BLOCK *pstMsg) 
{
    return SUCCESS;
}

UINT32 PIM_ProcMsg(MSG_BLOCK *pstMsg)
{
    UINT32 ulSenderId;
    UINT32 ulSenderPid;

    ulSenderId    = pstMsg->ulSenderCpuId;
    ulSenderPid   = pstMsg->ulSenderPid;
    MOD_quot_o2W(ulSenderId, ulSenderPid);
    return SUCCESS;
}
    
UINT32 PIM_Timer1Sec(MSG_BLOCK *pstMsg)
{
    UINT32 ulSenderId;
    UINT32 ulSenderPid;

    ulSenderId    = pstMsg->ulSenderCpuId;
    ulSenderPid   = pstMsg->ulSenderPid;
    MOD_quot_o2(ulSenderId, ulSenderPid);
    return SUCCESS;
}
    
UINT32 PIE_ProcMsg(MSG_BLOCK *pstMsg)
{
    UINT32 ulSenderId;
    MSG *pMsg ;
    
    ulSenderId    = pstMsg->ulSenderCpuId;
    pMsg = (MSG *)pstMsg->aucValue;    
    MOD_quot_msgW(ulSenderId, pMsg);
    return SUCCESS;
}
    
UINT32 PIE_Timer1Sec(MSG_BLOCK *pstMsg)
{
    UINT32 ulSenderId;
    MSG *pMsg ;
    
    ulSenderId    = pstMsg->ulSenderCpuId;
    pMsg = (MSG *)pstMsg->aucValue;    
    MOD_quot_msg(ulSenderId, pMsg);
    return SUCCESS;
}
    
UINT32 BIM_ProcMsg(MSG_BLOCK *pstMsg)
{
    UINT32 ulSenderId;
    
    ulSenderId    = pstMsg->ulSenderCpuId;
    vos_strtouq((const char *)pstMsg->aucValue, ulSenderId);
    return SUCCESS;
}
    
UINT32 BIM_Timer1Sec(MSG_BLOCK *pstMsg)
{
    return SUCCESS;
}
    
UINT32 BIE_ProcMsg(MSG_BLOCK *pstMsg)
{
    return SUCCESS;
}
    
UINT32 BIE_Timer1Sec(MSG_BLOCK *pstMsg)
{
    return SUCCESS;
}

UINT32 TIM_ProcMsg(MSG_BLOCK *pstMsg)
{
    return SUCCESS;
}

UINT32 TIM_Timer1Sec(MSG_BLOCK *pstMsg)
{
    return SUCCESS;
}

UINT32 TIE_ProcMsg(MSG_BLOCK *pstMsg)  
{
    return SUCCESS;
}    

UINT32 TIE_Timer1Sec(MSG_BLOCK *pstMsg) 
{
    return SUCCESS;
}






