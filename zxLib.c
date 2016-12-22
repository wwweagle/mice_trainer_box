#include "commons.h"
#include "zxLib.h"

//DEBUG


STIM_T stims = {.stim1Length = 1000u, .stim2Length = 1000u, .distractorLength = 500u, .currentDistractor = 7u, .distractorJudgingPair = 8u, .distr2 = 7u, .distr3 = 7u};
LASER_T laser = {.timer = 0u, .onTime = 65535u, .offTime = 0u, .ramp = 0u, .ramping = 0u, .on = 0u, .side = 1u}; //1L,2R,3LR
PWM_T pwm = {.L_Hi = 0xfe, .R_Hi = 0xfe, .L_Lo = 0u, .R_Lo = 0u, .fullDuty = 0xfe};
LICK_T lick = {.current = 0u, .filter = 0u, .flag = 0u, .LCount = 0u, .RCount = 0u};
const char odorTypes[] = " WBYLRQHN0123456789012345678901234567890123456789";
unsigned int laserSessionType = LASER_EVERY_TRIAL;
unsigned int taskType = DNMS_TASK;
unsigned int wait_Trial = 1u;
unsigned int timeSum = 0u;
int psedoRanInput;
//unsigned long timerCounter32;

unsigned int highLevelShuffleLength = 12u;

const _prog_addressT EE_Addr = 0x7ff000;

static void zxLaserTrial(int type, int firstOdor, STIM_T odors, _delayT interOdorDelay, int secondOdor, float waterPeroid, unsigned int ITI);
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
    timerCounterI = 0u;
    timerCounterJ = 0u;
    //    timerCounter32 = 0u;
    //

    TMR1 = 0;
    PR1 = TMR_RATE / 200;
    IFS0bits.T1IF = 0;
    IEC0bits.T1IE = 1;
    T1CON = 0x8020;
    ConfigIntTimer1(T1_INT_PRIOR_5 & T1_INT_ON);

    lick.LCount = 0u;
    lick.RCount = 0u;
}

inline int filtered(void) {
    return (timerCounterJ < lick.filter || timerCounterJ > lick.filter + 50u);
    //    return (timerCounter32 < lick.filter || timerCounter32 > lick.filter + 50u);
}

inline void tick(unsigned int i) {
    laser.timer += i;
    timerCounterI += i;
    if (timerCounterJ == 65535u) {
        timerCounterJ = 0;
    } else {
        timerCounterJ += i;
    }
    //    if (timerCounter32 == LONG_MAX) {
    //        timerCounter32 = 0u;
    //    } else {
    //        timerCounter32 += (unsigned long) i;
    //    }
}

static void resetTimerCounterJ() {
    timerCounterJ = 65535u;
    timeSum = 0;
    while (timerCounterJ > 1);
    //    timerCounter32 = LONG_MAX;
    //    while (timerCounter32 > 1);
    //    timeSum = 0u;
}

static void waitTimerJ(unsigned int dT) {
    timeSum += dT;
    while (timeSum > timerCounterJ);
}

static int isLikeOdorA(int odor) {
    if (odor == 2 || odor == 5 || odor == 7 || odor == 9) return 1; //B,R,H
    return 0;
}

