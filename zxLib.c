#include "commons.h"
#include "limits.h"
#include "zxLib.h"

//DEBUG


STIM_T stims = {.stim1Length = 1000, .stim2Length = 1000, .distractorLength = 1000};
LASER_T laser = {.timer = 0, .onTime = 65535u, .offTime = 0, .ramp = 0, .ramping = 0, .on = 0, .side = 3};
PWM_T pwm = {.L_Hi = 0xfe, .R_Hi = 0xfe, .L_Lo = 0, .R_Lo = 0, .fullDuty = 0xfe};
LICK_T lick = {.current = 0, .filter = 0, .flag = 0, .LCount = 0, .RCount = 0};
const char odorTypes[] = " WBJlRQMN0123456789012345678901234567890123456789";
unsigned int laserTrialType = LASER_EVERY_TRIAL;
unsigned int taskType = DNMS_TASK;
unsigned int wait_Trial = 1u;
unsigned long timeSum = 0u;
unsigned long timerCounter32;

unsigned int highLevelShuffleLength = 12u;

const _prog_addressT EE_Addr = 0x7ff000;

static void zxLaserTrial(int type, int firstOdor, STIM_T odors, _delayT interOdorDelay, int secondOdor, float waterPeroid, int ITI, float delay_before_reward, int laserOnTrial);
unsigned int getFuncNumberMarquee(int targetDigits, char input[], int l);

char * getVer(void) {
    static char ver[16] = {'Z', 'X', ' '};
    char date[] = __DATE__;
    char time[] = __TIME__;
    int i;
    for (i = 0; i < 6; i++) {
        ver[i + 3] = date[i];
    }
    ver[9] = ' ';
    for (i = 0; i < 5; i++) {
        ver[i + 10] = time[i];
    }
    return ver;
}

void initZXTMR(void) {

    //Stop general timer
    IEC0bits.T2IE = 0;
    IFS0bits.T2IF = 0;
    timerCounterI = 0;
//    timerCounterJ = 0;
    timerCounter32 = 0;
    //

    TMR1 = 0;
    PR1 = TMR_RATE / 200;
    IFS0bits.T1IF = 0;
    IEC0bits.T1IE = 1;
    T1CON = 0x8020;
    ConfigIntTimer1(T1_INT_PRIOR_5 & T1_INT_ON);

    lick.LCount = 0;
    lick.RCount = 0;
}

inline int filtered(void) {
//    return (timerCounterJ < lick.filter || timerCounterJ > lick.filter + 50u);
    return (timerCounter32 < lick.filter || timerCounter32 > lick.filter + 50u);
}

inline void tick(int i) {
    laser.timer += i;
    timerCounterI += i;
//    if (timerCounterJ == 65535) {
//        timerCounterJ = 0;
//    } else {
//        timerCounterJ += i;
//    }
    if (timerCounter32 == LONG_MAX) {
        timerCounter32 = 0;
    } else {
        timerCounter32 += i;
    }
}

static void resetTimerCounter32() {
//    timerCounterJ = 65535;
//    timeSum = 0;
//    while (timerCounterJ > 1);
    timerCounter32 = LONG_MAX;
    timeSum = 0;
    while (timerCounter32 > 1);
}

static void waitTimer32(unsigned int dT) {
    timeSum += dT;
    while (timerCounter32 < timeSum);
}

static int isLikeOdorA(int odor) {
    if (odor == 2 || odor == 5 || odor == 7 || odor == 9) return 1;
    return 0;
}

void __attribute__((__interrupt__, no_auto_psv)) _T1Interrupt(void) {
    IFS0bits.T1IF = 0;
    tick(5);


    if (laser.on && (laser.timer % (laser.onTime + laser.offTime) < laser.onTime)) {
        //    if (laser.on) {
        if (laser.on % 2) {
            Out4 = 1;
            Nop();
            Nop();
            PDC2 = pwm.L_Lo;
        }

        if ((laser.on >> 1) % 2) {
            Out5 = 1;
            Nop();
            Nop();
            PDC4 = pwm.R_Lo;
        }
        laser.ramping = laser.ramp;


    } else {
        Out4 = 0;
        __builtin_nop();
        __builtin_nop();
        Out5 = 0;
        if (laser.ramping >= 5) {
            laser.ramping -= 5;
            if (PDC2 != pwm.fullDuty)
                PDC2 = (pwm.L_Hi - pwm.L_Lo) * (1 - ((float) laser.ramping / laser.ramp)) + pwm.L_Lo;
            if (PDC4 != pwm.fullDuty)
                PDC4 = (pwm.R_Hi - pwm.R_Lo) * (1 - ((float) laser.ramping / laser.ramp)) + pwm.R_Lo;
        } else {
            PDC2 = pwm.fullDuty;
            Nop();
            Nop();
            PDC4 = pwm.fullDuty;
        }

    }
    if (LICK_LEFT && LICK_RIGHT) {
        Out1 = 1;
        Nop();
        Nop();
        Out6 = 1;
        lick.current = LICKING_BOTH;
    } else if (LICK_LEFT && lick.current != LICKING_LEFT && filtered()) {
//        lick.filter = timerCounterJ;
        lick.filter = timerCounter32;
        lick.current = LICKING_LEFT;
        Out1 = 1;
        lick.LCount++;
        Nop();
        localSendOnce(SpLick, taskType < 35 ? 1 : 2);
        //        tick(3);
    } else if (LICK_RIGHT && lick.current != LICKING_RIGHT && filtered()) {
//        lick.filter = timerCounterJ;
        lick.filter = timerCounter32;
        lick.current = LICKING_RIGHT;
        Out6 = 1;
        lick.RCount++;
        Nop();
        localSendOnce(SpLick, 3);
        //        tick(3);
    } else if (lick.current && !LICK_ANY) {

        lick.current = 0;
        Out1 = 0;
        Nop();
        Nop();
        Nop();
        Out6 = 0;
        Nop();
        Nop();
        Nop();
    }
}

static int setType(void) {

    int ports[] = {0, 2, 5, 7};
    return ports[getFuncNumber(1, "23 56 78")];
}

static int setSessionNum() {

    int sess[] = {20, 1, 5, 10, 15, 20, 30};
    return sess[getFuncNumber(1, "1 5 10 15 20 30")];
}

static int setDelay(int type) {
    if (type == 1) {
        int d[] = {0, 4, 5, 8, 12, 16, 20, 30, 40};
        unsigned int n = getFuncNumberMarquee(1, "0:0s 1:4s 2:5s 3:8s 4:12s 5:16s 6:20s 7:30s 8:40s ", 50);
        if (n > 0 && n < sizeof (d))
            return d[n];
        else return 5;
    }
    int n = getFuncNumber(1, "4 8 11 14 17");
    int d[] = {5, 4, 8, 11, 14, 17};
    return d[n];
}

static int setLaser() {
    int d[] = {LASER_NO_TRIAL, LASER_NO_TRIAL, LASER_OTHER_TRIAL, LASER_EVERY_TRIAL};
    int n = getFuncNumber(1, "None 1+1-  Every");
    if (n > 0 && n < 4)
        return d[n];
    else return LASER_NO_TRIAL;
}

//void setLaser(void) {
//    int n = getFuncNumber(1, "DC=1 20Hz=2");
//    switch (n) {
//        case 2:
//            laser.onTime = 20;
//            laser.offTime = 30;
//            break;
//        default:
//            laser.onTime = 65535;
//            laser.offTime = 0;
//    }
//}

void protectedSerialSend(int type, int value) {
#ifndef DEBUG
    IEC0bits.T1IE = 0;
    IEC0bits.T3IE = 0;
    unsigned int i;
    for (i = 0; i < 4000; i++); //1000 per ms
    localSendOnce(type, value);
    for (i = 0; i < 3000; i++); //1000 per ms
    tick(10);
    IEC0bits.T1IE = 1;
    IEC0bits.T3IE = 1;
#endif
}

static void turnOnLaser(unsigned int i) {
    laser.timer = 0;
    laser.on = i;

    if (i % 2) {
        Out4 = 1;
        Nop();
        Nop();
        PDC2 = pwm.L_Lo;
    }

    if ((i >> 1) % 2) {
        Out5 = 1;
        Nop();
        Nop();
        PDC4 = pwm.R_Lo;
    }


    lcdWriteChar('L', 4, 1);
    protectedSerialSend(SpLaserSwitch, i);
}

static void turnOffLaser() {
    laser.on = 0;
    Out4 = 0;
    Nop();
    Nop();
    Out5 = 0;
    if (laser.ramping == 0) {
        PDC2 = pwm.fullDuty;
        Nop();
        Nop();
        PDC4 = pwm.fullDuty;
    }
    lcdWriteChar('.', 4, 1);
    protectedSerialSend(SpLaserSwitch, 0);
}

