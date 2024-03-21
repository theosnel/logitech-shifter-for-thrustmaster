/*  Logitech Driving Force Shifter to Thrustmaster T300 converter
 *  Theo Snelleman
 *  
 *  Based on:
 *  USB Shifter to Thrustmaster Wheelbase (https://github.com/azzajess/USB-Shifter-to-Thrustmaster-Wheelbase)
 *  
 *  Simplified Thrustmaster T.16000M FCS Joystick Report Parser (https://github.com/felis/USB_Host_Shield_2.0/blob/master/examples/HID/t16km/t16km.ino)
 *  &
 *  People at ISRTV.com - https://www.isrtv.com/forums/topic/24532-gearbox-connector-on-base/
 *  
 *  Modified by azzajess to suit USB Shifter 6+R+Switch (https://github.com/azzajess/USB-Shifter-to-Thrustmaster-Wheelbase)
 *  v1.2
*/


#include <SPI.h>
#include <Wire.h>

const int xPin = 34;
const int yPin = 35;
const int revPin = 33;

#define LED 2

int currGear = 0;
int lastGear = 0;

bool toggle = false;

int readGear() {
  /* Output of my shifter
   * X: 912   -   1840      - 2850
   * Y: 3613  - 1870        - 317
  */
  
  int x = analogRead(xPin);
  int y = analogRead(yPin);

  bool rev = digitalRead(revPin);

  if (y > 2400) {
    // 1, 3, 5
    if (x < 1350) {
      return 1;
    }
    if (x > 2350) {
      return 5;
    }
    return 3;
  }
  if (y < 1000) {
    if (x < 1350) {
      return 2;
    }
    if (x > 2350) {
      if (rev) {
        return -1;
      }
      return 6;
    }
    return 4;
  }
  return 0;
}

byte command[14] = {
  0x00, // Shifter mode 0 - S / 0x80 - H
  0x0C, // Unknown
  0x01, // Unknown
  0x00, // Gear in H-mode
  0x00, // Gear in S-Mode 0x04 - center, 0x05 - down, 0x06 - up
  0x80, // Unknown
  0x80, // Unknown
  0x00, // Y cordinate
  0x00, // X cordinate
  0x00, // Unknown
  0x00, // Unknown
  0x00, // Unknown
  0x00, // Unknown
  0x00  // Unknown
};


//H mode shifting codes
void setHMode(bool isHMode) {
  if (isHMode) {
    command[0] |= 0x80;
  } else {
    command[0] &= ~0x80;
  }
}

//Switchs gear to specificed gear number and display in serial monitor
void switchHGear(byte gear) { // Gear num 0-N, 8-R
  command[3] = (0x80 >> (8-gear));
}

//sends command over PS2 port to T300 wheelbase
void sendCommand() {
  Wire.beginTransmission(0x01);
  Wire.write(command, 14);
  Wire.endTransmission();
}


void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(revPin, INPUT);
  pinMode(LED,OUTPUT);

  Wire.begin();
  setHMode(true);
  switchHGear(0);
  sendCommand();
}


void loop() {
  int gear=readGear();
  if (gear != currGear && !(gear == 6 && currGear == 7)) {
    if (gear == -1) {
      lastGear = -1;
    }
    // When shifting from 6th to 6th gear, go to 7th gear
    if (gear == 6) {
      if (lastGear == 6) {
        gear = 7;
      }
    }
    lastGear = currGear;
    
    Serial.print("Gear: ");
    Serial.println(gear);
    currGear = gear;

    byte newGear = gear;
    if (gear == -1) {
      newGear = 8;
    }
    switchHGear(newGear);
    sendCommand();
    toggle = !toggle;
    digitalWrite(LED, toggle);
  }
  
  /*
  // Dump values to adjust and debug
  int potValue = analogRead(xPin);
  Serial.print("X: ");
  Serial.print(potValue);
  Serial.print("  Y: ");
  Serial.print(analogRead(yPin));
  Serial.print("  Rev: ");
  Serial.println(digitalRead(revPin));
  Serial.print("Gear: ");
  Serial.println(readGear());
  delay(500);*/
}
