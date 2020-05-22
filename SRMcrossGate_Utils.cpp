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
extern bool  bMotorRunning;
//extern unsigned long ulMotorRunningTotalSeconds;
//extern unsigned long ulMotorNotRunningTotalSeconds;
extern unsigned long giGateUpStateAfterDelay;

//extern bool bDutyCycleExceededFlag;

// ***************************************************
//
// ReadTrackSensorAndDebouce()
//
// This function reads the track state IO pin to determine if 
// the track is occupied or not.
//
// ****************************************************
int ReadTrackSensorAndDebouce()
{
    int iCurrentTrackOcupationState;
    static int iPreviousTrackOcupationState = kTrackVacant;
    static unsigned long ulSensorChangeStartTime = 0;
    static unsigned long ulElapsedTime = 0;
  
    // read in the track state (is it Occupided or Vacant)
    iCurrentTrackOcupationState = digitalRead(kPinAddrGateTrackSensor);
  
    // has the track ocupation state changed?
    if (iCurrentTrackOcupationState != iPreviousTrackOcupationState)
    {
         // we need to make sure we debounce the state of the track.
         // as we do not want constant changes.
         if (ulSensorChangeStartTime == 0 )
         {
             // get the current time
             ulSensorChangeStartTime = millis();
             //Serial.print("State Change -- Debouce Started - ");
             //Serial.println(millis());
         }
         
         // calculate the elapsed time (current time - start time)
         ulElapsedTime = millis() - ulSensorChangeStartTime;
         if (ulElapsedTime >= 500)
         {
              iPreviousTrackOcupationState = iCurrentTrackOcupationState;
              //Serial.print("tate Change -- Debouce Completed - ");
              //Serial.println(millis()); 
              
              Serial.print("Track Sensor: ");
              if (iCurrentTrackOcupationState == kTrackOccupied)
              {
                 Serial.println("Detected");
                 digitalWrite(kPinAddrGateStatusLED, kStatusLEDon);
              }
              else
              {
                 Serial.println("Cleared");
                 digitalWrite(kPinAddrGateStatusLED, kStatusLEDoff);
              }
         }
         else
         {
              iCurrentTrackOcupationState = iPreviousTrackOcupationState;
         }  
         
    }
 
    // the track ocupation state has not changed since the last poll, or has bounce back between the states.   
    else
    {
        //iCurrentTrackOcupationState = iPreviousTrackOcupationState;
        ulSensorChangeStartTime = 0;
    }  
 
    return iCurrentTrackOcupationState;
        
}  //endof ReadTrackSensorAndDebouce()

// *****************************************************************************************
//
// ResetStateMachineIfNeeded()
//
// Determine what our track state is, we will reset the state machine if needed. 
//
// ****************************************************************************************
void ResetStateMachineIfNeeded(int *piTrackState,
                     unsigned long *pulGateDownEventTotalElapsedTime,
                     unsigned long *pulGateDownEventElapsedStartTime,
                               int *piTrackOcupationState,
                     unsigned long *pulGateUpEventTimeSpentInSequence, 
                     unsigned long *pulGateUpEventStartTime, 
                               int *piWarningLightTimerRightID,
                               int *piWarningLightTimerLeftID,
                              bool *pbGateState,
                     unsigned long *pulGateDownEventStartTime,
                              bool *pbMotorDirectionFlag,
                              bool *pbMotorOnFlag,
                               int *piGateDownState,
                     unsigned long *pulGateDownEventTimeSpentInSequence,
                               int *piGateUpState,
                              bool *pbMotorRunning,
                     unsigned long *pulMotorRunningTotalSeconds,
                     unsigned long *pulGateDownStateDelayBeforeGateUpEventStartTime,
                              bool *pbDutyCycleExceededFlag)
{
    // whenever we get the momentary track occupied signal, we reset the count  
    if (*piTrackState == kTrackOccupied) 
    {
        *pulGateDownEventTotalElapsedTime = 0;
        *pulGateDownEventElapsedStartTime  = 0;
          
        // if the track occupation is triggered, and the state is vacant, then 
        // we need to reset the state machine.
        if (*piTrackOcupationState == kTrackVacant)
        {
            *piTrackOcupationState = kTrackOccupied;
              
            // if we were raising the gate, we need to reset the motor duty cycle counter  
            if ((*pbGateState == kGateInDownPosition) && (*piGateUpState == kGateMovingUp_State_MotorOnDelay))
            {
                // we are going to reset the motor duty cylce timer, as we need to close the gate
                if ((*pulMotorRunningTotalSeconds > kMaxDutyCycleLimitReached) && (*pbDutyCycleExceededFlag != true))
                {
                     *pulMotorRunningTotalSeconds = 0; 
                     Serial.println("RESET -- Motor Running Seconds");    
                  
                }  
                
                // the gate was moving back up, so stop it, in its tracks.
                GateUpMotorOffState(pulGateUpEventStartTime, 
                                    piWarningLightTimerRightID,
                                    piWarningLightTimerLeftID,
                                    pbGateState,
                                    pulGateDownEventStartTime,
                                    pbMotorDirectionFlag,
                                    pbMotorOnFlag,
                                    piGateUpState,
                                    pbMotorRunning,
                                    pulMotorRunningTotalSeconds,
                                    pbDutyCycleExceededFlag);
                  
                // reset the state machine, such that we start over.  
                *pbGateState = kGateInTheUpPosition;
                *piGateDownState = kGateMovingDown_State_LightsAndBells;
                *piGateUpState = kGateMovingUp_State_Debouce;
                
                *pulGateDownEventStartTime = 0;
                *pulGateDownEventTotalElapsedTime = 0;
                *pulGateDownEventTimeSpentInSequence = 0;
                
                *pulGateUpEventTimeSpentInSequence = 0;
                *pulGateDownStateDelayBeforeGateUpEventStartTime = millis();
            }
            
            // if the gate is currently down, then reset the start time, this will hold the gate down for an additional period of time   
            else if (*pbGateState == kGateInDownPosition)
            {
                *pulGateDownStateDelayBeforeGateUpEventStartTime = millis();
            }  
        }
      
        // the track is occupied  
        else
        {
            // If the gate is down, then we are going to reset the time.  This should keep the gate down an additonal period of time 
            if (*pbGateState == kGateInDownPosition) 
            {  
                *pulGateDownStateDelayBeforeGateUpEventStartTime = millis(); 
                //Serial.println("Reseting the time we stay in the down position");
            }  

            *pulGateDownEventStartTime = 0;
        }  
         
    }
  
} // ResetStateMachineIfNeeded()  

