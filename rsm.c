#include "rsm.h"
#include <sys/time.h> 
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include "testenv.h"
#include <string.h>

int sw_rrpp_init(struct sw_dev* dev, int ntype, int nid, int portNum, struct sw_port* ports, struct sw_mac_addr localMac)
{
	dev->vlan_id = 1;
	dev->hello_interval= 1;
	dev->hello_fail_time = 3;
	dev->hello_seq = 0;
	dev->node_id = nid;
	dev->node_type = ntype;
	dev->hello_expire_time = -1;
	dev->hello_expire_seq= -1;
	memcpy(&dev->local_mac_addr,&localMac, sizeof(struct sw_mac_addr));

	dev->port_number = portNum;
	dev->ports = (struct sw_port*) malloc(sizeof(struct sw_port) * portNum);
	int i = 0;
	for(i = 0; i < portNum; i++)
	{
		struct sw_port* port = &dev->ports[i];
		port->id = ports[i].id;
		port->type = ports[i].type;
		port->ring_id = ports[i].ring_id;
		port->the_other_port = ports[i].the_other_port;
		sw_change_virt_port(dev, port->id, RPS_BLOCK);	
	}
	initRrppFrame(dev);
	//printFrame(getFrameModule());
	return 0;
} 

int sw_rrpp_start(struct sw_dev* dev)
{
	printf("RRPP start up \n");
	if(dev->node_type == RNODE_MAIN)
	{
		startHello(dev);
	} else
	{
	// no operation 	
	}
	return 0;	 
}

int sw_rrpp_stop(struct sw_dev* dev)
{
	printf("RRPP stop \n");
	stopHello(dev);
	return 0;
}
int sw_rrpp_destroy(struct sw_dev* dev)
{
	printf("RRPP destroy \n");
	free(dev->ports);
	return 0;
}


int sw_rrpp_frame_handler(struct sw_dev* dev, const struct sw_frame *frame, int from_port)
{
	int frametype = getRpkgType(frame);
       	int ringId = getRpkgRingId(frame);
	if(dev->node_type == RNODE_MAIN)	
	{
		switch (frametype) 
		{			
			case RPKG_HELLO:
				//recievedHelloAction();
				//updateExpireTimer(getRpkgHelloSeq(frame));
				dev->hello_seq += 1;
				sw_change_virt_port(dev, getSlavePortId(dev, ringId), RPS_BLOCK);	
				sw_change_virt_port(dev, getMasterPortId(dev, ringId), RPS_UP);
				sw_flush_fdb(dev);
				sendCompleteFlushPkg(dev, frame, getMasterPortId(dev, ringId) );
				sendCompleteFlushPkg(dev, frame, getSlavePortId(dev, ringId) );
				//stopTimer(getHelloExpireTimer(helloSeq));
				break;
			case RPKG_LINK_UP:
				//recievedUpAction();
				sw_change_virt_port(dev, getSlavePortId(dev, ringId), RPS_BLOCK);	
				sendCommonFlushPkg(dev, frame, getMasterPortId(dev, ringId) );
				sendCommonFlushPkg(dev, frame, getSlavePortId(dev, ringId) );
				break;
			case RPKG_LINK_DOWN:
				//recievedDownAction();
				sw_change_virt_port(dev, getSlavePortId(dev, ringId), RPS_UP);	
				sendCommonFlushPkg(dev, frame, getMasterPortId(dev, ringId) );
				sw_flush_fdb(dev);
				sendCommonFlushPkg(dev, frame, getSlavePortId(dev, ringId));
				break;
			default:
				//forwardPkg(frame, from_port);
				;
		}
	} else if(dev->node_type == RNODE_TRANSFER)
	{
		switch (frametype)
		{
			case RPKG_COMMON_FLUSH_FDB:
				sw_flush_fdb(dev);
				forwardPkg(dev, frame, getTheOtherPortId(dev, from_port));
				//recievedCommonFAction();
				break;
			case RPKG_COMPLETE_FLUSH_FDB:
				sw_change_virt_port(dev, getMasterPortId(dev, ringId), RPS_UP);
				sw_change_virt_port(dev, getSlavePortId(dev, ringId), RPS_UP);
				forwardPkg(dev, frame, getTheOtherPortId(dev, from_port));
				sw_flush_fdb(dev);
				//recievedCompleteFAction();
				break;
			default:
				forwardPkg(dev, frame, from_port);
		}

	}

	return 0;
}

int getTheOtherPortId(struct sw_dev* dev, int port)
{
	int i = 0;
	for(; i < dev->port_number; i++)
	{
		if(dev->ports[i].id == port)
			return dev->ports[i].the_other_port;
	}
       	return -1;
}
int getSlavePortId(struct sw_dev* dev, int ringId)
{
	int i = 0;
	for(; i < dev->port_number; i++)
	{
		if(dev->ports[i].ring_id == ringId && dev->ports[i].type == RPORT_TYPE_SLAVE)
			return i;
	}
	return -1;
}
int getMasterPortId(struct sw_dev* dev, int ringId)
{
	int i = 0;
	for(; i < dev->port_number; i++)
	{
		if(dev->ports[i].ring_id == ringId && dev->ports[i].type == RPORT_TYPE_MASTER)
			return i;
	}
	return -1;
}
int getPreforwardingPortId(struct sw_dev* dev, int ringId)
{
	int i = 0;
	for(; i < dev->port_number; i++)
	{
		if(dev->ports[i].ring_id == ringId && dev->ports[i].status == RPS_PREFORWARDING)
			return i;
	}
	return -1;
}

int sw_rrpp_link_change(struct sw_dev* dev, int port, int link_up)
{
	if(dev->node_type == RNODE_MAIN)
		return 0;
	if(link_up == 1)
	{
		sendUpPkg(dev, getFrameModule(dev), getTheOtherPortId(dev, port));
		sw_change_virt_port(dev, port, RPS_PREFORWARDING);
		// start preforwarding wait		
	}else
	{
		sw_change_virt_port(dev, port, RPS_BLOCK);
		sendDownPkg(dev, getFrameModule(dev), getTheOtherPortId(dev, port));
	}
	return 0;
}

void helloTimeout (struct sw_dev* dev)
{
	/*
	struct sw_dev* dev = (struct sw_dev*) dev_para;	struct itimerval t; 
	signal(SIGALRM, helloHandler);
	t.it_interval.tv_usec = 0;
	t.it_interval.tv_sec = helloInterval - helloExpireTime;
	t.it_value.tv_usec = 0;
	t.it_value.tv_sec = 0;

	helloExpireSeqNum ++;
	helloExpireTime += helloInterval;
	*/	
	//pthread_cond_timedwait(&(dev->hello_recieved), NULL, dev->hello_fail_time);
		
	//enablePort(getslavePortId(0));	

}
 
void polling(struct sw_dev *dev)
{
	while(1)
	{
		
		sendHelloPkg(dev, getMasterPortId(dev, 0));
		//start timer
		
		//printf("%d ", dev->hello_interval);
		//pthread_create(&dev->hello_fail_id, NULL, (void*)helloTimeout, (void *)dev );
		sleep(dev->hello_interval);

	}
	
}

int startHello(struct sw_dev* dev) 
{
	printf("dev %d start Hello proc\n", dev->node_id);
	pthread_create(&dev->polling_id, NULL, (void *)polling, (void *)dev);

	return 0; 
}
int stopHello(struct sw_dev* dev) {
	printf("stop Hello proc\n");
	pthread_cancel(dev->polling_id);
	//signal(SIGALRM, SIG_IGN);
	return 0;
}
