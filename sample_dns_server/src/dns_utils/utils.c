/*
 * Utilities for DNS server
 */

#include "utils.h"

/*
 * Utility to parse and print DNS header for the following:
 * - Transaction ID
 * - Domain name
 * - Query/Response type
 * Before caling this function, ensure that buffer has been verified
 * Qname received in this buffer is converted to human readable form
 *
 */
int read_and_print_dns_header(unsigned char *buffer, unsigned int buflen, qr_data_t *client_data, struct sockaddr_in *clientAddr)
{
	int i;
	int j;
	int p;
	struct DNS_HEADER *dns;
	unsigned char *qname, *qname_cpy;
	query_type_t q_type;
	char domain_name[MAX_NAME_LEN];

	if (buffer == NULL || clientAddr == NULL || client_data == NULL)
	{
		debug_print("Invalid input\n");
		return (-1);
	}

	memset(client_data, '\0', sizeof(qr_data_t));
	/* Buffer read has DNS HEADER and qname */
	dns = (struct DNS_HEADER *)buffer;
	client_data->tid = ntohs(dns->id);

	printf("\nClient address: %s\n", inet_ntoa(clientAddr->sin_addr));
	printf("Transaction id(base 10): %d\n", client_data->tid);

	/* After header is the domain name */
	qname = (unsigned char*)buffer + sizeof(struct DNS_HEADER);
	qname_cpy = qname;

	/* 
	 * Verify whether qname ends in a '\0',
	 * if so create a local copy 
	 * This ensures that qname is not blindly
	 * copied (ensuring that a rogue packet data isn't used)
	 */
	memset(domain_name, '\0', MAX_NAME_LEN);
	if(verify_and_copy_qname(qname_cpy, domain_name, buflen) == -1)
	{
		debug_print("domain name in buffer invalid\n");
		return -1;
	}

	/* Convert to domain_name format */
	for(i=0;i<(int)strlen(domain_name);i++) 
	{
		p=domain_name[i];
		for(j=0;j<(int)p;j++) 
		{
			domain_name[i]=domain_name[i+1];
			i=i+1;
		}
		domain_name[i]='.';
	}
	domain_name[i-1]='\0';

	strncpy(client_data->domain_name, domain_name, strlen(domain_name));

	printf("Domain name: %s\n", client_data->domain_name);

	/*
	 * QR is the query/response code
	 * if 0 - query
	 * if 1 - response
	 */
	q_type = ntohs(dns->qr);
	printf("Query type: %s\n", (q_type == REQUEST) ? "REQUEST" : "RESPONSE");
	return 0;
}

/*
 * Verifies that qname in input buffer is valid
 * Creates a local copy of qname in domain_name
 */
int verify_and_copy_qname(char *qname_cpy, char *domain_name, unsigned int buflen) {
	int i;

	if(qname_cpy == NULL || domain_name == NULL) {
		debug_print("Invalid input\n");
		return -1;
	}

	i = 0;
	while(*qname_cpy!= '\0')
	{
		domain_name[i] = *qname_cpy;
		if(i > buflen)
		{
			debug_print("Rogue packet\n");
			return -1;
		}
		qname_cpy++, i++;
	}
	domain_name[i] = '\0';
	return 0;
}

/*
 * Utility to change domain name as specified by DNS
 * Each name is preceded by the count of entries:
 * eg: 3www6google3com0 - 3 w's, 6 letters for google, 3 for com and 0 as terminator
 * 
 */
int ChangetoDnsNameFormat(unsigned char* dns,unsigned char* host) 
{
	int lock = 0 , i;
	if(dns == NULL || host == NULL)
	{
		debug_print("Invalid input\n");
		return -1;
	}
	strcat((char*)host,".");

	for(i = 0 ; i < strlen((char*)host) ; i++) 
	{
		if(host[i]=='.') 
		{
			*dns++ = i-lock;
			for(;lock<i;lock++) 
			{
				*dns++=host[lock];
			}
			lock++;
		}
	}
	*dns++='\0';
	return 0;
}

/*
 * Step 2 implementation
 * Forwarding query to upstream DNS
 * It also parses the response from the upstream DNS
 * server and prints the response
 * Buffer and nBytes are both used as input and output param
 * - Input buffer is the DNS query from DNS Client
 * - Output buffer is the DNS response from upstream DNS server
 * - Input nBytes is the buffer length from DNS client
 * - Output nBytes is the buffer length received from Upstream DNS server
 */
