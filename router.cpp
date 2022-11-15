#include <iostream>
#include <map>
#include <iomanip>
#include <sstream>
#include <fstream>
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
#include "routingtable.h"

using namespace std;


void createArpRequest(const int sockfd, ether_header &eh, iphdr &iph, string routerIP){
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
	  // src ip is the router's ip of the interface that the dest ip is on
	  src.s_addr = inet_addr(routerIP.c_str()); 
	  dest.s_addr = iph.daddr;

      // Source IP address
      memcpy(&arph2->arp_spa, &src, sizeof(uint32_t)); 
      // Target IP address 
      memcpy(&arph2->arp_tpa, &dest, sizeof(uint32_t)); 

      // Puts arp header into packet
      memcpy(&line[14], arph2, sizeof(ether_arp));
       
      // Sends packet
      send(sockfd, line, 42 ,0);
}

struct interface {
    unsigned char macaddr[6];
    string name;
    int sock;
};


int main(int argc, char **argv){

	int tableLen = 0;
	routingTableRow table[5];

	if(argc < 2){
		cout << "[ERROR] Filename for routing table is required" << endl;
		exit(1);
	}
	
	// can now just use the table number in the argv (ex: ./router 1)
	std:string filename;
	int table_int = stoi(argv[1]);
	if(table_int == 1){
		filename = "r1-table.txt";
	}else{
		filename = "r2-table.txt";
	}
	cout << "Router table file chosen: " << filename << endl;
	

	ifstream inputFile(filename);

	string text;
	if(inputFile.is_open()) {
		// Read all file contents into a string
		text.assign( (std::istreambuf_iterator<char>(inputFile) ),
		   (std::istreambuf_iterator<char>()));

		inputFile.close();
	}

	istringstream tokWords(text);
	string word;

	int numWords = 1;
	while(tokWords >> word) {
		if(numWords % 3 == 1){
			table[tableLen].networkPrefix = word;
		}
		else if(numWords % 3 == 2){
			table[tableLen].nextHopDevice = word;
		}
		else if(numWords % 3 == 0){
			table[tableLen].interfaceName = word;
			tableLen++;
		}
		numWords++;
	}
	    

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
			if(!strncmp(&(tmp->ifa_name[3]),"eth",3)){
				cout << "Creating Socket on interface " << tmp->ifa_name << endl;

				packet_sockets[i] = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
				if(packet_sockets[i]<0){
					perror("socket");
					return 2;
				}

			    // Mac map uses ip as the key and the value
                // is the mac, interface name, and socket number
                struct interface face;
                memcpy(&face.macaddr, s->sll_addr, 8);
                face.name = iName;
                face.sock = packet_sockets[i];

                if(iName.compare("r1-eth0") == 0) {
                    macMap.insert(make_pair("10.0.0.1", face));
                }
                else if(iName.compare("r1-eth1") == 0) {
                    macMap.insert(make_pair("10.1.0.1", face));
                }
                else if(iName.compare("r1-eth2") == 0) {
                    macMap.insert(make_pair("10.1.1.1", face));
                }
                else if(iName.compare("r2-eth0") == 0) {
                    macMap.insert(make_pair("10.0.0.2", face));
                }
                else if(iName.compare("r2-eth1") == 0) {
                    macMap.insert(make_pair("10.3.0.1", face));
                }
                else if(iName.compare("r2-eth2") == 0) {
                    macMap.insert(make_pair("10.3.1.1", face));
                }
                else if(iName.compare("r2-eth3") == 0) {
                    macMap.insert(make_pair("10.3.4.1", face));
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
				cout << "----------------------------------" << endl;
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
					struct iphdr iph;
					struct in_addr ipaddr, ipaddr2;
					memcpy(&iph, line+14, 20);

					ipaddr.s_addr = iph.daddr;
					ipaddr2.s_addr = iph.saddr;
					string destIP = inet_ntoa(ipaddr);
					string srcIP = inet_ntoa(ipaddr2);

					cout << "Received an ICMP packet for: " << destIP << endl;
					if(checkSum(&iph, sizeof(iphdr)) != 0) {
						cout << "Checksum is incorrect, packet is being dropped" << endl;
						continue;
					}

					string routerIP = getRouterIP(table, tableLen, destIP);
					 // if packet is NOT for the router
					if(macMap.count(destIP) == 0) {
							stringstream ss;
							for(unsigned char c : macMap[routerIP].macaddr)
								ss << setw(2) << setprecision(2) << setfill('0') << hex << (unsigned)c << ":";
							string strMac = ss.str();
							cout << "Hop to: " << routerIP << "\tMac address is: " << strMac << endl;

							iph.ttl--;
							iph.check = 0;
							iph.check = checkSum(&iph, sizeof(iphdr));
							// put new ip header into packet
							memcpy(&line[14], &iph, sizeof(iphdr)); 

							// time exceeded
							if(iph.ttl < 1){
								string rIP = getRouterIP(table, tableLen, srcIP);
								cout << "TTL reached 0, sending ICMP time exceeded packet" << endl;
								createICMPError(eh, iph, packet_sockets[j], 11, 0, line, rIP);
								continue;
							}
							
							
							string rIP = getRouterIP(table, tableLen, srcIP);
							if(routerIP.compare("DNE") == 0){
								cout << "No table entry found, sending ICMP Network unreachable packet" << endl;
								
								createICMPError(eh, iph, packet_sockets[j], 3, 1, line, rIP);
								continue;
							}

							createArpRequest(macMap[routerIP].sock, eh, iph, routerIP); 
							cout << "Sent ARP packet to " << routerIP << ", waiting for reply" << endl;
							
							// set timeout for ARP response
							struct timeval timeout;
							timeout.tv_sec=2;
							timeout.tv_usec=0;

							setsockopt(macMap[routerIP].sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
							char arpResponse[5000];
							int recv = recvfrom(macMap[routerIP].sock, arpResponse, 5000,0, (struct sockaddr*)&recvaddr, &recvaddrlen);
							// if no ARP response is received
							if(recv == -1){
								if(errno == EWOULDBLOCK){
									// send icmp destination unreachable packet
									createICMPError(eh, iph, packet_sockets[j], 3, 0, line, rIP);
									cout << "No ARP response received" << endl;
									continue;
								}
							}
							cout << "Received ARP response, building new ethernet header" << endl;
							// if ARP Response is received
								// construct new ethernet header
								struct ether_header response_eh;
								memcpy(&response_eh, arpResponse, 14);							
	 							
	 							struct ether_header EH2;
								struct ether_header *eh2 = &EH2;
								memcpy(&eh2->ether_shost, &response_eh.ether_dhost, ETH_ALEN);
								memcpy(&eh2->ether_dhost, &response_eh.ether_shost, ETH_ALEN);
								eh2->ether_type = htons(ETHERTYPE_IP);

								memcpy(&line[0], eh2, sizeof(ether_header));
	 							
								// forward packet on same socket
								cout << "Forwarding packet" << endl;
								send(macMap[routerIP].sock, line, n, 0);
	
							
					 }else{
						// if packet is for our router
						createICMPReply(eh, iph, packet_sockets[j], 0, -1, line);
					}
					
				}
				// arp type is 0x0806
				if(ntohs(eh.ether_type) == ETHERTYPE_ARP) {
					cout << "Received an ARP packet" << endl;
					  
					struct ether_arp arph;
					struct in_addr ipaddr;
					memcpy(&arph, line+14, 28);
					memcpy(&ipaddr.s_addr, arph.arp_tpa, 4);
				 
					string destIP = inet_ntoa(ipaddr);
					string routerIP = getRouterIP(table, tableLen, destIP);
					createArpReply(eh, arph, packet_sockets[j], line, macMap[routerIP].macaddr);
				}
			}
		}
	}
  freeifaddrs(ifaddr);
}

