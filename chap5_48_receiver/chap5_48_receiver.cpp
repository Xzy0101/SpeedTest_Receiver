/*
48. Write a test program that uses the socket interface to send messages between
a pair of Unix workstations connected by some LAN (e.g., Ethernet, 802.11). Use
this test program to perform the following experiments:
(a) Measure the round-trip latency of TCP and UDP for different message sizes
(e.g., 1 byte, 100 bytes, 200 bytes , . . . , 1000 bytes).
(b) Measure the throughput of TCP and UDP for 1-KB, 2-KB, 3-KB, . . . , 32-KB
messages. Plot the measured throughput as a function of message size.
(c) Measure the throughput of TCP by sending 1 MB of data from one host to another.
Do this in a loop that sends a message of some size¡ªfor example, 1024 iterations
of a loop that sends 1-KB messages. Repeat the experiment with different
message sizes and plot the results.
*/

/*
This is the receiver part.
Receive messages and acknowledge them by sending an ACK message.
*/

#include<stdlib.h>
#include<stdio.h>
#include<WinSock2.h>

#pragma comment(lib,"ws2_32.lib")

#define PORT 5432
#define K 1024
#define M 1024*1024
#define ACK "a"

#define MAX_PENDING 1

char buffer[M];	//too large to be in a function

int main()
{
	WSADATA wsaData;
	WORD version = MAKEWORD(2, 2);	//WSA version 2.2
	int iresult;	//initialization result
	int af, type, protocol;	//protocol type
	char mode;
	SOCKET s, new_s;
	sockaddr_in sin;
	int sockaddr_len = sizeof(sin);

	/*select protocol type*/
	printf("Protocol('t' for TCP, 'u' for UDP): ");	
	mode = fgetc(stdin);
	switch (mode)
	{
	case 't':
		af = AF_INET;
		type = SOCK_STREAM;
		protocol = IPPROTO_TCP;
		break;
	case 'u':
		af = AF_INET;
		type = SOCK_DGRAM;
		protocol = IPPROTO_UDP;
		break;
	default:
		printf("Protocol not supported!\n");
		exit(1);
	}
	

	/*set up receive socket*/
	if ((iresult = WSAStartup(version, &wsaData)) != 0) {
		printf("WSA initialize: Error %d\n", iresult);
		exit(1);
	}

	memset((char *)&sin, 0, sizeof(sin));
	sin.sin_family = af;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(PORT);

	if ((s = socket(af, type, protocol)) == SOCKET_ERROR) {
		printf("Socket: Error %d\n", WSAGetLastError());
		exit(1);
	}

	if (bind(s, (sockaddr*)(&sin), sizeof(sin))==SOCKET_ERROR) {
		printf("Bind: Error: %d\n", WSAGetLastError());
		exit(1);
	}

	switch (protocol)
	{
	case IPPROTO_TCP:
		listen(s, MAX_PENDING);
		if ((new_s = accept(s, (sockaddr*)(&sin), &sockaddr_len)) == SOCKET_ERROR) {
			printf("Accept: Error: %d\n", WSAGetLastError());
			exit(1);
		}
		break;
	case IPPROTO_UDP:
		new_s = s;
		break;
	}
	sockaddr_in sender_sin;
	int sender_sin_len = sizeof(sender_sin);
	switch (protocol) {
	case IPPROTO_UDP:
		/*get sender's address, then send ACK to that address*/
		while (recvfrom(new_s, buffer, M, 0,(sockaddr*)&sender_sin,&sender_sin_len)) {
			sendto(new_s, ACK, sizeof(ACK), 0, (sockaddr*)&sender_sin, sender_sin_len);
		}
		break;
	case IPPROTO_TCP:
		/*receive messages and acknowledge them*/
		while (recv(new_s, buffer, M, 0)) {
			send(new_s, ACK, sizeof(ACK), 0);
		}
		break;
	}
	/*clean up*/
	if (protocol == IPPROTO_TCP)
		closesocket(new_s);
	closesocket(s);
	WSACleanup();
	return 0;
}