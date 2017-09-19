
#include "protype.h"

RD_COMM_MSG  g_stTaintedArray[RD_MSG_MAX] = 
{
    { TRCP_APP_ADD ,    TRCP_BmAppAddPrePro , TRCP_BmrcResPro },
    { TRCP_APP_DEL ,    TRCP_BmAppDelPrePro , TRCP_BmrcDelPro },
    { TRCP_APP_INS ,    TRCP_BmAppInsPrePro , TRCP_BmrcInsPro },
    { TRCP_APP_MOD ,    TRCP_BmAppModPrePro , TRCP_BmrcModPro },
    { TRCP_APP_QRY ,    TRCP_BmAppQryPrePro , TRCP_BmrcQryPro },

    { PID_CCM ,    CCM_ProcMsg , CCM_Timer1Sec },        
    { PID_RRM ,    RRM_ProcMsg , RRM_Timer1Sec },
    { PID_LAC ,    LAC_ProcMsg , LAC_Timer1Sec },
    { PID_MAC ,    MAC_ProcMsg , MAC_Timer1Sec }
       
};

/* thread entry routine */
UINT32 TRCP_BmAppAddPrePro(MSG_BLOCK *pstMsg)
{
    CRDLC_DATA_IND *pstMtlsMsgBody;
    UINT16 usCrdlcCcbNo;
    UINT16 usMtlsCcbNo;
    
    pstMtlsMsgBody = (CRDLC_DATA_IND *)&pstMsg->aucValue[0];

    usCrdlcCcbNo  = pstMtlsMsgBody->usUsrCcb;
    usMtlsCcbNo   = pstMtlsMsgBody->usAbsCcb;

    //noncompliant, the array indexes could be out-of-bound while calling L2cfg_procCfgMsgW
    D2_procCfgMsgW(usCrdlcCcbNo, usMtlsCcbNo ,pstMtlsMsgBody ); 

    return SUCCESS;
}


/* thread entry routine */
UINT32 TRCP_BmAppDelPrePro(MSG_BLOCK *pstMsg)
{
    CRDLC_DATA_IND *pstMtlsMsgBody;
    UINT16 usCrdlcCcbNo;
    UINT16 usMtlsCcbNo;
    
    pstMtlsMsgBody = (CRDLC_DATA_IND *)&pstMsg->aucValue[0];

    usCrdlcCcbNo  = pstMtlsMsgBody->usUsrCcb;
    usMtlsCcbNo   = pstMtlsMsgBody->usAbsCcb;

    if((usCrdlcCcbNo >= LTE_CELL_MAX ) || (usMtlsCcbNo >= LTE_OPT_MAX ))
    {
        return FAILURE;
    }
    D2_procCfgMsgW(usCrdlcCcbNo, usMtlsCcbNo ,pstMtlsMsgBody );

    return SUCCESS;
}

/* thread entry routine */
UINT32 TRCP_BmrcResPro(MSG_BLOCK *pstMsg)
{
    CRDLC_DATA_IND *pstMtlsMsgBody;
    UINT16 usCrdlcCcbNo;
    UINT16 usMtlsCcbNo;
    
    pstMtlsMsgBody = (CRDLC_DATA_IND *)&pstMsg->aucValue[0];

    usCrdlcCcbNo  = pstMtlsMsgBody->usUsrCcb;
    usMtlsCcbNo   = pstMtlsMsgBody->usAbsCcb;

    D2_procCfgMsg(usCrdlcCcbNo, usMtlsCcbNo ,pstMtlsMsgBody );

    return SUCCESS;
}


/* thread entry routine */
UINT32 TRCP_BmrcDelPro(MSG_BLOCK *pstMsg)
{
    CRDLC_DATA_IND *pstMtlsMsgBody;
    UINT16 usCrdlcCcbNo;
    
    pstMtlsMsgBody = (CRDLC_DATA_IND *)&pstMsg->aucValue[0];
    usCrdlcCcbNo  = pstMtlsMsgBody->usUsrCcb;
    D1_cellAccessW(  usCrdlcCcbNo );

    return SUCCESS;
}


UINT32 TRCP_BmAppInsPrePro(MSG_BLOCK *pstMsg) 
{
    CRDLC_DATA_IND *pstMtlsMsgBody;
    UINT16 usCrdlcCcbNo;
    
    pstMtlsMsgBody = (CRDLC_DATA_IND *)&pstMsg->aucValue[0];
    usCrdlcCcbNo  = pstMtlsMsgBody->usUsrCcb;
    D1_cellAccess(  usCrdlcCcbNo );    
    return SUCCESS;
}

