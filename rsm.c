#include <sys/time.h> 
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include "testenv.h"
#include "rsm.h"
#include <string.h>
#include "log.h"

//  virt port id  --- >  output  mask 
// 
// only work when virt port exist
static int getMaskAddPort(int mask, int portId)
{
	if(portId >= 0)
		return mask + (1 << portId);
	else 
		return mask;
}
static int getMaskDelPort(int mask, int portId)
{
	if(portId >= 0 && (mask & (1<<portId)))
		return mask - (1 << portId);
	else
		return mask;
}
static int getMaskOfRing(const struct rrpp_ring* ring)
{
	int mask = 0;
	mask += (ring->slave_port.virt_port == NULL) ? 0: 1<<(ring->slave_port.virt_port->id) ;
	mask += (ring->master_port.virt_port == NULL) ? 0: 1<<(ring->master_port.virt_port->id) ;
	return mask;
}
static int getMaskLELevel(const struct rrpp_domain* domain, int level)
{
	int mask = 0;
	int i = 0;
	for(i = 0; i < domain->ring_num; i++)
	{
		struct rrpp_ring* ring = &domain->rings[i];
		if(ring->ring_level <= level)
		{
			mask += (ring->slave_port.virt_port != NULL) ? 1<<(ring->slave_port.virt_port->id) : 0;
			mask += (ring->master_port.virt_port != NULL) ? 1<<(ring->master_port.virt_port->id) : 0;
		}
	}
	return mask;

}


//    record/get  newest seq number of  (ringId, frameType)   
static int getHistorySeq(const struct rrpp_domain* domain, int frameType, int ringId)
{
	return domain->history_seqs[ringId * RRPP_FRAME_TYPE_NUMBER + frameType];
}
static int setHistorySeq(struct rrpp_domain* domain, int frameType, int ringId, int seq)
{
	domain->history_seqs[ringId * RRPP_FRAME_TYPE_NUMBER + frameType] = seq;
	return 0;
}


//     
static int changeRrppPort(struct rrpp_port* port, int status)
{
	if(port->virt_port == NULL)   // do nothing without this port
	{
		return 1;
	}
	port->status = status;
	if(status == RPORT_STATUS_UP)  //only when rrpp up, virt port transmit data
		status = PORT_UP;
	else
		status = PORT_DOWN;
	sw_change_virt_port(port->pring->pdomain->pdev, port->virt_port->id, status);
	return 0;
}

// timeout handlers 
static void flushTimeout (struct rrpp_ring* ring)
{
        sleep(ring->hello_fail_time/1000);
	struct sw_dev* dev = ring->pdomain->pdev; 
	if(ring->wait_port < 0) 
		return;
	struct rrpp_port* port = dev->ports[ring->wait_port].pport;
	if (port->status == RPORT_STATUS_PREFORWARDING)	          // waiting port turn into up
		changeRrppPort(port, RPORT_STATUS_UP);
}
static void helloTimeout (struct rrpp_ring* ring)
{
		
	int helloSeq = ring->seq[RPKG_HELLO];                  // record hello seq first
	sleep(ring->hello_fail_time/1000);
	if(getHistorySeq(ring->pdomain, RPKG_HELLO, ring->ring_id) < helloSeq && ring->status == RRING_COMPLETE) // no newer hello pkg recieved
	{
		//printf(" arrived seq %d helloseq %d hello interval %d\n", ring->arrived_hello_seq, helloSeq, ring->hello_interval);
		if(ring->master_port.virt_port == NULL)
			logError("master node miss master port");
		sendCommonFlushPkg(ring, getMaskAddPort(0, ring->master_port.virt_port->id));         // ring fail, use redundant link
		changeRrppPort(&ring->master_port, RPORT_STATUS_UP);	
		ring->status = RRING_FAIL;
	}	

}