static void assertLaser(int type, int step, int currentTrial) {
    if (currentTrial) {
        switch (type) {
            case laserDuringBeginningToOneSecInITI:
                if (step == atFirstOdorBeginning) {
                    turnOnLaser(3);
                } else if (step == atITIOneSecIn) {
                    turnOffLaser();
                }
                break;
            case laserDuringDelay:
                if (step == atFirstOdorEnd) {
                    turnOnLaser(3);
                } else if (step == atDelayLast200mSBegin) {
                    turnOffLaser();
                }
                break;
            case laserDuringDelayChR2:
                if (step == atDelay1SecIn) {
                    turnOnLaser(3);
                } else if (step == atDelayLastSecBegin) {
                    turnOffLaser();
                }
                break;
            case laserDuringDelay_Odor2:
                if (step == atDelay1SecIn) {
                    turnOnLaser(3);
                } else if (step == atSecondOdorEnd) {
                    turnOffLaser();
                }
                break;
            case laserDuringBaselineNDelay:
                if (step == atDelay1SecIn || step == threeSecBeforeFirstOdor) {
                    turnOnLaser(3);
                } else if (step == atDelayLastSecBegin || step == at200mSBeforeFirstOdor) {
                    turnOffLaser();
                }
                break;
            case laserDuringOdor:
                if (step == atFirstOdorBeginning || step == atSecondOdorBeginning) {
                    turnOnLaser(3);
                } else if (step == atFirstOdorEnd || step == atSecondOdorEnd) {
                    turnOffLaser();
                }
                break;
            case laserDuring1stOdor:
                if (step == atFirstOdorBeginning) {
                    turnOnLaser(laser.side);
                } else if (step == atFirstOdorEnd) {
                    turnOffLaser();
                }
                break;
            case laserDuring2ndOdor:
                if (step == atSecondOdorBeginning) {
                    turnOnLaser(3);
                } else if (step == atSecondOdorEnd) {
                    turnOffLaser();
                }
                break;
            case laserDuringEarlyHalf:
                if (step == atDelayBegin) {
                    turnOnLaser(3);
                } else if (step == atDelayMiddle) {
                    turnOffLaser();
                }
                break;

            case laserDuringLateHalf:
                if (step == atDelayMiddle) {
                    turnOnLaser(3);
                } else if (step == atSecondOdorBeginning) {
                    turnOffLaser();
                }
                break;

            case laserDuring1Quarter:
                if (step == atDelayBegin) {
                    turnOnLaser(laser.side);
                } else if (step == atDelay1_5SecIn) {
                    turnOffLaser();
                }
                break;
            case laserDuring2Quarter:
                if (step == atDelay2SecIn) {
                    turnOnLaser(laser.side);
                } else if (step == atDelay_5ToMiddle) {
                    turnOffLaser();
                }
                break;
            case laserDuring3Quarter:
                if (step == atDelayMiddle) {
                    turnOnLaser(laser.side);
                } else if (step == atDelayLast2_5SecBegin) {
                    turnOffLaser();
                }
                break;
            case laserDuring4Quarter:
                if (step == atDelayLast2SecBegin) {
                    turnOnLaser(laser.side);
                } else if (step == atDelayLast500mSBegin) {
                    turnOffLaser();
                }
                break;
            case laserDuring12s1Quarter:
                if (step == atDelayBegin) {
                    turnOnLaser(laser.side);
                } else if (step == atDelay2_5SecIn) {
                    turnOffLaser();
                }
                break;
            case laserDuring12s2Quarter:
                if (step == atDelay3SecIn) {
                    turnOnLaser(laser.side);
                } else if (step == atDelay_5ToMiddle) {
                    turnOffLaser();
                }
                break;
            case laserDuring12s3Quarter:
                if (step == atDelayMiddle) {
                    turnOnLaser(laser.side);
                } else if (step == atDelay8_5SecIn) {
                    turnOffLaser();
                }
                break;
            case laserDuring12s4Quarter:
                if (step == atDelay9SecIn) {
                    turnOnLaser(laser.side);
                } else if (step == atDelayLast500mSBegin) {
                    turnOffLaser();
                }
                break;
            case laserDuringResponseDelay:
                if (step == atSecondOdorEnd) {
                    turnOnLaser(3);
                } else if (step == atRewardBeginning) {
                    turnOffLaser();
                }
                break;
            case laserNoDelayControl:
                if (step == oneSecBeforeFirstOdor) {
                    turnOnLaser(3);
                } else if (step == atRewardBeginning) {
                    turnOffLaser();
                }
                break;
            case laserNoDelayControlShort:
                if (step == atFirstOdorBeginning) {
                    turnOnLaser(3);
                } else if (step == atSecondOdorEnd) {
                    turnOffLaser();
                }
                break;
            case laserDuringBaseline:
                if (step == threeSecBeforeFirstOdor) {
                    turnOnLaser(3);
                } else if (step == atFirstOdorBeginning) {
                    turnOffLaser();
                }
                break;
            case laserDuring3Baseline:
                if (step == fourSecBeforeFirstOdor) {
                    turnOnLaser(3);
                } else if (step == oneSecBeforeFirstOdor) {
                    turnOffLaser();
                }
                break;
            case laserDuring4Baseline:
                if (step == fourSecBeforeFirstOdor) {
                    turnOnLaser(3);
                } else if (step == at200mSBeforeFirstOdor) {
                    turnOffLaser();
                }
                break;
            case laserDuringBaseAndResponse:
                if (step == oneSecBeforeFirstOdor || step == atSecondOdorEnd) {
                    turnOnLaser(3);
                } else if (step == at200mSBeforeFirstOdor || step == atRewardBeginning) {

                    turnOffLaser();
                }
                break;
            case laser4sRamp:
                if (step == atDelay_5SecIn) {
                    turnOnLaser(3);
                } else if (step == atDelayLast500mSBegin) {
                    turnOffLaser();
                }
                break;
            case laser2sRamp:
                if (step == atDelayLast2_5SecBegin) {
                    turnOnLaser(3);
                } else if (step == atDelayLast500mSBegin) {
                    turnOffLaser();
                }
                break;
            case laser1sRamp:
                if (step == atDelayLast1_5SecBegin) {
                    turnOnLaser(3);
                } else if (step == atDelayLast500mSBegin) {
                    turnOffLaser();
                }
                break;
            case laser_5sRamp:
                if (step == atDelayLastSecBegin) {
                    turnOnLaser(3);
                } else if (step == atDelayLast500mSBegin) {
                    turnOffLaser();
                }
                break;
            case laserDelayDistractor:
                if (step == atDelay1SecIn) {
                    turnOnLaser(3);
                } else if (step == atDelay2SecIn) {
                    turnOffLaser();
                }
                break;
            case laserRampDuringDelay:
                if (step == atDelay1SecIn) {
                    turnOnLaser(laser.side);
                } else if (step == atDelayLast500mSBegin) {
                    turnOffLaser();
                }
                break;
        }
    }
}

void shuffleArray(unsigned int * orgArray, unsigned int arraySize) {
    if (arraySize == 0 || arraySize == 1)
        return;

    int iter;
    for (iter = 0; iter < arraySize; iter++) {
        orgArray[iter] = iter;
    }
    int index, temp;
    for (iter = arraySize - 1; iter > 0; iter--) {

        index = rand() % (iter + 1);
        temp = orgArray[index];
        orgArray[index] = orgArray[iter];
        orgArray[iter] = temp;
    }
}

void lcdWriteString(char s[]) {

    int iter;
    for (iter = 0; s[iter] && iter < 16; iter++) {
        lcd_data(s[iter]);
    }
}

int lcdMarqueeString(char s[], int l) {
    static unsigned int last;
    int startIdx = (timerCounter32 >> 9) % l;
    if (startIdx != last) {
        last = startIdx;
        home_clr();
        int i = 0;
        for (; i < 16; i++) {
            while (l - 1 < i + startIdx) {
                startIdx -= l;
            }
            lcd_data(s[i + startIdx]);
        }
        return 1;
    }
    return 0;

}

void lcdWriteNumber(int n, int digits, int x, int y) {
    if (digits < 1) {
        return;
    }
    LCD_set_xy(x, y);
    counts_dispose(n);

    switch (digits) {
        case 5:
        case 4:
            lcd_data(thous);
        case 3:
            lcd_data(hunds);

        case 2:
            lcd_data(tens);
    }
    lcd_data(ones);
}

void lcdWriteChar(char ch, int x, int y) {
    //    LCD_set_xy(x, y);

    lcd_cmd((y == 1 ? 0x7f : 0xbf) + x);
    lcd_data(ch);
}

unsigned int getFuncNumberMarquee(int targetDigits, char input[], int l) {
#ifdef DEBUG
    int n = 0;
    return n;
#else
    int bitSet[targetDigits];
    int bitValue[targetDigits];
    unsigned int n;
    int iter;
    int iter1;

    for (iter = 0; iter < targetDigits; iter++) {
        bitSet[iter] = 0;
        bitValue[iter] = -6;
    }

    home_clr();
    for (iter = 0; iter < targetDigits; iter++) {
        while (!bitSet[iter]) {
            if (lcdMarqueeString(input, l)) {
                line_2();
                for (iter1 = 0; iter1 < targetDigits; iter1++) {
                    lcd_data(bitValue[iter1] + 0x30);
                }
            }
            if (u2Received > 0) {
                bitValue[iter] = u2Received - 0x30;
                bitSet[iter] = 1;
                u2Received = -1;
            } else {
                Key_Event();
                if (hardwareKey) {
                    hardwareKey = 0;
                    bitValue[iter] = key_val;
                    bitSet[iter] = 1;
                }
            }
        }
        localSendOnce(SpCome, bitValue[iter]);
        safe_wait_ms(300);
    }
    n = 0;
    for (iter1 = 0; iter1 < targetDigits; iter1++) {
        n = n * 10 + bitValue[iter1];
    }
    return n;
#endif
}

void write_eeprom(int offset, int value) {

    _erase_eedata(EE_Addr + offset, _EE_WORD);
    _wait_eedata();
    _write_eedata_word(EE_Addr + offset, value);
    _wait_eedata();
}

int read_eeprom(int offset) {
    int temp;
    _memcpy_p2d16(&temp, EE_Addr + offset, 2);

    return temp;
}

static void processHit(float waterPeroid, int valve, int id) {
    protectedSerialSend(22, valve == 1 ? 1 : 2);
    Valve_ON(valve, pwm.fullDuty);
    wait_ms(waterPeroid * 1000);
    Valve_OFF(valve);
    currentMiss = 0;
    protectedSerialSend(SpHit, id);
    lcdWriteNumber(++hit, 3, 6, 1);
}

static void processFalse(int id) {
    currentMiss = 0;
    protectedSerialSend(SpFalseAlarm, id);
    lcdWriteNumber(++falseAlarm, 3, 6, 2);
}

static void processMiss(int id) {
    currentMiss++;
    protectedSerialSend(SpMiss, id);
    lcdWriteNumber(++miss, 3, 10, 1);
}

static void processLRTeaching(float waterPeroid, int LR) {
    int rr = rand() % 3;
    if (rr == 0) {
        lcdWriteChar(rr + 0x30, 4, 1);
        protectedSerialSend(22, LR == 2 ? 1 : 2);
        //        FreqL = lick.LCount;
        Valve_ON(LR == 2 ? 1 : 3, pwm.fullDuty);
        wait_ms(waterPeroid * 1000);
        Valve_OFF(LR == 2 ? 1 : 3);
        //        currentMiss = 0;
        protectedSerialSend(SpWater_sweet, LR);
    }
}

