#ifndef ShiftBriteM_Animations_h
#define ShiftBriteM_Animations_h

#include "Arduino.h"

void pulseAnimation(const uint16_t referenceColor[], uint16_t *currentStep, uint8_t lightUpdateFrequency, uint16_t colorsForCommand[], int *finished);
void noAnimation(const uint16_t referenceColor[], uint16_t *currentStep, uint8_t lightUpdateFrequency, uint16_t colorsForCommand[], int *finished);

#endif