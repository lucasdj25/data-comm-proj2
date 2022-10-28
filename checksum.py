"""
char packet[1600]

uint16_t *p = (uint16_t*)&packet[12] start at pos 12 so we skip the IP header

cast to bytes

uint32_t sum

for i in range(packet-length):
    sum += p[i] or packet[12+i]

sum = sum&0xffff + sum >> 16
      ^ gets first 16 bits ^ shifts sum by 16 bits

sum = flipped_bits(sum)
"""