static void waterNResult(int firstOdor, int secondOdor, float waterPeroid) {
    lick.flag = 0;
    switch (taskType) {
            /*
             *DNMS
             */
        case DNMS_TASK:
        case SHAPPING_TASK:
        case OPTO_DPAL_TASK:
        case OPTO_DPAL_SHAPING_TASK:
        case NO_ODOR_CATCH_TRIAL_TASK:
        case VARY_ODOR_LENGTH_TASK:

            //        case _ASSOCIATE_TASK:
            //        case _ASSOCIATE_SHAPPING_TASK:

            ///////////Detect/////////////////
            for (timerCounterI = 0; timerCounterI < 500 && !lick.flag; lick.flag = lick.current);

            /////Reward
            if (!lick.flag) {
                if (isLikeOdorA(firstOdor) == isLikeOdorA(secondOdor)) {
                    protectedSerialSend(SpCorrectRejection, 1);
                    lcdWriteNumber(++correctRejection, 3, 10, 2);
                } else {
                    processMiss(1);
                    if ((taskType == SHAPPING_TASK /*|| taskType == _ASSOCIATE_SHAPPING_TASK*/) && ((rand() % 3) == 0)) {
                        protectedSerialSend(22, 1);
                        Valve_ON(water_sweet, pwm.fullDuty);
                        protectedSerialSend(SpWater_sweet, 1);
                        wait_ms(waterPeroid * 1000);
                        Valve_OFF(water_sweet);
                    }
                }
            } else if (isLikeOdorA(firstOdor) == isLikeOdorA(secondOdor)) {
                processFalse(1);
            } else {
                processHit(waterPeroid, 1, 1);
            }
            break;

            /*
             *DNMS-LR
             */
        case DNMS_2AFC_TASK:
            //
            ///////////Detect/////////////////
            for (timerCounterI = 0; timerCounterI < 500 && !(lick.flag == 2 || lick.flag == 3); lick.flag = lick.current);
            /////Reward
            if (!lick.flag) {
                processMiss((firstOdor != secondOdor) ? 2 : 3);
            } else if (!(lick.flag & 1) != !(firstOdor^secondOdor)) {
                processHit(waterPeroid, lick.flag & 1 ? 3 : 1, lick.flag);
            } else {
                processFalse(lick.flag == LICKING_LEFT ? 2 : 3);
            }
            break;

        case DNMS_2AFC_TEACH:
            //
            ///////////Detect/////////////////
            for (timerCounterI = 0; timerCounterI < 500 && !(lick.flag == 2 || lick.flag == 3); lick.flag = lick.current);
            /////Reward
            if (!lick.flag) {
                processMiss((firstOdor != secondOdor) ? 2 : 3);
                processLRTeaching(waterPeroid, (firstOdor != secondOdor) ? 2 : 3);
            } else if (!(lick.flag & 1) != !(firstOdor^secondOdor)) {
                processHit(waterPeroid, lick.flag & 1 ? 3 : 1, lick.flag);
            } else {
                processFalse(lick.flag == LICKING_LEFT ? 2 : 3);
                processLRTeaching(waterPeroid, (firstOdor != secondOdor) ? 2 : 3);
            }
            break;

        case GONOGO_LR_TASK:
        case GONOGO_2AFC_TEACH:
            //
            ///////////Detect/////////////////
            for (timerCounterI = 0; timerCounterI < 500 && !(lick.flag == 2 || lick.flag == 3); lick.flag = lick.current);

            /////Reward
            if (lick.flag == LICKING_LEFT && isLikeOdorA(firstOdor)) {
                processHit(waterPeroid, 1, 2);
            } else if (lick.flag == LICKING_RIGHT && !isLikeOdorA(firstOdor)) {
                processHit(waterPeroid, 3, 3);
            } else if (lick.flag) {
                processFalse(lick.flag == LICKING_LEFT ? 2 : 3);
            } else {
                processMiss(isLikeOdorA(firstOdor) ? 2 : 3);
            }
            break;

        case GONOGO_TASK:

            for (timerCounterI = 0; timerCounterI < 500 && !lick.flag; lick.flag = lick.current);

            /////Reward
            if (!lick.flag) {
                if (!isLikeOdorA(firstOdor)) {
                    protectedSerialSend(SpCorrectRejection, 1);
                    lcdWriteNumber(++correctRejection, 3, 10, 2);
                } else {
                    processMiss(1);
                }
            } else if (!isLikeOdorA(firstOdor)) {
                processFalse(1);
            } else {
                processHit(waterPeroid, 1, 1);
            }
            break;
    }
}

static void distractor(int distractOdor) {
    Valve_ON(distractOdor, pwm.fullDuty);
    protectedSerialSend(SpOdor_C, distractOdor);
    lcdWriteChar('d', 4, 1);
    wait_ms(stims.distractorLength - 10);
    Valve_OFF(distractOdor);
    lcdWriteChar('D', 4, 1);
}

static void waitTrial() {
    if (!wait_Trial) {
        return;
    }

    while (u2Received != 0x31) {
        protectedSerialSend(20, 1);
    }
    u2Received = -1;
}

static void zxLaserSessions(int stim1, int stim2, int laserType, _delayT delay, int ITI, int trialsPerSession, float WaterLen, int missLimit, int totalSession, float delay_before_reward) {

    //    wait_ms(1000);
    int currentTrial = 0;
    int currentSession = 0;

    protectedSerialSend(SpWater_bitter, pwm.L_Lo);
    protectedSerialSend(SpValve8, pwm.R_Lo);
    protectedSerialSend(SpStepN, taskType);

    while ((currentMiss < missLimit) && (currentSession++ < totalSession)) {
        //        protectedSerialSend(SpOdorDelay, delay);
        protectedSerialSend(SpSess, 1);

        splash("    H___M___ __%", "S__ F___C___t___");

        lcdWriteNumber(currentSession, 2, 2, 2);
        hit = miss = falseAlarm = correctRejection = 0;
        unsigned int lastHit = 0;
        unsigned int shuffledList[4];
        unsigned int shuffledLongList[highLevelShuffleLength];
        shuffleArray(shuffledLongList, highLevelShuffleLength);
        int firstOdor, secondOdor;
        int lastOdor1;
        int lastOdor2;
        for (currentTrial = 0; currentTrial < trialsPerSession && currentMiss < missLimit;) {
            shuffleArray(shuffledList, 4);
            int iter;
            for (iter = 0; iter < 4 && currentMiss < missLimit; iter++) {
                //                wait_ms(1000);
                int index = shuffledList[iter];
                switch (taskType) {

                    case DNMS_TASK:
                        firstOdor = (index == 0 || index == 2) ? stim1 : stim2;
                        secondOdor = (index == 1 || index == 2) ? stim1 : stim2;
                        break;
                    case SHAPPING_TASK:
                        firstOdor = (index == 0 || index == 2) ? stim1 : stim2;
                        secondOdor = (firstOdor == stim1) ? stim2 : stim1;
                        break;
                    case GONOGO_TASK:
                    case GONOGO_LR_TASK:
                        firstOdor = (index == 0 || index == 2) ? stim1 : stim2;
                        secondOdor = 0;
                        break;
                    case DNMS_2AFC_TASK:
                    case DNMS_2AFC_TEACH:
                        if (hit > lastHit || currentTrial == 0) {
                            firstOdor = (index == 0 || index == 2) ? stim1 : stim2;
                            secondOdor = (index == 1 || index == 2) ? stim1 : stim2;
                            lastOdor1 = firstOdor;
                            lastOdor2 = secondOdor;
                        } else {
                            firstOdor = lastOdor1;
                            secondOdor = lastOdor2;
                        }

                        lastHit = hit;
                        break;
                    case GONOGO_2AFC_TEACH:
                        if (hit > lastHit || currentTrial == 0) {
                            firstOdor = (index == 0 || index == 2) ? stim1 : stim2;
                            lastOdor1 = firstOdor;
                            secondOdor = 0;
                        } else {
                            firstOdor = lastOdor1;
                            secondOdor = 0;
                        }

                        lastHit = hit;
                        break;
                    case NO_ODOR_CATCH_TRIAL_TASK:
                        firstOdor = (index == 0 || index == 2) ? stim1 : stim2;
                        secondOdor = (index == 1 || index == 2) ? stim1 : stim2;
                        if ((currentTrial > 3 && currentTrial < 8 && firstOdor == stim1 && secondOdor == stim1)
                                || (currentTrial > 7 && currentTrial < 12 && firstOdor == stim1 && secondOdor == stim2)
                                || (currentTrial > 11 && currentTrial < 16 && firstOdor == stim2 && secondOdor == stim1)
                                || (currentTrial > 15 && currentTrial < 20 && firstOdor == stim2 && secondOdor == stim2)) {
                            firstOdor -= 3;
                        } else {
                        }
                        break;

                    case VARY_ODOR_LENGTH_TASK:
                    {
                        static int varyLengths[] = {250, 500, 750, 1000};
                        firstOdor = (index == 0 || index == 2) ? stim1 : stim2;
                        secondOdor = (index == 1 || index == 2) ? stim1 : stim2;
                        unsigned int idxO1 = shuffledLongList[currentTrial]&0x03;
                        unsigned int idxO2 = shuffledLongList[currentTrial] >> 2;
                        stims.stim1Length = varyLengths[idxO1];
                        stims.stim2Length = varyLengths[idxO2];
                        //                        lcdWriteNumber(idxO1,2,13,2);
                        //                        lcdWriteNumber(idxO2,2,15,2);
                        //                        lcdWriteNumber(odors.odor1Length,4,1,2);
                        //                        lcdWriteNumber(odors.odor2Length,4,5,2);

                        break;
                    }
                    case OPTO_DPAL_TASK:
                        firstOdor = (index == 0 || index == 2) ? 9 : 10;
                        secondOdor = (index == 1 || index == 2) ? stim1 : stim2;
                        break;

                    case OPTO_DPAL_SHAPING_TASK:
                        firstOdor = (index == 0 || index == 2) ? 9 : 10;
                        secondOdor = (firstOdor == 9) ? stim2 : stim1;
                        break;
                }

                lcdWriteChar(odorTypes[firstOdor], 1, 1);
                lcdWriteChar(odorTypes[secondOdor], 2, 1);

                int laserCurrentTrial;

                switch (laserTrialType) {
                    case LASER_NO_TRIAL:
                        laserCurrentTrial = 0;
                        break;
                    case LASER_EVERY_TRIAL:
                        laserCurrentTrial = 1;
                        break;
                    case LASER_OTHER_TRIAL:
                        laserCurrentTrial = currentTrial % 2;
                        break;

                    case LASER_LR_EACH_QUARTER:
                        laser.side = isLikeOdorA(firstOdor) ? 1 : 2;
                    case LASER_EACH_QUARTER:
                        switch (currentTrial % 5) {
                            case 0:
                                laserCurrentTrial = 0;
                                break;
                            case 1:
                                laserType = laserDuring1Quarter;
                                laserCurrentTrial = 1;
                                break;
                            case 2:
                                laserType = laserDuring2Quarter;
                                laserCurrentTrial = 1;
                                break;
                            case 3:
                                laserType = laserDuring3Quarter;
                                laserCurrentTrial = 1;
                                break;
                            case 4:
                                laserType = laserDuring4Quarter;
                                laserCurrentTrial = 1;
                                break;
                        }
                        break;


                    case LASER_12s_LR_EACH_QUARTER:
                        laser.side = isLikeOdorA(firstOdor) ? 1 : 2;
                    case LASER_12s_EACH_QUARTER:
                        switch (currentTrial % 5) {
                            case 0:
                                laserCurrentTrial = 0;
                                break;
                            case 1:
                                laserType = laserDuring12s1Quarter;
                                laserCurrentTrial = 1;
                                break;
                            case 2:
                                laserType = laserDuring12s2Quarter;
                                laserCurrentTrial = 1;
                                break;
                            case 3:
                                laserType = laserDuring12s3Quarter;
                                laserCurrentTrial = 1;
                                break;
                            case 4:
                                laserType = laserDuring12s4Quarter;
                                laserCurrentTrial = 1;
                                break;
                        }
                        break;


                    case LASER_VARY_LENGTH:
                        switch (currentTrial % 5) {
                            case 0:
                                laserCurrentTrial = 0;
                                break;
                            case 1:
                                laserType = laser4sRamp;
                                laserCurrentTrial = 1;
                                break;
                            case 2:
                                laserType = laser2sRamp;
                                laserCurrentTrial = 1;
                                break;
                            case 3:
                                laserType = laser1sRamp;
                                laserCurrentTrial = 1;
                                break;
                            case 4:
                                laserType = laser_5sRamp;
                                laserCurrentTrial = 1;
                                break;
                        }
                        break;


                    case LASER_LR_EVERYTRIAL:
                        laser.side = isLikeOdorA(firstOdor) ? 1 : 2;
                        laserCurrentTrial = 1;
                        break;

                    case LASER_LR_EVERY_OTHER_TRIAL:
                        laser.side = isLikeOdorA(firstOdor) ? 1 : 2;
                        laserCurrentTrial = currentTrial % 2;
                        break;

                    case LASER_INCONGRUENT_CATCH_TRIAL:
                        if ((currentTrial > 3 && currentTrial < 8 && isLikeOdorA(firstOdor) && isLikeOdorA(secondOdor))
                                || (currentTrial > 7 && currentTrial < 12 && isLikeOdorA(firstOdor)&& !isLikeOdorA(secondOdor))
                                || (currentTrial > 11 && currentTrial < 16 && !isLikeOdorA(firstOdor) && isLikeOdorA(secondOdor))
                                || (currentTrial > 15 && currentTrial < 20 && !isLikeOdorA(firstOdor) && !isLikeOdorA(secondOdor))) {
                            laser.side = isLikeOdorA(firstOdor) ? 2 : 1;
                        } else {
                            laser.side = isLikeOdorA(firstOdor) ? 1 : 2;
                        }
                        laserCurrentTrial = 1;
                        break;
                }
                zxLaserTrial(laserType, firstOdor, stims, delay, secondOdor, WaterLen, ITI, delay_before_reward, laserCurrentTrial);
                currentTrial++;
            }
        }
        protectedSerialSend(SpSess, 0);

    }
    protectedSerialSend(SpTrain, 0); // send it's the end
    u2Received = -1;
}

