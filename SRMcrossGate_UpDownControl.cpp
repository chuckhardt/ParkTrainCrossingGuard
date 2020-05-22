// ***************************************************
//
// Crossing Gate Controller Program
//
// This software was developed to operate on an Arduino Uno
// microprocessor board.  It operates a crossing guard
// program for the Southeastern Railway Musuem.
//
// Author: C. Hardt
// Date: 10/21/13
//
// ****************************************************

#include "Timer.h"
#include "Arduino.h"
#include "SRMcrossGate_types.h"
#include "SRMcrossGate_UpDownControl.h"
#include "SRMcrossGate_Utils.h"

extern Timer gCrossingGateTimer;

static unsigned long ulMotorRunningTotalSecondsThisEvent = 0;
static unsigned long ulMotorRunningStartTimeThisEvent = 0;
static unsigned long ulMotorRunningTotalSeconds = 0;

static unsigned long gulGateDownStateDelayTime = 0;
static unsigned long gulGateDownStateDelayTimeEventStart = 0;

static unsigned long gulGateUpStateDelayTime = 0;
static unsigned long gulGateUpStateDelayTimeEventStart = 0;

static int giGateDownStateAfterDelay = kGateMovingDown_State_LightsAndBells;
static int giGateUpStateAfterDelay = kGateMovingUp_State_Debouce;

// ***************************************************
//
// InitializeTheGateTurnOnLightsBellsAndUpRelay()
//
// This function is called when we come out of reset (a power cycle).
// It's role is to turn on the lights, bells and switch on the Up motor.
//
// ****************************************************
void InitializeTheGateTurnOnLightsBellsAndUpRelay(int *piWarningLightTimerRightID, int *piWarningLightTimerLeftID, unsigned long *pulElapsedTime, int *piInitializationState)
{
     // record the state time of this event
     *pulElapsedTime = millis();   
  
     // start the flashing lights, we will use two timers for this 
     *piWarningLightTimerRightID = WarningLightTimerStart(kPinAddrGateLightsControlRight, 500, HIGH, -1);
     *piWarningLightTimerLeftID = WarningLightTimerStart(kPinAddrGateLightsControlLeft, 500, LOW, -1);
                         
     // Turn on the signal warning bell
     digitalWrite(kPinAddrGateBellControl, kWarningBellOn);
        
     // Before we turn on power to the motor, we want to make sure we set the direction of the motor 
     digitalWrite(kPinAddrGateArmControlMotorDirection, kGateArmControlMotorUp);

     // change the state before we exit
     *piInitializationState = kGateInitalize_MotorDirectionDelay;
        
     return;
        
}  //endof InitializeTheGateTurnOnLightsBellsAndUpRelay()

// ***************************************************
//
// InitializeMotorDirectionDelayState()
//
// As part of the gate initialization sequence, we want to insert a delay
// between setting the gate direction motor, and switching in the motor.
// After which, we need to advance to the next state.
//
// ****************************************************
void InitializeMotorDirectionDelayState(int *piGateDownState)
                                    
{
    static unsigned long ulDelayStartTime = 0;
    unsigned long ulTimeSpentInSequence;
    
    // we want to record/save our start time and will use it later to calculate elapsed time.
    if (ulDelayStartTime == 0)
    {
        ulDelayStartTime = millis();  
    }  
  
    // convert the time to reference the start of the event
    ulTimeSpentInSequence = millis() - ulDelayStartTime;
    
    // We do not want to advance to the next state until we have spent the perscribed time in our delay.
    if (ulTimeSpentInSequence >= kOneSecond)
    {
        *piGateDownState = kGateInitalize_MotorOn;
        ulDelayStartTime = 0;
    }
 
    return;  
  
}  // endof  InitializeMotorDirectionDelayState() 

