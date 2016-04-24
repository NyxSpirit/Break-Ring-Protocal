#include "common.h"


int sw_change_phy_port(sw_dev *dev, int port,
		const struct sw_port_attr *attr);
int sw_recv_frame(sw_dev *dev, const sw_frame *frame, int from_port);
