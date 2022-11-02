#include <iostream>
#include <map>
#include <iomanip>
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

//#include "header.hpp"

using namespace std;


void createArpReply(ether_header &eh, ether_arp &arph, const int sockfd, char *line, unsigned char *sourceMac, sockaddr_ll recvaddr){

  /* Ethernet header */
  struct ether_header *eh2;

//  struct ether_addr *src_mac = ether_aton(sourceMac);
 
  // Source Mac address
  memcpy(&eh2->ether_shost, sourceMac, 6);
  // Target Mac address
  memcpy(&eh2->ether_dhost, &eh.ether_shost, 6);
  // Ethernet type
  eh2->ether_type = htons(ETHERTYPE_ARP);

  // Puts header into packet
  memcpy(&line[0], eh2, sizeof(ether_header));

  /* Arp header */
  struct ether_arp *arph2;
	
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

  // Puts header into packet
  memcpy(&line[14], arph2, sizeof(ether_arp));
	
  // Sends packet
  cout << "Sending Arp Reply" << endl;
  int n = send(sockfd, line, 42 ,0);
  cout << n << endl;
  cout << "Arp Reply Sent" << endl;
}

void createICMPReply(ether_header *eh, iphdr *ih, int sockfd, char *line){

  // Ethernet header
  struct ether_header *eh2;
  memcpy(&eh2->ether_shost, &eh->ether_dhost, ETH_ALEN);
  memcpy(&eh2->ether_dhost, &eh->ether_shost, ETH_ALEN);
  eh->ether_type = htons(ETHERTYPE_IP);
  
  memcpy(&line, eh2, sizeof(ether_header));
  
  // IP header
  struct iphdr *ih2;
 	
}

int main(int argc, char **argv){
	int packet_sockets[16];
	struct ifaddrs *ifaddr, *tmp;

	if(getifaddrs(&ifaddr)==-1){
		perror("getifaddrs");
		return 1;
	}

	int i = 0;

	// <interface, mac address>
	map<char*,unsigned char*> macMap;

	// Puts all packet sockets into array
	for(tmp = ifaddr; tmp!=NULL; tmp=tmp->ifa_next){
		if(tmp->ifa_addr->sa_family==AF_PACKET){
		cout << "Interface: " << tmp->ifa_name << endl;

		cout << "Mac Address: "; 
		struct sockaddr_ll *s = (struct sockaddr_ll*)tmp->ifa_addr;
		for(int k = 0; k < 6; k++) {
			printf("%02x%c",(s->sll_addr[k]));
		}
		cout << endl << endl;
		macMap.insert(make_pair(tmp->ifa_name, s->sll_addr));

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

        // ip type is 0x0800, for pings
        if(ntohs(eh.ether_type) == ETHERTYPE_IP) {
        
        
        }
        // arp type is 0x0806
        if(ntohs(eh.ether_type) == ETHERTYPE_ARP) {
          cout << "Recieved an ARP packet" << endl;
          
          struct ether_arp arph;
          struct in_addr ipaddr;
          memcpy(&arph, line+14, 28);
          memcpy(&ipaddr.s_addr, arph.arp_tpa, 4);
          
          cout << "dest: " << inet_ntoa(ipaddr) << endl;
          string routerIP = inet_ntoa(ipaddr);
          string rIP = "10.1.0.1";
          string rIP2 = "10.1.1.1";
          if(rIP.compare(routerIP) == 0 || rIP2.compare(routerIP) == 0) {
                cout << "router ip is 10.1.0.1" << endl;
                char interf[] = {'r','1','-','e','t','h','1','\0'};
                char interf2[] = {'r','1','-','e','t','h','2','\0'};
                for(const auto& n : macMap) {
                        unsigned char macAddr[6];
                        memcpy(&macAddr, n.second, 6);
                        if(strcmp(interf,n.first) == 0) {
                          createArpReply(eh, arph, packet_sockets[j], line, macAddr, recvaddr);
                        }
                        else if(strcmp(interf2,n.first) == 0) {
                          cout << (void *) line << endl;
                          createArpReply(eh, arph, packet_sockets[j], line, macAddr, recvaddr);
                        }

                }
          }
        }
      }
    }
  }
  freeifaddrs(ifaddr);
}