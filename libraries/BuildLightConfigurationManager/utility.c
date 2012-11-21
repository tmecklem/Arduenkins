#include "utility.h"

void printIp(uint8_t ip[], char *buffer) {
  sprintf(buffer, "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
}