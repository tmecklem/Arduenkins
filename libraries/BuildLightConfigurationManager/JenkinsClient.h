#ifndef JenkinsClient_h
#define JenkinsClient_h

#include "Arduino.h"
#include "Ethernet.h"
#include "JenkinsJob.h"
#include "config.h"
#include "utility.h"

class JenkinsClient
{
public:
  uint16_t getStatusForJob(JenkinsJob * job, EthernetClient * client);
  
private:
  uint16_t getStatusForLocation(uint8_t ip[], uint16_t port, char *location, EthernetClient *client);
};

#endif
