#include <string>
#include <iostream>
#include "routingtable.h"

// return router ip of the same interface
std::string getRouterIP(struct routingTableRow table[], int tableLen, std::string destIP) {
    std::string routerIP;
    std::string subDestIP = destIP.substr(0,6);
    for(int i = 0; i < tableLen; i++) {
        std::string subRouterIP = table[i].networkPrefix.substr(0,6);
			int dC1, dC2, dC3, dC4, rC1, rC2, rC3, rC4, range;
			sscanf(destIP.c_str(), "%d.%d.%d.%d", &dC1, &dC2, &dC3, &dC4);
			sscanf(table[i].networkPrefix.c_str(), "%d.%d.%d.%d/%d", &rC1, &rC2, &rC3, &rC4, &range);  
			if(range == 16) {
				rC3 = 255;
				rC4 = 255;
			}
			else if(range == 24) {
				rC4 = 255;
			}
			
			// destIP is in range of the router ips
			if(dC2 == rC2 && dC3 <= rC3 && dC4 <= rC4) {
				if(table[i].nextHopDevice.length() > 1) {
					routerIP = table[i].nextHopDevice;
					std::cout << "Packet needs to hop to device: " << routerIP << std::endl;
					break;
				}
				else {
					routerIP = subRouterIP + ".1";
					break;
				}
			}
			else {
				routerIP = "DNE";
			}
    }
    return routerIP;
}
