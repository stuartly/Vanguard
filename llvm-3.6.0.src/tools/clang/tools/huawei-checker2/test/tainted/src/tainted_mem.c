#include <stdio.h>

#include "protype.h"

#define memset  VOS_MemSet
#define MemSet  VOS_MemSet

void *gpAlCCB = NULL;
UINT8 acFileName[FTM_MAX_FULL_FILE_NAME_LEN]    = {0};   

UINT32 PID_StaticMsgInit(MSG_BLOCK *pMsg)
{
    UINT16 usLen;
   
    if(usLen<1000000000)
    usLen = pMsg->uwLength;
    gpAlCCB = VOS_MemAlloc(usLen, usLen, usLen); //noncompliant, usLen is tainted

  
}



UINT32 PID_StaticMsgInit2(MSG_BLOCK *pMsg)
{
    UINT16 usLen;
   
    if(usLen<66666666666)
    usLen = pMsg->uwLength;
    gpAlCCB = VOS_MemAlloc(usLen, usLen, usLen); //noncompliant, usLen is tainted

  
}



UINT32 PID_DosMsgInitor(MSG_BLOCK *pMsg)
{
    
    UINT16 usLen;  
    
    usLen = pMsg->uwLength;
    gpAlCCB = VOS_MemAlloc(usLen, usLen, usLen); //noncompliant, usLen is tainted
 
    
}
UINT32 PID_DosMsgInit(MSG_BLOCK *pMsg)
{
   

    PID_StaticMsgInit(pMsg); 
    PID_StaticMsgInit2(pMsg); 
    PID_DosMsgInitor(pMsg);
    
}



void CALD_StartUpFileReq( void* pstMsg)
{
    UINT32 ulLoop     = 0;
    UINT32 ulCaldIp   = 0; 
    FTM_FTP_PARA_STRU     stFtpPara; 
    char       acFileName[FTM_MAX_FULL_FILE_NAME_LEN]    = {0};  
    CFG_MTN_REQ_STRU     *pstMtn  = (CFG_MTN_REQ_STRU *)pstMsg ; 

    for (ulLoop = 0; ulLoop < pstMtn->ulAttrNum; ulLoop++)
    {
        switch (pstMtn->pstTlvPara[ulLoop].ulParaId)
        {
             case CMOID_FTPSERVERIP:
                 VOS_MemCpy((void *)&ulCaldIp, (void *)pstMtn->pstTlvPara[ulLoop].pucValue, sizeof(UINT32));
                 break;
            case CMOID_FTPUSERNAME:
                  VOS_MemCpy((void *)stFtpPara.acFtpUserName, pstMtn->pstTlvPara[ulLoop].usLen, pstMtn->pstTlvPara[ulLoop].usLen);   //noncompliant. copy length could be out of bound of source buffer
                  break;
            case CMOID_FTPPWD:
                  VOS_MemCpy((void *)stFtpPara.acFtpPwd, (void *)pstMtn->pstTlvPara[ulLoop].pucValue,pstMtn->pstTlvPara[ulLoop].usLen);  //noncompliant.   copy length could be out of bound of source buffer
                  break;
            case CMOID_SRCFILENAME:
                  VOS_MemCpy((void *)acFileName, (void *)pstMtn->pstTlvPara[ulLoop].pucValue, pstMtn->pstTlvPara[ulLoop].usLen);  //noncompliant. copy length could be out of bound of acFileName
                  break;
            default:
                 printf(" Unknow CMOID! CMOID=0x%x ",pstMtn->pstTlvPara[ulLoop].ulParaId);
                 break;
        }
     }
	 
}


void CALD_StartDldFileReq( void* pstMsg)
{
    UINT32 ulLoop      = 0;
    UINT32 ulCaldIp    = 0; 
    FTM_FTP_PARA_STRU     stFtpPara; 
    char   acFileName[FTM_MAX_FULL_FILE_NAME_LEN]    = {0};   

    CFG_MTN_REQ_STRU  *pstMtn  = (CFG_MTN_REQ_STRU *)pstMsg ; 


    for (ulLoop = 0; ulLoop < pstMtn->ulAttrNum; ulLoop++)
    {
        switch (pstMtn->pstTlvPara[ulLoop].ulParaId)
        {
            case CMOID_FTPSERVERIP:
                VOS_MemCpy((void *)&ulCaldIp, (void *)pstMtn->pstTlvPara[ulLoop].pucValue, sizeof(UINT32));
                break;

            case CMOID_FTPUSERNAME:
		        if(pstMtn->pstTlvPara[ulLoop].usLen >= MAX_ID_LEN)
	    	    {
  		            printf("Ftp User Name Len too long!");
            }
             else
	   	      {			
                  VOS_MemCpy((void *)stFtpPara.acFtpUserName, (void *)pstMtn->pstTlvPara[ulLoop].pucValue, pstMtn->pstTlvPara[ulLoop].usLen); //compliant 
	           }
            break;

            case CMOID_FTPPWD:
    	       if(pstMtn->pstTlvPara[ulLoop].usLen >= MAX_ID_LEN)
	    	   {
    	           printf("Ftp Pwd Len too long!");
    		   }
               else
	       	 {
                   VOS_MemCpy((void *)stFtpPara.acFtpPwd, (void *)pstMtn->pstTlvPara[ulLoop].pucValue,pstMtn->pstTlvPara[ulLoop].usLen);  //compliant 
		       }
               break;

            case CMOID_SRCFILENAME:
                if(pstMtn->pstTlvPara[ulLoop].usLen >= FTM_MAX_FULL_FILE_NAME_LEN)
    	    	{
    			    printf("File Name Len too long!");
    		    }
    		    else
    	   	    {
                     VOS_MemCpy((void *)acFileName, (void *)pstMtn->pstTlvPara[ulLoop].pucValue, pstMtn->pstTlvPara[ulLoop].usLen);     //compliant 
    		    }		   
                break;

            default:
                 printf(" Unknow CMOID! CMOID=0x%x ",pstMtn->pstTlvPara[ulLoop].ulParaId);
                 break;
        }
     }	
}


