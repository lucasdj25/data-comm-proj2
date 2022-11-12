#include <string>
#include "routingtable.h"

// return router ip of the same interface
std::string getRouterIP(struct routingTableRow table[], int tableLen, std::string destIP) {
    std::string routerIP;
    std::string subDestIP = destIP.substr(0,6);
    for(int i = 0; i < tableLen; i++) {
        std::string subRouterIP = table[i].networkPrefix.substr(0,6);

        if(subRouterIP.compare(subDestIP) == 0) {
            if(table[i].nextHopDevice.length() > 1)
                routerIP = table[i].nextHopDevice;
            else
                routerIP = subRouterIP + ".1";
        }
    }

    return routerIP;
}
