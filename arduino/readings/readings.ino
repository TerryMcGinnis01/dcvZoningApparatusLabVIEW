/*
   Edited for personal use by: Zachery Nelson (04.05.2020)
   Reading CO2, humidity and temperature from the SCD30
   Original Creator: Nathan Seidle
   SparkFun Electronics
   Date: May 22nd, 2018
   License: MIT. See license file for more information but you can
   basically do whatever you want with this code.
   Feel like supporting open source hardware?
   Buy a board from SparkFun! https://www.sparkfun.com/products/15112
   This example prints the current CO2 level, relative humidity, and temperature in C.
   Hardware Connections:
   Attach RedBoard to computer using a USB cable.
   Connect SCD30 to RedBoard using Qwiic cable.
   Open Serial Monitor at 115200 baud.
 */

#include <Wire.h>

#include "SparkFun_SCD30_Arduino_Library.h" //Click here to get the library: http://librarymanager/All#SparkFun_SCD30

#define TCAADDR 0x70

void tcaselect(uint8_t i) {
        if (i > 7) return;

        Wire.beginTransmission(TCAADDR);
        Wire.write(1 << i);
        Wire.endTransmission();
}

SCD30 airSensor;

void setup()
{
        Serial.begin(115200);
        Serial.println("Begin Transmission");
        Wire.begin();
        tcaselect(3);
        if (airSensor.begin() == false)
        {
                Serial.println("Air sensor not detected. Please check wiring. Freezing...");
                while (1)
                        ;
        }
        for (int i = 1; i<8; i++) {
                tcaselect(i);
                if (airSensor.dataAvailable())
                {
                        airSensor.setAutoSelfCalibration(false);
                }
        }

        //The SCD30 has data ready every two seconds
}

void loop()
{
        for (int i = 1; i<8; i++) {
                tcaselect(i);
                if (airSensor.dataAvailable())
                {
                        Serial.print("c"); Serial.print(i-1); Serial.print(", "); Serial.println(airSensor.getCO2());
                        delay(100);
                        Serial.print("t"); Serial.print(i-1); Serial.print(", "); Serial.println(airSensor.getTemperature(), 1);
                        delay(100);
                        Serial.print("h"); Serial.print(i-1); Serial.print(", "); Serial.println(airSensor.getHumidity(), 1);
                        delay(150);
                }
        }
}
