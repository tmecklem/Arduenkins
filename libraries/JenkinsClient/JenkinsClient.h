#ifndef JenkinsClient_h
#define JenkinsClient_h

#include "Arduino.h"
#include "Ethernet.h"
#include "aJSON.h"      //https://github.com/interactive-matter/aJson 

class JenkinsClient
{
public:
	JenkinsClient();
	JenkinsClient(uint8_t ip[], EthernetClient *client);
	int update();
	char *getStatusForProject(char *projectName);
private:
	EthernetClient *_client;
	uint8_t _ip[4];
	aJsonObject *_root;
};

#endif