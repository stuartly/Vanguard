
#include "protype.h"

RD_COMM_MSG  g_stTaintedFor[RD_MSG_MAX] = 
{
 
    { PID_RPS ,    RPS_ProcMsg , RPS_Timer1Sec },        
    { PID_CDR ,    CDR_ProcMsg , CDR_Timer1Sec },
    { PID_BCM ,    BCM_ProcMsg , BCM_Timer1Sec },
    { PID_RCM ,    RCM_ProcMsg , RCM_Timer1Sec }
       
};

UINT32 RPS_ProcMsg(MSG_BLOCK *pstMsg)
{
    CRDLC_DATA_IND *pstMtlsMsgBody;
    UINT16 usCrdlcCcbNo;
    
    pstMtlsMsgBody = (CRDLC_DATA_IND *)&pstMsg->aucValue[0];
    usCrdlcCcbNo  = pstMtlsMsgBody->usUsrCcb;
    D1_cellAccessForIV(usCrdlcCcbNo); 
        
    return SUCCESS;
}    

UINT32 RPS_Timer1Sec(MSG_BLOCK *pstMsg)
{
    CRDLC_DATA_IND *pstMtlsMsgBody;
    UINT16 usCrdlcCcbNo;
    
    pstMtlsMsgBody = (CRDLC_DATA_IND *)&pstMsg->aucValue[0];
    usCrdlcCcbNo  = pstMtlsMsgBody->usUsrCcb;
    D1_cellAccessForBV(usCrdlcCcbNo); 
    
    return SUCCESS;
}

UINT32 CDR_ProcMsg(MSG_BLOCK *pstMsg)
{
    CRDLC_DATA_IND *pstMtlsMsgBody;
    UINT16 usCrdlcCcbNo;
    
    pstMtlsMsgBody = (CRDLC_DATA_IND *)&pstMsg->aucValue[0];
    usCrdlcCcbNo  = pstMtlsMsgBody->usUsrCcb;
    D1_cellAccessFor(usCrdlcCcbNo);     
    return SUCCESS;    
}

UINT32 CDR_Timer1Sec(MSG_BLOCK *pstMsg)
{
    CRDLC_DATA_IND *pstMtlsMsgBody;
    UINT16 usCrdlcCcbNo;
    UINT16 usMtlsCcbNo;
    
    pstMtlsMsgBody = (CRDLC_DATA_IND *)&pstMsg->aucValue[0];

    usCrdlcCcbNo  = pstMtlsMsgBody->usUsrCcb;
    usMtlsCcbNo   = pstMtlsMsgBody->usAbsCcb;

    D2_procCfgMsgWF(usCrdlcCcbNo, usMtlsCcbNo ,pstMtlsMsgBody );

    return SUCCESS;
}

UINT32 BCM_ProcMsg(MSG_BLOCK *pstMsg)
{
    CRDLC_DATA_IND *pstMtlsMsgBody;
    UINT16 usCrdlcCcbNo;
    UINT16 usMtlsCcbNo;
    
    pstMtlsMsgBody = (CRDLC_DATA_IND *)&pstMsg->aucValue[0];

    usCrdlcCcbNo  = pstMtlsMsgBody->usUsrCcb;
    usMtlsCcbNo   = pstMtlsMsgBody->usAbsCcb;

    D2_procCfgMsgFR(usCrdlcCcbNo, usMtlsCcbNo ,pstMtlsMsgBody );

    return SUCCESS;
}

UINT32 BCM_Timer1Sec(MSG_BLOCK *pstMsg)
{
    CRDLC_DATA_IND *pstMtlsMsgBody;
    UINT16 usCrdlcCcbNo;
    
    pstMtlsMsgBody = (CRDLC_DATA_IND *)&pstMsg->aucValue[0];
    usCrdlcCcbNo  = pstMtlsMsgBody->usUsrCcb;
    llcm_BackupCallInfo(usCrdlcCcbNo); 
        
    return SUCCESS;
}

UINT32 RCM_ProcMsg(MSG_BLOCK *pstMsg)  
{
    CRDLC_DATA_IND *pstMtlsMsgBody;
    UINT16 usCrdlcCcbNo;
    
    pstMtlsMsgBody = (CRDLC_DATA_IND *)&pstMsg->aucValue[0];
    usCrdlcCcbNo  = pstMtlsMsgBody->usUsrCcb;
    llcm_InitCallInfo(usCrdlcCcbNo);
    
    return SUCCESS;
}
    
UINT32 RCM_Timer1Sec(MSG_BLOCK *pstMsg)
{
    return SUCCESS;
}



