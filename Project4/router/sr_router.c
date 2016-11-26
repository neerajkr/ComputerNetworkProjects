/**********************************************************************
 * file:  sr_router.c
 * date:  Mon Feb 18 12:50:42 PST 2002
 * Contact: casado@stanford.edu
 *
 * Description:
 *
 * This file contains all the functions that interact directly
 * with the routing table, as well as the main entry method
 * for routing.
 *
 **********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>

#include "sr_if.h"
#include "sr_rt.h"
#include "sr_router.h"
#include "sr_protocol.h"
#include "sr_arpcache.h"
#include "sr_utils.h"

/* TODO: Add constant definitions here... */

#define MIN_IP_HDR_LEN 20     
#define MAX_IP_HDR_LEN 60   
#define DEFAULT_TTL 64    
#define ICMP_IP_HDR_LEN 5   
#define ICMP_IP_HDR_LEN_BYTES ICMP_IP_HDR_LEN * 4   
#define ICMP_COPIED_DATAGRAM_DATA_LEN 8   

#define ICMP_ECHO_REQUEST_CODE 8    
#define ICMP_ECHO_REPLY_CODE 0    
#define ICMP_UNREACHABLE_TYPE 3   
#define ICMP_HOST_CODE 1    
#define ICMP_NET_CODE 0   
#define ICMP_PORT_CODE 3    
#define ICMP_TIME_EXCEEDED_TYPE 11

/* TODO: Add helper functions here... */



void HandleArpRequst(struct sr_instance* sr,
                         struct sr_arp_hdr *arp_hdr,
                         struct sr_if*);

void ForwardToNextHop(struct sr_instance* sr, struct sr_ip_hdr *ip_hdr);

void sr_send_echo_reply(struct sr_instance* sr,
        uint8_t * packet/* lent */,
        unsigned int len,
        char* interface/* lent */);



/* See pseudo-code in sr_arpcache.h */
void handle_arpreq(struct sr_instance* sr, struct sr_arpreq *req){
  /* TODO: Fill this in */
  
}

/*---------------------------------------------------------------------
 * Method: sr_init(void)
 * Scope:  Global
 *
 * Initialize the routing subsystem
 *
 *---------------------------------------------------------------------*/

void sr_init(struct sr_instance* sr)
{
    /* REQUIRES */
    assert(sr);

    /* Initialize cache and cache cleanup thread */
    sr_arpcache_init(&(sr->cache));

    pthread_attr_init(&(sr->attr));
    pthread_attr_setdetachstate(&(sr->attr), PTHREAD_CREATE_JOINABLE);
    pthread_attr_setscope(&(sr->attr), PTHREAD_SCOPE_SYSTEM);
    pthread_attr_setscope(&(sr->attr), PTHREAD_SCOPE_SYSTEM);
    pthread_t thread;

    pthread_create(&thread, &(sr->attr), sr_arpcache_timeout, sr);
    
    /* TODO: (opt) Add initialization code here */

} /* -- sr_init -- */

/*---------------------------------------------------------------------
 * Method: sr_handlepacket(uint8_t* p,char* interface)
 * Scope:  Global
 *
 * This method is called each time the router receives a packet on the
 * interface.  The packet buffer, the packet length and the receiving
 * interface are passed in as parameters. The packet is complete with
 * ethernet headers.
 *
 * Note: Both the packet buffer and the character's memory are handled
 * by sr_vns_comm.c that means do NOT free either (signified by "lent" comment).  
 * Make a copy of the
 * packet instead if you intend to keep it around beyond the scope of
 * the method call.
 *
 *---------------------------------------------------------------------*/

