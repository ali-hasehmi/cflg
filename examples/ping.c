#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define PROGRAM_NAME "ping"
#define CFLG_DEBUG
#define CFLG_IMPLEMENTATION
#include "../cflg.h"

#define btoa(x) ((x) ? "true" : "false")

int
main(int argc, char * argv[]) {

	int64_t count = -1;
	bool ipv4_only = false;
	bool ipv6_only = false;
    bool precision   = false;
	float interval = 1;
	char *interface = "eth0";
	int mark = 0x0;
    uint ident = rand() % UINT16_MAX;
    double timeout = 0.2; // => 200 ms

	cflg_flgset_t fset;
	cflg_flgset_create(&fset);

	cflg_flgset_int64(&fset,&count, 'c', "count",  "NUM", "number of packets to send");
	cflg_flgset_int(&fset,&mark, 'm', "mark", "MARK", "set fwmark");
	cflg_flgset_bool(&fset,&precision, '3', NULL,"RTT precision do not round up");
	cflg_flgset_bool(&fset,&ipv4_only, '4', NULL,"use only ipv4");
	cflg_flgset_bool(&fset,&ipv6_only, '6', NULL, "use only ipv6");
	cflg_flgset_uint(&fset,&ident, 0, "identifier","NUM", "set the identifier");
	cflg_flgset_float(&fset,&interval, 'i', NULL, "TIME", "send packets with this interval");
	cflg_flgset_double(&fset,&timeout, 'W', "timeout", NULL, "timeout for waiting for a packet");
	cflg_flgset_string(&fset,&interface, 'I', "interface", NULL, "mend packets through this interface");

	cflg_flagset_parse(&fset, argc, argv);

	printf (
        "count: %ld\n"
        "with precision: %s\n"
		"use only ipv4: %s\n"
	 	"use only ipv6: %s\n"
        "identifier: %u\n"
        "timeout: %lf\n"
	 	"interval: %f\n"
	 	"interface: %s\n"
	 	"mark: %d\n",
	 	count,
        btoa(precision),
	 	btoa(ipv4_only),
	 	btoa(ipv6_only),
        ident,
        timeout,
	 	interval,
	 	interface,
	 	mark);

	 
	return 0;
}
