#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define CFLG_IMPLEMENTATION
#include "../cflg.h"

#define btoa(x) ((x) ? "true" : "false")

int main(int argc, char *argv[]) {
    // --- Variable Definitions (Grouped by Type) ---
    // Booleans
    bool verbose = false;
    bool quiet = false;
    bool audible = false;
    bool timestamps = false;
    bool precision = false;
    bool ipv4_only = false;
    bool ipv6_only = false;

    // Integers (int64, int, uint)
    int64_t count = -1;
    int deadline = 0;
    int ttl = 0;
    int mark = 0;
    unsigned int packet_size = 56;
    unsigned int ident = rand() % UINT16_MAX;

    // Floating-Point (float, double)
    float interval = 1.0f;
    double timeout = 0.0;

    // Strings
    char *interface = NULL;
    
    // --- Flag Set Initialization ---
    flgset_t fset = {0};

    // --- Flag Definitions (Grouped by Type) ---

    // Boolean Flags
    flgset_bool(&fset, &verbose, 'v', "verbose", "Verbose output.");
    flgset_bool(&fset, &quiet, 'q', "quiet", "Quiet output.");
    flgset_bool(&fset, &audible, 'a', NULL, "Audible ping (beep on success).");
    flgset_bool(&fset, &timestamps, 'D', NULL, "Print timestamp before each line.");
    flgset_bool(&fset, &precision, '3', NULL, "RTT precision, do not round (legacy).");
    flgset_bool(&fset, &ipv4_only, '4', NULL, "Use IPv4 only.");
    flgset_bool(&fset, &ipv6_only, '6', NULL, "Use IPv6 only.");

    // Integer Flags
    flgset_int64(&fset, &count, 'c', "count", "<NUM>", "Stop after sending <NUM> packets.");
    flgset_int(&fset, &deadline, 'w', "deadline", "<SECS>", "Exit after <SECS> seconds.");
    flgset_int(&fset, &ttl, 't', "ttl", "<NUM>", "Set the IP Time to Live.");
    flgset_int(&fset, &mark, 'm', "mark", "<MARK>", "Set the fwmark on outgoing packets.");
    flgset_uint(&fset, &packet_size, 's', "size", "<NUM>", "Set the packet size in bytes.");
    flgset_uint(&fset, &ident, 0, "identifier", "<NUM>", "Set the identifier (process ID).");

    // Floating-Point Flags
    flgset_float(&fset, &interval, 'i', "interval", "<TIME>", "Wait <TIME> seconds between sending each packet.");
    flgset_double(&fset, &timeout, 'W', "timeout", "<TIME>", "Time to wait for a response, in seconds.");

    // String Flags
    flgset_string(&fset, &interface, 'I', "interface", "<IFACE>", "Send packets through this interface.");

    // --- Parsing ---
    struct timespec start, end;
    timespec_get(&start, TIME_UTC);
    flgset_parse(&fset, argc, argv); 
    timespec_get(&end, TIME_UTC);

    // --- Print Parsed Values ---
    int label_width = 22;

    printf("--- Ping Emulation Results ---\n\n");

    printf("General:\n");
    printf("  %-*s %s\n", label_width, "Verbose:", btoa(verbose));
    printf("  %-*s %s\n", label_width, "Quiet:", btoa(quiet));
    printf("  %-*s %s\n", label_width, "Audible:", btoa(audible));
    printf("  %-*s %ld\n", label_width, "Packet Count:", count);
    printf("\n");

    printf("Timing & Precision:\n");
    printf("  %-*s %.2f seconds\n", label_width, "Interval:", interval);
    printf("  %-*s %.2f seconds\n", label_width, "Timeout:", timeout);
    printf("  %-*s %d seconds\n", label_width, "Deadline:", deadline);
    printf("  %-*s %s\n", label_width, "Timestamps:", btoa(timestamps));
    printf("  %-*s %s\n", label_width, "High Precision (legacy):", btoa(precision));
    printf("\n");
    
    printf("Network & Packet:\n");
    printf("  %-*s %s\n", label_width, "IPv4 Only:", btoa(ipv4_only));
    printf("  %-*s %s\n", label_width, "IPv6 Only:", btoa(ipv6_only));
    printf("  %-*s %s\n", label_width, "Interface:", interface ? interface : "(default)");
    printf("  %-*s %u bytes\n", label_width, "Packet Size:", packet_size);
    printf("  %-*s %d\n", label_width, "Time To Live (TTL):", ttl);
    printf("  %-*s %u\n", label_width, "Identifier (PID):", ident);
    printf("  %-*s %d\n", label_width, "Firewall Mark:", mark);
    printf("\n----------------------------------\n");

    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("Flags parsed in %.9f seconds\n", elapsed);

    return 0;
}
