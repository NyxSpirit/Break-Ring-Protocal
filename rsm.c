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

int sw_rrpp_init_device(struct sw_dev* dev, int domainNum)
{
	dev->domain_num = domainNum;
	dev->rrpp_domains = (struct rrpp_domain*)malloc(sizeof(struct rrpp_domain) * domainNum);
	return 0;
}
int sw_rrpp_init_domain(struct rrpp_domain* domain, struct sw_dev* dev, int domainId, int ringNum, int vlan_id, int nodeId)
{
	domain->pdev = dev;
	domain->domain_id = domainId;
	domain->ring_num = ringNum;
	domain->rings = (struct rrpp_ring*)malloc(sizeof(struct rrpp_ring) * ringNum);
	domain->node_id = nodeId;
	domain->vlan_id = vlan_id;
	return 0;
}
int sw_rrpp_init_ring(struct rrpp_ring* ring, struct rrpp_domain* pdomain, int ringId,
	       	int ringLevel, int nodeType,
	       	int masterPort, int slavePort, 
		int helloInterval, int helloFailTime)
{
	ring->pdomain = pdomain;
	ring->ring_id = ringId;
	ring->ring_level = ringLevel;
	ring->node_type = nodeType;
	ring->master_port = masterPort;
	ring->slave_port = slavePort;

	ring->hello_seq = 0;
	ring->hello_interval = helloInterval; 
	printf("hell0_interval %d to %d \n", helloInterval, ring->hello_interval);
	ring->hello_fail_time = helloFailTime;
	
	return 0;
} 

int sw_rrpp_init_frame(struct sw_dev* dev)
{
	int i, j;
	struct rrpp_domain* domain;
	for(i = 0; i < dev->domain_num; i ++)
	{
		domain = &dev->rrpp_domains[i];
		for(j = 0; j < domain->ring_num; j++)
			initRrppFrame(&domain->rings[j]);
	}
	return 0;
}
int sw_rrpp_start(struct sw_dev* dev)
{
	int i = 0;
	struct rrpp_ring* ring;
	for(i = 0; i < dev->rrpp_domains[0].ring_num; i++)
	{
		ring = &(dev->rrpp_domains[0].rings[i]);
		if(ring->node_type == RNODE_MAIN)
		{
			startHello(ring);
		}
	}
	
	return 0;	 
}

int sw_rrpp_stop(struct sw_dev* dev)
{
	int i = 0;
	struct rrpp_ring* ring;
	for(i = 0; i < dev->rrpp_domains[0].ring_num; i++)
	{
		ring = &(dev->rrpp_domains[0].rings[i]);
		if(ring->node_type == RNODE_MAIN)
		{
			stopHello(ring);
		}
	}
	return 0;
}
int sw_rrpp_destroy(struct sw_dev* dev)
{
	int i, j;
	for(i = 0; i < dev->domain_num; i++)
	{
		struct rrpp_domain* domain = &dev->rrpp_domains[i];
		free(domain->rings);

	}
	free(dev->rrpp_domains);
	return 0;
}

struct rrpp_domain* getDomain(const struct sw_dev* dev, int domainId)
{
	int i = 0;
	for(i = 0; i < dev->domain_num;i ++)
	{
		if(dev->rrpp_domains[i].domain_id == domainId)
			return &dev->rrpp_domains[i];
	}
	return NULL;
}
struct rrpp_ring* getRing(const struct rrpp_domain* domain, int ringId)
{
	int i = 0;
	for(i = 0; i < domain->ring_num; i++)
	{
		if(domain->rings[i].ring_id == ringId)
			return  &domain->rings[i];
	}
	return NULL;
}