int send_data_to_dns(unsigned char *buffer, unsigned int *nBytes)
{
	unsigned char *qname, *qname_cpy;
	unsigned char *reader;
	struct DNS_HEADER *dns;
	int i, j;
	long *p;
	char domain_name[MAX_NAME_LEN];
	struct DNS_HEADER *client_dns_req;
	int gdnssocket;
	struct sockaddr_in g_dest;
	struct QUESTION *qinfo;
	unsigned char *g_qname;
	struct DNS_HEADER *g_dns;

	/* Response Records from DNS server */
	struct RES_RECORD answers[RECORD_LEN];
	struct RES_RECORD auth[RECORD_LEN];
	struct RES_RECORD addit[RECORD_LEN];
	struct sockaddr_in a;
	int stop;
	int reuseaddr_on;
	struct timeval tv;

	qname = (unsigned char*)buffer + sizeof(struct DNS_HEADER);
	dns = (struct DNS_HEADER *)buffer;
	qname_cpy = qname;

	/* Verify that qname is valid, create a local copy */
	i = 0;
	memset(domain_name, '\0', MAX_NAME_LEN);
	if(verify_and_copy_qname(qname_cpy, domain_name, *nBytes) == -1)
	{
		debug_print("domain name in buffer invalid\n");
		return -1;
	}

	if(ChangetoDnsNameFormat(qname , domain_name) == -1)
	{
		debug_print("Failed to change name to DNS format\n");
		return -1;
	}

	/* Allocate memory for request DNS header */
	client_dns_req = malloc(sizeof(struct DNS_HEADER));
	memcpy(client_dns_req, dns, sizeof(struct DNS_HEADER));

	/* Forward this request to Upstream DNS */
	gdnssocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); /* UDP for DNS */
	g_dest.sin_family = AF_INET;
	g_dest.sin_port = htons(UPSTREAM_DNS_PORT);
	g_dest.sin_addr.s_addr = inet_addr(UPSTREAM_DNS_SERVER);

	reuseaddr_on = 1;
	if (setsockopt(gdnssocket, SOL_SOCKET, SO_REUSEADDR, &reuseaddr_on, 
		sizeof(reuseaddr_on)) != 0)
	{
		debug_print("Failed to set reuse option: %s\n", strerror(errno));
		close(gdnssocket);
		return -1;
	}

	/* Set timeout for Upstream DNS socket */
	tv.tv_sec = UPSTREAM_DNS_SOCKET_TIMEOUT; /* 5 seconds */
	tv.tv_usec = 0;
	if (setsockopt(gdnssocket, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0)
	{
		debug_print("Failed to set timeout option: %s\n", strerror(errno));
		close(gdnssocket);
		return -1;
	}

	memset(buffer, '\0', MAX_BUF_LEN);
	g_dns = (struct DNS_HEADER *)buffer;

	/*
	 * Copy details of header from client request header
	 * and pass it to GDNS
	 */

	memcpy(g_dns, client_dns_req, sizeof(dns));
	/* Now client_dns_req can be freed */
	free(client_dns_req);

	g_qname =(unsigned char*)buffer + sizeof(struct DNS_HEADER);
	if(ChangetoDnsNameFormat(g_qname , domain_name) == -1)
	{
		debug_print("Failed to change name to DNS format\n");
		close(gdnssocket);
		return -1;
	}

	/* QINFO is after the DNS header and QNAME in the message */
	qinfo =(struct QUESTION*)&buffer[sizeof(struct DNS_HEADER) + (strlen((const char *)domain_name))];

	qinfo->qtype = htons( T_A ); /* type of the query - only A supported */
	qinfo->qclass = htons(1); /* internet */

	if(sendto(gdnssocket, (char*)buffer,
		sizeof(struct DNS_HEADER) + (strlen((const char*)g_qname)+1) + sizeof(struct QUESTION), 0,(struct sockaddr*)&g_dest, sizeof(g_dest)) < 0)
	{
		debug_print("Failed to send pkt to Upstream DNS: %s\n", strerror(errno));
		close(gdnssocket);
		return -1;
	}

	i = sizeof(g_dest);
	if((*nBytes = recvfrom(gdnssocket, (char*)buffer, MAX_BUF_LEN, 0,
		(struct sockaddr*)&g_dest, (socklen_t*)&i)) < 0)
	{
		debug_print("recvfrom failed: %s\n", strerror(errno));
		close(gdnssocket);
		return -1;
	}
	/* Ensure buffer is properly terminated */
	buffer[*nBytes+1] = '\0';

	/* Verify the response message */
	if(verify_dns_buffer(buffer, *nBytes) == -1)
	{
		printf("Invalid data from upstream DNS server\n");
		close(gdnssocket);
		return -1;
	}

	/* Close the GDNS socket */
	close(gdnssocket);

	/* ------ this is what is recvd from Upstream DNS ------- */
	dns = (struct DNS_HEADER*)buffer;
	reader = buffer + sizeof(struct DNS_HEADER) + strlen(g_qname)+1 + sizeof(struct QUESTION);

	printf("\n\nResponse Data:\n");
	printf("------------------\n");
	printf("\nThe response contains : ");
	printf("\n %d Questions",ntohs(dns->q_count));
	printf("\n %d Answers",ntohs(dns->ans_count));
	printf("\n %d Authoritative Servers",ntohs(dns->auth_count));
	printf("\n %d Additional records\n",ntohs(dns->add_count));

	stop=0;
	for(i=0;i<ntohs(dns->ans_count);i++)
	{
		answers[i].name = uncompress_name(reader,buffer,&stop);
		reader = reader + stop;

		answers[i].resource = (struct R_DATA*)(reader);
		reader = reader + sizeof(struct R_DATA);

		if(ntohs(answers[i].resource->type) == 1) /* if its an ipv4 address */
		{
			answers[i].rdata = (unsigned char*)malloc(ntohs(answers[i].resource->data_len));

			for(j=0 ; j<ntohs(answers[i].resource->data_len) ; j++)
			{
				answers[i].rdata[j]=reader[j];
			}

			answers[i].rdata[ntohs(answers[i].resource->data_len)] = '\0';
			reader = reader + ntohs(answers[i].resource->data_len);
		}
		else
		{
			answers[i].rdata = uncompress_name(reader,buffer,&stop);
			reader = reader + stop;
		}
	}

	printf("Answer Records : %d \n" , ntohs(dns->ans_count) );
	for(i=0 ; i < ntohs(dns->ans_count) ; i++)
	{
		printf("Name : %s ",answers[i].name);
		if(ntohs(answers[i].resource->type) == T_A) /* IPv4 address */
		{
			p=(long*)answers[i].rdata;
			a.sin_addr.s_addr=ntohl(*p);
			printf("has IPv4 address : %s",inet_ntoa(a.sin_addr));
		}
		printf("\n");
	}
	for(i=0 ; i < ntohs(dns->ans_count) ; i++)
	{
		free(answers[i].name);
		free(answers[i].rdata);
	}
	return 0;
}