//static void optoStim(int stim, int length, int place) {
//    int stimSend = likeOdorA(stim) ? 9 : 10;
//    protectedSerialSend(stimSend, stim);
//    lcdWriteChar(place == 1 ? '1' : '2', 4, 1);
//    switch (stim) {
//        case 21:
//        {
//            int timePassed = 0;
//            while (timePassed < length) {
//                Out3 = 1;
//                wait_ms(85); //Uchida N.NS 2013
//                Out3 = 0;
//                wait_ms(85);
//                Out3 = 1;
//                wait_ms(85); //Uchida N.NS 2013
//                Out3 = 0;
//                wait_ms(245);
//                timePassed += 500;
//            }
//            break;
//        }
//        case 22:
//        {
//            int timePassed = 0;
//            while (timePassed < length) {
//                Out3 = 1;
//                wait_ms(85); //Uchida N.NS 2013
//                Out3 = 0;
//                wait_ms(165);
//                timePassed += 250;
//            }
//            break;
//        }
//        case 23:
//        {
//            int timePassed = 0;
//            while (timePassed < length) {
//                Out3 = 1;
//                wait_ms(170); //Uchida N.NS 2013
//                Out3 = 0;
//                wait_ms(330);
//                timePassed += 500;
//            }
//            break;
//        }
//    }
//    protectedSerialSend(stimSend, 0);
//    lcdWriteChar(place == 1 ? 'd' : 'D', 4, 1);
//}

static void stim(int place, int stim, int type, int laserOnTrial) {
    switch (stim) {
        case 0:
        case 2:
        case 3:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
            assertLaser(type, place == 1 ? atFirstOdorBeginning : atSecondOdorBeginning, laserOnTrial);
            Valve_ON(stim, pwm.fullDuty);
            int stimSend;
            if (isLikeOdorA(stim)) {
                stimSend = 9;
                Out2 = 1;
            } else {
                stimSend = 10;
                Out3 = 1;
            }
            protectedSerialSend(stimSend, stim);
            lcdWriteChar(place == 1 ? '1' : '2', 4, 1);
            waitTimer32(place == 1 ? stims.stim1Length : stims.stim2Length);
            Valve_OFF(stim);
            Out2 = 0;
            Nop();
            Nop();
            Out3 = 0;
            Nop();
            Nop();
            assertLaser(type, place == 1 ? atFirstOdorEnd : atSecondOdorEnd, laserOnTrial);
            protectedSerialSend(stimSend, 0);
            lcdWriteChar(place == 1 ? 'd' : 'D', 4, 1);
            waitTimer32(1000 - (place == 1 ? stims.stim1Length : stims.stim2Length));
            break;
            //        case 20:
            //        case 21:
            //        case 22:
            //        case 23:
            //            optoStim(stim, place == 1 ? stims.stim1Length : stims.stim2Length, place);
            //            break;


    }
}

static void zxLaserTrial(int type, int firstOdor, STIM_T odors, _delayT interOdorDelay, int secondOdor, float waterPeroid, int ITI, float delay_before_reward, int laserOnTrial) {
    resetTimerCounter32();
    protectedSerialSend(Sptrialtype, type);
    protectedSerialSend(Splaser, laserOnTrial);
    assertLaser(type, fourSecBeforeFirstOdor, laserOnTrial);
    waitTimer32(1000);
    assertLaser(type, threeSecBeforeFirstOdor, laserOnTrial);
    waitTimer32(2000);
    assertLaser(type, oneSecBeforeFirstOdor, laserOnTrial);
    waitTimer32(800);
    assertLaser(type, at200mSBeforeFirstOdor, laserOnTrial);
    waitTimer32(200);

    /////////////////////////////////////////////////
    stim(1, firstOdor, type, laserOnTrial);
    ////////////////////////////////////////////////

    if (taskType == GONOGO_LR_TASK || taskType == GONOGO_TASK || taskType == GONOGO_2AFC_TEACH) {
        //Do nothing during Go Nogo Tasks
        assertLaser(type, atSecondOdorEnd, laserOnTrial);
    } else {
        if (interOdorDelay == 0) {
            waitTimer32(200);
        } else {
            ///////////-inter odor interval-/////////////////
            assertLaser(type, atDelayBegin, laserOnTrial);
            waitTimer32(500);
            assertLaser(type, atDelay_5SecIn, laserOnTrial);
            waitTimer32(500);
            assertLaser(type, atDelay1SecIn, laserOnTrial);

            if (interOdorDelay < 4) {
                waitTimer32(interOdorDelay * 1000 - 2000);
            } else {
                //TODO INSERT DISTRACTOR HERE
                if (type == laserDelayDistractor) {
                    distractor(2);
                } else {
                    waitTimer32(500);
                    assertLaser(type, atDelay1_5SecIn, laserOnTrial);
                    waitTimer32(500);
                }
                assertLaser(type, atDelay2SecIn, laserOnTrial);

                if (interOdorDelay == 12) {
                    waitTimer32(500);
                    assertLaser(type, atDelay2_5SecIn, laserOnTrial);
                    waitTimer32(500);
                    assertLaser(type, atDelay3SecIn, laserOnTrial);
                    waitTimer32(2500);
                } else {
                    waitTimer32(interOdorDelay * 500 - 2500);
                }

                assertLaser(type, atDelay_5ToMiddle, laserOnTrial);
                waitTimer32(500);
                assertLaser(type, atDelayMiddle, laserOnTrial);
                if (interOdorDelay == 12) {
                    waitTimer32(2500);
                    assertLaser(type, atDelay8_5SecIn, laserOnTrial);
                    waitTimer32(500);
                    assertLaser(type, atDelay9SecIn, laserOnTrial);
                    waitTimer32(500);
                } else {
                    waitTimer32(interOdorDelay * 500 - 2500);
                }
                assertLaser(type, atDelayLast2_5SecBegin, laserOnTrial);
                waitTimer32(500);
                assertLaser(type, atDelayLast2SecBegin, laserOnTrial);
                waitTimer32(500);
                assertLaser(type, atDelayLast1_5SecBegin, laserOnTrial);
                waitTimer32(500);
            }
            assertLaser(type, atDelayLastSecBegin, laserOnTrial);
            waitTimer32(500);
            assertLaser(type, atDelayLast500mSBegin, laserOnTrial);
            waitTimer32(300);
            assertLaser(type, atDelayLast200mSBegin, laserOnTrial);
            waitTimer32(200);
        }

        ///////////-Second odor-/////////////////
        stim(2, secondOdor, type, laserOnTrial);
        //////////////////////////////////////////
    }
    waitTimer32(delay_before_reward * 1000);
    assertLaser(type, atRewardBeginning, laserOnTrial);
    lcdWriteChar('R', 4, 1);

    //Assess Performance here
    waterNResult(firstOdor, secondOdor, waterPeroid);

    waitTimer32(550); //water time sync

    // Total Trials
    int totalTrials = hit + correctRejection + miss + falseAlarm;
    lcdWriteNumber(totalTrials, 3, 14, 2);
    // Discrimination rate
    if (hit + correctRejection > 0) {

        correctRatio = 100 * (hit + correctRejection) / totalTrials;
        lcdWriteNumber(correctRatio, 2, 14, 1);
    }
    lcdWriteChar('I', 4, 1);
    ///--ITI1---///
    assertLaser(type, atITIBeginning, laserOnTrial);
    waitTimer32(1000);
    assertLaser(type, atITIOneSecIn, laserOnTrial);
    waitTimer32(ITI * 1000 - 5000); //another 4000 is at the beginning of the trials.
    protectedSerialSend(SpITI, 0);
    waitTrial();
}

