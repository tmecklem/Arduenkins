#include "JenkinsClient.h"
#include <string.h>
#include <stdlib.h>

#define JENKINS_POST_JOB_URL "/api/json?tree=color"
#define LINK_JOBS 2

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
  
  Serial.print(F("Config IP saved as "));;
  Serial.print(_ip[0]);
  Serial.print(F("."));;
  Serial.print(_ip[1]);
  Serial.print(F("."));;
  Serial.print(_ip[2]);
  Serial.print(F("."));;
  Serial.println(_ip[3]);
  
  _client = client;
  _configurationLocation = configurationLocation;
}

void JenkinsClient::clearJob(JenkinsJob *job) {
  if(job->jobUrl != NULL){
    free(job->jobUrl);
  }
  free(job);
}

JenkinsJob *JenkinsClient::createJob(){
  JenkinsJob *job = (JenkinsJob *)malloc(sizeof(JenkinsJob));
  job->jobUrl = (char *)malloc(sizeof(char) * MAX_CONFIG_LINE_LENGTH + 1);
  if(job ==NULL || job->jobUrl == NULL){
    Serial.println(F("Unable to allocate memory for a new job configuration"));
  }
  job->jobUrl[MAX_CONFIG_LINE_LENGTH] = NULL;
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
  Serial.print(F("Job config string is "));
  Serial.println(localJobConfigPtr);

  char *token;
  char *end_str;
  token = strsep (&localJobConfigPtr,".");
  int tokenIndex = 0;
  while (token != NULL){
    Serial.print(F("Token "));
    Serial.print(token);
    Serial.print(F(" found at index "));
    Serial.println(tokenIndex);
    Serial.print(F("Remaining string is "));
    Serial.println(localJobConfigPtr);
	if(tokenIndex <= 3){
	  job->ip[tokenIndex] = atoi(token);
	  if(tokenIndex == 2) {
        //Serial.println(F("Changing token delimiter to ','"));
	    switchToCommas = 1;
	  }
	} else if (tokenIndex == 4){
	  job->port = atol(token);
	} else if(tokenIndex == 5){
	  strncpy (job->jobUrl, token, MAX_CONFIG_LINE_LENGTH );
	  returnCode = 0;
	}
	
	token = strsep(&localJobConfigPtr, switchToCommas?",":".");
	tokenIndex++;
  }
  
  Serial.print(F("Job configuration saved as "));
  Serial.print(job->ip[0]);
  Serial.print(F("."));;
  Serial.print(job->ip[1]);
  Serial.print(F("."));;
  Serial.print(job->ip[2]);
  Serial.print(F("."));;
  Serial.print(job->ip[3]);
  Serial.print(F(":"));
  Serial.print(job->port);
  Serial.println(job->jobUrl);
  
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

/**
 * returns the number of jobs available for status updates per the server config, 
 * or -1 if a problem occurred in retrieving configuration
 */
int JenkinsClient::initializeConfiguration(){
  
  resetJobs();

  Serial.print(F("Requesting configuration from http://"));
  Serial.print(_ip[0]);
  Serial.print(F("."));
  Serial.print(_ip[1]);
  Serial.print(F("."));
  Serial.print(_ip[2]);
  Serial.print(F("."));
  Serial.print(_ip[3]);
  Serial.print(F(":"));
  Serial.print(_port);
  Serial.println(_configurationLocation);
  
  if (_client->connect(_ip, _port)) {
    Serial.print(F("connected\n"));;
    // Make a HTTP request:
    Serial.print(F("GET "));
    Serial.print(_configurationLocation);

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
        Serial.print(F("Found a job configuration: "));
        Serial.println(configResponse);
    
        JenkinsJob *job = createJob();
        int status = parseJob(configResponse, job);
        Serial.print(F("parseJob return status was "));
        Serial.println(status);
        if(status == 0){
          Serial.print(F("Adding job"));
         _jobs[_numConfiguredJobs++] = job;
        } else {
          Serial.println(F("Something went wrong, cleaning up job allocation"));
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

  Serial.println();
  Serial.print(F("disconnecting.\n"));;
  _client->stop();
  
  //Assuming things went well!
  Serial.print(F("Jobs Configured: "));
  Serial.println(_numConfiguredJobs);
  return _numConfiguredJobs;
}

uint16_t JenkinsClient::getStatusForProject(int jobNumber) {
  uint16_t disposition = 0;

  JenkinsJob *job = _jobs[jobNumber];
  Serial.print(F("Making request to  IP:"));
  Serial.print(job->ip[0]);
  Serial.print(F("."));
  Serial.print(job->ip[1]);
  Serial.print(F("."));
  Serial.print(job->ip[2]);
  Serial.print(F("."));
  Serial.println(job->ip[3]);

  // if you get a connection, report back via serial:
  if (_client->connect(job->ip, job->port)) {
    Serial.print(F("connected\n"));;
    // Make a HTTP request:
    Serial.print(F("GET "));
    Serial.print(job->jobUrl);
    Serial.println(JENKINS_POST_JOB_URL);

    _client->print("GET ");
    _client->print(job->jobUrl);
    _client->println(JENKINS_POST_JOB_URL);
  } 
  else {
    // if you didn't get a connection to the server:
    Serial.print(F("connection failed\n"));
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
  status[bytesRead] = '\0';
  Serial.println(status);
  _client->flush();

  Serial.println();
  Serial.print(F("disconnecting.\n"));;
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
  
  Serial.print(F("Found status: "));
  Serial.println(status);
  Serial.print(F("Mapped to disposition: "));
  Serial.println(disposition, BIN);

  return disposition;
}
