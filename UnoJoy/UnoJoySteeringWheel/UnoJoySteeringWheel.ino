/* Arduino Gaming Steering Wheel and Pedal Set, using IMU sensor MPU6050
 * An additional improvement on UnoJoy project to work with Racing or Car Simulator Games.
 * Project by: M. Cem AVCI
 * Date: 17/06/2020 
 * cem.avci@gazi.edu.tr
 * 
 *******************************************************************************************
 * Functions Added to UnoJoyArduinoSample.ino: 
 * setupIMU function sets up MPU6050 to the max. of +- 500 degree rotation, a normal steering wheel rotates about +- 450 degree so thats fine.
 * rotAngle function actually calculates the rotation angle but remember the position of the IMU is critical. You may need to use different
 * gyro angle depending on how you made the setup.
 * once your setup is done, upload this code to arduino while centering the wheel. The rest of the process is the same as transforming into PS3 controller.
 * More details on:
 * https://www.youtube.com/watch?v=5q3cpu5xVu4
 */

#include <Wire.h>
#include <UnoJoy.h>

long gyroX,gyroY,gyroZ;
float rotAngle_X,rotAngle_Y,rotAngle_Z;


void setup() {
setupPins();
setupUnoJoy();
setupIMU();
Serial.begin(9600);



}

void loop() {
dataForController_t controllerData = getControllerData();
setControllerData(controllerData);


}
void setupPins(void){
  // Set all the digital pins as inputs
  // with the pull-up enabled, except for the 
  // two serial line pins
  for (int i = 2; i <= 12; i++){
    pinMode(i, INPUT);
    digitalWrite(i, HIGH);
  }
  pinMode(A4, INPUT);
  digitalWrite(A4, HIGH);
  pinMode(A5, INPUT);
  digitalWrite(A5, HIGH);
}

void setupIMU(void){
  Wire.begin();
  Wire.beginTransmission(0b1101000); 
  Wire.write(0x6B); 
  Wire.write(0b00000000); 
  Wire.endTransmission();  
  Wire.beginTransmission(0b1101000); 
  Wire.write(0x1B);  
  Wire.write(0x00001000); // -+ 500 degree
  Wire.endTransmission(); 
 
}


dataForController_t getControllerData(void){
  float steeringWheel=rotAngle();
  // Set up a place for our controller data
  //  Use the getBlankDataForController() function, since
  //  just declaring a fresh dataForController_t tends
  //  to get you one filled with junk from other, random
  //  values that were in those memory locations before
  dataForController_t controllerData = getBlankDataForController();
  // Since our buttons are all held high and
  //  pulled low when pressed, we use the "!"
  //  operator to invert the readings from the pins
  controllerData.triangleOn = !digitalRead(2);
  controllerData.circleOn = !digitalRead(3);
  controllerData.squareOn = !digitalRead(4);
  controllerData.crossOn = !digitalRead(5);
  controllerData.dpadUpOn = !digitalRead(6);
  controllerData.dpadDownOn = !digitalRead(7);
  controllerData.dpadLeftOn = !digitalRead(8);
  controllerData.dpadRightOn = !digitalRead(9);
  controllerData.l1On = !digitalRead(10);
  controllerData.r1On = !digitalRead(11);
  controllerData.selectOn = !digitalRead(12);
  controllerData.startOn = !digitalRead(A4);
  controllerData.homeOn = !digitalRead(A5);
  
  // Set the analog sticks
  //  Since analogRead(pin) returns a 10 bit value,
  //  we need to perform a bit shift operation to
  //  lose the 2 least significant bits and get an
  //  8 bit number that we can use  
  controllerData.leftStickX = steeringWheel;
  controllerData.leftStickY = analogRead(A0) >> 2;
  controllerData.rightStickX = analogRead(A1) >> 2;
  controllerData.rightStickY = analogRead(A2) >> 2;
  // And return the data!
  return controllerData;
}


float rotAngle(){  
  
  Wire.beginTransmission(0b1101000); 
  Wire.write(0x43); 
  Wire.endTransmission();
  Wire.requestFrom(0b1101000,6); 
  while(Wire.available() < 6){};
  gyroX = Wire.read()<<8|Wire.read(); 
  gyroY = Wire.read()<<8|Wire.read(); 
  gyroZ = Wire.read()<<8|Wire.read(); 
  
  rotAngle_X = gyroX / 655.0;
  rotAngle_Y = gyroY / 655.0; 
  rotAngle_Z = gyroZ / 655.0;
  
  rotAngle_Z = map(rotAngle_Z,-450,450,52,200);
  
  return rotAngle_Z;
  delay(10);
  }
