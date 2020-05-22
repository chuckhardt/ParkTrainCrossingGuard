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

#include "Arduino.h"
#include "Timer.h"
#include "SRMcrossGate_types.h"
#include "SRMcrossGate_UpDownControl.h"
#include "SRMcrossGate_Utils.h"

Timer gCrossingGateTimer;
int giMainLoopEventTimerID;

// ***************************************************
//
// setup()
//
// This is an Arduino function that is called when the processor
// has restarted.   We are using this function to initialize
// our timers and set various variables.
//
// ****************************************************
void setup()
{
  // have to initialize the serial port if we want to use if for debug
  Serial.begin(9600);
  
  // Setup the Arduino pins for input and output.  
  // Then set their initial state
  pinMode(kPinAddrGateTrackSensor, INPUT);
  
  pinMode(kPinAddrGateBellControl, OUTPUT);
  digitalWrite(kPinAddrGateBellControl, kWarningBellOff);
  
  pinMode(kPinAddrGateLightsControlLeft, OUTPUT);
  digitalWrite(kPinAddrGateLightsControlLeft, kWarningLightsOff);
  
  pinMode(kPinAddrGateLightsControlRight, OUTPUT);
  digitalWrite(kPinAddrGateLightsControlRight, kWarningLightsOff);
  
  pinMode(kPinAddrGateArmControlMotorPower, OUTPUT);
  digitalWrite(kPinAddrGateArmControlMotorPower, kGateArmControlMotorOff);
  
  pinMode(kPinAddrGateArmControlMotorDirection, OUTPUT);
  digitalWrite(kPinAddrGateArmControlMotorDirection, kGateArmControlMotorDown);
  
  pinMode(kPinAddrGateStatusLED, OUTPUT);
  digitalWrite(kPinAddrGateStatusLED, kStatusLEDoff);
  
  // We are going to start the main loop event.
  // Each time this timer kicks, we are going to check the state of the track, and take 
  // whatever action as needed.
  giMainLoopEventTimerID = gCrossingGateTimer.every(250, CrossingSignalMain);
  
  Serial.println("Crossing Guard Controller - Ver 1.08");
  
}  //endof setup()

// ***************************************************
//
// loop()
//
// This is the main Arduino function that is called once setup()
// has finished.   We are using this loop to kick off our
// timer.  The timers provide us with a pseudo 
// operating system.
//
// ****************************************************
void loop()
{
    gCrossingGateTimer.update();
    
}  //endof loop()

