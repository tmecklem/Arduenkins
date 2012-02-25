#include "JenkinsClient.h"

JenkinsClient::JenkinsClient() {
	char serverName[] = "jenkins";
	uint8_t server[] = {192,168,0,1};
	JenkinsClient(server, NULL);
}

JenkinsClient::JenkinsClient(uint8_t ip[], EthernetClient *client) {
	_ip[0] = ip[0];
	_ip[1] = ip[1];
	_ip[2] = ip[2];
	_ip[3] = ip[3];
	
	Serial.print(F("Server IP saved as "));;
	Serial.print(_ip[0]);
	Serial.print(F("."));;
	Serial.print(_ip[1]);
	Serial.print(F("."));;
	Serial.print(_ip[2]);
	Serial.print(F("."));;
	Serial.println(_ip[3]);
	
	_client = client;
	
	_root = NULL;
}

int JenkinsClient::update() {

	if(_root != NULL){
		aJson.deleteItem(_root);
	}
	
	char *filters = "jobs,color,name";

	Serial.print(F("Making request to  IP:"));;
	Serial.print(_ip[0]);
	Serial.print(F("."));;
	Serial.print(_ip[1]);
	Serial.print(F("."));;
	Serial.print(_ip[2]);
	Serial.print(F("."));;
	Serial.println(_ip[3]);

	// if you get a connection, report back via serial:
	if (_client->connect(_ip, 80)) {
    	Serial.print(F("connected\n"));;
    	// Make a HTTP request:
    	_client->println("GET /~mecklem_t/jenkins.json");
    	_client->println();
  	} 
  	else {
    	// if you didn't get a connection to the server:
    	Serial.print(F("connection failed\n"));;
  	}
	
	
	while (!_client->available()) {
		//wait
	}

	_root = aJson.parse(_client, filters);

	if (!_client->connected()) {
		Serial.println();
		Serial.print(F("disconnecting.\n"));;
		_client->stop();
	}

	
}

char *JenkinsClient::getStatusForProject(char *projectName) {

	//TODO: fix brute force approach?
	/*Serial.print("Searching for status for ");
	Serial.println(projectName);

	aJsonObject* jobs = aJson.getObjectItem(root, "jobs");
	unsigned char arraySize = aJson.getArraySize(jobs);
	/*Serial.print("Found # jobs:" + arraySize);*/
	/*for(unsigned char i = 0 ; i < arraySize ; i++){
		aJsonObject* job = aJson.getArrayItem(jobs, i);
	
		char *name = (aJson.getObjectItem(job, "name"))->valuestring;
		char *color = (aJson.getObjectItem(job, "color"))->valuestring;
		
		Serial.write("Found project while searching: ");
		Serial.write(name);
		Serial.write(": ");
		Serial.println(color);
	
		if(strcmp(projectName, name) == 0){
			return color;
		}
	}*/
	return "blue";
}