// ***************************************************
//
// WarningLightTimerStart()
//
// This function will start the warning lights timers
// If the timer cannot be alloated, an error message will be
// printed and the ardinuo restarted.
//
// ****************************************************
int WarningLightTimerStart(uint8_t uiArduinoPin, unsigned long ulTimerPeriod, uint8_t uiPinStartingValue, int iRepeatCount)
{

     int iTimerIDnumber = -1;    
  
     iTimerIDnumber = gCrossingGateTimer.oscillate(uiArduinoPin, ulTimerPeriod, uiPinStartingValue, iRepeatCount);
        
     if (iTimerIDnumber == -1)
     {
          Serial.print("Timer Error, Pin: ");
          Serial.println(uiArduinoPin);
            
     }
        
     return iTimerIDnumber;
        
}  //endof WarningLightTimerStart()

// ***************************************************
//
// MotorDutyCycleCalcuate()
//
// This function will adjust the motor running elaped time variable
// based on the total time the motor is not running.  As an example, for every
// 9 seonds the motor is off, the running time is decrmented by 1 second.
//
// ****************************************************
void MotorDutyCycleCalcuate(bool *pbMotorRunning, unsigned long *pulMotorRunningTotalSeconds)
{
    static unsigned long ulPreviousTimeStamp = 0;
    static unsigned long ulCurrentTimeStamp = 0;
    unsigned long ulTimeDifference = 0;
    static unsigned long ulPrintOutCount = 0;
    unsigned long ulPreviousMotorRunningSeconds = 0;
  
    ulPreviousMotorRunningSeconds = *pulMotorRunningTotalSeconds;
    
    ulCurrentTimeStamp = millis();
    ulTimeDifference = ulCurrentTimeStamp - ulPreviousTimeStamp;
  
    // if the motor is not running, determine our duty cycle
    if (*pbMotorRunning == false) 
    {
        if (*pulMotorRunningTotalSeconds >= (ulTimeDifference/10))
        {
            *pulMotorRunningTotalSeconds = *pulMotorRunningTotalSeconds - ulTimeDifference / 10;
            
            if (ulPrintOutCount++ % 5 == 0)
            { 
                Serial.print("Motor Cooling Down, Remaining Seconds: ");
                Serial.println(*pulMotorRunningTotalSeconds / ((ulPreviousMotorRunningSeconds - *pulMotorRunningTotalSeconds) * 4));
            }      
        }
        else
        {
           *pulMotorRunningTotalSeconds = 0;         
        }  
        
    }  
    
    ulPreviousTimeStamp = ulCurrentTimeStamp;

} // MotorDutyCycleCalcuate()
      
