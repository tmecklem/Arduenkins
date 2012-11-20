#ifndef JenkinsClient_h
#define JenkinsClient_h

#include "Arduino.h"
#include "Ethernet.h"

#define MAX_CONFIG_LINE_LENGTH 100
#define MAX_SUPPORTED_JOBS 5

typedef struct JenkinsJob {
  uint8_t ip[4];
  uint16_t port;
  char jobUrl[MAX_CONFIG_LINE_LENGTH+1];
  JenkinsJob* linkedJob;
} JenkinsJob;

class JenkinsClient
{
public:
  JenkinsClient();
  JenkinsClient(uint8_t ip[], uint16_t port, EthernetClient *client, char *configurationLocation);
  /* returns 1 if failure to get connection */
  int getStatusForProject(int jobNumber, char *statusBuffer);
  int initializeConfiguration();
  
private:
  int parseConfig(char *config); //parses a config line
  int parseJob(char *config, JenkinsJob &job); //parses a job and populates a JenkinsJob
  JenkinsJob *createJob();
  void clearJob(JenkinsJob *job);
  void resetJobs();
  
  EthernetClient *_client;
  uint8_t _ip[4];
  uint16_t _port;
  char *_configurationLocation;
  
  JenkinsJob * _jobs[MAX_SUPPORTED_JOBS];
  int _numConfiguredJobs;
};

#endif
