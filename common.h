#ifndef THSS_SWITCH_COMMON
#define THSS_SWITCH_COMMON
#include <stdint.h>
#include <pthread.h>
#include "testenv.h"

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#define SW_MAX_ETH_LEN (1522)

#define PORT_UP        1
#define PORT_DOWN      0

#define RRPP_FRAME_TYPE_NUMBER 10 
#define MAX_POSSIBLE_RING_NUMBER 100

struct sw_frame {
	s32 length;
	u8 frame_data[SW_MAX_ETH_LEN];
};
struct sw_mac_addr {
	u8 val[6];
};
struct sw_port 			// virt
{
	u8 id;
	u8 status;
	struct rrpp_link* plink;
	struct rrpp_port* pport;
};
struct rrpp_port
{
	struct rrpp_ring* pring;    //pointer to parent

	struct sw_port* virt_port;  //null : not exist  
	u8 type;           // 1: Master 0:slave
	u8 status;
};

// 
struct rrpp_ring
{
	struct rrpp_domain* pdomain; //pointer to parent 

	u8 ring_id;
	u8 ring_level;
	u8 node_type;
	u8 status;
	
	struct rrpp_port slave_port;
	struct rrpp_port master_port;

	u32 seq[RRPP_FRAME_TYPE_NUMBER];
	
	u32 hello_interval;
	u32 hello_fail_time;
	
        struct sw_frame rrpp_frame;
	
	pthread_t polling_id;
	pthread_t hello_fail_id;
	
	pthread_t flush_wait_id;
	u32 wait_port;
	
};

struct rrpp_domain
{
	struct sw_dev* pdev;
	u8 domain_id;
	u8 vlan_id;
	u8 node_id;

	u8 ring_num;
	struct rrpp_ring* rings;
	u32 history_seqs[RRPP_FRAME_TYPE_NUMBER * MAX_POSSIBLE_RING_NUMBER];
};

struct sw_dev {
	struct sw_mac_addr local_mac_addr;

	u8 port_num;
	struct sw_port* ports;
	u8 domain_num;
	struct rrpp_domain* rrpp_domains;
};
#endif
