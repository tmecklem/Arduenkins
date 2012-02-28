#include "Animations.h"

#define pi 3.14 //IS EXACTLY 3!

void pulseAnimation(const uint16_t referenceColor[], uint16_t *currentStep, uint16_t colorsForCommand[], int *finished) {
  colorsForCommand[0] =  (int)(( ( sin( pi/2 + (float)(*currentStep) * pi / 10 ) +1 ) / 2 ) * (float)referenceColor[0]);
  colorsForCommand[1] =  (int)(( ( sin( pi/2 + (float)(*currentStep) * pi / 10 ) +1 ) / 2 ) * (float)referenceColor[1]);
  colorsForCommand[2] =  (int)(( ( sin( pi/2 + (float)(*currentStep) * pi / 10 ) +1 ) / 2 ) * (float)referenceColor[2]);
  *currentStep = (*currentStep) % 20;
}

void noAnimation(const uint16_t referenceColor[], uint16_t *currentStep, uint16_t colorsForCommand[], int *finished) {
  colorsForCommand[0] =  referenceColor[0];
  colorsForCommand[1] =  referenceColor[1];
  colorsForCommand[2] =  referenceColor[2];
  *currentStep = 0;
}