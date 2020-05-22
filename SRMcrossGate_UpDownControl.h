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
#include "SRMcrossGate_types.h"

// ***************************************************
//
// InitializeTheGateTurnOnLightsBellsAndUpRelay()
//
// This function is called when we come out of reset (a power cycle).
// It's role is to turn on the lights, bells and switch on the Up motor.
//
// ****************************************************
void InitializeTheGateTurnOnLightsBellsAndUpRelay(int *piWarningLightTimerRightID, int *piWarningLightTimerLeftID, unsigned long *pulElapsedTime, int *piInitializationState);

// ***************************************************
//
// InitializeMotorDirectionDelayState()
//
// As part of the gate initialization sequence, we want to insert a delay
// between setting the gate direction motor, and switching in the motor.
// After which, we need to advance to the next state.
//
// ****************************************************
void InitializeMotorDirectionDelayState(int *piGateDownState);

// ***************************************************
//
// InitializeTheGateTurnOnUpMotor()
//
// This function is called when we come out of reset (a power cycle).
// It's role is to turn on the arm drive motor
//
// ****************************************************
void InitializeTheGateTurnOnUpMotor(unsigned long ulStartTime, int *piInitializationState, bool *pbMotorRunning);

// ***************************************************
//
// InitializeTheGateTurnOffUpMotor()
//
// This function is called when we come out of reset (a power cycle).
// It's role is to turn on the lights, bells and switch on the Up motor.
//
// ****************************************************
void InitializeTheGateTurnOffUpMotor(int iWarningLightTimerRightID, int iWarningLightTimerLeftID, bool *pbMotorRunning, unsigned long *pulMotorRunningTotalSeconds);

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
                                    bool *pbDutyCycleExceededFlag);

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
                                                         int *piWarningLightTimerLeftID);


// ***************************************************
//
// GateDownWarningLightsAndBellDelayState()
//
// As part of the gate down state, this function provides the motor direction
// relay delay.  It then advances to the next state.
//
// ****************************************************
void GateDownWarningLightsAndBellDelayState(int *piGateDownState);

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
                                   bool *pbDutyCycleExceededFlag);

// ***************************************************
//
// GateDownMotorOnDelay()
//
// As part of the gate down state, this function turns calculates the delay between
// motor on and off.  It then advances to the next state.
//
// ****************************************************
void GateDownMotorOnDelay(int *piGateDownState);

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
                                    bool *pbDutyCycleExceededFlag);


// ***************************************************
//
// GateUpInactiveState()
//
// This function is called whenever the gate is up, and the track is 
// vacant.  We use it to keep track of the motor on duty cycle.
//
// ****************************************************
void GateUpInactiveState(void);

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
                                  int *piGateUpState);


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
                                         int *piGateUpState);

// ***************************************************
//
// GateUpMotorDirectionDelayState()
//
// As part of the gate up state, this function provides a delay between the selection
// of the motor direction, and the motor turning on. 
//
// ****************************************************
void GateUpMotorDirectionDelayState(int *piGateUpState);

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
                                 bool *pbMotorRunning);

// ***************************************************
//
// GateUpMotorOnDelay()
//
// As part of the gate up state, this function turns calculates the delay between
// motor on and off.  It then advances to the next state.
//
// ****************************************************
void GateUpMotorOnDelayState(int *piGateUpState);

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
                                  bool *pbDutyCycleExceededFlag);