// *****************************************************************************************
//
// CrossingSignalMain()
//
// The main loop of the crossing guard program is actually a state machine.   
// Each time the timer goes off, this function is called.   All the state variables are 
// listed as static, such that their previous values are maintained. 
//
// ****************************************************************************************
void CrossingSignalMain()
{
  int iTrackState;
  
  static bool bMotorOnFlag  = false;
  static bool bMotorOffFlag = false;
  static bool bMotorDirectionFlag = false;
  
  static bool bGateState = kGateInTheUpPosition;
  
  static int iWarningLightTimerRightID = 0;
  static int iWarningLightTimerLeftID = 0;
  
  static int iTrackOcupationState = kInitializing;
  static int iGateInitializationState = kGateInitalize_LightsBellsAndDirection;
  static int iWarningLightRightTimerID = 0; 
  static int iWarningLightLeftTimerID = 0;
  
  static int iGateMovingDown_State = kGateMovingDown_State_LightsAndBells;
  static int iGateMovingUp_State = kGateMovingUp_State_Debouce; 
  
  static unsigned long ulGateDownEventStartTime = 0;
  static unsigned long ulGateDownEventTotalElapsedTime = 0;
  static unsigned long ulGateDownEventElapsedStartTime = 0;
  static unsigned long ulGateDownEventTimeSpentInSequence = 0;
  
  static unsigned long ulGateUpEventStartTime = 0;
  static unsigned long ulGateUpEventTimeSpentInSequence = 0;
  static unsigned long ulGateInitializeStartTime = 0;

  static bool bMotorRunning = false;
  static unsigned long ulMotorRunningTotalSeconds = 0;

  static unsigned long ulGateDownStateDelayBeforeGateUpEventStartTime = 0;
  
  static bool bDutyCycleExceededFlag = false;

  // we do not want to read the track state if we are initializing the gates (to the up position)
  if (iTrackOcupationState != kInitializing)
  { 
      // read in the track state (Occupided or Vacant)
      iTrackState = ReadTrackSensorAndDebouce();
        
        
      ResetStateMachineIfNeeded(&iTrackState,
                                &ulGateDownEventTotalElapsedTime,
                                &ulGateDownEventElapsedStartTime,
                                &iTrackOcupationState,
                                &ulGateUpEventTimeSpentInSequence, 
                                &ulGateUpEventStartTime, 
                                &iWarningLightTimerRightID,
                                &iWarningLightTimerLeftID,
                                &bGateState,
                                &ulGateDownEventStartTime,
                                &bMotorDirectionFlag,
                                &bMotorOnFlag,
                                &iGateMovingDown_State,
                                &ulGateDownEventTimeSpentInSequence,
                                &iGateMovingUp_State,
                                &bMotorRunning,
                                &ulMotorRunningTotalSeconds,
                                &ulGateDownStateDelayBeforeGateUpEventStartTime,
                                &bDutyCycleExceededFlag);  
  }
  
  MotorDutyCycleCalcuate(&bMotorRunning, &ulMotorRunningTotalSeconds);
  
  // we are going to switch on the state of the track
  // Is the track occupied or not?
  switch (iTrackOcupationState)
  {
      // The Gate system has come out of reset, and needs to be initialized
      case kInitializing:
      
          // Before we do anything, we need to make sure we initialize the gate.
          // This will reset the gate, if it gets stuck somewhere
          switch (iGateInitializationState)
          {
              case kGateInitalize_LightsBellsAndDirection:
              
                  InitializeTheGateTurnOnLightsBellsAndUpRelay(&iWarningLightRightTimerID, 
                                                               &iWarningLightLeftTimerID, 
                                                               &ulGateInitializeStartTime, 
                                                               &iGateInitializationState);
                  break;
              
              case kGateInitalize_MotorDirectionDelay:
              
                   InitializeMotorDirectionDelayState(&iGateInitializationState);
                   break;
              
              case kGateInitalize_MotorOn:
      
                  InitializeTheGateTurnOnUpMotor(ulGateInitializeStartTime, &iGateInitializationState, &bMotorRunning);
                  break;
      
              case kGateInitalize_MotorOff:
              default:
                  
                  InitializeTheGateTurnOffUpMotor(iWarningLightRightTimerID, iWarningLightLeftTimerID, &bMotorRunning, &ulMotorRunningTotalSeconds);  
                  iTrackOcupationState = kTrackVacant;
                  break;
          
          }  // iGateInitializationState
          
          break;
          
      // The Track is occupied
      case kTrackOccupied:
       
          // Keep track of where we are in the down sequence
          // If the time == 0, then this is the start of the down event
          if (ulGateDownEventStartTime == 0)
          {
              // we store off the time we started this particular gate event.
              ulGateDownEventStartTime = millis();
          }  
          
          // whenever the sensor switch is closed, the gate down time gets reset to zero 
          if (ulGateDownEventTotalElapsedTime == 0)
          {
              // we store off the time we started this particular gate event.
              ulGateDownEventElapsedStartTime = millis();
          }  
          
          // Check to see if the gate is in the down position?  
          switch (bGateState)
          {
              // The track is occupied and the gate is down, thus we have nothing to do.
              case kGateInDownPosition:  
              {
                  GateDownInactiveState(&iTrackOcupationState, 
                                        &ulGateDownEventTotalElapsedTime, 
                                        &ulGateDownEventElapsedStartTime,
                                        &bGateState,
                                        &iGateMovingUp_State,
                                        &ulGateDownStateDelayBeforeGateUpEventStartTime,
                                        &bDutyCycleExceededFlag);
                  break;  
                  
              }  // kGateDown
            
              // The Track is occupied, but in this case the gate is in the up postion, and we need to bring it down.
              case kGateInTheUpPosition:  
              {                
                  switch (iGateMovingDown_State)
                  {
                   
                      case kGateMovingDown_State_LightsAndBells:
                      
                          GateDownLightsBellsAndMotorDirectionState(ulGateDownEventStartTime, 
                                                                    //&ulGateDownEventTimeSpentInSequence, 
                                                                    &ulGateUpEventTimeSpentInSequence, 
                                                                    &iGateMovingDown_State, 
                                                                    &iWarningLightTimerRightID, 
                                                                    &iWarningLightTimerLeftID);
                          break;
                       
                      case kGateMovingDown_State_LightsAndBellsDelay:
                      
                           GateDownWarningLightsAndBellDelayState(&iGateMovingDown_State); 
                           break;
                       
                      case kGateMovingDown_State_MotorOn:
                      
                          GateDownMotorOnState(ulGateDownEventStartTime, 
                                               &bMotorOnFlag, 
                                               &iGateMovingDown_State,
                                               &bMotorRunning,
                                               &ulMotorRunningTotalSeconds,
                                               &bDutyCycleExceededFlag);
                          break;
                      
                     case kGateMovingDown_State_MotorOnDelay:
                     
                          GateDownMotorOnDelay(&iGateMovingDown_State);
                      
                          break;
                      
                      case kGateMovingDown_State_MotorOff:
                      default:
                      
                          GateDownMotorOffState(ulGateDownEventStartTime, 
                                                &bMotorOnFlag, 
                                                &bMotorOffFlag, 
                                                &iGateMovingDown_State, 
                                                &bGateState,
                                                &bMotorRunning, 
                                                &ulMotorRunningTotalSeconds,
                                                &ulGateDownStateDelayBeforeGateUpEventStartTime,
                                                &bDutyCycleExceededFlag);
                                                
                          break;
                          
                  } // iGateMovingDown_State 
                  
                  break;
              
              }  // kGateUp
              
              default:
              {
                  break;
              }
              
          }  // bGateState    
          
          // update our total elapsed time.
          ulGateDownEventTotalElapsedTime = millis() -  ulGateDownEventStartTime;
          break;
      
      // the track is vacant
      case kTrackVacant:
      default:
      {
          // the track is vacant, but is the gate in the up position?
          switch (bGateState)
          {
              // The gates Arm is up, and the lights & Bells are off.
              // All we are going to do is keep count of the number of seconds the gate is up.
              case kGateInTheUpPosition:
              {
                  GateUpInactiveState();
                  break; 
              }  
            
              // In this state the track is vacant, but the gates are still down
              // and the lights are flashing and the bells are a ringing.
              // We need to transistion everything to off.
              case kGateInDownPosition:
              {
                  switch (iGateMovingUp_State)
                  {
                      case kGateMovingUp_State_Debouce:
                      
                          GateUpDebouceState(&ulGateUpEventTimeSpentInSequence, 
                                             &ulGateUpEventStartTime, 
                                             &iGateMovingUp_State);
                          break;
                          
                      case kGateMovingUp_State_MotorDirection:
                      
                          GateUpMotorDirectionState(&ulGateUpEventStartTime, 
                                                    &bMotorDirectionFlag,
                                                    &iGateMovingUp_State);
                          break;
                          
                      case kGateMovingUp_State_MotorDirectionDelay:
                      
                          GateUpMotorDirectionDelayState(&iGateMovingUp_State);
                          break;
                          
                          
                      case kGateMovingUp_State_MotorOn:
                      
                          GateUpMotorOnState(&ulGateUpEventStartTime, 
                                             &bMotorOnFlag, 
                                             &iGateMovingUp_State,
                                             &bMotorRunning); 
                           break;
                          
                     case kGateMovingUp_State_MotorOnDelay:
                     
                          GateUpMotorOnDelayState(&iGateMovingUp_State);
                          break;

                      case kGateMovingUp_State_MotorOff:
                      default:
                      
                         GateUpMotorOffState(&ulGateUpEventStartTime, 
                                             &iWarningLightTimerRightID,
                                             &iWarningLightTimerLeftID,
                                             &bGateState,
                                             &ulGateDownEventStartTime,
                                             &bMotorDirectionFlag,
                                             &bMotorOnFlag,
                                             &iGateMovingUp_State,
                                             &bMotorRunning,
                                             &ulMotorRunningTotalSeconds,
                                             &bDutyCycleExceededFlag);
                          break;
                          
                  }  // iGateMovingUp_State
                  
                  break;
                 
              }  // case kGateDown  
            
          }  // bGateState  
        
      } // endof case kTrackVacant 
  
  } // iTrackOcupationState  
  
}  //endof CrossingSignalMain()



