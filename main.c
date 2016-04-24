#include <stdio.h>
#include "rsm.h"

#define DEV_NUMBER 4
int main(int argc, char* args[])
{
	//Configuration 
	struct sw_dev devs[DEV_NUMBER];
	int portNum = 2;
	int portTypes[2] = {1, 0};
	int nodeType = 0;
	int nodeId = 1;
	struct sw_mac_addr localMac;
	//sw_rrpp_init(nodeType, nodeId, portNum, portTypes, localMac);
	sw_rrpp_init(&devs[0], RNODE_MAIN, 0, portNum, portTypes, localMac);
	sw_rrpp_init(&devs[1], RNODE_TRANSFER, 0, portNum, portTypes, localMac);
	//start test 
	int i;
	for(i = 0; i < DEV_NUMBER; i++)
	{
		sw_rrpp_start(&devs[i]);
	}	
	
	getchar();
	// Link Action Test here
	struct sw_frame frame;
	createHelloFrame(&frame, 0);
	sw_rrpp_frame_handler(&devs[0], &frame, 1);

	getchar();
	//end 
	for(i = 0; i < DEV_NUMBER; i++)
	{
		sw_rrpp_stop(&devs[i]);
		sw_rrpp_destroy(&devs[i]);
	}
	return 0;
}

