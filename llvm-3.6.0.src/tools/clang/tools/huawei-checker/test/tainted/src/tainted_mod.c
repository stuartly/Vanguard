#include <stdio.h>

#include "type.h"


void  MOD_quot_o2W(UINT32 i, UINT32 j )
{
    UINT32 quot;
    
    quot =  (i % j);  //non-compliant  ,  j could  equal to zero
    printf("quot is: %d\n", quot);
    
}

void  MOD_quot_o2(UINT32 i, UINT32 j )
{
    UINT32 quot;

    if(j==0)
        j = 5;
    
    quot =  (i % j);  //compliant  ,  j doesn't equal to zero
    printf("quot is: %d\n", quot);
    
}


void MOD_quot_msgW(UINT32 i, MSG  *msg)
{
    UINT32 quot;

    quot =  (i % msg->msg_len);  //non-compliant 
    printf("quot is: %d\n", quot);
    
}



void MOD_quot_msg(UINT32 i, MSG  *msg)
{
    UINT32 quot;

    if(msg->msg_len == 0 )
    {
		return;
    }
	
    quot =  (i % msg->msg_len);  //compliant .  msg->msg_len doesn't equal to zero
    printf("quot is: %d\n", quot);
    
}


	
void vos_strtouq(const char *pscNptr,  UINT32 siBase)
{
    const char *pscStr = pscNptr;
    char  ucChar = (UINT8)*pscStr;
    UINT32	ulQBase;
    UINT32	ulCutoff;
    int siNeg;
    int siCutLim;
	
    if ('-' == ucChar) 
    {
        siNeg = 1;
        ucChar = (UINT8)*pscStr++;
    } 
    else 
    {
        siNeg = 0;
    }
	
   if (0 == siBase)
   {
       siBase = ucChar == '0' ? 8:10;    
   }
 
   ulQBase = siBase;
   ulCutoff = siNeg ? (vos_quad_t) + QUAD_MAX : QUAD_MAX;
   siCutLim = (int)( ulCutoff % ulQBase );  //compliant, ulQBase won't be zero because the if-statement can guarantee the  safety.  If there is no the if-statement, this is a defect.
   ulCutoff /= ulQBase;
 
 
}


