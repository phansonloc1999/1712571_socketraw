// A simple Ping program to ping any address such as google.com in Linux
// run program as : gcc -o ping ping.c
//  then : ./ping google.com
// can ping localhost addresses
// see the RAW socket implementation
/*.....Ping Application....*/
/*   Note Run under root priveledges  */
/*  On terminal
  $ gcc -o out ping.c
  $ sudo su
  # ./out 127.0.0.1
   OR
  # ./out google.com */
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
#include <setjmp.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <byteswap.h>

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
struct timeval tvrecv;

void statistics(int signo);

unsigned short cal_chksum(unsigned short *addr, int len);

int pack(int pack_no);

void send_packet(void);
void recv_packet(void);

int unpack(char *buf, int len);

void tv_sub(struct timeval *out, struct timeval *in);

int bits_to_dec(int *bits, int len);

void statistics(int signo)
{
    printf("\n--------------------PING statistics-------------------\n");
    printf("%d packets transmitted, %d received , %%%d lost\n", nsend, nreceived, (nsend - nreceived) / nsend * 100);
    printf("\n");
    close(sockfd);
}

unsigned short cal_chksum(unsigned short *addr, int len)
{
    int nleft = len;
    int sum = 0;

    unsigned short *w = addr;
    unsigned short answer = 0;

    while (nleft > 1)
    {
        sum += *w++;
        nleft -= 2;
    }

    if (nleft == 1)
    {
        *(unsigned char *)(&answer) = *(unsigned char *)w;

        sum += answer;
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    answer = ~sum;

    return answer;
}

int pack(int pack_no)
{
    int i, packsize;
    struct icmp *icmp;
    struct timeval *tval;

    icmp = (struct icmp *)sendpacket;
    icmp->icmp_type = ICMP_ECHO;
    icmp->icmp_code = 0;
    icmp->icmp_cksum = 0;
    icmp->icmp_seq = pack_no;
    icmp->icmp_id = pid;

    packsize = 8 + datalen;

    tval = (struct timeval *)icmp->icmp_data;

    gettimeofday(tval, NULL);

    icmp->icmp_cksum = cal_chksum((unsigned short *)icmp, packsize);

    return packsize;
}

void send_packet()
{
    int packetsize;

    while (nsend < MAX_NO_PACKETS)
    {
        nsend++;
        packetsize = pack(nsend);

        if (sendto(sockfd, sendpacket, packetsize, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0)
        {
            perror("sendto error");

            continue;
        }
        sleep(1);
    }
}

void recv_packet()
{
    int n;
    socklen_t fromlen;
    extern int errno;

    fromlen = sizeof(from);

    signal(SIGALRM, statistics);

    while (nreceived < nsend)
    {
        // Wait 5 seconds to receive for each packets in nsend packet
        alarm(MAX_WAIT_TIME);
        
        if ((n = recvfrom(sockfd, recvpacket, sizeof(recvpacket), 0, (struct sockaddr *)&from, &fromlen)) < 0)
        {
            if (errno == EINTR)
                continue;

            // Bad socket descriptor indicates socket may be closed by statistics()
            // Stop calling recvfrom and return to main()
            if (errno = EBADF) 
                return;

            continue;
        }
        gettimeofday(&tvrecv, NULL);

        if (unpack(recvpacket, n) == -1)
            continue;

        nreceived++;
    }

    statistics(0);
}

int unpack(char *buf, int len)
{
    int i, iphdrlen;
    struct ip *ip;
    struct icmp *icmp;
    struct timeval *tvsend;
    double rtt;

    ip = (struct ip *)buf;
    iphdrlen = ip->ip_hl << 2;
    icmp = (struct icmp *)(buf + iphdrlen);
    len -= iphdrlen;

    if (len < 8)
    {
        printf("ICMP packets\'s length is less than 8\n");
        return -1;
    }

    if ((icmp->icmp_type == ICMP_ECHOREPLY) && (icmp->icmp_id == pid))
    {
        tvsend = (struct timeval *)icmp->icmp_data;
        tv_sub(&tvrecv, tvsend);
        rtt = tvrecv.tv_sec * 1000 + tvrecv.tv_usec / 1000;

        printf("%d byte from %s: icmp_seq=%u ttl=%d rtt=%.3f ms\n", len,

               inet_ntoa(from.sin_addr), icmp->icmp_seq, ip->ip_ttl, rtt);
    }
    else
        return -1;

    return 0;
}

int bits_to_dec(int *bits, int len)
{
    int result = 0, j = 0;
    for (int i = len - 1; i >= 0; i--)
    {
        result = result + bits[i] * pow(2, j);
        j++;
    }
    return result;
}

int cidr_to_ip_and_mask(const char *cidr, uint32_t *ip, uint32_t *mask)
{
    uint8_t a, b, c, d, bits;
    if (sscanf(cidr, "%hhu.%hhu.%hhu.%hhu/%hhu", &a, &b, &c, &d, &bits) < 5)
    {
        return -1; /* didn't convert enough of CIDR */
    }
    if (bits > 32)
    {
        return -1; /* Invalid bit count */
    }
    *ip = (a << 24UL) | (b << 16UL) | (c << 8UL) | (d);
    *mask = (0xFFFFFFFFUL << (32 - bits)) & 0xFFFFFFFFUL;

    return 0;
}

struct in_addr *get_all_host_ips(char *net_ip_and_subnet_bits, int *result_num_of_hosts)
{
    uint32_t network_ip;
    uint32_t mask;
    uint32_t host_ip;
    if (cidr_to_ip_and_mask(net_ip_and_subnet_bits, &network_ip, &mask) == 0)
    {
        *result_num_of_hosts = ~mask;
        size_t in_addr_size = sizeof(struct in_addr);
        int n = *result_num_of_hosts;
        struct in_addr *host_addresses = (struct in_addr *)malloc(in_addr_size * n);
        struct in_addr host_addr;
        for (int i = 1; i < (~mask); i++)
        {
            host_ip = i & (mask + i);
            host_addr = inet_makeaddr(network_ip, host_ip);
            host_addresses[i - 1] = host_addr;
        }
        return host_addresses;
    }
    else
        printf("[ERROR]: Failed to get host ips from CIDR");

    return NULL;
}

int main(int argc, char *argv[])
{
    struct hostent *host;
    struct protoent *protocol;
    unsigned long inaddr = 0l;
    int waittime = MAX_WAIT_TIME;
    int size = 50 * 1024;

    if (argc < 2)
    {
        printf("usage:%s hostname/IP address\n", argv[0]);
        exit(1);
    }
    if ((protocol = getprotobyname("icmp")) == NULL)
    {
        perror("getprotobyname");
        exit(1);
    }

    bzero(&dest_addr, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;

    int num_of_hosts = 0;
    struct in_addr *host_addresses = get_all_host_ips(argv[1], &num_of_hosts);

    for (int i = 0; i < num_of_hosts; i++)
    {
        datalen = 56;
        nsend = 0, nreceived = 0;

        dest_addr.sin_addr = host_addresses[i];

        if ((sockfd = socket(AF_INET, SOCK_RAW, protocol->p_proto)) < 0)
        {
            perror("socket error");
            exit(1);
        }
        setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));

        printf("PING %s: %d bytes data in ICMP packets.\n", inet_ntoa(host_addresses[i]), datalen);

        send_packet();
        recv_packet();
    }

    return 0;
}

void tv_sub(struct timeval *out, struct timeval *in)
{
    if ((out->tv_usec -= in->tv_usec) < 0)
    {
        --out->tv_sec;
        out->tv_usec += 1000000;
    }
    out->tv_sec -= in->tv_sec;
}