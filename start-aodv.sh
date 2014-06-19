#!/bin/sh

#CONFIG MESH DEVICE
ifconfig eth1 192.168.0.22 netmask 255.255.255.0
#NFNETLINK is necessary
modprobe nfnetlink

#MESH DEVICE
MESH_DEV="mesh_dev=eth1"

#MESH NETWORK&IP
AODV_NET="network_ip=192.168.0.22/255.255.255.0"

#I'm a gateway - Sending ST-RREQ
GATEWAY="aodv_gateway=0"

#Routing metric to be used - HOPS, ETT or WCIM
METRIC="routing_metric=ETT"

#Nominal rate if is fixed and ETT estimation is not required
#In Mbps*10 (E.g. 6Mbps = 60)
RATE="nominal_rate=60"

#IP_FORWARD y ROUTE_CACHE
echo "1" > /proc/sys/net/ipv4/ip_forward
echo "0" > /proc/sys/net/ipv4/route/min_delay

echo "Running AODV-MCC"
rmmod fbaodv_proto
insmod fbaodv_proto.ko $MESH_DEV $AODV_NET $METRIC $RATE $GATEWAY
