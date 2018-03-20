/*
 * A simple DNS server using libevent
 * Features:
 * - Supports Type A record query as of now
 * - Based on libevent
 * Todo:
 * - Log support
 *
 */
#include "utils.h"


/*
 * Entry point for DNS server
 * - Makes use of UDP sockets
 */
int main()
{
	int udpSocket;
	struct sockaddr_in serverAddr;
	struct event ev_receive;
	int reuseaddr_on;
	/* The libevent event base */
	struct event_base *evbase;

	/* Initialize libevent. */
	evbase = event_base_new();

	/*Create UDP socket*/
	struct addrinfo hints, *res, *rp;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype =  SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;

	if(getaddrinfo(NULL, LISTEN_PORT, &hints, &res) != 0) {
		debug_print("Failed to getaddrinfo: %s\n", strerror(errno));
		return -1;
	}
	/* getaddrinfo returns a list of sockets, use any one of them */
	for (rp = res; rp != NULL; rp = rp->ai_next) {
		udpSocket = socket(rp->ai_family, rp->ai_socktype,
				rp->ai_protocol);
		if (udpSocket == -1)
			continue;
		if (bind(udpSocket, rp->ai_addr, rp->ai_addrlen) == 0)
			break; /* Success */
		close(udpSocket);
	}

	if (rp == NULL) { /* No address succeeded */
		debug_print("Could not bind\n");
		return -1;
	}
	freeaddrinfo(res);

	reuseaddr_on = 1;
	if (setsockopt(udpSocket, SOL_SOCKET, SO_REUSEADDR, &reuseaddr_on, 
		sizeof(reuseaddr_on)) != 0)
	{
		debug_print("Failed to set UDP socket as non-blocking\n");
		close(udpSocket);
		return -1;
	}

	/* create a read event to
	 * be notified when reading from a client */
	event_assign(&ev_receive, evbase, udpSocket, EV_READ|EV_PERSIST, 
		on_recv, NULL);

	event_add(&ev_receive, NULL);

	/* Start the event loop. */
	event_base_dispatch(evbase);
	close(udpSocket);
	event_base_free(evbase);
	return 0;
}

/* Main callback function for libevent:
 * - Completes its tasks in the following steps:
 *    1. Reads the contents of request from a DNS client
 *    2. Parses the request to get TID, client IP, request type and domain name
 *    3. Creates a DNS request for Upstream DNS server and sends request
 *    4. Analyses the response to read TID, client IP, request type and domain name
 *    5. Forwards this response to the DNS client
 */
void on_recv(int udpSocket, short ev, void *arg)
{
	unsigned int nBytes;
	unsigned char *buffer;
	struct sockaddr_in *clientAddr;
	struct sockaddr_storage serverStorage;
	unsigned char *qname;

	socklen_t addr_size;
	qr_data_t client_data;

	/*
	 * Step 1: Read DNS request from client
	 * - Read the transaction ID, TID
	 * - Client IP address
	 * - Domain name
	 * - query type
	 */

	buffer = malloc(MAX_BUF_LEN);
	memset(buffer, '\0', sizeof(buffer));

	/* This is the incoming DNS query from client */
	addr_size = sizeof(serverStorage);
	if ((nBytes = recvfrom(udpSocket, buffer, MAX_BUF_LEN, 0,
		(struct sockaddr *)&serverStorage, (socklen_t*)&addr_size)) < 0)
	{
		debug_print("Failed to read DNS request: %s\n", strerror(errno));
		free(buffer);
		return;
	}

	/* Ensure buffer is properly terminated */
	buffer[nBytes+1] = '\0';

	/* Verify input buffer read */
	if(verify_dns_buffer(buffer, nBytes) != 0) {
		printf("Unsupported input, ignoring!\n");
		free(buffer);
		return;
	}

	printf("\n\nRequest data:\n");
	printf("------------------\n");
	/* Read client IP address from the UDP recvfrom sockaddr structure */
	clientAddr = (struct sockaddr_in *)&serverStorage;

	/* Read and print client details */
	if(read_and_print_dns_header(buffer, nBytes, &client_data, clientAddr) != 0)
	{
		debug_print("Failed to read Client DNS details\n");
		free(buffer);
		return;
	}
	/*
	 * The qname in the DNS query needs to be in the format of domain name
	 * Updating the qname in buffer with client_data.domain_name
	 */
	qname = (unsigned char*)buffer + sizeof(struct DNS_HEADER);
	strncpy(qname, client_data.domain_name, strlen(client_data.domain_name));
	*(qname+strlen(client_data.domain_name)) = '\0';


	/*
	 * Step 2: Forward request to Upstream DNS
	 * - Ensure TID is same
	 * - put domain name in request
	 * - check questions in buffer
	 */
	if(send_data_to_dns(buffer, &nBytes) != 0)
	{
		debug_print("Failed to send query to upstream DNS\n");
		free(buffer);
		return;
	}

	/*
	 * Step 3: Forward the response recieved from
	 * GDNS to the client
	 * Copy the buffer read from GDNS response
	 * to the response to be sent to client
	 */

	/* Read and print response details */
	if(read_and_print_dns_header(buffer, nBytes, &client_data, clientAddr) != 0)
	{
		debug_print("Failed to read Client DNS details: %s\n", strerror(errno));
		free(buffer);
		return;
	}

	/* Send back to the client */
	if(sendto(udpSocket, (char *)buffer, nBytes, 0, (struct sockaddr*)&serverStorage, sizeof(serverStorage)) < 0)
	{
		debug_print("Failed to send back data to client :%s\n", strerror(errno));
	}

	free(buffer);
	return;
}
