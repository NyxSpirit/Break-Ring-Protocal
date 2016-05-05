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
#include "log.h"
void flushTimeout (struct rrpp_ring* ring)
{
        sleep(ring->hello_fail_time/1000);
	struct sw_dev* dev = ring->pdomain->pdev; 
	int port = ring->wait_port;
	if (dev->ports[port].status == RPORT_STATUS_PREFORWARDING)	
		sw_change_virt_port(dev, port, RPORT_STATUS_UP);
}
void helloTimeout (struct rrpp_ring* ring)
{
		
	int helloSeq = ring->hello_seq;
	sleep(ring->hello_fail_time/1000);
	if(ring->arrived_hello_seq < helloSeq && ring->status == RRING_COMPLETE)
	{
		printf(" arrived seq %d helloseq %d hello interval %d\n", ring->arrived_hello_seq, helloSeq, ring->hello_interval);
		sendCommonFlushPkg(ring, ring->slave_port );
		sw_change_virt_port(ring->pdomain->pdev, ring->slave_port, RPORT_STATUS_UP);	
		ring->status = RRING_FAIL;

	}	

}
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
	ring->status = RRING_FAIL;

	ring->pdomain->pdev->ports[masterPort].status  = RPORT_STATUS_BLOCK;
	ring->pdomain->pdev->ports[masterPort].ring_id = ringId;
	ring->pdomain->pdev->ports[slavePort].ring_id = ringId;
	ring->pdomain->pdev->ports[slavePort].status = RPORT_STATUS_BLOCK;
	
	ring->hello_seq = 0;
	ring->arrived_hello_seq = 0;
	ring->hello_interval = helloInterval; 
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
	logInfo(dev, "Rrpp start");
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
	logInfo(dev, "Rrpp stop");
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
int mainNodeSM(struct rrpp_ring* ring, const struct sw_frame *frame,int from_port)
{
	int frametype = getRpkgType(frame);
	int seqNumber = getRpkgHelloSeq(frame);
	struct sw_dev* dev = ring->pdomain->pdev;
	if(ring->status == RRING_COMPLETE)
		{
			switch(frametype)
			{
				case RPKG_LINK_DOWN:
					sendCommonFlushPkg(ring, ring->master_port );
					sendCommonFlushPkg(ring, ring->slave_port);
					sw_change_virt_port(dev, ring->slave_port, RPORT_STATUS_UP);	
					sw_flush_fdb(dev);
					ring->status = RRING_FAIL;
					break;
				case RPKG_HELLO:
					ring->arrived_hello_seq = seqNumber;
					break;
				default:
					;

			}

		} else if(ring->status == RRING_FAIL)
		{
			switch (frametype) 
			{			
				case RPKG_HELLO:
					ring->arrived_hello_seq = seqNumber;
					sendCompleteFlushPkg(ring, ring->master_port );
					sendCompleteFlushPkg(ring, ring->slave_port );
					sw_change_virt_port(dev, ring->slave_port, RPORT_STATUS_BLOCK);
					sw_change_virt_port(dev, ring->master_port, RPORT_STATUS_UP);
					sw_flush_fdb(dev);
					ring->status = RRING_COMPLETE;
					break;
				case RPKG_LINK_UP:
					sendCommonFlushPkg(ring, ring->master_port );
					sendCommonFlushPkg(ring, ring->slave_port );
					sw_flush_fdb(dev);
					break;
				default: 
					;
			}
		}
	return 0;
}
int transNodeSM(struct rrpp_ring* ring, const struct sw_frame *frame,int from_port)
{
	int frametype = getRpkgType(frame);
	struct sw_dev* dev = ring->pdomain->pdev;
	switch (frametype)
		{
			case RPKG_COMMON_FLUSH_FDB:
				forwardPkg(ring, frame, getTheOtherPortId(ring, from_port));
				sw_flush_fdb(dev);
				break;
			case RPKG_COMPLETE_FLUSH_FDB:
				forwardPkg(ring, frame, getTheOtherPortId(ring, from_port));
				sw_change_virt_port(dev, ring->master_port, RPORT_STATUS_UP);
				sw_change_virt_port(dev, ring->slave_port, RPORT_STATUS_UP);
				sw_flush_fdb(dev);
				break;
			default:
				forwardPkg(ring, frame, getTheOtherPortId(ring, from_port));
		}
	return 0;
}
int edgeNodeSM(struct rrpp_ring* ring, const struct sw_frame *frame,int from_port)
{
	int frametype = getRpkgType(frame);
	struct sw_dev* dev = ring->pdomain->pdev;

	return 0;

}
int supNodeSM(struct rrpp_ring* ring, const struct sw_frame *frame,int from_port)
{
	int frametype = getRpkgType(frame);
	struct sw_dev* dev = ring->pdomain->pdev;

	return 0;

}
int sw_rrpp_frame_handler(struct sw_dev* dev, const struct sw_frame *frame, int from_port)
{
       	int ringId = getRpkgRingId(frame);
	int domainId = getRpkgDomainId(frame);
	int frametype = getRpkgType(frame);
	int seqNumber = getRpkgHelloSeq(frame);
	struct rrpp_ring* ring = getRing(getDomain(dev, domainId), ringId);
	char info[50];
	sprintf(info, "recieve a frame type %d, seq %d", frametype, seqNumber);
	logInfo(dev, info); 
	switch(ring->node_type)
	{
		case RNODE_MAIN:
			mainNodeSM(ring, frame, from_port);		
			break;
		case RNODE_TRANSFER:
			transNodeSM(ring, frame, from_port);
			break;
		case RNODE_EDGE_MAIN:
			edgeNodeSM(ring, frame, from_port);
			break;
		case RNODE_EDGE_SUPPORT:
			supNodeSM(ring, frame, from_port);
			break;
		default:
			logError("node_type error");
			;

	}
	return 0;
}

