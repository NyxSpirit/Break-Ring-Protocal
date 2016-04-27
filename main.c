#include <stdio.h>
#include "rsm.h"

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
	int nodeType = 0;
	int nodeId = 1;
	int masterPort;
	int slavePort;
	
	int i = 0;
	int vlan = 0;
	int domainNum = 1;
	int ringNum = 1;

	int domainId = 1;
	int ringId = 1;
	int ringLevel = 1;
	int helloInterval = 1000;
	int helloFailTime = 3000;
		// create an 3-node ring topo;
	int nodeNum = 3;
	for(i = 0; i < nodeNum; i++)
	{
		nodeId = i;
		if(i == 0)
			nodeType = RNODE_MAIN;
		else
			nodeType = RNODE_TRANSFER;
		masterPort = 0;
		slavePort = 1;

		sw_rrpp_init_device(&gl_devs[i], domainNum);
		sw_rrpp_init_domain(&gl_devs[i].rrpp_domains[0], &gl_devs[i], domainId, ringNum, vlan, nodeId);
		sw_rrpp_init_ring(&gl_devs[i].rrpp_domains[0].rings[0], &gl_devs[i].rrpp_domains[0], ringId, ringLevel, nodeType, masterPort, slavePort,helloInterval, helloFailTime);
		sw_rrpp_init_frame(&gl_devs[i]);

	        initLink(&gl_links[i], i, 0, (i+1)%3, 1);	
	}
	
	//Start RRPP
	
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