UINT32 TRCP_BmAppModPrePro(MSG_BLOCK *pstMsg)
{
    CRDLC_DATA_IND *pstMtlsMsgBody;
    UINT16 usCrdlcCcbNo;
    UINT16 usMtlsCcbNo;
    
    pstMtlsMsgBody = (CRDLC_DATA_IND *)&pstMsg->aucValue[0];

    usCrdlcCcbNo  = pstMtlsMsgBody->usUsrCcb;
    usMtlsCcbNo   = pstMtlsMsgBody->usAbsCcb;
    D1_cellAccessSubW( usCrdlcCcbNo, usMtlsCcbNo);
    return SUCCESS;
}
    
UINT32 TRCP_BmrcInsPro(MSG_BLOCK *pstMsg)
{
    CRDLC_DATA_IND *pstMtlsMsgBody;
    UINT16 usCrdlcCcbNo;
    UINT16 usMtlsCcbNo;
    
    pstMtlsMsgBody = (CRDLC_DATA_IND *)&pstMsg->aucValue[0];

    usCrdlcCcbNo  = pstMtlsMsgBody->usUsrCcb;
    usMtlsCcbNo   = pstMtlsMsgBody->usAbsCcb;
    D1_cellAccessSub( usCrdlcCcbNo, usMtlsCcbNo);
    return SUCCESS;
}
    
UINT32 TRCP_BmrcModPro(MSG_BLOCK *pstMsg)
{
    return SUCCESS;
}
    
UINT32 TRCP_BmAppQryPrePro(MSG_BLOCK *pstMsg)
{
    return SUCCESS;
}

UINT32 TRCP_BmrcQryPro(MSG_BLOCK *pstMsg)
{
    return SUCCESS;
}





/* * * * * * * * * ** * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* routines below  are about cases where tainted data is from return of a */
/* call                                                                   */
/* * * * * * * * * ** * * * * * * * * * * * * * * * * * * * * * * * * * * */

UINT32 CCM_ProcMsg(MSG_BLOCK *pstMsg)
{
    CRDLC_DATA_IND *pstMtlsMsgBody;
    
    pstMtlsMsgBody = (CRDLC_DATA_IND *)&pstMsg->aucValue[0];
    SET_BulkCfgStartW((void*)pstMtlsMsgBody);
    return SUCCESS;
}

UINT32 CCM_Timer1Sec(MSG_BLOCK *pstMsg)
{
    CRDLC_DATA_IND *pstMtlsMsgBody;
    
    pstMtlsMsgBody = (CRDLC_DATA_IND *)&pstMsg->aucValue[0];    
    SET_BulkCfgStart((void*)pstMtlsMsgBody);
    return SUCCESS;
}

UINT32 RRM_ProcMsg(MSG_BLOCK *pstMsg)
{
    CRDLC_DATA_IND *pstMtlsMsgBody;
    
    pstMtlsMsgBody = (CRDLC_DATA_IND *)&pstMsg->aucValue[0];        
    SET_BulkCfgStartMod((void*)pstMtlsMsgBody);
    return SUCCESS;
}

UINT32 RRM_Timer1Sec(MSG_BLOCK *pstMsg)
{
    MSG_BLOCK *pNewMsg;
    MSG_UM    *pUm;
    
    pNewMsg = VOS_AllocMsg(PID_RRM, VOS_MSG_LEN);
    if(NULL == pNewMsg)
        return FAILURE;

    pUm  = (MSG_UM*)pNewMsg->aucValue;
    pUm->category[pstMsg->ucIpcMsgType] = pstMsg->usAckNum;

    VOS_SendMsg((void *)pNewMsg);
    return SUCCESS;
}

UINT32 LAC_ProcMsg(MSG_BLOCK *pstMsg)
{
    return SUCCESS;
}

UINT32 LAC_Timer1Sec(MSG_BLOCK *pstMsg)
{
    return SUCCESS;
}

UINT32 MAC_ProcMsg(MSG_BLOCK *pstMsg) 
{
    return SUCCESS;
}

UINT32 MAC_Timer1Sec(MSG_BLOCK *pstMsg)
{
    return SUCCESS;
}

