#include "ft_ping.h"

double get_current_time_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)(tv.tv_sec * 1000.0) + (tv.tv_usec / 1000.0);
}

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

struct sockaddr_in resolve_address(const char *host, int verbose) {
    struct addrinfo hints, *res;
    struct sockaddr_in addr;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    if (getaddrinfo(host, NULL, &hints, &res) != 0) {
        dprintf(2, "ping: %s: Temporary failure in name resolution\n", host);
        exit(EXIT_FAILURE);
    }
    (void)verbose;
    memcpy(&addr, res->ai_addr, sizeof(addr));
    freeaddrinfo(res);
    return addr;
}

unsigned short checksum(void *b, int len) {
    unsigned short *buf = b;
    unsigned int sum = 0;
    unsigned short result;

    for (sum = 0; len > 1; len -= 2)
        sum += *buf++;
    if (len == 1)
        sum += *(unsigned char *)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}


void send_ping(int sockfd, struct sockaddr_in *addr, unsigned short sequence, double *start_time) {
    char packet[64];
    struct icmphdr *icmp = (struct icmphdr *)packet;

    memset(packet, 0, sizeof(packet));
    icmp->type = ICMP_ECHO;
    icmp->code = 0;
    icmp->checksum = 0;
    icmp->un.echo.id = getpid();
    icmp->un.echo.sequence = sequence;

    icmp->checksum = checksum(packet, sizeof(packet));

    *start_time = get_current_time_ms(); 

    if (sendto(sockfd, packet, sizeof(packet), 0, (struct sockaddr *)addr, sizeof(*addr)) <= 0) {
        perror("sendto");
        exit(EXIT_FAILURE);
    }
}



void receive_ping(int sockfd, double start_time, unsigned short sequence, int verbose) {
    char buffer[1024];
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    long end_time;
    struct iphdr *ip;
    struct icmphdr *icmp;

    int bytes = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&addr, &addr_len);
    if (bytes <= 0) {
        perror("recvfrom");
        exit(EXIT_FAILURE);
    }

    end_time = get_current_time_ms();

    ip = (struct iphdr *)buffer;
    icmp = (struct icmphdr *)(buffer + (ip->ihl * 4));
    if (icmp->type == ICMP_ECHOREPLY) {
        double rtt = end_time - start_time;
        printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%.1f ms\n",
               bytes - (ip->ihl * 4), inet_ntoa(addr.sin_addr), sequence, ip->ttl, rtt);
    } else if (verbose) {
        printf("Received unexpected ICMP packet: type=%d, code=%d\n", icmp->type, icmp->code);
    }
}



int main(int argc, char **argv) {
    if (argc < 2 || argc > 5) {
        return dprintf(2, "Error: Wrong Number of Arguments\n");
    }

    t_ping ping;

    if (check_input(argc, argv, &ping)) {
        return 1; 
    }

    if (ping.help) {
        printf("Usage: ping [options] <destination>\n");
        printf("Options:\n");
        printf("  -v     Enable verbose output\n");
        printf("  -?     Display this help message\n");
        return 0;
    }
    ping.addr = resolve_address(ping.target, ping.verbose);

    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    if (ping.verbose) {
        printf("sock4.fd: %d (socktype: SOCK_RAW), hints.ai_family: AF_UNSPEC\n\n", sockfd);
        printf("ai->ai_family: AF_INET, ai->ai_canonname: '%s'\n", ping.target);
    }
    printf("PING %s (%s) 56(84) bytes of data.\n", ping.target, inet_ntoa(ping.addr.sin_addr));

    unsigned short sequence = 1;
    while (1) { 
        double start_time;
        send_ping(sockfd, &ping.addr, sequence, &start_time);
        receive_ping(sockfd, start_time, sequence, ping.verbose);
        sequence++;
        sleep(1);
    }
    close(sockfd);
    return 0;
}
