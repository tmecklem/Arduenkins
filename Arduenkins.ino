/* 
Copyright (C) 2012 Timothy Mecklem

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include <SPI.h>
#include <Dhcp.h>
#include <Dns.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <EthernetServer.h>
#include <EthernetUdp.h>
#include <util.h>
#include <JenkinsClient.h>
#include <HTTPClient.h>          //https://github.com/interactive-matter/HTTPClient 
#include <aJSON.h>               //https://github.com/interactive-matter/aJson 
#include <ShiftBriteM.h>

#define NUM_SHIFTBRITES 4
#define SHIFTBRITE_CLOCK 8
#define SHIFTBRITE_ENABLE 7
#define SHIFTBRITE_LATCH 6
#define SHIFTBRITE_DATA 5

#define KNOWN_COLORS_SIZE 8
#define MAX 768
#define RED {MAX,0,0}
#define GREEN {0,MAX,0}
#define BLUE {0,0,MAX}
#define YELLOW {MAX,MAX,0}
#define CYAN {0,MAX,MAX}
#define MAGENTA {MAX,0,MAX}
#define OFF {0,0,0}
#define WHITE {511,511,511}

#define MAC_ADDRESS {0x90, 0xA2, 0xDA, 0x00, 0xFD, 0x1D}
#define JENKINS_SERVERNAME "tmecklem-mbp"
#define JENKINS_IP {192,168,10,147}

#define PROJECT_0_NAME "TestProject-2011.3.1"
#define PROJECT_1_NAME "TestProject-2011.3.2"
#define PROJECT_2_NAME "TestProject-2012.1"
#define PROJECT_3_NAME "TestProject-dev"

char* knownColors[]={  "red", "green", "blue", "yellow", "cyan", "magenta", "off", "white" };
int components[][3]={  RED,  GREEN, BLUE, YELLOW, CYAN, MAGENTA, OFF, WHITE };

//char testJSON[] = {"{\"assignedLabels\":[{}],\"mode\":\"EXCLUSIVE\",\"nodeDescription\":\"the master Jenkins node\",\"nodeName\":\"\",\"numExecutors\":1,\"description\":null,\"jobs\":[{\"name\":\"TestProject-2011.3.1\",\"url\":\"http://buildserver:4567/jenkins/job/TestProject-2011.3.1/\",\"color\":\"blue\"},{\"name\":\"TestProject-2011.3.2\",\"url\":\"http://buildserver:4567/jenkins/job/TestProject-2011.3.2/\",\"color\":\"blue\"},{\"name\":\"TestProject-2012.1\",\"url\":\"http://buildserver:4567/jenkins/job/TestProject-2012.1/\",\"color\":\"blue\"},{\"name\":\"TestProject-dev\",\"url\":\"http://buildserver:4567/jenkins/job/TestProject-dev/\",\"color\":\"blue\"},{\"name\":\"TestProject-dev-nightly\",\"url\":\"http://buildserver:4567/jenkins/job/TestProject-dev-nightly/\",\"color\":\"disabled\"},{\"name\":\"TestProject-NONTEST\",\"url\":\"http://buildserver:4567/jenkins/job/TestProject-TESTSET/\",\"color\":\"disabled\"},{\"name\":\"TestProject-Dev-Test\",\"url\":\"http://buildserver:4567/jenkins/job/TestProject-Dev-Test/\",\"color\":\"disabled\"},{\"name\":\"P6Spy-dev\",\"url\":\"http://buildserver:4567/jenkins/job/SOMETHINGELSE-DEV/\",\"color\":\"blue\"}],\"overallLoad\":{},\"primaryView\":{\"name\":\"All\",\"url\":\"http://buildserver:4567/jenkins/\"},\"quietingDown\":false,\"slaveAgentPort\":0,\"useCrumbs\":false,\"useSecurity\":false,\"views\":[{\"name\":\"All\",\"url\":\"http://buildserver:4567/jenkins/\"}]}"};
//char testJSON[] = {"{\"jobs\":[{\"name\":\"TestProject-2011.3.1\",\"color\":\"blue\"},{\"name\":\"TestProject-2011.3.2\",\"color\":\"yellow\"},{\"name\":\"TestProject-2012.1\",\"color\":\"blue\"},{\"name\":\"TestProject-dev\",\"color\":\"red\"}]}"};

// initialize the library instance:
EthernetClient client;
ShiftBriteM sb;
byte serverIP[] = JENKINS_IP;
JenkinsClient jenkinsClient(JENKINS_SERVERNAME, serverIP);
char *jenkinsProjects[] = {PROJECT_0_NAME, PROJECT_1_NAME, PROJECT_2_NAME, PROJECT_3_NAME};
    
void setup()
{
  pinMode(10, OUTPUT); 
  digitalWrite(10, HIGH); //enable ethernet
  
  Serial.begin(9600);
  byte mac[] = MAC_ADDRESS;
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    for(int i = 0 ; i < NUM_SHIFTBRITES ; i++){
      sb.sendColour(1023,0,0);
    }
    sb.activate();
    // no point in carrying on, so do nothing forevermore:
    for(;;)
      ;
  }
  Serial.println(Ethernet.localIP());
  sb = ShiftBriteM(SHIFTBRITE_CLOCK, SHIFTBRITE_ENABLE, SHIFTBRITE_LATCH, SHIFTBRITE_DATA);
  for(int i = 0 ; i < NUM_SHIFTBRITES ; i++){
    sb.sendColour(0,0,0);
  }
  sb.activate();
  delay(1000);
  Serial.print("Started Up");
}

void loop()
{
    if(jenkinsClient.update()){
      for(int projectIndex = 0 ; projectIndex < NUM_SHIFTBRITES ; projectIndex++){
        Serial.print("Looking for project ");
        Serial.println(jenkinsProjects[projectIndex]);
        char *color = jenkinsClient.getStatusForProject(jenkinsProjects[projectIndex]);
        Serial.print("Received color ");
        Serial.println(color);
        for(int i = 0 ; i < KNOWN_COLORS_SIZE ; i++){
            if(strncmp(knownColors[i],color,3) == 0){
                sb.sendColour(components[i][0], components[i][1], components[i][2]);
                Serial.print("Setting ShiftBrite to color ");
                Serial.println(knownColors[i]);
                break;
            }
        }
      }
      sb.activate();
      delay(60000);
    } else {
      delay(5000); 
    }
}
