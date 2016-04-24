#include "rpkg.h"
#include <stdio.h>
#include <string.h>
#define MAC_ADDR_SIZE 48

void assignFrameField(struct sw_frame* frame, int start, int length, u8* val)
{
	int i = 0;
	for (i = start; i < start + length; i++)
	{
		frame->frame_data[i] = val[i];
	}
}
void setMacAddr(u8* des, u8* src)
{
	int i = 0;
	if(src == NULL)
	{	
		for(; i < 6; i++)
		{
			des[i] = 0; 
		}
		return;
	}
	for(; i < 6; i++)
	{
		des[i] = src[i];
	} 
}

int forwardPkg(struct sw_dev* dev, const struct sw_frame* frame, int mask)
{
	printf("forwarding Pkg\n");
	struct rrpp_frame* pkg = (struct rrpp_frame*) frame->frame_data;
	struct rrpp_frame* data = (struct rrpp_frame*) (dev->rrpp_frame).frame_data;
	
	memcpy(data->src_mac_addr, pkg->src_mac_addr, MAC_ADDR_SIZE); 
	memcpy(data->des_mac_addr, pkg->des_mac_addr, MAC_ADDR_SIZE);		
	pkg->rrpp_type = data->rrpp_type;
	return sw_send_frame_virt(dev, &(dev->rrpp_frame), mask);
}
int sendUpPkg(struct sw_dev* dev, const struct sw_frame* frame, int mask) 
{
	printf("sending up pkg\n");
	struct rrpp_frame* pkg = (struct rrpp_frame*) frame->frame_data;
	struct rrpp_frame* data = (struct rrpp_frame*) (dev->rrpp_frame).frame_data;
	
	memcpy(data->src_mac_addr, pkg->src_mac_addr, MAC_ADDR_SIZE); 
	memcpy(data->des_mac_addr, pkg->des_mac_addr, MAC_ADDR_SIZE);
	pkg->rrpp_type = RPKG_LINK_UP;	
	
	return sw_send_frame_virt(dev, &(dev->rrpp_frame), mask);
}
int sendHelloPkg(struct sw_dev* dev, int mask) 
{
	  
	//struct rrpp_frame* pkg = (struct rrpp_frame*) frame->frame_data;
	struct rrpp_frame* data = (struct rrpp_frame*) (dev->rrpp_frame).frame_data;
	
	setMacAddr(data->des_mac_addr, data->sys_mac_addr);
	data->rrpp_type = RPKG_HELLO;
	data->hello_seq = helloSeq;
	return sw_send_frame_virt(dev, &(dev->rrpp_frame), mask);
}
int createHelloFrame(struct sw_frame* frame, int hello_seq)
{
	memcpy(frame, &(dev->rrpp_frame), sizeof(struct sw_frame));
	
	struct rrpp_frame* data = (struct rrpp_frame*) frame->frame_data;
	
	setMacAddr(data->des_mac_addr, data->sys_mac_addr);
	data->rrpp_type = RPKG_HELLO;
	data->hello_seq = hello_seq;
	return 0;
}
int sendDownPkg(struct sw_dev* dev, const struct sw_frame* frame, int mask) 
{
	struct rrpp_frame* pkg = (struct rrpp_frame*) frame->frame_data;
	struct rrpp_frame* data = (struct rrpp_frame*) (dev->rrpp_frame).frame_data;
	
	memcpy(data->src_mac_addr, pkg->src_mac_addr, MAC_ADDR_SIZE); 
	memcpy(data->des_mac_addr, pkg->des_mac_addr, MAC_ADDR_SIZE);
	data->rrpp_type = RPKG_LINK_DOWN;
	return sw_send_frame_virt(dev, &(dev->rrpp_frame), mask);
}
int sendCommonFlushPkg(struct sw_dev* dev, const struct sw_frame* frame, int mask) 
{
	struct rrpp_frame* pkg = (struct rrpp_frame*) frame->frame_data;
	struct rrpp_frame* data = (struct rrpp_frame*) (dev->rrpp_frame).frame_data;
	
	memcpy(data->src_mac_addr, pkg->src_mac_addr, MAC_ADDR_SIZE); 
	memcpy(data->des_mac_addr, pkg->des_mac_addr, MAC_ADDR_SIZE);
	data->rrpp_type = RPKG_COMMON_FLUSH_FDB;
	return sw_send_frame_virt(dev, &(dev->rrpp_frame), mask);
}
int sendCompleteFlushPkg(struct sw_dev* dev, const struct sw_frame* frame, int mask) 
{
	struct rrpp_frame* pkg = (struct rrpp_frame*) frame->frame_data;
	struct rrpp_frame* data = (struct rrpp_frame*) (dev->rrpp_frame).frame_data;
	
	memcpy(data->src_mac_addr, pkg->src_mac_addr, MAC_ADDR_SIZE); 
	memcpy(data->des_mac_addr, pkg->des_mac_addr, MAC_ADDR_SIZE);
	data->rrpp_type = RPKG_COMPLETE_FLUSH_FDB;
	return sw_send_frame_virt(dev, &(dev->rrpp_frame), mask);
}


struct sw_frame* getFrameModule(struct sw_dev* dev)
{
	return &(dev->rrpp_frame);
}

int initRrppFrame(struct sw_dev *dev);
{
	dev->rrpp_frame.length = 48 * 16;
	struct rrpp_frame* data = (struct rrpp_frame*)dev->rrpp_frame.frame_data;
        struct sw_mac_addr emptyAddr;
	u8 srcMac[] = {0x00,0x0f,0xe2,0x03,0xfd,0x75};
	setMacAddr(data->des_mac_addr, NULL);
	setMacAddr(data->src_mac_addr, srcMac);
	data->ether_type = 0x8100;
	data->pri_and_vlan_id = 0xE000 + dev->vlan_id;    
	data->frame_length = 0x48; 
	data->dsap_or_ssap = 0xAAAA; 
	data->control = 0x03;       
	data->oui[0] = 0x00;
	data->oui[1] = 0xe0;
       	data->oui[2] = 0x2b;	
	data->constants1[0] = 0x00;
	data->constants1[1] = 0xbb;
	data->constants1[2] = 0x99;
	data->constants1[3] = 0x0b;
	data->rrpp_length = 0x0040; 	
        data->rrpp_vers = 0x01;
	data->rrpp_type = 0;               // to be set 
	data->domain_id = 0;               // to be set
	data->ring_id = 0;                 // to be set
	data->zeroes1 = 0;
	setMacAddr(data->sys_mac_addr, dev->local_mac_addr.val);
	data->hello_timer = dev->hello_interval;
	data->fail_timer = dev->hello_fail_time;
	data->zeroes2 = 0;
	data->level = 0;		   // to be set
	data->hello_seq = 0;		   // to be set
	data->zeroes3 = 0;
        int i = 0;
	for(; i < 36; i++)
	{
		data->reserved[i] = 0;
	}
		
	return 0;
} 
int getRpkgType(const struct sw_frame* frame)
{
	struct rrpp_frame* pframe = (struct rrpp_frame*) frame->frame_data;
	return pframe->rrpp_type;
}
int getRpkgRingId(const struct sw_frame* frame)
{
	struct rrpp_frame* pframe = (struct rrpp_frame*) frame->frame_data;
	return pframe->ring_id;
}

int getRpkgHelloSeq(const struct sw_frame* frame)
{
	return (struct rrpp_frame*) frame->frame_data->hello_seq;
}


