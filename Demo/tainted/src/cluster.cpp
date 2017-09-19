#include <stdio.h>
#include "cluster.h"


static const char *sClusterStatus[]= { 
    "csUnknown",
    "csInitial",
    "csActiving",
    "csActive",
    "csDeactiving",
    "csDeactive",
    "csHoAsker",
    "csHoReceiver",
    "csFailure",
    "csBreakDown" }; 


cluster_controller::cluster_controller()
{
	_rcv_peer_stat = csUnknown;    
}
cluster_controller::~cluster_controller()
{
    
}
void cluster_controller::notify_heart_beat(const HeartBeatFrame& frame)
{

    if((((CLUSTER_STAT)frame.dwStatus) != _peer_stat)
        && (csInitial == ((CLUSTER_STAT)frame.dwStatus)))
    {
        _rcv_peer_stat   = csInitial;
    }
    if (((CLUSTER_STAT)frame.dwStatus) == _rcv_peer_stat)
    {
        _peer_stat = _rcv_peer_stat;
        printf("peer Status change to %s!", sClusterStatus[_peer_stat]); //non-compliant, tainted data 
    }
    else
    {
        _rcv_peer_stat = (CLUSTER_STAT)frame.dwStatus;
        _peer_stat_count = 0;
    }
    
}

bool cluster_controller::on_event(int code, int para1, int)
{
    if (1 == code)
    {
        lpHeartBeatFrame heart_beat_frame = (lpHeartBeatFrame)(para1);
        notify_heart_beat(*heart_beat_frame);
    }
    return 1;
}



