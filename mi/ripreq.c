#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>

#include "wrapper_functions.h"

#define MAX_ENTRY 25
#define PORT 520
#define MAX_DATA 504


struct rip_entry
{
    unsigned int af_identifier : 16;
    unsigned int route_tag : 16;
    unsigned int ip_address : 32;
    unsigned int subnet_mask : 32;
    unsigned int next_hop : 32;
    unsigned int metric : 32;
};   

struct rip_packet
{
    unsigned int command : 8;
    unsigned int version : 8;
    unsigned int must_be_zero : 16;
    struct rip_entry entries[MAX_ENTRY];
};

void set_rip_packet(struct rip_packet *packet)
{
    packet->command = 0x01;
    packet->version = 0x02;

    packet->entries[0].metric = ntohl(0x10);
    
    return;
}

void usage()
{
    fprintf(stderr, "Usage: /ripreq IP_address\n");
    exit(1);
}

int main(int argc, char **argv)
{
    if(argc != 2){
        usage();
    }
    
    char *ip_address = argv[1];
    int port = htons(PORT);

    int sockfd = w_socket(AF_INET, SOCK_DGRAM, 0);
    
    struct rip_packet packet;
    memset(&packet, 0, sizeof(packet));
    set_rip_packet(&packet);
    
    struct sockaddr_in serv_addr;
    struct addrinfo hints, *res;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    
    w_getaddrinfo(ip_address, NULL, &hints, &res);
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = port;
    serv_addr.sin_addr = ((struct sockaddr_in *)res->ai_addr)->sin_addr;

    w_sendto(sockfd, &packet, sizeof(packet)-24*sizeof(struct rip_entry), 0, (const struct sockaddr *)&serv_addr, sizeof(serv_addr));
    
    w_recvfrom(sockfd, &packet, sizeof(packet), 0, NULL, NULL);
    
    for(int i = 0; i < MAX_ENTRY; ++i){
		int address = packet.entries[i].ip_address;
		int submask = packet.entries[i].subnet_mask;
		int metrika = ntohl(packet.entries[i].metric);
		if(address == 0) break;
		struct in_addr addr;
		struct in_addr mask;
		addr.s_addr = address;
		char *ipaddr = inet_ntoa(addr);
		printf("%s/", ipaddr);
		mask.s_addr = submask;
		char *strmask = inet_ntoa(mask);
		printf("%s metrika: %d\n", strmask, metrika);
	}

    return 0;
}
