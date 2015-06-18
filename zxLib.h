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

typedef unsigned long _prog_addressT;
#define _EE_WORD         2
#define _memcpy_p2d16(dest, src, len)  _memcpy_helper(src, dest, len, 0) 
#define _wait_eedata() { while (NVMCONbits.WR); }
#define _erase_eedata  _eedata_helper1
#define _write_eedata_word  _eedata_helper3
extern void _eedata_helper1(_prog_addressT dst, int len);
extern void _eedata_helper3(_prog_addressT dst, int dat);
extern _prog_addressT _memcpy_helper(_prog_addressT src, void *dst,
                                     unsigned int len, int flags);


typedef unsigned int _delayT;

#define laserDuring3Baseline 3
#define laserDuring4Baseline 4
#define laserDuringBaseline 5
#define laserDuringBaseAndResponse 7
#define laserDuringBaselineNDelay 9
#define laserDuringDelay 10
#define laserDuringDelayChR2 11
#define laserDelayDistractor 12
#define laserRampDuringDelay 14
//#define laserLDuringDelay 15
//#define laserRDuringDelay 16
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
//#define laserDuring1Terice 81
//#define laserDuring2Terice 82
//#define laserDuring3Terice 83
#define laserDuringLateHalf 90
#define laserDuring1Quarter 91
#define laserDuring2Quarter 92
#define laserDuring3Quarter 93
#define laserDuring4Quarter 94
#define laserDuringResponseDelay 95

#define laserDuring12s1Quarter 96
#define laserDuring12s2Quarter 97
#define laserDuring12s3Quarter 98
#define laserDuring12s4Quarter 99

#define trialTypeDnmsSwitch 100
#define trialTypeGoNogoSwitch 110
#define laser4sRamp 121
#define laser2sRamp 122
#define laser1sRamp 123
#define laser_5sRamp 124
#define laserSufficiency 130


//#define atTrialStart 10
#define fourSecBeforeFirstOdor 4
#define threeSecBeforeFirstOdor 5
#define oneSecBeforeFirstOdor 10
#define at200mSBeforeFirstOdor 18
#define atFirstOdorBeginning 20
#define atFirstOdorEnd 30
#define atDelayBegin 40
#define atDelay_5SecIn 42
#define atDelay1SecIn 50
#define atDelay1_5SecIn 51
#define atDelay2SecIn 52
#define atDelay2_5SecIn 53
#define atDelay3SecIn 54
#define atDelay_5ToMiddle 55
#define atDelayMiddle 56
#define atDelay8_5SecIn 57
#define atDelay9SecIn 58
#define atDelayLast2_5SecBegin 59
#define atDelayLast2SecBegin 60
#define atDelayLast1_5SecBegin 61
#define atDelayLastSecBegin 63
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
#define _LASER_LR_EACH_QUARTER 20
#define _LASER_EACH_QUARTER 21
#define _LASER_12s_LR_EACH_QUARTER 25
#define _LASER_12s_EACH_QUARTER 26
#define _LASER_VARY_LENGTH 30
#define _LASER_LR_EVERYTRIAL 40
//#define laserCycleDelayLaser 30

#define _LICK_LEFT (PORTDbits.RD12 || !PORTDbits.RD14)
#define _LICK_RIGHT (PORTDbits.RD13 || !PORTDbits.RD15)
#define _LICK_ANY (PORTDbits.RD12 || PORTDbits.RD13 || !PORTDbits.RD14 || !PORTDbits.RD15)
#define _LICKING_LEFT 2
#define _LICKING_RIGHT 3

#define _SHAPPING_TASK 10
#define _DNMS_LEARNING_TASK 19
#define _DNMS_TASK 20
//#define _ASSOCIATE_SHAPPING_TASK 24
//#define _ASSOCIATE_TASK 25
#define _GONOGO_TASK 30
#define _DNMS_LR_TEACH 38
#define _DNMS_LR_LEARNING_TASK 39
#define _DNMS_LR_TASK 40
#define _GONOGO_LR_TEACH 49
#define _GONOGO_LR_TASK 50
#define _NO_ODOR_CATCH_TRIAL_TASK 60

#define _EEP_DUTY_LOW_L_OFFSET 0
#define _EEP_DUTY_HIGH_L_OFFSET 2
#define _EEP_DUTY_LOW_R_OFFSET 4
#define _EEP_DUTY_HIGH_R_OFFSET 6

//extern float odorLength;
extern char* zxVer;

extern unsigned int laserTimer;
extern unsigned int laserOnTime ;
extern unsigned int laserOffTime ;

extern unsigned int ramp;
extern unsigned int ramping;
extern unsigned char pwmDutyHiR;
extern unsigned char pwmDutyLoR;
extern unsigned int laserTimerOn;
extern unsigned int licking;
extern const char odorTypes[];
extern unsigned int timeFilter;
extern unsigned int laserTrialType;
extern unsigned int taskType;
extern unsigned int lickFlag;
extern unsigned int fullduty;
extern unsigned int lickLCount;
extern unsigned int lickRCount;

extern const char _DEBUGGING;


void shuffleArray(unsigned int orgArray[], unsigned int arraySize);
void lcdWriteNumber(int n, int digits, int x, int y);
void lcdWriteChar(char ch, int x, int y);
void lcdWriteString(char* s);

void odorDepeltion(int totalTrial);

//void zxDNMSSwiFtchTrial(int DNMS, int FirstOdorIn, float odorLength, float OdorDelayIn, int SecondOdorIn, float WaterLIn, int IntervalIn, float delay_before_reward);
void zxLaserTrial(int type, int FirstOdorIn, float odorLength, _delayT OdorDelayIn, int SecondOdorIn, float WaterLIn, int ITI, float delay_before_reward, int laserOnTrial);
void callFunction(int n);


void wait_ms(int time);
//void setLaser(void);

void feedWater();
void splash(char s1[], char s2[]);

void initZXTMR(void);
void protectedSerialSend(int type, int value);
int ppLaserSessions(int trialsPerSession, int missLimit, int totalSession);

#endif	/* ZXLIB_H */
