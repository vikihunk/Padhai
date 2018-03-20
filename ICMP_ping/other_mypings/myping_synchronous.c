/*
 * References :
 * RFC's:
 *		ICMP: https://tools.ietf.org/html/rfc792
 *		ICMP checksum: https://tools.ietf.org/html/rfc1071
 *
 * Useful links:
 * https://www.cs.utah.edu/~swalton/listings/sockets/programs/part4/chap18/myping.c
 * https://github.com/iputils/iputils/blob/master/ping.c
 *
 * XXX:
 *		IPv6 support
 *		ping options (are they needed?)
 */

#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <resolv.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <arpa/inet.h>

/*
 * ICMP header as defined in ip_icmp.h
 * struct icmphdr
 * {
 *		u_int8_t type;
 *		u_int8_t code;
 *		u_int16_t checksum;
 *		union
 *		{
 *			struct
 *			{
 *				u_int16_t	id;
 *				u_int16_t	sequence;
 *			} echo;	
 *			u_int32_t	gateway;
 *			struct
 *			{
 *				u_int16_t	__unused;
 *				u_int16_t	mtu;
 *			} frag;	
 *		} un;
 * }
 */

#define PACKETSIZE	64
#define PING_ATTEMPTS	4

struct icmp_packet
{
	struct icmphdr hdr;
	char msg[PACKETSIZE-sizeof(struct icmphdr)];
};

int pid=-1;
struct protoent *proto=NULL;

/*
 * checksum - standard 1s complement checksum
 */
unsigned short checksum(void *b, int len)
{	unsigned short *buf = b;
	unsigned int sum=0;
	unsigned short result;

	for ( sum = 0; len > 1; len -= 2 )
		sum += *buf++;
	if ( len == 1 )
		sum += *(unsigned char*)buf;
	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	result = ~sum;
	return result;
}

void display(void *buf, int bytes)
{	int i;
	struct iphdr *ip = buf;
	struct icmphdr *icmp = buf+ip->ihl*4; /* IPv4 Inernet header length: number of 32-bit words in header */

	printf("----------------\n");

	for ( i = 0; i < bytes; i++ )
	{
	if ( !(i & 15) ) printf("\n0x%04x:  ", i);
		printf("%02x ", ((unsigned char*)buf)[i]);
	}
	printf("\n");

	printf("IPv%d: hdr-size=%d pkt-size=%d protocol=%d TTL=%d src=%s ",
		ip->version, ip->ihl*4, ntohs(ip->tot_len), ip->protocol,
		ip->ttl, inet_ntoa(*(struct in_addr *)&(ip->saddr)));

	printf("dst=%s\n", inet_ntoa(*(struct in_addr *)&(ip->daddr)));

	if ( icmp->un.echo.id == pid )
	{
		printf("ICMP: type[%d/%d] checksum[%d] id[%d] seq[%d]\n",
			icmp->type, icmp->code, ntohs(icmp->checksum),
			icmp->un.echo.id, icmp->un.echo.sequence);
	}
}

void listener(void)
{	int sd;
	struct sockaddr_in addr;
	unsigned char buf[1024];
	int i;

	sd = socket(PF_INET, SOCK_RAW, proto->p_proto);
	if ( sd < 0 )
	{
		perror("socket");
		exit(0);
	}
	for ( i=0; i < PING_ATTEMPTS; i++ )
	{	int bytes, len=sizeof(addr);

		memset(buf, 0, sizeof(buf));
		bytes = recvfrom(sd, buf, sizeof(buf), 0, (struct sockaddr*)&addr, &len);
		if ( bytes > 0 )
			display(buf, bytes);
		else
			perror("recvfrom");
	}
	exit(0);
}

/*
 * Create ICMP echo request and send
 */
void ping(struct sockaddr_in *addr)
{	
	const int val=255;
	int i, sd, cnt=1, j=0;
	struct icmp_packet pckt;
	struct sockaddr_in r_addr;

	sd = socket(PF_INET, SOCK_RAW, proto->p_proto);
	if ( sd < 0 )
	{
		perror("socket");
		return;
	}

	if ( setsockopt(sd, SOL_IP, IP_TTL, &val, sizeof(val)) != 0)
		perror("Set TTL option");

	if ( fcntl(sd, F_SETFL, O_NONBLOCK) != 0 )
		perror("Request nonblocking I/O");

	for ( j=0; j < PING_ATTEMPTS; j++ )
	{	int len=sizeof(r_addr);

	if ( recvfrom(sd, &pckt, sizeof(pckt), 0, (struct sockaddr*)&r_addr, &len) > 0 )

			printf("***Got message!***\n");
		memset(&pckt, 0, sizeof(pckt));
		pckt.hdr.type = ICMP_ECHO;
		pckt.hdr.un.echo.id = pid;

		/* payload is not important - filling with montonically increasing numbers */
		for ( i = 0; i < sizeof(pckt.msg)-1; i++ )
			pckt.msg[i] = i+'0';
		pckt.msg[i] = 0;
		pckt.hdr.un.echo.sequence = cnt++;
		pckt.hdr.checksum = checksum(&pckt, sizeof(pckt));

		if ( sendto(sd, &pckt, sizeof(pckt), 0, (struct sockaddr*)addr, sizeof(*addr)) <= 0 )
			perror("sendto");

		sleep(1);
	}
}

int main(int argc, char *argv[])
{	
	struct hostent *hname;
	struct sockaddr_in addr;

	if ( argc != 2 )
	{
		printf("usage: %s <addr>\n", argv[0]);
		exit(0);
	}

	if ( argc > 1 )
	{
		pid = getpid();
		proto = getprotobyname("ICMP");
		hname = gethostbyname(argv[1]);
		memset(&addr, 0, sizeof(addr));
		addr.sin_family = hname->h_addrtype;
		addr.sin_port = 0;
		addr.sin_addr.s_addr = *(long*)hname->h_addr;
		ping(&addr);

		if ( fork() == 0 )
			listener();
		else
			ping(&addr);
		wait(0);
	}
	else
		printf("usage: myping <hostname>\n");
	return 0;
}
