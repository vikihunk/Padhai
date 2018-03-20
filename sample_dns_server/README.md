Simple DNS resolver designed using libevent.

--------
FEATURES
--------

This utility has following features:
1. It supports only A type records, in this version.
2. It is designed using asynchronous events library, libevent.
3. The utility listens on UDP port 53053.
4. It has been tested with dig utility.

The DNS resolver utility uses Google DNS upstream server (8.8.8.8) to forward DNS queries. 
On receiving the query from DNS client, it parses the header and message to print details of the client, viz.:

Client address,
Trsansaction id,
Domain name,
Query type

The above details are printed on the utility console for both request and response messages.

It also parses the response from Upstream DNS server and prints the Answer records (IP(s)) for the queried domain name.

--------------------
Directory structure
--------------------

Top dir:
sample_dns_server

Sub dir:

src: Consists of two sub directories

src/dns_server: The DNS resolver utility code in C

src/dns_utils: Utilities used by the DNS resolver

src/include: Header files, data structures and function declarations

Directories created on build:

obj: dns_utils.o

bin: server (DNS resolver utility)

Files created:

obj/dns_utils.o: Object files which has utility functions

bin/server: DNS resolver utility binary

----------------
Execution steps:
----------------

1. This utility is based on libevent, so it is expected that libevent is installed on the target system.
2. Build the utility using make from top directory.
3. Make creates an dns_utils.o in obj directory, which is set of functions used by the utility.
4. Server binary is created under bin path.
5. Ensure libevent.so is pointed to in the dynamic linker path (env var: LD_LIBRARY_PATH)
6. Start DNS resolver utility from command line:
	./bin/server
7. Issue command from another terminal using dig utility, eg:
	dig @127.0.0.1 -p 53053 www.udacity.com

-------------
Known issues:
-------------

1. The DNS utility is working consistently with 200 concurrent client connections, as tested with shell script running multiple dig commands in background (and hence concurrent).

However, "connection timed out;no servers could be reached" error can be seen at times, at the client end, when the number of client connections are increased beyond 500.

-------------------------------------
Script to test concurrent connections
-------------------------------------

Test setup:
The utility was tested by running it on Ubuntu (ver: 16.04.2 LTS) VirtualBox and running DNS client (dig utility) from the host OS (Mac version: 10.13.3).
The same was also tested using loopback address from the same VirtualBox.
 
Script that was used to test concurrent connections:
(The IP used to test on MAC was different from as mentioned in the script below.
 IP used was after enabling Host-only adapter on VirtualBox and using the IP as shown for the Host-only adapter interface).

#!/bin/sh

i=1

while [ $i -le 500 ]

do

  dig @127.0.0.1 -p 53053 www.udacity.com & #positive case

  dig @127.0.0.1 -p 53053 www.waste.com & #negative case

  i=$(( i+1 ))

done
"
