#include <map>
#include <string>
#include <vector>
#include <iostream>

using namespace std;

typedef map<string, std::vector<string>> table;

table t1 = {{"10.0.0.0/16", {"", "r1-eth0"}}, 
            {"10.1.0.0/24", {"", "r1-eth1"}},
            {"10.1.1.0/24", {"", "r1-eth2"}}, 
            {"10.3.0.0/16", {"10.0.0.2", "r1-eth0"}}};

table t2 = {{"10.0.0.0/16", {"", "r2-eth0"}}, 
            {"10.3.0.0/24", {"", "r2-eth1"}},
            {"10.3.1.0/24", {"", "r2-eth2"}},
            {"10.3.4.0/24", {"", "r2-eth3"}}, 
            {"10.1.0.0/16", {"10.0.0.1", "r2-eth0"}}};

// params: address to search with and number of the router (1 or 2)
// returns the hop, if it has one, else the interface
// TODO: add functionality to return interface if there's no hop
vector<string> get_interface_or_hop(string address, int router){
    cout << "Address input: " << address << endl;
    if (router == 1){
        try{ return t1[address]; }
        catch(exception e){ cout << e.what() << endl; }
    }

    try { return t2[address]; }
    catch(exception e){ cout << e.what() << endl; }

    return {"NONE"};
}
// matches a given address with it's table key
string match_address(string address, int router){
    
    // for prefix 10.0.0.0
    // if the last num = 24: address can be between 10.0.0.0 and 10.0.0.255
    // if the last num = 16: address can be between 10.0.0.0 and 10.0.255.255

    // the number of 
    int num_addrs = stoi(address.substr(address.length() - 2));

    // set table based on the router number
    table table = router == 1 ? t1 : t2;

    

    return NULL;
}

int main(void){
    return 0;
}


