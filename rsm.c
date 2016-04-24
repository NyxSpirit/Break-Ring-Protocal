#include "rsm.h"
#include <sys/time.h> 
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>
#include "testenv.h"
#include <string.h>

#define RRPP_PORT_STATUS_BLOCK 001
#define RRPP_PORT_STATUS_PREFORWARDING 002
#define RRPP_PORT_STATUS_DOWN 003
#define RRPP_PORT_STATUS_UP 004 

struct Port* ports;

int portNumber;

int sw_rrpp_init(struct sw_dev* dev, int ntype, int nid, int portNum, int *portType, struct sw_mac_addr localMac)
{
	dev->vlan_id = 1;
	dev->hello_interval= 2;
	dev->hello_fail_time = 4;
	dev->hello_seq = 0;
	dev->node_id = nid;
	dev->node_type = ntype;
	portNumber = portNum;
	dev->hello_expire_time = -1;
	dev->hello_expire_seq= -1;
	memcpy(&dev->local_mac_addr,&localMac, sizeof(struct sw_mac_addr));

	int ringId = 0;
	ports = (struct Port*) malloc(sizeof(struct Port) * portNum);
	int i = 0;
	for(i = 0; i < portNum; i++)
	{
		ports[i].isMaster = portType[i];
		ports[i].status = 0;
		ports[i].ringId = ringId; 
		disablePort(i);
	}
	initRrppFrame(struct sw_dev* dev);
	//printFrame(getFrameModule());
	return 0;
} 

int sw_rrpp_start(struct sw_dev* dev)
{
	printf("RRPP start up \n");
	if(dev->node_type == RNODE_MAIN)
	{
		startHello(struct sw_dev* dev);
	} else
	{
	// no operation 	
	}
	return 0;	 
}

int sw_rrpp_stop(struct sw_dev* dev)
{
	printf("RRPP stop \n");
	stopHello(struct sw_dev* dev);
	return 0;
}
int sw_rrpp_destroy(struct sw_dev* dev)
{
	printf("RRPP destroy \n");
	free(ports);
	return 0;
}

int sw_rrpp_frame_handler(struct sw_dev* dev, const struct sw_frame *frame, int from_port)
{
	int frametype = getRpkgType(frame);
       	int ringId = getRpkgRingId(frame);
	if(dev->node_type == RNODE_MAIN)	
	{
		switch (frametype) 
		{
			case RPKG_HELLO:
				//recievedHelloAction();
				updateExpireTimer(getRpkgHelloSeq(frame));
				blockPort(getSlavePortId(ringId));
				sendCompleteFlushPkg(dev, frame, getMasterPortId(ringId) );
				sendCompleteFlushPkg(dev, frame, getSlavePortId(ringId) );
				//stopTimer(getHelloExpireTimer(helloSeq));
				break;
			case RPKG_LINK_UP:
				//recievedUpAction();
				blockPort(getSlavePortId(ringId));
				sendCommonFlushPkg(dev, frame, getMasterPortId(ringId) );
				sendCommonFlushPkg(dev, frame, getSlavePortId(ringId) );
				break;
			case RPKG_LINK_DOWN:
				//recievedDownAction();
				enablePort(getSlavePortId(ringId));
				sw_flush_fdb(dev);
				sendCommonFlushPkg(dev, frame, getMasterPortId(ringId));
				break;
			default:
				//forwardPkg(frame, from_port);
				;
		}
	} else if(dev->node_type == RNODE_TRANSFER)
	{
		switch (frametype)
		{
			case RPKG_COMMON_FLUSH_FDB:
				sw_flush_fdb(dev);
				forwardPkg(dev, frame, getTheOtherPortId(from_port));
				//recievedCommonFAction();
				break;
			case RPKG_COMPLETE_FLUSH_FDB:
				enablePort(getPreforwardingPortId(ringId));
				sw_flush_fdb(dev);
				//recievedCompleteFAction();
				break;
			default:
				forwardPkg(dev, frame, from_port);
		}

	}

	return 0;
}
int getTheOtherPortId(int port){ return 1 - port;}
int getSlavePortId(int ringId){ return 1;}
int getMasterPortId(int ringId){ return 0;}
int getPreforwardingPortId(int ringId){ return 0;}
int blockPort(int portId) { printf("disable Port id %d\n", portId); return 0;}
int enablePort(int portId) { printf("enable Port id %d\n", portId); return 0;}
int disablePort(int portId) { printf("disable Port id %d\n", portId); return 0;}
int preforwardPort(int portId)
{ 
	printf("set port status preforwarding %d\n", portId); 
	//startPreforwardingTimeOut();
	return 0;
}

int sw_rrpp_link_change(struct sw_dev* dev, int port, int link_up)
{
	if(dev->node_type == RNODE_MAIN)
		return 0;
	if(link_up == 1)
	{
		sendUpPkg(dev, getFrameModule(), getTheOtherPortId(port));
		preforwardPort(port);
	}else
	{
		blockPort(port);
		sendDownPkg(dev, getFrameModule(), getTheOtherPortId(port));
	}
	return 0;
}
void helloTimeout (int signum, siginfo_t *info, void* dev_para)
{
	struct sw_dev* dev = (struct sw_dev*) dev_para;	struct itimerval t; 
	signal(SIGALRM, helloHandler);
	t.it_interval.tv_usec = 0;
	t.it_interval.tv_sec = helloInterval - helloExpireTime;
	t.it_value.tv_usec = 0;
	t.it_value.tv_sec = 0;

	helloExpireSeqNum ++;
	helloExpireTime += helloInterval;
			
}
void helloHandler (int a)
{
	dev-> helloSeq += 1;
	sendHelloPkg(getMasterPortId(0));

	if(helloExpireTime < helloInterval)
	{
		//hello fail
		struct itimerval t; 
		signal(SIGALRM, helloTimeout);
		t.it_interval.tv_usec = 0;
		t.it_interval.tv_sec = helloExpireTime - helloInterval;
		t.it_value.tv_usec = 0;
		t.it_value.tv_sec = 0;
		struct sigaction act;
		act.sa_sigaction= helloTimeout;
		act.sa_flags = SA_SIGINFO;
				
	}
	//set new expire timer 
	
}
void updateExpireTimer(int seqNo)
{
	if(seqNo >= helloExpireSeqNum)
	{
		helloExpireTime = helloExpireTime + (seqNo - helloExpireSeqNum + 1) * helloInterval; 
		helloExpireSeqNum = seqNo + 1;
	}	
}
 
void polling(struct sw_dev *dev)
{
	printf("start Hello proc\n");
	
}

int startHello(struct sw_dev* dev) 
{
	
	pthread_create( &dev->polling_id, NULL, (void *)polling, (void *)dev);

	return 0; 
}
int stopHello(struct sw_dev* dev) {
	printf("stop Hello proc\n");
	pthread_cancel(dev->polling_id);
	//signal(SIGALRM, SIG_IGN);
	return 0;
}
/*
 *sleep(10);
	struct sigaction act;
	act.sa_sigaction = helloHandler;
	act.sa_flags = SA_SIGINFO;
	struct itimerval t; 
	sigaction(SIGALRM, &act, NULL);
	t.it_interval.tv_usec = 0;
	t.it_interval.tv_sec = helloInterval;
	t.it_value.tv_usec = 0;
	t.it_value.tv_sec = 0;
	setitimer(0, &t, NULL); //
	union sigval dev_para;
	dev_para.sival_ptr = dev;
	sigqueue(getpid(), SIGALRM, dev_para); //
	signal(SIGALRM, startHelloExpireTimer(getFrameModule(), getMasterPortId()); //
	

 * */
