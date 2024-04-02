#include "Function.h"

void setup(){

  //Set up serial communication for UART communication
  Serial.begin(9600);

  //setup input and output pins
  pinSetup();

  //attach an interrupt pin to detect the falling signal (HIGH to LOW) of the digital pin 2 (STOP button)
  attachInterrupt(0,stopButtonPressed,FALLING);
}

void loop(){

  //triggered when the system is closed
  if(!isSystemStart){

    //close the conveyors when the system is closed
    if(!isConveyorClosed){

      //stop the conveyor line
      digitalWrite(OUTPUT0,LOW);  //upper conveyor
      digitalWrite(OUTPUT4,LOW);  //lower conveyor
    }

    //triggered when the START button is pressed
    if(digitalRead(INPUT6) == HIGH){

      //changing the flag for current state of the system
      isSystemStart = true;

      //start the conveyor line
      digitalWrite(OUTPUT0,HIGH); //upper conveyor
      digitalWrite(OUTPUT4,HIGH); //lower conveyor

      //change the flag that the conveyor is not closed (is open)
      isConveyorClosed = false;
    }
  }

  //looping when the system is in start state
  while(isSystemStart){

    //check if the object is metal
    if(digitalRead(INPUT0) == HIGH && !isMetal){

      //setup the buffer timer
      metalBufferTimer = millis() / 100;

      //object is metal
      isMetal = true;
    }

    //reset the flag every 2.5 seconds
    if((millis()/100 - metalBufferTimer) > 25){

      //reset flag for next detection
      isMetal = false;
    }

    //condition to triggered: the object is not metal, when there are object detected by INPUT1, plactic head in slot < 6, check if the current plastic is pushed
    if(!isMetal&&digitalRead(INPUT1) == HIGH && plasticHeadInSlot < 6 && !isPlasticPushed){

      //start the sort solenoid
      digitalWrite(OUTPUT1,HIGH);

      //check if the plastic is pushed
      isPlasticPushed = true;
      
      //timer for plastic head rolling from sort solenoid to rotary solenoid
      plasticRollingInSlotTimer = millis() / 100;

      //setup timer for avoiding bouncing situation (overcounting)
      sortPlasticBufferTimer = millis() / 100;

      //setup timer for sort solenoid return
      sortSolenoidReturnTimer = millis() / 100;

      //increase the plastic head in slot counter by 1
      plasticHeadInSlot++;

    }

    //reset counter after the buffer time of 2 seconds
    if((millis()/100 - sortPlasticBufferTimer) > 20){

      //reset flag that the plastic is pushed down
      isPlasticPushed = false;
    }

    //triggered condition: sort solenoid is triggered and the time span > 0.1 second
    if(digitalRead(INPUT9) == LOW && (millis()/100 - sortSolenoidReturnTimer) > 1){

      //retact the sort solenoid
      digitalWrite(OUTPUT1,LOW);
    }

    //update the flag that there is plastic head prepared to be assemblied
    if(digitalRead(INPUT2)==LOW){ //low = nothing

      //update flag
      isPlasticPrepared = false;
    }else{

      //update flag
      isPlasticPrepared = true;
    }

    //triggered condition: rolling time > 1 second, there is plastic to be assemblied, plastic head stock in slot > 0, rotary solenoid is not open
    if((millis()/100 - plasticRollingInSlotTimer) > 10 && !isPlasticPrepared && plasticHeadInSlot > 0 && !isRotarySolenoidOpen){

      //open the rotary solenoid
      digitalWrite(OUTPUT2,HIGH);

      //flag that rotary solenoid is opened
      isRotarySolenoidOpen = true;

      //setup timer for rotary solenoid to return
      rotarySolenoidReturnTimer = millis()/100;

    }
    
    //trigger condition: rotary solenoid is opened, rotary solenoid opening time > 0.3 second
    if(isRotarySolenoidOpen && (millis()/100 - rotarySolenoidReturnTimer) > 3){
      
      //retact the rotary solenoid
      digitalWrite(OUTPUT2,LOW);

      //reset the rotary solenoid trigger flag when there are no object in the preparation state for the duration of 1 second
      if((millis()/100-rotarySolenoidReturnTimer)>10){
        isRotarySolenoidOpen = false;
      }

      //when there is plastic head in preparation state
      if(digitalRead(INPUT2)==HIGH){

        //reduce the plastic head remaining in the storing slot
        plasticHeadInSlot--;

        // reset the flag when there are plastic in preparation state
        isRotarySolenoidOpen = false;
      }
    }
    

    //triggered condition: metal, not assembly, is returning for assembly
    if(digitalRead(INPUT8)==HIGH&&digitalRead(INPUT3)==HIGH&&!isAssemblyFlowLooping){
      
      //limit the program run once
      isAssemblyFlowLooping = true;

      //reset the timer calculation only when the rejected metal is detected
      if(metalRejection==0){

        //reset timer
        rejectionMetalMovingBufferTimer = millis()/100;
      }

      //timer to detect the next object
      rejectionMetalCalculationBufferTime = millis()/100;

      //increase the counter for rejected metal by 1
      metalRejection++;
    }

    //reset the flag every 0.5 second avoid overcounting when the metal still remain in the non-assembly detection region
    if((millis()/100-rejectionMetalCalculationBufferTime) > 5){
      isAssemblyFlowLooping = false;
    }

    //triggered condition: is object in the rejection region
    if(digitalRead(INPUT5) == HIGH){

      //check the number of required metal rejection > 0, timer for metal moving towards the rejection region > 2 seconds,
      if(metalRejection > 0 && (millis()/100 - rejectionMetalMovingBufferTimer) > 20 && !isMetalRejectionFlowLooping){

        //start the rejection solenoid
        digitalWrite(OUTPUT3,HIGH);

        //setup the timer for rejection solenoid to return
        rejectionSolenoidReturnTimer = millis()/100;

        //limit the program run once
        isMetalRejectionFlowLooping = true;

        //reduce the counter for required rejection metal by 1
        metalRejection--;
      }else if(!isAssemblyCalculationFlowLooping && !isMetalRejectionFlowLooping){  //check whether message needed to be sent to robot arm arduino
        isAssemblyCalculationFlowLooping = true;
      
        assemblyCalculationTimer = millis()/100;
      }
    }

    
    if(isAssemblyCalculationFlowLooping && (millis()/100 - assemblyCalculationTimer) > 15){

      //Send message to robot arm arduino
      Serial.write(1);

      //reset flag
      isAssemblyCalculationFlowLooping = false;
    }

    //triggered condition: rejection solenoid is triggered and the time span > 0.1 second
    if(digitalRead(INPUT10)==LOW&&(millis()/100-rejectionSolenoidReturnTimer)>1){

      //retract the rejection solenoid
      digitalWrite(OUTPUT3,LOW);

      //reset the flag to avoid the over reduction of the counter for metal rejection
      isMetalRejectionFlowLooping = false;
    }
  }
}