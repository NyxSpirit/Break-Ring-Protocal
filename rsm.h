#ifndef RSM_H_
#define RSM_H_

#include "rpkg.h"
#include "common.h"

 

int sw_rrpp_init_device(struct sw_dev* dev, int domainNum);
int sw_rrpp_init_domain(struct rrpp_domain* domain, struct sw_dev* pdev, int domainId, int ringNum, int vlanId, int nodeId);
int sw_rrpp_init_ring(struct rrpp_ring* ring, struct rrpp_domain* pdomain, int ringId,
	       	int ringLevel, int nodeType,
	       	int masterPort, int slavePort, 
		int helloInterval, int helloFailTime);
int sw_rrpp_init_frame(struct sw_dev* dev);

int sw_rrpp_start(struct sw_dev* dev); 
int sw_rrpp_stop(struct sw_dev* dev);
int sw_rrpp_destroy(struct sw_dev* dev);
int sw_rrpp_frame_handler(struct sw_dev* dev, 
		const struct sw_frame *frame,
		int from_port);
int sw_rrpp_link_change(struct sw_dev* dev, int port, int link_up);

#endif
