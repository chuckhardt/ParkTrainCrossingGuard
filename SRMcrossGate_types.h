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

#ifndef SRMcrossGate_Types_h
#define SRMcrossGate_Types_h

const bool kGateInDownPosition = 1;
const bool kGateInTheUpPosition = 0;

const bool kTrackOccupiedSequence = 1;
const bool kTrackVacantSequence = 0;

const bool kLowerGate = 1;
const bool kRaiseGate = 0;

const int kInitializing = 0;
const int kTrackOccupied = 1;
const int kTrackVacant = 2;

const int kGateInitalize_LightsBellsAndDirection = 0;
const int kGateInitalize_MotorDirectionDelay = 1;
const int kGateInitalize_MotorOn = 2;
const int kGateInitalize_MotorOff = 3;

const int kGateMovingDown_State_LightsAndBells = 0;
const int kGateMovingDown_State_LightsAndBellsDelay = 1;
const int kGateMovingDown_State_MotorDirection = 2;
const int kGateMovingDown_State_MotorOn = 3;
const int kGateMovingDown_State_MotorOnDelay = 4;
const int kGateMovingDown_State_MotorOff = 5;

const int kGateMovingUp_State_Debouce = 0;
const int kGateMovingUp_State_MotorDirection = 1;
const int kGateMovingUp_State_MotorDirectionDelay = 2;
const int kGateMovingUp_State_MotorOn = 3;
const int kGateMovingUp_State_MotorOnDelay = 4;
const int kGateMovingUp_State_MotorOff = 5;

const int kPinAddrGateBellControl = 12;
const int kPinAddrGateLightsControlRight = 11;
const int kPinAddrGateLightsControlLeft = 10;
const int kPinAddrGateArmControlMotorDirection = 9;
const int kPinAddrGateArmControlMotorPower = 8;
const int kPinAddrGateStatusLED = 3;
const int kPinAddrGateTrackSensor = 2;

//const unsigned long kMaxTrackOccupiedFaultCount = 20000;
//const unsigned long kMinTimeTrackMustBeVacantToClearFault = 20000;
const unsigned long kMaxGateDownTimelimitReached = 20000;

const unsigned long kZeroSeconds = 0;
const unsigned long kOneSecond = 1000;
const unsigned long kThreeSeconds = 3000;
const unsigned long kFourSeconds = 4000;
const unsigned long kFiveSeconds = 5000;
const unsigned long kSevenSeconds = 7000;
const unsigned long kTenSeconds = 10000;
const unsigned long kThirteenSeconds = 13000;
const unsigned long kTwentySeconds = 20000;
const unsigned long kThirtySeconds = 30000;


const bool kWarningBellOn = 0;
const bool kWarningBellOff = 1;

const bool kWarningLightsOn = 0;
const bool kWarningLightsOff = 1;

const bool kGateArmControlMotorOn = 1;
const bool kGateArmControlMotorOff = 0;

const bool kGateArmControlMotorUp = 1;
const bool kGateArmControlMotorDown = 0;

const bool kStatusLEDon = 1;
const bool kStatusLEDoff = 0;

const unsigned long kMaxDutyCycleLimitReached = 80000;

#endif

