#ifndef THSS_SWITCH_COMMON
#define THSS_SWITCH_COMMON

#ifndef USER_MODE
#include <asm/types.h>
#else
#include <stdint.h>
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
#endif

#define SW_MAX_ETH_LEN (1522)

/*
 *	Malloc and free memory
 *	As libc.malloc, the memory returned is not zero-filled
 *	sw_malloc returns a valid pointer on success,
 *		or NULL on failure
 *	sw_free has no return value
 */
void *sw_malloc(int size);
void sw_free(void *ptr);


/*
 *	For upper layers
 */
struct sw_timespec {
	s64	secs;
	long	nsecs;
};
struct sw_timer_impl;
struct sw_timer {
	sw_timer_impl *impl;
};
struct sw_dev;


/*
 *	Timeout Handler
 *	In kernel mode, runs as a normal function
 *	In user mode, runs as a POSIX thread
 *	In both modes, the function is assumed to run inside a lock
 *	Thus, no lock is required
 */
typedef void (*sw_timer_handler)(struct sw_dev *dev, void *opaque);

/*
 *	sw_timer_setup: setup a timer, with handler and data object
 *	sw_timer_delete: delete a timer
 *	sw_timer_modify: modify a timer, by relative time from now
 *	sw_timer_get: get the time until next expiration
 *
 *	return 0 on success, -1 otherwise
 */
int sw_timer_setup(struct sw_dev *dev, struct sw_timer *timer,
		sw_timer_handler handler, void *opaque);
int sw_timer_delete(struct sw_dev *dev, struct sw_timer *timer);
int sw_timer_modify(struct sw_dev *dev, struct sw_timer *timer, 
		const struct sw_timespec *expire);
int sw_timer_get(struct sw_dev *dev, struct sw_timer *timer,
		struct sw_timespec *expire);


struct sw_frame {
	s32 length;
	u8 frame_data[SW_MAX_ETH_LEN] data;
};

/*
 *	Send frame
 *	sw_send_frame_any : switch selects the outgoing port
 *	sw_send_frame_phy : physical ports are selected,
 *			phy port i is selected iff (mask & (1 << i))
 *	sw_send_frame_virt : virtual ports are selected,
 *			virtual port i is selected iff (mask & (i << i))
 *
 *	return 0 on success, -1 otherwise (not decided)
 */
int sw_send_frame_any(struct sw_dev *dev, const struct sw_frame *frame);
int sw_send_frame_phy(struct sw_dev *dev, const struct sw_frame *frame, 
		u32 mask);
int sw_send_frame_virt(struct sw_dev *dev, const struct sw_frame *frame,
		u32 mask);


struct sw_mac_addr {
	u8 val[6];
};

/*
 *	get mac address of a physical or virtual port
 *	return 0 on success, -1 otherwise (not decided)
 */
int sw_get_port_addr_phy(struct sw_dev *dev, int port,
		struct sw_mac_addr *mac);
int sw_get_port_addr_virt(struct sw_dev *dev, int port,
		struct sw_mac_addr *mac);


/*
 *	Clear forward database, used by RRPP
 *	return 0 on success, -1 otherwise (not decided)
 */
int sw_flush_fdb(struct sw_dev *sw);

/*
 *	set virtual port status, used by RRPP
 *	return 0 on success, -1 otherwise (not decided)
 */
int sw_enable_virtual_port(struct sw_dev *dev, int port, int enabled);


/*
 *	set physical port status, used by LACP
 *	return 0 on success, -1 otherwise (not decided)
 */
int sw_set_lacp_tx_state(struct sw_dev *dev, int port, int enabled);
int sw_set_lacp_rx_state(struct sw_dev *dev, int port, int enabled);


enum sw_link_speed {
	SW_PORT_SPEED_10Mbps,
	SW_PORT_SPEED_100Mbps,
	SW_PORT_SPEED_1Gbps,
	SW_PORT_SPEED_OTHER
};
struct sw_port_attr {
	int	duplex;
	int	link_up;
	int	shared;
	enum sw_link_speed speed;
};
int sw_get_port_attr(struct sw_dev *dev, int port, 
		struct sw_port_attr *attr);


/*
 *	Change the status of virtual ports, 
 */
int sw_change_virt_port(struct sw_dev *dev, int port, int link_up);


struct sw_ipv4_addr {
	u8	val[4];
};

#endif
