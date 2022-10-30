
R1_TABLE = {"10.0.0.0/16" : "r1-eth0",
            "10.1.0.0/24" : "r1-eth1",
            "10.1.1.0/24" : "r1-eth2",
            "10.3.0.0/16" : "r1-eth0",
            "10.0.0.2" : "r1-eth0"}

R2_TABLE = {"10.0.0.0/16" : "r2-eth0",
            "10.3.0.0/24" : "r2-eth1",
            "10.3.1.0/24" : "r2-eth2",
            "10.3.4.0/24" : "r2-eth3",
            "10.1.0.0/16" : "r2-eth0",
            "10.0.0.1" : "r2-eth1"}



def checksum():
    """
char packet[1600]

(arp packets should not be 1600 bytes)

uint16_t *p = (uint16_t*)&packet[12] start at pos 12 so we skip the IP header

cast to bytes

uint32_t sum

for i in range(packet-length):
    sum += p[i] or packet[12+i]

sum = sum&0xffff + sum >> 16
      ^ gets first 16 bits ^ shifts sum by 16 bits

sum = flipped_bits(sum)
"""
    pass

# returns the address from the given routing table
# TODO: modify params 
def read(table: str, other_params: str):
    pass


def ARP_recieved():
    pass

def ICMP_received():
    pass
    