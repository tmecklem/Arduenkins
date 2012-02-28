#include "Arduino.h"
#include "ShiftBriteM.h"

ShiftBriteM::ShiftBriteM()
{
  ShiftBriteM(1,5,6,7,8);
}

ShiftBriteM::ShiftBriteM(uint8_t numLights, uint8_t dataPin, uint8_t latchPin, uint8_t enablePin, uint8_t clockPin)
{
  _numLights = numLights > MAX_LIGHTS ? MAX_LIGHTS : numLights;

  _dPin = dataPin;
  _lPin = latchPin;
  _ePin = enablePin;
  _cPin = clockPin;
  
  pinMode(_dPin, OUTPUT);
  pinMode(_lPin, OUTPUT);
  pinMode(_ePin, OUTPUT);
  pinMode(_cPin, OUTPUT);
  
  digitalWrite(_lPin, LOW);
  digitalWrite(_ePin, LOW);
  
  _commandNeeded = 1;
  
  for(int i = 0 ; i < MAX_LIGHTS ; i++){
    _currentStep[i] = 0;
    _animationFunction[i] = NULL;
  }
}

void ShiftBriteM::setColor(uint8_t lightIndex, uint16_t r, uint16_t g, uint16_t b)
{
  setColor(lightIndex, r, g, b, NULL);
}

void ShiftBriteM::setColor(uint8_t lightIndex, uint16_t r, uint16_t g, uint16_t b, animationFunctionPtr animationFunction)
{
  if(lightIndex >= _numLights) {
    return;
  }
  
  _lights[lightIndex][0] = r;
  _lights[lightIndex][1] = g;
  _lights[lightIndex][2] = b;
  _currentStep[lightIndex] = 0;
  _animationFunction[lightIndex] = animationFunction;
  _commandNeeded = 1;
}


void ShiftBriteM::_sendCommand(uint16_t r, uint16_t g, uint16_t b) {
if (r <= 1023 && g <= 1023 && b <= 1023) 
  {
    _SB_CommandMode = B00;
    _SB_RedCommand = r;
    _SB_GreenCommand = g;
    _SB_BlueCommand = b;
    _SB_SendPacket();
  }
  else
  {
    //FLASH RED 7 times for error
    _SB_CommandMode = B00;
    _SB_RedCommand = 0;
    _SB_GreenCommand = 0;
    _SB_BlueCommand = 0;
    _SB_SendPacket();
    
    for(int i = 0; i < 7; i++)
    {
      //delay(100);
      _SB_RedCommand = 1023;
      _SB_SendPacket();
      //delay(100);
      _SB_RedCommand = 0;
      _SB_SendPacket();
    }
  }
}

void shiftOutWithDelay(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, byte val, uint8_t delayVal)
{
  int i;

  for (i = 0; i < 8; i++)  {
    if (bitOrder == LSBFIRST)
      digitalWrite(dataPin, !!(val & (1 << i)));
    else
      digitalWrite(dataPin, !!(val & (1 << (7 - i))));

    digitalWrite(clockPin, HIGH);
    delay(delayVal);
    digitalWrite(clockPin, LOW);
  }
}

void ShiftBriteM::_SB_SendPacket() 
{
  _SB_CommandPacket = _SB_CommandMode & B11;
  _SB_CommandPacket = (_SB_CommandPacket << 10)  | (_SB_BlueCommand & 1023);
  _SB_CommandPacket = (_SB_CommandPacket << 10)  | (_SB_RedCommand & 1023);
  _SB_CommandPacket = (_SB_CommandPacket << 10)  | (_SB_GreenCommand & 1023);

  shiftOutWithDelay(_dPin, _cPin, MSBFIRST, _SB_CommandPacket >> 24, 0);
  shiftOutWithDelay(_dPin, _cPin, MSBFIRST, _SB_CommandPacket >> 16, 0);
  shiftOutWithDelay(_dPin, _cPin, MSBFIRST, _SB_CommandPacket >> 8, 0);
  shiftOutWithDelay(_dPin, _cPin, MSBFIRST, _SB_CommandPacket, 0);

}

void ShiftBriteM::_activate(){
  delay(1);
  digitalWrite(_lPin,HIGH);
  delay(1);
  digitalWrite(_lPin,LOW);
}

int ShiftBriteM::performNextStep(){
  uint16_t colorCommand[3] = {0};
  if(_commandNeeded){
    _commandNeeded = 0;
    for(int i = 0 ; i < _numLights ; i++){
      colorCommand[0] = _lights[i][0];
      colorCommand[1] = _lights[i][1];
      colorCommand[2] = _lights[i][2];
      if(_animationFunction[i] != NULL){
        _commandNeeded = 1; //reset the bit since we're using animation functions
        int finished = 0;
        _animationFunction[i](_lights[i], &_currentStep[i], colorCommand, &finished);
        _currentStep[i]++;
        if(finished){
          _currentStep[i] = 0;
          _animationFunction[i] = NULL;
        }
      }
      _sendCommand(colorCommand[0], colorCommand[1], colorCommand[2]);
    }
    _activate();
  }
  return 2;
}
