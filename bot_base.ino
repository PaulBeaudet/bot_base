/* bot_base.ino ~ Copyright 2015 ~ Paul Beaudet
 * MIT licence ~ See licence for details
 * Compiled with Arduino IDE 1.6.5
 * The goal of this project is to provide remote control and some autonomous
 * program modes for a 2 wheeled robot driven by the 2x12 Sabertooth motor controller
 * NOTICE!! no ramping, assumes your motor controllor does it !!
 * This robot also has a IR long range distance sensor panned by a servo
 * in order to cover a range of posible obstacles
 * The readme will include details of remote control via serial port
 */
#include <Servo.h>      // defines basic pinout
Servo leftWheel ;       // left servo
Servo rightWheel;       // right servo
Servo sensorPan ;       // neck servo
#define RIGHT_SERVO  53 // on Arduino Mega
#define LEFT_SERVO   52
#define SENSOR_SERVO 51

// direction constants
#define BACK_LEFT  '1'
#define BACK       '2'
#define BACK_RIGHT '3'
#define SPIN_LEFT  '4'
#define STOP       '5'
#define SPIN_RIGHT '6'
#define FWD_LEFT   '7'
#define FWD        '8'
#define FWD_RIGHT  '9'
// command types
#define MOVEMENT   'M'
#define SPEED      'S'
#define PROGRAM    'P'
// modes of operation
#define REMOTE_OP  '1'
#define OBSTACLE   '2'

// boolean synonyms
#define MONITOR_MODE 0
#define TRIGER       1

// drive speeds
#define MAX_POWER 255
#define ERROR     256


// --------------- Main routines ------------------
void setup(){
  Serial.begin(115200);
  leftWheel.attach(LEFT_SERVO, 1000, 2000);   // left servo with min & max val
  rightWheel.attach(RIGHT_SERVO, 1000, 2000); // right servo with min & max val
  sensorPan.attach(SENSOR_SERVO);             // panning sensor servo

}

void loop(){
  commandHandler(commandListen(Serial.read()));
  /* pass the result of read into listener and listen into handler.
   * these are passed like this as a reminder that only one char is read on
   * any given program loop, new command processed when packet is finished
   */
  if(char mode = programMode(MONITOR_MODE) != REMOTE_OP){
    if(mode == OBSTACLE){/*execute obstacle avoidence rotine*/}
    // check current autonomous program routine
  }
}

//-------------- Main supporting functions ------------

char* commandListen(char incoming){  // returns if a command has been recieved
  static char packet[2] = { 0, 0 }; // store potential incoming command packet
  // reset if a full packet has been recieved
  if(packet[0] && packet[1]){packet[0] = 0; packet[1] = 0;}

  if(incoming != 0xFF){     // data available from Serial? 0xff signals no data
    if(packet[0]){          // do we already have the first char in packet?
      packet[1] = incoming; // then this this the second
      return packet;        // in this case a "packet" is 2 chars, we are done
    }                       // return pointer to first address of the packet
    else if(validCommand(incoming)){packet[0] = incoming;}
    // otherwise this is the first char in the packet, record it as such
  }
  return 0;
}

void commandHandler(char* packet){ // handle incoming commands
  if(packet){   // as soon as a packet comes in
  // if serial listener returns a pointer to first char of an expected packet
    if(packet[0] == MOVEMENT){
      driveControl( 0, packet[1] );
      programMode(REMOTE_OP); // in this way commands interupt program mode
    }
    else if(packet[0] == SPEED){
      driveControl( packet[1], 0 );
      programMode(REMOTE_OP); // interupt program mode
    }
    else if(packet[0] == PROGRAM) {programMode(packet[1]);}
  }
}

char validCommand(char command){
  if     (command == MOVEMENT){;}
  else if(command == SPEED){;}
  else if(command == PROGRAM){;}
  else{
    Serial.print(F("E:Invalid command ")); Serial.println(command);
    command = 0; // set command to zero if not a valid command
  }
  return command;
}

