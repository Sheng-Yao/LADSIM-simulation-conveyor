#include <Arduino.h>

// //I2C library (For SDA and SCL)
// #include <Wire.h>
// //LCD display library
// #include <LiquidCrystal_I2C.h>

// //initialize lcd instances
// LiquidCrystal_I2C lcd(0x27,16,2);

//input pin

//upper part
#define INPUT0 36 //metal detector
#define INPUT1 37 //object detector
//plastic slot
#define INPUT2 38 //plastic detector
//lower part1
#define INPUT3 39 //assembly detector (ACTIVE HIGH)
#define INPUT4 40 //object detector (ACTIVE HIGH) --> determine region (not used)
#define INPUT5 41 //object detector --> rejection region
//buttons
#define INPUT6 42 //ON button
#define INPUT7 43 //OFF button (ACTIVE HIGH)
//lower part2
#define INPUT8 44 //metal detector
//solenoid return detector
#define INPUT9 45 //sort solenoid return (metal detector) (upper) (ACTIVE HIGH)
#define INPUT10 46  //reject solenoid return (metal detector) (lower) (ACTIVE HIGH)

//output pin
#define OUTPUT0 8 //upper conveyor
#define OUTPUT1 9 //sort solenoid
#define OUTPUT2 10  //rotary solenoid
#define OUTPUT3 11  //rejection solenoid
#define OUTPUT4 12  //lower conveyor

//flag
bool isSystemStart = false; //current state of the system (changing during the start and stop action by the buttons)
bool isConveyorClosed = true; //current state of both (upper and lower) conveyors
bool isMetal = false;  //metal and non-metal flag --> result of the metal detection
bool isPlasticPrepared = false; //is plastic head prepared to be assemble with incoming metal
bool isAssemblyFlowLooping = false; //is the object is assembly
bool isAssemblyCalculationFlowLooping = false; //is the complete assembly pass-by
bool isPlasticPushed = false; //flag to determine plastic is pushed by the sort solenoid
bool isRotarySolenoidOpen = false;  //check is the rotary solenoid is opened
bool isMetalRejectionFlowLooping = false; //avoid the looping when the metal rejection process

//timer (every timer is being divide with 100 which is representing 1 = 0.1 second)
unsigned long metalBufferTimer;  //timing between object and seperator
unsigned long plasticRollingInSlotTimer;    //timer for the plastic head to rolling down from sort solenoid to the rotary solenoid
unsigned long rejectionMetalMovingBufferTimer;    //timer for rejection solenoid to work (avoid pushing the previous assembly)
unsigned long sortSolenoidReturnTimer;  //timer for sort selonoid to return
unsigned long rejectionSolenoidReturnTimer; //timer for rejection seloniod to return
unsigned long sortPlasticBufferTimer;   //avoid the bouncing (overcounting)
unsigned long rotarySolenoidReturnTimer;    //timer for rotary solenoid to return
unsigned long rejectionMetalCalculationBufferTime;  //timer to calculate the next object
unsigned long assemblyCalculationTimer; // buffer time to reset the calculation of the timer

//counter
byte plasticHeadInSlot = 0; //counter for the plastic head in the waiting slot
byte metalRejection = 0;  //counter for metal base (not assembly) needed for reject

//interrupt function
void stopButtonPressed(){
  isSystemStart = false;  //reset the state of the system
}

//setup for input and output pins
void pinSetup(){

  //setup input pins
  pinMode(INPUT0,INPUT);
  pinMode(INPUT1,INPUT);
  pinMode(INPUT2,INPUT);
  pinMode(INPUT3,INPUT);
  pinMode(INPUT4,INPUT);
  pinMode(INPUT5,INPUT);
  pinMode(INPUT6,INPUT);
  pinMode(INPUT7,INPUT);
  pinMode(INPUT8,INPUT);
  pinMode(INPUT9,INPUT);
  pinMode(INPUT10,INPUT);

  //setup output pins
  pinMode(OUTPUT0,OUTPUT);
  pinMode(OUTPUT1,OUTPUT);
  pinMode(OUTPUT2,OUTPUT);
  pinMode(OUTPUT3,OUTPUT);
  pinMode(OUTPUT4,OUTPUT);
  
  //initialize the state of output pins
  digitalWrite(OUTPUT0,LOW);
  digitalWrite(OUTPUT1,LOW);
  digitalWrite(OUTPUT2,LOW);
  digitalWrite(OUTPUT3,LOW);
  digitalWrite(OUTPUT4,LOW);
}