// ***************************************************
//
// InitializeTheGateTurnOnUpMotor()
//
// This function is called when we come out of reset (a power cycle).
// It's role is to turn on the arm drive motor
//
// ****************************************************
void InitializeTheGateTurnOnUpMotor(unsigned long ulStartTime, int *piInitializationState, bool *pbMotorRunning)
{
     static bool bPrintFlag = false;
     unsigned long ulGateEventElapsedTime;
  
     // Turn on the motor 
     digitalWrite(kPinAddrGateArmControlMotorPower, kGateArmControlMotorOn);

     // calculate the elapsed time.
     ulGateEventElapsedTime = millis() - ulStartTime;  

     // We have to allow the gate time to rise back up.  Once we hit the target, change state
     if (ulGateEventElapsedTime >= kTenSeconds) 
     {
        *piInitializationState = kGateInitalize_MotorOff; 
     }  

     // we only want print this message one.
     if (bPrintFlag = false)
     {
          // We want to record the start time of this event
          ulMotorRunningStartTimeThisEvent = millis(); 
     
          //Serial.println("Init - Motor: On");
          bPrintFlag = true;
     }
     
     // set the flag that the motor is running
     *pbMotorRunning = true;
     
     return;
        
}  //endof InitializeTheGateTurnOnUpMotor()

// ***************************************************
//
// InitializeTheGateTurnOffUpMotor()
//
// This function is called when we come out of reset (a power cycle).
// It's role is to turn on the lights, bells and switch on the Up motor.
//
// ****************************************************
void InitializeTheGateTurnOffUpMotor(int iWarningLightTimerRightID, int iWarningLightTimerLeftID, bool *pbMotorRunning, unsigned long *pulMotorRunningTotalSeconds)
{
      // kill the warning light
      gCrossingGateTimer.stop(iWarningLightTimerRightID);
      gCrossingGateTimer.stop(iWarningLightTimerLeftID);
      
      // clear the timer IDs, such that they are never used again
      iWarningLightTimerRightID = 0;
      iWarningLightTimerLeftID  = 0;
      
      // While we just stopped the lights, we need to make sure they are in the off state
      digitalWrite(kPinAddrGateLightsControlLeft, kWarningLightsOff);
      digitalWrite(kPinAddrGateLightsControlRight, kWarningLightsOff);
      
      // Shut off the warning bell
      digitalWrite(kPinAddrGateBellControl, kWarningBellOff);
      
      // this turns OFF power to the gate are motor
      digitalWrite(kPinAddrGateArmControlMotorDirection, kGateArmControlMotorDown);  
      digitalWrite(kPinAddrGateArmControlMotorPower, kGateArmControlMotorOff);
  
      // log the total motor run time.  We need this, as the motor has a 10% duty cycle
      ulMotorRunningTotalSecondsThisEvent = millis() - ulMotorRunningStartTimeThisEvent; 
      *pulMotorRunningTotalSeconds = *pulMotorRunningTotalSeconds + ulMotorRunningTotalSecondsThisEvent;
  
      Serial.println("Gate Is Up");
  
      // set the flag that the motor is not running
      *pbMotorRunning = false;
  
    return;
  
}  //endof InitializeTheGateTurnOffUpMotor()  

// ***************************************************
//
// GateDownInactiveState()
//
// This function handles the inactive gate down state.
// Once the gate is down, and lights flashing this state is 
// entered.
//
// ****************************************************
void GateDownInactiveState(          int *piTrackOcupationState, 
                           unsigned long *pulGateDownEventTotalElapsedTime, 
                           unsigned long *pulGateDownEventElapsedStartTime,
                                    bool *pbGateState,
                                     int *piGateUpState,
                           unsigned long *pulGateDownStateDelayBeforeGateUpEventStartTime,
                                    bool *pbDutyCycleExceededFlag)
{
      static unsigned long ulPrintOutCount = 0;
  
      // need to convert the CPU clock time into a down gate reference time 
      *pulGateDownEventTotalElapsedTime = millis() - *pulGateDownStateDelayBeforeGateUpEventStartTime;
      
      // if the motor duty cycle has been excceded, then we are going to ignore the up
      // sequence and just reset the state machine.
      if (*pbDutyCycleExceededFlag == true)
      {
          *piTrackOcupationState = kTrackVacant;
          *pulGateDownEventTotalElapsedTime = 0;
          *pulGateDownEventElapsedStartTime = 0;
          
          *piGateUpState = kGateMovingUp_State_MotorOnDelay;
          
          gulGateUpStateDelayTime = kTwentySeconds;
          gulGateUpStateDelayTimeEventStart = millis();
          giGateUpStateAfterDelay = kGateMovingUp_State_MotorOff;
          
      }  
      
      // if we have reached the maximum gate down time, then the down sequence is over 
      // and we need to open the gate.
      else if (*pulGateDownEventTotalElapsedTime >= kMaxGateDownTimelimitReached)
      {
          //Serial.print("Max elapsed Time Reached: ");
          //Serial.println(*pulGateDownEventTotalElapsedTime); 
          *piTrackOcupationState = kTrackVacant;
          *pulGateDownEventTotalElapsedTime = 0;
          *pulGateDownEventElapsedStartTime  = 0;
      }
      else
      {
           if (ulPrintOutCount++ % 5 == 0)
           {
               Serial.print("Time Remaining Before Gate Lift: ");
               Serial.println(kMaxGateDownTimelimitReached - *pulGateDownEventTotalElapsedTime);
           }    
      }  
  
    return;  
  
}  // endof  GateDownInactiveState() 

