#ifndef JenkinsJob_h
#define JenkinsJob_h

#include "Arduino.h"

#define MAX_CONFIG_LINE_LENGTH 100
#define MAX_LOCATIONS_PER_LINE 4

#define JOB_INVALID_STATUS 0x00
#define JOB_DISABLED 0x01
#define JOB_SUCCEEDED 0x02
#define JOB_FAILED 0x04
#define JOB_UNSTABLE 0x08
#define JOB_CANCELED 0x10
#define JOB_IN_PROGRESS 0x80

class JenkinsJob {
public:
  void initializeJob();
  void freeMemory(); //frees up any internal memory malloc'ed along the way
  void setServer(uint8_t ip[4], uint16_t port);
  void addJobLocation(const char *jobLocation);
  void printJob();
  void printIp(uint8_t ip[]);
  const uint8_t getIp();
  const uint16_t getPort();
  const char *getJobLocation(int index);

private:
  friend class JenkinsClient;
  uint8_t *m_ip;
  uint16_t m_port;
  char *m_jobLocations[MAX_LOCATIONS_PER_LINE];
  uint8_t m_numJobLocations;
};

#endif
