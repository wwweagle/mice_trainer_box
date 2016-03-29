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

typedef struct {
    unsigned char L_Hi;
    unsigned char L_Lo;
    unsigned char R_Hi;
    unsigned char R_Lo;
    const unsigned char fullDuty;
} PWM_T;

typedef struct {
    unsigned int timer;
    unsigned int onTime;
    unsigned int offTime;
    unsigned int ramp;
    unsigned int ramping;
    unsigned int on;
    unsigned int side;
} LASER_T;

typedef struct {
    unsigned int current;
    unsigned int filter;
    unsigned int flag;
    unsigned int LCount;
    unsigned int RCount;
} LICK_T;

typedef struct {
    unsigned int stim1Length;
    unsigned int stim2Length;
    unsigned int distractorLength;
    unsigned int currentDistractor;
} STIM_T;

#define _EE_WORD         2
#define _memcpy_p2d16(dest, src, len)  _memcpy_helper(src, dest, len, 0) 
#define _wait_eedata() { while (NVMCONbits.WR); }
#define _erase_eedata  _eedata_helper1
#define _write_eedata_word  _eedata_helper3
extern void _eedata_helper1(_prog_addressT dst, int len);
extern void _eedata_helper3(_prog_addressT dst, int dat);
extern _prog_addressT _memcpy_helper(_prog_addressT src, void *dst,
        unsigned int len, int flags);
extern void safe_wait_ms(int duration);


typedef unsigned int _delayT;

#define laserOff 0
#define laserDuring3Baseline 3
#define laserDuring4Baseline 4
#define laserDuringBaseline 5
#define laserDuringDelay_Odor2 6
#define laserDuringBaseAndResponse 7
#define laserDuringBaselineNDelay 9
#define laserDuringDelay 10
#define laserDuringDelayChR2 11

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
#define atDelay500msToMiddle 55
#define atDelayMiddle 56
#define atDelay8_5SecIn 57
#define atDelay9SecIn 58
#define atDelayLast2_5SecBegin 59
#define atDelayLast2SecBegin 60
#define atDelayLast1_5SecBegin 61
#define atDelayLastSecBegin 63
#define atDelayLast500mSBegin 65
//#define atDelayLast200mSBegin 68
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
#define LASER_OTHER_TRIAL 1
#define LASER_NO_TRIAL 2
#define LASER_EVERY_TRIAL 3
//#define laserFollowOdorA 4
//#define laserFollowOdorB 5
//#define laser1and2Half 6
//#define laser3and4Quarter 10
#define LASER_LR_EACH_QUARTER 20
#define LASER_EACH_QUARTER 21
#define LASER_12s_LR_EACH_QUARTER 25
#define LASER_12s_EACH_QUARTER 26
#define LASER_VARY_LENGTH 30
#define LASER_LR_EVERYTRIAL 40
#define LASER_LR_EVERY_OTHER_TRIAL 42
#define LASER_INCONGRUENT_CATCH_TRIAL 45





#define LICK_LEFT (PORTDbits.RD12 || !PORTDbits.RD14)
#define LICK_RIGHT (PORTDbits.RD13 || !PORTDbits.RD15)
#define LICK_ANY (PORTDbits.RD12 || PORTDbits.RD13 || !PORTDbits.RD14 || !PORTDbits.RD15)
#define LICKING_LEFT 2
#define LICKING_RIGHT 3
#define LICKING_BOTH 127

#define SHAPPING_TASK 10
#define DNMS_LEARNING_TASK 19
#define DNMS_TASK 20
//#define _ASSOCIATE_SHAPPING_TASK 24
//#define _ASSOCIATE_TASK 25
#define GONOGO_TASK 30
#define DNMS_2AFC_TEACH 38
#define DNMS_LR_LEARNING_TASK 39
#define DNMS_2AFC_TASK 40
#define GONOGO_2AFC_TEACH 49
#define GONOGO_LR_TASK 50
#define NO_ODOR_CATCH_TRIAL_TASK 60
#define VARY_ODOR_LENGTH_TASK 70
#define OPTO_DPAL_TASK 80
#define OPTO_DPAL_SHAPING_TASK 81
#define DELAY_DISTRACTOR_EARLY_TASK 90
#define DELAY_DISTRACTOR_LATE_TASK 95


#define EEP_DUTY_LOW_L_OFFSET 0
#define EEP_DUTY_HIGH_L_OFFSET 2
#define EEP_DUTY_LOW_R_OFFSET 4
#define EEP_DUTY_HIGH_R_OFFSET 6



#define PERM_INFO 21

#define DMS_LR_Teach_LOFF 100
#define DMS_LR_Teach_LON 101
#define MSWOD_LR_Teach_LOFF 102
#define DMS_LR_Teach_ONOFF 103
#define DMS_LR_5Delay_Laser 104
#define DMS_LR_8Delay_Laser 105
#define DMS_LR_12Delay_Laser 106
#define DMS_LR_1Odor_Laser 107
#define DMS_LR_2Odor_Laser 108
#define DMS_LR_bothOdor_Laser 109
#define DMS_LR_baseline_Laser 110
#define DMS_LR_response_Laser 111


//extern float odorLength;
char * getVer(void);

extern const char odorTypes[];
extern unsigned int laserSessionType;
extern unsigned int taskType;
//extern unsigned int licking;
//extern unsigned int timeFilter;
//extern unsigned int lickFlag;
//extern unsigned int lickLCount;
//extern unsigned int lickRCount;

extern const int _DEBUGGING;


void shuffleArray(unsigned int orgArray[], unsigned int arraySize);
void lcdWriteNumber(int n, int digits, int x, int y);
void lcdWriteChar(char ch, int x, int y);
void lcdWriteString(char* s);

void odorDepeltion(int totalTrial, int p3);

//extern static void zxLaserTrial(int type, int FirstOdorIn, float odorLength, _delayT OdorDelayIn, int SecondOdorIn, float WaterLIn, int ITI, float delay_before_reward, int laserOnTrial);
void callFunction(int n);


void wait_ms(int time);
//void setLaser(void);

//void feedWater();
void splash(char s1[], char s2[]);

void initZXTMR(void);
void protectedSerialSend(int type, int value);
int ppLaserSessions(int trialsPerSession, int missLimit, int totalSession);

int read_eeprom(int offset);
void write_eeprom(int offset, int value);

#endif	/* ZXLIB_H */
