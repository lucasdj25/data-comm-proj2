import fcntl
import socket
import select
import netifaces


def recievedArp():
    # handle arp packet
    return

def recievedICMP():
    # handle ICMP packet
    return

if __name__ == "__main__":

    socks = []

    # create socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_RAW, socket.htons(socket.ETH_P_ALL))

    # ip = input("Enter IP of server: ")
    # port = int(input("Enter port # of server: "))

    server_addr = (socket.INADDR_ANY, 8080)

    # bind to client address
    sock.bind(server_addr)

    # read through respective routing table
    

    # set file descriptors using fcntl
    

    # append to socks


    while 1:

        # this will block until at least one socket is ready
        ready_socks,_,_ = select.select(socks, [], []) 
        for sock in ready_socks:
            data, addr = sock.recvfrom(65565) # This is will not block


        # if arp packet, call receivedArp 

        # if ICMP packet, call receivedICMP