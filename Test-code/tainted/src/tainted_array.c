#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "type.h"



UINT32  g_ulRbDl[LTE_CELL_MAX ][ LTE_OPT_MAX ];
UINT32  g_ulCell[LTE_CELL_MAX ];


void  D2_procCfgMsgW(UINT32  cellid,  UINT32  cnOpIdx,   CRDLC_DATA_IND *pMsg)
{ 
    g_ulRbDl[cellid][cnOpIdx] = pMsg->usDataLen;  
  
}


void  D2_procCfgMsg(UINT32  cellid,  UINT32  cnOpIdx,   CRDLC_DATA_IND *pMsg)
{
       if((cellid >= LTE_CELL_MAX ) || (cnOpIdx >= LTE_OPT_MAX ))
       {
              return;
       }
       g_ulRbDl[cellid][cnOpIdx] = pMsg->usDataLen;  //compliant.  Array indexes are valid

}

void  D1_cellAccessW(UINT16  cellid )
{    
    g_ulCell[cellid] = 0;  //noncompliant
}

void  D1_cellAccess(UINT16  cellid )
{    
    if(cellid >= LTE_CELL_MAX)
        return;
    g_ulCell[cellid] = 0;
}

void  D1_cellAccessSubW(UINT16 minuend ,UINT16 subtrahend)
{
    if(subtrahend > minuend)
        g_ulCell[subtrahend - minuend] = 0;    //non-compliant. the substraction is not validated
    else
        g_ulCell[ minuend - subtrahend] = 0;   //non-compliant. the substraction is not validated     
}


void  D1_cellAccessSub(UINT16 minuend ,UINT16 subtrahend)
{
    if(subtrahend > minuend) 
        if((subtrahend-minuend) < LTE_CELL_MAX)
            g_ulCell[subtrahend - minuend] = 0;    //compliant
    else
        if((minuend - subtrahend) < LTE_CELL_MAX)
            g_ulCell[ minuend - subtrahend] = 0;   //compliant
}


/*******  memory copy   *******/
#if 0
#define   FTM_MAX_FULL_FILE_NAME_LEN  100
#define   MAX_ID_LEN  100
#define   MAX_ATTR_LEN  100

typedef enum  tagCMOID
{
       CMOID_FTPSERVERIP,
	    CMOID_FTPUSERNAME,
       CMOID_FTPPWD,
       CMOID_SRCFILENAME,
       CMOID_NUM
}CMOID;


typedef struct tagtlvpara
{
     char *pucValue ;
     UINT32  ulParaId;
     UINT16 usLen;
     UINT16 usRsv;
}TLVPARA;


typedef struct  tagcfgmtnreq
{
    UINT32 ulAttrNum;
    TLVPARA  *pstTlvPara;
}CFG_MTN_REQ_STRU;

typedef struct  tagftmftpppara
{
     char  acFtpUserName[MAX_ID_LEN];
     char  acFtpPwd[MAX_ID_LEN];
	 
}FTM_FTP_PARA_STRU;

UINT32 CALD_StartDldFileReqW( void* pstMsg)
{
    UINT32 ulLoop           = 0;
    UINT32 ulCaldIp               = 0; 
    FTM_FTP_PARA_STRU     stFtpPara; 
    char       acFileName[FTM_MAX_FULL_FILE_NAME_LEN]    = {0};  
    CFG_MTN_REQ_STRU     *pstMtn  = (CFG_MTN_REQ_STRU *)pstMsg ; 

    for (ulLoop = 0; ulLoop < pstMtn->ulAttrNum; ulLoop++)
    {
        switch (pstMtn->pstTlvPara[ulLoop].ulParaId)
        {
             case CMOID_FTPSERVERIP:
                 memcpy((void *)&ulCaldIp, (void *)pstMtn->pstTlvPara[ulLoop].pucValue, sizeof(UINT32));
                 break;
            case CMOID_FTPUSERNAME:
                  memcpy((void *)stFtpPara.acFtpUserName, (void *)pstMtn->pstTlvPara[ulLoop].pucValue, pstMtn->pstTlvPara[ulLoop].usLen);   //non-compliant   copy length could be out of bound of source buffer
                  break;

            case CMOID_FTPPWD:
                  memcpy((void *)stFtpPara.acFtpPwd, (void *)pstMtn->pstTlvPara[ulLoop].pucValue,pstMtn->pstTlvPara[ulLoop].usLen);  //non-compliant   copy length could be out of bound of source buffer
                  break;

            case CMOID_SRCFILENAME:
                  memcpy((void *)acFileName, (void *)pstMtn->pstTlvPara[ulLoop].pucValue, pstMtn->pstTlvPara[ulLoop].usLen);     //non-compliant   copy length could be out of bound of acFileName
                  break;
            default:
                 printf(" Unknow CMOID! CMOID=0x%x ",pstMtn->pstTlvPara[ulLoop].ulParaId);
                 break;
        }
     }
     return 0;	
}


UINT32 CALD_StartDldFileReq( void* pstMsg)
{
    UINT32 ulLoop           = 0;
    UINT32 ulCaldIp               = 0; 
    FTM_FTP_PARA_STRU     stFtpPara; 
    char       acFileName[FTM_MAX_FULL_FILE_NAME_LEN]    = {0};   

    CFG_MTN_REQ_STRU     *pstMtn  = (CFG_MTN_REQ_STRU *)pstMsg ; 


    for (ulLoop = 0; ulLoop < pstMtn->ulAttrNum; ulLoop++)
    {
        switch (pstMtn->pstTlvPara[ulLoop].ulParaId)
        {
            case CMOID_FTPSERVERIP:
                memcpy((void *)&ulCaldIp, (void *)pstMtn->pstTlvPara[ulLoop].pucValue, sizeof(UINT32));
                break;

            case CMOID_FTPUSERNAME:
                if(pstMtn->pstTlvPara[ulLoop].usLen >= MAX_ID_LEN)
	    	    {
  		             printf("Ftp User Name Len too long!");
                }
                else
                {				
                    memcpy((void *)stFtpPara.acFtpUserName, (void *)pstMtn->pstTlvPara[ulLoop].pucValue, pstMtn->pstTlvPara[ulLoop].usLen); //compliant 
		        }
                break;

            case CMOID_FTPPWD:
		        if(pstMtn->pstTlvPara[ulLoop].usLen >= MAX_ID_LEN)
	    	    {
                    printf("Ftp Pwd Len too long!");
                }
                else
                {
                    memcpy((void *)stFtpPara.acFtpPwd, (void *)pstMtn->pstTlvPara[ulLoop].pucValue,pstMtn->pstTlvPara[ulLoop].usLen);  //compliant 
                }
                break;

            case CMOID_SRCFILENAME:
                if(pstMtn->pstTlvPara[ulLoop].usLen >= FTM_MAX_FULL_FILE_NAME_LEN)
	    	    {
                    printf("File Name Len too long!");
                }
                else
                {
                    memcpy((void *)acFileName, (void *)pstMtn->pstTlvPara[ulLoop].pucValue, pstMtn->pstTlvPara[ulLoop].usLen);     //compliant 
                }
                break;
            default:
                printf(" Unknow CMOID! CMOID=0x%x ",pstMtn->pstTlvPara[ulLoop].ulParaId);
                break;
        }
     }	
     return 0;
}
#endif

