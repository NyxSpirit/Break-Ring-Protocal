#ifndef TEST_ENV_H_
#define TEST_ENV_H_
#include "common.h"
#include <pthread.h>
#include <sys/types.h>

#define DEV_NUMBER 8 
#define LINK_NUMBER 10 

#define RLINK_UP PORT_UP
#define RLINK_DOWN PORT_DOWN

struct rrpp_link 
{
	int status;
	struct sw_port* port[2];
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

//struct rrpp_link* getLink(int nodeId, int portId);

int linkStart(struct rrpp_link* link);

int initLink(struct rrpp_link* link, struct sw_port* p0, struct sw_port* p1);

void passLink(int* dir);
//struct sw_dev* getDev(int nodeId);
void changeLinkStatus(struct rrpp_link* link, int status);
void printLink(struct rrpp_link* link); 

#endif