void sr_handlepacket(struct sr_instance* sr,
        uint8_t * packet/* lent */,
        unsigned int len,
        char* interface/* lent */)
{

  /* REQUIRES */
  assert(sr);
  assert(packet);
  assert(interface);

  printf("*** -> Received packet of length %d \n",len);
  if (len < sizeof(struct sr_ethernet_hdr))     return;

  print_hdrs(packet,len);


  /* Handle ARP packet */
  if (ethertype(packet) == ethertype_arp) {

    /* print_hdr_arp(packet+sizeof(sr_ethernet_hdr_t));*/

    struct sr_arpentry *arp_entry;
    struct sr_arpreq *arp_req;
    struct sr_arp_hdr *arp_hdr;
    struct sr_if* rec_if;
    
    if (len < sizeof(struct sr_ethernet_hdr) + sizeof(struct sr_arp_hdr))   return 0;
    
    rec_if = sr_get_interface(sr, interface);
    arp_hdr = arp_header(packet);

    /* Its a either broadcasted msg or reply to arp request msg, if destination is not the router interface then return*/
    if (rec_if->ip != arp_hdr->ar_tip)          return;
    arp_entry = sr_arpcache_lookup(&sr->cache, arp_hdr->ar_sip);
    if (arp_entry != 0) {
      free(arp_entry);
    } 

    else {
      arp_req = sr_arpcache_insert(&sr->cache, arp_hdr->ar_sha, arp_hdr->ar_sip);
      if (arp_req != 0) {
        sr_arpreq_send_packets(sr, arp_req);
      }
    }
    if (arp_opcode(arp_hdr) == arp_op_request) {
      HandleArpRequst(sr, arp_hdr, rec_if);
    }

  } 


  /* Handle IP packet*/

  else {
    /* print_hdr_ip(packet+sizeof(sr_ethernet_hdr_t));*/
    struct sr_ip_hdr *ip_hdr;
    uint16_t expected_cksum;
    uint16_t received_cksum;
    if (len < sizeof(struct sr_ip_hdr) + sizeof(struct sr_ethernet_hdr))  return ;    
    ip_hdr = ip_header(packet);
    if (len < sizeof(struct sr_ethernet_hdr) + ip_ihl(ip_hdr))   return;
    received_cksum = ip_hdr->ip_sum;
    ip_hdr->ip_sum = 0;
    expected_cksum = cksum(ip_hdr, ip_ihl(ip_hdr));

    printf("IP packet :-received_cksum: %d  expected_cksum: %d\n", received_cksum,expected_cksum);


    if (expected_cksum != received_cksum)     return ;


    if (sr_interface_ip_match(sr, ip_hdr->ip_dst)) {  

      if (ip_hdr->ip_p == ip_protocol_icmp) {
        printf("========================= ICMP  ECHO reply=============\n");

        sr_send_echo_reply(sr,   packet,   len, interface);
      } 

      else {
        printf("========================= TCP/UDP: ICMP unreachable=============\n");

       
        sr_send_port_unreachable(sr,   packet,   len, interface); 
        
      }
    

    } 

    else {

      printf("============================Forwarding IP packet==============\n");
      ForwardToNextHop(sr, ip_hdr);
    }
  }

}/* end sr_ForwardPacket */

void FillAndSendPkt(struct sr_instance* sr,
                        uint8_t *packet, 
                        unsigned int len, 
                        uint32_t dip,
                        int send_icmp,
                        enum sr_ethertype type)
{
  struct sr_arpentry *arp_entry;
  struct sr_arpreq *arp_req;
  struct sr_ethernet_hdr eth_hdr;
  uint8_t *eth_pkt;
  struct sr_if *interface;
  struct sr_rt *rt;
  unsigned int eth_pkt_len;
  
  rt = sr_longest_prefix_match(sr, ip_in_addr(dip));
  
  interface = sr_get_interface(sr, rt->interface);
  
  arp_entry = sr_arpcache_lookup(&sr->cache, rt->gw.s_addr);
  if(arp_entry) printf("ARP entry already exists, preparing to send the msg now\n");

  if (arp_entry || type == ethertype_arp) {
    
    /* Create the ethernet packet. */
    eth_pkt_len = len + sizeof(eth_hdr);
    printf("eth_pkt_len: %d\n",eth_pkt_len);
    printf("len: %d\n", len);

    eth_hdr.ether_type = htons(type);
    
    if (type == ethertype_arp && ((struct sr_arp_hdr *)packet)->ar_op == htons(arp_op_request)){
      printf("Destination is broadcast because its a arp request\n");
      memset(eth_hdr.ether_dhost, 255, ETHER_ADDR_LEN);
    }
    
    else
      memcpy(eth_hdr.ether_dhost, arp_entry->mac, ETHER_ADDR_LEN);
      memcpy(eth_hdr.ether_shost, interface->addr, ETHER_ADDR_LEN);
      eth_pkt = malloc(eth_pkt_len);
      memcpy(eth_pkt, &eth_hdr, sizeof(eth_hdr));
      memcpy(eth_pkt + sizeof(eth_hdr), packet, len);
      sr_send_packet(sr, eth_pkt, eth_pkt_len, rt->interface);
      printf("packet sent of size: %d\n", eth_pkt_len);
      free(eth_pkt);
      if (arp_entry)
        free(arp_entry);
  
  } 

