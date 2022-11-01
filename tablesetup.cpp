#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <cstdio>
/*
All code in this file will eventually end up in router.cpp.
This file only exists to test out functions and not clutter the main program for now
*/
using namespace std;

typedef map<string, vector<string> > table;

table t1 = {{"10.0.0.0/16", {"", "r1-eth0"}}, 
            {"10.1.0.0/24", {"", "r1-eth1"}},
            {"10.1.1.0/24", {"", "r1-eth2"}}, 
            {"10.3.0.0/16", {"10.0.0.2", "r1-eth0"}}};

table t2 = {{"10.0.0.0/16", {"", "r2-eth0"}}, 
            {"10.3.0.0/24", {"", "r2-eth1"}},
            {"10.3.1.0/24", {"", "r2-eth2"}},
            {"10.3.4.0/24", {"", "r2-eth3"}}, 
            {"10.1.0.0/16", {"10.0.0.1", "r2-eth0"}}};

// returns the next hop for an address
string get_next_hop(int router, string address){
    string hop = "";
    try {
        hop = router == 1 ? t1[address][0] : t2[address][0];
    }
    catch(exception e){
    }
    return hop;
}

// returns the interface for an address
string get_interface(int router, string address){
    return router == 1 ? t1[address][1] : t2[address][1];
}

// matches a given address with it's table key
/* string match_address(string address, int router){
    
    // for prefix 10.0.0.0
    // if the last num = 24: address can be between 10.0.0.0 and 10.0.0.255
    // if the last num = 16: address can be between 10.0.0.0 and 10.0.255.255

    // the number after the /
    int num_addrs = stoi(address.substr(address.length() - 2));

    // set table based on the router number
    table table = router == 1 ? t1 : t2;
    int num1, num2, num3, num4, num5;
    if (sscanf(address, "%d.%d.%d.%d/%d", &num1, &num2, &num3, &num4, &num5) == 5){
        int x = 1;
    }

    

    return "";
} */

// matches an input address with it's key in the table
// ex: 10.0.0.255 would match with 10.0.0.0/16
/*string match_address(int router, string address){
    int length = stoi(address.substr(address.length() - 2));

    // the number of digits in the ip address to match
    // this depends on whether it's /16 or /24
    int nums_to_match = 0;
    if (length == 16){

    }
} */


int main(void){

    string test_list[5] = {"10.0.0.0/16", "10.1.1.0/24", "apple"};
    int length = sizeof(test_list) / sizeof(string);
    for (int i = 0; i < 3; i++){
        cout << "interface for " << test_list[i] << ": " << get_interface(1, test_list[i]) << endl;
    }


    return 0;
}


