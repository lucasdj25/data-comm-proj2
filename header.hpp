#ifndef __HPP_HEADER__
#define __HPP_HEADER__

#include <string>
using namespace std;

typedef struct EtherHeader{
    string src_mac;
    string dest_mac;
    char data[1500];

}EtherHeader;

typedef struct IPHeader{
    string src_addr;
    string dest_addr;
    char data[1500];
}IPHeader;

typedef struct ARPHeader{

}ARPHeader;

typedef struct ARPPacket{
    ARPHeader ARP_header;
    EtherHeader ether_header;
}ARPPacket;

typedef struct ICMPHeader{
	uint8_t type;
	uint8_t code;
	uint16_t checksum;
	uint8_t identifier;
	uint8_t seq_num;
	char data[1500];
}ICMPHeader;

void createIcmpReply(struct EtherHeader, struct IPHeader, struct ICMPHeader);
void createArpReply(struct EtherHeader, struct ARPHeader);

#endif
