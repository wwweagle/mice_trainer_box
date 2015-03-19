/* 
 * File:   zxLib.h
 * Author: ZX
 *
 * Created on 2013/8/13, 9:24
 */

#ifndef ZXLIB_H
#define	ZXLIB_H

#ifdef	__cplusplus
extern "C" {
#endif

#ifdef	__cplusplus
}
#endif

//#define zxLibVer "z611"

#define lowThres 75
#define HighThres 80

#define laserDuring3Baseline 3
#define laserDuring4Baseline 4
#define laserDuringBaseline 5
#define laserDuringBaseAndResponse 7
#define laserDuringBaselineNDelay 9
#define laserDuringDelay 10
#define laserDuringDelayChR2 11
#define laserDuringOdor 20
#define laserDuring1stOdor 21
#define laserDuring2ndOdor 22
#define laserDuringBeginningToOneSecInITI 30
#define laserDuringEarlyDelay 40
#define laserDuringMidDelay 50
#define laserDuringLateDelay 60
#define laserNoDelayControlShort 69
#define laserNoDelayControl 70
#define laserDuringEarlyHalf 80
#define laserDuring1Terice 81
#define laserDuring2Terice 82
#define laserDuring3Terice 83
#define laserDuringLateHalf 90
#define laserDuring1Quarter 91
#define laserDuring2Quarter 92
#define laserDuring3Quarter 93
#define laserDuring4Quarter 94
#define laserDuringResponseDelay 95
#define trialTypeDnmsSwitch 100
#define trialTypeGoNogoSwitch 110
#define laser4sRamp 121
#define laser2sRamp 122
#define laser1sRamp 123
#define laser_5sRamp 124

//#define atTrialStart 10
#define fourSecBeforeFirstOdor 4
#define threeSecBeforeFirstOdor 5
#define oneSecBeforeFirstOdor 10
#define at200mSBeforeFirstOdor 18
#define atFirstOdorBeginning 20
#define atFirstOdorEnd 30
#define atDelayBegin 40
#define atDelay_5SecIn 45
#define atDelay1SecIn 50
#define atDelay1_5SecIn 51
#define atDelay2SecIn 52
#define atDelay_5ToMiddle 54
#define atDelayMiddle 55
#define atDelayMiddleEnd 56
#define atDelayLast2_5SecBegin 57
#define atDelayLast2SecBegin 58
#define atDelayLast1_5SecBegin 59
#define atDelayLastSecBegin 60
#define atDelayLast500mSBegin 65
#define atDelayLast200mSBegin 68
#define atSecondOdorBeginning 70
#define atSecondOdorEnd 80
//#define atRewardDelayBeginning 90
#define atRewardBeginning 100
//#define atRewardBeginning 110
#define atRewardEnd 120
#define atITIBeginning 130
#define atITIOneSecIn 140
#define atITILastSecIn 150
#define atITIEnd 160


//Laser Delivery Type, including ZJ's variety
#define _LASER_OTHER_TRIAL 1
#define _LASER_NO_TRIAL 2
#define _LASER_EVERY_TRIAL 3
//#define laserFollowOdorA 4
//#define laserFollowOdorB 5
//#define laser1and2Half 6
//#define laser3and4Quarter 10
#define laserEachQuarter 20
#define laserVaryLength 30
//#define laserCycleDelayLaser 30

#define _LICK_LEFT (PORTDbits.RD12 || !PORTDbits.RD14)
#define _LICK_RIGHT (PORTDbits.RD13 || !PORTDbits.RD15)
#define _LICK_ANY (PORTDbits.RD12 || PORTDbits.RD13 || !PORTDbits.RD14 || !PORTDbits.RD15)
#define _LICKING_LEFT 2
#define _LICKING_RIGHT 3

#define _SHAPPING_TASK 10
#define _DNMS_LEARNING_TASK 19
#define _DNMS_TASK 20
#define _ASSOCIATE_SHAPPING_TASK 24
#define _ASSOCIATE_TASK 25
#define _GONOGO_TASK 30
#define _DNMS_LR_LEARNING_TASK 39
#define _DNMS_LR_TASK 40
#define _GONOGO_LR_TRAIN 49
#define _GONOGO_LR_TASK 50

#define _EEP_FREQ_LOW_OFFSET 0
#define _EEP_FREQ_HIGH_OFFSET 2

extern float odorLength;
extern char* zxVer;
extern unsigned int licking, lickFlag;
void setOdorLength(float in);

void shuffleArray(unsigned int orgArray[], unsigned int arraySize);
void lcdWriteNumber(int n, int digits, int x, int y);
void lcdWriteChar(char ch, int x, int y);
void lcdWriteString(char* s);

void odorDepeltion(int totalTrial);

//void zxDNMSSwitchTrial(int DNMS, int FirstOdorIn, float odorLength, float OdorDelayIn, int SecondOdorIn, float WaterLIn, int IntervalIn, float delay_before_reward);
void zxLaserTrial(int type, int FirstOdorIn, float odorLength, float OdorDelayIn, int SecondOdorIn, float WaterLIn, int ITI, float delay_before_reward, int laserOnTrial);
void callFunction(int n);

//
//void zxGoNogoTrial(int go_odor, int isGo, int laserOn, float odorlen, int delay, float waterlen,
//        int ITI);
void wait_ms(int time);
//void setLaser(void);

void feedWater();
void splash(char * s1, char * s2);

void initZXTMR(void);
void protectedSerialSend(int type, int value);

#endif	/* ZXLIB_H */
