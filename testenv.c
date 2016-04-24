#include "testenv.h"
#include <pThread.h>
#include <stdio.h>

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
	printFrame(frame);
	return 0;
}
int sw_flush_fdb(struct sw_dev *dev)
{
	printf("flushing fdb now ");
	return 0;
}

int sw_change_virt_port(struct sw_dev *dev, int port, int link_up)
{
	printf("change port id %d to status %d", port, link_up);
	return 0;
}
