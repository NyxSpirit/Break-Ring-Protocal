#include "testenv.h"
#include <stdio.h>
#include <string.h>
#include "rsm.h"
#include <stdlib.h>
#include "log.h"
#include "rpkg.h"

#define LINK_PASS_TIME 1000 

void printFrame(const struct sw_frame*  frame)
{
	int i = 0;
	for(; i < frame->length/8; i++) 
	{
		printf("%02x", frame->frame_data[i]);
		if(i % 2 == 1)
			printf(" ");
		if(i % 6 == 5)
			printf("\n");
	}
}

int sw_send_frame_virt(struct sw_dev *dev, const struct sw_frame *frame, u32 mask)
{
	char info[50];
	sprintf(info, "sending frame type %d from port mask: %d ", getRpkgType(frame), mask);
	logInfo(dev, info);
	struct rrpp_link* link;
	int i = 0;
	for(i = 0; i < dev->port_num; i++)
	{
		if((1 << i) & mask)
		{
			link = dev->ports[i].plink;
	int* direction = (int*)malloc(sizeof(int));
	if(link->port[0] == &dev->ports[i])
		*direction = 0;
	else
		*direction = 1;
	memcpy(&(link->frame[*direction]), frame, sizeof(struct sw_frame));
	pthread_create(&(link->pass[*direction]), NULL, (void*)*passLink, direction);
		}

	}
	return 0;
}
int sw_flush_fdb(struct sw_dev *dev)
{
	logInfo(dev, "flushing fdb now ");
	return 0;
}

int sw_change_virt_port(struct sw_dev *dev, int port, int link_up)
{
	int domainId = 0;
	char info[80];
	sprintf(info, "change dev id %d port id %d status from %d to %d", dev->rrpp_domains[domainId].node_id, port, dev->ports[port].status, link_up);
        
	switch(link_up)
	{
		case PORT_UP:
			strcat(info, "UP" );
			break;
		case PORT_DOWN:
			strcat(info, "DOWN");
			break;
		default:
			strcat(info, "ERROR");
	}
	if(dev->ports[port].status != link_up)
	{
		dev->ports[port].status = link_up;
		logInfo(dev, info);

	}
	return 0;
}
/*
struct rrpp_link* getLink(int nodeId, int portId)
{
	int i;
	for(i = 0; i < LINK_NUMBER; i++ )	
	{
		if(gl_links[i].node_id[0] == nodeId && gl_links[i].port_id[0] == portId)
		return &gl_links[i];
		if(gl_links[i].node_id[1] == nodeId && gl_links[i].port_id[1] == portId)
		return &gl_links[i];
	}
	return NULL;
}
*/
int initLink(struct rrpp_link* link, struct sw_port* p0, struct sw_port* p1)
{
	link->port[0] = p0;
	link->port[1] = p1;
	link->status = RLINK_UP;
	p0->plink = link;
	p1->plink = link;
	return 0;
}
void printLink(struct rrpp_link* link) 
{
	struct rrpp_port* p0 = link->port[0]->pport;
	struct rrpp_port* p1 = link->port[1]->pport;
	int ringId = p0->pring->ring_id;
	int domainId = p0->pring->pdomain->domain_id;
	char ps1 = (p0->status == PORT_UP)? '-' : 'x';
	char ps2 = (p1->status == PORT_UP)? '-' : 'x';
	char* ls = (link->status == RLINK_UP)? "---------" : "XXXXXXXXX";

	printf("domain %d ring %d : dev%d %d %c %s %c %d dev%d\n", domainId, ringId, p0->pring->pdomain->node_id, link->port[0]->id, ps1, ls, ps2, link->port[1]->id, p1->pring->pdomain->node_id);
}


void passLink(int *dir)
{
	int direction = *dir;
	free(dir);
	usleep(LINK_PASS_TIME);
	struct rrpp_link* link = NULL;
	int i = 0;
	for(i = 0; i < LINK_NUMBER; i++)
	{
		if(gl_links[i].pass[direction] == pthread_self())
			link = &gl_links[i];
	}
	if(link == NULL)
	{
		logError("link not found");
		return ;

	}
	if(link->status == RLINK_DOWN)
	{
		logError("trying to pass throngh a link down");
		return ;
	}
	struct sw_port* fromport = link->port[direction];
	struct sw_port* toport = link->port[1 - direction];
	char info[100];
	sprintf(info, "sending a frame from node %d port %d to node %d port %d",
		    fromport->pport->pring->pdomain->node_id, fromport->id, toport->pport->pring->pdomain->node_id, toport->id);
	logLink(link, info);
	sw_rrpp_frame_handler(toport->pport->pring->pdomain->pdev, &(link->frame[direction]), link->port[1-direction]->id);
	
	return ;	
}
/*
struct sw_dev* getDev(int domainId, int nodeId)
{
	int i = 0;
	int domain = 0;
	for(i = 0; i < DEV_NUMBER; i++)
	{
		int j = 0;
		for(j = 0; j < gl_devs[i].domain_num; j++)
		{
			if(gl_devs[i].rrpp_domains[j].domain_id == domainId && gl_devs[i].rrpp_domains[j].node_id == nodeId)
				return &gl_devs[i];
		}
	}
	return NULL;
}
*/
void changeLinkStatus(struct rrpp_link* link, int status)
{
	link->status = status;
	int i;
	for(i = 0; i < 2; i++)
	{
		struct sw_port* port = link->port[i];
		struct sw_dev* dev = port->pport->pring->pdomain->pdev;
		sw_rrpp_link_change(dev, port->id, status);
	}

}
