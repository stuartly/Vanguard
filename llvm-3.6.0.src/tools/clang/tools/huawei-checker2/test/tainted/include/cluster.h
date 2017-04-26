#ifndef _CLUSTER_H_
#define _CLUSTER_H_

#ifdef OS_VXWORKS
#include <vxTypesOld.h> 
#endif

#include "type.h"

typedef enum
{
    csUnknown     = 0,
    csInitial     = 1,
    csActiving    = 2,
    csActive      = 3,
    csDeactiving  = 4,
    csDeactive    = 5,
    csHoAsker     = 6,
    csHoReceiver  = 7,
    csFailure     = 8,
    csBreakDown   = 9
}CLUSTER_STAT;

struct tagHeartBeatFrame 
{
    UINT8   sVerInfo[36]; 
    UINT16  nCode;        
    UINT16  nOption;      
    UINT32  nNodeId;      
    UINT32  dwStatus;     
    UINT32  dwTimeStamp;  
};

typedef struct tagHeartBeatFrame HeartBeatFrame,*lpHeartBeatFrame;


class cluster_controller
{

private:
    bool          _is_dual_mode;
    bool          _stop_check_vip;
    CLUSTER_STAT  _own_stat;
    UINT16        _own_stat_assoc_opt;

    CLUSTER_STAT _last_own_stat;
    UINT16        _last_own_stat_assoc_opt;

    CLUSTER_STAT _peer_stat;
    UINT16 _peer_data_sync_allow_opt;
    UINT16 _peer_ho_asker_assoc_opt;

    UINT32 _node_id;
    UINT32 _peer_node_id;

    CLUSTER_STAT _rcv_peer_stat;
    UINT16        _peer_stat_count;


public:
    cluster_controller();
    ~cluster_controller();
    void notify_heart_beat(const HeartBeatFrame& frame);
    bool on_event(int code, int para1, int);
    
};

#endif


