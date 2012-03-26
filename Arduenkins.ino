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
#include "ArduenkinsConfig.h"
#include "Animations.h"
#include <JenkinsClient.h>
#include <ShiftBriteM.h>
#include <MemoryFree.h>
#include <string.h>
#include <avr/io.h>
#include <avr/wdt.h>

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

char* knownColors[]={  "red", "green", "blue", "yellow", "cyan", "magenta", "off", "aborted" };
int components[][3]={  RED,  GREEN, BLUE, YELLOW, CYAN, MAGENTA, OFF, WHITE };

// initialize the library instance:
EthernetClient client;
ShiftBriteM sb;
byte serverIP[] = JENKINS_IP;
JenkinsClient jenkinsClient(serverIP, JENKINS_PORT, &client);
char *jenkinsProjects[] = {PROJECT_0_NAME, PROJECT_1_NAME, PROJECT_2_NAME, PROJECT_3_NAME};
int cyclesBeforeRefresh = 0;
int failureCount = 0;

// Function Pototype
void wdt_init(void) __attribute__((naked)) __attribute__((section(".init3")));

void initializeEthernet() {
  
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
          sb.setColor(i,1023,0,0);
        }
        sb.performNextStep();
        delay(500);
        for(int i = 0 ; i < NUM_SHIFTBRITES ; i++){
          sb.setColor(i,0,0,0);
        }
        sb.performNextStep();
        delay(500);
      }
    }
  }
  
  //give the Ethernet module time to initialize... because I saw this in someone else's code.
  delay(1000);
  
  Serial.print(F("Local IP is: "));
  Serial.println(Ethernet.localIP());
}

void setup()
{
  wdt_init();
  wdt_disable();
  
  Serial.begin(9600);
  sb = ShiftBriteM(NUM_SHIFTBRITES, SHIFTBRITE_DATA, SHIFTBRITE_LATCH, SHIFTBRITE_ENABLE, SHIFTBRITE_CLOCK, LIGHT_UPDATE_FREQUENCY);
  
  pinMode(4, OUTPUT); 
  digitalWrite(4, LOW); //disable SD
  pinMode(10, OUTPUT); 
  digitalWrite(10, HIGH); //enable ethernet
  
  //init all purple (hopefully Jenkins doesn't have a purple status)
  for(int i = 0 ; i < NUM_SHIFTBRITES ; i++){
    sb.setColor(i,MAX,0,MAX);
  }
  sb.performNextStep();
  delay(20);
  
  initializeEthernet();
  
  for(int i = 0 ; i < NUM_SHIFTBRITES ; i++){
    sb.setColor(i,0,0,0);
  }
  Serial.print(F("Started Up\n"));
  
  client.setTimeout(5000);
  
  if(ENABLE_WATCHDOG){
    wdt_enable(WDTO_8S);
  }
}

/* This is to work around a bug in the ethernet client code for the 5100 which causes it to 
have... trouble after a while. Supposedly fixed in 1.0.1 (not out yet) */
void doWorkaround(int failure, int *failureCount){
  if(failure){
    (*failureCount) = (*failureCount) + 1;
    Serial.print(F("Failure count is now "));
    Serial.println(*failureCount);
    if(*failureCount >= 10) {
      Serial.println(F("Maximum consecutive failures reached, resetting ethernet"));
      initializeEthernet();
      *failureCount = 0;
    }
  } else {
    //reset failure count
    *failureCount = 0; 
  }
}

void loop()
{
  //Serial.print(F("freeMemory()="));
  //Serial.println(freeMemory());
  
  if( cyclesBeforeRefresh <=0 ) {
    cyclesBeforeRefresh = (UPDATE_INTERVAL*LIGHT_UPDATE_FREQUENCY) - 1;
    
    for(int projectIndex = 0 ; projectIndex < NUM_SHIFTBRITES ; projectIndex++){
      Serial.print(F("Looking for project "));
      Serial.println(jenkinsProjects[projectIndex]);
      char color[24] = {'\0'};
      //TODO: spread out these requests so that the animations don't stop for so long...
      int retry = 0;
      int failure = 0;
      while(failure = jenkinsClient.getStatusForProject(jenkinsProjects[projectIndex], color, JENKINS_PRE_JOB_URL, JENKINS_POST_JOB_URL) == 1 && retry < 5){
        doWorkaround(failure, &failureCount);
        retry++;
      }
      if(retry < 5) {
        //we succeeded without hitting the max count, so allow the failureCount to reset
        doWorkaround(failure, &failureCount);
      }
      if(!strlen(color)==0){
        int found = 0;
        Serial.print(F("Received color "));
        Serial.println(color);
        int animate = (strstr(color, "anime") != NULL)?1:0;
        int success = (strstr(color, "blue") != NULL)?1:0;
        
        for(int i = 0 ; i < KNOWN_COLORS_SIZE ; i++){
          if( (!success && strncmp(knownColors[i],color,3) == 0 )
              || (success && strncmp(knownColors[i],SUCCESS_LIGHT_COLOR,3) == 0)){
            sb.setColor(projectIndex, components[i][0], components[i][1], components[i][2], animate?&pulseAnimation:NULL);
            Serial.print(F("Setting ShiftBrite to color "));
            Serial.println(knownColors[i]);
            found = 1;
            break;
          }
        }
        if(!found){
          sb.setColor(projectIndex,0,0,0);
        }
      } else {
        Serial.println(F("Failure, turning off light"));
        sb.setColor(projectIndex,0,0,0);
      }
    }
    if(ENABLE_WATCHDOG){
      //reset the watchdog timer in case the request took a while
      wdt_reset();
    }
  }
  
  int cyclesLost = sb.performNextStep();
  //check again in a tenth of a second minus whatever it took to update the shiftbrites
  delay((int)( ((float)1000) /LIGHT_UPDATE_FREQUENCY) - cyclesLost);
  cyclesBeforeRefresh--;
  
  if(ENABLE_WATCHDOG){
    wdt_reset();
  }
}

// Function Implementation
void wdt_init(void)
{
    MCUSR = 0;
    wdt_disable();

    return;
}