// find rrpp domain/ring  according to id (not the same with index )
static struct rrpp_domain* getDomain(const struct sw_dev* dev, int domainId)
{
	int i = 0;
	for(i = 0; i < dev->domain_num;i ++)
	{
		if(dev->rrpp_domains[i].domain_id == domainId)
			return &dev->rrpp_domains[i];
	}
	return NULL;
}
static struct rrpp_ring* getRing(const struct rrpp_domain* domain, int ringId)
{
	int i = 0;
	for(i = 0; i < domain->ring_num; i++)
	{
		if(domain->rings[i].ring_id == ringId)
			return  &domain->rings[i];
	}
	return NULL;
}


//============================================== frame handlers ===========================
// 
static int helloHandler(struct rrpp_domain* domain, const struct sw_frame* frame, int from_port)
{
	int ringId = getRpkgRingId(frame);
	struct rrpp_ring* ring = getRing(domain, ringId);
	if(ring != NULL && ring->node_type == RNODE_MASTER && ring->status != RRING_COMPLETE) // when corresponding ring master catch hello
	{
		struct sw_dev* dev = ring->pdomain->pdev;
		sendCompleteFlushPkg(ring, getMaskOfRing(ring));
		changeRrppPort(&ring->master_port, RPORT_STATUS_BLOCK);
		changeRrppPort(&ring->slave_port, RPORT_STATUS_UP);
		sw_flush_fdb(dev);
		ring->status = RRING_COMPLETE;
		
	} else 				    // lower level node transfer hello  
	{
		int level = getRpkgRingLevel(frame);
		forwardPkg(domain, frame, getMaskDelPort(getMaskLELevel(domain, level), from_port));
	}
	return 0;
}	
static int linkDownHandler(struct rrpp_domain* domain, const struct sw_frame* frame, int from_port)
{
	int ringId = getRpkgRingId(frame);
	struct rrpp_ring* ring = getRing(domain, ringId);
	struct sw_dev* dev = ring->pdomain->pdev;
	if(ring != NULL && ring -> node_type == RNODE_MASTER && ring -> status == RRING_COMPLETE)
	{
		sendCommonFlushPkg(ring, getMaskLELevel(domain, ring->ring_level));
		sw_flush_fdb(dev);
		changeRrppPort(&ring->master_port, RPORT_STATUS_UP);	
		ring->status = RRING_FAIL;
	} else if (ring != NULL) {
		forwardPkg(domain, frame, getMaskDelPort(getMaskOfRing(ring), from_port));
	}
	return 0;
}
static int linkUpHandler(struct rrpp_domain* domain, const struct sw_frame* frame, int from_port)
{
	int ringId = getRpkgRingId(frame);
	struct rrpp_ring* ring = getRing(domain, ringId);
	if(ring != NULL && ring -> node_type == RNODE_MASTER && ring -> status == RRING_FAIL)
	{
		sendCommonFlushPkg(ring, getMaskLELevel(domain, ring->ring_level));
		sw_flush_fdb(ring->pdomain->pdev);
	} else if (ring != NULL) {
		forwardPkg(domain, frame, getMaskDelPort(getMaskOfRing(ring), from_port));
	}
	return 0;
}
static int commonFlushHandler(struct rrpp_domain* domain, const struct sw_frame* frame, int from_port)
{
	int ringId = getRpkgRingId(frame);
	struct rrpp_ring* ring = getRing(domain, ringId); 
	int level = getRpkgRingLevel(frame);
	if(!(ring != NULL && ring->node_type == RNODE_MASTER))
	{
		forwardPkg(domain, frame, getMaskDelPort(getMaskLELevel(domain, level), from_port));
		sw_flush_fdb(domain->pdev);
	}
	return 0;
} 
static int completeFlushHandler(struct rrpp_domain* domain, const struct sw_frame* frame, int from_port)
{
	int ringId = getRpkgRingId(frame);
	struct rrpp_ring* ring = getRing(domain, ringId);
	struct sw_dev* dev = ring->pdomain->pdev;
	if(ring != NULL && ring->node_type != RNODE_MASTER)                // in the ring 
	{
		forwardPkg(domain, frame, getMaskDelPort(getMaskOfRing(ring), from_port));
		changeRrppPort(&ring->master_port, RPORT_STATUS_UP);
		changeRrppPort(&ring->slave_port, RPORT_STATUS_UP);
		sw_flush_fdb(ring->pdomain->pdev);
	} else { 			
		;
	}
	return 0;
}

