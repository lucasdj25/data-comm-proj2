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
#include <net/if.h>
#include <errno.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <net/if_arp.h>
#include "reply.h"

using namespace std;


void createArpRequest(const int sockfd, ether_header &eh, iphdr &iph){
      char line[1500];  
       
      /* Ethernet header */
      struct ether_header EH2;
      struct ether_header *eh2 = &EH2;
       
      // Source Mac address
	// src is the router's mac addr
      memcpy(&eh2->ether_shost, eh.ether_dhost, 6); 
      // Target Mac address
      memcpy(&eh2->ether_dhost, ether_aton("ff:ff:ff:ff:ff:ff"), 6); 
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
      arph2->arp_op = htons(ARPOP_REQUEST);
       
      // Source Mac address
      memcpy(&arph2->arp_sha, eh.ether_dhost, 6); 
      // Target Mac address
      memcpy(&arph2->arp_tha, ether_aton("00:00:00:00:00:00"), 6); 
       
	struct in_addr src, dest;
	//src.s_addr = iph.saddr; 
	//dest.s_addr = iph.daddr;
	src.s_addr = inet_addr("10.1.1.1");
	dest.s_addr = inet_addr("10.1.1.5");
      // Source IP address
      memcpy(&arph2->arp_spa, &src, sizeof(uint32_t)); 
      // Target IP address 
      memcpy(&arph2->arp_tpa, &dest, sizeof(uint32_t)); 
	cout << "Src from icmp: " << inet_ntoa(src) << "\tdest from icmp: " << inet_ntoa(dest) << endl;

      // Puts arp header into packet
      memcpy(&line[14], arph2, sizeof(ether_arp));
       
      // Sends packet
      cout << "Sending Arp Request" << endl;
      int n = send(sockfd, line, 42 ,0);
      if(n == 42){
        cout << "Arp Request Sent" << endl;
      }   
}


struct interface {
    unsigned char macaddr[8];
    string name;
    int sock;
};

int main(int argc, char **argv){
	int packet_sockets[16];
	struct ifaddrs *ifaddr, *tmp;

	if(getifaddrs(&ifaddr)==-1){
		perror("getifaddrs");
		return 1;
	}

	int i = 0;

	// <interface ip, mac address>
	map<string,interface> macMap;

	// Puts all packet sockets into array
	for(tmp = ifaddr; tmp!=NULL; tmp=tmp->ifa_next){
		if(tmp->ifa_addr->sa_family==AF_PACKET){
		cout << "Interface: " << tmp->ifa_name << endl;
		string iName = string(tmp->ifa_name);
		struct sockaddr_ll *s = (struct sockaddr_ll*)tmp->ifa_addr;
/*
		// Mac map uses ip as the key and mac as the value
		if(iName.compare("lo") == 0)
			macMap.insert(make_pair("lo", s->sll_addr));
		else if(iName.compare("r1-eth0") == 0)
			macMap.insert(make_pair("10.0.0.1", s->sll_addr));
		else if(iName.compare("r1-eth1") == 0)
			macMap.insert(make_pair("10.1.0.1", s->sll_addr));
		else if(iName.compare("r1-eth2") == 0)
			macMap.insert(make_pair("10.1.1.1", s->sll_addr));
		else if(iName.compare("r2-eth0") == 0)
			macMap.insert(make_pair("10.0.0.2", s->sll_addr));
		else if(iName.compare("r2-eth1") == 0)
			macMap.insert(make_pair("10.3.0.1", s->sll_addr));
		else if(iName.compare("r2-eth2") == 0)
			macMap.insert(make_pair("10.3.1.1", s->sll_addr));
		else if(iName.compare("r2-eth3") == 0)
			macMap.insert(make_pair("10.3.4.1", s->sll_addr));
*/
			if(!strncmp(&(tmp->ifa_name[3]),"eth",3)){
				cout << "Creating Socket on interface " << tmp->ifa_name << endl;

				packet_sockets[i] = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
				cout << "socket: " << packet_sockets[i] << endl;
				if(packet_sockets[i]<0){
					perror("socket");
					return 2;
				}
				
				// Mac map uses ip as the key and mac as the value
                if(iName.compare("r1-eth0") == 0) {
                    struct interface eth0;
                    memcpy(&eth0.macaddr, s->sll_addr, 8); 
                    eth0.name = iName;
                    eth0.sock = packet_sockets[i];
                    macMap.insert(make_pair("10.0.0.1", eth0));
                }   
                else if(iName.compare("r1-eth1") == 0) {
                    struct interface eth1;
                    memcpy(&eth1.macaddr, s->sll_addr, 8); 
                    eth1.name = iName;
                    eth1.sock = packet_sockets[i];
                    macMap.insert(make_pair("10.1.0.1", eth1));
                }   
                else if(iName.compare("r1-eth2") == 0) {
                    struct interface eth2;
                    memcpy(&eth2.macaddr, s->sll_addr, 8); 
                    eth2.name = iName;
                    eth2.sock = packet_sockets[i];
                    macMap.insert(make_pair("10.1.1.1", eth2));
                }  
				if(bind(packet_sockets[i],tmp->ifa_addr,sizeof(struct sockaddr_ll))==-1){
					perror("bind");
				}

				i++;
			}
		}
	}

	// Prints mac address with their corresponding interfaces
/*
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
*/

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
         struct in_addr ipaddr1, ipaddr2;
         memcpy(&iph, line+14, 20);

		 ipaddr1.s_addr = iph.saddr;
		 ipaddr2.s_addr = iph.daddr;

         string sourceIP = inet_ntoa(ipaddr1);
		 string destIP = inet_ntoa(ipaddr2);
		cout << "Src ip from ip packet: " << sourceIP << endl;
		cout << "Dest ip from ip packet: " << destIP << endl;
		 // check if packet is for the router
		 if(macMap.count(destIP) == 0) {
			cout << "ICMP Packet not for router" << endl;
			string subDest = destIP.substr(0,6);
			string routerIP;
			for(const auto& k: macMap) {
				string subKey = k.first.substr(0,6);
				if(subDest.compare(subKey) == 0) {
					cout << "here" << endl;	
					routerIP = k.first;
				}
			}
			createArpRequest(macMap[routerIP].sock, eh, iph); 	
				// forward based on routing table
				// create arp req
				// send arp req
         }
//		 else
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
		  if(macMap.count(routerIP) > 0) {
			  // save source ip and mac

			  // send arp from router to host
		  }
          if(rIP.compare(routerIP) == 0 || rIP2.compare(routerIP) == 0) {
		createArpReply(eh, arph, packet_sockets[j], line, macMap[routerIP].macaddr);
          }
        }
      }
    }
  }
  freeifaddrs(ifaddr);
}