UINT16 ALG_EncodeL3RawInfo(UINT8 *pucMsgCode)
{
    UINT16 usMsgLen;
    UINT8 *pucMsg;
    UINT16 usBitNum;
    UINT8 ucNcellNum;
    UINT16 i;
    UINT8 ucSpareBitNum,ucBitPos;
    

    pucMsg = pucMsgCode;
    usMsgLen = 0;    

    *pucMsg = EI_ABIS_L3_INFO;
    pucMsg++;
    usMsgLen++;
        
    pucMsg += 2;   
    usMsgLen += 2;

    *pucMsg = PD_RR; 
    pucMsg++;
    usMsgLen++;

    *pucMsg = 0x7f; 
    pucMsg++;
    usMsgLen++;

    return usMsgLen;
}

void ALG_HandleRawMrFreq(ABIS_MEASURE_REPORT *pstAbisMeasureReport,  MTLS_MSG_HEADER_STRU *pstMtlsMsgHeader, UINT8 *pFinalMsg)
{
    MTLS_ALG_RSLDATA_IND_STRU      *pstMtlsMsgBody; 
    UINT8 *pucMsgHead = NULL;
    UINT16 usL3InfoLenFreq;
    UINT16 usMsgLenHeadToL3 = 0;
    UINT8 aucL3InfoFreq[MAX_LEN_OF_MR_INFOR];

    pstMtlsMsgBody = (MTLS_ALG_RSLDATA_IND_STRU *)&pstMtlsMsgHeader->aucMsg[0];
    pucMsgHead = (UINT8 *)pstMtlsMsgHeader;

    VOS_MemSet(aucL3InfoFreq, 0, MAX_LEN_OF_MR_INFOR);
    usL3InfoLenFreq = ALG_EncodeL3RawInfo(aucL3InfoFreq);

    usMsgLenHeadToL3 = (UINT16)(pstAbisMeasureReport->pucL3StartPos - pucMsgHead);
    
    VOS_MemCpy(pFinalMsg + usMsgLenHeadToL3, aucL3InfoFreq, usL3InfoLenFreq); //noncompliant, usL3InfoLenFreq


}


void ALG_HandleMetaMrFreq(ABIS_MEASURE_REPORT *pstAbisMeasureReport,  MTLS_MSG_HEADER_STRU  *pstMtlsMsgHeader, UINT8 *pFinalMsg)
{
    MTLS_ALG_RSLDATA_IND_STRU      *pstMtlsMsgBody; 
    UINT8 *pucMsgHead = NULL;
    UINT16 usL3InfoLenFreq;
    UINT16 usMsgLenHeadToL3 = 0;
    UINT8 aucL3InfoFreq[MAX_LEN_OF_MR_INFOR];

    pstMtlsMsgBody = (MTLS_ALG_RSLDATA_IND_STRU *)&pstMtlsMsgHeader->aucMsg[0];
    pucMsgHead = (UINT8 *)pstMtlsMsgHeader;

    VOS_MemSet(aucL3InfoFreq, 0, MAX_LEN_OF_MR_INFOR);
    usL3InfoLenFreq = ALG_EncodeL3RawInfo(aucL3InfoFreq);

    usMsgLenHeadToL3 = (UINT16)(pstAbisMeasureReport->pucL3StartPos - pucMsgHead);
    
    if ((usMsgLenHeadToL3 > MAX_LEN_OF_MR_INFOR)
         ||(usL3InfoLenFreq > MAX_LEN_OF_MR_INFOR)
         ||((usMsgLenHeadToL3 + usL3InfoLenFreq) > MAX_LEN_OF_MR_INFOR))
    {   
        return ;
    }

    VOS_MemCpy(pFinalMsg + usMsgLenHeadToL3, aucL3InfoFreq, usL3InfoLenFreq); //compliant


}



extern UINT8* xrpc_get_request_arg(UINT32);
extern UINT32 common_handshake_get_tasknum(UINT32,UINT32*);
void monitor_handshake_request_handler(UINT32 context)
{
    UINT32 loop;
    UINT32 taskno;
    UINT32 taskNum;
    UINT8 *pucTaskNum = NULL;
    UINT8 *pucTaskNo  = NULL;
    UINT8 *pucTemp    = NULL;
    char  acTask[LMT_ARRAY_LEN_SMALL] = {0};

    if(FAILURE == common_handshake_get_tasknum(context,&taskNum))
        return;
    
    if(NULL==(pucTemp=(UINT8*)xrpc_get_request_arg(context)))
        return;
    
    pucTaskNo = pucTemp;
    for(loop=0;loop<taskNum;loop++)
    {
       if(NULL==(pucTemp=(UINT8*)strstr((char*)pucTaskNo,",")))
           return;
       else
       {
          memset(acTask,0,LMT_ARRAY_LEN_SMALL);
          strncpy(acTask,(char*)pucTaskNo, pucTemp - pucTaskNo); //non-compliant
          taskno = htonl((UINT32)atoi(acTask));          
          pucTaskNo = pucTemp + 1;
      }
   }

}      







