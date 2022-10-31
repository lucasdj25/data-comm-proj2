#include <iostream>
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


using namespace std;

/* struct fd_set {
	int fd_count;
	socket fd_array[];
}
*/

int main(int argc, char **argv){
	int packet_sockets[16];
	struct ifaddrs *ifaddr, *tmp;

	if(getifaddrs(&ifaddr)==-1){
		perror("getifaddrs");
		return 1;
	}

	int i = 0;

	// Puts all packet sockets into array
	for(tmp = ifaddr; tmp!=NULL; tmp=tmp->ifa_next){
		if(tmp->ifa_addr->sa_family==AF_PACKET){
		cout << "Interface: " << tmp->ifa_name << endl;

			if(!strncmp(&(tmp->ifa_name[3]),"eth",3)){
				cout << "Creating Socket on interface " << tmp->ifa_name << endl;

				packet_sockets[i] = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
				if(packet_socket[i]<0){
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
          memcpy(&arph, line+14, 20);
          // ARP reply consists of the ethernet header and ARP header
          createArpReply(eh, arph);
        }
		  }
    }
	}
}
