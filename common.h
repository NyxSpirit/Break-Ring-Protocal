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
	u8 hello_expire_time;
	u8 hello_expire_seq;
	u8 hello_seq;
	
	u8 hello_interval;
	u8 hello_fail_time;
	
	struct sw_mac_addr local_mac_addr;
	
        struct sw_frame rrpp_frame;

	pthread_t polling_id;
};
#endif
