#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <linux/if_link.h>

#define PACKET_SIZE 4096
#define MAX_WAIT_TIME 5
#define MAX_NO_PACKETS 3
#define MAX_IP_STR_LEN 16
#define MAX_IP_BIT_LEN 32

char sendpacket[PACKET_SIZE];
char recvpacket[PACKET_SIZE];

int sockfd, datalen = 56;
int nsend = 0, nreceived = 0;

struct sockaddr_in dest_addr;

pid_t pid;

struct sockaddr_in from;
struct timeval timevrecv;

FILE *outfile = NULL;

int num_of_if_addresses = 0;
char **if_addresses = NULL;

void statistics(int signo);

unsigned short cal_chksum(unsigned short *addr, int len);

int pack(int pack_no);

void send_packet(void);
void recv_packet(void);

int unpack(char *buf, int len);

void tv_sub(struct timeval *out, struct timeval *in);

int bits_to_dec(int *bits, int len);