// ***************************************************
//
// GateDownLightsBellsAndMotorDirectionState()
//
// This function turns on the lights, bells & the down direction relay.
// Once this is done, the state machines advances to the next state.
//
// ****************************************************
void GateDownLightsBellsAndMotorDirectionState(unsigned long ulGateDownEventStartTime, 
                                               //unsigned long *pulGateDownEventTimeSpentInSequence, 
                                               unsigned long *pulGateUpEventTimeSpentInSequence,
                                                         int *piGateDownState, 
                                                         int *piWarningLightTimerRightID, 
                                                         int *piWarningLightTimerLeftID)
{

    // need to convert the CPU clock time into a down gate reference time 
    //*pulGateDownEventTimeSpentInSequence = millis() - ulGateDownEventStartTime;  
        
   // we need to turn on the warning bells and lights.  But we only want to do this once.
   // So we look at one time it is, and if the warning light timers have a valid ID. 
   if ((*piWarningLightTimerLeftID == 0) && (*piWarningLightTimerRightID == 0))
   {
       // start the flashing lights, we will use two timers for this 
       *piWarningLightTimerRightID = WarningLightTimerStart(kPinAddrGateLightsControlRight, 500, HIGH, -1);
       *piWarningLightTimerLeftID = WarningLightTimerStart(kPinAddrGateLightsControlLeft, 500, LOW, -1);
       
       // Turn on the signal warning bell
       digitalWrite(kPinAddrGateBellControl, kWarningBellOn);
      
       Serial.println("Lights & Bells: On");
      
       // At this point in the sequence we want to setup the motor direction.
       // Such that when we apply power to the motor, the direction control relay is already in position
       digitalWrite(kPinAddrGateArmControlMotorDirection, kGateArmControlMotorDown);
       
       Serial.println("Motor Direction: Down");
      
       // since we are closing the gate, let's reset the count of the number of seconds the gate has been open
       *pulGateUpEventTimeSpentInSequence = 0;
       
       // advance to the next state
       *piGateDownState = kGateMovingDown_State_LightsAndBellsDelay;
       
       // setup our delay and state after the delay
       gulGateUpStateDelayTimeEventStart = millis();
  } 
  
    return;  
  
}  // endof  GateDownLightsBellsAndMotorDirectionState() 

// ***************************************************
//
// GateDownWarningLightsAndBellDelayState()
//
// As part of the gate down state, this function provides the motor direction
// relay delay.  It then advances to the next state.
//
// ****************************************************
void GateDownWarningLightsAndBellDelayState(int *piGateDownState)
                                    
{
    unsigned long ulGateDownEventTimeSpentInSequence;
  
    // convert the time to reference the start of the up event
    ulGateDownEventTimeSpentInSequence = millis() - gulGateUpStateDelayTimeEventStart;
    
    // We do not want to advance to the next state until we have spent the perscribed time in our delay.
    if (ulGateDownEventTimeSpentInSequence >= kThreeSeconds)
    {
        //Serial.println("Down Delay Max Time Reached"); 
        *piGateDownState = kGateMovingDown_State_MotorOn;
    }
 
    return;  
  
}  // endof  GateDownWarningLightsAndBellDelayState() 