  else {

    printf("Adding to ARP request queue\n");
    eth_pkt = malloc(len);
    memcpy(eth_pkt, packet, len);
    arp_req = sr_arpcache_queuereq(&sr->cache, rt->gw.s_addr, eth_pkt, len, rt->interface);
    sr_arpreq_handle(sr, arp_req);
    free(eth_pkt);
  }
}


void HandleArpRequst(struct sr_instance* sr,
                         struct sr_arp_hdr *arp_hdr,
                         struct sr_if* interface)
{
  struct sr_arp_hdr reply_arp_hdr;
  
  reply_arp_hdr.ar_hrd = htons(arp_hrd_ethernet);
  reply_arp_hdr.ar_pro = htons(arp_pro_ip);
  reply_arp_hdr.ar_hln = ETHER_ADDR_LEN;
  reply_arp_hdr.ar_pln = sizeof(uint32_t);
  reply_arp_hdr.ar_op = htons(arp_op_reply);
  reply_arp_hdr.ar_sip = interface->ip;
  reply_arp_hdr.ar_tip = arp_hdr->ar_sip;
  memcpy(reply_arp_hdr.ar_tha, arp_hdr->ar_sha, ETHER_ADDR_LEN);
  memcpy(reply_arp_hdr.ar_sha, interface->addr, ETHER_ADDR_LEN);

  FillAndSendPkt(sr, 
                     (uint8_t *)&reply_arp_hdr, 
                     sizeof(struct sr_arp_hdr), 
                     arp_hdr->ar_sip,
                     1,
                     ethertype_arp);
}





void ForwardToNextHop(struct sr_instance* sr, struct sr_ip_hdr *ip_hdr)
{
  uint8_t *fwd_ip_pkt;
  unsigned int len;

  printf("Received TTL is %d=====================\n", ip_hdr->ip_ttl);

  ip_hdr->ip_ttl--;

  len = ip_len(ip_hdr);

  ip_hdr->ip_sum = 0;
  ip_hdr->ip_sum = cksum(ip_hdr, ip_ihl(ip_hdr));
  
  fwd_ip_pkt = malloc(len);
  memcpy(fwd_ip_pkt, ip_hdr, len);
  FillAndSendPkt(sr, fwd_ip_pkt, len, ip_hdr->ip_dst, 1, ethertype_ip);
  free(fwd_ip_pkt);
}


void sr_send_echo_reply(struct sr_instance* sr,
        uint8_t * packet/* lent */,
        unsigned int len,
        char* interface/* lent */){


  sr_ethernet_hdr_t *SendingEthernet, *ReceivedEthernet;
  sr_ip_hdr_t *SendingIP, *ReceivedIP;
  sr_icmp_hdr_t *sd_icmp_header;

  ReceivedEthernet = (sr_ethernet_hdr_t *)packet;
  ReceivedIP = (sr_ip_hdr_t *)(packet + sizeof(sr_ethernet_hdr_t));

  uint8_t *packetToBeSent = (uint8_t *)malloc(len);
  SendingEthernet = (sr_ethernet_hdr_t *)packetToBeSent;
  SendingIP = (sr_ip_hdr_t *)(packetToBeSent + sizeof(sr_ethernet_hdr_t));
  sd_icmp_header = (sr_icmp_hdr_t *)(packetToBeSent + sizeof(sr_ethernet_hdr_t) + sizeof(sr_ip_hdr_t));


  sd_icmp_header->icmp_type = ICMP_ECHO_REPLY_CODE;     
  sd_icmp_header->icmp_code = 0;
  sd_icmp_header->icmp_sum = 0;
  sd_icmp_header->icmp_sum = cksum(sd_icmp_header, sizeof(sr_icmp_hdr_t));


  SendingIP->ip_hl = ReceivedIP->ip_hl;
  SendingIP->ip_v  = ReceivedIP->ip_v;
  SendingIP->ip_tos = ReceivedIP->ip_tos;
  SendingIP->ip_len = ReceivedIP->ip_len;
  SendingIP->ip_id = ReceivedIP->ip_id;
  SendingIP->ip_off = ReceivedIP->ip_off;
  SendingIP->ip_p = ReceivedIP->ip_p;
  SendingIP->ip_ttl = 63;
  SendingIP->ip_src = ReceivedIP->ip_dst;
  SendingIP->ip_dst = ReceivedIP->ip_src;

  SendingIP->ip_sum = 0;
  SendingIP->ip_sum = cksum(SendingIP, sizeof(sr_ip_hdr_t));


  memcpy(SendingEthernet->ether_dhost, ReceivedEthernet->ether_shost, ETHER_ADDR_LEN);
  memcpy(SendingEthernet->ether_shost, ReceivedEthernet->ether_dhost, ETHER_ADDR_LEN);
  SendingEthernet->ether_type  = ReceivedEthernet->ether_type;

  unsigned int TotalHeaderLength = sizeof(sr_ethernet_hdr_t)+sizeof(sr_ip_hdr_t)+sizeof(sr_icmp_hdr_t);

  memcpy(packetToBeSent+TotalHeaderLength, packet+TotalHeaderLength, len-TotalHeaderLength);

  sr_send_packet(sr, packetToBeSent, len, interface);
}



