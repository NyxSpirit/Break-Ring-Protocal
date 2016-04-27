#ifndef RPKG_H_
#define RPKG_H_
#include "common.h"
#include "testenv.h"

enum rrpp_node_type {
	RNODE_MAIN,
	RNODE_TRANSFER,
	RNODE_EDGE_MAIN,
	RNODE_EDGE_SUPPORT
};

enum rrpp_pkg_type {
	RPKG_HELLO,
	RPKG_LINK_UP,
	RPKG_LINK_DOWN,
	RPKG_COMMON_FLUSH_FDB,
	RPKG_COMPLETE_FLUSH_FDB,
	RPKG_EDGE_HELLO,
	RPKG_MAJOR_FAULT
};


struct rrpp_frame
{
	u8 des_mac_addr[6];
	u8 src_mac_addr[6];          
	
	u16 ether_type;
	u16 pri_and_vlan_id;
	u16 frame_length;
	
	u16 dsap_or_ssap;
	u8 control;
	u8 oui[3];

	u8 constants1[4];	
	u16 rrpp_length;

	u8 rrpp_vers;
	u8 rrpp_type;
	u16 domain_id;
	u16 ring_id;

	u16 zeroes1;
	u8 sys_mac_addr[6];
	u16 hello_timer;
	u16 fail_timer;

	u8 zeroes2;
	u8 level;
	u16 hello_seq;
	u16 zeroes3;

	u8 reserved[36];
};

int initRrppFrame(struct rrpp_ring *ring);
int forwardPkg(struct rrpp_ring *ring, const struct sw_frame* frame, int mask);
int sendUpPkg(struct rrpp_ring *ring, const struct sw_frame* frame, int mask); 
int sendHelloPkg(struct rrpp_ring *ring, int mask); 
int sendDownPkg(struct rrpp_ring *ring, const struct sw_frame* frame, int mask); 
int sendCommonFlushPkg(struct rrpp_ring *ring, const struct sw_frame* frame, int mask); 
int sendCompleteFlushPkg(struct rrpp_ring *ring, const struct sw_frame* frame, int mask); 
struct sw_frame* getFrameModule(struct rrpp_ring *ring);
int getRpkgRingId(const struct sw_frame* frame);
int getRpkgType(const struct sw_frame* frame);
int getRpkgHelloSeq(const struct sw_frame* frame);
int getRpkgDomainId(const struct sw_frame* frame);

void printFrame(const struct sw_frame* frame);
int createHelloFrame(struct rrpp_ring *ring, struct sw_frame* frame, int hello_seq);

#endif
