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

#ifndef SRMcrossGate_Utils_h
#define SRMcrossGate_Utils_h


// ***************************************************
//
// ReadTrackSensorAndDebouce()
//
// This function reads the track state IO pin to determine if 
// the track is occupied or not.
//
// ****************************************************
int ReadTrackSensorAndDebouce();

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
                              bool *pbDutyCycleExceededFlag);

// ***************************************************
//
// WarningLightTimerStart()
//
// This function will start the warning lights timers
// If the timer cannot be alloated, an error message will be
// printed and the ardinuo restarted.
//
// ****************************************************
int WarningLightTimerStart(uint8_t uiArduinoPin, unsigned long ulTimerPeriod, uint8_t uiPinStartingValue, int iRepeatCount);

// ***************************************************
//
// MotorDutyCycleCalcuate()
//
// This function will adjust the motor running elaped time variable
// based on the total time the motor is not running.  As an example, for every
// 9 seonds the motor is off, the running time is decrmented by 1 second.
//
// ****************************************************
void MotorDutyCycleCalcuate(bool *pbMotorRunning, unsigned long *pulMotorRunningTotalSeconds);

#endif