static void delaymSecKey(unsigned int N, unsigned int type) {
    timerCounterI = 0;
    while (timerCounterI < N) {
        Key_Event();
        if (hardwareKey == 1 || u2Received > 0) {
            switch (type) {
                case 3:
                    laser.on = ((laser.on + 1) % 2)*3;
                    hardwareKey = 0;
                    u2Received = -1;
                    break;
                case 1:
                case 2:
                    ;
                    unsigned char * hi = type == 1 ? &pwm.L_Hi : &pwm.R_Hi;
                    unsigned char * lo = type == 1 ? &pwm.L_Lo : &pwm.R_Lo;
                    int key = 0;
                    if (u2Received > 0) {
                        key = u2Received - 0x30;
                        u2Received = -1;
                    } else {
                        Key_Event();
                        if (hardwareKey == 1) {
                            hardwareKey = 0;
                            key = key_val;
                        }
                    }
                    switch (key) {
                        case 1:
                            *lo -= 10;
                            break;
                        case 2:
                            *lo -= 2;
                            break;
                        case 3:
                            *lo += 2;
                            break;
                        case 4:
                            *lo += 10;
                            break;
                        case 5:
                            write_eeprom(type == 1 ? EEP_DUTY_LOW_L_OFFSET : EEP_DUTY_LOW_R_OFFSET, *lo);
                            break;
                        case 6:
                            *hi -= 10;
                            break;
                        case 7:
                            *hi -= 2;
                            break;
                        case 8:
                            *hi += 2;
                            break;
                        case 9:
                            *hi += 10;
                            break;
                        case 0:
                            write_eeprom(type == 1 ? EEP_DUTY_HIGH_L_OFFSET : EEP_DUTY_HIGH_R_OFFSET, *hi);
                            break;
                    }
                    if (*lo == 85 || *lo == 170)*lo -= 1;
                    if (*hi == 85 || *hi == 170)*hi += 1;

                    protectedSerialSend(type == 1 ? SpWater_bitter : SpValve8, *lo);
                    protectedSerialSend(type == 1 ? SpWater_bitter : SpValve8, *hi);
                    break;
            }
        }
        int tt = timerCounterI;
        while (timerCounterI < tt + 200) {
            Nop();
        }
    }
}

void odorDepeltion(int totalTrial, int p3) {
    initZXTMR();
    protectedSerialSend(1, 0xff);
    PDC2 = pwm.fullDuty;
    PDC4 = pwm.fullDuty;
    int i;
    for (i = 0; i < totalTrial; i++) {
        home_clr();
        lcdWriteString("Odor Depletion");
        int percent = i > 599 ? 999 : i * 100 / 60;
        lcdWriteNumber(percent, 3, 1, 2);
        lcdWriteString("%, Valve ");

        LCD_set_xy(13, 2);
        lcdWriteString("2|5");
        Valve_ON(5, pwm.fullDuty);
        Nop();
        Nop();
        Valve_ON(2, pwm.fullDuty);
        delaymSecKey(15000, 3);
        Valve_OFF(5);
        Nop();
        Nop();
        Valve_OFF(2);
        LCD_set_xy(13, 2);
        lcdWriteString("3|6");
        Valve_ON(6, pwm.fullDuty);
        if (p3) {
            Nop();
            Nop();
            Valve_ON(3, pwm.fullDuty);
        }
        delaymSecKey(15000, 3);
        Valve_OFF(6);
        Nop();
        Nop();
        Valve_OFF(3);
    }
}

static void rampTweak(int L, int itiIndex) {
    int laserLength[] = {3500, 3500, 6500, 10500, 1500, 2500};
    int delay = laserLength[getFuncNumber(1, "5 8 12 1.5 2.5s")];
    int iti = itiIndex ? (delay + 1500)*2 : 2500;

    laser.ramp = 500;
    splash("Ramping", "");
    lcdWriteChar(L ? 'L' : 'R', 9, 1);

    for (;;) {
        unsigned char Hi = L ? pwm.L_Hi : pwm.R_Hi;
        unsigned char Lo = L ? pwm.L_Lo : pwm.R_Lo;
        lcdWriteNumber(Lo, 3, 8, 2);
        lcdWriteNumber(Hi, 3, 13, 2);


        delaymSecKey(iti, L ? 1 : 2);
        turnOnLaser(L ? 1 : 2);
        delaymSecKey(delay, L ? 1 : 2);
        turnOffLaser();
    }
}
//
//void feedWater(int topTime) {
//    initZXTMR();
//    int times[] = {600, 2, 5, 10, 30, 600};
//    topTime = times [topTime];
//    splash("Auto Feed Water", "Wait for lick...");
//    while (!lick.licking) {
//    }
//    int go = 1;
//    while (go) {
//        splash("Feeding", "   s left.");
//        Valve_ON(water_sweet, fullduty);
//        int secondsWithoutLick = 0;
//        int licked = 0;
//        while (secondsWithoutLick < topTime) {
//            lcdWriteNumber(topTime - secondsWithoutLick, 3, 1, 2);
//            timerCounterI = 0;
//            while (timerCounterI < 1000) {
//                if (_LICK_ANY) {
//
//                    licked = 1;
//                }
//            }
//            secondsWithoutLick = licked ? 0 : secondsWithoutLick + 1;
//            licked = 0;
//        }
//        Valve_OFF(water_sweet);
//        go = (getFuncNumber(1, "No Lick. Again=1") == 1);
//    }
//}

static void feedWaterLR(float waterLength) {
    taskType = GONOGO_LR_TASK;
    int lastLocation = 0;
    lick.LCount = 0;
    lick.RCount = 0;
    unsigned int waterCount = 0;
    unsigned int lastL = 0;
    unsigned int lastR = 0;
    splash("L       R", "W     FL   FR");
    PDC1 = pwm.fullDuty;
    PDC2 = pwm.fullDuty;
    timerCounterI = 2000;

    while (1) {
        if (lick.LCount > lastL) {
            if (timerCounterI > 2000 && lastLocation != LICKING_LEFT) {
                protectedSerialSend(22, 1);
                PORTGbits.RG0 = 1;
                lastLocation = LICKING_LEFT;
                timerCounterI = 0;
                lcdWriteChar('L', 16, 1);
                lcdWriteNumber(++waterCount, 4, 2, 2);
            }
            lcdWriteNumber(lick.LCount, 4, 3, 1);
            lastL = lick.LCount;
        } else if (lick.RCount > lastR) {
            if (timerCounterI > 2000 && lastLocation != LICKING_RIGHT) {
                protectedSerialSend(22, 2);
                PORTGbits.RG1 = 1;
                lastLocation = LICKING_RIGHT;
                timerCounterI = 0;
                lcdWriteChar('R', 16, 1);
                lcdWriteNumber(++waterCount, 4, 2, 2);
            }
            lcdWriteNumber(lick.RCount, 4, 11, 1);
            lastR = lick.RCount;
        }

        if (timerCounterI > waterLength) {
            PORTGbits.RG0 = 0;
            Nop();
            Nop();
            PORTGbits.RG1 = 0;
            Nop();
            Nop();
        }
    }
}

static void feedWaterFast(int waterLength) {

    lick.LCount = 0;
    lick.RCount = 0;
    unsigned int waterCount = 0;
    unsigned int totalLickCount = 0;
    splash("Total Lick", "");

    timerCounterI = 1000;
    while (1) {
        if (lick.LCount + lick.RCount > totalLickCount) {
            if (timerCounterI >= 500) {
                Valve_ON(water_sweet, pwm.fullDuty);
                timerCounterI = 0;
                lcdWriteNumber(++waterCount, 4, 13, 2);
            }
            totalLickCount = lick.LCount + lick.RCount;
            lcdWriteNumber(totalLickCount, 4, 12, 1);
        }
        if (timerCounterI >= waterLength) {
            Valve_OFF(water_sweet);
        }
    }
}

void splash(char s1[], char s2[]) {

    home_clr();
    lcdWriteString(s1);
    line_2();
    lcdWriteString(s2);
    wait_ms(1000);
}