void polling(struct rrpp_ring* ring)
{
	while(1)
	{
		sendHelloPkg(ring, getMaskAddPort(0, ring->master_port.virt_port->id));
		pthread_create(&ring->hello_fail_id, NULL, (void*)helloTimeout, (void *)ring );
		sleep(ring->hello_interval/1000);
	}
	
} 

static int startHello(struct rrpp_ring* ring) 
{
	char info[100];
	sprintf(info, "Ring %d start Hello process", ring->ring_id);
	logInfo(ring->pdomain->pdev, info); 
	pthread_create(&ring->polling_id, NULL, (void *)polling, (void *)ring);

	return 0; 
}
static int stopHello(struct rrpp_ring* ring)
{
	logInfo(ring->pdomain->pdev, "stop Hello proc");
	pthread_cancel(ring->polling_id);
	return 0;
}

//===================================================init RRPP structure==========================
static int rrpp_init_port(struct rrpp_port* port, struct rrpp_ring* pring, struct sw_port* vport, int type)
{
	vport->pport = port;
	vport->status = PORT_DOWN;
	port->virt_port = vport;
	port->type = type;
	port->pring = pring;
	port->status  = RPORT_STATUS_BLOCK;
	return 0;
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
	memset(domain->history_seqs, 0, sizeof(domain->history_seqs));
	return 0;
}
int sw_rrpp_init_ring(struct rrpp_ring* ring, struct rrpp_domain* pdomain, int ringId,
	       	int ringLevel, int nodeType,
	       	int masterPort, int slavePort, 
		int helloInterval, int helloFailTime)
{
	struct sw_port* port;
	ring->pdomain = pdomain;
	ring->ring_id = ringId;
	ring->ring_level = ringLevel;
	ring->node_type = nodeType;
	ring->status = RRING_INIT;

	if(masterPort != -1)
	{
		port = &ring->pdomain->pdev->ports[masterPort];
		rrpp_init_port(&ring->master_port, ring, port, RPORT_TYPE_MASTER);
	}else 
		ring->master_port.virt_port = NULL;

	if(slavePort != -1)
	{
		port = &ring->pdomain->pdev->ports[slavePort];
		rrpp_init_port(&ring->slave_port, ring, port, RPORT_TYPE_SLAVE);	
	} else
		ring->slave_port.virt_port = NULL;

	memset(ring->seq, 0, sizeof(ring->seq));
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

// 
int rrpp_start_domain(struct rrpp_domain* domain)
{
	int i = 0;
	struct rrpp_ring* ring;
	for(i = 0; i < domain->ring_num; i++)
	{
		ring = &(domain->rings[i]);
		if(ring->node_type == RNODE_MASTER)
		{
			startHello(ring);
		}
	}
	return 0;
}

int rrpp_stop_domain(struct rrpp_domain* domain)
{
	int i = 0;
	struct rrpp_ring* ring;
	for(i = 0; i < domain->ring_num; i++)
	{
		ring = &(domain->rings[i]);
		if(ring->node_type == RNODE_MASTER)
		{
			stopHello(ring);
		}
	}
	return 0;
	
}

int sw_rrpp_start(struct sw_dev* dev)
{
	logInfo(dev, "Rrpp start all domain");
	int i = 0;
	for(i = 0; i < dev->domain_num; i++)
	{
		struct rrpp_domain* domain = &dev->rrpp_domains[i];
		rrpp_start_domain(domain);
	}
	return 0;	 
}
int sw_rrpp_stop(struct sw_dev* dev)
{
	logInfo(dev, "Rrpp stop all domains");
	int i = 0;
	for(i = 0; i < dev->domain_num; i++)
	{
		struct rrpp_domain* domain = &dev->rrpp_domains[i];
		rrpp_stop_domain(domain);
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


int sw_rrpp_frame_handler(struct sw_dev* dev, const struct sw_frame *frame, int from_port)
{
	int domainId = getRpkgDomainId(frame);
	int frameType = getRpkgType(frame);
	struct rrpp_domain* domain;
	if((domain = getDomain(dev, domainId)) == NULL)
	       	return -1; 		//only handle frame of this domain, the others should be treated as data frame
						
	struct rrpp_ring* ring;
	
	
	int seqNumber = getRpkgSeq(frame);
	int ringId = getRpkgRingId(frame);

	if(getHistorySeq(domain, frameType, ringId) >= seqNumber)  // if dev recieved the frame, ignore it 
	{
		return 0;
	} else {
		setHistorySeq(domain, frameType, ringId, seqNumber);
	}

	char info[50];
	sprintf(info, "recieve a frame type %d, seq %d, port %d", frameType, seqNumber, from_port);
	logInfo(dev, info); 

	switch(frameType)
	{
		case RPKG_HELLO:
			helloHandler(domain, frame, from_port);
			break;
		case RPKG_LINK_UP:
			linkUpHandler(domain, frame, from_port);
			break;
		case RPKG_LINK_DOWN:
			linkDownHandler(domain, frame, from_port);
			break;
		case RPKG_COMMON_FLUSH_FDB:
			commonFlushHandler(domain, frame, from_port);
			break;
		case RPKG_COMPLETE_FLUSH_FDB:
			completeFlushHandler(domain, frame, from_port);
			break;

		default:
			logError("Unknown RRPP PKG TYPE");
	}
	return 0;
}

int sw_rrpp_link_change(struct sw_dev* dev, int port, int link_up)
{
	struct rrpp_ring* ring = dev->ports[port].pport->pring;

	if(dev->ports[port].status == link_up) 
	{
		logInfo(dev, "port status not changed in fact");
	}
	if(link_up == RPORT_STATUS_UP)
	{
		if(ring->node_type == RNODE_MASTER)            // handle directly
		{
			sendCommonFlushPkg(ring, getMaskOfRing(ring));
		 	sw_flush_fdb(dev);	
		}else {
			// inform master node 
			sendUpPkg(ring, getMaskDelPort(getMaskOfRing(ring), port));
			// wait for master node close its backup link, then up the link.
			changeRrppPort(dev->ports[port].pport, RPORT_STATUS_PREFORWARDING);
			ring->wait_port = port;
			pthread_create(&ring->flush_wait_id, NULL, (void*)flushTimeout, (void *)ring);
		}
		return 0;
	}else if(link_up == RPORT_STATUS_DOWN)
	{
		if(ring->node_type == RNODE_MASTER)               // handle directly
		{
			if(ring->slave_port.virt_port != NULL && ring->slave_port.virt_port->id == port &&ring->status == RRING_COMPLETE)
			{
				changeRrppPort(&ring->master_port, RPORT_STATUS_UP);	
			}
			changeRrppPort(dev->ports[port].pport, RPORT_STATUS_DOWN);
			sendCommonFlushPkg(ring, getMaskOfRing(ring));
			ring->status = RRING_FAIL;
			sw_flush_fdb(dev);
		} else {
			
			sendDownPkg(ring, getMaskDelPort(getMaskOfRing(ring), port));
			changeRrppPort(dev->ports[port].pport, RPORT_STATUS_DOWN);
		}
		return 0;
	}
	printf("attention please !!!!!!!!!!!!!!!!!!!!!!! %d\n",link_up);
	logError("link change trigger error");
	return -1;
}


 

