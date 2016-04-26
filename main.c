#include <stdio.h>
#include "rsm.h"


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
	struct sw_port ports[10];
	int portNum = 2;
	int nodeType = 0;
	int nodeId = 1;
	struct sw_mac_addr localMac;
	//sw_rrpp_init(nodeType, nodeId, portNum, portTypes, localMac);
	
	// create an 3-node ring topo;
	
	int i = 0;
	int nodeNum = 3;
	for(i = 0; i < nodeNum; i++)
	{
		portInit(&ports[0], 0, RPORT_TYPE_MASTER, 0, 1);
		portInit(&ports[1], 1, RPORT_TYPE_SLAVE, 0, 0);
		if(i == 0)
			nodeType = RNODE_MAIN;
		else
			nodeType = RNODE_TRANSFER;
		sw_rrpp_init(&gl_devs[i], nodeType, i, portNum, ports, localMac);
		initLink(&gl_links[i], i, 0, (i+1)%3, 1);	
	}
	//sw_rrpp_init(&gl_devs[1], RNODE_TRANSFER, 0, portNum, portTypes, localMac);
	
	//start test 
	
	for(i = 0; i < DEV_NUMBER; i++)
	{
		sw_rrpp_start(&gl_devs[i]);
	}	

	
	getchar();
	//sw_rrpp_link_change(&gl_devs[]);

	getchar();
	//end 
	for(i = 0; i < DEV_NUMBER; i++)
	{
		sw_rrpp_stop(&gl_devs[i]);
		sw_rrpp_destroy(&gl_devs[i]);
	}
	return 0;
}

