#include "testenv.h"
#include <stdio.h>
#include <string.h>
#include "rsm.h"
#include <stdlib.h>
#include "log.h"
#include "rpkg.h"

#define LOG_FILE "log"
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
	int port = mask;
	struct rrpp_link* link;
	int nodeId = dev->rrpp_domains[0].node_id;
	link = getLink(nodeId, port);
	int* direction = (int*)malloc(sizeof(int));
	if(link->node_id[0] == nodeId)
		*direction = 0;
	else
		*direction = 1;
	memcpy(&(link->frame[*direction]), frame, sizeof(struct sw_frame));
	pthread_create(&(link->pass[*direction]), NULL, (void*)*passLink, direction);
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
		case RPORT_STATUS_UP:
			strcat(info, "UP" );
			break;
		case RPORT_STATUS_BLOCK:
			strcat(info, "BLOCK");
			break;
		case RPORT_STATUS_DOWN:
			strcat(info,"DOWN");
			break;
		default:
			strcat(info, "PRE / ERROR");
	}
	if(dev->ports[port].status != link_up)
	{
		dev->ports[port].status = link_up;
		logInfo(dev, info);

	}
	return 0;
}

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
int initLink(struct rrpp_link* link, int n0, int p0, int n1, int p1)
{
	link->node_id[0] = n0;
	link->node_id[1] = n1;
	link->port_id[0] = p0;
	link->port_id[1] = p1;
	link->status = RLINK_UP;
	return 0;
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
	struct sw_dev* dev;
	dev = getDev(link->node_id[1-direction]);
	char info[100];
	sprintf(info, "sending a frame from node %d port %d to node %d port %d", link->node_id[direction], link->port_id[direction], link->node_id[1-direction], link->port_id[1-direction]);
	logLink(link, info);
	sw_rrpp_frame_handler(dev, &(link->frame[direction]), link->port_id[1-direction]);
	
	return ;	
}
struct sw_dev* getDev(int nodeId)
{
	int i = 0;
	int domain = 0;
	for(i = 0; i < DEV_NUMBER; i++)
	{
		if(gl_devs[i].rrpp_domains[0].node_id == nodeId)
			return &gl_devs[i];
	}
	return NULL;
}
void changeLinkStatus(struct rrpp_link* link, int status)
{
	link->status = status;
	int i;
	for(i = 0; i < 2; i++)
	{
		int port = link->port_id[i];
		int node = link->node_id[i];
		struct sw_dev* dev = getDev(node);

		sw_rrpp_link_change(dev, port, status);
	}

}
