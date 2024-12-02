#ifndef FT_PING_H
# define FT_PING_H


# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <netinet/ip.h>
# include <netinet/ip_icmp.h>
# include <arpa/inet.h>
# include <unistd.h>
# include <netdb.h>

typedef struct s_ping {
    char *command;
    int verbose;
    int help;
    char *target;
    struct sockaddr_in addr;    
} t_ping;

#endif