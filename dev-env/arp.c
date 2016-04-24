#include "common.h"

/*
 * 	_init includes configuration,
 * 	it is expected to allocate enough memory for later calls on the
 * 	device.
 * 	_setup starts the operation of the feature on dev.
 * 	_stop stops the operation of the feature on dev, stops all related
 * 	timers on dev, and clear temperory data.
 * 	_destroy removes everything related to this module on dev.
 *
 * 	State Machine:
 *  			     _init	       _start
 * 	Pre-initialization <-------> Stopped <--------> Started
 * 			   _destroy		_stop
 *
 *
 *	The functions are xpected to return 0 on success, -1 on failure.
 *
 *	Only _init is expected to call sw_mall
 */

int sw_arp_init(struct sw_dev *dev, ...)
{
}

int sw_arp_start(struct sw_dev *dev)
{
}

int sw_arp_stop(struct sw_dev *dev)
{
}

int sw_arp_destroy(struct sw_dev *dev)
{
}

int sw_arp_handle_frame(struct sw_dev *dev, const sw_frame *frame)
{
}

int sw_arp_set_monitor(struct sw_dev *dev, struct sw_ipv4_addr src_addr,
		struct sw_ipv4_addr dst_addr, u16 flow_id)
{
}
