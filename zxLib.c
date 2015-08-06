#include "commons.h"

#include "zxLib.h"

//DEBUG

const char _DEBUGGING = 0;

char* zxVer = "z5806";
float odorLength = 1.0;

unsigned int laserTimer = 0u;
unsigned int laserOnTime = 65535u;
unsigned int laserOffTime = 0u;
unsigned int ramp = 0u;
unsigned int ramping = 0u;
unsigned char pwmDutyHiR = 0xfe;
unsigned char pwmDutyHiL = 0xfe;
unsigned char pwmDutyLoR = 0u;
unsigned char pwmDutyLoL = 0u;
unsigned int laserTimerOn = 0u;
unsigned int licking = 0u;
const char odorTypes[] = {' ', 'W', 'B', 'J', 'w', 'R', 'Q', 'r', 'q'};
unsigned int timeFilter = 0u;
unsigned int laserTrialType = _LASER_EVERY_TRIAL;
unsigned int taskType = _DNMS_TASK;
unsigned int lickFlag = 0u;
unsigned int fullduty = 0xfe;
unsigned int lickLCount = 0u;
unsigned int lickRCount = 0u;
unsigned int wait_Trial = 1u;
unsigned int timeSum = 0u;
unsigned int laserSide = 3u;

unsigned int highLevelShuffleLength = 12u;

_prog_addressT EE_Addr = 0x7ff000;

void initZXTMR(void) {

    //Stop general timer
    IEC0bits.T2IE = 0;
    IFS0bits.T2IF = 0;
    timerCounterI = 0;
    timerCounterJ = 0;
    //

    TMR1 = 0;
    PR1 = TMR_RATE / 200;
    IFS0bits.T1IF = 0;
    IEC0bits.T1IE = 1;
    T1CON = 0x8020;
    ConfigIntTimer1(T1_INT_PRIOR_5 & T1_INT_ON);

    lickLCount = 0;
    lickRCount = 0;
}

int filtered(void) {
    return (timerCounterJ < timeFilter || timerCounterJ > timeFilter + 50u);
}

void tick(int i) {
    laserTimer += i;
    timerCounterI += i;
    if (timerCounterJ == 65535) {
        timerCounterJ = 0;
    } else {
        timerCounterJ += i;
    }
}

void resetTimerCounterJ() {
    timerCounterJ = 65535;
    timeSum = 0;
    while (timerCounterJ > 1);
}

void waitJ(unsigned int dT) {
    timeSum += dT;
    while (timerCounterJ < timeSum);
}

int likeOdorA(int odor) {
    if (odor == 2 || odor == 5 || odor == 7) return 1;
    return 0;
}

void __attribute__((__interrupt__, no_auto_psv)) _T1Interrupt(void) {
    IFS0bits.T1IF = 0;
    tick(5);


    if (laserTimerOn && (laserTimer % (laserOnTime + laserOffTime) < laserOnTime)) {
        //    if (laserTimerOn) {
        if (laserTimerOn % 2) {
            Out4 = 1;
            Nop();
            Nop();
            PDC2 = pwmDutyLoL;
        }

        if ((laserTimerOn >> 1) % 2) {
            Out5 = 1;
            Nop();
            Nop();
            PDC4 = pwmDutyLoR;
        }
        ramping = ramp;


    } else {
        Out4 = 0;
        __builtin_nop();
        __builtin_nop();
        Out5 = 0;
        if (ramping >= 5) {
            ramping -= 5;
            if (PDC2 != fullduty)
                PDC2 = (pwmDutyHiL - pwmDutyLoL) * (1 - ((float) ramping / ramp)) + pwmDutyLoL;
            if (PDC4 != fullduty)
                PDC4 = (pwmDutyHiR - pwmDutyLoR) * (1 - ((float) ramping / ramp)) + pwmDutyLoR;
        } else {
            PDC2 = fullduty;
            Nop();
            Nop();
            PDC4 = fullduty;
        }

    }
    if (_LICK_LEFT && _LICK_RIGHT) {
        Out1 = 1;
        Nop();
        Nop();
        Out6 = 1;
    } else if (_LICK_LEFT && !licking && filtered()) {
        timeFilter = timerCounterJ;
        licking = _LICKING_LEFT;
        Out1 = 1;
        lickLCount++;
        Nop();
        localSendOnce(SpLick, taskType < 35 ? 1 : 2);
        //        tick(3);
    } else if (_LICK_RIGHT && !licking && filtered()) {
        timeFilter = timerCounterJ;
        licking = _LICKING_RIGHT;
        Out6 = 1;
        lickRCount++;
        Nop();
        localSendOnce(SpLick, 3);
        //        tick(3);
    } else if (licking && !_LICK_ANY) {

        licking = 0;
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

int setType(void) {

    int ports[] = {0, 0, 3, 5, 0, 0};
    return ports[getFuncNumber(1, "P2P3 P5P6 P7P8")];
}

int setSessionNum() {

    int sess[] = {20, 1, 5, 10, 15, 20, 30};
    return sess[getFuncNumber(1, "1 5 10 15 20 30")];
}

int setDelay(int type) {
    if (type == 1) {
        int d[] = {5, 4, 5, 8, 12};
        int n = getFuncNumber(1, "4s 5s 8s 12s");
        if (n > 0 && n < 5)
            return d[n];
        else return 5;
    }
    int n = getFuncNumber(1, "4 8 11 14 17");
    int d[] = {5, 4, 8, 11, 14, 17};
    return d[n];
}

int setLaser() {
    int d[] = {_LASER_NO_TRIAL, _LASER_NO_TRIAL, _LASER_OTHER_TRIAL, _LASER_EVERY_TRIAL};
    int n = getFuncNumber(1, "None 1+1-  Every");
    if (n > 0 && n < 4)
        return d[n];
    else return _LASER_NO_TRIAL;
}

//void setLaser(void) {
//    int n = getFuncNumber(1, "DC=1 20Hz=2");
//    switch (n) {
//        case 2:
//            laserOnTime = 20;
//            laserOffTime = 30;
//            break;
//        default:
//            laserOnTime = 65535;
//            laserOffTime = 0;
//    }
//}

void protectedSerialSend(int type, int value) {
    if (_DEBUGGING) return; //DEBUG
    IEC0bits.T1IE = 0;
    IEC0bits.T3IE = 0;
    unsigned int i;
    for (i = 0; i < 4000; i++); //1000 per ms
    localSendOnce(type, value);
    for (i = 0; i < 3000; i++); //1000 per ms
    tick(10);
    IEC0bits.T1IE = 1;
    IEC0bits.T3IE = 1;
}

void turnOnLaser(unsigned int i) {
    laserTimer = 0;
    laserTimerOn = i;

    if (i % 2) {
        Out4 = 1;
        Nop();
        Nop();
        PDC2 = pwmDutyLoL;
    }

    if ((i >> 1) % 2) {
        Out5 = 1;
        Nop();
        Nop();
        PDC4 = pwmDutyLoR;
    }


    lcdWriteChar('L', 4, 1);
    protectedSerialSend(SpLaserSwitch, i);
}

void turnOffLaser() {
    laserTimerOn = 0;
    Out4 = 0;
    Nop();
    Nop();
    Out5 = 0;
    if (ramping == 0) {
        PDC2 = fullduty;
        Nop();
        Nop();
        PDC4 = fullduty;
    }
    lcdWriteChar('.', 4, 1);
    protectedSerialSend(SpLaserSwitch, 0);
}

void assertLaser(int type, int step, int currentTrial) {
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
                    turnOnLaser(3);
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
                //            case laserDuring1Terice:
                //                if (step == atDelayBegin) {
                //                    turnOnLaser(3);
                //                } else if (step == atDelay2SecIn) {
                //                    turnOffLaser();
                //                }
                //                break;
                //            case laserDuring2Terice:
                //                if (step == atDelay2SecIn) {
                //                    turnOnLaser(3);
                //                } else if (step == atDelayLast2SecBegin) {
                //                    turnOffLaser();
                //                }
                //                break;
                //            case laserDuring3Terice:
                //                if (step == atDelayLast2SecBegin) {
                //                    turnOnLaser(3);
                //                } else if (step == atSecondOdorBeginning) {
                //                    turnOffLaser();
                //                }
                //                break;
            case laserDuring1Quarter:
                if (step == atDelayBegin) {
                    turnOnLaser(laserSide);
                } else if (step == atDelay1_5SecIn) {
                    turnOffLaser();
                }
                break;
            case laserDuring2Quarter:
                if (step == atDelay2SecIn) {
                    turnOnLaser(laserSide);
                } else if (step == atDelay_5ToMiddle) {
                    turnOffLaser();
                }
                break;
            case laserDuring3Quarter:
                if (step == atDelayMiddle) {
                    turnOnLaser(laserSide);
                } else if (step == atDelayLast2_5SecBegin) {
                    turnOffLaser();
                }
                break;
            case laserDuring4Quarter:
                if (step == atDelayLast2SecBegin) {
                    turnOnLaser(laserSide);
                } else if (step == atDelayLast500mSBegin) {
                    turnOffLaser();
                }
                break;
            case laserDuring12s1Quarter:
                if (step == atDelayBegin) {
                    turnOnLaser(laserSide);
                } else if (step == atDelay2_5SecIn) {
                    turnOffLaser();
                }
                break;
            case laserDuring12s2Quarter:
                if (step == atDelay3SecIn) {
                    turnOnLaser(laserSide);
                } else if (step == atDelay_5ToMiddle) {
                    turnOffLaser();
                }
                break;
            case laserDuring12s3Quarter:
                if (step == atDelayMiddle) {
                    turnOnLaser(laserSide);
                } else if (step == atDelay8_5SecIn) {
                    turnOffLaser();
                }
                break;
            case laserDuring12s4Quarter:
                if (step == atDelay9SecIn) {
                    turnOnLaser(laserSide);
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
                    turnOnLaser(laserSide);
                } else if (step == atDelayLast500mSBegin) {
                    turnOffLaser();
                }
                break;
        }
    }
}

