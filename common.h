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

#define RPORT_STATUS_BLOCK 001
#define RPORT_STATUS_PREFORWARDING 002
#define RPORT_STATUS_DOWN 003
#define RPORT_STATUS_UP 004 

#define RRING_COMPLETE 0
#define RRING_FAIL 1
#define RRING_INIT 2

struct sw_frame {
	s32 length;
	u8 frame_data[SW_MAX_ETH_LEN] ;
};
struct sw_mac_addr {
	u8 val[6];
};
struct sw_port 			// virt
{
	int id;
	int type;           // 1: Master 0:slave
	int ring_id; 
	int status;
};
// describe dev info on certain rrpp ring 
struct rrpp_ring
{
	struct rrpp_domain* pdomain;
	u8 ring_id;
	u8 ring_level;
	u8 node_type;
	u8 status;
	
	u8 slave_port;
	u8 master_port;

	u8 hello_seq;
	
	u16 hello_interval;
	u16 hello_fail_time;
	
        struct sw_frame rrpp_frame;
	
	pthread_t polling_id;
	pthread_t hello_fail_id;
	u8 arrived_hello_seq;
	
	pthread_t flush_wait_id;
	u8 wait_port;
	
};
struct rrpp_domain
{
	struct sw_dev* pdev;
	u8 domain_id;
	u8 vlan_id;
	u8 node_id;
	u8 ring_num;
	struct rrpp_ring* rings;
};

struct sw_dev {
	struct sw_mac_addr local_mac_addr;

	u8 port_number;
	struct sw_port* ports;
	u8 domain_num;
	struct rrpp_domain* rrpp_domains;
};
#endif
