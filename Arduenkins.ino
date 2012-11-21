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
//#include <Dhcp.h>
#include <Ethernet.h>
#include "ArduenkinsConfig.h"
#include "Animations.h"
#include <BuildLightConfigurationManager.h>
#include <ShiftBriteM.h>
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

#define FAILED 0
#define SUCCEEDED 1
#define UNSTABLE 3
#define CANCELED 7
#define DISABLED 6
char* knownColors[]={  "red", "green", "blue", "yellow", "cyan", "magenta", "off", "aborted" };
int components[][3]={  RED,  GREEN, BLUE, YELLOW, CYAN, MAGENTA, OFF, WHITE };

// initialize the library instance:
EthernetClient client;
ShiftBriteM sb;
byte serverIP[] = CONFIG_IP;
BuildLightConfigurationManager jenkinsClient(serverIP, CONFIG_PORT, &client, CONFIG_LOCATION);
char *jenkinsProjects[] = {};
int cyclesBeforeRefresh = 0;
int cyclesBeforeReinitialization = 0;
int failureCount = 0;

int numberOfJobsConfigured = 0;

// Function Pototype
void wdt_init(void) __attribute__((naked)) __attribute__((section(".init3")));

int freeRam() {
  int byteCounter = 0; // ini­tial­ize a counter
  byte *byteArray; // cre­ate a pointer to a byte array
  // More on point­ers here: http://en.wikipedia.org/wiki/Pointer#C_pointers

  // use the mal­loc func­tion to repeat­edly attempt allo­cat­ing a cer­tain num­ber of bytes to mem­ory
  // More on mal­loc here: http://en.wikipedia.org/wiki/Malloc
  while ( (byteArray = (byte*) malloc (byteCounter * sizeof(byte))) != NULL ) {
    byteCounter++; // if allo­ca­tion was suc­cess­ful, then up the count for the next try
    free(byteArray); // free mem­ory after allo­cat­ing it
  }
  
  free(byteArray); // also free mem­ory after the func­tion fin­ishes
  return byteCounter; // send back the high­est num­ber of bytes suc­cess­fully allo­cated
}

void initializeEthernet() {
  
  byte mac[] = MAC_ADDRESS;
  if(!USE_DHCP){
    byte staticIP[] = STATIC_IP;
    Serial.print(F("Configuring Ethernet with static IP "));
    Serial.print(staticIP[0]);
    Serial.print(F("."));;
    Serial.print(staticIP[1]);
    Serial.print(F("."));;
    Serial.print(staticIP[2]);
    Serial.print(F("."));;
    Serial.println(staticIP[3]);
    Serial.print(F("freeMemory()="));
    Serial.println(freeRam());
    Ethernet.begin(mac, staticIP);
    Serial.println(F("Ethernet.begin finished"));
  } else {
    Serial.println(F("Configuring Ethernet via DHCP"));
    while (Ethernet.begin(mac) == 0) {
      Serial.println(F("Failed to configure Ethernet using DHCP"));
      //blink like a moron so people know you're broken
      for(int delaySecs = 0 ; delaySecs < 15 ; delaySecs++){
        for(int i = 0 ; i < NUM_LIGHTS ; i++){
          sb.setColor(i,1023,0,0);
        }
        sb.performNextStep();
        delay(500);
        for(int i = 0 ; i < NUM_LIGHTS ; i++){
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
  while (!Serial){
    ;
  }
  Serial.println(F("Booting app."));
  sb = ShiftBriteM(NUM_LIGHTS, SHIFTBRITE_DATA, SHIFTBRITE_LATCH, SHIFTBRITE_ENABLE, SHIFTBRITE_CLOCK, LIGHT_UPDATE_FREQUENCY);
  
  pinMode(4, OUTPUT); 
  digitalWrite(4, LOW); //disable SD
  pinMode(10, OUTPUT); 
  digitalWrite(10, HIGH); //enable ethernet
  
  //init all purple (hopefully Jenkins doesn't have a purple status)
  for(int i = 0 ; i < NUM_LIGHTS ; i++){
    sb.setColor(i,MAX,0,MAX);
  }
  sb.performNextStep();
  delay(20);
  
  initializeEthernet();
  
  for(int i = 0 ; i < NUM_LIGHTS ; i++){
    sb.setColor(i,0,0,0);
  }
  Serial.println(F("Started Up"));
  
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
  
  if(cyclesBeforeReinitialization <= 0) {
    Serial.print(F("freeMemory()="));
    Serial.println(freeRam());
    Serial.println(F("Re-initializing from configuration site"));
    numberOfJobsConfigured = jenkinsClient.initializeConfiguration();
    if(numberOfJobsConfigured > 0){
      cyclesBeforeReinitialization = (UPDATE_INTERVAL*LIGHT_UPDATE_FREQUENCY) - 1;
    }
    if(ENABLE_WATCHDOG){
      //reset the watchdog timer in case the request took a while
      wdt_reset();
    }
    cyclesBeforeRefresh = 0;
  }
  
  if( cyclesBeforeRefresh <=0 ) {
    cyclesBeforeRefresh = (UPDATE_INTERVAL*LIGHT_UPDATE_FREQUENCY) - 1;
    
    for(int projectIndex = 0 ; projectIndex < numberOfJobsConfigured ; projectIndex++){
      Serial.print(F("Updating project "));
      Serial.println(projectIndex);
      int retry = 0;
      int failure = 0;
      uint16_t status = 0;
      
      while( (status = jenkinsClient.getStatusForProject(projectIndex)) == JOB_INVALID_STATUS && retry < 5){
        doWorkaround(failure, &failureCount);
        retry++;
      }
      if(retry < 5) {
        //we succeeded without hitting the max count, so allow the failureCount to reset
        doWorkaround(failure, &failureCount);
      }
      Serial.print(F("Status received from client"));
      Serial.println(status, BIN);
      if(status != JOB_INVALID_STATUS){
        int indexOfColorToUse = DISABLED;
        if(status & JOB_SUCCEEDED) {
          indexOfColorToUse = SUCCEEDED;
        } else if(status & JOB_FAILED){
          indexOfColorToUse = FAILED;
        } else if(status & JOB_UNSTABLE) {
          indexOfColorToUse = UNSTABLE;
        } else if(status & JOB_CANCELED) {
          indexOfColorToUse = CANCELED;
        }
        
        int animate = status & JOB_IN_PROGRESS;
        sb.setColor(projectIndex, components[indexOfColorToUse][0], components[indexOfColorToUse][1], components[indexOfColorToUse][2], animate?&pulseAnimation:NULL);
        Serial.print(F("Setting ShiftBrite to color "));
        Serial.println(knownColors[indexOfColorToUse]);

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
  cyclesBeforeReinitialization--;
  
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
