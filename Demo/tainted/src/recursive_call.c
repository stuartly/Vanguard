#include <stdio.h>
#include "protype.h"

/* * * * * * * * * * * * * * * * * * * * * * * * *

function shall not call themselves directly or indirectly

* * * * * * * * * * * * * * * * * * * * * * * * */


UINT32 fn ( UINT32 x )
{
   if ( x > 0 )
   {
      x = x * fn ( x - 1 ); // Non-compliant
   }
   
   return  x ;
}


UINT32 fn_2 ( UINT32 x )
{
   if ( x > 0 )
   {
      x = x * fn_3 ( x - 1 ); // Non-compliant
   }
   return  x ;
}


UINT32 fn_3 ( UINT32 x )
{
   if ( x == 0 )
   {
       x = x * fn_2 ( x - 1 ); // Non-compliant
   }
   
   return  x ;
}


UINT32  fn_call_recur_1( UINT32 x )
{
    if ( x > 0 )
    {
        x = x * fn_call_recur_2 ( x - 1 ); // Non-compliant
    }
    
    return  x ;
}

UINT32  fn_call_recur_2( UINT32 x )
{
    if ( x > 0 )
    {
        x = x * fn_call_recur_3 ( x - 1 ); // Non-compliant
    }
    
    return  x ;
}


UINT32  fn_call_recur_3( UINT32 x )
{
    if ( x > 0 )
    {
        x = x * fn_call_recur_1 ( x - 1 ); // Non-compliant
    }
    
    return  x ;

}

