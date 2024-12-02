#include "ft_ping.h"

int check_input(int argc, char **input, t_ping *ping) {
    memset(ping, 0, sizeof(t_ping));
    if (strncmp(input[1], "ping", 5))
        return dprintf(2, "Error: Invalid command '%s'\n", input[1]);
    if (!input[2])
        return dprintf(2, "ping: usage error: Destination address required\n");
    ping->command = input[1];
    for (int i = 2; i < argc; i++){
        if (!strncmp(input[i], "-v", 3))
            ping->verbose = 1;
        else if (!strncmp(input[i], "-?", 3))
            ping->help = 1;
        else if (input[i][0] == '-')
            return dprintf(2, "Error: Invalid Parameter\n");
        else {
            if (ping->target)
                return dprintf(2, "Error: Multiple targets specified\n");
            else
            ping->target = input[i];
        }
    }
    if (!ping->target)
        return dprintf(2, "ping: usage error: Destination address required\n");
    return 0;
}

struct sockaddr_in resolve_address(const char *host) {
    struct addrinfo hints, *res;
    struct sockaddr_in addr;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;

    if (getaddrinfo(host, NULL, &hints, &res) != 0) {
        dprintf(2, "ping: %s: Temporary failure in name resolution\n", host);
        exit(EXIT_FAILURE);
    }

    memcpy(&addr, res->ai_addr, sizeof(addr));
    freeaddrinfo(res);

    return addr;
}


int main(int argc, char **argv) {
    if (argc < 2 || argc > 5)
        return dprintf(2, "Error: Wrong Number of Arguments\n");
    t_ping ping;
    if (check_input(argc, argv, &ping))
        return 1;
    ping.addr = resolve_address(ping.target);
}