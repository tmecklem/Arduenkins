#ifndef BuildLightConfigurationManager_h
#define BuildLightConfigurationManager_h

#include "Arduino.h"
#include "JenkinsJob.h"
#include "JenkinsClient.h"
#include "config.h"
#include "utility.h"

#define MAX_SUPPORTED_JOBS 5

class BuildLightConfigurationManager
{
public:
  BuildLightConfigurationManager();
  BuildLightConfigurationManager(uint8_t ip[], uint16_t port, EthernetClient *client, char *configurationLocation);
  int initializeConfiguration();
  uint16_t getStatusForProject(int jobNumber);
  
private:
  int parseConfig(char *config); //parses a config line
  int parseJob(char *config, JenkinsJob *job); //parses a job and populates a JenkinsJob
  void resetJobs();

  JenkinsClient *m_jenkinsClient;
  EthernetClient *_client;
  uint8_t _ip[4];
  uint16_t _port;
  char *_configurationLocation;
  
  JenkinsJob * _jobs[MAX_SUPPORTED_JOBS];
  int _numConfiguredJobs;
};

#endif
