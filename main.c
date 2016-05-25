#include "testenv.h"
#include "rsm.h"
#include "log.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* args[])
{
	//Configuration 
	int nodeType = 0;
	int nodeId = 1;
	int masterPort;
	int slavePort;
	
	int i = 0;
	int j = 0;
	int vlan = 0;
	int domainNum = 1;
	int ringNum = 1;

	int domainId = 1;
	int ringId = 1;
	int ringLevel = 0;
	int helloInterval = 1000000;
	int helloFailTime = 3000000;
		// create an 8-node ladder topo;
	int nodeNum = DEV_NUMBER;
	int PORT_NUMBER = 2;
		
	masterPort = 0;
	slavePort = 1;

	// init virt port
	for(i = 0; i < nodeNum; i++)
	{
		if(i == 3 || i == 2 || i == 4 || i == 5)
			PORT_NUMBER = 3;
		else 
			PORT_NUMBER = 2;
		gl_devs[i].ports = (struct sw_port*) malloc(sizeof(struct sw_port) * PORT_NUMBER);
		gl_devs[i].port_num = PORT_NUMBER;
		for(j = 0; j < PORT_NUMBER; j++)
		{
			struct sw_port* port = &gl_devs[i].ports[j];
			port-> status = PORT_UP;
			port-> id = j;
		}

	}
	//init node rrpp 
	nodeId = i = 0;
	nodeType = RNODE_MASTER;
		sw_rrpp_init_device(&gl_devs[i], domainNum);
		sw_rrpp_init_domain(&gl_devs[i].rrpp_domains[0], &gl_devs[i], domainId, ringNum, vlan, nodeId);
		sw_rrpp_init_ring(&gl_devs[i].rrpp_domains[0].rings[0], &gl_devs[i].rrpp_domains[0], ringId, ringLevel, nodeType, masterPort, slavePort,helloInterval, helloFailTime);
		sw_rrpp_init_frame(&gl_devs[i]);
	nodeId = i = 1;
	nodeType = RNODE_TRANSFER;
		sw_rrpp_init_device(&gl_devs[i], domainNum);
		sw_rrpp_init_domain(&gl_devs[i].rrpp_domains[0], &gl_devs[i], domainId, ringNum, vlan, nodeId);
		sw_rrpp_init_ring(&gl_devs[i].rrpp_domains[0].rings[0], &gl_devs[i].rrpp_domains[0], ringId, ringLevel, nodeType, masterPort, slavePort,helloInterval, helloFailTime);
	ringNum = 2;
	nodeId = i = 2;
		sw_rrpp_init_device(&gl_devs[i], domainNum);
		sw_rrpp_init_domain(&gl_devs[i].rrpp_domains[0], &gl_devs[i], domainId, ringNum, vlan, nodeId);
		sw_rrpp_init_ring(&gl_devs[i].rrpp_domains[0].rings[0], &gl_devs[i].rrpp_domains[0], ringId, ringLevel, nodeType, masterPort, slavePort,helloInterval, helloFailTime);
	nodeType = RNODE_MASTER;
		sw_rrpp_init_ring(&gl_devs[i].rrpp_domains[0].rings[1], &gl_devs[i].rrpp_domains[0], ringId+1, ringLevel+1, nodeType, 2, -1,helloInterval, helloFailTime);
		sw_rrpp_init_frame(&gl_devs[i]);
	nodeId = i = 3;
		sw_rrpp_init_device(&gl_devs[i], domainNum);
		sw_rrpp_init_domain(&gl_devs[i].rrpp_domains[0], &gl_devs[i], domainId, ringNum, vlan, nodeId);
	nodeType = RNODE_TRANSFER;
		sw_rrpp_init_ring(&gl_devs[i].rrpp_domains[0].rings[0], &gl_devs[i].rrpp_domains[0], ringId, ringLevel, nodeType, masterPort, slavePort,helloInterval, helloFailTime);
		sw_rrpp_init_ring(&gl_devs[i].rrpp_domains[0].rings[1], &gl_devs[i].rrpp_domains[0], ringId+1, ringLevel+1, nodeType, -1, 2, helloInterval, helloFailTime);
		sw_rrpp_init_frame(&gl_devs[i]);
	ringId++;
	ringLevel++;
	nodeId = i = 4;
		sw_rrpp_init_device(&gl_devs[i], domainNum);
		sw_rrpp_init_domain(&gl_devs[i].rrpp_domains[0], &gl_devs[i], domainId, ringNum, vlan, nodeId);
		sw_rrpp_init_ring(&gl_devs[i].rrpp_domains[0].rings[0], &gl_devs[i].rrpp_domains[0], ringId, ringLevel, nodeType, masterPort, slavePort,helloInterval, helloFailTime);
	nodeType = RNODE_MASTER;
		sw_rrpp_init_ring(&gl_devs[i].rrpp_domains[0].rings[1], &gl_devs[i].rrpp_domains[0], ringId+1, ringLevel+1, nodeType, 2, -1,helloInterval, helloFailTime);
		sw_rrpp_init_frame(&gl_devs[i]);
	nodeId = i = 5;
		sw_rrpp_init_device(&gl_devs[i], domainNum);
		sw_rrpp_init_domain(&gl_devs[i].rrpp_domains[0], &gl_devs[i], domainId, ringNum, vlan, nodeId);
	nodeType = RNODE_TRANSFER;
		sw_rrpp_init_ring(&gl_devs[i].rrpp_domains[0].rings[0], &gl_devs[i].rrpp_domains[0], ringId, ringLevel, nodeType, masterPort, slavePort,helloInterval, helloFailTime);
		sw_rrpp_init_ring(&gl_devs[i].rrpp_domains[0].rings[1], &gl_devs[i].rrpp_domains[0], ringId+1, ringLevel+1, nodeType, -1, 2, helloInterval, helloFailTime);
		sw_rrpp_init_frame(&gl_devs[i]);
	ringId++;
	ringLevel++;
	ringNum = 1;
	nodeId = i = 6;
		sw_rrpp_init_device(&gl_devs[i], domainNum);
		sw_rrpp_init_domain(&gl_devs[i].rrpp_domains[0], &gl_devs[i], domainId, ringNum, vlan, nodeId);
		sw_rrpp_init_ring(&gl_devs[i].rrpp_domains[0].rings[0], &gl_devs[i].rrpp_domains[0], ringId, ringLevel, nodeType, masterPort, slavePort,helloInterval, helloFailTime);
		sw_rrpp_init_frame(&gl_devs[i]);
	nodeId = i = 7;
		sw_rrpp_init_device(&gl_devs[i], domainNum);
		sw_rrpp_init_domain(&gl_devs[i].rrpp_domains[0], &gl_devs[i], domainId, ringNum, vlan, nodeId);
		sw_rrpp_init_ring(&gl_devs[i].rrpp_domains[0].rings[0], &gl_devs[i].rrpp_domains[0], ringId, ringLevel, nodeType, masterPort, slavePort,helloInterval, helloFailTime);
		sw_rrpp_init_frame(&gl_devs[i]);

	

	//init link
	initLink(&gl_links[0], &gl_devs[0].ports[0], &gl_devs[1].ports[1]);	   
	initLink(&gl_links[1], &gl_devs[2].ports[0], &gl_devs[0].ports[1]);	   
	initLink(&gl_links[2], &gl_devs[1].ports[0], &gl_devs[3].ports[1]);	   
	initLink(&gl_links[3], &gl_devs[3].ports[0], &gl_devs[2].ports[1]);	   
	initLink(&gl_links[4], &gl_devs[2].ports[2], &gl_devs[4].ports[1]);	   
	initLink(&gl_links[5], &gl_devs[5].ports[0], &gl_devs[3].ports[2]);	   
	initLink(&gl_links[6], &gl_devs[4].ports[0], &gl_devs[5].ports[1]);	   
	initLink(&gl_links[7], &gl_devs[4].ports[2], &gl_devs[6].ports[1]);	   
	initLink(&gl_links[8], &gl_devs[7].ports[0], &gl_devs[5].ports[2]);	   
	initLink(&gl_links[9], &gl_devs[6].ports[0], &gl_devs[7].ports[1]);	   
	
	//Configuration end
	
	//Start RRPP test
	
	initLogger(&logger);
	for(i = 0; i < DEV_NUMBER; i++)
	{
		sw_rrpp_start(&gl_devs[i]);
	}	
	char line[100] = "";
	
	printf("test started \n");
	char* cmd = line;
	char* buf = line;
	int linkId;
	while(strcmp("quit", cmd) != 0 && strcmp("q", cmd) != 0)
	{
		scanf("%s", cmd);
		buf = line;
		if(strcmp("show", cmd) == 0)
		{
			for(i = 0; i < LINK_NUMBER; i++) 
			{
				printLink(&gl_links[i]);
			}
		}
		if(strcmp("down", cmd) == 0)
		{
			scanf("%d", &linkId);
			changeLinkStatus(&gl_links[linkId], RLINK_DOWN);
		}
		if(strcmp("up", cmd) == 0)
		{
			scanf("%d", &linkId);
			changeLinkStatus(&gl_links[linkId], RLINK_UP);
		}
		if(strcmp("auto", cmd) == 0)
		{

			printf("press any key to disable link between node 1 & 2\n");
			getchar();
			changeLinkStatus(&gl_links[1], RLINK_DOWN);
			
			printf("link status changed\n");
			printf("press any key to enable link between node 1 & 2\n");
			getchar();
			changeLinkStatus(&gl_links[1], RLINK_UP);
		}
	
	}
	for(i = 0; i < DEV_NUMBER; i++)
	{
		sw_rrpp_stop(&gl_devs[i]);
		sw_rrpp_destroy(&gl_devs[i]);
	}
	destroyLogger(&logger);
	//RRPP test end 
	return 0;
}

