#ifndef ShiftBriteM_Animations_h
#define ShiftBriteM_Animations_h
#ifdef __cplusplus
 extern "C" {
#endif 

#include "Arduino.h"

void pulseAnimation(const uint16_t referenceColor[], uint16_t *currentStep, uint8_t lightUpdateFrequency, uint16_t colorsForCommand[], int *finished);
void noAnimation(const uint16_t referenceColor[], uint16_t *currentStep, uint8_t lightUpdateFrequency, uint16_t colorsForCommand[], int *finished);
void fastFlashAnimation(const uint16_t referenceColor[], uint16_t *currentStep, uint8_t lightUpdateFrequency, uint16_t colorsForCommand[], int *finished);

#ifdef __cplusplus
 }
#endif 
#endif
