#include <stdio.h>
#include "rsm.h"

#define DEV_NUMBER  1
#define LINK_NUMBER 2

void readControl()
{
	char control[10];
	int id;
	scanf("%s %d",control, &id);

}
void portInit(struct sw_port* port, int id, int type, int ringId, int theOtherPort)
{
	port->id = id;
	port->type = type;
	port->the_other_port = theOtherPort;
	port->ring_id = ringId;
	
}
int main(int argc, char* args[])
{
	//Configuration 
	struct sw_dev devs[DEV_NUMBER];
	struct sw_port ports[10];
	int portNum = 2;
	int nodeType = 0;
	int nodeId = 1;
	struct sw_mac_addr localMac;
	//sw_rrpp_init(nodeType, nodeId, portNum, portTypes, localMac);
	
	portInit(&ports[0], 0, RPORT_TYPE_MASTER, 1, 1);
	portInit(&ports[1], 1, RPORT_TYPE_SLAVE, 1, 0);
	sw_rrpp_init(&devs[0], RNODE_MAIN, 0, portNum, ports, localMac);
	//sw_rrpp_init(&devs[1], RNODE_TRANSFER, 0, portNum, portTypes, localMac);
	struct rrpp_link links[LINK_NUMBER];
	
	//start test 
	int i;
	for(i = 0; i < DEV_NUMBER; i++)
	{
		sw_rrpp_start(&devs[i]);
	}	

	getchar();
	// Link Action Test here
	struct sw_frame frame;
	createHelloFrame(&devs[0], &frame, 0);
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

