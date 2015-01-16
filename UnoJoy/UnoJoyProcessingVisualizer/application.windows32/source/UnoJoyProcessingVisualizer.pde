/* Code for UnoJoy Processing Visualizer

    Wouter Walmink + Alan Chatham
    Copyright 2012-2013
    This code is released under the GPL v3
    
    Code for dropdown menu from
    http://forum.processing.org/topic/simple-code-create-a-drop-down-list-with-available-com-ports
*/
import processing.serial.*;
// Dropdown menu stuff
ControlP5 controlP5;             //Define the variable controlP5 as a ControlP5 type.
DropdownList ports;              //Define the variable ports as a Dropdownlist.
int Ss;                          //The dropdown list will return a float value, which we will connvert into an int. we will use this int for that).
String[] comList ;               //A string to hold the ports in.
boolean serialSet;               //A value to test if we have setup the Serial port.
boolean Comselected = false;     //A value to test if you have chosen a port in the list.

// Game controller image
PImage controller;
int imageWidth = 516;
int imageHeight = 521;
Point offset = new Point( 40, 40 );

int stickIndicatorSize = 20;
int actionIndicatorSize = 35;
int functionIndicatorSize = 35;
int dpadIndicatorSize = 50;

int stickDiameter = int( 0.134 * imageWidth );

// Indicator positions
Point leftStickPos = new Point( 0.351*imageWidth + offset.x, 0.581*imageWidth + offset.y );
Point rightStickPos = new Point( 0.642*imageWidth + offset.x, 0.581*imageWidth + offset.y );

Point trianglePos = new Point( 0.795*imageWidth + offset.x, 0.357*imageWidth + offset.y );
Point circlePos = new Point( 0.868*imageWidth + offset.x, 0.430*imageWidth + offset.y );
Point squarePos = new Point( 0.721*imageWidth + offset.x, 0.430*imageWidth + offset.y );
Point crossPos = new Point( 0.795*imageWidth + offset.x, 0.504*imageWidth + offset.y );

Point l1Pos = new Point( 0.201*imageWidth + offset.x, 0.068*imageWidth + offset.y );
Point l2Pos = new Point( 0.201*imageWidth + offset.x, 0.156*imageWidth + offset.y );
Point l3Pos = new Point( leftStickPos.x,leftStickPos.y );

Point r1Pos = new Point( 0.795*imageWidth + offset.x, 0.068*imageWidth + offset.y );
Point r2Pos = new Point( 0.795*imageWidth + offset.x, 0.156*imageWidth + offset.y );
Point r3Pos = new Point( rightStickPos.x,rightStickPos.y );

Point selectPos = new Point( 0.405*imageWidth + offset.x, 0.446*imageWidth + offset.y );
Point startPos = new Point( 0.583*imageWidth + offset.x, 0.446*imageWidth + offset.y );
Point homePos = new Point( 0.502*imageWidth + offset.x, 0.518*imageWidth + offset.y );

Point dpadLeftPos = new Point( 0.130*imageWidth + offset.x, 0.426*imageWidth + offset.y );
Point dpadUpPos = new Point( 0.200*imageWidth + offset.x, 0.357*imageWidth + offset.y );
Point dpadRightPos = new Point( 0.270*imageWidth + offset.x, 0.427*imageWidth + offset.y );
Point dpadDownPos = new Point( 0.200*imageWidth + offset.x, 0.500*imageWidth + offset.y );

// joystick data
Point leftStickData = new Point( 128,128 );
Point rightStickData = new Point( 128,128 );

// Control flags
int triangleOn = 0;
int circleOn = 0;
int squareOn = 0;
int crossOn = 0;

int l1On = 0;
int l2On = 0;
int l3On = 0;

int r1On = 0;
int r2On = 0;
int r3On = 0;

int selectOn = 0;
int startOn = 0;
int homeOn = 0;

int dpadLeftOn = 0;
int dpadUpOn = 0;
int dpadRightOn = 0;
int dpadDownOn = 0;


int SerialPortLoaded = 0;
Serial SerialPort;

PFont font;