void shuffleArray(unsigned int orgArray[], unsigned int arraySize) {
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

void lcdWriteString(char* s) {

    int iter;
    for (iter = 0; s[iter] > 0 && iter < 16; iter++) {

        lcd_data(s[iter]);
    }
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

void processHit(float waterPeroid, int valve, int id) {
    protectedSerialSend(22, valve == 1 ? 1 : 2);
    Valve_ON(valve, fullduty);
    wait_ms(waterPeroid * 1000);
    Valve_OFF(valve);
    currentMiss = 0;
    protectedSerialSend(SpHit, id);
    lcdWriteNumber(++hit, 3, 6, 1);
}

void processFalse(int id) {
    currentMiss = 0;
    protectedSerialSend(SpFalseAlarm, id);
    lcdWriteNumber(++falseAlarm, 3, 6, 2);
}

void processMiss(int id) {
    currentMiss++;
    protectedSerialSend(SpMiss, id);
    lcdWriteNumber(++miss, 3, 10, 1);
}

void processLRTeaching(float waterPeroid, int LR) {
    int rr = rand() % 3;
    if (rr == 0) {
        lcdWriteChar(rr + 0x30, 4, 1);
        protectedSerialSend(22, LR == 2 ? 1 : 2);
        //        FreqL = lickLCount;
        Valve_ON(LR == 2 ? 1 : 3, fullduty);
        wait_ms(waterPeroid * 1000);
        Valve_OFF(LR == 2 ? 1 : 3);
        //        currentMiss = 0;
        protectedSerialSend(SpWater_sweet, LR);
    }
}

void waterNResult(int firstOdor, int secondOdor, float waterPeroid) {
    lickFlag = 0;
    switch (taskType) {
            /*
             *DNMS
             */
        case _DNMS_TASK:
        case _SHAPPING_TASK:
        case _NO_ODOR_CATCH_TRIAL_TASK:

            //        case _ASSOCIATE_TASK:
            //        case _ASSOCIATE_SHAPPING_TASK:

            ///////////Detect/////////////////
            for (timerCounterI = 0; timerCounterI < 500 && !lickFlag; lickFlag = licking);

            /////Reward
            if (!lickFlag) {
                if (likeOdorA(firstOdor) == likeOdorA(secondOdor)) {
                    protectedSerialSend(SpCorrectRejection, 1);
                    lcdWriteNumber(++correctRejection, 3, 10, 2);
                } else {
                    processMiss(1);
                    if ((taskType == _SHAPPING_TASK /*|| taskType == _ASSOCIATE_SHAPPING_TASK*/) && ((rand() % 3) == 0)) {
                        protectedSerialSend(22, 1);
                        Valve_ON(water_sweet, fullduty);
                        protectedSerialSend(SpWater_sweet, 1);
                        wait_ms(waterPeroid * 1000);
                        Valve_OFF(water_sweet);
                    }
                }
            } else if (likeOdorA(firstOdor) == likeOdorA(secondOdor)) {
                processFalse(1);
            } else {
                processHit(waterPeroid, 1, 1);
            }
            break;

            /*
             *DNMS-LR
             */
        case _DNMS_LR_TASK:
            //
            ///////////Detect/////////////////
            for (timerCounterI = 0; timerCounterI < 500 && !lickFlag; lickFlag = licking);
            /////Reward
            if (!lickFlag) {
                processMiss((firstOdor != secondOdor) ? 2 : 3);
            } else if (!(lickFlag & 1) != !(firstOdor^secondOdor)) {
                processHit(waterPeroid, lickFlag & 1 ? 3 : 1, lickFlag);
            } else {
                processFalse(lickFlag == _LICKING_LEFT ? 2 : 3);
            }
            break;

        case _DNMS_LR_TEACH:
            //
            ///////////Detect/////////////////
            for (timerCounterI = 0; timerCounterI < 500 && !lickFlag; lickFlag = licking);
            /////Reward
            if (!lickFlag) {
                processMiss((firstOdor != secondOdor) ? 2 : 3);
                processLRTeaching(waterPeroid, (firstOdor != secondOdor) ? 2 : 3);
            } else if (!(lickFlag & 1) != !(firstOdor^secondOdor)) {
                processHit(waterPeroid, lickFlag & 1 ? 3 : 1, lickFlag);
            } else {
                processFalse(lickFlag == _LICKING_LEFT ? 2 : 3);
                processLRTeaching(waterPeroid, (firstOdor != secondOdor) ? 2 : 3);
            }
            break;

        case _GONOGO_LR_TASK:
        case _GONOGO_LR_TEACH:
            //
            ///////////Detect/////////////////
            for (timerCounterI = 0; timerCounterI < 500 && !lickFlag; lickFlag = licking);

            /////Reward
            if (lickFlag == _LICKING_LEFT && likeOdorA(firstOdor)) {
                processHit(waterPeroid, 1, 2);
            } else if (lickFlag == _LICKING_RIGHT && !likeOdorA(firstOdor)) {
                processHit(waterPeroid, 3, 3);
            } else if (lickFlag) {
                processFalse(lickFlag == _LICKING_LEFT ? 2 : 3);
            } else {
                processMiss(likeOdorA(firstOdor) ? 2 : 3);
            }
            break;

        case _GONOGO_TASK:

            for (timerCounterI = 0; timerCounterI < 500 && !lickFlag; lickFlag = licking);

            /////Reward
            if (!lickFlag) {
                if (!likeOdorA(firstOdor)) {
                    protectedSerialSend(SpCorrectRejection, 1);
                    lcdWriteNumber(++correctRejection, 3, 10, 2);
                } else {
                    processMiss(1);
                }
            } else if (!likeOdorA(firstOdor)) {
                processFalse(1);
            } else {
                processHit(waterPeroid, 1, 1);
            }
            break;
    }
}

void distractor(int distractOdor) {
    Valve_ON(distractOdor, fullduty);
    protectedSerialSend(SpOdor_C, distractOdor);
    lcdWriteChar('d', 4, 1);
    wait_ms(odorLength * 1000 - 10);
    Valve_OFF(distractOdor);
    lcdWriteChar('D', 4, 1);
}

void waitTrial() {
    if (!wait_Trial) {
        return;
    }

    while (u2Received != 0x31) {
        protectedSerialSend(20, 1);
    }
    u2Received = -1;
}

void zxLaserSessions(int odorType, int laserType, _delayT delay, int ITI, int trialsPerSession, float WaterLen, int missLimit, int totalSession, float delay_before_reward) {
    //    wait_ms(1000);
    int currentTrial = 0;
    int currentSession = 0;

    protectedSerialSend(SpWater_bitter, pwmDutyLoL);
    protectedSerialSend(SpValve8, pwmDutyLoR);
    protectedSerialSend(SpStepN, taskType);

    while ((currentMiss < missLimit) && (currentSession++ < totalSession)) {
        //        protectedSerialSend(SpOdorDelay, delay);
        protectedSerialSend(SpSess, 1);

        splash("    H___M___ __%", "S__ F___C___t___");

        lcdWriteNumber(currentSession, 2, 2, 2);
        hit = miss = falseAlarm = correctRejection = 0;
        unsigned int lastHit = 0;
        unsigned int shuffledList[4];
        unsigned int shuffledDelays[highLevelShuffleLength];
        shuffleArray(shuffledDelays, highLevelShuffleLength);
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

                    case _DNMS_TASK:
                    case _DNMS_LR_TASK:
                        firstOdor = (index == 0 || index == 2) ? odor_A : odor_B;
                        secondOdor = (index == 1 || index == 2) ? odor_A : odor_B;
                        break;
                    case _SHAPPING_TASK:
                        firstOdor = (index == 0 || index == 2) ? odor_A : odor_B;
                        secondOdor = (firstOdor == odor_A) ? odor_B : odor_A;
                        break;
                    case _GONOGO_TASK:
                    case _GONOGO_LR_TASK:
                        firstOdor = (index == 0 || index == 2) ? odor_A : odor_B;
                        secondOdor = -odorType;
                        break;
                    case _DNMS_LR_TEACH:
                        if (hit > lastHit || currentTrial == 0) {
                            firstOdor = (index == 0 || index == 2) ? odor_A : odor_B;
                            secondOdor = (index == 1 || index == 2) ? odor_A : odor_B;
                            lastOdor1 = firstOdor;
                            lastOdor2 = secondOdor;
                        } else {
                            firstOdor = lastOdor1;
                            secondOdor = lastOdor2;
                        }

                        lastHit = hit;
                        break;
                    case _GONOGO_LR_TEACH:
                        if (hit > lastHit || currentTrial == 0) {
                            firstOdor = (index == 0 || index == 2) ? odor_A : odor_B;
                            lastOdor1 = firstOdor;
                            secondOdor = -odorType;
                        } else {
                            firstOdor = lastOdor1;
                            secondOdor = -odorType;
                        }

                        lastHit = hit;
                        break;
                    case _NO_ODOR_CATCH_TRIAL_TASK:
                        firstOdor = (index == 0 || index == 2) ? odor_A : odor_B;
                        secondOdor = (index == 1 || index == 2) ? odor_A : odor_B;
                        if ((currentTrial > 3 && currentTrial < 8 && firstOdor == odor_A && secondOdor == odor_A)
                                || (currentTrial > 7 && currentTrial < 12 && firstOdor == odor_A && secondOdor == odor_B)
                                || (currentTrial > 11 && currentTrial < 16 && firstOdor == odor_B && secondOdor == odor_A)
                                || (currentTrial > 15 && currentTrial < 20 && firstOdor == odor_B && secondOdor == odor_B)) {
                            firstOdor -= odorType;
                        } else {
                        }
                        break;

                }
                firstOdor += odorType;
                secondOdor += odorType;

                lcdWriteChar(odorTypes[firstOdor], 1, 1);
                lcdWriteChar(odorTypes[secondOdor], 2, 1);

                int laserCurrentTrial;
                switch (laserTrialType) {
                    case _LASER_NO_TRIAL:
                        laserCurrentTrial = 0;
                        break;
                    case _LASER_EVERY_TRIAL:
                        laserCurrentTrial = 1;
                        break;
                    case _LASER_OTHER_TRIAL:
                        laserCurrentTrial = currentTrial % 2;
                        break;

                    case _LASER_LR_EACH_QUARTER:
                        laserSide = likeOdorA(firstOdor) ? 1 : 2;
                    case _LASER_EACH_QUARTER:
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


                    case _LASER_12s_LR_EACH_QUARTER:
                        laserSide = likeOdorA(firstOdor) ? 1 : 2;
                    case _LASER_12s_EACH_QUARTER:
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


                    case _LASER_VARY_LENGTH:
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





                    case _LASER_LR_EVERYTRIAL:
                        laserSide = likeOdorA(firstOdor) ? 1 : 2;
                        laserCurrentTrial = 1;
                        break;




                    case _LASER_INCONGRUENT_CATCH_TRIAL:
                        if ((currentTrial > 3 && currentTrial < 8 && likeOdorA(firstOdor) && likeOdorA(secondOdor))
                                || (currentTrial > 7 && currentTrial < 12 && likeOdorA(firstOdor)&& !likeOdorA(secondOdor))
                                || (currentTrial > 11 && currentTrial < 16 && !likeOdorA(firstOdor) && likeOdorA(secondOdor))
                                || (currentTrial > 15 && currentTrial < 20 && !likeOdorA(firstOdor) && !likeOdorA(secondOdor))) {
                            laserSide = likeOdorA(firstOdor) ? 2 : 1;
                        } else {
                            laserSide = likeOdorA(firstOdor) ? 1 : 2;
                        }
                        laserCurrentTrial = 1;
                        break;
                }
                zxLaserTrial(laserType, firstOdor, odorLength, delay, secondOdor, WaterLen, ITI, delay_before_reward, laserCurrentTrial);
                currentTrial++;
            }
        }
        protectedSerialSend(SpSess, 0);

    }
    protectedSerialSend(SpTrain, 0); // send it's the end
    u2Received = -1;
}

