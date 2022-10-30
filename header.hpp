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
#endif