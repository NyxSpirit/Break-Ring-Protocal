#ifndef RSM_H_
#define RSM_H_

#include "rpkg.h"
#include "common.h"

// time unit: ms
struct Port
{
	int isMaster; 
	int ringId; 
	int status;  
}; 

int sw_rrpp_init(struct sw_dev* dev, int ntype, int nid, int portNum, int *portType, struct sw_mac_addr localMac);
int sw_rrpp_start(struct sw_dev* dev); 
int sw_rrpp_stop(struct sw_dev* dev);
int sw_rrpp_destroy(struct sw_dev* dev);
int sw_rrpp_frame_handler(struct sw_dev* dev, 
		const struct sw_frame *frame,
		int from_port);
int sw_rrpp_link_change(struct sw_dev* dev, int port, int link_up);

#endif
