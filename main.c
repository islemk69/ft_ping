#include "ft_ping.h"

int valid_command(const char *cmd) {
  return strncmp(cmd, "ping", 5) == 0;
}

int valid_parameter(const char *param) {
  return (strncmp(param, "-v", 3) == 0 || strncmp(param, "-?", 3) == 0);
}

int main(int argc, char **argv) {
  if (argc > 3 || argc == 1)
    return (dprintf(2, "Error: Wrong Number fo Arguments.\n"));
  if (!valid_command(argv[1]))
    return (dprintf(2, "Error: Wrong Command\n"));
  if (argv[2] && !valid_parameter(argv[2]))
    return (dprintf(2, "Error: Wrong Parameter\n"));
}