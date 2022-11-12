#ifndef ROUTINGTABLE_H
#define ROUTINGTABLE_H

#include <string>

struct routingTableRow {
    std::string networkPrefix;
    std::string nextHopDevice;
    std::string interfaceName;
};

std::string getRouterIP(struct routingTableRow table[], int, std::string);

#endif