void wait_ms(int time) {
#ifndef DEBUG    
    timerCounterI = 0;
    while (time > 0 && timerCounterI < time) {
    }
#endif
}

static void variableVoltage() {
    laser.on = 1;
    char VString[] = {'1', '2', '7', ' ', ' ', ' ', '-', '-', ' ', '-', ' ', '+', ' ', '+', '+', ' '};
    int tmpFreq;
    tmpFreq = pwm.R_Lo;

    while (1) {
        tmpFreq = (tmpFreq > pwm.fullDuty) ? pwm.fullDuty : tmpFreq;
        tmpFreq = (tmpFreq < 0) ? 0 : tmpFreq;
        pwm.R_Lo = tmpFreq;
        protectedSerialSend(SpValve8, pwm.R_Lo);
        VString[0] = tmpFreq / 100 + 0x30;
        VString[1] = ((tmpFreq % 100) / 10) + 0x30;
        VString[2] = (tmpFreq % 10) + 0x30;
        int n = getFuncNumber(1, VString);
        switch (n) {
            case 1:
                tmpFreq -= 10;
                break;
            case 2:
                tmpFreq -= 2;
                break;
            case 3:
                tmpFreq += 2;
                break;
            case 4:
                tmpFreq += 10;
                break;
            case 5:
                ;
                int m = getFuncNumber(1, "Low Hi");
                write_eeprom(m == 1 ? EEP_DUTY_LOW_L_OFFSET : EEP_DUTY_HIGH_L_OFFSET, tmpFreq);
        }

    }
}

static void laserTrain() {
    unsigned int freqs[] = {1, 5, 10, 20, 50, 100};
    laser.onTime = 5;
    unsigned int idx = 0;
    for (idx = 0; idx < 6; idx++) {
        unsigned int duration = 1000 / freqs[idx];
        laser.offTime = duration - laser.onTime;
        turnOnLaser(3);
        wait_ms(duration * 10 - 1);
        turnOffLaser();
        wait_ms(2000);
    }
}

//void shiftingLaser(void) {
//    while (1) {
//
//        Out1 = 1;
//        Nop();
//        Nop();
//        Out2 = 1;
//        Nop();
//        Nop();
//        Out3 = 1;
//        Nop();
//        Nop();
//        Out4 = 1;
//        Nop();
//        Nop();
//        Out5 = 1;
//        Nop();
//        Nop();
//        Out6 = 1;
//        wait_ms(3000);
//        Out1 = 0;
//        Nop();
//        Nop();
//        Out2 = 0;
//        Nop();
//        Nop();
//        Out3 = 0;
//        Nop();
//        Nop();
//        Out4 = 0;
//        Nop();
//        Nop();
//        Out5 = 0;
//        Nop();
//        Nop();
//        Out6 = 0;
//        wait_ms(20000);
//    }
//}

void testValve(void) {
    int v = getFuncNumber(1, "Valve No?");
    while (1) {

        Valve_ON(v, pwm.fullDuty);
        wait_ms(1000);
        Valve_OFF(v);
        wait_ms(1000);
    }
}

static void testVolume(int waterLength) {
    int n = getFuncNumber(1, "Valve #");
    int i;
    for (i = 0; i < 100; i++) {

        Valve_ON(n, pwm.fullDuty);
        wait_ms(waterLength);
        Valve_OFF(n);
        wait_ms(500 - waterLength);
    }
}

static void test_Laser(void) {
    int i = 1;
    while (1) {

        laser.on = i;
        Out3 = i;
        getFuncNumber(1, "Toggle Laser");
        i = (i + 1) % 2;

    }
}

static void flashLaser(void) {
    splash("Test Laser", "");
    int count = 0;
    while (1) {
        lcdWriteNumber(++count, 3, 10, 2);
        resetTimerCounter32();
        turnOnLaser(3);
        waitTimer32(2000);
        turnOffLaser();
        waitTimer32(5000);
    }
}

/****************************************
 *for calibration between counter and time
 *****************************************
void correctTime() {
    int cy = 0;
    for (; cy < 10; cy++) {
        unsigned int i;
        Out2 = 1;
        for (i = 0; i < 64000; i++);
        Out2 = 0;
        for (i = 0; i < 64000; i++);
    }
}
 ****************************************/

static void stepLaser() {
    int laserOn = 3000;
    int laserOff = 16000;
    int step;
    int sCounter = 0;
    PORTCbits.RC1 = 1;
    pwm.R_Lo = 0xfe;
    for (; sCounter < 20; sCounter++) {
        int powerx10 = getFuncNumber(2, "Power x10");
        splash("     Power", "Trial");
        lcdWriteNumber(powerx10, 2, 12, 1);

        step = 20;
        for (; step > 0; step--) {
            lcdWriteNumber(step, 2, 7, 2);
            wait_ms(laserOff);
            int tag = powerx10 * 10;
            Out2 = 1;
            int timer;
            for (timer = 0; timer < 1000; timer++);
            Out2 = 0;
            for (; tag > 0; tag--) {
                for (timer = 0; timer < 1000; timer++);
            }
            Out2 = 1;
            for (timer = 0; timer < 1000; timer++);
            Out2 = 0;
            turnOnLaser(3);
            wait_ms(laserOn);
            turnOffLaser();
        }
    }
}