void zxLaserTrial(int type, int firstOdor, float odorLength, _delayT interOdorDelay, int secondOdor, float waterPeroid, int ITI, float delay_before_reward, int laserOnTrial) {
    resetTimerCounterJ();
    protectedSerialSend(Sptrialtype, type);
    protectedSerialSend(Splaser, laserOnTrial);
    assertLaser(type, fourSecBeforeFirstOdor, laserOnTrial);
    waitJ(1000);
    assertLaser(type, threeSecBeforeFirstOdor, laserOnTrial);
    waitJ(2000);
    assertLaser(type, oneSecBeforeFirstOdor, laserOnTrial);
    waitJ(800);
    assertLaser(type, at200mSBeforeFirstOdor, laserOnTrial);
    waitJ(200);
    assertLaser(type, atFirstOdorBeginning, laserOnTrial);
    Valve_ON(firstOdor, fullduty);
    int firstSend;
    if (firstOdor == 2 || firstOdor == 5 || firstOdor == 7) {
        firstSend = 9;
        Out2 = 1;
    } else {
        firstSend = 10;
        Out3 = 1;
    }
    protectedSerialSend(firstSend, firstOdor);
    lcdWriteChar('1', 4, 1);
    waitJ(odorLength * 1000);
    Valve_OFF(firstOdor);
    Out2 = 0;
    Nop();
    Nop();
    Out3 = 0;
    Nop();
    Nop();
    lcdWriteChar('d', 4, 1);
    assertLaser(type, atFirstOdorEnd, laserOnTrial);
    protectedSerialSend(firstSend, 0);
    if (interOdorDelay < 1.0) {
        waitJ(interOdorDelay * 1000);
    } else {
        ///////////-inter odor interval-/////////////////
        assertLaser(type, atDelayBegin, laserOnTrial);
        waitJ(500);
        assertLaser(type, atDelay_5SecIn, laserOnTrial);
        waitJ(500);
        assertLaser(type, atDelay1SecIn, laserOnTrial);
        if (interOdorDelay < 4) {
            waitJ(interOdorDelay * 1000 - 2000);
        } else {
            //TODO INSERT DISTRACTOR HERE
            if (type == laserDelayDistractor) {
                distractor(2);
            } else {
                waitJ(500);
                assertLaser(type, atDelay1_5SecIn, laserOnTrial);
                waitJ(500);
            }
            assertLaser(type, atDelay2SecIn, laserOnTrial);
            if (interOdorDelay == 12) {
                waitJ(500);
                assertLaser(type, atDelay2_5SecIn, laserOnTrial);
                waitJ(500);
                assertLaser(type, atDelay3SecIn, laserOnTrial);
                waitJ(2500);
            } else {
                waitJ(interOdorDelay * 500 - 2500);
            }
            assertLaser(type, atDelay_5ToMiddle, laserOnTrial);
            waitJ(500);
            assertLaser(type, atDelayMiddle, laserOnTrial);
            if (interOdorDelay == 12) {
                waitJ(2500);
                assertLaser(type, atDelay8_5SecIn, laserOnTrial);
                waitJ(500);
                assertLaser(type, atDelay9SecIn, laserOnTrial);
                waitJ(500);
            } else {
                waitJ(interOdorDelay * 500 - 2500);
            }
            assertLaser(type, atDelayLast2_5SecBegin, laserOnTrial);
            waitJ(500);
            assertLaser(type, atDelayLast2SecBegin, laserOnTrial);
            waitJ(500);
            assertLaser(type, atDelayLast1_5SecBegin, laserOnTrial);
            waitJ(500);
        }
        assertLaser(type, atDelayLastSecBegin, laserOnTrial);
        waitJ(500);
        assertLaser(type, atDelayLast500mSBegin, laserOnTrial);
        waitJ(300);
        assertLaser(type, atDelayLast200mSBegin, laserOnTrial);
        waitJ(200);
    }


    assertLaser(type, atSecondOdorBeginning, laserOnTrial);
    if (taskType == _GONOGO_LR_TASK || taskType == _GONOGO_TASK || taskType == _GONOGO_LR_TEACH) {
        //Do nothing during Go Nogo Tasks
        assertLaser(type, atSecondOdorEnd, laserOnTrial);
    } else {
        ///////////-Second odor-/////////////////
        Valve_ON(secondOdor, fullduty);
        //    if (secondOdor == odor_A) Out1 = 1;
        //    else if (secondOdor == odor_B) Out2 = 1;
        int secondSend;
        if (secondOdor == 2 || secondOdor == 5 || secondOdor == 7) {
            secondSend = 9;
            Out2 = 1;
        } else {
            secondSend = 10;
            Out3 = 1;
        }
        protectedSerialSend(secondSend, secondOdor);
        lcdWriteChar('2', 4, 1);
        waitJ(odorLength * 1000 - 10);
        Valve_OFF(secondOdor);
        lcdWriteChar('D', 4, 1);
        assertLaser(type, atSecondOdorEnd, laserOnTrial);
        Out2 = 0;
        Nop();
        Nop();
        Out3 = 0;
        Nop();
        Nop();
        protectedSerialSend(secondSend, 0);
        ////////-delay before reward-///////
    }
    waitJ(delay_before_reward * 1000);
    assertLaser(type, atRewardBeginning, laserOnTrial);
    lcdWriteChar('R', 4, 1);

    //Assess Performance here
    waterNResult(firstOdor, secondOdor, waterPeroid);

    waitJ(550); //water time sync

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
    waitJ(1000);
    assertLaser(type, atITIOneSecIn, laserOnTrial);
    waitJ(ITI * 1000 - 5000); //another 4000 is at the beginning of the trials.
    protectedSerialSend(SpITI, 0);
    waitTrial();
}