void __attribute__((__interrupt__, no_auto_psv)) _T1Interrupt(void) {
    IFS0bits.T1IF = 0;
    tick(5u);


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
    if (LICK_LEFT && LICK_RIGHT && (lick.current != LICKING_BOTH) && filtered()) {
        lick.filter = timerCounterJ;
        Out1 = 1;
        Nop();
        Nop();
        Nop();
        Out6 = 1;
        if (lick.current == LICKING_LEFT) {
            lick.RCount++;
        } else if (lick.current == LICKING_RIGHT) {
            lick.LCount++;
        } else {
            lick.LCount++;
            lick.RCount++;
        }
        lick.current = LICKING_BOTH;
        localSendOnce(SpLick, 4);
    } else if (LICK_LEFT && lick.current != LICKING_LEFT && filtered()) {
        lick.filter = timerCounterJ; //        lick.filter = timerCounter32;
        lick.current = LICKING_LEFT;
        Out1 = 1;
        Nop();
        Nop();
        Nop();
        Out6 = 0;
        lick.LCount++;
        Nop();
        localSendOnce(SpLick, taskType < 35 ? 1 : 2);
        //        tick(3);
    } else if (LICK_RIGHT && lick.current != LICKING_RIGHT && filtered()) {
        lick.filter = timerCounterJ;
        //        lick.filter = timerCounter32;
        lick.current = LICKING_RIGHT;
        Out6 = 1;
        Nop();
        Nop();
        Nop();
        Out1 = 0;
        lick.RCount++;
        Nop();
        if (taskType != BALL_IMMOBILE) localSendOnce(SpLick, 3);
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

static unsigned int setSessionNum() {

    _delayT d[] = {0u, 1u, 5u, 10u, 12u, 15u, 20u, 24u, 30u};
    unsigned int n = getFuncNumberMarquee(1, "1:1sess 2:5sess 3:10sess 4:12sess 5:15sess 6:20sess 7:24sess 8:30sess ", 70);
    if (n > 0 && n < sizeof (d))
        return d[n];
    else return 5u;
}

static _delayT setDelay() {

    _delayT d[] = {0u, 4u, 5u, 8u, 12u, 13u, 16u, 20u, 30u};
    unsigned int n = getFuncNumberMarquee(1, "0:0s 1:4s 2:5s 3:8s 4:12s 5:13s 6:16s 7:20s 8:30s ", 50);
    if (n > 0 && n < sizeof (d))
        return d[n];
    else return 5u;

}

static int setLaser() {
    int d[] = {LASER_NO_TRIAL, LASER_NO_TRIAL, LASER_OTHER_TRIAL, LASER_EVERY_TRIAL};
    unsigned int n = getFuncNumber(1, "None 1+1-  Every");
    if (n > 0 && n < 4)
        return d[n];
    else return LASER_NO_TRIAL;
}

//void setLaser(void) {
//    unsigned int n = getFuncNumber(1, "DC=1 20Hz=2");
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
    unsigned int i;
    for (i = 0; i < 4000; i++); //1000 per ms
    localSendOnce(type, value);
    for (i = 0; i < 3000; i++); //1000 per ms
    tick(10);
    IEC0bits.T1IE = 1;
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

static void assertLaser(int type, int step) {
    switch (type) {
        case laserOff:
            break;
        case laserDuringBeginningToOneSecInITI:
            if (step == atFirstOdorBeginning) {
                turnOnLaser(3);
            } else if (step == atITIOneSecIn) {
                turnOffLaser();
            }
            break;
        case laserDuringDelay:
            if (step == atDelay1SecIn) {
                turnOnLaser(3);
            } else if (step == atDelayLast500mSBegin) {
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
            } else if (step == atDelayLastSecBegin || step == at500mSBeforeFirstOdor) {
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
            } else if (step == atDelayLast500mSBegin) {
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
            } else if (step == atDelay500msToMiddle) {
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
            } else if (step == atDelay500msToMiddle) {
                turnOffLaser();
            }
            break;
        case laserDuring12s3Quarter:
            if (step == atDelayMiddle) {
                turnOnLaser(laser.side);
            } else if (step == atDelayMid2_5Sec) {
                turnOffLaser();
            }
            break;
        case laserDuring12s4Quarter:
            if (step == atDelayMid3Sec) {
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
            } else if (step == at500mSBeforeFirstOdor) {
                turnOffLaser();
            }
            break;
        case laserDuringBaseAndResponse:
            if (step == oneSecBeforeFirstOdor || step == atSecondOdorEnd) {
                turnOnLaser(3);
            } else if (step == at500mSBeforeFirstOdor || step == atRewardBeginning) {

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
            //        case laserDelayDistractorEarly:
            //            if (step == atDelay2SecIn) {
            //                turnOnLaser(3);
            //            } else if (step == atDelay2SecIn) {
            //                turnOffLaser();
            //            }
            //            break;
            //        case laserDelayDistractorLate:
            //            if (step == atDelay1SecIn) {
            //                turnOnLaser(3);
            //            } else if (step == atDelay2SecIn) {
            //                turnOffLaser();
            //            }
            //            break;
        case laserRampDuringDelay:
            if (step == atDelay1SecIn) {
                turnOnLaser(laser.side);
            } else if (step == atDelayLast500mSBegin) {
                turnOffLaser();
            }
            break;
            //        case laserAfterDistractor:
            //            if (step == atDelayMid2_5Sec) {
            //                turnOnLaser(laser.side);
            //            } else if (step == atDelayLast2_5SecBegin) {
            //                turnOffLaser();
            //            }
            //            break;
            //        case laserAfterDistractorLong:
            //            if (step == atDelayMid2_5Sec) {
            //                turnOnLaser(laser.side);
            //            } else if (step == atDelayLast500mSBegin) {
            //                turnOffLaser();
            //            }
            //            break;
        case laserDuring1Terice:
            if (step == atFirstOdorEnd) {
                turnOnLaser(laser.side);
            } else if (step == atDelay3_5SIn) {
                turnOffLaser();
            }
            break;
        case laserDuring2Terice:
            if (step == atDelay4_5SIn) {
                turnOnLaser(laser.side);
            } else if (step == atDelayMid1_5Sec) {
                turnOffLaser();
            }
            break;
        case laserDuring3Terice:
            if (step == atDelayMid2_5Sec) {
                turnOnLaser(laser.side);
            } else if (step == atDelayLast500mSBegin) {
                turnOffLaser();
            }
            break;
        case laserCoverDistractor:
            if (step == atPreDualTask) {
                turnOnLaser(laser.side);
            } else if (step == atPostDualTask) {
                turnOffLaser();
            }
            break;

        case laserAfterDistractorMax:
            if (step == atPostDualTask) {
                turnOnLaser(laser.side);
            } else if (step == atDelayLast500mSBegin) {
                turnOffLaser();
            }
            break;

        case laserAfterMultiDistractor:
            if (step == atPostDualTask) {
                turnOnLaser(laser.side);
            } else if (step == atPreDualTask) {
                turnOffLaser();
            }
            break;


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
    int startIdx = (timerCounterJ >> 9) % l;
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

static void waterNResult(int firstOdor, int secondOdor, float waterPeroid, int id) {
    lick.flag = 0;
    switch (taskType) {

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

        default:
            /*///////////////
             *DNMS
             *//////////////
            //        case DNMS_TASK:
            //        case SHAPING_TASK:
            //        case OPTO_ODPA_TASK:
            //        case OPTO_ODPA_SHAPING_TASK:
            //        case NO_ODOR_CATCH_TRIAL_TASK:
            //        case VARY_ODOR_LENGTH_TASK:
            //        case DELAY_DISTRACTOR:

            //        case _ASSOCIATE_TASK:
            //        case _ASSOCIATE_SHAPING_TASK:
            //        

            ///////////Detect/////////////////
            for (timerCounterI = 0; timerCounterI < 500 && !lick.flag; lick.flag = lick.current);

            /////Reward
            if (!lick.flag) {
                if (isLikeOdorA(firstOdor) == isLikeOdorA(secondOdor)) {
                    protectedSerialSend(SpCorrectRejection, id);
                    lcdWriteNumber(++correctRejection, 3, 10, 2);
                } else {
                    processMiss(id);
                    if ((taskType == SHAPING_TASK || taskType == ODPA_SHAPING_TASK
                            || taskType == DUAL_TASK_LEARNING || taskType == DNMS_DUAL_TASK_LEARNING) && ((rand() % 3) == 0)) {
                        protectedSerialSend(22, 1);
                        Valve_ON(water_sweet, pwm.fullDuty);
                        protectedSerialSend(SpWater_sweet, 1);
                        wait_ms(waterPeroid * 1000);
                        Valve_OFF(water_sweet);
                    }
                }
            } else if (isLikeOdorA(firstOdor) == isLikeOdorA(secondOdor)) {
                processFalse(id);
            } else {
                processHit(waterPeroid, 1, id);
            }
            break;

    }
}

static void distractor(unsigned int distractOdor, unsigned int judgingPair, float waterLen) {
    if (distractOdor == 0) {
        wait_ms(1500u);
    } else {
        Valve_ON(distractOdor, pwm.fullDuty);
        if (isLikeOdorA(distractOdor)) Out2 = 1;
        else Out3 = 1;
        protectedSerialSend(isLikeOdorA(distractOdor) ? SpOdor_C : SpOdor_D, distractOdor);
        lcdWriteChar(isLikeOdorA(distractOdor) ? '.' : ':', 4, 1);
        wait_ms(stims.distractorLength - 10u);
        protectedSerialSend(isLikeOdorA(distractOdor) ? SpOdor_C : SpOdor_D, 0);
        Valve_OFF(distractOdor);
        Out2 = 0;
        Nop();
        Nop();
        Out3 = 0;
        Nop();
        Nop();
        lcdWriteChar('D', 4, 1);
        wait_ms(450u);
        waterNResult(distractOdor, judgingPair, waterLen, 3);
    }
}

static void waitTrial() {
    static int waitingLickRelease = 0;
    if (!wait_Trial) {
        return;
    }
    while (LICK_ANY) {
        if (!waitingLickRelease) {
            protectedSerialSend(20, 100);
            waitingLickRelease = 1;
        }
    }
    waitingLickRelease = 0;

    while (u2Received != 0x31) {
        protectedSerialSend(20, 1);
    }
    u2Received = -1;
}

static void zxLaserSessions(int stim1, int stim2, int laserTrialType, _delayT delay, unsigned int ITI, int trialsPerSession, float WaterLen, int missLimit, unsigned int totalSession) {

    //    wait_ms(1000);
    int currentTrial = 0;
    unsigned int currentSession = 0;
    int laserOnType = laserTrialType;

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
            int iterOf4;
            for (iterOf4 = 0; iterOf4 < 4 && currentMiss < missLimit; iterOf4++) {
                //                wait_ms(1000);
                int index = shuffledList[iterOf4];
                switch (taskType) {

                    case DNMS_TASK:
                        firstOdor = (index == 0 || index == 2) ? stim1 : stim2;
                        secondOdor = (index == 1 || index == 2) ? stim1 : stim2;
                        break;
                    case SHAPING_TASK:
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
                    case OPTO_ODPA_TASK:
                        firstOdor = (index == 0 || index == 2) ? 9 : 10;
                        secondOdor = (index == 1 || index == 2) ? stim1 : stim2;
                        break;

                    case OPTO_ODPA_SHAPING_TASK:
                        firstOdor = (index == 0 || index == 2) ? 9 : 10;
                        secondOdor = (firstOdor == 9) ? stim2 : stim1;
                        break;
                    case ODPA_TASK:
                        firstOdor = (index == 0 || index == 2) ? stim1 : (stim1 + 1);
                        secondOdor = (index == 1 || index == 2) ? stim2 : (stim2 + 1);
                        break;

                    case ODPA_SHAPING_TASK:
                        firstOdor = (index == 0 || index == 2) ? stim1 : (stim1 + 1);
                        secondOdor = (firstOdor == stim1) ? (stim2 + 1) : stim2;
                        break;
                    case DUAL_TASK_LEARNING:
                    case DUAL_TASK:
                        firstOdor = (index == 0 || index == 2) ? stim1 : (stim1 + 1);
                        secondOdor = (index == 1 || index == 2) ? stim2 : (stim2 + 1);
                        switch (shuffledLongList[currentTrial] % 3) {
                            case 0:
                                stims.currentDistractor = 0u;
                                break;
                            case 1:
                                stims.currentDistractor = 7u;
                                break;
                            case 2:
                                stims.currentDistractor = 8u;
                                break;
                        }
                        break;
                    case DUAL_TASK_EVERY_TRIAL:
                        firstOdor = (index == 0 || index == 2) ? stim1 : (stim1 + 1);
                        secondOdor = (index == 1 || index == 2) ? stim2 : (stim2 + 1);
                        if (shuffledLongList[currentTrial] % 2)
                            stims.currentDistractor = 7u;
                        else
                            stims.currentDistractor = 8u;
                        break;
                    case DUAL_TASK_DISTRx3_TASK:
                        firstOdor = (index == 0 || index == 2) ? stim1 : (stim1 + 1);
                        secondOdor = (index == 1 || index == 2) ? stim2 : (stim2 + 1);

                        unsigned int di[] = {7u, 8u};
                        stims.currentDistractor = di[shuffledLongList[currentTrial * 3] % 2];
                        stims.distr2 = di[shuffledLongList[currentTrial * 3 + 1] % 2];
                        stims.distr3 = di[shuffledLongList[currentTrial * 3 + 2] % 2];
                        break;

                    case DUAL_TASK_ON_OFF_LASER_TASK:
                        firstOdor = (index == 0 || index == 2) ? stim1 : (stim1 + 1);
                        secondOdor = (index == 1 || index == 2) ? stim2 : (stim2 + 1);
                        switch (shuffledLongList[currentTrial] % 8) {
                            case 0:
                            case 1:
                            case 2:
                            case 3:
                                stims.currentDistractor = 0u;
                                break;
                            case 4:
                            case 6:
                                stims.currentDistractor = 7u;
                                break;
                            case 5:
                            case 7:
                                stims.currentDistractor = 8u;
                                break;
                        }
                        break;

                    case DUAL_TASK_ODAP_ON_OFF_LASER_TASK:
                        stims.currentDistractor = (index % 2) ? 7u : 8u;
                        switch (shuffledLongList[currentTrial] % 8) {
                            case 0:
                            case 1:
                            case 2:
                            case 3:
                                firstOdor = secondOdor = 20u;
                                break;
                            case 4:
                                firstOdor = stim1;
                                secondOdor = stim2;
                                break;
                            case 5:
                                firstOdor = stim1;
                                secondOdor = stim2 + 1;
                                break;
                            case 6:
                                firstOdor = stim1 + 1;
                                secondOdor = stim2;
                                break;
                            case 7:
                                firstOdor = stim1 + 1;
                                secondOdor = stim2 + 1;
                                break;
                        }
                        break;
                    case DNMS_DUAL_TASK_LEARNING:
                    case DNMS_DUAL_TASK:
                        firstOdor = (index == 0 || index == 2) ? stim1 : stim2;
                        secondOdor = (index == 1 || index == 2) ? stim1 : stim2;
                        switch (shuffledLongList[currentTrial] % 3) {
                            case 0:
                                stims.currentDistractor = 0u;
                                break;
                            case 1:
                                stims.currentDistractor = 7u;
                                break;
                            case 2:
                                stims.currentDistractor = 8u;
                                break;
                        }
                        break;
                }

                lcdWriteChar(odorTypes[firstOdor], 1, 1);
                lcdWriteChar(odorTypes[secondOdor], 2, 1);

                //                int laserCurrentTrial;

                switch (laserSessionType) {
                    case LASER_NO_TRIAL:
                        laserTrialType = laserOff;
                        break;
                    case LASER_EVERY_TRIAL:
                        break;
                    case LASER_OTHER_TRIAL:
                        laserTrialType = (currentTrial % 2) == 0 ? laserOff : laserOnType;
                        break;

                    case LASER_LR_EACH_QUARTER:
                        laser.side = isLikeOdorA(firstOdor) ? 1 : 2;
                    case LASER_EACH_QUARTER:
                        switch (currentTrial % 5) {
                            case 0:
                                laserTrialType = laserOff;
                                break;
                            case 1:
                                laserTrialType = laserDuring1Quarter;
                                break;
                            case 2:
                                laserTrialType = laserDuring2Quarter;
                                break;
                            case 3:
                                laserTrialType = laserDuring3Quarter;
                                break;
                            case 4:
                                laserTrialType = laserDuring4Quarter;
                                break;
                        }
                        break;


                    case LASER_12s_LR_EACH_QUARTER:
                        laser.side = isLikeOdorA(firstOdor) ? 1 : 2;
                    case LASER_12s_EACH_QUARTER:
                        switch (currentTrial % 5) {
                            case 0:
                                laserTrialType = laserOff;
                                break;
                            case 1:
                                laserTrialType = laserDuring12s1Quarter;
                                break;
                            case 2:
                                laserTrialType = laserDuring12s2Quarter;
                                break;
                            case 3:
                                laserTrialType = laserDuring12s3Quarter;
                                break;
                            case 4:
                                laserTrialType = laserDuring12s4Quarter;
                                break;
                        }
                        break;


                    case LASER_VARY_LENGTH:
                        switch (currentTrial % 5) {
                            case 0:
                                laserTrialType = laserOff;
                                break;
                            case 1:
                                laserTrialType = laser4sRamp;
                                break;
                            case 2:
                                laserTrialType = laser2sRamp;
                                break;
                            case 3:
                                laserTrialType = laser1sRamp;
                                break;
                            case 4:
                                laserTrialType = laser_5sRamp;
                                break;
                        }
                        break;


                    case LASER_LR_EVERYTRIAL:
                        laser.side = isLikeOdorA(firstOdor) ? 1 : 2;
                        break;

                    case LASER_LR_EVERY_OTHER_TRIAL:
                        laser.side = isLikeOdorA(firstOdor) ? 1 : 2;

                        laserTrialType = (currentTrial % 2) == 0 ? laserOff : laserOnType;
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
                        break;
                    case LASER_13s_EarlyMidLate:
                        switch (currentTrial % 4) {
                            case 0:
                                laserTrialType = laserOff;
                                break;
                            case 1:
                                laserTrialType = laserDuring1Terice;
                                break;
                            case 2:
                                laserTrialType = laserDuring2Terice;
                                break;
                            case 3:
                                laserTrialType = laserDuring3Terice;
                                break;
                        }
                        break;
                    case LASER_DUAL_TASK_ON_OFF:
                        switch (shuffledLongList[currentTrial] % 8) {
                            case 0:
                            case 1:
                            case 4:
                            case 5:
                                laserTrialType = laserOff;
                                break;
                            case 2:
                            case 3:
                            case 6:
                            case 7:
                                laserTrialType = laserCoverDistractor;
                                break;
                        }
                        break;
                    case LASER_DUAL_TASK_ODAP_ON_OFF:
                        laserTrialType = (index < 2) ? laserOff : laserCoverDistractor;
                        break;
                    case LASER_OTHER_BLOCK:
                        if (psedoRanInput)
                            laserTrialType = currentTrial < (trialsPerSession / 2) ? laserOff : laserOnType;
                        else
                            laserTrialType = currentTrial < (trialsPerSession / 2) ? laserOnType : laserOff;
                        break;
                }
                zxLaserTrial(laserTrialType, firstOdor, stims, delay, secondOdor, WaterLen, ITI);
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
//   i switch (stim) {
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

static void stim(int place, int stim, int type) {
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
        case 20://FAKE ODAP 
            assertLaser(type, place == 1 ? atFirstOdorBeginning : atSecondOdorBeginning);
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
            waitTimerJ(place == 1 ? stims.stim1Length : stims.stim2Length);
            Valve_OFF(stim);
            Out2 = 0;
            Nop();
            Nop();
            Out3 = 0;
            Nop();
            Nop();
            protectedSerialSend(stimSend, 0);
            assertLaser(type, place == 1 ? atFirstOdorEnd : atSecondOdorEnd);
            lcdWriteChar(place == 1 ? 'd' : 'D', 4, 1);
            waitTimerJ(1000u - (place == 1 ? stims.stim1Length : stims.stim2Length));
            break;
            //        case 20:
            //        case 21:
            //        case 22:
            //        case 23:
            //            optoStim(stim, place == 1 ? stims.stim1Length : stims.stim2Length, place);
            //            break;


    }
}

static void zxLaserTrial(int type, int firstOdor, STIM_T odors, _delayT interOdorDelay, int secondOdor, float waterPeroid, unsigned int ITI) {
    resetTimerCounterJ();
    protectedSerialSend(Sptrialtype, type);
    protectedSerialSend(Splaser, (type != laserOff));
    assertLaser(type, fourSecBeforeFirstOdor);
    waitTimerJ(1000u);
    assertLaser(type, threeSecBeforeFirstOdor);
    waitTimerJ(2000u);
    assertLaser(type, oneSecBeforeFirstOdor);
    waitTimerJ(500u);
    assertLaser(type, at500mSBeforeFirstOdor);
    waitTimerJ(500u);

    /////////////////////////////////////////////////
    stim(1, firstOdor, type);
    ////////////////////////////////////////////////


    switch (taskType) {
            /********* 3 x distractor ************************/
        case DUAL_TASK_DISTRx3_TASK:
            waitTimerJ(2000u);
            unsigned int di[] = {stims.currentDistractor, stims.distr2, stims.distr3};
            int iter = 0;
            for (; iter < 3; iter++) {
                distractor(di[iter], stims.distractorJudgingPair, waterPeroid);
                waitTimerJ(1500u);
                assertLaser(type, atPostDualTask);
                waitTimerJ(2000u);
                assertLaser(type, atPreDualTask);
                waitTimerJ(500u);
            }
            stim(2, secondOdor, type);
            break;

            //Do nothing during Go Nogo Tasks
        case GONOGO_LR_TASK:
        case GONOGO_TASK:
        case GONOGO_2AFC_TEACH:
            assertLaser(type, atSecondOdorEnd);
            break;


        default:////////////////////////////////////DELAY/////////////////////
            if (interOdorDelay == 0) {
                waitTimerJ(200u); ////////////////NO DELAY////////////////////
            } else {

                assertLaser(type, atDelayBegin);
                waitTimerJ(500u);
                assertLaser(type, atDelay_5SecIn);
                waitTimerJ(500u);
                assertLaser(type, atDelay1SecIn); ////////////////1Sec////////////

                if (interOdorDelay < 4) {
                    waitTimerJ(interOdorDelay * 1000u - 2000u);
                } else {

                    waitTimerJ(500u);
                    assertLaser(type, atDelay1_5SecIn); ////////////////1.5Sec////////////////
                    waitTimerJ(500u);

                    assertLaser(type, atDelay2SecIn); /////////////2Sec/////////////



                    /*/////////////////////////////////////////////////
                     * ////////DISTRACTOR//////////////////////////////
                     * //////////////////////////////////////////////*/
                    if (taskType == DUAL_TASK_LEARNING || taskType == DUAL_TASK
                            || taskType == DUAL_TASK_ON_OFF_LASER_TASK || taskType == DUAL_TASK_ODAP_ON_OFF_LASER_TASK
                            || taskType == DNMS_DUAL_TASK_LEARNING || taskType == DNMS_DUAL_TASK || taskType == DUAL_TASK_EVERY_TRIAL) {
                        assertLaser(type, atPreDualTask); //@2s
                        distractor(stims.currentDistractor, stims.distractorJudgingPair, waterPeroid);
                        waitTimerJ(1500u);
                        assertLaser(type, atPostDualTask); //distractor@3.5sec
                        if (interOdorDelay > 8u) {
                            waitTimerJ((interOdorDelay - 8u)*500u);
                        }
                    } else if (interOdorDelay >= 12u) {
                        waitTimerJ(500u);
                        assertLaser(type, atDelay2_5SecIn);
                        waitTimerJ(500u);
                        assertLaser(type, atDelay3SecIn);
                        waitTimerJ(500u); //->3.5s
                        assertLaser(type, atDelay3_5SIn);
                        waitTimerJ(1000u); //->4.5s
                        assertLaser(type, atDelay4_5SIn);
                        waitTimerJ(1000u);
                        waitTimerJ((interOdorDelay - 12u)*500u);
                    } else {
                        waitTimerJ(interOdorDelay * 500u - 3000u);
                        assertLaser(type, atDelay1sToMiddle); //13@5500
                        waitTimerJ(500u); //13@6000
                    }

                    assertLaser(type, atDelay500msToMiddle); //distractor@6 //13@6000
                    waitTimerJ(500u);
                    assertLaser(type, atDelayMiddle); //13@6.5
                    if (interOdorDelay >= 12) {
                        waitTimerJ(500u); //13@7
                        assertLaser(type, atDelayMid500mSec);
                        waitTimerJ(500u); //13@7.5
                        assertLaser(type, atDelayMid1Sec);
                        waitTimerJ(500u); //13@8S
                        assertLaser(type, atDelayMid1_5Sec);
                        waitTimerJ(500u); //13@8.5
                        assertLaser(type, atDelayMid2Sec);
                        waitTimerJ(500u); //distractor@9s//13@9
                        assertLaser(type, atDelayMid2_5Sec);
                        waitTimerJ(500u); //distractor@9.5s//13@9.5
                        assertLaser(type, atDelayMid3Sec);
                        waitTimerJ((interOdorDelay - 11)*500u); //13@10
                    } else {
                        waitTimerJ(interOdorDelay * 500uL - 2500u);
                    }
                    assertLaser(type, atDelayLast2_5SecBegin);
                    waitTimerJ(500u); //13@10.5
                    assertLaser(type, atDelayLast2SecBegin); //////////////-2 Sec//////////////////////

                    waitTimerJ(500u);
                    assertLaser(type, atDelayLast1_5SecBegin);
                    waitTimerJ(500u);

                }
                assertLaser(type, atDelayLastSecBegin); /////////////////////////-1 Sec////////////////
                waitTimerJ(500u);
                assertLaser(type, atDelayLast500mSBegin);
                //            waitTimerJ(300u);
                //            assertLaser(type, atDelayLast200mSBegin);
                //            waitTimerJ(200u);
                waitTimerJ(500u);
            }

            ///////////-Second odor-/////////////////
            stim(2, secondOdor, type);
            //////////////////////////////////////////
            break;
    }
    waitTimerJ(1000u);
    assertLaser(type, atRewardBeginning);
    lcdWriteChar('R', 4, 1);

    //Assess Performance here
    int id = (taskType == DUAL_TASK || taskType == DUAL_TASK_LEARNING
            || taskType == DUAL_TASK_ON_OFF_LASER_TASK || taskType == DUAL_TASK_ODAP_ON_OFF_LASER_TASK
            || taskType == DNMS_DUAL_TASK_LEARNING || taskType == DNMS_DUAL_TASK || taskType == DUAL_TASK_EVERY_TRIAL) ? 2 : 1;
    waterNResult(firstOdor, secondOdor, waterPeroid, id);

    waitTimerJ(550u); //water time sync

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
    assertLaser(type, atITIBeginning);
    waitTimerJ(1000u);
    assertLaser(type, atITIOneSecIn);
    unsigned int trialITI = ITI - 5u;
    while (trialITI > 60u) {
        resetTimerCounterJ();
        waitTimerJ(60u * 1000u);
        trialITI -= 60u;
    }
    resetTimerCounterJ();
    waitTimerJ(trialITI * 1000u); //another 4000 is at the beginning of the trials.
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

void odorDepeltion(int totalTrial, int p3, int p78) {
    initZXTMR();
    protectedSerialSend(1, 0xff);
    PDC2 = pwm.fullDuty;
    PDC4 = pwm.fullDuty;
    int i;
    for (i = 0; i < totalTrial; i++) {
        home_clr();
        lcdWriteString("Odor Depletion");
        ;
        lcdWriteNumber(i, 3, 1, 2);
        lcdWriteString(", Valve ");
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

        if (p78) {
            LCD_set_xy(13, 2);
            lcdWriteString("7|8");
            Valve_ON(7, pwm.fullDuty);
            Nop();
            Nop();
            Valve_ON(8, pwm.fullDuty);
            delaymSecKey(15000, 3);
            Valve_OFF(7);
            Nop();
            Nop();
            Valve_OFF(8);
        }
    }
}

static void rampTweak(int L, int itiIndex) {
    int laserLength[] = {3500, 3500, 6500, 10500, 1500, 2500};
    unsigned int delay = laserLength[getFuncNumber(1, "5 8 12 1.5 2.5s")];
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

void resetLaserPower() {
    unsigned int f = getFuncNumber(1, "0 Confirm Reset");
    if (f == 0) {
        write_eeprom(EEP_DUTY_LOW_L_OFFSET, 255);
        safe_wait_ms(100);
        write_eeprom(EEP_DUTY_LOW_R_OFFSET, 255);
        safe_wait_ms(100);
        write_eeprom(EEP_DUTY_HIGH_L_OFFSET, 255);
        safe_wait_ms(100);
        write_eeprom(EEP_DUTY_HIGH_R_OFFSET, 255);
        safe_wait_ms(100);
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
        if (lick.LCount > totalLickCount) {
            if (timerCounterI >= 500) {
                Valve_ON(water_sweet, pwm.fullDuty);
                timerCounterI = 0;
                lcdWriteNumber(++waterCount, 4, 13, 2);
            }
            totalLickCount = lick.LCount;
            lcdWriteNumber(totalLickCount, 4, 12, 1);
        }
        if (timerCounterI >= waterLength) {
            Valve_OFF(water_sweet);
        }
    }
}
//
//static void feedWaterBall(int waterLength) {
//
//    lick.LCount = 0;
//    lick.RCount = 0;
//    unsigned int waterCount = 0;
//    unsigned int totalLickCount = 0;
//    splash("Total Lick", "");
//
//    timerCounterI = 1000;
//    while (1) {
//        if (lick.LCount + lick.RCount > totalLickCount) {
//            if (timerCounterI >= 500) {
//                Valve_ON(water_sweet, pwm.fullDuty);
//                timerCounterI = 0;
//                lcdWriteNumber(++waterCount, 4, 13, 2);
//            }
//            totalLickCount = lick.LCount + lick.RCount;
//            lcdWriteNumber(totalLickCount, 4, 12, 1);
//        }
//        if (timerCounterI >= waterLength) {
//            Valve_OFF(water_sweet);
//        }
//    }
//}

void immobileBall(int t, int waterRation) {
    int rationStart=waterRation;
//    int waterITI = t * 1000;
    protectedSerialSend(SpStepN, taskType);
    splash("Rewarded", "");
    //    int waterRation = 400;
    int moving = 0;
    int lickCount = 0;

    while (1) {
        int recentMove = (lick.current == LICKING_BOTH || lick.current == LICKING_RIGHT);
        if (recentMove) {
            if (!moving) {
                moving = 1;
                lcdWriteChar('M', 15, 1);
                protectedSerialSend(SpBallMove, 1);
            }
            wait_ms(20);
        } else {
            timerCounterI = 0;
            int moveFlag = 0;
            while (timerCounterI < t * 1000) {
                if (lick.current == LICKING_BOTH || lick.current == LICKING_RIGHT) {
                    moveFlag = 1;
                    break;
                }
            }
            if (!moveFlag) {
                if (moving) {
                    moving = 0;
                    protectedSerialSend(SpBallMove, 0);
                    lcdWriteChar(' ', 15, 1);
                }
                if (waterRation > 0) {
                    lickCount = lick.LCount;
                    lcdWriteNumber(rationStart - waterRation, 3, 1, 2);
                    waterRation--;
                    protectedSerialSend(SpHit, 1);
                    Valve_ON(1, pwm.fullDuty);
                    wait_ms(50);
                    Valve_OFF(1);
                }
//                wait_ms(waterITI - 50);
            }
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
        unsigned int n = getFuncNumber(1, VString);
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
                unsigned int m = getFuncNumber(1, "Low Hi");
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
    unsigned int v = getFuncNumber(1, "Valve No?");
    while (1) {

        Valve_ON(v, pwm.fullDuty);
        wait_ms(1000);
        Valve_OFF(v);
        wait_ms(1000);
    }
}

static void testVolume(int waterLength) {
    unsigned int n = getFuncNumber(1, "Valve #");
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
        resetTimerCounterJ();
        turnOnLaser(3);
        waitTimerJ(2000u);
        turnOffLaser();
        waitTimerJ(5000u);
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
    int laserOff_ = 16000;
    int step;
    int sCounter = 0;
    PORTCbits.RC1 = 1;
    pwm.R_Lo = 0xfe;
    for (; sCounter < 20; sCounter++) {
        unsigned int powerx10 = getFuncNumber(2, "Power x10");
        splash("     Power", "Trial");
        lcdWriteNumber(powerx10, 2, 12, 1);

        step = 20;
        for (; step > 0; step--) {
            lcdWriteNumber(step, 2, 7, 2);
            wait_ms(laserOff_);
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

void varifyOpticalSuppression() {
    splash("L OFF   0/100", "");
    int i;
    for (i = 0; i < 100; i++) {
        resetTimerCounterJ();
        Out4 = 1;
        LCD_set_xy(3, 1);
        lcdWriteString("ON ");
        lcdWriteNumber(i + 1, 3, 7, 1);
        waitTimerJ(2000);
        Out4 = 0;
        LCD_set_xy(3, 1);
        lcdWriteString("OFF");
        protectedSerialSend(Splaser, i);
        waitTimerJ(18000);
    }
}

void callFunction(int n) {
    currentMiss = 0;
    pwm.L_Lo = read_eeprom(EEP_DUTY_LOW_L_OFFSET);
    pwm.L_Hi = read_eeprom(EEP_DUTY_HIGH_L_OFFSET);
    pwm.R_Lo = read_eeprom(EEP_DUTY_LOW_R_OFFSET);
    pwm.R_Hi = read_eeprom(EEP_DUTY_HIGH_R_OFFSET);


    PORTFbits.RF0 = 1;
    //    waitTrial();
    srand(timerCounterI);
    initZXTMR();
    switch (n) {
            //            int m;
            //ZX's functions

            //        case 4301:
            //            taskType = NO_ODOR_CATCH_TRIAL_TASK;
            //            laser.ramp = 500;
            //            splash("No Odor Catch", "");
            //
            //            laserTrialType = LASER_LR_EVERYTRIAL;
            //            zxLaserSessions(5, 6, laserRampDuringDelay, 2u, 5u, 20, 0.05, 20, setSessionNum());
            //            break;

        case 4301:
            taskType = BALL_IMMOBILE;
            int waterT;
            int waterRation;
            waterT = getFuncNumber(1, "Water interval");
            waterRation = getFuncNumber(3, "Water Ration");
            immobileBall(waterT, waterRation);
            break;

        case 4302:
        {
            splash("VARY ODOR LEN", "");
            taskType = VARY_ODOR_LENGTH_TASK;
            laserSessionType = LASER_NO_TRIAL;
            _delayT delay = setDelay();
            highLevelShuffleLength = 16;
            int type = setType();
            zxLaserSessions(type, type + 1, laserDuringDelayChR2, delay, delay * 2u, 16u, 0.05, 20, setSessionNum());
            break;
        }
        case 4303:
            splash("Each Quarter", "Delay LR Laser");
            laserSessionType = LASER_LR_EACH_QUARTER;
            laser.ramp = 500;
            zxLaserSessions(5, 6, laserDuringDelay, 12u, 24u, 20, 0.05, 20, setSessionNum());
            break;
        case 4304:
        {
            splash("DNMS ", "Shaping");
            laserSessionType = LASER_NO_TRIAL;
            taskType = SHAPING_TASK;
            int type = setType();
            zxLaserSessions(type, type + 1, laserDuringDelay, 4u, 8u, 20, 0.05, 50, setSessionNum());
            break;
        }
            //        case 4305:
            //        {
            //            splash("DNMS Task", "");
            //            taskType = DNMS_TASK;
            //            laserSessionType = LASER_NO_TRIAL;
            //            int type = setType();
            //            zxLaserSessions(type, type + 1, laserDuringDelayChR2, 5u, 10u, 20, 0.05, 20, setSessionNum());
            //            break;
            //        }
        case 4306:
        {
            splash("OB Opto Stim", "Go No-Go");
            //            zxGoNogoSessions(type,type+1,3, 20, 1, 0.5, 4);
            laserSessionType = LASER_NO_TRIAL;
            taskType = GONOGO_TASK;
            int type = setType();
            zxLaserSessions(type, type + 1, laserDuring3Baseline, 0u, 5u, 20, 0.05, 20, setSessionNum());
            break;
        }
        case 4307:
        {
            splash("OB Opto Stim", "");
            //            zxGoNogoSessions(type,type+1,3, 20, 1, 0.5, 4);
            laserSessionType = LASER_LR_EVERYTRIAL;
            taskType = OPTO_ODPA_TASK;
            int type = setType();
            zxLaserSessions(type, type + 1, laserDuring1stOdor, 4u, 8u, 20, 0.05, 30, setSessionNum());
            break;
        }

        case 4308:
        {
            splash("OB Opto Stim", "Shaping");
            //            zxGoNogoSessions(type,type+1,3, 20, 1, 0.5, 4);
            laserSessionType = LASER_LR_EVERYTRIAL;
            taskType = OPTO_ODPA_SHAPING_TASK;
            int type = setType();
            zxLaserSessions(type, type + 1, laserDuring1stOdor, 4u, 8u, 20, 0.05, 30, setSessionNum());
            break;
        }
        case 4310:
        {
            unsigned int m = getFuncNumber(2, "Time in ms");
            testVolume(m);
            break;
        }
        case 4311:
            testVolume(50);
            break;

        case 4312:
        {
            splash("Response Delay", "Laser Control");
            laserSessionType = LASER_OTHER_TRIAL;
            int type = setType();
            zxLaserSessions(type, type + 1, laserDuringResponseDelay, 5u, 10u, 20, 0.05, 20, setSessionNum());
            break;
        }
        case 4313:
        {
            splash("GoNogo Control", "For DNMS");

            laserSessionType = LASER_OTHER_TRIAL;
            taskType = GONOGO_TASK;
            int type = setType();
            zxLaserSessions(type, type + 1, laserDuring3Baseline, 0u, 5u, 20, 0.05, 20, setSessionNum());
            break;
        }
        case 4314:
        {
            splash("Vary Length", "Delay Laser");
            laserSessionType = LASER_VARY_LENGTH;
            laser.ramp = 500;
            int type = setType();
            zxLaserSessions(type, type + 1, laserDuringDelay, 5u, 10u, 20, 0.05, 15, setSessionNum());
            laser.ramp = 0;
            break;
        }
        case 4315:
            splash("Each Quarter", "Delay Laser");
            laserSessionType = LASER_EACH_QUARTER;
            laser.ramp = 500;
            zxLaserSessions(5, 6, laserDuringDelay, 8u, 16u, 20, 0.05, 20, setSessionNum());
            laser.ramp = 0;
            break;

        case 4316:
            stepLaser();
            break;

        case 4317:
            taskType = NO_ODOR_CATCH_TRIAL_TASK;
            laser.ramp = 500;
            splash("No Odor Catch", "");

            laserSessionType = LASER_LR_EVERYTRIAL;
            zxLaserSessions(5, 6, laserRampDuringDelay, 12u, 24u, 20, 0.05, 20, setSessionNum());
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
            _delayT delay = setDelay();
            laserSessionType = LASER_LR_EVERYTRIAL;
            zxLaserSessions(5, 6, laserRampDuringDelay, delay, delay * 2u, 20u, 0.05, 20, setSessionNum());
            break;
        }

        case 4320:
            splash("12s Each Quarter", "Delay LR Laser");
            laserSessionType = LASER_12s_LR_EACH_QUARTER;
            laser.ramp = 500;
            zxLaserSessions(5, 6, laserDuringDelay, 12u, 24u, 20, 0.05, 25, setSessionNum());
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
            //            zxLaserSessions(type,type+1,m, 0.2, 5u, 20, 0.05, 20, setSessionNum());
            break;
        case 4322:
            variableVoltage();
            break;

        case 4323:
            splash("DNMS LR TASK", "Ramp Laser ++");
            protectedSerialSend(PERM_INFO, DMS_LR_Teach_LON);
            taskType = DNMS_2AFC_TEACH;
            laserSessionType = LASER_EVERY_TRIAL;
            laser.ramp = 500;
            zxLaserSessions(5, 6, laserRampDuringDelay, 5u, 10u, 20, 0.05, 30, setSessionNum());
            break;

        case 4324:
            splash("DNMS LR TASK", "Ramp Laser +-");
            protectedSerialSend(PERM_INFO, DMS_LR_Teach_ONOFF);
            taskType = DNMS_2AFC_TEACH;
            laserSessionType = LASER_OTHER_TRIAL;
            laser.ramp = 500;
            zxLaserSessions(5, 6, laserRampDuringDelay, 5u, 10u, 20, 0.05, 30, setSessionNum());
            break;

        case 4325:
            splash("BaseLine RQ", "Control");
            laserSessionType = LASER_OTHER_TRIAL;
            zxLaserSessions(5, 6, laserDuring3Baseline, 5u, 10u, 20, 0.05, 20, setSessionNum());
            break;

        case 4326:
            taskType = DNMS_TASK;
            laser.ramp = 500;
            splash("Incongrument", "LR Laser");
            laserSessionType = LASER_INCONGRUENT_CATCH_TRIAL;
            zxLaserSessions(5, 6, laserRampDuringDelay, 12u, 24u, 20, 0.05, 20, setSessionNum());
            break;

        case 4327:
            splash("DNMS LR TEACH", "No Laser");
            protectedSerialSend(PERM_INFO, DMS_LR_Teach_LOFF);
            taskType = DNMS_2AFC_TEACH;
            laserSessionType = LASER_NO_TRIAL;
            zxLaserSessions(5, 6, laserRampDuringDelay, 5u, 10u, 20, 0.05, 30, setSessionNum());
            break;

        case 4328:
            splash("MW/OD LR TEACH", "No Laser");
            protectedSerialSend(PERM_INFO, MSWOD_LR_Teach_LOFF);
            taskType = DNMS_2AFC_TEACH;
            laserSessionType = LASER_NO_TRIAL;
            zxLaserSessions(5, 6, laserRampDuringDelay, 0u, 10u, 20, 0.05, 30, setSessionNum());
            break;

        case 4329:
            splash("LR DNMS 5s Delay", "Laser + -");
            protectedSerialSend(PERM_INFO, DMS_LR_5Delay_Laser);
            taskType = DNMS_2AFC_TEACH;
            laserSessionType = LASER_OTHER_TRIAL;
            laser.ramp = 500;
            zxLaserSessions(5, 6, laserRampDuringDelay, 5u, 10u, 20, 0.05, 30, setSessionNum());
            break;

        case 4330:
            splash("LR DNMS 8s Delay", "Laser + -");
            protectedSerialSend(PERM_INFO, DMS_LR_8Delay_Laser);
            taskType = DNMS_2AFC_TEACH;
            laserSessionType = LASER_OTHER_TRIAL;
            laser.ramp = 500;
            zxLaserSessions(5, 6, laserRampDuringDelay, 8u, 16u, 20, 0.05, 30, setSessionNum());
            break;


        case 4331:
        {
            splash("Delay+Odor2", "Control");
            laserSessionType = LASER_OTHER_TRIAL;
            int type = setType();
            zxLaserSessions(type, type + 1, laserDuringDelay_Odor2, 5u, 10u, 20, 0.05, 20, setSessionNum());
            break;
        }
        case 4332:
            splash("Odor", "Control");
            laserSessionType = LASER_OTHER_TRIAL;
            unsigned int m = getFuncNumber(1, "1st 2nd BothOdor");
            int type = setType();
            zxLaserSessions(type, type + 1, m == 1 ? laserDuring1stOdor : m == 2 ? laserDuring2ndOdor : laserDuringOdor, 5u, 10u, 20, 0.05, 50, setSessionNum());
            break;
        case 4333:
            test_Laser();
            break;

        case 4334:
            splash("LR DNMS 12s Delay", "Laser + -");
            protectedSerialSend(PERM_INFO, DMS_LR_12Delay_Laser);
            taskType = DNMS_2AFC_TEACH;
            laserSessionType = LASER_OTHER_TRIAL;
            laser.ramp = 500;
            zxLaserSessions(5, 6, laserRampDuringDelay, 12u, 24u, 20, 0.05, 30, setSessionNum());
            break;


        case 4335:
            splash("LR DNMS 1st Odor", "Laser + -");
            protectedSerialSend(PERM_INFO, DMS_LR_1Odor_Laser);
            taskType = DNMS_2AFC_TEACH;
            laserSessionType = LASER_OTHER_TRIAL;
            zxLaserSessions(5, 6, laserDuring1stOdor, 5u, 10u, 20, 0.05, 30, setSessionNum());
            break;

        case 4336:
            splash("LR DNMS 2nd Odor", "Laser + -");
            protectedSerialSend(PERM_INFO, DMS_LR_2Odor_Laser);
            taskType = DNMS_2AFC_TEACH;
            laserSessionType = LASER_OTHER_TRIAL;
            zxLaserSessions(5, 6, laserDuring2ndOdor, 5u, 10u, 20, 0.05, 30, setSessionNum());
            break;

        case 4337:
            splash("LR DNMS 1+2 Odor", "Laser + -");
            protectedSerialSend(PERM_INFO, DMS_LR_bothOdor_Laser);
            taskType = DNMS_2AFC_TEACH;
            laserSessionType = LASER_OTHER_TRIAL;
            zxLaserSessions(5, 6, laserDuringOdor, 5u, 10u, 20, 0.05, 30, setSessionNum());
            break;

        case 4338:
            splash("LR DNMS Baseline", "Laser + -");
            protectedSerialSend(PERM_INFO, DMS_LR_baseline_Laser);
            taskType = DNMS_2AFC_TEACH;
            laserSessionType = LASER_OTHER_TRIAL;
            zxLaserSessions(5, 6, laserDuringBaseline, 5u, 10u, 20, 0.05, 30, setSessionNum());
            break;

        case 4339:
            splash("LR DNMS Response", "Laser + -");
            protectedSerialSend(PERM_INFO, DMS_LR_response_Laser);
            taskType = DNMS_2AFC_TEACH;
            laserSessionType = LASER_OTHER_TRIAL;
            zxLaserSessions(5, 6, laserDuringResponseDelay, 5u, 10u, 20, 0.05, 30, setSessionNum());
            break;

        case 4340:
        {
            taskType = DNMS_TASK;
            laser.ramp = 500;
            splash("LR LASER DNMS", "Sufficiency");
            _delayT delay = setDelay();
            laserSessionType = LASER_LR_EVERY_OTHER_TRIAL;
            zxLaserSessions(5, 6, laserRampDuringDelay, delay, delay * 2u, 20u, 0.05, 20, setSessionNum());
            break;
        }

            //        case 4334:
            //            splash("DNMS Shaping", "");
            //            laserTrialType = _LASER_NO_TRIAL;
            //            taskType = _SHAPING_TASK;
            //            zxLaserSessions(5,6, laserDuringDelay, 4u, 8u, 20, 0.5, 50, setSessionNum());
            //            break;

            //        case 4335:
            //        {
            //            splash("DNMS 4s", "DC laser,");
            //            taskType = _DNMS_TASK;
            //            zxLaserSessions(5,6, laserDuringDelay, 4u, 8u, 20, 0.5, 50, setSessionNum());
            //            break;
            //        }

        case 4341:
            splash("DNMS 5s Shaping", "RQ");
            laserSessionType = LASER_NO_TRIAL;
            taskType = SHAPING_TASK;
            zxLaserSessions(5, 6, laserDuringDelayChR2, 5u, 10u, 20, 0.05, 50, setSessionNum());
            break;

            //        case 4342:
            //        {
            //            splash("DNMS 5s Water+", "RQ DC laser");
            //            laserTrialType = _LASER_EVERY_TRIAL;
            //            taskType = _DNMS_TASK;
            //            zxLaserSessions(5,6, laserDuringDelayChR2, 5u, 10u, 20, 0.1, 50, setSessionNum());
            //            break;
            //        }
            //        case 4343:
            //        {
            //            splash("DNMS 5s ++", "RQ DC laser");
            //            laserSessionType = LASER_EVERY_TRIAL;
            //            taskType = DNMS_TASK;
            //            zxLaserSessions(5, 6, laserDuringDelayChR2, 5u, 10u, 20, 0.05, 50, setSessionNum());
            //            break;
            //        }
            //        case 4344:
            //        {
            //            splash("DNMS 5s", "RQ NoLaser");
            //            laserSessionType = LASER_NO_TRIAL;
            //            taskType = DNMS_TASK;
            //            zxLaserSessions(5, 6, laserDuringDelayChR2, 5u, 10u, 20, 0.05, 50, setSessionNum());
            //            break;
            //        }
            //
            //        case 4345:
            //        {
            //            splash("DNMS 8s ++", "RQ DC laser");
            //            laserSessionType = LASER_EVERY_TRIAL;
            //            taskType = DNMS_TASK;
            //            zxLaserSessions(5, 6, laserDuringDelayChR2, 8u, 16u, 20, 0.05, 50, setSessionNum());
            //            break;
            //        }


            //        case 4351:
            //            splash("Rule Match Shap", "C-B D-A");
            //            laserTrialType = _LASER_NO_TRIAL;
            //            taskType = _ASSOCIATE_SHAPING_TASK;
            //            zxLaserSessions(0, laserDuringDelayChR2, 5u, 10, 20u, 0.05, 50, setSessionNum());
            //            break;
            //
            //        case 4352:
            //            splash("Rule Match", "C-B D-A");
            //            laserTrialType = _LASER_NO_TRIAL;
            //            taskType = _ASSOCIATE_TASK;
            //            zxLaserSessions(0, laserDuringDelayChR2, 5u, 10, 20u, 0.05, 50, setSessionNum());
            //            break;

            //        case 4353:
            //        {
            //            splash("DNMS 5s +-", "RQ DC laser");
            //            laserSessionType = LASER_OTHER_TRIAL;
            //            taskType = DNMS_TASK;
            //            zxLaserSessions(5, 6, laserDuringDelayChR2, 5u, 10u, 20, 0.05, 50, setSessionNum());
            //            break;
            //        }

        case 4354:
            rampTweak(1, 0);
            break;

            //        case 4355:
            //        {
            //            splash("DNMS 8s +-", "RQ DC laser");
            //            laserSessionType = LASER_OTHER_TRIAL;
            //            taskType = DNMS_TASK;
            //            zxLaserSessions(5, 6, laserDuringDelayChR2, 8u, 16u, 20, 0.05, 50, setSessionNum());
            //            break;
            //        }


        case 4355:
            resetLaserPower();
            break;

        case 4356:
            rampTweak(0, 0);
            break;

        case 4357:
            rampTweak(0, 1);
            break;

        case 4358:
            flashLaser();
            break;

        case 4359:
            rampTweak(1, 1);
            break;

        case 4360:
        {
            splash("Dual Task", "Training");
            highLevelShuffleLength = 24;
            splash("Sample Odor", "");
            int sampleType = setType();
            splash("Test Odor", "");
            int testType = setType();
            taskType = DUAL_TASK_LEARNING;
            zxLaserSessions(sampleType, testType, laserOff, 13u, 13u, 24u, 0.05, 30, setSessionNum());
            break;
        }



        case 4361:
        {
            splash("Dual Task", "");
            highLevelShuffleLength = 24;
            splash("Sample Odor", "");
            int sampleType = setType();
            splash("Test Odor", "");
            int testType = setType();
            taskType = DUAL_TASK;
            laserSessionType = LASER_OTHER_TRIAL;
            laser.ramp = 500;
            zxLaserSessions(sampleType, testType, laserAfterDistractorMax, 13u, 13u, 24u, 0.05, 30, setSessionNum());
            break;
        }
        case 4362:
        {
            splash("Distractor|Laser", "On Off");
            highLevelShuffleLength = 24;
            splash("Sample Odor", "");
            int sampleType = setType();
            splash("Test Odor", "");
            int testType = setType();
            taskType = DUAL_TASK_ON_OFF_LASER_TASK;
            laserSessionType = LASER_DUAL_TASK_ON_OFF;
            laser.ramp = 500;
            zxLaserSessions(sampleType, testType, laserOff, 13u, 13u, 24u, 0.05, 30, setSessionNum());
            break;
        }
        case 4363:
        {
            splash("Distractor|ODAP", "On Off");
            highLevelShuffleLength = 24;
            splash("Sample Odor", "");
            int sampleType = setType();
            splash("Test Odor", "");
            int testType = setType();
            taskType = DUAL_TASK_ODAP_ON_OFF_LASER_TASK;
            laserSessionType = LASER_DUAL_TASK_ODAP_ON_OFF;
            laser.ramp = 500;
            zxLaserSessions(sampleType, testType, laserOff, 13u, 13u, 24u, 0.05, 30, setSessionNum());
            break;
        }

        case 4364:
        {
            splash("Dual Task", "Training Laser");
            highLevelShuffleLength = 24;
            splash("Sample Odor", "");
            int sampleType = setType();
            splash("Test Odor", "");
            int testType = setType();
            taskType = DUAL_TASK_LEARNING;
            laserSessionType = LASER_EVERY_TRIAL;
            laser.ramp = 500;
            zxLaserSessions(sampleType, testType, laserAfterDistractorMax, 13u, 13u, 24u, 0.05, 30, setSessionNum());
            break;
        }

        case 4365:
        {
            splash("Dual Task", "3x Distr");
            highLevelShuffleLength = 72;
            splash("Sample Odor", "");
            int sampleType = setType();
            splash("Test Odor", "");
            int testType = setType();
            taskType = DUAL_TASK_DISTRx3_TASK;
            laserSessionType = LASER_OTHER_TRIAL;
            laser.ramp = 500;
            zxLaserSessions(sampleType, testType, laserAfterMultiDistractor, 14u, 14u, 24u, 0.05, 30, setSessionNum());
            break;
        }
        case 4366:
        {
            splash("Dual Task", "");
            highLevelShuffleLength = 48;
            splash("Sample Odor", "");
            int sampleType = setType();
            splash("Test Odor", "");
            int testType = setType();
            taskType = DUAL_TASK_EVERY_TRIAL;
            laserSessionType = LASER_NO_TRIAL;
            zxLaserSessions(sampleType, testType, laserAfterDistractorMax, 13u, 13u, 48u, 0.05, 30, setSessionNum());
            break;
        }


        case 4367:
        {
            splash("Dual Task", "Block Laser");
            highLevelShuffleLength = 48;
            splash("Sample Odor", "");
            int sampleType = setType();
            splash("Test Odor", "");
            int testType = setType();
            taskType = DUAL_TASK_EVERY_TRIAL;
            laserSessionType = LASER_OTHER_BLOCK;
            laser.ramp = 500;
            int sesss = setSessionNum();
            psedoRanInput = getFuncNumber(1, "Rand = 0 or 1");
            zxLaserSessions(sampleType, testType, laserAfterDistractorMax, 13u, 13u, 48u, 0.05, 30, sesss);
            break;
        }

        case 4368:
        {
            splash("Dual Task Block", "Baseline");
            highLevelShuffleLength = 48;
            splash("Sample Odor", "");
            int sampleType = setType();
            splash("Test Odor", "");
            int testType = setType();
            taskType = DUAL_TASK_EVERY_TRIAL;
            laserSessionType = LASER_OTHER_BLOCK;
            laser.ramp = 500;
            int sesss = setSessionNum();
            psedoRanInput = getFuncNumber(1, "Rand = 0 or 1");
            zxLaserSessions(sampleType, testType, laserDuring4Baseline, 13u, 13u, 48u, 0.05, 30, sesss);
            break;
        }


        case 4369:
        {
            splash("Dual Task", "3x Distr BLOCK");
            highLevelShuffleLength = 72;
            splash("Sample Odor", "");
            int sampleType = setType();
            splash("Test Odor", "");
            int testType = setType();
            taskType = DUAL_TASK_DISTRx3_TASK;
            laserSessionType = LASER_OTHER_BLOCK;
            laser.ramp = 500;
            int sesss = setSessionNum();
            psedoRanInput = getFuncNumber(1, "Rand = 0 or 1");
            zxLaserSessions(sampleType, testType, laserAfterMultiDistractor, 14u, 14u, 48u, 0.05, 30, sesss);
            break;
        }

        case 4370:
        {
            splash("ODPA", "Shaping");
            laserSessionType = LASER_NO_TRIAL;
            taskType = ODPA_SHAPING_TASK;
            splash("Sample Odor", "");
            int sampleType = setType();
            splash("Test Odor", "");
            int testType = setType();

            zxLaserSessions(sampleType, testType, laserOff, 13u, 13u, 24u, 0.05, 30, setSessionNum());
            break;
        }

        case 4371:
        {
            splash("ODPA", "No Laser");
            laserSessionType = LASER_NO_TRIAL;
            taskType = ODPA_TASK;
            splash("Sample Odor", "");
            int sampleType = setType();
            splash("Test Odor", "");
            int testType = setType();
            zxLaserSessions(sampleType, testType, laserOff, 13u, 13u, 24u, 0.05, 30, setSessionNum());
            break;
        }

        case 4372:
        {
            splash("ODPA", "Laser");
            laserSessionType = LASER_OTHER_TRIAL;
            laser.ramp = 500;
            taskType = ODPA_TASK;
            splash("Sample Odor", "");
            int sampleType = setType();
            splash("Test Odor", "");
            int testType = setType();
            zxLaserSessions(sampleType, testType, laserDuringDelay, 13u, 13u, 24u, 0.05, 30, setSessionNum());
            break;
        }

        case 4373:
        {
            splash("ODPA Both Odor", "Laser");
            laserSessionType = LASER_OTHER_TRIAL;

            taskType = ODPA_TASK;
            splash("Sample Odor", "");
            int sampleType = setType();
            splash("Test Odor", "");
            int testType = setType();
            zxLaserSessions(sampleType, testType, laserDuringOdor, 5u, 10u, 20u, 0.05, 30, setSessionNum());
            break;
        }

        case 4374:
        {

            splash("ODPA", "Block Laser");
            highLevelShuffleLength = 48;
            laserSessionType = LASER_OTHER_BLOCK;
            laser.ramp = 500;
            taskType = ODPA_TASK;
            splash("Sample Odor", "");
            int sampleType = setType();
            splash("Test Odor", "");
            int testType = setType();
            int sesss = setSessionNum();
            psedoRanInput = getFuncNumber(1, "Rand = 0 or 1");
            zxLaserSessions(sampleType, testType, laserDuringDelay, 13u, 13u, 48u, 0.05, 30, sesss);
            break;
        }

        case 4380:
        {
            splash("ODPA", "Shaping");
            laserSessionType = LASER_NO_TRIAL;
            taskType = ODPA_SHAPING_TASK;
            splash("Sample Odor", "");
            int sampleType = setType();
            splash("Test Odor", "");
            int testType = setType();
            unsigned int delay = setDelay();
            zxLaserSessions(sampleType, testType, laserOff, delay, delay, 24u, 0.05, 30, setSessionNum());
            break;
        }

        case 4381:
        {
            splash("ODPA", "No Laser");
            laserSessionType = LASER_NO_TRIAL;
            taskType = ODPA_TASK;
            splash("Sample Odor", "");
            int sampleType = setType();
            splash("Test Odor", "");
            int testType = setType();
            unsigned int delay = setDelay();
            zxLaserSessions(sampleType, testType, laserOff, delay, delay, 24u, 0.05, 30, setSessionNum());
            break;
        }

        case 4382:
        {
            splash("ODPA", "Laser");
            laserSessionType = LASER_OTHER_TRIAL;
            laser.ramp = 500;
            taskType = ODPA_TASK;
            splash("Sample Odor", "");
            int sampleType = setType();
            splash("Test Odor", "");
            int testType = setType();
            unsigned int delay = setDelay();
            zxLaserSessions(sampleType, testType, laserDuringDelay, delay, delay, 24u, 0.05, 30, setSessionNum());
            break;
        }
        case 4383:
        {
            splash("DualTask-8s", "Training");
            highLevelShuffleLength = 24;
            splash("Sample Odor", "");
            int sampleType = setType();
            splash("Test Odor", "");
            int testType = setType();
            taskType = DUAL_TASK_LEARNING;
            zxLaserSessions(sampleType, testType, laserOff, 8u, 8u, 24u, 0.05, 30, setSessionNum());
            break;
        }



        case 4384:
        {
            splash("DualTask-8s", "No Laser");
            highLevelShuffleLength = 24;
            splash("Sample Odor", "");
            int sampleType = setType();
            splash("Test Odor", "");
            int testType = setType();
            taskType = DUAL_TASK;
            laserSessionType = LASER_NO_TRIAL;
            zxLaserSessions(sampleType, testType, laserOff, 8u, 8u, 24u, 0.05, 30, setSessionNum());
            break;
        }

        case 4390:
        {
            splash("ODPA", "Early Mid Late");
            laserSessionType = LASER_13s_EarlyMidLate;
            taskType = ODPA_TASK;
            splash("Sample Odor", "");
            int sampleType = setType();
            splash("Test Odor", "");
            int testType = setType();
            laser.ramp = 500;
            zxLaserSessions(sampleType, testType, laserOff, 13u, 13u, 24u, 0.05, 30, setSessionNum());
            break;
        }

        case 4399:
        {
            splash("DNMS Dual Task", "Learning Laser");
            highLevelShuffleLength = 24;
            splash("Odor", "");
            int odorType = setType();
            taskType = DNMS_DUAL_TASK_LEARNING;
            laserSessionType = LASER_OTHER_TRIAL;
            laser.ramp = 500;
            zxLaserSessions(odorType, odorType + 1, laserAfterDistractorMax, 13u, 26u, 24u, 0.05, 30, setSessionNum());
            break;
        }

        case 4400:
        {
            splash("DNMS Dual Task", "Training");
            highLevelShuffleLength = 24;
            splash("Odor", "");
            int odorType = setType();
            taskType = DNMS_DUAL_TASK_LEARNING;
            zxLaserSessions(odorType, odorType + 1, laserOff, 13u, 26u, 24u, 0.05, 30, setSessionNum());
            break;
        }

        case 4401:
        {
            splash("DNMS Dual Task", "Laser");
            highLevelShuffleLength = 24;
            splash("Odor", "");
            int odorType = setType();
            taskType = DNMS_DUAL_TASK;
            laserSessionType = LASER_OTHER_TRIAL;
            laser.ramp = 500;
            zxLaserSessions(odorType, odorType + 1, laserAfterDistractorMax, 13u, 26u, 24u, 0.05, 30, setSessionNum());
            break;
        }



        case 4410:
            write_eeprom(EEP_DUTY_LOW_R_OFFSET, 0x7f);
            write_eeprom(EEP_DUTY_LOW_L_OFFSET, 0x7f);
            break;



        case 4411:
            splash("During Delay", "");
            //            setLaser();
            zxLaserSessions(2, 3, laserDuringDelay, 4u, 8u, 20, 0.05, 50, setSessionNum());
            break;



        case 4412:
        {
            splash("Vary Delay", "Shaping");
            laserSessionType = LASER_NO_TRIAL;
            taskType = SHAPING_TASK;
            _delayT delay = setDelay();
            int type = setType();
            zxLaserSessions(type, type + 1, laserDuringDelay, delay, delay * 2u, 20u, 0.05, 50, setSessionNum());
            break;
        }

            //
            //        case 4413:
            //        {
            //            splash("DNMS LR Distra.", "Laser EveryTrial");
            //            taskType = DNMS_2AFC_TASK;
            //            laserTrialType = LASER_EVERY_TRIAL;
            //            zxLaserSessions(5, 6, laserDelayDistractor, 5u, 10u, 20, 0.05, 200, setSessionNum());
            //            break;
            //        }
            //        case 4414:
            //        {
            //            splash("Varying Delay", "DC laser,");
            //            taskType = DNMS_TASK;
            //            zxLaserSessions(5, 6, laserDuringDelay, 4u, 8u, 20, 0.125, 50, setSessionNum());
            //            break;
            //        }
        case 4415:
        {
            splash("Varying Delay", "DC laser,");
            _delayT delay = setDelay();
            taskType = DNMS_TASK;
            laserSessionType = setLaser();
            int type = setType();
            zxLaserSessions(type, type + 1, laserDuringDelayChR2, delay, delay * 2u, 20u, 0.05, 50, setSessionNum());
            break;
        }


            //        case 4416:
            //        {
            //            splash("Varying Delay", "DC laser,");
            //            _delayT delay = setDelay();
            //            taskType = DNMS_TASK;
            //            laserSessionType = setLaser();
            //            zxLaserSessions(5, 6, laserDuringDelayChR2, delay, delay * 2u, 20u, 0.036, 50, setSessionNum());
            //            break;
            //        }


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
            laserSessionType = LASER_NO_TRIAL;
            taskType = SHAPING_TASK;
            int type = setType();
            zxLaserSessions(type, type + 1, laserDuringDelay, 4u, 8u, 20, 0.05, 50, setSessionNum());
            break;
        }
            //        case 4422:
            //        {
            //            splash("DNMS Shaping", "Long Water");
            //            laserSessionType = LASER_NO_TRIAL;
            //            taskType = SHAPING_TASK;
            //            int type = setType();
            //            zxLaserSessions(type, type + 1, laserDuringDelay, 4u, 8u, 20, 0.1, 50, 40);
            //            break;
            //        }
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
            zxLaserSessions(type, type + 1, laserDuringDelay, 4u, 8u, 20, 0.05, 60, setSessionNum());
            break;
        }
            //        case 4426:
            //            shiftingLaser();
            //            break;

        case 4431:
        {
            splash("VarDelay LR", "DC laser");
            _delayT delay = setDelay();
            taskType = DNMS_2AFC_TASK;
            int type = setType();
            zxLaserSessions(type, type + 1, laserDuringDelay, delay, delay * 2u, 20u, 0.05, 50, setSessionNum());
            break;
        }
        case 4432:
        {
            taskType = GONOGO_LR_TASK;
            splash("GoNogo LR", "");
            unsigned int m = getFuncNumber(1, "No Pre Odor Post");
            int laserPeriod;
            laserPeriod = laserDuring4Baseline;
            switch (m) {
                case 1:
                    taskType = GONOGO_2AFC_TEACH;
                    laserSessionType = LASER_NO_TRIAL;
                    break;
                case 3:
                    laserPeriod = laserDuring1stOdor;
                    break;
                case 4:
                    laserPeriod = laserDuringResponseDelay;
                    break;
            }
            int type = setType();
            zxLaserSessions(type, type + 1, laserPeriod, 0u, 5u, 20, 0.05, 50, setSessionNum());
            break;
        }

        case 4433:
        {
            splash("DNMS LR", "Laser EveryTrial");
            taskType = DNMS_2AFC_TASK;
            laserSessionType = LASER_EVERY_TRIAL;
            zxLaserSessions(5, 6, laserDuringDelayChR2, 5u, 10u, 20, 0.05, 200, setSessionNum());
            break;
        }

        case 4434:
            laserTrain();
            break;

        case 4435:
        {
            splash("Baseline LR", "Laser EveryTrial");
            taskType = DNMS_2AFC_TASK;
            zxLaserSessions(5, 6, laserDuring3Baseline, 5u, 10u, 20, 0.05, 200, setSessionNum());
            break;
        }

            //        case 4436:
            //        {
            //            splash("DNMS LR", "Laser EveryTrial");
            //            taskType = _DNMS_LR_TASK;
            //            laserTrialType = _LASER_EVERY_TRIAL;
            //            zxLaserSessions(5,6, laserDelayDistractor, 5u, 10u, 20, 0.05, 200, setSessionNum());
            //            break;
            //        }

        case 4441:
        {
            splash("First Odor LR", "Laser EveryTrial");
            taskType = DNMS_2AFC_TASK;
            zxLaserSessions(5, 6, laserDuring1stOdor, 5u, 10u, 20, 0.05, 200, setSessionNum());
            break;
        }
        case 4442:
        {
            splash("Second Odor LR", "Laser EveryTrial");
            taskType = DNMS_2AFC_TASK;
            zxLaserSessions(5, 6, laserDuring2ndOdor, 5u, 10u, 20, 0.05, 200, setSessionNum());
            break;
        }
        case 4443:
        {
            splash("Response Delay LR", "Laser EveryTrial");
            taskType = DNMS_2AFC_TASK;
            zxLaserSessions(5, 6, laserDuringResponseDelay, 5u, 10u, 20, 0.05, 200, setSessionNum());
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

        case 4450:
            varifyOpticalSuppression();

    }
}


