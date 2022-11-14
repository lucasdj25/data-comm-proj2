#include <iostream>			  // cout
#include <cstring>			  // memcpy
#include <net/ethernet.h>	// ether header
#include <net/if_arp.h>		// arp header
#include <netinet/ip.h>		// ip header
#include <sys/socket.h>		// send
#include <sys/types.h>		// uint8...
#include <arpa/inet.h>		// htons
#include "reply.h" 

void createArpReply(ether_header &eh, ether_arp &arph, int sockfd, char *line, unsigned char *sourceMac){
  /* Ethernet header */
  struct ether_header EH2;
  struct ether_header *eh2 = &EH2;

  // Source Mac address
  memcpy(&eh2->ether_shost, sourceMac, 6);
  // Target Mac address
  memcpy(&eh2->ether_dhost, &eh.ether_shost, 6);
  // Ethernet type
  eh2->ether_type = htons(ETHERTYPE_ARP);

  // Puts ether header into packet
  memcpy(&line[0], eh2, sizeof(ether_header));

  /* Arp header */
  struct ether_arp ARPH2;
  struct ether_arp *arph2 = &ARPH2;

  // Fixed header
  arph2->arp_hrd = htons(ARPHRD_ETHER);
  arph2->arp_pro = htons(ETHERTYPE_IP);
  arph2->arp_hln = ETH_ALEN;
  arph2->arp_pln = 4;
  arph2->arp_op = htons(ARPOP_REPLY);

  // Source Mac address
  memcpy(&arph2->arp_sha, sourceMac, 6);
  // Target Mac address
  memcpy(&arph2->arp_tha, &arph.arp_sha, 6);

  // Source IP address
  memcpy(&arph2->arp_spa, &arph.arp_tpa, 4);
  // Target IP address
  memcpy(&arph2->arp_tpa, &arph.arp_spa, 4);

  // Puts arp header into packet
  memcpy(&line[14], arph2, sizeof(ether_arp));

  // Sends packet
  std::cout << "Sending Arp Reply" << std::endl;
  int n = send(sockfd, line, 42 ,0);
  if(n == 42){
    std::cout << "Arp Reply Sent" << std::endl;
  }
}

void sendICMPTimeExceeded(int sockfd, iphdr &ih){
  uint8_t type = 11; // indicates time exceeded message
  uint8_t code = 0;  // 0 is the time exceeded type

  char icmp_response[1500];

  memcpy(&icmp_response[0], &ih, sizeof(iphdr));

  struct icmp_header icmph;
  icmph.type = type;
  icmph.code = code;
  icmph.checksum = ih.check;
  
  memcpy(&icmp_response[20], &icmph, 48);

  size_t send_size = sizeof(icmp_header) + sizeof(iphdr);
  int n = send(sockfd, icmp_response, send_size, 0);


}

// pass in -1 for code if sending a echo reply
void createICMPReply(ether_header &eh, iphdr &ih, int sockfd, uint8_t type, uint8_t code, char *line){

  // Drop packet if wrong checksum
  uint16_t recvChecksum = checkSum(&ih, sizeof(iphdr));
  if(recvChecksum != 0) {
	  std::cout << "Checksum is incorrect, packet is being dropped" << std::endl;
	  return;
  }

  char line2[1500];
  // Ethernet header
  struct ether_header EH2;
  struct ether_header *eh2 = &EH2;
  memcpy(&eh2->ether_shost, &eh.ether_dhost, ETH_ALEN);
  memcpy(&eh2->ether_dhost, &eh.ether_shost, ETH_ALEN);
  eh2->ether_type = htons(ETHERTYPE_IP);

  memcpy(&line2[0], eh2, sizeof(ether_header));

  // IP header
  struct iphdr ih2;
  memcpy(&ih2, &line[14], sizeof(iphdr));
  memcpy(&ih2.saddr, &ih.daddr, sizeof(uint32_t));
  memcpy(&ih2.daddr, &ih.saddr, sizeof(uint32_t));
  
  ih2.ttl = 64;
  ih2.check = 0;
  // Recalculate checksum b/c ttl changed
  ih2.check = checkSum(&ih2, sizeof(iphdr));

  
  memcpy(&line2[14], &ih2, sizeof(iphdr));

  struct icmp_header icmph;
  memcpy(&icmph, &line[34], 6);
  // ICMP type, 0 is reply
  icmph.type = type;
  if(code != -1){
  icmph.code = code;
  }

  // ICMP checksum, start with 0 b/c a new checksum needs to be calculated
  icmph.checksum = (0<<0);

  int dataStart = sizeof(ether_header) + sizeof(iphdr) + sizeof(icmp_header);

  // length of data is in ip header
  // the length includes the ip header, icmp header, and the data
  // so subtract the lengths of headers to get the size of the data
  int dataLen = htons(ih2.tot_len) - sizeof(iphdr) - sizeof(icmp_header);
  char data[1500];
  memcpy(&data, &line[dataStart], dataLen);

  char data_for_checksum[1500];

  // put ICMP header into array
  memcpy(&data_for_checksum[0], &icmph, sizeof(struct icmp_header));

  // put data into array
  memcpy(&data_for_checksum[sizeof(icmp_header)], &data, dataLen);

  // checksum is calculate using the bytes from the icmp header AND data
  uint16_t newChecksum = checkSum(data_for_checksum, dataLen+sizeof(icmp_header));

  // copy the new checksum into the icmp header
  icmph.checksum = newChecksum;

  // ICMP sequence number
  memcpy(&line2[34], &icmph, sizeof(icmph));
  memcpy(&line2[40], data, dataLen);
  // Sends packet
  std::cout << "Sending ICMP Reply" << std::endl;
  int n = send(sockfd, line2, sizeof(ether_header)+htons(ih2.tot_len),0);
  std::cout << "ICMP Reply Sent" << std::endl;
}


uint16_t checkSum(const void* data, size_t len){
    auto p = reinterpret_cast<const uint16_t*>(data);
    uint32_t sum = 0;

    // if the length is odd
    if (len & 1){
        sum = reinterpret_cast<const uint8_t*>(p)[len - 1];
    }
    len /= 2;

    while (len--){
        sum += *p++;
        if (sum & 0xffff0000){
            sum = (sum >> 16) + (sum & 0xffff);
        }
    }
    return static_cast<uint16_t>(~sum);
}