// ***************************************************
//
// GateDownMotorOnState()
//
// This function turns gate motor.  After the timeout period, the state
// machine is advanced to the next state.
//
// ****************************************************
void GateDownMotorOnState(unsigned long ulGateDownEventStartTime, 
                                   bool *pbMotorOnFlag,
                                    int *piGateDownState,
                                   bool *pbMotorRunning,
                          unsigned long *pulMotorRunningTotalSeconds,
                                   bool *pbDutyCycleExceededFlag)
{
       
    // only run if the motor duty cycle is within limits.
    if (*pulMotorRunningTotalSeconds < kMaxDutyCycleLimitReached)
    { 
        // We are going to set the direction of the gate (up or down) and turn on the motor
        // However we already set the direction when we turn the lights on.  
        digitalWrite(kPinAddrGateArmControlMotorDirection, kGateArmControlMotorDown);
        digitalWrite(kPinAddrGateArmControlMotorPower, kGateArmControlMotorOn);
       
        // We only want to print this message once
        if(*pbMotorOnFlag == false)
        {
            Serial.println("Motor: On");
            
            // We want to record the start time of this event
            ulMotorRunningStartTimeThisEvent = millis(); 
            *pbMotorOnFlag = true;
            *pbMotorRunning = true;
        }
    
        gulGateDownStateDelayTime = kThirteenSeconds;
        gulGateUpStateDelayTimeEventStart = millis();
    
    }
    else
    {
         // if we have exceeded the motor duty cycle, then we will not turn on the motor. 
         if (*pbDutyCycleExceededFlag == false)
         {
             Serial.print("Motor Max Duty Cycle, Ignoring Motor On Cmd: ");
             Serial.println(*pulMotorRunningTotalSeconds); 
         }
         
         *pbDutyCycleExceededFlag = true;
         
         gulGateDownStateDelayTime = kSevenSeconds;
         gulGateUpStateDelayTimeEventStart = millis();
    }  

    // setup the next state 
    *piGateDownState = kGateMovingDown_State_MotorOnDelay;
 
    return;  
  
}  // endof  GateDownMotorOnState() 

// ***************************************************
//
// GateDownMotorOnDelay()
//
// As part of the gate up state, this function turns calculates the delay between
// motor on and off.  It then advances to the next state.
//
// ****************************************************
void GateDownMotorOnDelay(int *piGateDownState)
                                    
{
    unsigned long ulGateDownEventTimeSpentInSequence;
  
    // convert the time to reference the start of the up event
    ulGateDownEventTimeSpentInSequence = millis() - gulGateUpStateDelayTimeEventStart;
    
    // We do not want to advance to the next state until we have spent the perscribed time in our delay.
    if (ulGateDownEventTimeSpentInSequence >= gulGateDownStateDelayTime)
    {
        //Serial.println("Down Delay Max Time Reached"); 
        *piGateDownState = kGateMovingDown_State_MotorOff;
    }
 
    return;  
  
}  // endof  GateDownMotorOnDelay() 

// ***************************************************
//
// GateDownMotorOffState()
//
// This function turns off the gate motor, and resets to the default gate down state.
//
// ****************************************************
void GateDownMotorOffState(unsigned long ulGateDownEventStartTime, 
                                    bool *pbMotorOnFlag,
                                    bool *pbMotorOffFlag,
                                     int *piGateDownState,
                                    bool *pbGateState,
                                    bool *pbMotorRunning, 
                           unsigned long *pulMotorRunningTotalSeconds,
                           unsigned long *pulGateDownStateDelayBeforeGateUpEventStartTime,
                                    bool *pbDutyCycleExceededFlag)
{
 
  
    // We only want to print this message once
    if (*pbMotorOffFlag == false);
    {
        Serial.println("Motor: Off");
        Serial.println("Gate is Down");
        
        *pbMotorOffFlag = true;
        
    }
    
    // Shut off the gate motor
    digitalWrite(kPinAddrGateArmControlMotorPower, kGateArmControlMotorOff);
    digitalWrite(kPinAddrGateArmControlMotorDirection, kGateArmControlMotorDown);
    
    *pbGateState = kGateInDownPosition;
    *pbMotorOnFlag = false;
    *pbMotorOffFlag = false;
    
    // if the motor duty cycle has been exceeded, then do not calculate run time.
    if (*pbDutyCycleExceededFlag == false)
    {
        // log the total motor run time.  We need this, as the motor has a 10% duty cycle
        ulMotorRunningTotalSecondsThisEvent = millis() - ulMotorRunningStartTimeThisEvent; 
        *pulMotorRunningTotalSeconds = *pulMotorRunningTotalSeconds + ulMotorRunningTotalSecondsThisEvent;
        
        *pbDutyCycleExceededFlag == false;
    }
        
    // reset the state machine to the default state
    *piGateDownState = kGateMovingDown_State_LightsAndBells;
     
    // set the flag that the motor is not running
    *pbMotorRunning = false;
    
    // we need to log the time before we start the gate up event
    *pulGateDownStateDelayBeforeGateUpEventStartTime = millis();
     
    return;  
  
}  // endof  GateDownMotorOffState() 

