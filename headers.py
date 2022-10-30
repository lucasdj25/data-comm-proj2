import struct
import netifaces
"""
ARP request - response
Rq       Rs
src ip   dest ip
src mac  dest mac
dest ip  src ip
dest mac src mac

"""

class EtherHeader:
    def __init__(self, src_mac, dest_mac, data):
        self.src_mac : bytes = src_mac
        self.dest_mac : bytes = dest_mac
        self.data : bytes = data
      # self.checksum = checksum



class IPHeader:
    def __init__(self, src_addr, dest_addr, data):
        self.src_addr : bytes = src_addr
        self.dest_addr : bytes = dest_addr
        self.data : bytes = data
        # self.checksum = checksum
    
    

class ICMPHeader:
    pass


class ARPHeader:
    pass

class ARP:
    def __init__(self, ether_header, ARP_header):
        self.ether_header : EtherHeader = ether_header
        self.ARP_header : ARPHeader = ARP_header


