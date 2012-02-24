#include "JenkinsClient.h"
#include "HTTPClient.h" //https://github.com/interactive-matter/HTTPClient 

JenkinsClient::JenkinsClient() {
	char serverName[] = "jenkins";
	uint8_t server[] = {192,168,0,1};
	JenkinsClient(serverName, server);
}

JenkinsClient::JenkinsClient(char *serverName, uint8_t ip[]) {
	_serverName = serverName;
	_ip[0] = ip[0];
	_ip[1] = ip[1];
	_ip[2] = ip[2];
	_ip[3] = ip[3];
	
	root = NULL;
}

int JenkinsClient::update() {

	if(root != NULL){
		aJson.deleteItem(root);
	}
	
	char *filters[] = {"jobs", "color", "name"};

	HTTPClient client(_serverName, _ip);
	
	client.debug(1);
	Serial.print("Making request to ");
	Serial.print(_serverName);
	Serial.print(", IP:");
	Serial.print(_ip[0]);
	Serial.print(".");
	Serial.print(_ip[1]);
	Serial.print(".");
	Serial.print(_ip[2]);
	Serial.print(".");
	Serial.println(_ip[3]);
	
	FILE* jenkinsResponse = client.getURI("/~mecklem_t/jenkins.json");
	
	int returnCode = client.getLastReturnCode();
	Serial.print("Return code: ");
	Serial.println(returnCode);
	
	delay(1000);
	
	if(jenkinsResponse != NULL){
		Serial.println("Received file handler");
		
		root = aJson.parse(jenkinsResponse, filters);
 	 	Serial.println("Parsed response");
   	 	
	    client.closeStream(jenkinsResponse);
	    return 1;
    } else {
    	Serial.println("return from server was NULL!");
    	return 0;
    }
}

char *JenkinsClient::getStatusForProject(char *projectName) {

	//TODO: fix brute force approach?
	Serial.print("Searching for status for ");
	Serial.println(projectName);

	aJsonObject* jobs = aJson.getObjectItem(root, "jobs");
	unsigned char arraySize = aJson.getArraySize(jobs);
	/*Serial.print("Found # jobs:" + arraySize);*/
	for(unsigned char i = 0 ; i < arraySize ; i++){
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
	}
}