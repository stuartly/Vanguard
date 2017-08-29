#ifndef  _PROTYPE_H_
#define  _PROTYPE_H_

#include "type.h"
#include <string.h>

#ifdef OS_LINUX
#include <stddef.h>
#endif

#define VOS_MemSet( ToSet, Char, Count )    memset((void *)(ToSet), (int)(Char), (size_t)(Count))

void *msg_breeding(int len);
int msg_free(void *p);

void rrm_proc_msg(void* msg);
void rrm_send_thread();
void ccb_send_thread();
void ccb_proc_msg(void* msg);

void *VOS_MemAlloc(UINT32 ulPID, UINT8 ucPtNo, UINT32 ulSize);
void *VOS_AllocMsg(int pid, int len);
void  VOS_MemCpy(void *dst, void *src, size_t len);
int   VOS_SendMsg( void *msg );
int   VOS_FreeMsg(void *msg);


UINT32 TRCP_BmAppAddPrePro(MSG_BLOCK *pstMsg);
UINT32 TRCP_BmrcResPro(MSG_BLOCK *pstMsg);
UINT32 TRCP_BmAppDelPrePro(MSG_BLOCK *pstMsg);
UINT32 TRCP_BmrcDelPro(MSG_BLOCK *pstMsg);
UINT32 TRCP_BmAppInsPrePro(MSG_BLOCK *pstMsg);
UINT32 TRCP_BmrcInsPro(MSG_BLOCK *pstMsg);
UINT32 TRCP_BmAppModPrePro(MSG_BLOCK *pstMsg);
UINT32 TRCP_BmrcModPro(MSG_BLOCK *pstMsg);
UINT32 TRCP_BmAppQryPrePro(MSG_BLOCK *pstMsg);
UINT32 TRCP_BmrcQryPro(MSG_BLOCK *pstMsg);

UINT32 CCM_ProcMsg(MSG_BLOCK *pstMsg);
UINT32 CCM_Timer1Sec(MSG_BLOCK *pstMsg);
UINT32 RRM_ProcMsg(MSG_BLOCK *pstMsg); 
UINT32 RRM_Timer1Sec(MSG_BLOCK *pstMsg);
UINT32 LAC_ProcMsg(MSG_BLOCK *pstMsg);
UINT32 LAC_Timer1Sec(MSG_BLOCK *pstMsg);
UINT32 MAC_ProcMsg(MSG_BLOCK *pstMsg); 
UINT32 MAC_Timer1Sec(MSG_BLOCK *pstMsg);
UINT32 MTP_ProcMsg(MSG_BLOCK *pstMsg); 
UINT32 MTP_Timer1Sec(MSG_BLOCK *pstMsg);
UINT32 CIE_ProcMsg(MSG_BLOCK *pstMsg); 
UINT32 CIE_Timer1Sec(MSG_BLOCK *pstMsg);
UINT32 PIM_ProcMsg(MSG_BLOCK *pstMsg);
UINT32 PIM_Timer1Sec(MSG_BLOCK *pstMsg);
UINT32 PIE_ProcMsg(MSG_BLOCK *pstMsg);
UINT32 PIE_Timer1Sec(MSG_BLOCK *pstMsg);
UINT32 BIM_ProcMsg(MSG_BLOCK *pstMsg);
UINT32 BIM_Timer1Sec(MSG_BLOCK *pstMsg);
UINT32 BIE_ProcMsg(MSG_BLOCK *pstMsg);
UINT32 BIE_Timer1Sec(MSG_BLOCK *pstMsg);
UINT32 TIM_ProcMsg(MSG_BLOCK *pstMsg); 
UINT32 TIM_Timer1Sec(MSG_BLOCK *pstMsg);
UINT32 TIE_ProcMsg(MSG_BLOCK *pstMsg);
UINT32 TIE_Timer1Sec(MSG_BLOCK *pstMsg);
UINT32 PCU_ProcMsg(MSG_BLOCK *pstMsg);
UINT32 PCU_Timer1Sec(MSG_BLOCK *pstMsg);
UINT32 BPU_ProcMsg(MSG_BLOCK *pstMsg);
UINT32 BPU_Timer1Sec(MSG_BLOCK *pstMsg);
UINT32 RPS_ProcMsg(MSG_BLOCK *pstMsg); 
UINT32 RPS_Timer1Sec(MSG_BLOCK *pstMsg);
UINT32 CDR_ProcMsg(MSG_BLOCK *pstMsg); 
UINT32 CDR_Timer1Sec(MSG_BLOCK *pstMsg);
UINT32 BCM_ProcMsg(MSG_BLOCK *pstMsg); 
UINT32 BCM_Timer1Sec(MSG_BLOCK *pstMsg);
UINT32 RCM_ProcMsg(MSG_BLOCK *pstMsg); 
UINT32 RCM_Timer1Sec(MSG_BLOCK *pstMsg);
UINT32 SAAL_ProcMsg(MSG_BLOCK *pstMsg, VOS_BLOCK *pstVos);
UINT32 MTP3B_ProcMsg(MSG_BLOCK *pstMsg, VOS_BLOCK *pstVos);
UINT32 SCCP_ProcMsg(MSG_BLOCK *pstMsg, VOS_BLOCK *pstVos);
UINT32 QAAL2_ProcMsg(MSG_BLOCK *pstMsg, VOS_BLOCK *pstVos);
UINT32 SAAL_Timer1sec(MSG_BLOCK *pstMsg, VOS_BLOCK *pstVos, UINT8 *pucMsg);
UINT32 MTP3B_Timer1sec(MSG_BLOCK *pstMsg, VOS_BLOCK *pstVos, UINT8 *pucMsg);
UINT32 SCCP_Timer1sec(MSG_BLOCK *pstMsg, VOS_BLOCK *pstVos, UINT8 *pucMsg);
UINT32 QAAL2_Timer1sec(MSG_BLOCK *pstMsg, VOS_BLOCK *pstVos, UINT8 *pucMsg);
UINT32 TBM_ProcMsg(MSG_BLOCK *pstMsg) ;
UINT32 TBM_Timer1Sec(MSG_BLOCK *pstMsg);