//
/// SETUP
//
void setup() {
  if (Serial.list().length > 0){
    SerialPortLoaded = 1;
    String portName = Serial.list()[0];
    println(portName);
    SerialPort = new Serial(this, portName, 38400);
  }
  font = loadFont("AngsanaNew-28.vlw");
  size( 600,600 );
  controlP5 = new ControlP5(this);
  rectMode( CENTER );
  ellipseMode( CENTER );
  smooth();

  // load image
  controller = loadImage( "data/game_controller.png" );
  // We need to wait a bit, otherwise the Arduino
  //  won't be ready for serial data on OSX.
  //  delay() doesn't work in a setup function in
  //  Processing, so we do it the real way
  try {
   Thread.sleep(1000);
  } catch (InterruptedException e) { }

//Make a dropdown list calle ports. Lets explain the values: ("name", left margin, top margin, width, height (84 here since the boxes have a height of 20, and theres 1 px between each item so 4 items (or scroll bar).
  ports = controlP5.addDropdownList("list-1",10,25,100,84);
  //Setup the dropdownlist by using a function. This is more pratical if you have several list that needs the same settings.
  customize(ports); 
} // End of Setup

//The dropdown list returns the data in a way, that i dont fully understand, again mokey see monkey do. However once inside the two loops, the value (a float) can be achive via the used line ;).
void controlEvent(ControlEvent theEvent) {
  if (theEvent.isGroup()) 
  {
    //Store the value of which box was selected, we will use this to acces a string (char array).
    float S = theEvent.group().value();
    //Since the list returns a float, we need to convert it to an int. For that we us the int() function.
    Ss = int(S);
    //With this code, its a one time setup, so we state that the selection of port has been done. You could modify the code to stop the serial connection and establish a new one.
    SerialPort.stop();
      // We need to wait a bit, otherwise the Arduino
    //  won't be ready for serial data on OSX.
    //  delay() doesn't work in a setup function in
    //  Processing, so we do it the real way
    try {
     Thread.sleep(500);
    } catch (InterruptedException e) { }
    SerialPort = new Serial(this, Serial.list()[Ss], 38400);
    println(Serial.list()[Ss]);
    Comselected = true;
  }
}

//here we setup the dropdown list.
void customize(DropdownList ddl) {
  //Set the background color of the list (you wont see this though).
  ddl.setBackgroundColor(color(200));
  //Set the height of each item when the list is opened.
  ddl.setItemHeight(20);
  //Set the height of the bar itself.
  ddl.setBarHeight(15);
  //Set the lable of the bar when nothing is selected.
  ddl.captionLabel().set("Select COM port");
  //Set the top margin of the lable.
  ddl.captionLabel().style().marginTop = 3;
  //Set the left margin of the lable.
  ddl.captionLabel().style().marginLeft = 3;
  //Set the top margin of the value selected.
  ddl.valueLabel().style().marginTop = 3;
  //Store the Serial ports in the string comList (char array).
  comList = SerialPort.list();
  if (Serial.list().length == 0)
    return;
  //We need to know how many ports there are, to know how many items to add to the list, so we will convert it to a String object (part of a class).
  String comlist = join(comList, ",");
  //We also need how many characters there is in a single port name, we´ll store the chars here for counting later.
  String COMlist = comList[0];
  //Here we count the length of each port name.
  int size2 = COMlist.length();
  //Now we can count how many ports there are, well that is count how many chars there are, so we will divide by the amount of chars per port name.
  int size1 = comlist.length() / size2;
  //Now well add the ports to the list, we use a for loop for that. How many items is determined by the value of size1.
  for(int i=0; i< size1; i++)
  {
    //This is the line doing the actual adding of items, we use the current loop we are in to determin what place in the char array to access and what item number to add it as.
    ddl.addItem(comList[i],i);
  }
  //Set the color of the background of the items and the bar.
  ddl.setColorBackground(color(60));
  //Set the color of the item your mouse is hovering over.
  ddl.setColorActive(color(255,128));
}


/**
 * DRAW
 */
 
  int counter = 0;
