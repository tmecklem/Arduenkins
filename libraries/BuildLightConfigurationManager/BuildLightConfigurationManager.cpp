#include "BuildLightConfigurationManager.h"
#include <string.h>
#include <stdlib.h>

#define JENKINS_POST_JOB_URL "/api/json?tree=color"
#define LINK_JOBS 2
#define DEBUG_JENKINS_CLIENT //Uncomment to print debug statements over serial

BuildLightConfigurationManager::BuildLightConfigurationManager() {
  uint8_t server[] = {192,168,0,1};
}

BuildLightConfigurationManager::BuildLightConfigurationManager(uint8_t configIp[], uint16_t configPort, EthernetClient *client, char *configurationLocation) {
  _ip[0] = configIp[0];
  _ip[1] = configIp[1];
  _ip[2] = configIp[2];
  _ip[3] = configIp[3];
  
  _port = configPort;
  
  Serial.print(F("Config IP saved as "));
  printIp(_ip);
  Serial.println();
  
  _client = client;
  _configurationLocation = configurationLocation;
  
  m_jenkinsClient = (JenkinsClient *)malloc(sizeof(JenkinsClient));
}

uint16_t BuildLightConfigurationManager::getStatusForProject(int jobNumber) {
  return m_jenkinsClient->getStatusForJob(_jobs[jobNumber], _client);
}

//0 is fail, 1 is success, 2 is point previous job to this one
int BuildLightConfigurationManager::parseJob(char *jobConfig, JenkinsJob *job) {
  int switchToCommas = 0;
  int returnCode = 1;
  char *localJobConfigPtr = jobConfig;
#ifdef DEBUG_JENKINS_CLIENT  
  Serial.print(F("Job config string is "));
  Serial.println(localJobConfigPtr);
#endif

  uint8_t ip[4] = {0};
  uint16_t port = 0;

  char *token;
  char *end_str;
  token = strsep (&localJobConfigPtr,".");
  int tokenIndex = 0;
  while (token != NULL){
#ifdef DEBUG_JENKINS_CLIENT    
    Serial.print(F("Token "));
    Serial.print(token);
    Serial.print(F(" found at index "));
    Serial.println(tokenIndex);
    Serial.print(F("Remaining string is "));
    Serial.println(localJobConfigPtr);
#endif
	if(tokenIndex <= 3){
	  ip[tokenIndex] = atoi(token);
	  if(tokenIndex == 2) {
#ifdef DEBUG_JENKINS_CLIENT  
        Serial.println(F("Changing token delimiter to ','"));
#endif
	    switchToCommas = 1;
	  }
	} else if (tokenIndex == 4){
	  port = atol(token);
	  job->setServer(ip, port);
	} else if(tokenIndex >= 5 && (tokenIndex - 5) < MAX_LOCATIONS_PER_LINE){
	  int locationIndex = tokenIndex - 5;
	  int lengthOfLocation = strlen(token);
	  job->addJobLocation(token);
	  returnCode = 0;
	}
	
	token = strsep(&localJobConfigPtr, switchToCommas?",":".");
	tokenIndex++;
  }
  
#ifdef DEBUG_JENKINS_CLIENT  
  Serial.print(F("Job configuration saved as "));
  job->printJob();
#endif
  
  return returnCode;
}

int BuildLightConfigurationManager::parseConfig(char *config) {
 return 0;
}

void BuildLightConfigurationManager::resetJobs(){
  for(int i = 0 ; i < _numConfiguredJobs ; i++) {
    if(_jobs[i] != NULL){
      _jobs[i]->freeMemory();
      free(_jobs[i]);
    }
    _jobs[i] = NULL;
  }
  _numConfiguredJobs = 0;
}

int BuildLightConfigurationManager::initializeConfiguration(){
  
  resetJobs();

#ifdef DEBUG_JENKINS_CLIENT  
  Serial.print(F("Requesting configuration from http://"));
  printIp(_ip);
  Serial.print(F(":"));
  Serial.print(_port);
  Serial.println(_configurationLocation);
#endif
  
  if (_client->connect(_ip, _port)) {
#ifdef DEBUG_JENKINS_CLIENT  
    Serial.print(F("connected\n"));;
    // Make a HTTP request:
    Serial.print(F("GET "));
    Serial.print(_configurationLocation);
#endif

    _client->print("GET ");
    _client->print(_configurationLocation);
    _client->println();
  } 
  else {
    // if you didn't get a connection to the server:
    Serial.print(F("connection failed\n"));
    _client->stop();
    return -1;
  }
  
  
  while (!_client->available()) {
    //wait
  }
  
  char * configResponse = (char *)malloc(sizeof(char)*MAX_CONFIG_LINE_LENGTH+1);
  configResponse[MAX_CONFIG_LINE_LENGTH] = NULL;
  int position = 0;
  
  for(;;){
  	
  	if (_client->available()) {
      char nextChar = _client->read();
      if(nextChar == '\n'){
        configResponse[position++] = NULL;
#ifdef DEBUG_JENKINS_CLIENT  
        Serial.print(F("Found a job configuration: "));
        Serial.println(configResponse);
#endif
    
        JenkinsJob *job = (JenkinsJob *)malloc(sizeof(JenkinsJob));
        job->initializeJob();
        int status = parseJob(configResponse, job);
#ifdef DEBUG_JENKINS_CLIENT  
        Serial.print(F("parseJob return status was "));
        Serial.println(status);
#endif
        if(status == 0){
#ifdef DEBUG_JENKINS_CLIENT  
          Serial.print(F("Adding job"));
#endif
         _jobs[_numConfiguredJobs++] = job;
        } else {
#ifdef DEBUG_JENKINS_CLIENT  
          Serial.println(F("Something went wrong, cleaning up job allocation"));
#endif
          job->freeMemory();
          free(job);
        }
        position = 0;
      } else {
      	configResponse[position++] = nextChar;
      }
    }
  	
  	if (!_client->connected()) {
  	  break;
  	}
  }
  _client->flush();
  free(configResponse);

#ifdef DEBUG_JENKINS_CLIENT  
  Serial.println();
  Serial.print(F("disconnecting.\n"));
#endif
  _client->stop();
  
  Serial.print(F("Jobs Configured: "));
  Serial.println(_numConfiguredJobs);
  return _numConfiguredJobs;
}

void BuildLightConfigurationManager::printIp(uint8_t ip[]) {
  for(int i = 0 ; i < 4 ; i++){
    Serial.print(ip[i]);
    if(i<3) { Serial.print(F(".")); }
  }
}

