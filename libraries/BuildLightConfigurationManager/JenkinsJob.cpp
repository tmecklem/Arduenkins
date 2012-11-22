#include "JenkinsJob.h"
#include <string.h>
#include <stdlib.h>

void JenkinsJob::initializeJob(){
  for(int i = 0 ; i < MAX_LOCATIONS_PER_LINE ; i++){
    m_jobLocations[i] = NULL;
  }
  m_ip = (uint8_t *)malloc(sizeof(uint8_t) * 4);
  for(int i = 0 ; i < 4 ; i++){
    m_ip[i] = 0;
  }
  m_port = 0;
  m_numJobLocations = 0;
}

void JenkinsJob::freeMemory() {
  free(m_ip);
  for(int i = 0 ; i < MAX_LOCATIONS_PER_LINE ; i++){
	free(m_jobLocations[i]);
    m_jobLocations[i] = NULL;
  }
}

void JenkinsJob::setServer(uint8_t ip[4], uint16_t port) {
  memcpy(m_ip, ip, sizeof(uint8_t)*4);
  m_port = port;
}

void JenkinsJob::addJobLocation(const char *jobLocation) {
  if(m_numJobLocations == MAX_LOCATIONS_PER_LINE){
    return;
  }
  uint8_t length = strlen(jobLocation);
  m_jobLocations[m_numJobLocations] = (char *)malloc(sizeof(char)*length+1);
  strncpy(m_jobLocations[m_numJobLocations], jobLocation, length);
  m_jobLocations[m_numJobLocations][length] = NULL;
  Serial.print(F("Added job location "));
  Serial.print(m_jobLocations[m_numJobLocations]);
  Serial.print(F(" to position "));
  Serial.println(m_numJobLocations);
  
  m_numJobLocations++;
}

void JenkinsJob::printJob() {
  char buffer[16] = {NULL};
  printIp(m_ip, buffer);
  Serial.print(buffer);
  Serial.print(F(":"));
  Serial.print(m_port);
  for(int i = 0 ; i < MAX_LOCATIONS_PER_LINE ; i++){
    Serial.print(m_jobLocations[i]);
  }
  Serial.println();
}

