#ifndef JenkinsClient_h
#define JenkinsClient_h

#include "Arduino.h"
#include "Ethernet.h"

class JenkinsClient
{
public:
	JenkinsClient();
	JenkinsClient(uint8_t ip[], int port, EthernetClient *client);
	void getStatusForProject(char *projectName, char *statusBuffer);
private:
	EthernetClient *_client;
	uint8_t _ip[4];
	int _port;
};

#endif