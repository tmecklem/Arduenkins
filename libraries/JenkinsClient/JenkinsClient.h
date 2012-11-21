#ifndef JenkinsClient_h
#define JenkinsClient_h

#include "Arduino.h"
#include "Ethernet.h"

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

typedef struct JenkinsJob {
  uint8_t ip[4];
  uint16_t port;
  char *jobLocations[MAX_LOCATIONS_PER_LINE];
} JenkinsJob;

class JenkinsClient
{
public:
  JenkinsClient();
  JenkinsClient(uint8_t ip[], uint16_t port, EthernetClient *client, char *configurationLocation);
  uint16_t getStatusForProject(int jobNumber);
  int initializeConfiguration();
  
private:
  int parseConfig(char *config); //parses a config line
  int parseJob(char *config, JenkinsJob *job); //parses a job and populates a JenkinsJob
  JenkinsJob *createJob();
  void clearJob(JenkinsJob *job);
  void resetJobs();
  void printIp(uint8_t ip[]);

  EthernetClient *_client;
  uint8_t _ip[4];
  uint16_t _port;
  char *_configurationLocation;
  
  JenkinsJob * _jobs[MAX_SUPPORTED_JOBS];
  int _numConfiguredJobs;
};

#endif
