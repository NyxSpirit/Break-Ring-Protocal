#ifndef RSM_H_
#define RSM_H_

#include "rpkg.h"
#include "common.h"

 

int sw_rrpp_init(struct sw_dev* dev, int ntype, int nid, int portNum, struct sw_port* ports, struct sw_mac_addr localMac);
int sw_rrpp_start(struct sw_dev* dev); 
int sw_rrpp_stop(struct sw_dev* dev);
int sw_rrpp_destroy(struct sw_dev* dev);
int sw_rrpp_frame_handler(struct sw_dev* dev, 
		const struct sw_frame *frame,
		int from_port);
int sw_rrpp_link_change(struct sw_dev* dev, int port, int link_up);

#endif