int sw_rrpp_frame_handler(struct sw_dev* dev, const struct sw_frame *frame, int from_port)
{
	int frametype = getRpkgType(frame);
       	int ringId = getRpkgRingId(frame);
	int domainId = getRpkgDomainId(frame);
	struct rrpp_ring* ring = getRing(getDomain(dev, domainId), ringId);
	if(ring->node_type == RNODE_MAIN)	
	{
		switch (frametype) 
		{			
			case RPKG_HELLO:
				ring->hello_seq += 1;
				sw_change_virt_port(dev, ring->slave_port, RPORT_STATUS_BLOCK);
				sw_change_virt_port(dev, ring->master_port, RPORT_STATUS_UP);
				sw_flush_fdb(dev);
				sendCompleteFlushPkg(ring, frame, ring->master_port );
				sendCompleteFlushPkg(ring, frame, ring->slave_port );
				//stopTimer(getHelloExpireTimer(helloSeq));
				break;
			case RPKG_LINK_UP:
				//recievedUpAction();
				sw_change_virt_port(dev, ring->slave_port, RPORT_STATUS_BLOCK);	
				sendCommonFlushPkg(ring, frame, ring->master_port );
				sendCommonFlushPkg(ring, frame, ring->slave_port );
				break;
			case RPKG_LINK_DOWN:
				//recievedDownAction();
				sw_change_virt_port(dev, ring->slave_port, RPORT_STATUS_UP);	
				sendCommonFlushPkg(ring, frame, ring->master_port );
				sw_flush_fdb(dev);
				sendCommonFlushPkg(ring, frame, ring->slave_port);
				break;
			default:
				//forwardPkg(frame, from_port);
				;
		}
	} else if(ring->node_type == RNODE_TRANSFER)
	{
		switch (frametype)
		{
			case RPKG_COMMON_FLUSH_FDB:
				sw_flush_fdb(dev);
				forwardPkg(ring, frame, getTheOtherPortId(dev, from_port));
				//recievedCommonFAction();
				break;
			case RPKG_COMPLETE_FLUSH_FDB:
				sw_change_virt_port(dev, ring->master_port, RPORT_STATUS_UP);
				sw_change_virt_port(dev, ring->slave_port, RPORT_STATUS_UP);
				forwardPkg(ring, frame, getTheOtherPortId(ring, from_port));
				sw_flush_fdb(dev);
				//recievedCompleteFAction();
				break;
			default:
				//forwardPkg(ring, frame, getTheOtherfrom_port);
				forwardPkg(ring, frame, getTheOtherPortId(ring, from_port));
		}

	}

	return 0;
}

int getTheOtherPortId(struct rrpp_ring* ring, int port)
{
	if(port == ring->slave_port)
		return ring->master_port;
	if(port == ring->master_port)
		return ring->slave_port;
	return -1;
}

int getPreforwardingPortId(struct sw_dev* dev, int ringId)
{
	int i = 0;
	for(; i < dev->port_number; i++)
	{
		if(dev->ports[i].ring_id == ringId && dev->ports[i].status == RPORT_STATUS_PREFORWARDING)
			return i;
	}
	return -1;
}

int sw_rrpp_link_change(struct sw_dev* dev, int port, int link_up)
{
	/*
	if(dev->node_type == RNODE_MAIN)
		return 0;
	if(link_up == 1)
	{
		sendUpPkg(ring, getFrameModule(dev), getTheOtherPortId(dev, port));
		sw_change_virt_port(dev, port, RPORT_STATUS_PREFORWARDING);
		// start preforwarding wait		
	}else
	{
		sw_change_virt_port(dev, port, RPORT_STATUS_BLOCK);
		sendDownPkg(ring, getFrameModule(dev), getTheOtherPortId(dev, port));
	}
	*/
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
 
void polling(struct rrpp_ring* ring)
{
	while(1)
	{
		
		sendHelloPkg(ring, ring->master_port);
		//	printf("send hello  now \n");
		//start timer
		
		//printf("%d ", dev->hello_interval);
		//pthread_create(&dev->hello_fail_id, NULL, (void*)helloTimeout, (void *)dev );
		sleep(ring->hello_interval/1000);
		//sleep(1);

	}
	
} 

int startHello(struct rrpp_ring* ring) 
{
	printf("dev %d start Hello proc\n", ring->pdomain->node_id);
	pthread_create(&ring->polling_id, NULL, (void *)polling, (void *)ring);

	return 0; 
}
int stopHello(struct rrpp_ring* ring) {
	printf("stop Hello proc\n");
	pthread_cancel(ring->polling_id);
	//signal(SIGALRM, SIG_IGN);
	return 0;
}
