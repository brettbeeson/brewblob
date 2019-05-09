#pragma once

#include <Arduino.h>
#include "Controller.h"
#include "Sensor.h"

/*
 * setpoint and deadband:
 * 
 *        setpoint
 *            V
 * 23         24          25
 * |<---- deadband ------->|
 * 
 * Cases:
 * Is OFF: If >25, turn ON
 * Is ON : If <23, turn OFF
 * Changes only allowed once per minute.
 * 
 * _on and _off are required as some relays as *active* when output is *low*
 */

class FridgeRelay : public Controller {
  public:
    FridgeRelay (Sensor *inputSensor, int outputRelayPin, float setpoint,float deadband);
    void begin();
    void end();
    void adjust();  // change relay state if required
    
    int _minChangeMs = 10 * 1000;  
    int _on = LOW;        // this value of digital outputs turns on *device* ...
    int  _off = HIGH;      // ... and this value of digital output turns it off.
    
  private:
    Sensor* _sensor = NULL;
    int _relayPin;
    bool _currentState;
    long _lastChangeMs;
    float _deadband=0;
    float _setpoint=0;
};
