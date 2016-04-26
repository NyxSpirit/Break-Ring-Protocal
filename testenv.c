#include "testenv.h"
#include <stdio.h>
#include <string.h>
#include "rsm.h"
#include <stdlib.h>

#define LOG_FILE "log"
#define LINK_PASS_TIME 0 


void printFrame(const struct sw_frame*  frame)
{
	printf("This frame is of length: %d\n", frame->length);
	int i = 0;
	for(; i < frame->length/8; i++) 
	{
		printf("%02x", frame->frame_data[i]);
		if(i % 2 == 1)
			printf(" ");
		if(i % 6 == 5)
			printf("\n");
	}
	printf("\nprinting over.\n");
}

int sw_send_frame_virt(struct sw_dev *dev, const struct sw_frame *frame, u32 mask)
{
	printf("sending frame from port mask: %d\n ", mask);
	//	printFrame(frame);
	int port = mask;
	struct rrpp_link* link;
	link = getLink(dev->node_id, port);
	int* direction = (int*)malloc(sizeof(int));
	if(link->node_id[0] == dev->node_id)
		*direction = 0;
	else
		*direction = 1;
	memcpy(&(link->frame[*direction]), frame, sizeof(struct sw_frame));
	pthread_create(&(link->pass[*direction]), NULL, (void*)*passLink, direction);
	return 0;
}
int sw_flush_fdb(struct sw_dev *dev)
{
	printf("flushing fdb now\n ");
	return 0;
}

int sw_change_virt_port(struct sw_dev *dev, int port, int link_up)
{
	printf("change sw id %d, port id %d to status ", dev->node_id, port);
	switch(link_up)
	{
		case RPS_UP:
			printf("UP \n");
			break;
		case RPS_BLOCK:
			printf("BLOCK \n");
			break;
		case RPS_DOWN:
			printf("DOWN \n");
			break;
		default:
			printf("PRE / ERROR \n");
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
void* passLink(int *dir)
{
	int direction = *dir;
	free(dir);
	sleep(LINK_PASS_TIME);
	struct rrpp_link* link = NULL;
	int i = 0;
	for(i = 0; i < LINK_NUMBER; i++)
	{
		if(gl_links[i].pass[direction] == pthread_self())
			link = &gl_links[i];
	}
	struct sw_dev* dev;
	dev = getDev(link->node_id[1-direction]);
	sw_rrpp_frame_handler(dev, &(link->frame[direction]), link->port_id[1-direction]);
	
	return 0;	
}
struct sw_dev* getDev(int nodeId)
{
	int i = 0;
	for(i = 0; i < DEV_NUMBER; i++)
	{
		if(gl_devs[i].node_id == nodeId)
			return &gl_devs[i];
	}
	return NULL;
}