void draw() {
  // If the controller is connected,
  //  read the data in
  if (SerialPortLoaded == 1){
    readController();
  }

  background( 255 );
  image( controller, offset.x, offset.y );
  
  if(SerialPortLoaded == 0){
    textFont(font, 28);
    fill(250,0,0);
    text("Couldn't connect to the Arduino...", 175, 500);
  }
  
  
  noStroke();
  fill( 255, 0, 0, 100 );
    
  /// L & R BUTTONS ///
  if( l1On == 1 )
    ellipse( l1Pos.x, l1Pos.y, actionIndicatorSize, actionIndicatorSize );
  if( r1On == 1 )
    ellipse( r1Pos.x, r1Pos.y, actionIndicatorSize, actionIndicatorSize ); 
  if( l2On == 1 )
    ellipse( l2Pos.x, l2Pos.y, actionIndicatorSize, actionIndicatorSize );    
  if( r2On == 1 )
    ellipse( r2Pos.x, r2Pos.y, actionIndicatorSize, actionIndicatorSize ); 
  if( l3On == 1 )
    ellipse( l3Pos.x, l3Pos.y, stickDiameter, stickDiameter );
  if( r3On == 1 )
    ellipse( r3Pos.x, r3Pos.y, stickDiameter, stickDiameter ); 
//
  // STICKS //
  
  // LEFT
  
  strokeWeight( 1 );
  stroke( 255, 0, 0 );
  fill( 255,255,255, 100 );
  rect( leftStickPos.x, leftStickPos.y, stickDiameter, stickDiameter );
  
  //leftStickData.x = float(mouseX) / 600 * 255;
  //leftStickData.y = float(mouseY) / 600 * 255;
  
  stroke( 255, 0, 0 );
  strokeWeight( 2 );  
  
  line( leftStickPos.x + stickDiameter * (leftStickData.x-128)/255 - stickIndicatorSize/2, 
        leftStickPos.y + stickDiameter * (leftStickData.y-128)/255, 
        leftStickPos.x + stickDiameter * (leftStickData.x-128)/255 + stickIndicatorSize/2, 
        leftStickPos.y + stickDiameter * (leftStickData.y-128)/255  );
  
  line( leftStickPos.x + stickDiameter * (leftStickData.x-128)/255, 
        leftStickPos.y + stickDiameter * (leftStickData.y-128)/255 - stickIndicatorSize/2, 
        leftStickPos.x + stickDiameter * (leftStickData.x-128)/255, 
        leftStickPos.y + stickDiameter * (leftStickData.y-128)/255 + stickIndicatorSize/2  );
  
  noStroke();
  fill( 255, 0, 0, 100 );
  
  ellipse( leftStickPos.x + stickDiameter * (leftStickData.x-128)/255, 
           leftStickPos.y + stickDiameter * (leftStickData.y-128)/255, 
           stickIndicatorSize/1.5, stickIndicatorSize/1.5 );
  
  // RIGHT
  
  strokeWeight( 1 );
  stroke( 255, 0, 0 );
  fill( 255,255,255, 100 );
  rect( rightStickPos.x, rightStickPos.y, stickDiameter, stickDiameter );
  
 // rightStickData.x = float(mouseX) / 600 * 255;
 // rightStickData.y = float(mouseY) / 600 * 255;
  
  stroke( 255, 0, 0 );
  strokeWeight( 2 );  
  
  line( rightStickPos.x + stickDiameter * (rightStickData.x-128)/255 - stickIndicatorSize/2, 
        rightStickPos.y + stickDiameter * (rightStickData.y-128)/255, 
        rightStickPos.x + stickDiameter * (rightStickData.x-128)/255 + stickIndicatorSize/2, 
        rightStickPos.y + stickDiameter * (rightStickData.y-128)/255  );
  
  line( rightStickPos.x + stickDiameter * (rightStickData.x-128)/255, 
        rightStickPos.y + stickDiameter * (rightStickData.y-128)/255 - stickIndicatorSize/2, 
        rightStickPos.x + stickDiameter * (rightStickData.x-128)/255, 
        rightStickPos.y + stickDiameter * (rightStickData.y-128)/255 + stickIndicatorSize/2  );
  
  noStroke();
  fill( 255, 0, 0, 100 );
  
  ellipse( rightStickPos.x + stickDiameter * (rightStickData.x-128)/255, 
           rightStickPos.y + stickDiameter * (rightStickData.y-128)/255, 
           stickIndicatorSize/1.5, stickIndicatorSize/1.5 );
  
  /// ACTION BUTTONS ***
  // triangle
  if( triangleOn == 1 )
    ellipse( trianglePos.x, trianglePos.y, actionIndicatorSize, actionIndicatorSize );
  // circle
  if( circleOn == 1 )
    ellipse( circlePos.x, circlePos.y, actionIndicatorSize, actionIndicatorSize );
  // square
  if( squareOn == 1 )
    ellipse( squarePos.x, squarePos.y, actionIndicatorSize, actionIndicatorSize );
  // cross
  if( crossOn == 1 )
    ellipse( crossPos.x, crossPos.y, actionIndicatorSize, actionIndicatorSize );
  
  // SPECIAL BUTTONS //
  
  // select
  if( selectOn == 1 )
    ellipse( selectPos.x, selectPos.y, functionIndicatorSize, functionIndicatorSize );
    
  // start
  if( startOn == 1 )
    ellipse( startPos.x, startPos.y, functionIndicatorSize, functionIndicatorSize );

   // home
  if( homeOn == 1 )
    ellipse( homePos.x, homePos.y, functionIndicatorSize, functionIndicatorSize );
    
    
  // DPAD BUTTONS //
  
  // left
  if( dpadLeftOn == 1 )
    ellipse( dpadLeftPos.x, dpadLeftPos.y, dpadIndicatorSize, dpadIndicatorSize );
  
  // up
  if( dpadUpOn == 1 )
    ellipse( dpadUpPos.x, dpadUpPos.y, dpadIndicatorSize, dpadIndicatorSize );
    
  // right
  if( dpadRightOn == 1 )
    ellipse( dpadRightPos.x, dpadRightPos.y, dpadIndicatorSize, dpadIndicatorSize );
    
  // down
  if( dpadDownOn == 1 )
    ellipse( dpadDownPos.x, dpadDownPos.y, dpadIndicatorSize, dpadIndicatorSize );

}