void callFunction(int n) {
    currentMiss = 0;
    pwm.L_Lo = read_eeprom(EEP_DUTY_LOW_L_OFFSET);
    pwm.L_Hi = read_eeprom(EEP_DUTY_HIGH_L_OFFSET);
    pwm.R_Lo = read_eeprom(EEP_DUTY_LOW_R_OFFSET);
    pwm.R_Hi = read_eeprom(EEP_DUTY_HIGH_R_OFFSET);


    PORTCbits.RC1 = 1;
    initZXTMR();
    switch (n) {
            //            int m;
            //ZX's functions

        case 4301:
            taskType = NO_ODOR_CATCH_TRIAL_TASK;
            laser.ramp = 500;
            splash("No Odor Catch", "");

            laserTrialType = LASER_LR_EVERYTRIAL;
            zxLaserSessions(5, 6, laserRampDuringDelay, 2, 5, 20, 0.05, 20, 15, 1.0);
            break;

        case 4302:
        {
            splash("VARY ODOR LEN", "");
            taskType = VARY_ODOR_LENGTH_TASK;
            laserTrialType = LASER_NO_TRIAL;
            int delay = setDelay(1);
            highLevelShuffleLength = 16;
            int type = setType();
            zxLaserSessions(type, type + 1, laserDuringDelayChR2, delay, delay * 2, 16, 0.05, 20, setSessionNum(), 1.0);
            break;
        }
        case 4303:
            splash("Each Quarter", "Delay LR Laser");
            laserTrialType = LASER_LR_EACH_QUARTER;
            laser.ramp = 500;
            zxLaserSessions(5, 6, laserDuringDelay, 12, 24, 20, 0.05, 20, setSessionNum(), 1.0);
            break;
        case 4304:
        {
            splash("OB Opto Stim ", "Shaping");
            laserTrialType = LASER_NO_TRIAL;
            taskType = SHAPPING_TASK;
            int type = setType();
            zxLaserSessions(type, type + 1, laserDuringDelay, 4, 8, 20, 0.05, 50, setSessionNum(), 1.0);
            break;
        }
        case 4305:
        {
            splash("OB Opto Stim", "DNMS");
            taskType = DNMS_TASK;
            laserTrialType = LASER_NO_TRIAL;
            int type = setType();
            zxLaserSessions(type, type + 1, laserDuringDelayChR2, 5, 10, 20, 0.05, 20, setSessionNum(), 1.0);
            break;
        }
        case 4306:
        {
            splash("OB Opto Stim", "Go No-Go");
            //            zxGoNogoSessions(type,type+1,3, 20, 1, 0.5, 4);
            laserTrialType = LASER_NO_TRIAL;
            taskType = GONOGO_TASK;
            int type = setType();
            zxLaserSessions(type, type + 1, laserDuring3Baseline, 0, 5, 20, 0.05, 20, setSessionNum(), 1.0);
            break;
        }
        case 4307:
        {
            splash("OB Opto Stim", "");
            //            zxGoNogoSessions(type,type+1,3, 20, 1, 0.5, 4);
            laserTrialType = LASER_LR_EVERYTRIAL;
            taskType = OPTO_DPAL_TASK;
            int type = setType();
            zxLaserSessions(type, type + 1, laserDuring1stOdor, 4, 8, 20, 0.05, 30, setSessionNum(), 1.0);
            break;
        }

        case 4308:
        {
            splash("OB Opto Stim", "Shaping");
            //            zxGoNogoSessions(type,type+1,3, 20, 1, 0.5, 4);
            laserTrialType = LASER_LR_EVERYTRIAL;
            taskType = OPTO_DPAL_SHAPING_TASK;
            int type = setType();
            zxLaserSessions(type, type + 1, laserDuring1stOdor, 4, 8, 20, 0.05, 30, setSessionNum(), 1.0);
            break;
        }
        case 4310:
        {
            int m = getFuncNumber(2, "Time in ms");
            testVolume(m);
            break;
        }
        case 4311:
            testVolume(50);
            break;

        case 4312:
        {
            splash("Response Delay", "Laser Control");
            laserTrialType = LASER_OTHER_TRIAL;
            int type = setType();
            zxLaserSessions(type, type + 1, laserDuringResponseDelay, 5, 10, 20, 0.05, 20, 20, 1.0);
            break;
        }
        case 4313:
        {
            splash("GoNogo Control", "For DNMS");

            laserTrialType = LASER_OTHER_TRIAL;
            taskType = GONOGO_TASK;
            int type = setType();
            zxLaserSessions(type, type + 1, laserDuring3Baseline, 0, 5, 20, 0.05, 20, 20, 1.0);
            break;
        }
        case 4314:
        {
            splash("Vary Length", "Delay Laser");
            laserTrialType = LASER_VARY_LENGTH;
            laser.ramp = 500;
            int type = setType();
            zxLaserSessions(type, type + 1, laserDuringDelay, 5, 10, 20, 0.05, 15, 25, 1.0);
            laser.ramp = 0;
            break;
        }
        case 4315:
            splash("Each Quarter", "Delay Laser");
            laserTrialType = LASER_EACH_QUARTER;
            laser.ramp = 500;
            zxLaserSessions(5, 6, laserDuringDelay, 8, 16, 20, 0.05, 20, 15, 1.0);
            laser.ramp = 0;
            break;

        case 4316:
            stepLaser();
            break;

        case 4317:
            taskType = NO_ODOR_CATCH_TRIAL_TASK;
            laser.ramp = 500;
            splash("No Odor Catch", "");

            laserTrialType = LASER_LR_EVERYTRIAL;
            zxLaserSessions(5, 6, laserRampDuringDelay, 12, 24, 20, 0.05, 20, 15, 1.0);
            break;



            //        case 4317:
            //            correctTime();
            //            break;

        case 4318:
            splash("No Trial Wait", "LR Laser");
            wait_Trial = 0;
            break;

        case 4319:
        {
            taskType = DNMS_TASK;
            laser.ramp = 500;
            splash("LR LASER DNMS", "Sufficiency");
            int delay = setDelay(1);
            laserTrialType = LASER_LR_EVERYTRIAL;
            zxLaserSessions(5, 6, laserRampDuringDelay, delay, delay * 2, 20, 0.05, 20, setSessionNum(), 1.0);
            break;
        }

        case 4320:
            splash("12s Each Quarter", "Delay LR Laser");
            laserTrialType = LASER_12s_LR_EACH_QUARTER;
            laser.ramp = 500;
            zxLaserSessions(5, 6, laserDuringDelay, 12, 24, 20, 0.05, 25, 15, 1.0);
            laser.ramp = 0;
            break;
        case 4321:
            //            setLaser();
            //            taskType = _DNMS_TASK;
            //            splash("NoDelay Control", "For DNMS");
            //            laserTrialType = _LASER_OTHER_TRIAL;
            //            m = getFuncNumber(1, "Short Long B+R");
            //            switch (m) {
            //                case 1: m = laserNoDelayControlShort;
            //                    break;
            //                case 2: m = laserNoDelayControl;
            //                    break;
            //                case 3: m = laserDuringBaseAndResponse;
            //                    break;
            //            }
            //            zxLaserSessions(type,type+1,m, 0.2, 5, 20, 0.05, 20, 20, 1.0);
            break;
        case 4322:
            variableVoltage();
            break;

        case 4323:
            splash("DNMS LR TASK", "Ramp Laser ++");
            protectedSerialSend(PERM_INFO, DMS_LR_Teach_LON);
            taskType = DNMS_2AFC_TEACH;
            laserTrialType = LASER_EVERY_TRIAL;
            laser.ramp = 500;
            zxLaserSessions(5, 6, laserRampDuringDelay, 5, 10, 20, 0.05, 30, 20, 1.0);
            break;

        case 4324:
            splash("DNMS LR TASK", "Ramp Laser +-");
            protectedSerialSend(PERM_INFO, DMS_LR_Teach_ONOFF);
            taskType = DNMS_2AFC_TEACH;
            laserTrialType = LASER_OTHER_TRIAL;
            laser.ramp = 500;
            zxLaserSessions(5, 6, laserRampDuringDelay, 5, 10, 20, 0.05, 30, 20, 1.0);
            break;

        case 4325:
            splash("BaseLine RQ", "Control");
            laserTrialType = LASER_OTHER_TRIAL;
            zxLaserSessions(5, 6, laserDuring3Baseline, 5, 10, 20, 0.05, 20, 15, 1.0);
            break;

        case 4326:
            taskType = DNMS_TASK;
            laser.ramp = 500;
            splash("Incongrument", "LR Laser");
            laserTrialType = LASER_INCONGRUENT_CATCH_TRIAL;
            zxLaserSessions(5, 6, laserRampDuringDelay, 12, 24, 20, 0.05, 20, 15, 1.0);
            break;

        case 4327:
            splash("DNMS LR TEACH", "No Laser");
            protectedSerialSend(PERM_INFO, DMS_LR_Teach_LOFF);
            taskType = DNMS_2AFC_TEACH;
            laserTrialType = LASER_NO_TRIAL;
            zxLaserSessions(5, 6, laserRampDuringDelay, 5, 10, 20, 0.05, 30, 20, 1.0);
            break;

        case 4328:
            splash("MW/OD LR TEACH", "No Laser");
            protectedSerialSend(PERM_INFO, MSWOD_LR_Teach_LOFF);
            taskType = DNMS_2AFC_TEACH;
            laserTrialType = LASER_NO_TRIAL;
            zxLaserSessions(5, 6, laserRampDuringDelay, 0, 10, 20, 0.05, 30, 20, 1.0);
            break;

        case 4329:
            splash("LR DNMS 5s Delay", "Laser + -");
            protectedSerialSend(PERM_INFO, DMS_LR_5Delay_Laser);
            taskType = DNMS_2AFC_TEACH;
            laserTrialType = LASER_OTHER_TRIAL;
            laser.ramp = 500;
            zxLaserSessions(5, 6, laserRampDuringDelay, 5, 10, 20, 0.05, 30, 20, 1.0);
            break;

        case 4330:
            splash("LR DNMS 8s Delay", "Laser + -");
            protectedSerialSend(PERM_INFO, DMS_LR_8Delay_Laser);
            taskType = DNMS_2AFC_TEACH;
            laserTrialType = LASER_OTHER_TRIAL;
            laser.ramp = 500;
            zxLaserSessions(5, 6, laserRampDuringDelay, 8, 16, 20, 0.05, 30, 20, 1.0);
            break;


        case 4331:
        {
            splash("Delay+Odor2", "Control");
            laserTrialType = LASER_OTHER_TRIAL;
            int type = setType();
            zxLaserSessions(type, type + 1, laserDuringDelay_Odor2, 5, 10, 20, 0.05, 20, 15, 1.0);
            break;
        }
        case 4332:
            splash("Odor", "Control");
            laserTrialType = LASER_OTHER_TRIAL;
            int m = getFuncNumber(1, "1st 2nd BothOdor");
            int type = setType();
            zxLaserSessions(type, type + 1, m == 1 ? laserDuring1stOdor : m == 2 ? laserDuring2ndOdor : laserDuringOdor, 5, 10, 20, 0.05, 50, 30, 1.0);
            break;
        case 4333:
            test_Laser();
            break;

        case 4334:
            splash("LR DNMS 12s Delay", "Laser + -");
            protectedSerialSend(PERM_INFO, DMS_LR_12Delay_Laser);
            taskType = DNMS_2AFC_TEACH;
            laserTrialType = LASER_OTHER_TRIAL;
            laser.ramp = 500;
            zxLaserSessions(5, 6, laserRampDuringDelay, 12, 24, 20, 0.05, 30, 20, 1.0);
            break;


        case 4335:
            splash("LR DNMS 1st Odor", "Laser + -");
            protectedSerialSend(PERM_INFO, DMS_LR_1Odor_Laser);
            taskType = DNMS_2AFC_TEACH;
            laserTrialType = LASER_OTHER_TRIAL;
            zxLaserSessions(5, 6, laserDuring1stOdor, 5, 10, 20, 0.05, 30, 20, 1.0);
            break;

        case 4336:
            splash("LR DNMS 2nd Odor", "Laser + -");
            protectedSerialSend(PERM_INFO, DMS_LR_2Odor_Laser);
            taskType = DNMS_2AFC_TEACH;
            laserTrialType = LASER_OTHER_TRIAL;
            zxLaserSessions(5, 6, laserDuring2ndOdor, 5, 10, 20, 0.05, 30, 20, 1.0);
            break;

        case 4337:
            splash("LR DNMS 1+2 Odor", "Laser + -");
            protectedSerialSend(PERM_INFO, DMS_LR_bothOdor_Laser);
            taskType = DNMS_2AFC_TEACH;
            laserTrialType = LASER_OTHER_TRIAL;
            zxLaserSessions(5, 6, laserDuringOdor, 5, 10, 20, 0.05, 30, 20, 1.0);
            break;

        case 4338:
            splash("LR DNMS Baseline", "Laser + -");
            protectedSerialSend(PERM_INFO, DMS_LR_baseline_Laser);
            taskType = DNMS_2AFC_TEACH;
            laserTrialType = LASER_OTHER_TRIAL;
            zxLaserSessions(5, 6, laserDuringBaseline, 5, 10, 20, 0.05, 30, 20, 1.0);
            break;

        case 4339:
            splash("LR DNMS Response", "Laser + -");
            protectedSerialSend(PERM_INFO, DMS_LR_response_Laser);
            taskType = DNMS_2AFC_TEACH;
            laserTrialType = LASER_OTHER_TRIAL;
            zxLaserSessions(5, 6, laserDuringResponseDelay, 5, 10, 20, 0.05, 30, 20, 1.0);
            break;

        case 4340:
        {
            taskType = DNMS_TASK;
            laser.ramp = 500;
            splash("LR LASER DNMS", "Sufficiency");
            int delay = setDelay(1);
            laserTrialType = LASER_LR_EVERY_OTHER_TRIAL;
            zxLaserSessions(5, 6, laserRampDuringDelay, delay, delay * 2, 20, 0.05, 20, setSessionNum(), 1.0);
            break;
        }

            //        case 4334:
            //            splash("DNMS Shaping", "");
            //            laserTrialType = _LASER_NO_TRIAL;
            //            taskType = _SHAPPING_TASK;
            //            zxLaserSessions(5,6, laserDuringDelay, 4, 8, 20, 0.5, 50, 20, 1.0);
            //            break;

            //        case 4335:
            //        {
            //            splash("DNMS 4s", "DC laser,");
            //            taskType = _DNMS_TASK;
            //            zxLaserSessions(5,6, laserDuringDelay, 4, 8, 20, 0.5, 50, setSessionNum(), 1.0);
            //            break;
            //        }

        case 4341:
            splash("DNMS 5s Shaping", "RQ");
            laserTrialType = LASER_NO_TRIAL;
            taskType = SHAPPING_TASK;
            zxLaserSessions(5, 6, laserDuringDelayChR2, 5, 10, 20, 0.05, 50, setSessionNum(), 1.0);
            break;

            //        case 4342:
            //        {
            //            splash("DNMS 5s Water+", "RQ DC laser");
            //            laserTrialType = _LASER_EVERY_TRIAL;
            //            taskType = _DNMS_TASK;
            //            zxLaserSessions(5,6, laserDuringDelayChR2, 5, 10, 20, 0.1, 50, setSessionNum(), 1.0);
            //            break;
            //        }
        case 4343:
        {
            splash("DNMS 5s ++", "RQ DC laser");
            laserTrialType = LASER_EVERY_TRIAL;
            taskType = DNMS_TASK;
            zxLaserSessions(5, 6, laserDuringDelayChR2, 5, 10, 20, 0.05, 50, setSessionNum(), 1.0);
            break;
        }
        case 4344:
        {
            splash("DNMS 5s", "RQ NoLaser");
            laserTrialType = LASER_NO_TRIAL;
            taskType = DNMS_TASK;
            zxLaserSessions(5, 6, laserDuringDelayChR2, 5, 10, 20, 0.05, 50, setSessionNum(), 1.0);
            break;
        }

        case 4345:
        {
            splash("DNMS 8s ++", "RQ DC laser");
            laserTrialType = LASER_EVERY_TRIAL;
            taskType = DNMS_TASK;
            zxLaserSessions(5, 6, laserDuringDelayChR2, 8, 16, 20, 0.05, 50, setSessionNum(), 1.0);
            break;
        }


            //        case 4351:
            //            splash("Rule Match Shap", "C-B D-A");
            //            laserTrialType = _LASER_NO_TRIAL;
            //            taskType = _ASSOCIATE_SHAPPING_TASK;
            //            zxLaserSessions(0, laserDuringDelayChR2, 5, 10, 20, 0.05, 50, setSessionNum(), 1.0);
            //            break;
            //
            //        case 4352:
            //            splash("Rule Match", "C-B D-A");
            //            laserTrialType = _LASER_NO_TRIAL;
            //            taskType = _ASSOCIATE_TASK;
            //            zxLaserSessions(0, laserDuringDelayChR2, 5, 10, 20, 0.05, 50, setSessionNum(), 1.0);
            //            break;

        case 4353:
        {
            splash("DNMS 5s +-", "RQ DC laser");
            laserTrialType = LASER_OTHER_TRIAL;
            taskType = DNMS_TASK;
            zxLaserSessions(5, 6, laserDuringDelayChR2, 5, 10, 20, 0.05, 50, setSessionNum(), 1.0);
            break;
        }

        case 4354:
            rampTweak(1, 0);
            break;

        case 4355:
        {
            splash("DNMS 8s +-", "RQ DC laser");
            laserTrialType = LASER_OTHER_TRIAL;
            taskType = DNMS_TASK;
            zxLaserSessions(5, 6, laserDuringDelayChR2, 8, 16, 20, 0.05, 50, setSessionNum(), 1.0);
            break;
        }
        case 4356:
            rampTweak(0, 0);
            break;

        case 4357:
            rampTweak(0, 1);
            break;

        case 4358:
            flashLaser();
            break;

        case 4400:
            write_eeprom(EEP_DUTY_LOW_R_OFFSET, 0x7f);
            write_eeprom(EEP_DUTY_LOW_L_OFFSET, 0x7f);
            break;

        case 4411:
            splash("During Delay", "");
            //            setLaser();
            zxLaserSessions(2, 3, laserDuringDelay, 4, 8, 20, 0.05, 50, setSessionNum(), 1.0);
            break;



        case 4412:
        {
            splash("Vary Delay", "Shaping");
            laserTrialType = LASER_NO_TRIAL;
            taskType = SHAPPING_TASK;
            int delay = setDelay(2);
            int type = setType();
            zxLaserSessions(type, type + 1, laserDuringDelay, delay, delay * 2, 20, 0.05, 50, 40, 1.0);
            break;
        }


        case 4413:
        {
            splash("DNMS LR Distra.", "Laser EveryTrial");
            taskType = DNMS_2AFC_TASK;
            laserTrialType = LASER_EVERY_TRIAL;
            zxLaserSessions(5, 6, laserDelayDistractor, 5, 10, 20, 0.05, 200, 10, 1.0);
            break;
        }
        case 4414:
        {
            splash("Varying Delay", "DC laser,");
            taskType = DNMS_TASK;
            zxLaserSessions(5, 6, laserDuringDelay, 4, 8, 20, 0.125, 50, 5, 1.0);
            break;
        }
        case 4415:
        {
            splash("Varying Delay", "DC laser,");
            int delay = setDelay(1);
            taskType = DNMS_TASK;
            laserTrialType = setLaser();
            int type = setType();
            zxLaserSessions(type, type + 1, laserDuringDelayChR2, delay, delay * 2, 20, 0.05, 50, setSessionNum(), 1.0);
            break;
        }


        case 4416:
        {
            splash("Varying Delay", "DC laser,");
            int delay = setDelay(1);
            taskType = DNMS_TASK;
            laserTrialType = setLaser();
            zxLaserSessions(5, 6, laserDuringDelayChR2, delay, delay * 2, 20, 0.036, 50, setSessionNum(), 1.0);
            break;
        }

        case 4417:
        {
            splash("DNMS Shaping", "");
            laserTrialType = LASER_NO_TRIAL;
            taskType = SHAPPING_TASK;
            int type = setType();
            zxLaserSessions(type, type + 1, laserDuringDelay, 4, 8, 20, 0.036, 50, setSessionNum(), 1.0);
            break;
        }
        case 4418:
            splash("Feed Water", "");
            feedWaterFast(36);
            break;

        case 4419:
            splash("Test Volume", "");
            testVolume(36);
            break;

        case 4421:
        {
            splash("DNMS Shaping", "");
            laserTrialType = LASER_NO_TRIAL;
            taskType = SHAPPING_TASK;
            int type = setType();
            zxLaserSessions(type, type + 1, laserDuringDelay, 4, 8, 20, 0.05, 50, setSessionNum(), 1.0);
            break;
        }
        case 4422:
        {
            splash("DNMS Shaping", "Long Water");
            laserTrialType = LASER_NO_TRIAL;
            taskType = SHAPPING_TASK;
            int type = setType();
            zxLaserSessions(type, type + 1, laserDuringDelay, 4, 8, 20, 0.1, 50, 40, 0.5);
            break;
        }
        case 4423:
            testValve();
            break;

        case 4424:
            feedWaterFast(50);
            break;


        case 4425:
        {
            splash("Learning", "DC laser");
            int type = setType();
            zxLaserSessions(type, type + 1, laserDuringDelay, 4, 8, 20, 0.05, 60, setSessionNum(), 1.0);
            break;
        }
            //        case 4426:
            //            shiftingLaser();
            //            break;

        case 4431:
        {
            splash("VarDelay LR", "DC laser");
            int delay = setDelay(1);
            taskType = DNMS_2AFC_TASK;
            int type = setType();
            zxLaserSessions(type, type + 1, laserDuringDelay, delay, delay * 2, 20, 0.05, 50, setSessionNum(), 1.0);
            break;
        }
        case 4432:
        {
            taskType = GONOGO_LR_TASK;
            splash("GoNogo LR", "");
            int m = getFuncNumber(1, "No Pre Odor Post");
            int laserPeriod;
            laserPeriod = laserDuring4Baseline;
            switch (m) {
                case 1:
                    taskType = GONOGO_2AFC_TEACH;
                    laserTrialType = LASER_NO_TRIAL;
                    break;
                case 3:
                    laserPeriod = laserDuring1stOdor;
                    break;
                case 4:
                    laserPeriod = laserDuringResponseDelay;
                    break;
            }
            int type = setType();
            zxLaserSessions(type, type + 1, laserPeriod, 0, 5, 20, 0.05, 50, 30, 1.0);
            break;
        }

        case 4433:
        {
            splash("DNMS LR", "Laser EveryTrial");
            taskType = DNMS_2AFC_TASK;
            laserTrialType = LASER_EVERY_TRIAL;
            zxLaserSessions(5, 6, laserDuringDelayChR2, 5, 10, 20, 0.05, 200, 10, 1.0);
            break;
        }

        case 4434:
            laserTrain();
            break;

        case 4435:
        {
            splash("Baseline LR", "Laser EveryTrial");
            taskType = DNMS_2AFC_TASK;
            zxLaserSessions(5, 6, laserDuring3Baseline, 5, 10, 20, 0.05, 200, 20, 1.0);
            break;
        }

            //        case 4436:
            //        {
            //            splash("DNMS LR", "Laser EveryTrial");
            //            taskType = _DNMS_LR_TASK;
            //            laserTrialType = _LASER_EVERY_TRIAL;
            //            zxLaserSessions(5,6, laserDelayDistractor, 5, 10, 20, 0.05, 200, 10, 1.0);
            //            break;
            //        }

        case 4441:
        {
            splash("First Odor LR", "Laser EveryTrial");
            taskType = DNMS_2AFC_TASK;
            zxLaserSessions(5, 6, laserDuring1stOdor, 5, 10, 20, 0.05, 200, 20, 1.0);
            break;
        }
        case 4442:
        {
            splash("Second Odor LR", "Laser EveryTrial");
            taskType = DNMS_2AFC_TASK;
            zxLaserSessions(5, 6, laserDuring2ndOdor, 5, 10, 20, 0.05, 200, 20, 1.0);
            break;
        }
        case 4443:
        {
            splash("Response Delay LR", "Laser EveryTrial");
            taskType = DNMS_2AFC_TASK;
            zxLaserSessions(5, 6, laserDuringResponseDelay, 5, 10, 20, 0.05, 200, 20, 1.0);
            break;
        }


        case 4445:
            feedWaterLR(50);
            break;

        case 4446:
        {
            unsigned int f = getFuncNumberMarquee(3, "0123456798ABCDEFGHIJKLMN", 24);
            home_clr();
            lcdWriteNumber(f, 4, 1, 1);
            while (1);
        }

    }
}


