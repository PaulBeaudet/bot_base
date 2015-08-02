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
#include <TimeCheck.h> // https://github.com/PaulBeaudet/TimeCheck
#include "keyDefinitions.h" // See for contant definitions

// -------- ~ Pinout ~ Arduino Mega ~ -----------------
#define IR_PIN       15
#define RIGHT_SERVO  53
#define LEFT_SERVO   52
#define SENSOR_SERVO 51

// --------------- Main routine ------------------
void setup(){
  Serial.begin(115200);
  setupServos();
}

void loop(){
  // remote control
  if(Serial.available()){
    commandHandler(commandListen(Serial.read()));
  }
  /* pass the result of read into listener and listen into handler.
   * these are passed like this as a reminder that only one char is read on
   * any given program loop, new command processed when packet is finished
   */
  // Autonomous modes
  if(char mode = programMode(MONITOR_MODE) != REMOTE_OP){
    if(mode == OBSTACLE){/*execute obstacle avoidence rotine*/}
    obstacleAvoid();
  }
}

//-------------- supporting functions ------------

char* commandListen(char incoming){  // returns if a command has been recieved
  static char packet[2] = { 0, 0 }; // store potential incoming command packet
  // reset if a full packet has been recieved
  if(packet[0] && packet[1]){packet[0] = 0; packet[1] = 0;}

  if(incoming){     // data available from Serial? 0xff signals no data
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

char validCommand(char cmd){
  if     (cmd == MOVEMENT){;}
  else if(cmd == SPEED){;}
  else if(cmd == PROGRAM){;}
  else{
    Serial.print(F("E:Invalid cmd ")); Serial.println(cmd);
    cmd = 0; // set command to zero if not a valid command
  }
  return cmd;
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

// --------------- obstacle avoidence -----------------
void obstacleAvoid(){
  panSensor();                      // constantly pan sensor
  sensorReact(analogRead(IR_PIN));  // react to reading of IR pin
}

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
