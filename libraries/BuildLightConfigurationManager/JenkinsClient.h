#ifndef JenkinsClient_h
#define JenkinsClient_h

#include "Arduino.h"
#include "Ethernet.h"
#include "JenkinsJob.h"

#define MAX_CONFIG_LINE_LENGTH 100
#define MAX_SUPPORTED_JOBS 5
#define MAX_LOCATIONS_PER_LINE 4

#define JOB_INVALID_STATUS 0x00
#define JOB_DISABLED 0x01
#define JOB_SUCCEEDED 0x02
#define JOB_FAILED 0x04
#define JOB_UNSTABLE 0x08
#define JOB_CANCELED 0x10
#define JOB_IN_PROGRESS 0x80

class JenkinsClient
{
public:
  uint16_t getStatusForJob(JenkinsJob * job, EthernetClient * client);
  
private:
  void printIp(uint8_t ip[]);
};

#endif
