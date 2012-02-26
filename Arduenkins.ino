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
#include <Ethernet.h>
#include <JenkinsClient.h>
#include <ShiftBriteM.h>
#include <MemoryFree.h>
#include <string.h>

#define NUM_SHIFTBRITES 4
#define SHIFTBRITE_DATA 5
#define SHIFTBRITE_LATCH 6
#define SHIFTBRITE_ENABLE 7
#define SHIFTBRITE_CLOCK 8

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
#define USE_DHCP 1
#define STATIC_IP {192,168,33,2}
#define JENKINS_IP {192,168,33,1}
#define JENKINS_PORT 80

#define PROJECT_0_NAME "Care360-dev"
#define PROJECT_1_NAME "Care360-2012.1"
#define PROJECT_2_NAME "Care360-2011.3.2"
#define PROJECT_3_NAME "Care360-2011.3.1"

char* knownColors[]={  "red", "green", "blue", "yellow", "cyan", "magenta", "off", "white" };
int components[][3]={  RED,  GREEN, BLUE, YELLOW, CYAN, MAGENTA, OFF, WHITE };

// initialize the library instance:
EthernetClient client;
ShiftBriteM sb;
byte serverIP[] = JENKINS_IP;
JenkinsClient jenkinsClient(serverIP, JENKINS_PORT, &client);
char *jenkinsProjects[] = {PROJECT_0_NAME, PROJECT_1_NAME, PROJECT_2_NAME, PROJECT_3_NAME};
  
void setup()
{
  Serial.begin(9600);
  sb = ShiftBriteM(SHIFTBRITE_DATA, SHIFTBRITE_LATCH, SHIFTBRITE_ENABLE, SHIFTBRITE_CLOCK);
  
  pinMode(4, OUTPUT); 
  digitalWrite(4, LOW); //disable SD
  pinMode(10, OUTPUT); 
  digitalWrite(10, HIGH); //enable ethernet
  
  //init all purple (hopefully Jenkins doesn't have a purple status)
  for(int i = 0 ; i < NUM_SHIFTBRITES ; i++){
    sb.sendColour(MAX,0,MAX);
  }
  sb.activate();
  delay(20);
  
  byte mac[] = MAC_ADDRESS;
  if(!USE_DHCP){
    byte staticIP[] = STATIC_IP;
    Ethernet.begin(mac, staticIP);
  } else {
    Serial.println(F("Configuring Ethernet via DHCP"));
    while (Ethernet.begin(mac) == 0) {
      Serial.println(F("Failed to configure Ethernet using DHCP"));
      //blink like a moron so people know you're broken
      for(int delaySecs = 0 ; delaySecs < 15 ; delaySecs++){
        for(int i = 0 ; i < NUM_SHIFTBRITES ; i++){
          sb.sendColour(1023,0,0);
        }
        sb.activate();
        delay(500);
        for(int i = 0 ; i < NUM_SHIFTBRITES ; i++){
          sb.sendColour(0,0,0);
        }
        sb.activate();
        delay(500);
      }
    }
  }
  
  //give the Ethernet module time to initialize... because I saw this in someone else's code.
  delay(1000);
  
  Serial.print(F("Local IP is: "));
  Serial.println(Ethernet.localIP());
  
  for(int i = 0 ; i < NUM_SHIFTBRITES ; i++){
    sb.sendColour(0,0,0);
  }
  sb.activate();
  Serial.print(F("Started Up\n"));
}

void loop()
{
  //Serial.print(F("freeMemory()="));
  //Serial.println(freeMemory());
  for(int projectIndex = 0 ; projectIndex < NUM_SHIFTBRITES ; projectIndex++){
    Serial.print(F("Looking for project "));
    Serial.println(jenkinsProjects[projectIndex]);
    char color[24] = {'\0'};
    jenkinsClient.getStatusForProject(jenkinsProjects[projectIndex], color);
    if(!strlen(color)==0){
      Serial.print(F("Received color "));
      Serial.println(color);
      for(int i = 0 ; i < KNOWN_COLORS_SIZE ; i++){
        if(strncmp(knownColors[i],color,3) == 0){
          sb.sendColour(components[i][0], components[i][1], components[i][2]);
          Serial.print(F("Setting ShiftBrite to color "));
          Serial.println(knownColors[i]);
          break;
        }
      }
    } else {
      Serial.println(F("Failure, turning off light"));
      sb.sendColour(0,0,0);
    }
  }
  sb.activate();
  //check again in a minute
  delay(60000);
}
