#include "JenkinsClient.h"
#include <string.h>

JenkinsClient::JenkinsClient() {
  uint8_t server[] = {192,168,0,1};
  JenkinsClient(server, 80, NULL);
}

JenkinsClient::JenkinsClient(uint8_t ip[], int port, EthernetClient *client) {
  _ip[0] = ip[0];
  _ip[1] = ip[1];
  _ip[2] = ip[2];
  _ip[3] = ip[3];
  
  _port = port;
  
  Serial.print(F("Server IP saved as "));;
  Serial.print(_ip[0]);
  Serial.print(F("."));;
  Serial.print(_ip[1]);
  Serial.print(F("."));;
  Serial.print(_ip[2]);
  Serial.print(F("."));;
  Serial.println(_ip[3]);
  
  _client = client;
}

void JenkinsClient::getStatusForProject(char *projectName, char *statusBuffer, char *preJobUrl, char *postJobUrl) {

  Serial.print(F("Making request to  IP:"));;
  Serial.print(_ip[0]);
  Serial.print(F("."));;
  Serial.print(_ip[1]);
  Serial.print(F("."));;
  Serial.print(_ip[2]);
  Serial.print(F("."));;
  Serial.println(_ip[3]);

  // if you get a connection, report back via serial:
  if (_client->connect(_ip, _port)) {
    Serial.print(F("connected\n"));;
    // Make a HTTP request:
    Serial.print(F("GET "));
    Serial.print(preJobUrl);
    Serial.print(projectName);
    Serial.println(postJobUrl);

    _client->print("GET ");
    _client->print(preJobUrl);
    _client->print(projectName);
    _client->println(postJobUrl);
    _client->println();
  } 
  else {
    // if you didn't get a connection to the server:
    Serial.print(F("connection failed\n"));
    _client->stop();
    return;
  }
  
  
  while (!_client->available()) {
    //wait
  }
  
  char status[31] = {'\0'};
  int pos = 0;
  
  // if there are incoming bytes available 
  // from the server, read them and print them:
  if (_client->available()) {
    int bytesRead = _client->readBytesUntil('\0',status,30);
    Serial.println(status);
  }

  if (!_client->connected()) {
    Serial.println();
    Serial.print(F("disconnecting.\n"));;
    _client->stop();
  }
  
  char prefix[] = "{\"color\":\"";
  
  if(!strncmp(status, prefix, strlen(prefix))==0){
    return;
  }
  
  //Assuming things went well!
  int statusLength = strlen(status);
  int endPosition = statusLength-strlen(prefix)-2;
  strncpy(statusBuffer, status+(sizeof(char)*strlen(prefix)), endPosition);
  statusBuffer[endPosition] = '\0';
  
  //Serial.print(F("Found status: "));
  //Serial.println(statusBuffer);
  
}
