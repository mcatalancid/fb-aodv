/***************************************************************************
 packet_out.c  -  description
 -------------------
 begin                : Mon Aug 4 2003
 copyright            : (C) 2003 by Luke Klein-Berndt
 email                : kleinb@nist.gov
 ***************************************************************************/
/***************************************************************************
 Modified by Miguel Catalan Cid - miguel.catcid@gmail.com - Version: Mon Jan 1 2010
 ***************************************************************************/

#include "packet_out.h"

extern u_int32_t g_broadcast_ip;
extern u_int32_t g_mesh_ip;
extern u_int8_t g_aodv_gateway;
extern u_int32_t g_null_ip;
extern u_int8_t g_routing_metric;
extern int initialized;

unsigned int output_handler(unsigned int hooknum, struct sk_buff *skb,
		const struct net_device *in, const struct net_device *out, int (*okfn) (struct sk_buff *)) {

	unsigned char *ucp;
	struct iphdr *ip= ip_hdr(skb);
	u_int32_t source = g_null_ip;
	u_int32_t destination = g_null_ip;

	aodv_route *tmp_route;
	//	aodv_neigh *tmp_neigh;
	void *p = (uint32_t *) ip + ip->ihl;
	struct udphdr *udp = (struct udphdr *) p;

	if (!initialized) { // this is required otherwise kernel calls this function without insmod completing the module loading process.
		return NF_ACCEPT;
	}

	//Case 1: AODV-SIGNALLING or Broadcast - ACCEPT!
	ucp = (unsigned char *) &ip->daddr;
	if ( (ip->daddr == g_broadcast_ip) || ((ucp[3] & 0xff) == 0xff)
			|| (udp->dest == htons(AODVPORT)) || (udp->dest == htons(DHCP1))
			|| (udp->dest == htons(DHCP2))) // either all 255s or last octet is 255 or the AODVPORT (RREP, ETT...)
	{
		return NF_ACCEPT;
	}

	if (is_internal(ip->daddr))
		destination = ip->daddr;
	
	if (is_internal(ip->saddr))
		source = ip->saddr;

	//This packet should not be routed using FB-AODV
	// Its upto the kernel route table entry to handle these data packets.
	if (destination == source)
		return NF_ACCEPT;
	
	tmp_route = find_aodv_route(source, destination, ip->tos);

	if ((tmp_route == NULL) || (tmp_route->state == INVALID)) {
		if (source == g_mesh_ip || (source == g_null_ip && g_aodv_gateway)) {
			if (gen_rreq(source, destination, ip->tos))
				return NF_QUEUE;
				else
				return NF_DROP;
		}
		else if (destination == g_null_ip && g_aodv_gateway) //i'am  a gateway, routing to Internet!
			return NF_ACCEPT;
		else
			return NF_DROP;
	}
	else{
		if (tmp_route->state==REPLIED) {
			tmp_route->state = ACTIVE;
			if (g_routing_metric == WCIM)
				insert_timer_simple(TASK_UPDATE_LOAD, 1, g_mesh_ip);
		}
		tmp_route->lifetime = getcurrtime() + ACTIVE_ROUTE_TIMEOUT;
		
		return NF_ACCEPT;
	}
	
}