// ***************************************************
//
// GateUpInactiveState()
//
// This function is called whenever the gate is up, and the track is 
// vacant.  We use it to keep track of the motor on duty cycle.
//
// ****************************************************
void GateUpInactiveState(void)
{
 
}  // GateUpInactiveState()

// ***************************************************
//
// GateUpDebouceState()
//
// This function is a debouce state that is probably no longer needed.
// We will keep it for now
//
// ****************************************************
void GateUpDebouceState(unsigned long *pulGateUpEventTimeSpentInSequence, 
                        unsigned long *pulGateUpEventStartTime, 
                                  int *piGateUpState)
{
 
    // This is our track sensor switch debounce.  We want to make two back
    // to back readings confirming whether or not the track is occupied
    // So we ignore the first event
    if (*pulGateUpEventTimeSpentInSequence == 0)
    {
        // we store off the time we started this particular gate event.
        *pulGateUpEventStartTime  = millis();
          
        // we are going to increment the time by one ms, only so we do not repeat this step again.
        *pulGateUpEventTimeSpentInSequence += 1;
        
        Serial.println("Track is Vacant!"); 
      }
 
    // reset the state machine to the default state
    *piGateUpState = kGateMovingUp_State_MotorDirection;
     
    return;  
  
}  // endof  GateUpDebouceState() 

// ***************************************************
//
// GateUpMotorDirectionState()
//
// As part of the gate up state, this function sets the gate motor direction
// Then advances to the next state.
//
// ****************************************************
void GateUpMotorDirectionState(unsigned long *pulGateUpEventStartTime, 
                                        bool *pbMotorDirectionFlag,
                                         int *piGateUpState)
{
    // Before we turn on power to the motor, we want to make sure we set the direction of the motor 
    digitalWrite(kPinAddrGateArmControlMotorDirection, kGateArmControlMotorUp);
        
    // we only want to print the Motor direction once
    if (*pbMotorDirectionFlag == false)
    {
        Serial.println("Motor Direction: Up");
        *pbMotorDirectionFlag = true;
    }       
    
    // setup our next state
    *piGateUpState = kGateMovingUp_State_MotorDirectionDelay;
    
    // since we need to delay, setup the delay, and the state after the delay
    gulGateUpStateDelayTimeEventStart = millis();
  
    return;  
  
}  // endof  GateUpMotorDirectionState() 

// ***************************************************
//
// GateUpMotorDirectionDelayState()
//
// As part of the gate up state, this function provides a delay between the selection
// of the motor direction, and the motor turning on. 
//
// ****************************************************
void GateUpMotorDirectionDelayState(int *piGateUpState)
{
    unsigned long ulGateUpEventTimeSpentInSequence;  
  
    // convert the time to reference the start of the up event
    ulGateUpEventTimeSpentInSequence = millis() - gulGateUpStateDelayTimeEventStart;
    
    // We need to allow time for the gate to be raised.
    if (ulGateUpEventTimeSpentInSequence >= kOneSecond)
    {
        //Serial.println("Motor Direction Delay Complete");
        *piGateUpState = kGateMovingUp_State_MotorOn;  
    }  
 
    return;  
  
}  // endof  GateUpMotorDirectionDelayState() 


// ***************************************************
//
// GateUpMotorOnState()
//
// As part of the gate up state, this function turns on the gate motor.
// Then advances to the next state.
//
// ****************************************************
void GateUpMotorOnState(unsigned long *pulGateUpEventStartTime, 
                                 bool *pbMotorOnFlag,
                                  int *piGateUpState,
                                 bool *pbMotorRunning)
{
 
    // this turns power ON to the UP gate motor 
    digitalWrite(kPinAddrGateArmControlMotorDirection, kGateArmControlMotorUp); 
    digitalWrite(kPinAddrGateArmControlMotorPower, kGateArmControlMotorOn);
    
    // we only want to print the Motor direction once
    if (*pbMotorOnFlag == false)
    {
        Serial.println("Motor: On");
        
        // We want to record the start time of this event
        ulMotorRunningStartTimeThisEvent = millis();
        
        *pbMotorOnFlag = true;
    }       
    
    // set the flag that the motor is running
    *pbMotorRunning = true;

    // setup our next state
    *piGateUpState = kGateMovingUp_State_MotorOnDelay;
    
    // since we need to delay, setup the delay, and the state after the delay
    gulGateUpStateDelayTime = kThirteenSeconds;
    gulGateUpStateDelayTimeEventStart = millis();
 
    return;  
  
}  // endof  GateUpMotorOnState() 

