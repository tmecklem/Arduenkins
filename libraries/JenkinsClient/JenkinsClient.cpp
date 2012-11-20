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
  free(job);
}

JenkinsJob *JenkinsClient::createJob(){
  JenkinsJob *job = (JenkinsJob *)malloc(sizeof(JenkinsJob));
  return job;
}

//0 is fail, 1 is success, 2 is point previous job to this one
int JenkinsClient::parseJob(char *jobConfig, JenkinsJob &job) {
  int switchToCommas = 0;
  int returnCode = 0;

  char *token;
  char *end_str;
  token = strtok_r (jobConfig,".",&end_str);
  int tokenIndex = 0;
  while (token != NULL){
    //Serial.print(F("Token "));
    //Serial.print(token);
    //Serial.print(F(" found at index "));
    //Serial.println(tokenIndex);
	if(tokenIndex <= 3){
	  job.ip[tokenIndex] = atoi(token);
	  if(tokenIndex == 2) {
        //Serial.println(F("Changing token delimiter to ','"));
	    switchToCommas = 1;
	  }
	} else if (tokenIndex == 4){
	  job.port = atol(token);
	} else if(tokenIndex == 5){
	  strcpy(job.jobUrl, token); //don't buffer overflow me, bro! (TODO: check)
	} else if (tokenIndex == 6){
	  if(strncmp(token,"1",1) == 0){
	    returnCode = LINK_JOBS;
	  } else {
	    returnCode = 1;
	  }
	} 
	
	token = strtok_r(NULL, switchToCommas?",":".",&end_str);
	tokenIndex++;
  }
  
  Serial.print(F("Job configuration saved as "));
  Serial.print(job.ip[0]);
  Serial.print(F("."));;
  Serial.print(job.ip[1]);
  Serial.print(F("."));;
  Serial.print(job.ip[2]);
  Serial.print(F("."));;
  Serial.print(job.ip[3]);
  Serial.print(F(":"));
  Serial.print(job.port);
  Serial.println(job.jobUrl);
  
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
  Serial.println(_ip[3]);
  Serial.print(F(":"));
  Serial.print(_port);
  Serial.print(_configurationLocation);
  
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
  
  char configResponse[MAX_CONFIG_LINE_LENGTH+1] = {'\0'};
  int position = 0;
  
  for(;;){
  	
  	if (_client->available()) {
      char nextChar = _client->read();
      if(nextChar == '\n'){
        Serial.print(F("Found a job configuration: "));
        Serial.println(configResponse);
    
        JenkinsJob *job = createJob();
        int status = parseJob(configResponse, *job);
        Serial.print(F("parseJob return status was "));
        Serial.println(status);
        if(status == 1){
          Serial.print(F("Adding job"));
         _jobs[_numConfiguredJobs++] = job;
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

  Serial.println();
  Serial.print(F("disconnecting.\n"));;
  _client->stop();
  
  //Assuming things went well!
  Serial.print(F("Jobs Configured: "));
  Serial.println(_numConfiguredJobs);
  return _numConfiguredJobs;
}

int JenkinsClient::getStatusForProject(int jobNumber, char *statusBuffer) {

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
    return 1;
  }
  
  
  while (!_client->available()) {
    //wait
  }
  
  char status[31] = {'\0'};
  int pos = 0;
  
  //assuming that the project name won't have a } in it.
  int bytesRead = _client->readBytesUntil('}',status,30);
  Serial.println(status);
  _client->flush();

  Serial.println();
  Serial.print(F("disconnecting.\n"));;
  _client->stop();
  
  char prefix[] = "{\"color\":\"";
  
  if(!strncmp(status, prefix, strlen(prefix))==0){
    return 0;
  }
  
  //Assuming things went well!
  int statusLength = strlen(status);
  int endPosition = statusLength-strlen(prefix)-1;
  strncpy(statusBuffer, status+(sizeof(char)*strlen(prefix)), endPosition);
  statusBuffer[endPosition] = '\0';
  
  //Serial.print(F("Found status: "));
  //Serial.println(statusBuffer);
  
  return 0;
  
}
