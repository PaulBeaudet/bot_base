// drive.ino ~ add on file to bot_base.ino ~ Copyright 2015 ~ Paul Beaudet
// MIT licence ~ see LICENCE for details
// ----------- drive train functions ---------------------

#include <Servo.h>
Servo leftWheel ;        // left servo
Servo rightWheel;        // right servo
Servo sensorPan ;        // neck servo
#define RIGHT_SERVO  53  // on Arduino Mega
#define LEFT_SERVO   52
#define SENSOR_SERVO 51
#define SENSOR_LEFT  30  // left most desired pan range of sensor
#define SENSOR_RIGHT 140 // right most desired pan range of sensor
#define SENSOR_SPEED 600 // speed of sensor pan


void setupServos(){
  leftWheel.attach(LEFT_SERVO, 1000, 2000);   // left servo with min & max val
  rightWheel.attach(RIGHT_SERVO, 1000, 2000); // right servo with min & max val
  sensorPan.attach(SENSOR_SERVO);             // panning sensor servo
}


void drive(int spd, int steer){
  int finalSpeedLeft = 0;
  int finalSpeedRight = 0;

  if(spd == 0 && steer) {
    finalSpeedLeft = steer;
    finalSpeedRight = -steer;
  } else {
    finalSpeedLeft = spd * ((-255 - steer) / -255.0);
    finalSpeedRight = spd * ((255 - steer) / 255.0);

    if (spd > 0 && finalSpeedLeft > spd){finalSpeedLeft = spd;}
    if (spd > 0 && finalSpeedRight > spd){finalSpeedRight = spd;}
    if (spd < 0 && finalSpeedLeft < spd){finalSpeedLeft = spd;}
    if (spd < 0 && finalSpeedRight < spd){finalSpeedRight = spd;}
  }
  // !! no ramping assumes your motor controllor does it !!
  leftWheel.writeMicroseconds(map(finalSpeedLeft,-255,255,1000,2000));
  rightWheel.writeMicroseconds(map(finalSpeedRight,-255,255,1000,2000));
}

#define ERR 256 // invalid value error flag

void driveControl(char steer, char spd){
  static int thisSpeed = 0; // record last spd driven for directional spd

  int thisDirection = 0;
  if(spd){
    thisSpeed = figurePower(spd);
    if(thisSpeed == 1){return;}
  }

  if(steer){
    thisDirection = figureDirection(steer, thisSpeed);
    if(thisDirection != ERR){thisSpeed =directionSpeed(steer, thisSpeed);}
    else{return;} // invalid command skip setting drive
  }

  drive(thisSpeed, thisDirection);
}

//- the following function interpert serial commands into values and setings --
#define MAX_POWER 255 // max drive spd

byte figurePower(char ascii){ // converts ascii numbers to a fraction of spd
  byte speedValue = 1; // notice there are no cases where this would be true
  if     (ascii == '1'){speedValue = MAX_POWER * 0.25;} // 25% spd
  else if(ascii == '2'){speedValue = MAX_POWER * 0.50;} // 50% spd
  else if(ascii == '3'){speedValue = MAX_POWER * 0.75;} // 75% spd
  else if(ascii == '4'){speedValue = MAX_POWER;}
  else{Serial.println(F("E:Invalid entry"));}
  Serial.print(F("W:S:"));
  Serial.println(ascii);
  return speedValue;
}

// steer constants
#define BACK_LEFT  '1'
#define BACK       '2'
#define BACK_RIGHT '3'
#define SPIN_LEFT  '4'
#define STOP       '5'
#define SPIN_RIGHT '6'
#define FWD_LEFT   '7'
#define FWD        '8'
#define FWD_RIGHT  '9'

int figureDirection(char ascii, int lastSpeed){
  int directionValue = ERR;
  if     ( ascii == STOP)      {directionValue = 0;}
  else if( ascii == BACK_LEFT) {directionValue = 0-lastSpeed;}
  else if( ascii == BACK)      {directionValue = 0;}
  else if( ascii == BACK_RIGHT){directionValue = lastSpeed;}
  else if( ascii == SPIN_LEFT) {directionValue = 0-(lastSpeed/1.25);}
  else if( ascii == SPIN_RIGHT){directionValue = lastSpeed/1.25;}
  else if( ascii == FWD_LEFT)  {directionValue = 0-lastSpeed;}
  else if( ascii == FWD)       {directionValue = 0;}
  else if( ascii == FWD_RIGHT) {directionValue = lastSpeed;}
  else {Serial.println(F("E:Invalid Move"));}
  Serial.print(F("W:D:"));
  Serial.println(ascii);
  return directionValue;
}

int directionSpeed(char ascii, int lastSpeed){
  int speedValue = ERR;
  if     ( ascii == STOP)      {speedValue = 0;}
  else if( ascii == BACK_LEFT) {speedValue = 0 - lastSpeed;}
  else if( ascii == BACK)      {speedValue = 0 - lastSpeed;}
  else if( ascii == BACK_RIGHT){speedValue = 0 - lastSpeed;}
  else if( ascii == SPIN_LEFT) {speedValue = 0;}
  else if( ascii == SPIN_RIGHT){speedValue = 0;}
  else if( ascii == FWD_LEFT)  {speedValue = lastSpeed;}
  else if( ascii == FWD)       {speedValue = lastSpeed;}
  else if( ascii == FWD_RIGHT) {speedValue = lastSpeed;}
  return speedValue;
}

// -------------- Sensor Servo -----------------
void sensorReact(int sensorValue){
	// sorry this doesn't take into acount where the sensor is pointed
  static TimeCheck timer;
  static char currentDir = FWD; // default to moving forward

  if(timer.check()){ // given last set time is up: defaults to true
    if(sensorValue < 300){     // far enough away from something
      if(currentDir = FWD){
        driveControl('2',FWD);
        currentDir = FWD;
      } else if (currentDir == BACK){ // we backed up enough
        driveControl('2',BACK_RIGHT);
        currentDir = BACK_RIGHT;
        timer.set(200);              // check back in x ms
      } else if (currentDir == BACK_RIGHT){
        driveControl('2',FWD);
        currentDir = FWD;
      }
    } else {                           // too close to something
      if(currentDir == FWD){
        driveControl(0, BACK);         // back up!
        currentDir = BACK;
        timer.set(200);
      } else if (currentDir == BACK){ // keep backing up!
        driveControl('4', 0);         // speed up something is comming towards
        timer.set(100);
      } else if (currentDir == BACK_RIGHT){  // turned into a tight position
        timer.set(100);                      // keep going
      }
    }
  }
}

void panSensor(){
  static TimeCheck timer;
  static byte position = SENSOR_LEFT;
  
  if(timer.check()){
    if(position == SENSOR_LEFT){position = SENSOR_RIGHT;}
    else{position = SENSOR_LEFT;} // alternate position movement
    sensorPan.write(position);    // write current position
    timer.set(SENSOR_SPEED);      // set next time to alternate possition
  }
}