// ***************************************************
//
// GateUpMotorOnDelayState()
//
// As part of the gate up state, this function turns calculates the delay between
// motor on and off.  It then advances to the next state.
//
// ****************************************************
void GateUpMotorOnDelayState(int *piGateUpState)
{
    unsigned long ulGateUpEventTimeSpentInSequence;  
  
    // convert the time to reference the start of the up event
    ulGateUpEventTimeSpentInSequence = millis() - gulGateUpStateDelayTimeEventStart;
    
    // We need to allow time for the gate to be raised.
    if (ulGateUpEventTimeSpentInSequence >= gulGateUpStateDelayTime)
    {
        *piGateUpState = kGateMovingUp_State_MotorOff;  
    }  
 
    return;  
  
}  // endof  GateUpMotorOnDelayState() 


// ***************************************************
//
// GateUpMotorOffState()
//
// As part of the gate up state, this function turns off the gate motor.
// Then advances to the default state.
//
// ****************************************************
void GateUpMotorOffState(unsigned long *pulGateUpEventStartTime, 
                                   int *piWarningLightTimerRightID,
                                   int *piWarningLightTimerLeftID,
                                  bool *pbGateState,
                         unsigned long *pulGateDownEventStartTime,
                                  bool *pbMotorDirectionFlag,
                                  bool *pbMotorOnFlag,
                                   int *piGateUpState,
                                  bool *pbMotorRunning,
                         unsigned long *pulMotorRunningTotalSeconds,
                                  bool *pbDutyCycleExceededFlag)
{
 
        
    if (*pbGateState == kGateInDownPosition)
    {   
        // kill the warning light
        gCrossingGateTimer.stop(*piWarningLightTimerRightID);
        gCrossingGateTimer.stop(*piWarningLightTimerLeftID);
        
        // clear the timer IDs, such that they are never used again
        *piWarningLightTimerRightID = 0;
        *piWarningLightTimerLeftID  = 0;
        
        // While we just stopped the lights, we need to make sure they are in the off state
        digitalWrite(kPinAddrGateLightsControlLeft, kWarningLightsOff);
        digitalWrite(kPinAddrGateLightsControlRight, kWarningLightsOff);
        
        // Shut off the warning bell
        digitalWrite(kPinAddrGateBellControl, kWarningBellOff);
        
        // this turns OFF power to the gate are motor
        digitalWrite(kPinAddrGateArmControlMotorDirection, kGateArmControlMotorDown);  
        digitalWrite(kPinAddrGateArmControlMotorPower, kGateArmControlMotorOff);
         
        Serial.println("Motor: Off");
        Serial.println("Bell/Lights: Off");
        Serial.println("Gate is Up");
        
        *pbMotorDirectionFlag = false;
        *pbMotorOnFlag = false;
        
        *pulGateDownEventStartTime = 0;
        
        if (*pbDutyCycleExceededFlag == false)
        {
            // log the total motor run time.  We need this, as the motor has a 10% duty cycle
            ulMotorRunningTotalSecondsThisEvent = millis() - ulMotorRunningStartTimeThisEvent; 
            *pulMotorRunningTotalSeconds = *pulMotorRunningTotalSeconds + ulMotorRunningTotalSecondsThisEvent;
        
            Serial.print("Total Motor Run Time: ");
            Serial.println(*pulMotorRunningTotalSeconds); 
        }
        else
        {
            *pbDutyCycleExceededFlag = false;  
          
        }  
        
        // set the flag that the motor is not running
        *pbMotorRunning = false;

    } 
    
    *pbGateState = kGateInTheUpPosition;
    
    // reset the state machine to the default state
    *piGateUpState = kGateMovingUp_State_Debouce;

    return;  
  
}  // endof  GateUpMotorOffState() 


