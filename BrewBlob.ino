#include "LoRa.h" // custom 
#include "esp_task_wdt.h"
#include "Blob.h"
#include "OLED.h"
#include "LoraPublisher.h"
#include "DSTempSensors.h"
#include "BlobSensor.h"
#include "FridgeRelay.h"
#include "MQ3Sensor.h"
#include "LoraReader.h"

using namespace std;

#define MQ3PIN 4
#define OLED_FRAME_PIN 12
#define OLED_READING_PIN 14
#define RELAY_PIN 2  
#define DS_PIN 15
#define SDA 21
#define SCL 22
#define OLEDPIN 16

int channel = 0;
int setpoint = 18;
int deadband = 1;

RTC_DATA_ATTR static int bootCount = 0;
long startTime = 0;
const long periodMs = 10000;

Blob blob;

DSTempSensors ds(blob.readingsQueue, DS_PIN);
BlobSensor ts(blob.readingsQueue);
MQ3Sensor booze(blob.readingsQueue, MQ3PIN);
FridgeRelay fridgeRelay(&ds, RELAY_PIN, setpoint, deadband);
LoraPublisher loraPub(blob.readingsQueue);

void setup() {
  bootCount++;
  Serial.begin(115200); while (!Serial) {};
  Log.begin (LOG_LEVEL_NOTICE, &Serial); // SILENT|FATAL|ERROR|WARNING|NOTICE|TRACE|VERBOSE
  Log.notice("Setup executing on core %d, bootCount=%d reset reason 0,1=%s,%s\n", xPortGetCoreID(), bootCount, resetReasonStr(rtc_get_reset_reason(0)).c_str(), resetReasonStr(rtc_get_reset_reason(1)).c_str());

  try {
    OLED.setTargetFPS(10);
    OLED.begin(OLEDPIN, SDA, SCL);   // starts Wire

    OLED.message("Brew Blob!");
    OLED.setFramesToDisplay(OLEDClass::MessagesMask);

    OLED.setFramesToDisplay(OLEDClass::ReadingsMask + OLEDClass::MessagesMask);
    OLED.setButtons( OLED_FRAME_PIN, OLED_READING_PIN, 20 /* threshold */);

    ds.showOnOLED = true;
    booze.showOnOLED = true;

    blob.addSensor(&ds);
    blob.addSensor(&ts);
    blob.addSensor(&booze);

    blob.setupLora();
    //blob.setupI2CBus(); OLED.begin already does this
    blob.addPublisher(&loraPub);

    blob.begin();
    Log.verbose("Blob: %s",blob.toString().c_str());
    fridgeRelay.showOnOLED = true;
    blob.addController(&fridgeRelay);
    blob.addPublisher(&loraPub);

  } catch (const runtime_error& e) {
    Log.fatal("Failed setup. Exception: %s", e.what());
    OLED.message("Setup failed:"); OLED.message(e.what());
    delay(10000);
    ESP.restart();
  }
  // Log.trace("Setup executing on core %d\n", xPortGetCoreID());
  //pinMode(2,OUTPUT);
  // 
  fridgeRelay.begin();
}


void loop(void) {
  long sleepTime;

  try {
    
    //digitalWrite(2,HIGH);
    
    startTime = millis();
    blob.readSensors();
    blob.adjustControllers();
    
    blob.publish();
    
    //Log.verbose("Booze: %s\n", booze.getReadingPtr(0)->tostr().c_str());
    //Log.verbose("Temp: %s\n", ds.getReadingPtr(0)->tostr().c_str());
    sleepTime = periodMs - (millis() - startTime);
    if (sleepTime > 0) {
      // Log.verbose("Sleeping for %d ms, ran for %d ms\n", sleepTime, millis() - startTime);
      delay(sleepTime);
    } else {
      Log.notice ("Overloaded... no sleep.\n");
    }
  } catch (const runtime_error& e) {
    Log.fatal("Failed loop. Exception: %s\n", e.what());
    OLED.message("Exception: "); OLED.message(e.what());
    delay(60000);
    ESP.restart();
  }
}
