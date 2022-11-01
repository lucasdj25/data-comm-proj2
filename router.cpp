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
#include <net/if.h>
#include <errno.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <arpa/inet.h>

#include "header.hpp"



using namespace std;


void createArpReply(ether_header *eh, arphdr *arph, int sockfd, char line[], char sourceMac[]){

  char newLine[5000];
  memcpy(&newLine, eh->ether_shost, 6);
  memcpy(&newLine+6, eh->ether_dhost, 6);

  struct ether_header eh2;
  eh2.ether_dhost = eh->ether_shost;
  eh2.ether_shost = eh->ether_dhost;
  eh2.ether_type = eh->ether_type;

  memcpy(&newLine, eh2, sizeof(ether_header));

  struct arphdr arph2;
  arhp2.ar_hrd = arph.ar_hrd;
  arph2.ar_pro = arph.ar_pro;
  arph2.ar_hln = arph.ar_hln;
  arph2.ar_pln = arph.ar_pln;
  arph2.ar_op = 2;

  arph2.__ar_tha[ETH_ALEN] = arph.__ar_sha[ETH_ALEN];
  arph2.__ar_tip[4] = arph.__ar_sip[4];
  arph2.__ar_sip[4] = arph.__ar_tip[4];
  arph2.__ar_sha[ETH_ALEN] = sourceMac;

  memcpy(&newLine+14, arph2, sizeof(arphdr));


  int n = sendto(sockfd, );
}

void createICMPReply(ether_header *eh, iphdr *iphdrph, int sockfd){
  
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
          memcpy(&eh,line,14);
          struct iphdr iph;
          struct ICMPHeader icmph;
          memcpy(&iph,line+14,20);

          // ICMP echo reply consists of the ethernet header, ip header, and icmp header
          createIcmpReply(eh, iph, icmph);
        }
        // arp type is 0x0806
        if(ntohs(eh.ether_type) == ETHERTYPE_ARP) {

          struct arphdr arph;
          memcpy(&arph, line+14, 28);
          // ARP reply consists of the ethernet header and ARP header
          createArpReply(eh, arph, packet_sockets[j], line);
        }
		  }
    }
	}
  freeifaddrs(ifaddr);
}