void delaymSecKey(unsigned int N, unsigned int type) {
    timerCounterI = 0;
    while (timerCounterI < N) {
        Key_Event();
        if (hardwareKey == 1 || u2Received > 0) {
            switch (type) {
                case 3:
                    laserTimerOn = ((laserTimerOn + 1) % 2)*3;
                    hardwareKey = 0;
                    u2Received = -1;
                    break;
                case 1:
                case 2:
                    ;
                    unsigned char * hi = type == 1 ? &pwmDutyHiL : &pwmDutyHiR;
                    unsigned char * lo = type == 1 ? &pwmDutyLoL : &pwmDutyLoR;
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
                            write_eeprom(type == 1 ? _EEP_DUTY_LOW_L_OFFSET : _EEP_DUTY_LOW_R_OFFSET, *lo);
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
                            write_eeprom(type == 1 ? _EEP_DUTY_HIGH_L_OFFSET : _EEP_DUTY_HIGH_R_OFFSET, *hi);
                            break;
                    }
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

void odorDepeltion(int totalTrial) {
    initZXTMR();
    PDC2 = fullduty;
    PDC4 = fullduty;
    int i;
    for (i = 0; i < totalTrial; i++) {
        home_clr();
        lcdWriteString("Odor Depletion");
        int percent = i > 599 ? 999 : i * 100 / 60;
        lcdWriteNumber(percent, 3, 1, 2);
        lcdWriteString("%, Valve ");

        LCD_set_xy(13, 2);
        lcdWriteString("2|5");
        Valve_ON(5, fullduty);
        Nop();
        Nop();
        Valve_ON(2, fullduty);
        delaymSecKey(15000, 3);
        Valve_OFF(5);
        Nop();
        Nop();
        Valve_OFF(2);
        LCD_set_xy(13, 2);
        lcdWriteString("3|6");
        Valve_ON(6, fullduty);
        Nop();
        Nop();
        Valve_ON(3, fullduty);
        delaymSecKey(15000, 3);
        Valve_OFF(6);
        Nop();
        Nop();
        Valve_OFF(3);
    }
}

void rampTweak(int L, int itiIndex) {
    int laser[] = {3500, 3500, 6500, 10500, 1500, 2500};
    int delay = laser[getFuncNumber(1, "5 8 12 1.5 2.5s")];
    int iti = itiIndex ? (delay + 1500)*2 : 2500;

    ramp = 500;
    splash("Ramping", "");
    lcdWriteChar(L ? 'L' : 'R', 9, 1);

    for (;;) {
        unsigned char Hi = L ? pwmDutyHiL : pwmDutyHiR;
        unsigned char Lo = L ? pwmDutyLoL : pwmDutyLoR;
        lcdWriteNumber(Lo, 3, 8, 2);
        lcdWriteNumber(Hi, 3, 13, 2);


        delaymSecKey(iti, L ? 1 : 2);
        turnOnLaser(L ? 1 : 2);
        delaymSecKey(delay, L ? 1 : 2);
        turnOffLaser();
    }
}

void feedWater(int topTime) {
    initZXTMR();
    int times[] = {600, 2, 5, 10, 30, 600};
    topTime = times [topTime];
    splash("Auto Feed Water", "Wait for lick...");
    while (!licking) {
    }
    int go = 1;
    while (go) {
        splash("Feeding", "   s left.");
        Valve_ON(water_sweet, fullduty);
        int secondsWithoutLick = 0;
        int licked = 0;
        while (secondsWithoutLick < topTime) {
            lcdWriteNumber(topTime - secondsWithoutLick, 3, 1, 2);
            timerCounterI = 0;
            while (timerCounterI < 1000) {
                if (_LICK_ANY) {

                    licked = 1;
                }
            }
            secondsWithoutLick = licked ? 0 : secondsWithoutLick + 1;
            licked = 0;
        }
        Valve_OFF(water_sweet);
        go = (getFuncNumber(1, "No Lick. Again=1") == 1);
    }
}

void feedWaterLR(float waterLength) {
    taskType = _GONOGO_LR_TASK;
    int lastLocation = 0;
    lickLCount = 0;
    lickRCount = 0;
    unsigned int waterCount = 0;
    unsigned int lastL = 0;
    unsigned int lastR = 0;
    splash("L       R", "W     FL   FR");
    PDC1 = fullduty;
    PDC2 = fullduty;
    timerCounterI = 2000;

    while (1) {
        if (lickLCount > lastL) {
            if (timerCounterI > 2000 && lastLocation != _LICKING_LEFT) {
                protectedSerialSend(22, 1);
                PORTGbits.RG0 = 1;
                lastLocation = _LICKING_LEFT;
                timerCounterI = 0;
                lcdWriteChar('L', 16, 1);
                lcdWriteNumber(++waterCount, 4, 2, 2);
            }
            lcdWriteNumber(lickLCount, 4, 3, 1);
            lastL = lickLCount;
        } else if (lickRCount > lastR) {
            if (timerCounterI > 2000 && lastLocation != _LICKING_RIGHT) {
                protectedSerialSend(22, 2);
                PORTGbits.RG1 = 1;
                lastLocation = _LICKING_RIGHT;
                timerCounterI = 0;
                lcdWriteChar('R', 16, 1);
                lcdWriteNumber(++waterCount, 4, 2, 2);
            }
            lcdWriteNumber(lickRCount, 4, 11, 1);
            lastR = lickRCount;
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

void feedWaterFast(int waterLength) {

    lickLCount = 0;
    lickRCount = 0;
    unsigned int waterCount = 0;
    unsigned int totalLickCount = 0;
    splash("Total Lick", "");

    timerCounterI = 1000;
    while (1) {
        if (lickLCount + lickRCount > totalLickCount) {
            if (timerCounterI >= 500) {
                Valve_ON(water_sweet, fullduty);
                timerCounterI = 0;
                lcdWriteNumber(++waterCount, 4, 13, 2);
            }
            totalLickCount = lickLCount + lickRCount;
            lcdWriteNumber(totalLickCount, 4, 12, 1);
        }
        if (timerCounterI >= waterLength) {
            Valve_OFF(water_sweet);
        }
    }
}

void splash(char * s1, char * s2) {

    home_clr();
    lcdWriteString(s1);
    line_2();
    lcdWriteString(s2);
    wait_ms(1000);
}

void wait_ms(int time) {
    if (_DEBUGGING) return; //DEBUG
    timerCounterI = 0;
    while (time > 0 && timerCounterI < time) {
    }
}

void variableVoltage() {
    laserTimerOn = 1;
    char VString[] = {'1', '2', '7', ' ', ' ', ' ', '-', '-', ' ', '-', ' ', '+', ' ', '+', '+', ' '};
    int tmpFreq;
    tmpFreq = pwmDutyLoR;

    while (1) {
        tmpFreq = (tmpFreq > fullduty) ? fullduty : tmpFreq;
        tmpFreq = (tmpFreq < 0) ? 0 : tmpFreq;
        pwmDutyLoR = tmpFreq;
        protectedSerialSend(SpValve8, pwmDutyLoR);
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
                write_eeprom(m == 1 ? _EEP_DUTY_LOW_L_OFFSET : _EEP_DUTY_HIGH_L_OFFSET, tmpFreq);
        }

    }
}

void testPort(void) {
    splash("TEST", "PORT");
    int i;
    while (1) {
        for (i = 1; i < 9; i++) {

            Valve_ON(i, fullduty);
            wait_ms(250);
            Valve_OFF(i);
            wait_ms(10);
        }
    }
}

void laserTrain() {
    unsigned int freqs[] = {1, 5, 10, 20, 50, 100};
    laserOnTime = 5;
    unsigned int idx = 0;
    for (idx = 0; idx < 6; idx++) {
        unsigned int duration = 1000 / freqs[idx];
        laserOffTime = duration - laserOnTime;
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

        Valve_ON(v, fullduty);
        wait_ms(1000);
        Valve_OFF(v);
        wait_ms(1000);
    }
}

void testVolume(int waterLength) {
    int n = getFuncNumber(1, "Valve #");
    int i;
    for (i = 0; i < 100; i++) {

        Valve_ON(n, fullduty);
        wait_ms(waterLength);
        Valve_OFF(n);
        wait_ms(500 - waterLength);
    }
}

void test_Laser(void) {
    int i = 1;
    while (1) {

        laserTimerOn = i;
        getFuncNumber(1, "Toggle Laser");
        i = (i + 1) % 2;

    }
}

void flashLaser(void) {
    splash("Test Laser", "");
    int count = 0;
    while (1) {
        lcdWriteNumber(++count, 3, 10, 2);
        resetTimerCounterJ();
        turnOnLaser(3);
        waitJ(2000);
        turnOffLaser();
        waitJ(5000);
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

void stepLaser() {
    int laserOn = 3000;
    int laserOff = 16000;
    int step;
    int sCounter = 0;
    PORTCbits.RC1 = 1;
    pwmDutyLoR = 0xfe;
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
    pwmDutyLoL = read_eeprom(_EEP_DUTY_LOW_L_OFFSET);
    pwmDutyHiL = read_eeprom(_EEP_DUTY_HIGH_L_OFFSET);
    pwmDutyLoR = read_eeprom(_EEP_DUTY_LOW_R_OFFSET);
    pwmDutyHiR = read_eeprom(_EEP_DUTY_HIGH_R_OFFSET);


    PORTCbits.RC1 = 1;
    initZXTMR();
    switch (n) {
            //            int m;
            //ZX's functions

        case 4301:
            taskType = _NO_ODOR_CATCH_TRIAL_TASK;
            ramp = 500;
            splash("No Odor Catch", "");

            laserTrialType = _LASER_LR_EVERYTRIAL;
            zxLaserSessions(3, laserRampDuringDelay, 2, 5, 20, 0.05, 20, 15, 1.0);
            break;

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
            splash("Response Delay", "Laser Control");
            laserTrialType = _LASER_OTHER_TRIAL;
            zxLaserSessions(setType(), laserDuringResponseDelay, 5, 10, 20, 0.05, 20, 20, 1.0);
            break;

        case 4313:
            splash("GoNogo Control", "For DNMS");
            //            zxGoNogoSessions(setType(), 3, 20, 1, 0.5, 4);
            laserTrialType = _LASER_OTHER_TRIAL;
            taskType = _GONOGO_TASK;
            zxLaserSessions(setType(), laserDuring3Baseline, 0, 5, 20, 0.05, 20, 20, 1.0);
            break;

        case 4314:
            splash("Vary Length", "Delay Laser");
            laserTrialType = _LASER_VARY_LENGTH;
            ramp = 500;
            zxLaserSessions(setType(), laserDuringDelay, 5, 10, 20, 0.05, 15, 25, 1.0);
            ramp = 0;
            break;

        case 4315:
            splash("Each Quarter", "Delay Laser");
            laserTrialType = _LASER_EACH_QUARTER;
            ramp = 500;
            zxLaserSessions(3, laserDuringDelay, 8, 16, 20, 0.05, 20, 15, 1.0);
            ramp = 0;
            break;

        case 4316:
            stepLaser();
            break;

        case 4317:
            taskType = _NO_ODOR_CATCH_TRIAL_TASK;
            ramp = 500;
            splash("No Odor Catch", "");

            laserTrialType = _LASER_LR_EVERYTRIAL;
            zxLaserSessions(3, laserRampDuringDelay, 12, 24, 20, 0.05, 20, 15, 1.0);
            break;

            //        case 4317:
            //            splash("Each Quarter", "Delay LR Laser");
            //            laserTrialType = _LASER_LR_EACH_QUARTER;
            //            ramp = 500;
            //            zxLaserSessions(3, laserDuringDelay, 8, 16, 20, 0.05, 15, 25, 1.0);
            //            ramp = 0;
            //            break;

            //        case 4317:
            //            correctTime();
            //            break;

        case 4318:
            splash("No Trial Wait", "LR Laser");
            wait_Trial = 0;
            break;

        case 4319:
            taskType = _DNMS_TASK;
            ramp = 500;
            splash("LR LASER DNMS", "Sufficiency");
            int delay = setDelay(1);
            laserTrialType = _LASER_LR_EVERYTRIAL;
            zxLaserSessions(3, laserRampDuringDelay, delay, delay * 2, 20, 0.05, 20, setSessionNum(), 1.0);
            break;

        case 4320:
            splash("12s Each Quarter", "Delay LR Laser");
            laserTrialType = _LASER_12s_LR_EACH_QUARTER;
            ramp = 500;
            zxLaserSessions(3, laserDuringDelay, 12, 24, 20, 0.05, 25, 15, 1.0);
            ramp = 0;
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
            //            zxLaserSessions(setType(), m, 0.2, 5, 20, 0.05, 20, 20, 1.0);
            break;
        case 4322:
            variableVoltage();
            break;

        case 4323:
        {
            splash("DNMS LR TEACH", "Ramp Laser ++");
            protectedSerialSend(21, 101);
            taskType = _DNMS_LR_TEACH;
            laserTrialType = _LASER_EVERY_TRIAL;
            ramp = 500;
            zxLaserSessions(3, laserRampDuringDelay, 5, 10, 20, 0.05, 10, 15, 1.0);
            break;
        }

        case 4324:
            testPort();
            break;
        case 4325:
            splash("BaseLine RQ", "Control");
            laserTrialType = _LASER_OTHER_TRIAL;
            zxLaserSessions(3, laserDuring3Baseline, 5, 10, 20, 0.05, 20, 15, 1.0);
            break;

        case 4326:
            taskType = _DNMS_TASK;
            ramp = 500;
            splash("Incongrument", "LR Laser");
            laserTrialType = _LASER_INCONGRUENT_CATCH_TRIAL;
            zxLaserSessions(3, laserRampDuringDelay, 12, 24, 20, 0.05, 20, 15, 1.0);
            break;

        case 4327:
        {
            splash("DNMS LR TEACH", "No Laser");
            protectedSerialSend(21, 100);
            taskType = _DNMS_LR_TEACH;
            laserTrialType = _LASER_NO_TRIAL;
            zxLaserSessions(3, laserRampDuringDelay, 5, 10, 20, 0.05, 20, 15, 1.0);
            break;
        }

        case 4331:
            splash("Delay+Odor2", "Control");
            laserTrialType = _LASER_OTHER_TRIAL;
            zxLaserSessions(setType(), laserDuringDelay_Odor2, 5, 10, 20, 0.05, 20, 15, 1.0);
            break;

        case 4332:
            splash("Odor", "Control");
            laserTrialType = _LASER_OTHER_TRIAL;
            int m = getFuncNumber(1, "1st 2nd BothOdor");
            zxLaserSessions(setType(), m == 1 ? laserDuring1stOdor : m == 2 ? laserDuring2ndOdor : laserDuringOdor, 5, 10, 20, 0.05, 50, 30, 1.0);
            break;
        case 4333:
            test_Laser();
            break;


            //        case 4334:
            //            splash("DNMS Shaping", "");
            //            laserTrialType = _LASER_NO_TRIAL;
            //            taskType = _SHAPPING_TASK;
            //            zxLaserSessions(3, laserDuringDelay, 4, 8, 20, 0.5, 50, 20, 1.0);
            //            break;

            //        case 4335:
            //        {
            //            splash("DNMS 4s", "DC laser,");
            //            taskType = _DNMS_TASK;
            //            zxLaserSessions(3, laserDuringDelay, 4, 8, 20, 0.5, 50, setSessionNum(), 1.0);
            //            break;
            //        }

        case 4341:
            splash("DNMS 5s Shaping", "RQ");
            laserTrialType = _LASER_NO_TRIAL;
            taskType = _SHAPPING_TASK;
            zxLaserSessions(3, laserDuringDelayChR2, 5, 10, 20, 0.05, 50, setSessionNum(), 1.0);
            break;

            //        case 4342:
            //        {
            //            splash("DNMS 5s Water+", "RQ DC laser");
            //            laserTrialType = _LASER_EVERY_TRIAL;
            //            taskType = _DNMS_TASK;
            //            zxLaserSessions(3, laserDuringDelayChR2, 5, 10, 20, 0.1, 50, setSessionNum(), 1.0);
            //            break;
            //        }
        case 4343:
        {
            splash("DNMS 5s ++", "RQ DC laser");
            laserTrialType = _LASER_EVERY_TRIAL;
            taskType = _DNMS_TASK;
            zxLaserSessions(3, laserDuringDelayChR2, 5, 10, 20, 0.05, 50, setSessionNum(), 1.0);
            break;
        }
        case 4344:
        {
            splash("DNMS 5s", "RQ NoLaser");
            laserTrialType = _LASER_NO_TRIAL;
            taskType = _DNMS_TASK;
            zxLaserSessions(3, laserDuringDelayChR2, 5, 10, 20, 0.05, 50, setSessionNum(), 1.0);
            break;
        }

        case 4345:
        {
            splash("DNMS 8s ++", "RQ DC laser");
            laserTrialType = _LASER_EVERY_TRIAL;
            taskType = _DNMS_TASK;
            zxLaserSessions(3, laserDuringDelayChR2, 8, 16, 20, 0.05, 50, setSessionNum(), 1.0);
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
            laserTrialType = _LASER_OTHER_TRIAL;
            taskType = _DNMS_TASK;
            zxLaserSessions(3, laserDuringDelayChR2, 5, 10, 20, 0.05, 50, setSessionNum(), 1.0);
            break;
        }

        case 4354:
            rampTweak(1, 0);
            break;

        case 4355:
        {
            splash("DNMS 8s +-", "RQ DC laser");
            laserTrialType = _LASER_OTHER_TRIAL;
            taskType = _DNMS_TASK;
            zxLaserSessions(3, laserDuringDelayChR2, 8, 16, 20, 0.05, 50, setSessionNum(), 1.0);
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
            write_eeprom(_EEP_DUTY_LOW_R_OFFSET, 0x7f);
            write_eeprom(_EEP_DUTY_LOW_L_OFFSET, 0x7f);
            break;

        case 4411:
            splash("During Delay", "");
            //            setLaser();
            zxLaserSessions(0, laserDuringDelay, 4, 8, 20, 0.05, 50, setSessionNum(), 1.0);
            break;



        case 4412:
        {
            splash("Vary Delay", "Shaping");
            laserTrialType = _LASER_NO_TRIAL;
            taskType = _SHAPPING_TASK;
            int delay = setDelay(2);
            zxLaserSessions(setType(), laserDuringDelay, delay, delay * 2, 20, 0.05, 50, 40, 1.0);
            break;
        }


        case 4413:
        {
            splash("DNMS LR Distra.", "Laser EveryTrial");
            taskType = _DNMS_LR_TASK;
            laserTrialType = _LASER_EVERY_TRIAL;
            zxLaserSessions(3, laserDelayDistractor, 5, 10, 20, 0.05, 200, 10, 1.0);
            break;
        }
        case 4414:
        {
            splash("Varying Delay", "DC laser,");
            taskType = _DNMS_TASK;
            zxLaserSessions(3, laserDuringDelay, 4, 8, 20, 0.125, 50, 5, 1.0);
            break;
        }
        case 4415:
        {
            splash("Varying Delay", "DC laser,");
            int delay = setDelay(1);
            taskType = _DNMS_TASK;
            laserTrialType = setLaser();
            zxLaserSessions(3, laserDuringDelayChR2, delay, delay * 2, 20, 0.05, 50, setSessionNum(), 1.0);
            break;
        }


        case 4416:
        {
            splash("Varying Delay", "DC laser,");
            int delay = setDelay(1);
            taskType = _DNMS_TASK;
            laserTrialType = setLaser();
            zxLaserSessions(3, laserDuringDelayChR2, delay, delay * 2, 20, 0.036, 50, setSessionNum(), 1.0);
            break;
        }

        case 4417:
            splash("DNMS Shaping", "");
            laserTrialType = _LASER_NO_TRIAL;
            taskType = _SHAPPING_TASK;
            zxLaserSessions(setType(), laserDuringDelay, 4, 8, 20, 0.036, 50, setSessionNum(), 1.0);
            break;

        case 4418:
            splash("Feed Water", "");
            feedWaterFast(36);
            break;

        case 4419:
            splash("Test Volume", "");
            testVolume(36);
            break;

        case 4421:
            splash("DNMS Shaping", "");
            laserTrialType = _LASER_NO_TRIAL;
            taskType = _SHAPPING_TASK;
            zxLaserSessions(setType(), laserDuringDelay, 4, 8, 20, 0.05, 50, setSessionNum(), 1.0);
            break;
        case 4422:
            splash("DNMS Shaping", "Long Water");
            laserTrialType = _LASER_NO_TRIAL;
            taskType = _SHAPPING_TASK;
            zxLaserSessions(setType(), laserDuringDelay, 4, 8, 20, 0.1, 50, 40, 0.5);
            break;
        case 4423:
            testValve();
            break;

        case 4424:
            feedWaterFast(50);
            break;


        case 4425:
            splash("Learning", "DC laser");
            zxLaserSessions(setType(), laserDuringDelay, 4, 8, 20, 0.05, 60, setSessionNum(), 1.0);
            break;

            //        case 4426:
            //            shiftingLaser();
            //            break;

        case 4431:
        {
            splash("VarDelay LR", "DC laser");
            int delay = setDelay(1);
            taskType = _DNMS_LR_TASK;
            zxLaserSessions(setType(), laserDuringDelay, delay, delay * 2, 20, 0.05, 50, setSessionNum(), 1.0);
            break;
        }
        case 4432:
        {
            taskType = _GONOGO_LR_TASK;
            splash("GoNogo LR", "");
            int m = getFuncNumber(1, "No Pre Odor Post");
            int laserPeriod;
            laserPeriod = laserDuring4Baseline;
            switch (m) {
                case 1:
                    taskType = _GONOGO_LR_TEACH;
                    laserTrialType = _LASER_NO_TRIAL;
                    break;
                case 3:
                    laserPeriod = laserDuring1stOdor;
                    break;
                case 4:
                    laserPeriod = laserDuringResponseDelay;
                    break;
            }
            zxLaserSessions(setType(), laserPeriod, 0, 5, 20, 0.05, 50, 30, 1.0);
            break;
        }

        case 4433:
        {
            splash("DNMS LR", "Laser EveryTrial");
            taskType = _DNMS_LR_TASK;
            laserTrialType = _LASER_EVERY_TRIAL;
            zxLaserSessions(3, laserDuringDelayChR2, 5, 10, 20, 0.05, 200, 10, 1.0);
            break;
        }

        case 4434:
            laserTrain();
            break;

        case 4435:
        {
            splash("Baseline LR", "Laser EveryTrial");
            taskType = _DNMS_LR_TASK;
            zxLaserSessions(3, laserDuring3Baseline, 5, 10, 20, 0.05, 200, 20, 1.0);
            break;
        }

            //        case 4436:
            //        {
            //            splash("DNMS LR", "Laser EveryTrial");
            //            taskType = _DNMS_LR_TASK;
            //            laserTrialType = _LASER_EVERY_TRIAL;
            //            zxLaserSessions(3, laserDelayDistractor, 5, 10, 20, 0.05, 200, 10, 1.0);
            //            break;
            //        }

        case 4441:
        {
            splash("First Odor LR", "Laser EveryTrial");
            taskType = _DNMS_LR_TASK;
            zxLaserSessions(3, laserDuring1stOdor, 5, 10, 20, 0.05, 200, 20, 1.0);
            break;
        }
        case 4442:
        {
            splash("Second Odor LR", "Laser EveryTrial");
            taskType = _DNMS_LR_TASK;
            zxLaserSessions(3, laserDuring2ndOdor, 5, 10, 20, 0.05, 200, 20, 1.0);
            break;
        }
        case 4443:
        {
            splash("Response Delay LR", "Laser EveryTrial");
            taskType = _DNMS_LR_TASK;
            zxLaserSessions(3, laserDuringResponseDelay, 5, 10, 20, 0.05, 200, 20, 1.0);
            break;
        }


        case 4445:
            feedWaterLR(50);
            break;
    }
}


