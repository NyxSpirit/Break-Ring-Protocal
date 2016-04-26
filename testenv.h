#ifndef TEST_ENV_H_
#define TEST_ENV_H_
#include "common.h"
#include <pthread.h>
#include <sys/types.h>

#define DEV_NUMBER  3
#define LINK_NUMBER 3 

#define RLINK_UP 0
#define RLINK_DOWN 1

struct rrpp_link 
{
	int status;
	int port_id[2];
	int node_id[2];
	pthread_t pass[2];  //Full duplex   two Dirction pipe
					//  node0 -> node1 
					//  node1 -> node0
	struct sw_frame frame[2];   // frame in two pipes

};
struct sw_dev gl_devs[DEV_NUMBER];
struct rrpp_link gl_links[LINK_NUMBER];

int sw_send_frame_virt(struct sw_dev *dev, const struct sw_frame* frame, u32 mask);

int sw_flush_fdb(struct sw_dev *dev);

void printFrame(const struct sw_frame*  frame);

int sw_change_virt_port(struct sw_dev *dev, int port, int link_up);



struct rrpp_link* getLink(int nodeId, int portId);

int linkStart(struct rrpp_link* link);

int initLink(struct rrpp_link* link, int n0, int p0, int n1, int p1);

void *passLink(int* dir);
struct sw_dev* getDev(int nodeId);
#endif
