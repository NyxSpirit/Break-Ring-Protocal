#include <stdio.h>
#include <string.h>
#include "testenv.h"
#include "rpkg.h"
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

int forwardPkg(struct rrpp_domain* domain, const struct sw_frame* frame, int mask)
{
	//struct rrpp_frame* pkg = (struct rrpp_frame*) frame->frame_data;
	//struct rrpp_frame* data = (struct rrpp_frame*) (ring->rrpp_frame).frame_data;
	
	//data->rrpp_type = pkg->rrpp_type;
	//data->seq = pkg->seq[RPKG_HELLO];
	return sw_send_frame_virt(domain->pdev, frame, mask);
}

int sendHelloPkg(struct rrpp_ring* ring, int mask) 
{
	//struct rrpp_frame* pkg = (struct rrpp_frame*) frame->frame_data;
	struct rrpp_frame* data = (struct rrpp_frame*) (ring->rrpp_frame).frame_data;
	//setMacAddr(data->des_mac_addr, data->sys_mac_addr);
	ring->seq[RPKG_HELLO]++;  
	data->rrpp_type = RPKG_HELLO;
	data->seq = ring->seq[RPKG_HELLO];
	return sw_send_frame_virt(ring->pdomain->pdev, &(ring->rrpp_frame), mask);
}

int createHelloFrame(struct rrpp_ring* ring, struct sw_frame* frame, int seq)
{
	memcpy(frame, &(ring->rrpp_frame), sizeof(struct sw_frame));
	
	struct rrpp_frame* data = (struct rrpp_frame*) frame->frame_data;
	
	//setMacAddr(data->des_mac_addr, data->sys_mac_addr);
	data->rrpp_type = RPKG_HELLO;
	data->seq = seq;
	return 0;
}
int sendUpPkg(struct rrpp_ring* ring,int mask) 
{
	struct rrpp_frame* data = (struct rrpp_frame*) (ring->rrpp_frame).frame_data;
	int type = RPKG_LINK_UP;

	ring->seq[type] ++;
	data->rrpp_type = type;
	data->seq = ring->seq[type];
	return sw_send_frame_virt(ring->pdomain->pdev, &(ring->rrpp_frame), mask);
}
int sendDownPkg(struct rrpp_ring* ring, int mask) 
{
	struct rrpp_frame* data = (struct rrpp_frame*) (ring->rrpp_frame).frame_data;
	int type = RPKG_LINK_DOWN;
	ring->seq[type] ++;
	data->rrpp_type = type;
	data->seq = ring->seq[type];
	return sw_send_frame_virt(ring->pdomain->pdev, &(ring->rrpp_frame), mask);
}
int sendCommonFlushPkg(struct rrpp_ring* ring, int mask) 
{
	struct rrpp_frame* data = (struct rrpp_frame*) (ring->rrpp_frame).frame_data;
	
	int type = RPKG_COMMON_FLUSH_FDB;
	ring->seq[type] ++;
	data->rrpp_type = type;
	data->seq = ring->seq[type];
	return sw_send_frame_virt(ring->pdomain->pdev, &(ring->rrpp_frame), mask);
}
int sendCompleteFlushPkg(struct rrpp_ring* ring, int mask) 
{
	struct rrpp_frame* data = (struct rrpp_frame*) (ring->rrpp_frame).frame_data;
	
	int type = RPKG_COMPLETE_FLUSH_FDB;
	ring->seq[type] ++;
	data->rrpp_type = type;
	data->seq = ring->seq[type];
	return sw_send_frame_virt(ring->pdomain->pdev, &(ring->rrpp_frame), mask);
}



int initRrppFrame(struct rrpp_ring* ring)
{
	struct sw_dev* dev = ring->pdomain->pdev;
	ring->rrpp_frame.length = 48 * 16;
	struct rrpp_frame* data = (struct rrpp_frame*)ring->rrpp_frame.frame_data;
        struct sw_mac_addr emptyAddr;
	u8 srcMac[] = {0x00,0x0f,0xe2,0x03,0xfd,0x75};
	setMacAddr(data->des_mac_addr, NULL);
	setMacAddr(data->src_mac_addr, srcMac);
	data->ether_type = 0x8100;
	data->pri_and_vlan_id = 0xE000 + ring->pdomain->vlan_id;    
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
	data->domain_id = ring->pdomain->domain_id;               // to be set
	data->ring_id = ring->ring_id;                 // to be set
	data->zeroes1 = 0;
	setMacAddr(data->sys_mac_addr, dev->local_mac_addr.val);
	data->hello_timer = ring->hello_interval;
	data->fail_timer = ring->hello_fail_time;
	data->zeroes2 = 0;
	data->level = ring->ring_level;		   // to be set
	data->seq =  0;		   // to be set
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

int getRpkgSeq(const struct sw_frame* frame)
{
	struct rrpp_frame* pframe = (struct rrpp_frame*) frame->frame_data;
	return pframe->seq;
}

int getRpkgRingLevel(const struct sw_frame* frame)
{
	return ((struct rrpp_frame*) frame->frame_data) -> level;
}

int getRpkgDomainId(const struct sw_frame* frame)
{
	return ((struct rrpp_frame*) frame->frame_data) -> domain_id;
}

