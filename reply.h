#ifndef REPLY_H
#define REPLY_H

#include <netinet/ip.h>
#include <net/ethernet.h>
#include <netinet/ether.h>
#include <net/if_arp.h>
#include <sys/types.h>

struct icmp_header {
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    uint8_t identifier;
    uint8_t seq;
};


void createArpReply(ether_header &eh, ether_arp &arph, int, char*, unsigned char*); 
void createICMPReply(ether_header &eh, iphdr &ih, int, char*);
uint16_t checkSum(const void*, size_t);

#endif
