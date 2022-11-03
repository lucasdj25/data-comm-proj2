#include <iostream>
#include <map>
#include <iomanip>
#include <sstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ether.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <cstring>
#include <unistd.h>
#include <sys/select.h>
#include <netpacket/packet.h> 
#include <net/ethernet.h>
#include <netinet/ether.h>
#include <net/if.h>
#include <errno.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <net/if_arp.h>

using namespace std;


void createArpReply(ether_header &eh, ether_arp &arph, const int sockfd, char *line, unsigned char *sourceMac){

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
  cout << "Sending Arp Reply" << endl;
  int n = send(sockfd, line, 42 ,0);
  if(n == 42){
  	cout << "Arp Reply Sent" << endl;
  }
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

struct icmp_header {
	uint8_t type;
	uint8_t code;
	uint16_t checksum;
	uint8_t identifier;
	uint8_t seq;
};

void createICMPReply(ether_header &eh, iphdr &ih, int sockfd, char *line){
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
  
  struct icmp_header icmph;
  memcpy(&icmph, &line[34], 6);
  // ICMP type, 0 is reply
  icmph.type = (1<<0);
  icmph.checksum = (1<<0);
  
  // ICMP checksum, start with 0 b/c a new checksum needs to be calculated 
  //memcpy(&icmph->checksum, &start, 2);
  
  char data[1500];
  int dataStart = sizeof(ether_header) + sizeof(iphdr) + sizeof(icmp_header);
  
  // length of data is in ip header
  // the length includes the ip header, icmp header, and the data
  // so subtract the lengths of headers to get the size of the data
  int dataEnd = htons(ih2.tot_len) - sizeof(iphdr) - sizeof(icmp_header); 
  memcpy(&data, &line[dataStart], dataEnd);

  char data_for_checksum[dataEnd];
  
  // put ICMP header into array
  memcpy(&data_for_checksum[0], &icmph, sizeof(struct icmp_header));
  
  // put data into array
  memcpy(&data_for_checksum[sizeof(icmp_header)], &data, sizeof(data));
  
  // checksum is calculate using the bytes from the icmp header AND data
  uint16_t newChecksum = checkSum(data_for_checksum, dataEnd);
  
  // copy the new checksum into the icmp header
  icmph.checksum = newChecksum;

  // ICMP sequence number 
  memcpy(&line2[34], &icmph, sizeof(icmph));
  memcpy(&line2[40], data, dataEnd);
  // Sends packet
  cout << "Sending ICMP Reply" << endl;
  int n = send(sockfd, line2, 1500,0);
  cout << "ICMP Reply Sent" << endl;
}

int main(int argc, char **argv){
	int packet_sockets[16];
	struct ifaddrs *ifaddr, *tmp;

	if(getifaddrs(&ifaddr)==-1){
		perror("getifaddrs");
		return 1;
	}

	int i = 0;

	// <interface ip, mac address>
	map<string,unsigned char*> macMap;

	// Puts all packet sockets into array
	for(tmp = ifaddr; tmp!=NULL; tmp=tmp->ifa_next){
		if(tmp->ifa_addr->sa_family==AF_PACKET){
		cout << "Interface: " << tmp->ifa_name << endl;
		string iName = string(tmp->ifa_name);
		struct sockaddr_ll *s = (struct sockaddr_ll*)tmp->ifa_addr;
		// Mac map uses ip as the key and mac as the value
		if(iName.compare("lo") == 0)
			macMap.insert(make_pair("lo", s->sll_addr));
		else if(iName.compare("r1-eth0") == 0)
			macMap.insert(make_pair("10.0.0.1", s->sll_addr));
		else if(iName.compare("r1-eth1") == 0)
			macMap.insert(make_pair("10.1.0.1", s->sll_addr));
		else if(iName.compare("r1-eth2") == 0)
			macMap.insert(make_pair("10.1.1.1", s->sll_addr));

			if(!strncmp(&(tmp->ifa_name[3]),"eth",3)){
				cout << "Creating Socket on interface " << tmp->ifa_name << endl;

				packet_sockets[i] = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
				if(packet_sockets[i]<0){
					perror("socket");
					return 2;
				}

				if(bind(packet_sockets[i],tmp->ifa_addr,sizeof(struct sockaddr_ll))==-1){
					perror("bind");
				}

				i++;
			}
		}
	}

	// Prints mac address with their corresponding interfaces
	cout << "Mac Address Map" << endl;
	for(const auto& n : macMap) {
		unsigned char macAddr[6];
		memcpy(&macAddr, n.second, 6);
		stringstream ss;
		for(unsigned char c : macAddr)
			ss << setw(2) << setprecision(2) << setfill('0') << hex << (unsigned)c << " ";
		string sMac = ss.str();
		cout << "Mac addr on interface " << n.first << " is: " << sMac << endl;
	}


	// Creates a fd_set with all the sockets
	fd_set fds;

	FD_ZERO(&fds);

	for(int j=0;j<i;j++){
		FD_SET(packet_sockets[j], &fds);
	}

  // Listens for packets on all sockets
	while(1){
		fd_set tmp = fds;
		int nn=select(FD_SETSIZE, &tmp, NULL, NULL, NULL);
    for(int j=0; j<i;j++){
      if(FD_ISSET(packet_sockets[j],&tmp)){
        cout << "Got something on the socket" << endl;

        char line[5000];
        struct sockaddr_ll recvaddr;
        unsigned int recvaddrlen=sizeof(struct sockaddr_ll);
  
        int n = recvfrom(packet_sockets[j], line, 5000,0,
          (struct sockaddr*)&recvaddr, &recvaddrlen);

        if(recvaddr.sll_pkttype==PACKET_OUTGOING)
          continue;

        struct ether_header eh;
        memcpy(&eh, line, 14);

        // ip type is 0x0800
        if(ntohs(eh.ether_type) == ETHERTYPE_IP) {
         cout << "Received an ICMP packet" << endl;
         
         struct iphdr iph;
         memcpy(&iph, line+14, 20);
         createICMPReply(eh, iph, packet_sockets[j], line);
        
        }
        // arp type is 0x0806
        if(ntohs(eh.ether_type) == ETHERTYPE_ARP) {
          cout << "Received an ARP packet" << endl;
          
          struct ether_arp arph;
          struct in_addr ipaddr;
          memcpy(&arph, line+14, 28);
          memcpy(&ipaddr.s_addr, arph.arp_tpa, 4);
          
          string routerIP = inet_ntoa(ipaddr);
          string rIP = "10.1.0.1";
          string rIP2 = "10.1.1.1";
          if(rIP.compare(routerIP) == 0 || rIP2.compare(routerIP) == 0) {
		createArpReply(eh, arph, packet_sockets[j], line, macMap[routerIP]);
          }
        }
      }
    }
  }
  freeifaddrs(ifaddr);
}
