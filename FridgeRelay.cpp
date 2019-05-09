#include <ArduinoLog.h>
#include "FridgeRelay.h"
#include "OLED.h"

FridgeRelay :: FridgeRelay (Sensor *inputSensor, int outputRelayPin, float setpoint, float deadband):
  _relayPin(outputRelayPin),
  _sensor(inputSensor),
  _setpoint(setpoint),
  _deadband(deadband),
  _currentState(_off),
  _lastChangeMs(0)
{
}

void FridgeRelay :: begin() {
  pinMode(_relayPin, OUTPUT);
}

void FridgeRelay :: end() {
  _sensor = NULL;
}

void FridgeRelay :: adjust() {
   /*
digitalWrite(_relayPin, HIGH);
Log.warning("Set relay (pin %d) to: %d\n", _relayPin, _currentState);
delay(1000);
digitalWrite(_relayPin, LOW);
delay(1000);
return;
*/
  try {
    Reading* r = _sensor->getReadingPtr(0);
    if (!r) {
      Log.error("No temp reading available for FridgeRelay\n");
      return;
    }
    if (_lastChangeMs == 0) {
      _lastChangeMs = millis();
      if (r->value <= _setpoint) {
        _currentState = _off;
        OLED.message("FridgeRelay: OFF");
      } else {
        _currentState = _on;
        OLED.message("FridgeRelay: ON");
      }
      digitalWrite(_relayPin, _currentState);
      Log.notice("First run, set relay (pin %d) to: %d as temp: %s\n", _relayPin, _currentState, r->shortStr().c_str());
      return;
    }
    //Log.verbose("FridgeRelay :: adjust(): using Reading: %s\n", r->tostr().c_str());
    if (millis() > _lastChangeMs + _minChangeMs) {
      if (_currentState == _on) {
        if (r->value < _setpoint - _deadband / 2) {
          _currentState = _off;
          _lastChangeMs = millis();
          digitalWrite(_relayPin, _currentState);
          Log.notice("Set relay (pin %d) to: %d as temp: %s\n", _relayPin, _currentState, r->shortStr().c_str());
          OLED.message("FridgeRelay: OFF");
        }
      } else if (_currentState == _off) {
        if (r->value > _setpoint + _deadband / 2) {
          _currentState = _on;
          _lastChangeMs = millis();
          digitalWrite(_relayPin, _on);
          Log.notice("Set relay (pin %d) to: %d as temp: %s\n", _relayPin, _currentState, r->shortStr().c_str());
          OLED.message("FridgeRelay: ON");
        }
      }
    }
  } catch (std::exception& e) {
    Log.error("Exception: %s", e.what());
  }
}