void  D2_procCfgMsg(UINT32  cellid,  UINT32  cnOpIdx,   CRDLC_DATA_IND  *pMsg);
void  D2_procCfgMsgFR(UINT32  cellid,  UINT32  cnOpIdx, CRDLC_DATA_IND  *pMsg);
void  D2_procCfgMsgW(UINT32  cellid,  UINT32  cnOpIdx,  CRDLC_DATA_IND *pMsg);
void  D2_procCfgMsgWF(UINT32  cellid,  UINT32  cnOpIdx, CRDLC_DATA_IND  *pMsg);

void  D1_cellAccess(UINT16  cellid );
void  D1_cellAccessW(UINT16  cellid );
void  D1_cellAccessSubW(UINT16 minuend ,UINT16 subtrahend);
void  D1_cellAccessSub(UINT16 minuend ,UINT16 subtrahend);
void  D1_cellAccessFor(UINT16  cellid );
void  D1_cellAccessForBV(UINT16  cellid );
void  D1_cellAccessForIV(UINT16  cellid );
    
void vos_strtouq(const char *pscNptr,  UINT32 siBase);
void SET_BulkCfgStartW(void*  pstrMuxStartMsg);
void SET_BulkCfgStart(void*  pstrMuxStartMsg);
void SET_BulkCfgStartMod(void*  pstrMuxStartMsg);

/******************* lte l2 *******************/
int  ltel2_traffic_gbuffer_apply();
void ltel2_ltraffic_process();
void ltel2_latraffic_process();
void ltel2_gtraffic_process();
void ltel2_gatraffic_process();
void ltel2_localMalloc();

/******************* lte l3 *******************/
void lte_L3MsgEntry(void* msg,  int length);
void lte_L3MsgProc(void* msg,  int length);
void lte_l3UserInfoAccess(void* msg);

/*************/
void  MOD_quot_msgW(UINT32 i, MSG  *msg);
void  MOD_quot_msg(UINT32 i, MSG  *msg);
void  MOD_quot_o2(UINT32 i, UINT32 j );
void  MOD_quot_o2W(UINT32 i, UINT32 j );

void  DIV_quot_msg(UINT32 i, MSG  *msg);
void  DIV_quot_msgW(UINT32 i, MSG  *msg);
void  DIV_quot_o2(UINT32 i, UINT32 j );
void  DIV_quot_o2W(UINT32 i, UINT32 j );
void DIV_SetDelay(UINT32 ulLgcPort, UINT32 * plForwardDelayAverageBase, UINT32 *plForwardJitterMean);
void DIV_SetDelayW(UINT32 ulLgcPort, UINT32 * plForwardDelayAverageBase, UINT32 *plForwardJitterMean);
    
MR_CCB* getCcbByCcbNo(UINT16 usMrCcbNo);

void ATM_CellRecv(void *pBuf,UINT32 ulLen);
void make_message();
void dma_dataBuffer(void* msg,  int length);


void llcm_BackupCallInfo(UINT16  cellid );
void llcm_InitCallInfo(UINT16  cellid );

void CALD_StartUpFileReq( void* pstMsg);
void CALD_StartDldFileReq( void* pstMsg);
void ALG_HandleRawMrFreq(ABIS_MEASURE_REPORT *pstAbisMeasureReport,  MTLS_MSG_HEADER_STRU *pstMtlsMsgHeader, UINT8 *pFinalMsg);
void ALG_HandleMetaMrFreq(ABIS_MEASURE_REPORT *pstAbisMeasureReport,  MTLS_MSG_HEADER_STRU  *pstMtlsMsgHeader, UINT8 *pFinalMsg);
UINT32 PID_StaticMsgInit(MSG_BLOCK *pMsg);
UINT32 PID_DosMsgInit(MSG_BLOCK *pMsg);

UINT32 fn ( UINT32 x );
UINT32 fn_2 ( UINT32 x );
UINT32 fn_3 ( UINT32 x );
UINT32  fn_call_recur_1( UINT32 x );
UINT32  fn_call_recur_2( UINT32 x );
UINT32  fn_call_recur_3( UINT32 x );

#endif


