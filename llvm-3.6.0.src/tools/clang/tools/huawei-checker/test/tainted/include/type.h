#ifndef  _TYPE_H_
#define  _TYPE_H_

#ifdef OS_VXWORKS
#include <vxTypesOld.h> 
#endif

#ifdef OS_LINUX
typedef unsigned int   UINT32; 
typedef unsigned short UINT16; 
typedef unsigned char  UINT8; 
#endif

typedef unsigned int   UINT32; 
typedef unsigned short UINT16; 
typedef unsigned char  UINT8; 

/*********   MACRO       ********/
#define  L2_ERROR  -1
#define  L2_SUCCESS  0

#define  FAILURE  0
#define  SUCCESS  1

#define  OS_CACHE_LINE_SIZE  4
#define  MUM_CAP    20

#define  LTE_CELL_MAX    200
#define  LTE_OPT_MAX     100
#define  VOS_MSG_LEN     100

#define  TRCP_APP        2

#define  RD_MSG_MAX      23
#define  TRCP_APP_ADD    1
#define  TRCP_APP_DEL    2
#define  TRCP_APP_INS    3
#define  TRCP_APP_MOD    4
#define  TRCP_APP_QRY    5

#define  PID_CCM         6
#define  PID_RRM         7
#define  PID_LAC         8
#define  PID_MAC         9
#define  PID_MTP         10
#define  PID_CIE         11
#define  PID_PIM         12
#define  PID_PIE         13
#define  PID_BIM         14
#define  PID_BIE         15

#define  PID_TIM         16
#define  PID_TIE         17
#define  PID_PCU         18
#define  PID_BPU         19

#define  PID_RPS         20
#define  PID_CDR         21
#define  PID_BCM         22
#define  PID_RCM         23

#define  PID_SAAL        30
#define  PID_MTP3B       31
#define  PID_SCCP        32
#define  PID_QAAL2       33
#define  PID_CONN        34
#define  PID_TBM      35
#define  PID_ARP      36
#define  PID_RECV     37
#define  PID_CMD      38

#define  MAX_CPU         4 
#define  MAX_MSG_LEN     1024
#define  MAX_LEN_OF_MR_INFOR    255

#define   FTM_MAX_FULL_FILE_NAME_LEN  100
#define   MAX_ID_LEN  100
#define   MAX_ATTR_LEN  100

#define vos_quad_t   1024
#define QUAD_MAX     256

#define STATIC_MEM_PT    0
#define PD_RR            0x06
#define EI_ABIS_L3_INFO  0x0b

#define LMT_ARRAY_LEN_SMALL 1500
#define CID_RR_RRDATAVER 0x7400f400

#define _NORMAL_BLOCK 1

typedef struct __tagMSG
{
    UINT32 SenderPid;
    UINT32 RecvPid;
    UINT32 stop;
    UINT32 start;
    UINT32 msg_id;
    UINT16 a;
    UINT16 b;
    UINT32 rbDl;
    UINT32 msg_len;
    char msg[20];
}MSG;


typedef struct tagBulkCfgMsg
{
    UINT16  usCpuNo[MAX_CPU];
    UINT32  ulHandle;
    MSG* pMsg;
}BULKCFGMSG;

typedef struct __tag_msc_msg
{
    UINT32 SenderPid;
    UINT32 RecvPid;
    UINT32 stop;
    UINT32 start;
    UINT32 msg_id;
    UINT16 a;
    UINT16 b;
    UINT32 msg_len;
    UINT8 *msg;
}MSC_MSG;

typedef struct __tagMSGUM
{
    UINT32 protocol;
    UINT32 probe;
    UINT32 pilot_id;
    UINT32 msg_len;
    UINT32 category[RD_MSG_MAX];
    UINT8  msg[20];
}MSG_UM;

#define MSG_HEADER      UINT16 usSendSeqNum; \
                        UINT16 usAckNum; \
                        UINT8  ucIpcMsgType; \
                        UINT8  ucFragFlg; \
                        UINT8  ucReserved; \
                        UINT8  ucPrio; \
                        UINT32 ulSenderCpuId;  \
                        UINT32 ulSenderPid;    \
                        UINT32 ulReceiverCpuId;\
                        UINT32 ulReceiverPid;  \
                        UINT16 uwLength;




