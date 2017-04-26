#include <stdio.h>
#include "type.h"
#include "project.inc"

PFM_CFG_LOGPORT_STRU     *g_astLgcPrtInfo = NULL;
extern UINT32 gul_PID_active;

void  DIV_quot_o2W(UINT32 i, UINT32 j )
{
    UINT32 quot;
    
    quot =  (i / j);  //non-compliant  ,  j could  equal to zero
    printf("quot is: %d\n", quot);
    
}

void  DIV_quot_o2(UINT32 i, UINT32 j )
{
    UINT32 quot;

    if(j==0)
        j = 5;
    
    quot =  (i / j);  //compliant  ,  j doesn't equal to zero
    printf("quot is: %d\n", quot);
    
}


void DIV_quot_msgW(UINT32 i, MSG  *msg)
{
    UINT32 quot;

    quot =  (i / msg->msg_len);  //non-compliant 
    printf("quot is: %d\n", quot);
    
}



void DIV_quot_msg(UINT32 i, MSG  *msg)
{
    UINT32 quot;

    if(msg->msg_len == 0 )
    {
		return;
    }
	
    quot =  (i / msg->msg_len);  //compliant .  msg->msg_len doesn't equal to zero
    printf("quot is: %d\n", quot);

    quot =  i / gul_PID_active;   //non-compliant ?
    printf("quot1 is: %d\n", quot);    
}

void DIV_SetDelay(UINT32 ulLgcPort, UINT32 * plForwardDelayAverageBase, UINT32 *plForwardJitterMean)
{

    UINT32 ulLatUpExp = 1;
    UINT32 ulLatDownExp = 1;
    UINT32 lBackwardDelayAverageBaseLowUp = 0;
        
    if(PFM_MAX_LGCPRT_NO > ulLgcPort)
    {
        if((g_astLgcPrtInfo[ulLgcPort].usLatUpExp >= PFM_IPPM_UP_DOWN_EXP_CHK ) ||
           (g_astLgcPrtInfo[ulLgcPort].usLatDownExp >= PFM_IPPM_UP_DOWN_EXP_CHK ))  
        {
            ulLatUpExp   = ulLatUpExp   << 8;
            ulLatDownExp = ulLatDownExp << 3;
        }
        else
        {
            ulLatUpExp   =  ulLatUpExp << g_astLgcPrtInfo[ulLgcPort].usLatUpExp;
            ulLatDownExp =  ulLatDownExp << g_astLgcPrtInfo[ulLgcPort].usLatDownExp;
        }
    }
    else
    {
        ulLatUpExp   = ulLatUpExp   << 8;
        ulLatDownExp = ulLatDownExp << 3;
    }

    
    *plForwardDelayAverageBase  = *plForwardJitterMean / ulLatUpExp;   //compliant
    lBackwardDelayAverageBaseLowUp = *plForwardJitterMean % ulLatUpExp;//compliant

}

void DIV_SetDelayW(UINT32 ulLgcPort, UINT32 * plForwardDelayAverageBase, UINT32 *plForwardJitterMean)
{

    UINT32 ulLatUpExp = 1;
    UINT32 ulLatDownExp = 1;
    UINT32 lBackwardDelayAverageBaseLowUp = 0;
        
    if(PFM_MAX_LGCPRT_NO > ulLgcPort)
    {
        ulLatUpExp   =  ulLatUpExp << g_astLgcPrtInfo[ulLgcPort].usLatUpExp;
        ulLatDownExp =  ulLatDownExp << g_astLgcPrtInfo[ulLgcPort].usLatDownExp;
    }
    else
    {
        ulLatUpExp   = ulLatUpExp   << 8;
        ulLatDownExp = ulLatDownExp << 3;
    }

    
    *plForwardDelayAverageBase  = *plForwardJitterMean / ulLatUpExp;   //non-compliant
    lBackwardDelayAverageBaseLowUp = *plForwardJitterMean % ulLatUpExp;//non-compliant

}


