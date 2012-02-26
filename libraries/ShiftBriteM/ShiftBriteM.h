#ifndef ShiftBriteM_h
#define ShiftBriteM_h

#include "Arduino.h"

class ShiftBriteM
{
public:
  ShiftBriteM();
  ShiftBriteM(int dataPin, int latchPin, int enablePin, int clockPin);
  void sendColour(int r, int g, int b);
  void activate();
private:
  int _SB_CommandMode;
  int _SB_RedCommand;
  int _SB_GreenCommand;
  int _SB_BlueCommand;
  int _dPin;
  int _lPin;
  int _ePin;
  int _cPin;
  unsigned long _SB_CommandPacket;
  void _SB_SendPacket();
};

#endif