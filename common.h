#ifndef THSS_SWITCH_COMMON
#define THSS_SWITCH_COMMON
#include <stdint.h>
#include <pthread.h>
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#define SW_MAX_ETH_LEN (1522)
#define RPORT_TYPE_MASTER 1
#define RPORT_TYPE_SLAVE 0

#define RPS_BLOCK 001
#define RPS_PREFORWARDING 002
#define RPS_DOWN 003
#define RPS_UP 004 

struct sw_port 			// virt
{
	int id;
	int type;           // 1: Master 0:slave
	int ring_id; 
	int status;
	int the_other_port; // in the same ring
};
// describe dev info on certain rrpp ring 
struct rrpp_ring
{
	u8 ring_id;
	struct sw_port slave_port;
	struct sw_port master_port;
};
struct rrpp_domain
{
	struct rrpp_ring* rings;
	u8 domain_id;
};
struct sw_frame {
	s32 length;
	u8 frame_data[SW_MAX_ETH_LEN] ;
};
struct sw_mac_addr {
	u8 val[6];
};
struct sw_dev {
	u8 vlan_id;
	u8 node_id;
	u8 node_type;
	
	u8 port_number;
	struct sw_port* ports;

	u8 hello_expire_time;
	u8 hello_expire_seq;
	u8 hello_seq;
	
	u8 hello_interval;
	u8 hello_fail_time;
	
	struct sw_mac_addr local_mac_addr;
	
        struct sw_frame rrpp_frame;
	
	pthread_t polling_id;
	pthread_t hello_fail_id;
	pthread_key_t hello_wait_key;	
	pthread_cond_t hello_recieved;
};
#endif
