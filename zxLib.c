#include "commons.h"
#include "tasks.h"
#include "zxLib.h"
#include <libpic30.h>

char* zxVer = "z5310";
float odorLength = 1.0;

unsigned int laserTimer = 0;
//unsigned int laserOnTime = 65535;
//unsigned int laserOffTime = 0;
unsigned int ramp = 0;
unsigned int ramping = 0;
int pwmFreqHi = 0xfe;
int pwmFreqLo = 0;
unsigned int laserTimerOn = 0;
unsigned int licking = 0;
const char odorTypes[] = {' ', 'W', 'B', 'J', 'w', 'R', 'Q', 'r', 'q'};
unsigned int timeFilter = 0;
unsigned int laserTrialType = _LASER_EVERY_TRIAL;
unsigned int taskType = _DNMS_TASK;
unsigned int lickFlag = 0;
unsigned int fullduty = 0xFE;

int highLevelShuffleLength = 12;

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
}

int filtered(void) {
    return (timerCounterJ < timeFilter || timerCounterJ > timeFilter + 50);
}

void __attribute__((__interrupt__, no_auto_psv)) _T1Interrupt(void) {
    IFS0bits.T1IF = 0;
    laserTimer += 5;
    timerCounterI += 5;
    timerCounterJ += 5;


    //    if (laserTimerOn && (laserTimer % (laserOnTime + laserOffTime) < laserOnTime)) {
    if (laserTimerOn) {
        Out4 = 1;
        __builtin_nop();
        __builtin_nop();
        Out5 = 1;
        PDC4 = pwmFreqLo;
        ramping = ramp;


    } else {
        Out4 = 0;
        __builtin_nop();
        __builtin_nop();
        Out5 = 0;
        if (ramping > 0) {
            PDC4 = (pwmFreqHi - pwmFreqLo) * (1 - ((float) ramping / ramp)) + pwmFreqLo;
            ramping -= 5;
        } else {
            PDC4 = 0xfe;
        }

    }

    if (_LICK_LEFT && !licking && filtered()) {
        timeFilter = timerCounterJ;
        licking = _LICKING_LEFT;
        Out1 = 1;
        Nop();
        Nop();
        Nop();
        localSendOnce(SpLick, taskType < 35 ? 1 : 2);
    } else if (_LICK_RIGHT && !licking && filtered()) {
        timeFilter = timerCounterJ;
        licking = _LICKING_RIGHT;
        localSendOnce(SpLick, 3);
        Out6 = 1;
        Nop();
        Nop();
        Nop();
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

    int sess[] = {20, 1, 5, 10, 20, 30};
    return sess[getFuncNumber(1, "1 5 10 20 30Ses")];
}

int setDelay(int type) {
    if (type == 1) {
        int n = getFuncNumber(1, "4s 8s 12s");

        return n == 3 ? 12 : n == 2 ? 8 : 4;
    }
    int n = getFuncNumber(1, "4 8 11 14 17");
    int d[] = {5, 4, 8, 11, 14, 17};
    return d[n];
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
    IEC0bits.T1IE = 0;
    IEC0bits.T3IE = 0;
    unsigned int i;
    for (i = 0; i < 6400; i++); //1600 per ms
    localSendOnce(type, value);

    for (i = 0; i < 4800; i++); //1600 per ms
    timerCounterI += 10;
    timerCounterJ += 10;
    IEC0bits.T1IE = 1;
    IEC0bits.T3IE = 1;
}

void turnOnLaser() {

    lcdWriteChar('L', 4, 1);
    protectedSerialSend(SpLaserSwitch, 1);
    laserTimerOn = 1;
}

void turnOffLaser() {

    lcdWriteChar('.', 4, 1);
    protectedSerialSend(SpLaserSwitch, 0);
    laserTimerOn = 0;
}

void assertLaser(int type, int step, int currentTrial) {
    if (currentTrial) {
        switch (type) {
            case laserDuringBeginningToOneSecInITI:
                if (step == atFirstOdorBeginning) {
                    turnOnLaser();
                } else if (step == atITIOneSecIn) {
                    turnOffLaser();
                }
                break;
            case laserDuringDelay:
                if (step == atFirstOdorEnd) {
                    turnOnLaser();
                } else if (step == atDelayLast200mSBegin) {
                    turnOffLaser();
                }
                break;
            case laserDuringDelayChR2:
                if (step == atDelay1SecIn) {
                    turnOnLaser();
                } else if (step == atDelayLastSecBegin) {
                    turnOffLaser();
                }
                break;
            case laserDuringBaselineNDelay:
                if (step == atDelay1SecIn || step == threeSecBeforeFirstOdor) {
                    turnOnLaser();
                } else if (step == atDelayLastSecBegin || step == at200mSBeforeFirstOdor) {
                    turnOffLaser();
                }
                break;
            case laserDuringOdor:
                if (step == atFirstOdorBeginning || step == atSecondOdorBeginning) {
                    turnOnLaser();
                } else if (step == atFirstOdorEnd || step == atSecondOdorEnd) {
                    turnOffLaser();
                }
                break;
            case laserDuring1stOdor:
                if (step == atFirstOdorBeginning) {
                    turnOnLaser();
                } else if (step == atFirstOdorEnd) {
                    turnOffLaser();
                }
                break;
            case laserDuring2ndOdor:
                if (step == atSecondOdorBeginning) {
                    turnOnLaser();
                } else if (step == atSecondOdorEnd) {
                    turnOffLaser();
                }
                break;
            case laserDuringEarlyHalf:
                if (step == atDelayBegin) {
                    turnOnLaser();
                } else if (step == atDelayMiddle) {
                    turnOffLaser();
                }
                break;

            case laserDuringLateHalf:
                if (step == atDelayMiddle) {
                    turnOnLaser();
                } else if (step == atSecondOdorBeginning) {
                    turnOffLaser();
                }
                break;
            case laserDuring1Terice:
                if (step == atDelayBegin) {
                    turnOnLaser();
                } else if (step == atDelay2SecIn) {
                    turnOffLaser();
                }
                break;
            case laserDuring2Terice:
                if (step == atDelay2SecIn) {
                    turnOnLaser();
                } else if (step == atDelayLast2SecBegin) {
                    turnOffLaser();
                }
                break;
            case laserDuring3Terice:
                if (step == atDelayLast2SecBegin) {
                    turnOnLaser();
                } else if (step == atSecondOdorBeginning) {
                    turnOffLaser();
                }
                break;
            case laserDuring1Quarter:
                if (step == atDelayBegin) {
                    turnOnLaser();
                } else if (step == atDelay1_5SecIn) {
                    turnOffLaser();
                }
                break;
            case laserDuring2Quarter:
                if (step == atDelay2SecIn) {
                    turnOnLaser();
                } else if (step == atDelay_5ToMiddle) {
                    turnOffLaser();
                }
                break;
            case laserDuring3Quarter:
                if (step == atDelayMiddle) {
                    turnOnLaser();
                } else if (step == atDelayLast2_5SecBegin) {
                    turnOffLaser();
                }
                break;
            case laserDuring4Quarter:
                if (step == atDelayLast2SecBegin) {
                    turnOnLaser();
                } else if (step == atDelayLast500mSBegin) {
                    turnOffLaser();
                }
                break;
            case laserDuringResponseDelay:
                if (step == atSecondOdorEnd) {
                    turnOnLaser();
                } else if (step == atRewardBeginning) {
                    turnOffLaser();
                }
                break;
            case laserNoDelayControl:
                if (step == oneSecBeforeFirstOdor) {
                    turnOnLaser();
                } else if (step == atRewardBeginning) {
                    turnOffLaser();
                }
                break;
            case laserNoDelayControlShort:
                if (step == atFirstOdorBeginning) {
                    turnOnLaser();
                } else if (step == atSecondOdorEnd) {
                    turnOffLaser();
                }
                break;
            case laserDuringBaseline:
                if (step == threeSecBeforeFirstOdor) {
                    turnOnLaser();
                } else if (step == atFirstOdorBeginning) {
                    turnOffLaser();
                }
                break;
            case laserDuring3Baseline:
                if (step == fourSecBeforeFirstOdor) {
                    turnOnLaser();
                } else if (step == oneSecBeforeFirstOdor) {
                    turnOffLaser();
                }
                break;
            case laserDuring4Baseline:
                if (step == fourSecBeforeFirstOdor) {
                    turnOnLaser();
                } else if (step == at200mSBeforeFirstOdor) {
                    turnOffLaser();
                }
                break;
            case laserDuringBaseAndResponse:
                if (step == oneSecBeforeFirstOdor || step == atSecondOdorEnd) {
                    turnOnLaser();
                } else if (step == at200mSBeforeFirstOdor || step == atRewardBeginning) {

                    turnOffLaser();
                }
                break;
            case laser4sRamp:
                if (step == atDelay_5SecIn) {
                    turnOnLaser();
                } else if (step == atDelayLast500mSBegin) {
                    turnOffLaser();
                }
                break;
            case laser2sRamp:
                if (step == atDelayLast2_5SecBegin) {
                    turnOnLaser();
                } else if (step == atDelayLast500mSBegin) {
                    turnOffLaser();
                }
                break;
            case laser1sRamp:
                if (step == atDelayLast1_5SecBegin) {
                    turnOnLaser();
                } else if (step == atDelayLast500mSBegin) {
                    turnOffLaser();
                }
                break;
            case laser_5sRamp:
                if (step == atDelayLastSecBegin) {
                    turnOnLaser();
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
    int length = strlen(s) > 16 ? 16 : strlen(s);
    int iter;
    for (iter = 0; iter < length; iter++) {

        lcd_data((char) s[iter]);
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

void waterNResult(int firstOdor, int secondOdor, float waterPeroid) {
    lickFlag = 0;
    switch (taskType) {
            /*
             *DNMS
             */
        case _DNMS_TASK:
        case _SHAPPING_TASK:
        case _ASSOCIATE_TASK:
        case _ASSOCIATE_SHAPPING_TASK:

            ///////////Detect/////////////////
            for (timerCounterI = 0; timerCounterI < 500 && !lickFlag;) {
                lickFlag = licking;
            }

            /////Reward
            if (!lickFlag) {
                if (firstOdor == secondOdor || firstOdor == secondOdor + 3) {
                    protectedSerialSend(SpCorrectRejection, 1);
                    lcdWriteNumber(++correctRejection, 3, 10, 2);
                } else {
                    currentMiss++;
                    protectedSerialSend(SpMiss, 1);
                    lcdWriteNumber(++miss, 3, 10, 1);
                    if ((taskType == _SHAPPING_TASK || taskType == _ASSOCIATE_SHAPPING_TASK) && ((rand() % 3) == 0)) {
                        Valve_ON(water_sweet, fullduty);
                        protectedSerialSend(SpMiss3, 1);
                        wait_ms(waterPeroid * 1000);
                        Valve_OFF(water_sweet);
                    }
                }
            } else if (firstOdor == secondOdor || firstOdor == secondOdor + 3) {
                currentMiss = 0;
                protectedSerialSend(SpFalseAlarm, 1);
                lcdWriteNumber(++falseAlarm, 3, 6, 2);
            } else {
                Valve_ON(water_sweet, fullduty);
                wait_ms(waterPeroid * 1000);
                Valve_OFF(water_sweet);
                currentMiss = 0;
                protectedSerialSend(SpHit, 1);
                lcdWriteNumber(++hit, 3, 6, 1);
            }
            break;


            /*
             *DNMS-LR
             */
        case _DNMS_LR_TASK:
            //
            ///////////Detect/////////////////
            for (timerCounterI = 0; timerCounterI < 1000 && !lickFlag;) {
                lickFlag = licking;
            }

            /////Reward
            if (!lickFlag) {
                currentMiss++;
                protectedSerialSend(SpMiss, (firstOdor != secondOdor) ? 2 : 3);
                lcdWriteNumber(++miss, 3, 10, 1);
            } else if (!(lickFlag & 1) != !(firstOdor^secondOdor)) {
                Valve_ON(lickFlag & 1 ? 4 : 1, fullduty);
                wait_ms(waterPeroid * 1000);
                Valve_OFF(lickFlag & 1 ? 4 : 1);
                currentMiss = 0;
                protectedSerialSend(SpHit, lickFlag);
                lcdWriteNumber(++hit, 3, 6, 1);
            } else {
                currentMiss = 0;
                protectedSerialSend(SpFalseAlarm, lickFlag == _LICKING_LEFT ? 2 : 3);
                lcdWriteNumber(++falseAlarm, 3, 6, 2);
            }
            break;

        case _GONOGO_LR_TASK:
        case _GONOGO_LR_TRAIN:
            //
            ///////////Detect/////////////////
            for (timerCounterI = 0; timerCounterI < 1000 && !lickFlag; lickFlag = licking) {
            }

            /////Reward
            if (lickFlag == _LICKING_LEFT && (firstOdor == 2 || firstOdor == 5 || firstOdor == 7)) {
                Valve_ON(1, 0xfe);
                wait_ms(waterPeroid * 1000);
                Valve_OFF(1);
                currentMiss = 0;
                protectedSerialSend(SpHit, 2);
                lcdWriteNumber(++hit, 3, 6, 1);
            } else if (lickFlag == _LICKING_RIGHT && (firstOdor == 3 || firstOdor == 6 || firstOdor == 8)) {
                Valve_ON(4, 0xfe);
                wait_ms(waterPeroid * 1000);
                Valve_OFF(4);
                currentMiss = 0;
                protectedSerialSend(SpHit, 3);
                lcdWriteNumber(++hit, 3, 6, 1);
            } else if (lickFlag) {
                currentMiss = 0;
                protectedSerialSend(SpFalseAlarm, lickFlag == _LICKING_LEFT ? 2 : 3);
                lcdWriteNumber(++falseAlarm, 3, 6, 2);
            } else {
                currentMiss++;
                protectedSerialSend(SpMiss, (firstOdor == 2 || firstOdor == 5 || firstOdor == 7) ? 2 : 3);
                lcdWriteNumber(++miss, 3, 10, 1);
            }
            break;

        case _GONOGO_TASK:

            for (timerCounterI = 0; timerCounterI < 500 && !lickFlag;) {
                lickFlag = licking;
            }

            /////Reward
            if (!lickFlag) {
                if (firstOdor == 3 || firstOdor == 6 || firstOdor == 8) {
                    protectedSerialSend(SpCorrectRejection, 1);
                    lcdWriteNumber(++correctRejection, 3, 10, 2);
                } else {
                    currentMiss++;
                    protectedSerialSend(SpMiss, 1);
                    lcdWriteNumber(++miss, 3, 10, 1);
                }
            } else if (firstOdor == 3 || firstOdor == 6 || firstOdor == 8) {
                currentMiss = 0;
                protectedSerialSend(SpFalseAlarm, 1);
                lcdWriteNumber(++falseAlarm, 3, 6, 2);
            } else {

                Valve_ON(water_sweet, fullduty);
                wait_ms(waterPeroid * 1000);
                Valve_OFF(water_sweet);
                currentMiss = 0;
                protectedSerialSend(SpHit, 1);
                lcdWriteNumber(++hit, 3, 6, 1);
            }
            break;
    }
}

void zxLaserSessions(int odorType, int laserType, float delay, int ITI, int trialsPerSession, float WaterLen, int missLimit, int totalSession, float delay_before_reward) {
    wait_ms(1000);
    int currentTrial = 0;
    int currentSession = 0;

    protectedSerialSend(SpValve8, pwmFreqLo);
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
        int lastOdor;
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
                    case _GONOGO_LR_TRAIN:
                        if (hit > lastHit || currentTrial == 0) {
                            firstOdor = (index == 0 || index == 2) ? odor_A : odor_B;
                            lastOdor = firstOdor;
                            secondOdor = -odorType;
                        } else {
                            firstOdor = lastOdor;
                            secondOdor = -odorType;
                        }

                        lastHit = hit;
                        break;
                    case _ASSOCIATE_SHAPPING_TASK:
                        firstOdor = (index == 0 || index == 2) ? odor_C : odor_D;
                        secondOdor = (firstOdor == odor_C) ? odor_B : odor_A;
                        break;


                    case _ASSOCIATE_TASK:
                        firstOdor = (index == 0 || index == 2) ? odor_C : odor_D;
                        secondOdor = (index == 1 || index == 2) ? odor_A : odor_B;
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

                    case laserEachQuarter:
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
                    case laserVaryLength:
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
                }
                zxLaserTrial(laserType, firstOdor, odorLength, delay, secondOdor, WaterLen, ITI, delay_before_reward, laserCurrentTrial);
                currentTrial++;
            }
        }
        protectedSerialSend(SpSess, 0);

    }
    protectedSerialSend(SpTrain, 0); // send it's the end
}

void zxLaserTrial(int type, int firstOdor, float odorLength, float interOdorDelay, int secondOdor, float waterPeroid, int ITI, float delay_before_reward, int laserOnTrial) {
    timerCounterJ = 0;
    protectedSerialSend(Sptrialtype, type);
    protectedSerialSend(Splaser, laserOnTrial);
    assertLaser(type, fourSecBeforeFirstOdor, laserOnTrial);
    wait_ms(1000);
    assertLaser(type, threeSecBeforeFirstOdor, laserOnTrial);
    wait_ms(2000);
    assertLaser(type, oneSecBeforeFirstOdor, laserOnTrial);
    wait_ms(800);
    assertLaser(type, at200mSBeforeFirstOdor, laserOnTrial);
    wait_ms(200);
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
    wait_ms(odorLength * 1000);
    Valve_OFF(firstOdor);
    Out2 = 0;
    Nop();
    Nop();
    Nop();
    Out3 = 0;
    Nop();
    Nop();
    Nop();
    lcdWriteChar('d', 4, 1);
    assertLaser(type, atFirstOdorEnd, laserOnTrial);
    protectedSerialSend(firstSend, 0);
    if (interOdorDelay < 1.0) {
        wait_ms(interOdorDelay * 1000);
    } else {
        ///////////-inter odor interval-/////////////////
        assertLaser(type, atDelayBegin, laserOnTrial);
        wait_ms(500);
        assertLaser(type, atDelay_5SecIn, laserOnTrial);
        wait_ms(500);
        assertLaser(type, atDelay1SecIn, laserOnTrial);
        if (interOdorDelay < 4) {
            wait_ms(interOdorDelay * 1000 - 2000);
        } else {
            wait_ms(500);
            assertLaser(type, atDelay1_5SecIn, laserOnTrial);
            wait_ms(500);
            assertLaser(type, atDelay2SecIn, laserOnTrial);
            wait_ms(interOdorDelay * 500 - 2500);
            assertLaser(type, atDelay_5ToMiddle, laserOnTrial);
            wait_ms(500);
            assertLaser(type, atDelayMiddle, laserOnTrial);
            wait_ms(interOdorDelay * 500 - 2500);
            assertLaser(type, atDelayLast2_5SecBegin, laserOnTrial);
            wait_ms(500);
            assertLaser(type, atDelayLast2SecBegin, laserOnTrial);
            wait_ms(500);
            assertLaser(type, atDelayLast1_5SecBegin, laserOnTrial);
            wait_ms(500);
        }
        assertLaser(type, atDelayLastSecBegin, laserOnTrial);
        wait_ms(500);
        assertLaser(type, atDelayLast500mSBegin, laserOnTrial);
        wait_ms(300);
        assertLaser(type, atDelayLast200mSBegin, laserOnTrial);
        wait_ms(200);
    }


    assertLaser(type, atSecondOdorBeginning, laserOnTrial);
    if (taskType == _GONOGO_LR_TASK || taskType == _GONOGO_TASK || taskType == _GONOGO_LR_TRAIN) {
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
        wait_ms(odorLength * 1000);
        Valve_OFF(secondOdor);
        lcdWriteChar('D', 4, 1);
        assertLaser(type, atSecondOdorEnd, laserOnTrial);
        Out2 = 0;
        Nop();
        Nop();
        Nop();
        Out3 = 0;
        Nop();
        Nop();
        Nop();
        protectedSerialSend(secondSend, 0);
        ////////-delay before reward-///////
    }
    wait_ms(delay_before_reward * 1000);
    assertLaser(type, atRewardBeginning, laserOnTrial);
    lcdWriteChar('R', 4, 1);

    //Assess Performance here
    waterNResult(firstOdor, secondOdor, waterPeroid);

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
    wait_ms(1000);
    assertLaser(type, atITIOneSecIn, laserOnTrial);
    wait_ms(ITI * 1000 - 5000); //another 4000 is at the beginning of the trials.
    protectedSerialSend(SpITI, 0);
}

void delaymSecKey(unsigned int N) {
    timerCounterI = 0;
    while (timerCounterI < N) {
        Key_Event();
        if (hardwareKey == 1 || u2Received > 0) {
            laserTimerOn = (laserTimerOn + 1) % 2;
            hardwareKey = 0;
            u2Received = -1;
        }
        int tt = timerCounterI;
        while (timerCounterI < tt + 300) {

            Nop();
        }
    }
}

void odorDepeltion(int totalTrial) {
    initZXTMR();
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
        delaymSecKey(15000);
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
        delaymSecKey(15000);
        Valve_OFF(6);
        Nop();
        Nop();
        Valve_OFF(3);
    }
}

void feedWaterCage() {
    splash("Enter Times", "");
    int counter = 0;
    int wasLicking = 0;

    while (1) {
        if (licking && !wasLicking) {
            wasLicking = 1;
            Valve_ON(1, fullduty);
            lcdWriteNumber(++counter, 4, 1, 2);
        } else if (wasLicking && !licking) {

            Valve_OFF(1);
        }
        wait_ms(50);
    }
}

void osci() {
    unsigned int ramps[] = {2000, 2000, 1000, 500, 250, 0};
    ramp = ramps[getFuncNumber(1, "RP 2 1 .5 .25 0")];

    splash("Ramping", "  /20           ");
    lcdWriteNumber(pwmFreqLo, 3, 8, 2);
    lcdWriteNumber(pwmFreqHi, 3, 13, 2);
    protectedSerialSend(SpValve8, pwmFreqLo);
    protectedSerialSend(SpValve8, pwmFreqHi);

    int trial;
    for (trial = 0; trial < 20; trial++) {

        lcdWriteNumber(ramp, 4, 10, 1);
        lcdWriteNumber(trial + 1, 2, 1, 2);
        wait_ms(4000);
        turnOnLaser();
        wait_ms(2000);
        turnOffLaser();
        wait_ms(2000);
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
    int lickCounterL = 0;
    int lickCounterR = 0;
    int lastLocation = 0;
    splash("L       R", "Last Port");
    while (1) {
        if (licking == _LICKING_LEFT && lastLocation != _LICKING_LEFT) {
            Valve_ON(1, fullduty);
            lcdWriteNumber(++lickCounterL, 4, 3, 1);
            lcdWriteChar('L', 11, 2);
            lastLocation = _LICKING_LEFT;
            wait_ms(waterLength);
            Valve_OFF(1);

        } else if (licking == _LICKING_RIGHT && lastLocation != _LICKING_RIGHT) {

            Valve_ON(4, fullduty);
            lcdWriteNumber(++lickCounterR, 4, 11, 1);
            lcdWriteChar('R', 11, 2);
            lastLocation = _LICKING_RIGHT;
            wait_ms(waterLength);
            Valve_OFF(4);

        }
    }
}

void feedWaterNew(void) {
    int waterLength = 1000;
    int wasLicking = 0;
    int lickCounter = 0;
    splash("Total Lick", "");
    while (1) {
        if (licking && !wasLicking) {
            timerCounterI = 0;
            Valve_ON(water_sweet, fullduty);
            wasLicking = 1;
            lickCounter++;
            lcdWriteNumber(lickCounter, 4, 12, 1);
        } else if (wasLicking && !licking) {
            wasLicking = 0;
        }
        if (timerCounterI > waterLength) {

            Valve_OFF(water_sweet);
        }
    }
}

void feedWaterFast(int waterLength) {

    int wasLicking = 0;
    int lickCounter = 0;
    splash("Total Lick", "");

    timerCounterI = 1000;
    while (1) {
        if (licking && !wasLicking) {
            if (timerCounterI >= 500) {
                Valve_ON(water_sweet, fullduty);
                //                Nop();
                //                Nop();
                //                Valve_ON(8, fullduty);
                timerCounterI = 0;
            }
            wasLicking = 1;
            lickCounter++;
            lcdWriteNumber(lickCounter, 4, 12, 1);
        } else if (wasLicking && !licking) {
            wasLicking = 0;
        }
        if (timerCounterI >= waterLength) {

            Valve_OFF(water_sweet);
            //            Nop();
            //            Nop();
            //            Valve_OFF(8);
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
    timerCounterI = 0;
    while (time > 0 && timerCounterI < time) {
    }
}

void variableVoltage() {
    laserTimerOn = 1;
    char VString[] = {'1', '2', '7', ' ', ' ', ' ', '-', '-', ' ', '-', ' ', '+', ' ', '+', '+', ' '};
    int tmpFreq;
    tmpFreq = pwmFreqLo;

    while (1) {
        tmpFreq = (tmpFreq > 254) ? 254 : tmpFreq;
        tmpFreq = (tmpFreq < 0) ? 0 : tmpFreq;
        pwmFreqLo = tmpFreq;
        protectedSerialSend(SpValve8, pwmFreqLo);
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
                write_eeprom(m == 1 ? _EEP_FREQ_LOW_OFFSET : _EEP_FREQ_HIGH_OFFSET, tmpFreq);
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

void shiftingLaser(void) {
    while (1) {

        Out1 = 1;
        Nop();
        Nop();
        Out2 = 1;
        Nop();
        Nop();
        Out3 = 1;
        Nop();
        Nop();
        Out4 = 1;
        Nop();
        Nop();
        Out5 = 1;
        Nop();
        Nop();
        Out6 = 1;
        wait_ms(3000);
        Out1 = 0;
        Nop();
        Nop();
        Out2 = 0;
        Nop();
        Nop();
        Out3 = 0;
        Nop();
        Nop();
        Out4 = 0;
        Nop();
        Nop();
        Out5 = 0;
        Nop();
        Nop();
        Out6 = 0;
        wait_ms(20000);
    }
}

void testValve(void) {
    int v = getFuncNumber(1, "Valve No?");
    while (1) {

        Valve_ON(v, fullduty);
        wait_ms(1000);
        Valve_OFF(v);
        wait_ms(1000);
    }
}

void testVolume(void) {
    int n = getFuncNumber(1, "Valve #");
    int i;
    for (i = 0; i < 100; i++) {

        Valve_ON(n, fullduty);
        wait_ms(50);
        Valve_OFF(n);
        wait_ms(450);
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

void callFunction(int n) {
    currentMiss = 0;
    pwmFreqLo = read_eeprom(_EEP_FREQ_LOW_OFFSET);
    pwmFreqHi = read_eeprom(_EEP_FREQ_HIGH_OFFSET);
    if (pwmFreqHi < 0 || pwmFreqHi > 254) {
        pwmFreqHi = 127;
    }

    if (pwmFreqLo < 0 || pwmFreqLo > 254) {
        pwmFreqLo = 127;
    }
    PORTCbits.RC1 = 1;
    initZXTMR();
    switch (n) {
            int m;
            //ZX's functions
        case 4311:
            testVolume();
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
            laserTrialType = laserVaryLength;
            ramp = 500;
            zxLaserSessions(setType(), laserDuringDelay, 5, 10, 20, 0.05, 15, 25, 1.0);
            ramp = 0;
            break;

        case 4315:
            splash("Each Quarter", "Delay Laser");
            laserTrialType = laserEachQuarter;
            ramp = 500;
            zxLaserSessions(setType(), laserDuringDelay, 8, 16, 20, 0.05, 15, 25, 1.0);
            ramp = 0;
            break;
        case 4321:
            //            setLaser();
            taskType = _DNMS_TASK;
            splash("NoDelay Control", "For DNMS");
            laserTrialType = _LASER_OTHER_TRIAL;
            m = getFuncNumber(1, "Short Long B+R");
            switch (m) {
                case 1: m = laserNoDelayControlShort;
                    break;
                case 2: m = laserNoDelayControl;
                    break;
                case 3: m = laserDuringBaseAndResponse;
                    break;
            }
            zxLaserSessions(setType(), m, 0.2, 5, 20, 0.05, 20, 20, 1.0);
            break;
        case 4322:
            variableVoltage();
            break;
        case 4324:
            testPort();
            break;
        case 4325:
            splash("BaseLine", "Control");
            zxLaserSessions(setType(), laserDuringBaseline, 4, 8, 20, 0.05, 50, 30, 1.0);
            break;
        case 4331:
            splash("BaseLine+Delay", "Control");
            zxLaserSessions(setType(), laserDuringBaselineNDelay, 4, 8, 20, 0.05, 50, 30, 1.0);
            break;

        case 4332:
            splash("Odor", "Control");
            laserTrialType = _LASER_OTHER_TRIAL;
            m = getFuncNumber(1, "1st 2nd BothOdor");
            zxLaserSessions(setType(), m == 1 ? laserDuring1stOdor : m == 2 ? laserDuring2ndOdor : laserDuringOdor, 5, 10, 20, 0.05, 50, 30, 1.0);
            break;
        case 4333:
            test_Laser();
            break;


        case 4334:
            splash("DNMS Shaping", "");
            laserTrialType = _LASER_NO_TRIAL;
            taskType = _SHAPPING_TASK;
            zxLaserSessions(3, laserDuringDelay, 4, 8, 20, 0.5, 50, 20, 1.0);
            break;

        case 4335:
        {
            splash("DNMS 4s", "DC laser,");
            taskType = _DNMS_TASK;
            zxLaserSessions(3, laserDuringDelay, 4, 8, 20, 0.5, 50, setSessionNum(), 1.0);
            break;
        }
        case 4341:
            splash("DNMS 5s Shaping", "RQ");
            laserTrialType = _LASER_NO_TRIAL;
            taskType = _SHAPPING_TASK;
            zxLaserSessions(3, laserDuringDelayChR2, 5, 10, 20, 0.05, 50, setSessionNum(), 1.0);
            break;

        case 4342:
        {
            splash("DNMS 5s Water+", "RQ DC laser");
            laserTrialType = _LASER_EVERY_TRIAL;
            taskType = _DNMS_TASK;
            zxLaserSessions(3, laserDuringDelayChR2, 5, 10, 20, 0.1, 50, setSessionNum(), 1.0);
            break;
        }
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
            splash("DNMS 8s", "RQ DC laser");
            laserTrialType = _LASER_EVERY_TRIAL;
            taskType = _DNMS_TASK;
            zxLaserSessions(3, laserDuringDelayChR2, 8, 16, 20, 0.05, 50, setSessionNum(), 1.0);
            break;
        }


        case 4351:
            splash("Rule Match Shap", "C-B D-A");
            laserTrialType = _LASER_NO_TRIAL;
            taskType = _ASSOCIATE_SHAPPING_TASK;
            zxLaserSessions(0, laserDuringDelayChR2, 5, 10, 20, 0.05, 50, setSessionNum(), 1.0);
            break;

        case 4352:
            splash("Rule Match", "C-B D-A");
            laserTrialType = _LASER_NO_TRIAL;
            taskType = _ASSOCIATE_TASK;
            zxLaserSessions(0, laserDuringDelayChR2, 5, 10, 20, 0.05, 50, setSessionNum(), 1.0);
            break;

        case 4353:
        {
            splash("DNMS 5s +-", "RQ DC laser");
            laserTrialType = _LASER_OTHER_TRIAL;
            taskType = _DNMS_TASK;
            zxLaserSessions(3, laserDuringDelayChR2, 5, 10, 20, 0.05, 50, setSessionNum(), 1.0);
            break;
        }

        case 4354:
            osci();
            break;

        case 4355:
        {
            splash("DNMS 8s", "RQ DC laser");
            laserTrialType = _LASER_OTHER_TRIAL;
            taskType = _DNMS_TASK;
            zxLaserSessions(3, laserDuringDelayChR2, 8, 16, 20, 0.05, 50, setSessionNum(), 1.0);
            break;
        }

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
            zxLaserSessions(setType(), laserDuringDelay, delay, delay * 2, 20, 0.05, 50, setSessionNum(), 1.0);
            break;
        }






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

        case 4426:
            shiftingLaser();
            break;

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
            m = getFuncNumber(1, "No Pre Odor Post");
            int laserPeriod;
            laserPeriod = laserDuring4Baseline;
            switch (m) {
                case 1:
                    taskType = _GONOGO_LR_TRAIN;
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
            splash("DNMS LR LEARN", "Laser EveryTrial");
            taskType = _DNMS_LR_TASK;
            zxLaserSessions(setType(), laserDuringDelay, 4, 8, 20, 0.1, 200, 10, 1.0);
            break;
        }

        case 4434:
        {
            splash("DNMS LR", "Laser EveryTrial");
            taskType = _DNMS_LR_TASK;
            zxLaserSessions(setType(), laserDuringDelay, 4, 8, 20, 0.05, 400, 20, 1.0);
            break;
        }
        case 4435:
        {
            splash("Baseline LR", "Laser EveryTrial");
            taskType = _DNMS_LR_TASK;
            zxLaserSessions(setType(), laserDuring4Baseline, 4, 8, 20, 0.05, 400, 20, 1.0);
            break;
        }

        case 4441:
        {
            splash("First Odor LR", "Laser EveryTrial");
            taskType = _DNMS_LR_TASK;
            zxLaserSessions(setType(), laserDuring1stOdor, 4, 8, 20, 0.05, 400, 20, 1.0);
            break;
        }
        case 4442:
        {
            splash("Second Odor LR", "Laser EveryTrial");
            taskType = _DNMS_LR_TASK;
            zxLaserSessions(setType(), laserDuring2ndOdor, 4, 8, 20, 0.05, 400, 20, 1.0);
            break;
        }
        case 4443:
        {
            splash("Response Delay LR", "Laser EveryTrial");
            taskType = _DNMS_LR_TASK;
            zxLaserSessions(setType(), laserDuringResponseDelay, 4, 8, 20, 0.05, 400, 20, 1.0);
            break;
        }

        case 4444:
            feedWaterNew();
            break;

        case 4445:
            feedWaterLR(50);
            break;
    }
}