char programMode(char mode){      // set autonomous program mode
  static char taskAtHand = REMOTE_OP;  // task defauts to remote opperation

  if     (mode == REMOTE_OP){taskAtHand = REMOTE_OP;}
  else if(mode == OBSTACLE){taskAtHand = OBSTACLE;}
  else if(mode == 0){;} // MONITOR just skip to return the taskAtHand
  // in all other cases mode is set to zero, preventing Invalid options
  else{
    Serial.print(F("E:Invalid mode "));
    Serial.println(mode);
    taskAtHand = REMOTE_OP;
  }
  return taskAtHand; // returns the set programMode
}

// ----------- drive train ------------------------
void drive(int speed, int direction){
  int finalSpeedLeft = 0;
  int finalSpeedRight = 0;

  if(speed == 0 && direction) {
    finalSpeedLeft = direction;
    finalSpeedRight = -direction;
  } else {
    finalSpeedLeft = speed * ((-255 - direction) / -255.0);
    finalSpeedRight = speed * ((255 - direction) / 255.0);

    if (speed > 0 && finalSpeedLeft > speed){finalSpeedLeft = speed;}
    if (speed > 0 && finalSpeedRight > speed){finalSpeedRight = speed;}
    if (speed < 0 && finalSpeedLeft < speed){finalSpeedLeft = speed;}
    if (speed < 0 && finalSpeedRight < speed){finalSpeedRight = speed;}
  }
  // !! no ramping assumes your motor controllor does it !!
  leftWheel.writeMicroseconds(map(finalSpeedLeft,-255,255,1000,2000));
  rightWheel.writeMicroseconds(map(finalSpeedRight,-255,255,1000,2000));
}

void driveControl(char direction, char speed){
  static int thisSpeed = 0; // record last speed driven for directional speed

  int thisDirection = 0;
  if(direction){
    thisDirection = figureDirection(direction, thisSpeed);
    if(thisDirection != ERROR){ thisSpeed = directionSpeed(direction, thisSpeed);}
    else{return;} // invalid command skip setting drive
  } else {
    thisSpeed = figureSpeed(speed);
    if(thisSpeed != 1){return;} // invalid command skip setting drive
  }
  drive(thisSpeed, thisDirection);
}

byte figureSpeed(char ascii){ // converts ascii numbers to a fraction of power
  byte powerValue = 1; // notice there are no cases where this would be true
  if     (ascii == '1'){powerValue = MAX_POWER * 0.25;} // 25% power
  else if(ascii == '2'){powerValue = MAX_POWER * 0.50;} // 50% power
  else if(ascii == '3'){powerValue = MAX_POWER * 0.75;} // 75% power
  else if(ascii == '4'){powerValue = MAX_POWER;}
  else{Serial.println(F("E:Invalid entry"));}
  Serial.print(F("W:S:"));
  Serial.println(ascii);
  return powerValue;
}

int figureDirection(char ascii, int lastPower){
  int directionValue = ERROR;
  if     ( ascii == STOP)      {directionValue = 0;}
  else if( ascii == BACK_LEFT) {directionValue = 0-lastPower;}
  else if( ascii == BACK)      {directionValue = 0;}
  else if( ascii == BACK_RIGHT){directionValue = lastPower;}
  else if( ascii == SPIN_LEFT) {directionValue = 0-(lastPower/1.25);}
  else if( ascii == SPIN_RIGHT){directionValue = lastPower/1.25;}
  else if( ascii == FWD_LEFT)  {directionValue = 0-lastPower;}
  else if( ascii == FWD)       {directionValue = 0;}
  else if( ascii == FWD_RIGHT) {directionValue = lastPower;}
  else {Serial.println("E:Invalid Move");}
  Serial.print(F("W:D:"));
  Serial.println(ascii);
  return directionValue;
}

int directionSpeed(char ascii, int lastPower){
  int speedValue = ERROR;
  if     ( ascii == STOP)      {speedValue = 0;}
  else if( ascii == BACK_LEFT) {speedValue = 0 - lastPower;}
  else if( ascii == BACK)      {speedValue = 0 - lastPower;}
  else if( ascii == BACK_RIGHT){speedValue = 0 - lastPower;}
  else if( ascii == SPIN_LEFT) {speedValue = 0;}
  else if( ascii == SPIN_RIGHT){speedValue = 0;}
  else if( ascii == FWD_LEFT)  {speedValue = lastPower;}
  else if( ascii == FWD)       {speedValue = lastPower;}
  else if( ascii == FWD_RIGHT) {speedValue = lastPower;}
  return speedValue;
}
