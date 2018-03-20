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
/* Libevent. */
#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

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
#define PING_ATTEMPTS	5

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

int verify_echo_reply(void *buf, int bytes)
{
	struct iphdr *ip = buf;
	struct icmphdr *icmp = buf + ip->ihl*4;

	if (icmp->type == ICMP_ECHOREPLY) {
		return 0;
	}
	return -1;
}

void display(void *buf, int bytes)
{	int i;
	struct iphdr *ip = buf;
	struct icmphdr *icmp = buf+ip->ihl*4; /* IPv4 Inernet header length: number of 32-bit words in header */

	printf("----------------\n");

#ifdef _DISPLAY_RAW_
	for ( i = 0; i < bytes; i++ )
	{
	if ( !(i & 15) ) printf("\n0x%04x:  ", i);
		printf("%02x ", ((unsigned char*)buf)[i]);
	}
	printf("\n");
#endif

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

void listen_ev_thread(int sd, short ev, void *arg)
{
	unsigned char buf[1024];
	int bytes, len;
	struct sockaddr_in addr;
	int i;

	if (ev & EV_TIMEOUT) 
	{
		printf("Timeout: Destination Host Unreachable\n");
		exit(0);
	}

	memset(buf, 0, sizeof(buf));
	for ( i=1; i < PING_ATTEMPTS; i++ )
	{	
		len = sizeof(addr);
		memset(buf, 0, sizeof(buf));
		bytes = recvfrom(sd, buf, sizeof(buf), 0, (struct sockaddr*)&addr, &len);
		/* Check if the response is echo reply */
		if ( verify_echo_reply(buf, bytes) < 0)
		{
			printf("Not an echo response\n");
			continue;
		}

		if ( bytes > 0 && bytes <= PACKETSIZE + sizeof(struct iphdr))
			display(buf, bytes);
		else
			perror("recvfrom");
	}
	exit(0);
}

void listener(void)
{	
	int sd;
	struct event ev_receive;
	/* The libevent event base */
	struct event_base *evbase;
	struct timeval timeout_val = { 5, 0 };

	/* Initialize libevent. */
	evbase = event_base_new();

	sd = socket(PF_INET, SOCK_RAW, proto->p_proto);
	if ( sd < 0 )
	{
		perror("socket");
		exit(0);
	}

	if (setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO,&timeout_val,sizeof(timeout_val)) < 0)
	{
		printf("Failed to set timeout\n");
		exit(0);
	}

	/* create a read event to
	 * be notified when reading from a client */
	event_assign(&ev_receive, evbase, sd, EV_TIMEOUT|EV_READ|EV_PERSIST, 
			listen_ev_thread, NULL);

	event_add(&ev_receive, &timeout_val); /* With a timeout of 5 seconds */

	/* Start the event loop. */
	event_base_dispatch(evbase);
	close(sd);
	event_base_free(evbase);
	return;
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

	if ( fcntl(sd, F_SETFL, O_NONBLOCK) != 0 )
		perror("Request nonblocking I/O");

	for ( j=1; j < PING_ATTEMPTS; j++ )
	{	int len=sizeof(r_addr);

		printf("Echo Msg #%d\n", j);
		memset(&pckt, 0, sizeof(pckt));
		pckt.hdr.type = ICMP_ECHO;
		pckt.hdr.un.echo.id = pid;

		/* payload is not important - filling with ASCII values */
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
		if (hname == NULL) {
			printf("Failed to lookup host: %s\n", argv[1]);
			exit(0);
		}
		memset(&addr, 0, sizeof(addr));
		addr.sin_family = hname->h_addrtype;
		addr.sin_port = 0;
		addr.sin_addr.s_addr = *(long*)hname->h_addr;

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
