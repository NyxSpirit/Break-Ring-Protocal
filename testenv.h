#ifndef TEST_ENV_H_
#define TEST_ENV_H_
#include "common.h"

struct rrpp_link 
{
	int status;
	int port_number;
	struct sw_frame* frame;

};
int sw_send_frame_virt(struct sw_dev *dev, const struct sw_frame* frame, u32 mask);

int sw_flush_fdb(struct sw_dev *dev);

void printFrame(const struct sw_frame*  frame);

int sw_change_virt_port(struct sw_dev *dev, int port, int link_up);

#endif
