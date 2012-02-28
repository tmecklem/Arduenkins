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
}

void ShiftBriteM::setColor(uint8_t lightIndex, uint16_t r, uint16_t g, uint16_t b)
{
  if(lightIndex >= _numLights) {
    return;
  }
  
  _lights[lightIndex][0] = r;
  _lights[lightIndex][1] = g;
  _lights[lightIndex][2] = b;
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

  shiftOutWithDelay(_dPin, _cPin, MSBFIRST, _SB_CommandPacket >> 24, 1);
  shiftOutWithDelay(_dPin, _cPin, MSBFIRST, _SB_CommandPacket >> 16, 1);
  shiftOutWithDelay(_dPin, _cPin, MSBFIRST, _SB_CommandPacket >> 8, 1);
  shiftOutWithDelay(_dPin, _cPin, MSBFIRST, _SB_CommandPacket, 1);

}

void ShiftBriteM::_activate(){
  delay(1);
  digitalWrite(_lPin,HIGH);
  delay(1);
  digitalWrite(_lPin,LOW);
}

int ShiftBriteM::performNextStep(){
  if(_commandNeeded){
    for(int i = 0 ; i < _numLights ; i++){
      _sendCommand(_lights[i][0], _lights[i][1], _lights[i][2]);
    }
    _activate();
    _commandNeeded = 0;
  }
  return 2;
}
