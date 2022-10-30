#include <map>

using namespace std;


std::map<string, tuple <string, string> > table1(){
    map<string, tuple<string, string> > table;
    table.insert("10.0.0.0/16", (NULL, "r1-eth0"));
    table.insert("10.1.0.0/24", (NULL, "r1-eth1"));
    table.insert("10.1.1.0/24", (NULL, "r1-eth2"));
    table.insert("10.3.0.0/16", ("10.0.0.2", "r1-eth0"));

    return table;
}

std::map<string, tuple <string, string> > table2(){
    map<string, tuple<string, string> > table;
    table.insert("10.0.0.0/16", (NULL, "r2-eth0"));
    table.insert("10.3.0.0/24", (NULL, "r2-eth1"));
    table.insert("10.3.1.0/24", (NULL, "r2-eth2"));
    table.insert("10.3.4.0/24", (NULL, "r2-eth3"));
    table.insert("10.1.0.0/16", ("10.0.0.1", "r2-eth0"));
    
    return table;
}





