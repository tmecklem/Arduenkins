#ifndef JenkinsClient_h
#define JenkinsClient_h

#include "Arduino.h"
#include "aJSON.h"      //https://github.com/interactive-matter/aJson 

class JenkinsClient
{
public:
	JenkinsClient();
	JenkinsClient(char *serverName, uint8_t ip[]);
	int update();
	char *getStatusForProject(char *projectName);
private:
	char* _serverName;
	uint8_t _ip[4];
	aJsonObject* root;
};

#endif