typedef struct tagMSG_BLOCK
{
    MSG_HEADER
    UINT8  aucValue[2];
} MSG_BLOCK;

typedef struct tagVOS_BLOCK
{
    MSG_HEADER
    UINT8  msg[2];
} VOS_BLOCK;

typedef UINT32  RdMsgPro(MSG_BLOCK * );
typedef UINT32  TransMsgPro(MSG_BLOCK * , VOS_BLOCK *);
typedef UINT32  TransMsgPostPro(MSG_BLOCK * , VOS_BLOCK *, UINT8 * );

typedef struct
{
    UINT32        ulMsgNo ;        
    RdMsgPro      *pPreProFunc;   
    RdMsgPro      *pProFunc;
}RD_COMM_MSG;

typedef struct
{
    UINT32        ulMsgNo ;        
    TransMsgPro          *pPreProFunc;   
    TransMsgPostPro      *pProFunc;   
}TRANS_MSG;


typedef struct
{
    UINT16      usUsrCcb;            
    UINT16      usAbsCcb;            
    UINT8       aucReserved[8];  
    UINT16      usDataLen;           
    UINT8       aucData[1];          
}CRDLC_DATA_IND;

typedef struct
{    
    UINT8   ucPriority;    
    UINT8   ucGsm900MsMaxPower;   
    UINT8   ucGsm1800MsMaxPower;  
    UINT8   ucPcs1900MsMaxPower;  

    UINT8   ucMsPowerCapacity;      
    UINT8   ucMsPowerMin;           
    UINT8   ucMsFilterRxlev;        
    UINT8   ucBsFilterRxlev;        

    UINT8   ucMsPowerCapability;
    UINT8   ucSDAccessCasue;    
    UINT8   ucDelayStaSDMrTimes;
    UINT8   ucSrvType;          
    
    UINT16  usMsPowerDbm;
    UINT16  usBsPowerDbm;

    UINT16  usUlPathLoss;
    UINT16  usRxLevEnhancedConcentricCell;
    
    UINT32  ulGlobalCallId;    

    UINT8   ucCechmRecordMeasInfo;
    UINT8   ucCurrentPhase;     
    UINT8   ucChInterfInd;  
    UINT8   ucOldBsPowerLev;
    
    UINT8   ucOldMsPowerLev;
    UINT8   ucTA;           
    UINT8   ucEnhancedIUOCellAsNcell;    
    UINT8   ucPathLoss;  

    UINT16   usMulRxLevUL;  
    UINT16   usMulRxLevDL;  
    UINT16   usMulRxQualUL;
    UINT16   usMulRxQualDL;

    UINT32   ulMrCount;
    
    UINT16   usMaxNcellRxLevDL;    
    UINT8    ucUlPCTimer; 
    UINT8    ucDlPCTimer; 

}MR_CCB;



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

typedef struct
{
    UINT16   usCauseOfDecodeFail; 
    UINT16   usChanNo; 
    UINT8    *pucL3StartPos;  
    UINT8    *pucL3EndPos;
    UINT32   ulCellExt; 
}ABIS_MEASURE_REPORT;

typedef struct
{
    MSG_HEADER
    UINT16 usMsgType; 
    UINT16 usHandle;  
    UINT8  aucMsg[1]; 
}MTLS_MSG_HEADER_STRU;

typedef struct
{
    UINT16      usUsrCcb;        
    UINT16      usAbsCcb;        
    UINT8       aucReserved[8];  
    UINT16      usDataLen;       
    UINT8       aucData[1];      
}MTLS_CRDLC_RSLDATA_IND_STRU;

typedef MTLS_CRDLC_RSLDATA_IND_STRU MTLS_ALG_RSLDATA_IND_STRU;

typedef struct tagPfmLogPort
{
    UINT8       ucSlotIndex;      
    UINT8       ucPortSlot;       
    UINT16      usLogicPortNo;    
    UINT16      usLatUpExp;       
    UINT16      usLatDownExp;     

}PFM_CFG_LOGPORT_STRU;


#define VOS_MemFree( ulPID, pAddr ) \
    _free_dbg( pAddr, _NORMAL_BLOCK )

#define _free_dbg(p, t)                 free(p)

#endif


