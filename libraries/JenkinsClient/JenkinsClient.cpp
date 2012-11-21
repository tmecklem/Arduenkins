#include "JenkinsClient.h"
#include <string.h>
#include <stdlib.h>

#define JENKINS_POST_JOB_URL "/api/json?tree=color"
#define LINK_JOBS 2
#define DEBUG_JENKINS_CLIENT //Uncomment to print debug statements over serial

JenkinsClient::JenkinsClient() {
  uint8_t server[] = {192,168,0,1};
  JenkinsClient(server, 80, NULL, NULL);
}

JenkinsClient::JenkinsClient(uint8_t configIp[], uint16_t configPort, EthernetClient *client, char *configurationLocation) {
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
}

void JenkinsClient::clearJob(JenkinsJob *job) {
  for(int i = 0 ; i < MAX_LOCATIONS_PER_LINE ; i++){
	free(job->jobLocations[i]);
    job->jobLocations[i] = NULL;
  }

  free(job);
}

JenkinsJob *JenkinsClient::createJob(){
  JenkinsJob *job = (JenkinsJob *)malloc(sizeof(JenkinsJob));
  for(int i = 0 ; i < MAX_LOCATIONS_PER_LINE ; i++){
    job->jobLocations[i] = NULL;
  }

  if(job ==NULL){
    Serial.println(F("Unable to allocate memory for a new job configuration"));
  }
  for(int i = 0 ; i < 4 ; i++){
    job->ip[i] = 0;
  }
  job->port = 0;
  return job;
}

//0 is fail, 1 is success, 2 is point previous job to this one
int JenkinsClient::parseJob(char *jobConfig, JenkinsJob *job) {
  int switchToCommas = 0;
  int returnCode = 1;
  char *localJobConfigPtr = jobConfig;
#ifdef DEBUG_JENKINS_CLIENT  
  Serial.print(F("Job config string is "));
  Serial.println(localJobConfigPtr);
#endif

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
	  job->ip[tokenIndex] = atoi(token);
	  if(tokenIndex == 2) {
#ifdef DEBUG_JENKINS_CLIENT  
        Serial.println(F("Changing token delimiter to ','"));
#endif
	    switchToCommas = 1;
	  }
	} else if (tokenIndex == 4){
	  job->port = atol(token);
	} else if(tokenIndex >= 5 && (tokenIndex - 5) < MAX_LOCATIONS_PER_LINE){
	  int locationIndex = tokenIndex - 5;
	  int lengthOfLocation = strlen(token);
	  job->jobLocations[locationIndex] = (char *)malloc(sizeof(char)*lengthOfLocation + 1);
	  strncpy (job->jobLocations[locationIndex], token, lengthOfLocation );
	  job->jobLocations[locationIndex][lengthOfLocation] = NULL;
	  returnCode = 0;
	}
	
	token = strsep(&localJobConfigPtr, switchToCommas?",":".");
	tokenIndex++;
  }
  
#ifdef DEBUG_JENKINS_CLIENT  
  Serial.print(F("Job configuration saved as "));
  printIp(job->ip);
  Serial.print(F(":"));
  Serial.print(job->port);
  for(int i = 0 ; i < MAX_LOCATIONS_PER_LINE ; i++){
    Serial.print(job->jobLocations[i]);
  }
  Serial.println();
#endif
  
  return returnCode;
}

int JenkinsClient::parseConfig(char *config) {
 return 0;
}

void JenkinsClient::resetJobs(){
  for(int i = 0 ; i < _numConfiguredJobs ; i++) {
    clearJob(_jobs[i]);
    _jobs[i] = NULL;
  }
  _numConfiguredJobs = 0;
}

int JenkinsClient::initializeConfiguration(){
  
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
    
        JenkinsJob *job = createJob();
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
          clearJob(job);
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

uint16_t JenkinsClient::getStatusForProject(int jobNumber) {
  uint16_t disposition = 0;

  JenkinsJob *job = _jobs[jobNumber];
#ifdef DEBUG_JENKINS_CLIENT  
  Serial.print(F("Making request to  IP:"));
  printIp(job->ip);
  Serial.println();
#endif

  // if you get a connection, report back via serial:
  if (_client->connect(job->ip, job->port)) {
#ifdef DEBUG_JENKINS_CLIENT  
    Serial.print(F("connected\n"));
    // Make a HTTP request:
    Serial.print(F("GET "));
    Serial.print(job->jobLocations[0]);
    Serial.println(JENKINS_POST_JOB_URL);
#endif

    _client->print("GET ");
    _client->print(job->jobLocations[0]);
    _client->println(JENKINS_POST_JOB_URL);
  } 
  else {
    // if you didn't get a connection to the server:
#ifdef DEBUG_JENKINS_CLIENT  
    Serial.print(F("connection failed\n"));
#endif
    _client->stop();
    return JOB_INVALID_STATUS;
  }
  
  while (!_client->available()) {
    //wait
  }
  
  char status[31] = {'\0'};
  int pos = 0;
  
  //assuming that the project name won't have a } in it.
  int bytesRead = _client->readBytesUntil('}',status,30);
  status[bytesRead] = NULL;
#ifdef DEBUG_JENKINS_CLIENT  
  Serial.println(status);
#endif
  _client->flush();
  
#ifdef DEBUG_JENKINS_CLIENT  
  Serial.println();
  Serial.println(F("disconnecting."));
#endif
  _client->stop();
  
  char prefix[] = "{\"color\":\"";
  
  if(!strncmp(status, prefix, strlen(prefix))==0){
    return JOB_INVALID_STATUS;
  }
  
  disposition |= (strstr(status, "disabled") != NULL) ? JOB_DISABLED : 0;
  disposition |= (strstr(status, "blue") != NULL) ? JOB_SUCCEEDED : 0;
  disposition |= (strstr(status, "red") != NULL) ? JOB_FAILED : 0;
  disposition |= (strstr(status, "yellow") != NULL) ? JOB_UNSTABLE : 0;
  disposition |= (strstr(status, "grey") != NULL) ? JOB_CANCELED : 0;
  disposition |= (strstr(status, "anime") != NULL) ? JOB_IN_PROGRESS : 0;
  
#ifdef DEBUG_JENKINS_CLIENT  
  Serial.print(F("Found status: "));
  Serial.println(status);
  Serial.print(F("Mapped to disposition: "));
  Serial.println(disposition, BIN);
#endif

  return disposition;
}

void JenkinsClient::printIp(uint8_t ip[]) {
  for(int i = 0 ; i < 4 ; i++){
    Serial.print(ip[i]);
    if(i<3) { Serial.print(F(".")); }
  }
}

