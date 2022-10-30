#ifndef __HPP_ROUTER__
#define __HPP_ROUTER__

#include <string>
using namespace std;

typedef struct EtherHeader{
    string src_mac;
    string dest_mac;
    char data[1500];

}EtherHeader;

typedef struct

#endif