// This waits 'timeout' ms for data from the serial port.
//  It returns the serial data if it didn't timeout,
//  and returns defaultValue if it did timeout.
// Using this prevents the serial port from holding
//  up the program in case of a transmission error.
int readSerialPort(int timeout, int defaultValue){
  while((SerialPort.available() == 0) && (timeout > 0)){
    timeout--;
    delay(1);
  }
  if (timeout <= 0)
    return defaultValue;
  return SerialPort.read();
}

void readController(){
  SerialPort.clear();
  SerialPort.write(0);
  // Wait 25ms to get in data, or make these buttons zeroes
  int buttonData1 = readSerialPort(25, 0);
  
  SerialPort.write(1);
  // Wait 25ms to get in data, or make these buttons zeroes
  int buttonData2 = readSerialPort(25, 0);
  
  SerialPort.write(2);
  // Wait 25ms to get in data, or make these buttons zeroes
  int buttonData3 = readSerialPort(25, 0);
  
  SerialPort.write(3);
  // Wait 25ms to get in data, or make these buttons zeroes
  leftStickData.x = readSerialPort(25, 128);
  
  SerialPort.write(4);
  // Wait 25ms to get in data, or make these buttons zeroes
  leftStickData.y = readSerialPort(25, 128);
  
  SerialPort.write(5);
  // Wait 25ms to get in data, or make these buttons zeroes
  rightStickData.x = readSerialPort(25, 128);
  
  // Wait 25ms to get in data, or make these buttons zeroes
  rightStickData.y = readSerialPort(25, 128);
  
  // Now assign the buttons based on the data
  triangleOn = 1 & (buttonData1 >> 0);
  circleOn = 1 & (buttonData1 >> 1);
  squareOn = 1 & (buttonData1 >> 2);
  crossOn = 1 & (buttonData1 >> 3);
  l1On = 1 & (buttonData1 >> 4);
  l2On = 1 & (buttonData1 >> 5);
  l3On = 1 & (buttonData1 >> 6);
  r1On = 1 & (buttonData1 >> 7);
  		
  r2On = 1 & (buttonData2 >> 0);
  r3On = 1 & (buttonData2 >> 1);
  selectOn = 1 & (buttonData2 >> 2);
  startOn = 1 & (buttonData2 >> 3);
  homeOn = 1 & (buttonData2 >> 4);
  dpadLeftOn = 1 & (buttonData2 >> 5);
  dpadUpOn = 1 & (buttonData2 >> 6);
  dpadRightOn = 1 & (buttonData2 >> 7);
  
  dpadDownOn = 1 & (buttonData3 >> 0);
  
  
}

void turnAllOn() {
  
  triangleOn = 1;
  circleOn = 1;
  squareOn = 1;
  crossOn = 1;
  
  l1On = 1;
  l2On = 1;
  l3On = 1;
  
  r1On = 1;
  r2On = 1;
  r3On = 1;
  
  selectOn = 1;
  startOn = 1;
  homeOn = 1;
  
  dpadLeftOn = 1;
  dpadUpOn = 1;
  dpadRightOn = 1;
  dpadDownOn = 1;
}

void turnAllOff(){
  triangleOn = 0;
  circleOn = 0;
  squareOn = 0;
  crossOn = 0;
  
  l1On = 0;
  l2On = 0;
  l3On = 0;
  
  r1On = 0;
  r2On = 0;
  r3On = 0;
  
  selectOn = 0;
  startOn = 0;
  homeOn = 0;
  
  dpadLeftOn = 0;
  dpadUpOn = 0;
  dpadRightOn = 0;
  dpadDownOn = 0;
}


