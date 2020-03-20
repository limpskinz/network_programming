#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>

#define MESSAGE_MAXLEN 512
#define PORTLEN 22

struct pairs{
	char ip[INET_ADDRSTRLEN];
	char port[PORTLEN];
};

struct msg{
	char command;
	struct pairs entries[20];
};

void usage(){
	fprintf(stderr, "Usage: ./bot server_ip server_port\n");
}

void parse_packet(struct msg *message, char *packet, int bytes_recv){
	int i = PORTLEN + INET_ADDRSTRLEN;
	bytes_recv--; // decrementing by size of char command
	int counter = bytes_recv / i;
	message->command = *packet++;
	printf("%s\n", message->command);
	for(i = 0; i < counter; i++){
		strcpy(message->entries[i].ip, packet);
		packet += sizeof(message->entries[i].ip);
		strcpy(message->entries[i].port, packet);
		packet += sizeof(message->entries[i].port);
	}
}

void prog(struct msg *message){

	char payload[512];
	char *data = "HELLO\n";

	char *udp_server_ip = message->entries[0].ip;
	char *udp_server_port = message->entries[0].port;

	struct addrinfo *udp_server;
	struct addrinfo hints;

	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_DGRAM;

	int sock;
	sock = socket(PF_INET, SOCK_DGRAM, 0);

	if(sock < 0){
		err(2, "socket error\n");
	}

	int gai_error;

	if((gai_error = getaddrinfo(udp_server_ip, udp_server_port, &hints, &udp_server)) != 0){
		err(3, "getaddrinfo() failed. %s\n", gai_strerror(gai_error));
	}

	int bytes_sent = sendto(sock, data, sizeof(data), 0, udp_server->ai_addr, udp_server->ai_addrlen);

	int bytes_recv = recvfrom(sock, payload, sizeof(payload), 0, NULL, NULL);

	printf("%s\n", payload);
}

int main(int argc, char **argv){

	//the program requires 2 arguments
	if (argc != 3){
		usage();
		return 1;
	}

	char *server_ip = argv[1];
	char *server_port = argv[2];

	const char *data = "REG\n";
	char packet[MESSAGE_MAXLEN];
	struct msg message;

	struct addrinfo *server_address;
	struct addrinfo hints;

	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_DGRAM;

	int gai_error;

	int sock; //the socket

	sock = socket(PF_INET, SOCK_DGRAM, 0);

	if(sock < 0){
		fprintf(stderr, "socket error\n");
		return 2;
	}

	if((gai_error = getaddrinfo(server_ip, server_port, &hints, &server_address)) != 0){
		fprintf(stderr, "getaddrinfo() failed. %s\n", gai_strerror(gai_error));
		return 3;
	}

	int bytes_sent = sendto(sock, data, sizeof(data), 0, server_address->ai_addr, server_address->ai_addrlen);

	int bytes_recv = recvfrom(sock, packet, sizeof(packet), 0, NULL, NULL);

	parse_packet(&message, packet, bytes_recv);

	if(message.command == '0'){
		prog();
	}

	freeaddrinfo(server_address);
}