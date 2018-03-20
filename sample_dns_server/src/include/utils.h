#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/queue.h>
#include <unistd.h>
/* Libevent. */
#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

#define T_A 						1 /* Ipv4 address */
#define MAX_NAME_LEN 				256
#define MAX_BUF_LEN 				512
#define RECORD_LEN 					20
#define TID_SIZE					16

#define LISTEN_PORT 				"53053"
#define UPSTREAM_DNS_SERVER			"8.8.8.8"
#define UPSTREAM_DNS_PORT			53
#define UPSTREAM_DNS_SOCKET_TIMEOUT 5 /* 5 seconds */

/* The structure of a DNS record after header
 *    *************************************
 *    |         32-bit header             |
 *    *************************************
 *    | Question Count   |  Answer Count  |
 *    *************************************
 *    | Auth Resource Record | Add RR     |
 *    *************************************
 *    |           Questions               |
 *    *************************************
 *    |           Answer RRs              |
 *    *************************************
 *    |           Authority RRs           |
 *    *************************************
 *    |           Additional RRs          |
 *    *************************************
 *        -- RR: Resource Record
 */
/*
 * 32- DNS header
 * with fields as explained below
 */
struct DNS_HEADER
{
    unsigned short id; /* identification number */
 
    unsigned char rd :1; /* recursion desired */
    unsigned char tc :1; /* truncated message */
    unsigned char aa :1; /* authoritive answer */
    unsigned char opcode :4; /* purpose of message */
    unsigned char qr :1; /* query/response flag */
 
    unsigned char rcode :4; /* response code */
    unsigned char cd :1; /* checking disabled */
    unsigned char ad :1; /* authenticated data */
    unsigned char z :1; /* reserved */
    unsigned char ra :1; /* recursion available */
 
    unsigned short q_count; /* number of question entries */
    unsigned short ans_count; /* number of answer entries */
    unsigned short auth_count; /* number of authority entries */
    unsigned short add_count; /* number of resource entries */
};

/*
 * Constant sized fields of query structure
 */
struct QUESTION
{
    unsigned short qtype;
    unsigned short qclass;
};
 
/*
 * Constant sized fields of the resource record structure
 */
#pragma pack(push, 1)
struct R_DATA
{
    unsigned short type;
    unsigned short _class;
    unsigned int ttl;
    unsigned short data_len;
};
#pragma pack(pop)
 
/*
 * Pointers to resource record contents
 */
struct RES_RECORD
{
    unsigned char *name;
    struct R_DATA *resource;
    unsigned char *rdata;
};
 
/*
 * Structure of a Query
 */
typedef struct
{
    unsigned char *name;
    struct QUESTION *ques;
} QUERY;

/* Structure for Request/Reponse DNS header */
typedef enum query_type_t {
  REQUEST=0,
  RESPONSE
} query_type_t;

typedef struct qr_data {
  int tid;
  char domain_name[MAX_NAME_LEN]; 
  query_type_t q_type;
} qr_data_t;

/* 
 * Utility functions declaration
 */
int ChangetoDnsNameFormat(unsigned char*, unsigned char*);
u_char* uncompress_name(unsigned char*, unsigned char*, int*);
void on_recv(int, short, void *);
int read_and_print_dns_header(unsigned char *, unsigned int, qr_data_t *, struct sockaddr_in *);
int send_data_to_dns(unsigned char *, unsigned int *);
int verify_dns_buffer(unsigned char *, unsigned int);
int verify_and_copy_qname(char *, char *, unsigned int);
 
/* DEBUG print */
#define debug_print(fmt, args...) \
	do { if (DEBUG) fprintf(stderr, "DEBUG: "fmt, ##args); } while (0)
#endif /* _UTILS_H_ */