void sr_send_port_unreachable(struct sr_instance* sr,
        uint8_t * packet/* lent */,
        unsigned int len,
        char* interface/* lent */){

  sr_ethernet_hdr_t *SendingEthernet, *ReceivedEthernet;
  sr_ip_hdr_t *SendingIP, *ReceivedIP;
  sr_icmp_t3_hdr_t *SendingicmpT3;

  ReceivedEthernet = (sr_ethernet_hdr_t *)packet;
  ReceivedIP = (sr_ip_hdr_t *)(packet + sizeof(sr_ethernet_hdr_t));


  unsigned int TotalHeaderLength = sizeof(sr_ethernet_hdr_t)+sizeof(sr_ip_hdr_t)+sizeof(sr_icmp_t3_hdr_t);


  uint8_t *packetToBeSent = (uint8_t *)malloc(TotalHeaderLength);

  SendingEthernet = (sr_ethernet_hdr_t *)packetToBeSent;
  SendingIP = (sr_ip_hdr_t *)(packetToBeSent + sizeof(sr_ethernet_hdr_t));
  SendingicmpT3 = (sr_icmp_t3_hdr_t *)(packetToBeSent + sizeof(sr_ethernet_hdr_t) + sizeof(sr_ip_hdr_t));

  SendingicmpT3->icmp_type = ICMP_UNREACHABLE_TYPE;     
  SendingicmpT3->icmp_code = 3;

  struct sr_if* Interface=sr_get_interface(sr,interface);

  int i;
  for(i=0; i<28; i++)
  SendingicmpT3->data[i] = packet[i+sizeof(sr_ethernet_hdr_t)];

  SendingicmpT3->icmp_sum  = cksum(SendingicmpT3, sizeof(sr_icmp_t3_hdr_t));

  SendingIP->ip_hl = ReceivedIP->ip_hl;
  SendingIP->ip_src = Interface->ip;
  SendingIP->ip_dst = ReceivedIP->ip_src;
  SendingIP->ip_v  = ReceivedIP->ip_v;
  SendingIP->ip_tos = ReceivedIP->ip_tos;
  SendingIP->ip_len = ntohs(sizeof(sr_ip_hdr_t)+sizeof(sr_icmp_t3_hdr_t));
  SendingIP->ip_id = ReceivedIP->ip_id;
  SendingIP->ip_off = ReceivedIP->ip_off;
  SendingIP->ip_p = ip_protocol_icmp; 
  
  SendingIP->ip_ttl = 63;
  SendingIP->ip_sum = cksum(SendingIP, sizeof(sr_ip_hdr_t));

  /* creating ethernet header */
  memcpy(SendingEthernet->ether_dhost, ReceivedEthernet->ether_shost, ETHER_ADDR_LEN);
  memcpy(SendingEthernet->ether_shost, Interface->addr, ETHER_ADDR_LEN);
  SendingEthernet->ether_type  = ReceivedEthernet->ether_type;


  printf("%d\n", sizeof(sr_ethernet_hdr_t));
  printf("%d\n", sizeof(sr_ip_hdr_t));
  printf("%d\n", sizeof(sr_icmp_t3_hdr_t));
  sr_send_packet(sr, packetToBeSent, TotalHeaderLength, interface);
  print_hdrs(packetToBeSent,TotalHeaderLength-sizeof(sr_icmp_t3_hdr_t)+sizeof(sr_icmp_hdr_t));
  printf("Packet of %d length sent of type ICMP_UNREACHABLE_TYPE\n",TotalHeaderLength );
}