/*
 * DNS compression scheme for multiple occurences of domain name
 * - if google.com appears 10 times en for the first time it is
 *  written as google.com and after that for other references a
 *  pointer is used for every next occurence of google.com
 * eg: if www.google.com is writen at offset of 12 and *reader
 *  contains another say, ns.google.com, then this is represented
 *  as ns.16
 *  2 bytes are used where first 2 bits are set to 1 and the rest
 *  14 are used as offset. So for 16 offset 
 *  1100 0000 0000 0000 + 1000
 *  1100 0000 0000 0000 is 49152 
 */
u_char* uncompress_name(unsigned char* reader,unsigned char* buffer,int* count)
{
	unsigned char *name;
	unsigned int p=0,jumped=0,offset;
	int i, j;

	*count = 1;
	name = (unsigned char*)malloc(MAX_NAME_LEN);

	name[0]='\0';

	while(*reader!=0)
	{
		/*
		 * Decoding DNS with multiple names in response 
		 */
		if(*reader >= 192)
		{
			offset = (*reader)*MAX_NAME_LEN + *(reader+1) - 49152;
			reader = buffer + offset - 1;
			jumped = 1;
		}
		else
		{
			name[p++]=*reader;
		}
		reader = reader+1;
		if(jumped==0)
		{
			*count = *count + 1;
		}
	}

	name[p]='\0';
	if(jumped==1)
	{
		*count = *count + 1;
	}

	for(i=0;i<(int)strlen((const char*)name);i++) 
	{
		p=name[i];
		for(j=0;j<(int)p;j++) 
		{
			name[i]=name[i+1];
			i=i+1;
		}
		name[i]='.';
	}
	name[i-1]='\0';
	return name;
}

int verify_dns_buffer(unsigned char *buffer, unsigned int buflen) {
	int i;
	struct QUESTION *qinfo;
	unsigned char *qname;
	char domain_name[MAX_NAME_LEN];
	struct DNS_HEADER *dns;

	qname = buffer + sizeof(struct DNS_HEADER);

	/* Ensure that tid length is correct */
	dns = (struct DNS_HEADER *)buffer;
	if(sizeof(ntohs(dns->id))*2 > TID_SIZE) {
		debug_print("Rogue packet\n");
		return -1;
	}

	/* Verify that qname ends in a '\0' */
	i = 0;
	memset(domain_name, '\0', MAX_NAME_LEN);

	while(*qname!= '\0')
	{
		domain_name[i] = *qname;
		if(i > buflen)
		{
			debug_print("Rogue packet\n");
			return -1;
		}
		qname++, i++;
	}
	domain_name[i] = '\0';

	/*
	 * Only T_A type supported in this version
	 */
	qinfo =(struct QUESTION*)(buffer + sizeof(struct DNS_HEADER) + (strlen(domain_name) + 1));
	if(ntohs(qinfo->qtype) != T_A) {
		debug_print("Unsupported query type\n");
		return -1;
	}
	return 0;
}
