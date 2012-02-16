#include <HughesyShiftBrite.h>   //http://code.google.com/p/shiftbritehughesyarduino/
#include <aJSON.h>               //https://github.com/interactive-matter/aJson 

#define KNOWN_COLORS_SIZE 8
#define RED {1023,0,0}
#define GREEN {0,1023,0}
#define BLUE {0,0,1023}
#define YELLOW {1023,1023,0}
#define CYAN {0,1023,1023}
#define MAGENTA {1023,0,1023}
#define OFF {0,0,0}
#define WHITE {1023,1023,1023}

//char testJSON[] = {"{\"assignedLabels\":[{}],\"mode\":\"EXCLUSIVE\",\"nodeDescription\":\"the master Jenkins node\",\"nodeName\":\"\",\"numExecutors\":1,\"description\":null,\"jobs\":[{\"name\":\"TestProject-2011.3.1\",\"url\":\"http://buildserver:4567/jenkins/job/TestProject-2011.3.1/\",\"color\":\"blue\"},{\"name\":\"TestProject-2011.3.2\",\"url\":\"http://buildserver:4567/jenkins/job/TestProject-2011.3.2/\",\"color\":\"blue\"},{\"name\":\"TestProject-2012.1\",\"url\":\"http://buildserver:4567/jenkins/job/TestProject-2012.1/\",\"color\":\"blue\"},{\"name\":\"TestProject-dev\",\"url\":\"http://buildserver:4567/jenkins/job/TestProject-dev/\",\"color\":\"blue\"},{\"name\":\"TestProject-dev-nightly\",\"url\":\"http://buildserver:4567/jenkins/job/TestProject-dev-nightly/\",\"color\":\"disabled\"},{\"name\":\"TestProject-NONTEST\",\"url\":\"http://buildserver:4567/jenkins/job/TestProject-TESTSET/\",\"color\":\"disabled\"},{\"name\":\"TestProject-Dev-Test\",\"url\":\"http://buildserver:4567/jenkins/job/TestProject-Dev-Test/\",\"color\":\"disabled\"},{\"name\":\"P6Spy-dev\",\"url\":\"http://buildserver:4567/jenkins/job/SOMETHINGELSE-DEV/\",\"color\":\"blue\"}],\"overallLoad\":{},\"primaryView\":{\"name\":\"All\",\"url\":\"http://buildserver:4567/jenkins/\"},\"quietingDown\":false,\"slaveAgentPort\":0,\"useCrumbs\":false,\"useSecurity\":false,\"views\":[{\"name\":\"All\",\"url\":\"http://buildserver:4567/jenkins/\"}]}"};
char testJSON[] = {"{\"jobs\":[{\"name\":\"TestProject-2011.3.1\",\"color\":\"blue\"},{\"name\":\"TestProject-2011.3.2\",\"color\":\"blue\"},{\"name\":\"TestProject-2012.1\",\"color\":\"blue\"},{\"name\":\"TestProject-dev\",\"color\":\"red\"}]}"};

HughesyShiftBrite sb;
char commandBuffer[100] = {0};
char* knownColors[]={  "red", "green", "blue", "yellow", "cyan", "magenta", "off", "white" };
int components[][3]={  RED,  GREEN, BLUE, YELLOW, CYAN, MAGENTA, OFF, WHITE };
int pos = 0;


int run = 0;
//char** jsonFilter = {"jobs","name","color",NULL};
    
void setup()
{
  Serial.begin(9600);
  sb = HughesyShiftBrite(10,11,12,13);
  randomSeed(analogRead(0));
  sb.sendColour(1023,1023,1023);
  delay(200);
  sb.sendColour(0,0,0);
  Serial.print("Started Up");
}

void loop()
{
  if(!run){
    aJsonObject* root = aJson.parse(testJSON/*,jsonFilter*/);
  
    aJsonObject* jobs = aJson.getObjectItem(root, "jobs");
    unsigned char arraySize = aJson.getArraySize(jobs);
    for(unsigned char i = 0 ; i < arraySize ; i++){
        aJsonObject* job = aJson.getArrayItem(jobs, i);
        char* color = (aJson.getObjectItem(job, "color"))->valuestring;
        Serial.write("Found project ");
        Serial.write(aJson.print(aJson.getObjectItem(job, "name")));
        Serial.write(": ");
        Serial.println(color);
        for(int i = 0 ; i < KNOWN_COLORS_SIZE ; i++){
            if(strncmp(knownColors[i],color,3) == 0){
                sb.sendColour(components[i][0], components[i][1], components[i][2]);
                Serial.print("Setting to color ");
                Serial.write(knownColors[i]);
                break;
            }
        }
    }
    aJson.deleteItem(root);
  }
  run = 1;
}
