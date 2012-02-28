#ifndef ShiftBriteM_h
#define ShiftBriteM_h

#define MAX_LIGHTS 10
//function is responsible for updating the rgb values in colorsForCommand (out variable) and indicating that it's finished with a true value. Finished animations will be removed from the animation list and the last value kept. referenceColor[] is the array of rgb values used in the setColor command. CurrentStep can be modified if necessary to avoid overflow for looping animations
typedef void (*animationFunctionPtr)(const uint16_t referenceColor[], uint16_t *currentStep, uint8_t lightUpdateFrequency, uint16_t colorsForCommand[], int *finished);

#include "Arduino.h"

class ShiftBriteM
{
public:
  ShiftBriteM();
  ShiftBriteM(uint8_t numLights, uint8_t dataPin, uint8_t latchPin, uint8_t enablePin, uint8_t clockPin, uint8_t lightUpdateFrequency);
  void setColor(uint8_t lightIndex, uint16_t r, uint16_t g, uint16_t b, animationFunctionPtr);
  void setColor(uint8_t lightIndex, uint16_t r, uint16_t g, uint16_t b);
  
  int performNextStep();
private:

  int _commandNeeded;

  void _sendCommand(uint16_t r, uint16_t g, uint16_t b);
  uint16_t _lights[MAX_LIGHTS][3];
  uint16_t _currentStep[MAX_LIGHTS];
  animationFunctionPtr _animationFunction[MAX_LIGHTS];

  uint8_t _numLights;
  uint8_t _lightUpdateFrequency;

  uint8_t _dPin;
  uint8_t _lPin;
  uint8_t _ePin;
  uint8_t _cPin;
  
  uint8_t _SB_CommandMode;
  uint16_t _SB_RedCommand;
  uint16_t _SB_GreenCommand;
  uint16_t _SB_BlueCommand;
  unsigned long _SB_CommandPacket;
  void _SB_SendPacket();
  void _activate();
};

#endif