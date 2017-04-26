#include "protype.h"

extern UINT32  g_ulCell[];
UINT32  g_ulRbUl[LTE_CELL_MAX ][ LTE_OPT_MAX ];

void  D1_cellAccessForIV(UINT16  cellid )
{
    UINT16  i;	   	
    
    for(i=0; i<cellid; i++)
   	{
        if(cellid >= LTE_CELL_MAX)
            break;
        g_ulCell[i] = 0;     //compliant
    }
}


void  D1_cellAccessFor(UINT16  cellid )
{
    UINT16  i;	   	
    
    for(i=0; i<cellid; i++)
   	{
        g_ulCell[i] = 0; //non-compliant
    }
}


void  D1_cellAccessForBV(UINT16  cellid )
{
    UINT16  i;	   	

    if(cellid >= LTE_CELL_MAX)
        return;
    
    for(i=0; i<cellid; i++)
   	{        
        g_ulCell[i] = 0;   //compliant
    }
}


void  D2_procCfgMsgWF(UINT32  cellid,  UINT32  cnOpIdx,   CRDLC_DATA_IND *pMsg)
{
       UINT32  i;

       if(cellid >= LTE_CELL_MAX)
          return;
       
       for(i=0; i<cnOpIdx; i++)
       {
           g_ulRbUl[cellid][i] = pMsg->usDataLen;  //noncompliant, the  index i  could be out-of-bound
       }
  
}

void  D2_procCfgMsgFR(UINT32  cellid,  UINT32  cnOpIdx,   CRDLC_DATA_IND *pMsg)
{
      UINT32  i;	  

      if((cellid >= LTE_CELL_MAX)||(cnOpIdx >= LTE_OPT_MAX))
          return;
      
      for(i=0; i<cnOpIdx; i++)
      {
          g_ulRbUl[cellid][i] = pMsg->usDataLen;  //compliant
      }
  
}


void llcm_BackupCallInfo(UINT16  cellid )
{
    UINT16  i;	   	

    if(cellid >= LTE_CELL_MAX)
        return;
    
    while(cellid > 0)
   	{        
        g_ulCell[cellid] = 0; //compliant
        cellid--;
    }
}



void llcm_InitCallInfo(UINT16  cellid )
{
    UINT16  i;	   	

    i = cellid;
    
    while(i > 0)
   	{        
        g_ulCell[i] = 0; //noncompliant, the  index i  could be out-of-bound
        i--;
    }
}