int getTheOtherPortId(struct rrpp_ring* ring, int port)
{
	if(port == ring->slave_port)
		return ring->master_port;
	if(port == ring->master_port)
		return ring->slave_port;
	char error[100];
	sprintf(error, "wrong port number, ring %d portid  should be %d or %d  but it is %d", ring->ring_id,  ring->slave_port, ring->master_port, port);
	logError(error);
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
	struct rrpp_ring* ring = getRing(&dev->rrpp_domains[0], dev->ports[port].ring_id);
	if(ring->node_type == RNODE_MAIN)
	{
		sw_change_virt_port(dev, port, link_up);
		return 0;
	}
	if(dev->ports[port].status == link_up) 
	{
		logInfo(dev, "port status not changed in fact");
	}
	if(link_up == RPORT_STATUS_UP)
	{
		sendUpPkg(ring, getTheOtherPortId(ring, port));
		sw_change_virt_port(dev, port, RPORT_STATUS_PREFORWARDING);
		ring->wait_port = port;
		pthread_create(&ring->flush_wait_id, NULL, (void*)flushTimeout, (void *)ring);
	}else if(link_up == RPORT_STATUS_DOWN)
	{
		sendDownPkg(ring, getTheOtherPortId(ring, port));
		sw_change_virt_port(dev, port, RPORT_STATUS_BLOCK);
	}
	logError("link change trigger error");
	return -1;
}


 
void polling(struct rrpp_ring* ring)
{
	while(1)
	{
		ring->hello_seq += 1;
		sendHelloPkg(ring, ring->master_port);
		pthread_create(&ring->hello_fail_id, NULL, (void*)helloTimeout, (void *)ring );
		sleep(ring->hello_interval/1000);
	}
	
} 

int startHello(struct rrpp_ring* ring) 
{
	logInfo(ring->pdomain->pdev, "start Hello proc");
	pthread_create(&ring->polling_id, NULL, (void *)polling, (void *)ring);

	return 0; 
}
int stopHello(struct rrpp_ring* ring)
{
	logInfo(ring->pdomain->pdev, "stop Hello proc");
	pthread_cancel(ring->polling_id);
	//signal(SIGALRM, SIG_IGN);
	return 0;
}
