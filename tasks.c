#include "commons.h"
#include "tasks.h"
#include "zxLib.h"
//#include "ldLib.h"

unsigned char text7[] = {0x53, 0x5F, 0x2E, 0x5F, 0x48, 0x5F, 0x5F, 0x5F, 0x4D, 0x5F, 0x5F, 0x5F, 0x64, 0x5F, 0x5F, 0x25};//S_._H___M___d__%
unsigned char text8[] = {0x53, 0x5F, 0x5F, 0x5F, 0x46, 0x5F, 0x5F, 0x5F, 0x43, 0x5F, 0x5F, 0x5F, 0x74, 0x5F, 0x5F, 0x5F};//S___F___C___t___
//struct flagStruct Flags;
unsigned int hit = 0, miss = 0, falseAlarm = 0, correctRejection = 0, correctRatio = 0;
int currentMiss = 0;
//unsigned int ra = 0, rb = 0, rc = 0, rd = 0, re = 0;
//unsigned int ma = 0, mb = 0, mc = 0, md = 0, me = 0;
//unsigned int wa = 0, wb = 0, wc = 0, wd = 0, we = 0;
//unsigned int ca = 0, cb = 0, cc = 0, cd = 0, ce = 0;
//unsigned int ITIa = 0, ITIb = 0, ITIc = 0, ITId = 0, ITIe = 0;
//unsigned int Delay = 0, Delaya = 0, Delayb = 0, Delayc = 0, Delayd = 0, Delaye = 0;
//unsigned int totalTrials = 0;
//unsigned int cra = 0, crb = 0, crc = 0, crd = 0, cre = 0;
//unsigned int Flagsaaa = 0, Flagsaab = 0, Flagsaac = 0, Flagsaad = 0, Flagsaae = 0;
//unsigned int Flagsbba = 0, Flagsbbb = 0, Flagsbbc = 0, Flagsbbd = 0, Flagsbbe = 0;
//unsigned int Flagsaba = 0, Flagsabb = 0, Flagsabc = 0, Flagsabd = 0, Flagsabe = 0;
//unsigned int Flagsbaa = 0, Flagsbab = 0, Flagsbac = 0, Flagsbad = 0, Flagsbae = 0;
//unsigned int flaga = 0;
//unsigned int flagb = 0;
//
//void AB_BA_Go(int thisdelay, int ITI, float waterlen, float delay2) {
//    localSendOnce(SpS1S1, 1);
//    home_clr();
//    puts_lcd((unsigned char*) &text7[0], sizeof (text7));
//    line_2();
//    puts_lcd((unsigned char*) &text8[0], sizeof (text8));
//    timerCounterI = 0;
//    while (timerCounterI < 2 * 1000) {
//    }
//    while (1) {
//        int temptrial = 0;
//        while (temptrial < 21) {
//            int ValRan = rand() % 2;
//            switch (ValRan) {
//                case 0:
//                    ODNMS_Go(odor_A, 1.0, thisdelay, odor_B, 1.0, waterlen, ITI, 0, delay2);
//                    temptrial++;
//                    ODNMS_Go(odor_B, 1.0, thisdelay, odor_A, 1.0, waterlen, ITI, 0, delay2);
//                    temptrial++;
//                    break;
//                case 1:
//                    ODNMS_Go(odor_B, 1.0, thisdelay, odor_A, 1.0, waterlen, ITI, 0, delay2);
//                    temptrial++;
//                    ODNMS_Go(odor_A, 1.0, thisdelay, odor_B, 1.0, waterlen, ITI, 0, delay2);
//                    temptrial++;
//                    break;
//            }
//        };
//    };
//}
//
//void AA_BB_Go(int thisdelay, int ITI, float waterlen, float delay2) {
//    localSendOnce(SpS1S1, 1);
//    home_clr();
//    puts_lcd((unsigned char*) &text7[0], sizeof (text7));
//    line_2();
//    puts_lcd((unsigned char*) &text8[0], sizeof (text8));
//    timerCounterI = 0;
//    while (timerCounterI < 2 * 1000) {
//    }
//    while (1) {
//        int temptrial = 0;
//        while (temptrial < 21) {
//            int ValRan = rand() % 2;
//            switch (ValRan) {
//                case 0:
//                    ODNMS_Go(odor_A, 1.0, thisdelay, odor_A, 1.0, waterlen, ITI, 0, delay2);
//                    temptrial++;
//                    ODNMS_Go(odor_B, 1.0, thisdelay, odor_B, 1.0, waterlen, ITI, 0, delay2);
//                    temptrial++;
//                    break;
//                case 1:
//                    ODNMS_Go(odor_B, 1.0, thisdelay, odor_B, 1.0, waterlen, ITI, 0, delay2);
//                    temptrial++;
//                    ODNMS_Go(odor_A, 1.0, thisdelay, odor_A, 1.0, waterlen, ITI, 0, delay2);
//                    temptrial++;
//                    break;
//            }
//        };
//    };
//}
//
//void test_water(void) {
//    home_clr();
//    while (1) {
//        Valve_ON(1, period * 2 * 1);
//    }
//}
//
//void test_odorA(void) {
//    home_clr();
//    while (1) {
//        Valve_ON(odor_A, period * 2 * 1);
//        DelayNmSecNew(2000);
//        Valve_OFF(odor_A);
//        DelayNmSecNew(2000);
//    }
//}
//
//void test_odorB(void) {
//    home_clr();
//    while (1) {
//        Valve_ON(odor_B, period * 2 * 1);
//        DelayNmSecNew(2000);
//        Valve_OFF(odor_B);
//        DelayNmSecNew(2000);
//    }
//}
//
//void test_odorC(void) {
//    home_clr();
//    while (1) {
//        Valve_ON(5, period * 2 * 1);
//        DelayNmSecNew(2000);
//        Valve_OFF(5);
//        DelayNmSecNew(2000);
//    }
//}
//
//void test_odorD(void) {
//    home_clr();
//    while (1) {
//        Valve_ON(6, period * 2 * 1);
//        DelayNmSecNew(2000);
//        Valve_OFF(6);
//        DelayNmSecNew(2000);
//    }
//}
//
//void ODNMS_Go(int FirstOdorIn, float FirOdorLen, int OdorDelayIn, int SecondOdorIn, float SecOdorLen, float WaterLIn, int IntervalIn, int prolong, float delay_before_reward) {
//
//    Valve_ON(FirstOdorIn, period * 2 * 1.0);
//    if (FirstOdorIn == odor_A) localSendOnce(SpOdor_A, 1);
//    else if (FirstOdorIn == odor_B) localSendOnce(SpOdor_B, 1);
//    timerCounterI = 0;
//    while (timerCounterI < FirOdorLen * 1000) {
//        if (LickOnPin) {
//            Flags.pun = 1;
//            localSendOnce(SpLick, 1);
//        }
//        if (Flags.pun == 1 && Flags.punable == 1) {
//            Valve_ON(4, period * 2 * 1);
//            DelayNmSecNew(200);
//            Valve_OFF(4);
//            Flags.punable = 0;
//        }
//    }
//    Flags.pun = 0;
//    Flags.punable = 1;
//    Valve_OFF(FirstOdorIn);
//    if (FirstOdorIn == odor_A) localSendOnce(SpOdor_A, 0);
//    else if (FirstOdorIn == odor_B) localSendOnce(SpOdor_B, 0);
//    timerCounterI = 0;
//    while (timerCounterI < OdorDelayIn * 1000) {
//        if (LickOnPin) {
//            Flags.pun = 1;
//            localSendOnce(SpLick, 1);
//        }
//        if (Flags.pun == 1 && Flags.punable == 1) {
//            Valve_ON(4, period * 2 * 1);
//            DelayNmSecNew(200);
//            Valve_OFF(4);
//            Flags.punable = 0;
//        }
//    }
//    Flags.pun = 0;
//    Flags.punable = 1;
//    Valve_ON(SecondOdorIn, period * 2 * 1.0);
//    if (SecondOdorIn == odor_A) localSendOnce(SpOdor_A, 1);
//    else if (SecondOdorIn == odor_B) localSendOnce(SpOdor_B, 1);
//    timerCounterI = 0;
//    while (timerCounterI < FirOdorLen * 1000) {
//        if (LickOnPin) {
//            Flags.pun = 1;
//            localSendOnce(SpLick, 1);
//        }
//        if (Flags.pun == 1 && Flags.punable == 1) {
//            Valve_ON(4, period * 2 * 1);
//            DelayNmSecNew(200);
//            Valve_OFF(4);
//            Flags.punable = 0;
//        }
//    }
//    Flags.pun = 0;
//    Flags.punable = 1;
//    Valve_OFF(SecondOdorIn);
//    if (SecondOdorIn == odor_A) localSendOnce(SpOdor_A, 0);
//    else if (SecondOdorIn == odor_B) localSendOnce(SpOdor_B, 0);
//
//    timerCounterI = 0;
//    while (timerCounterI < delay_before_reward * 1000) {
//        if (LickOnPin) {
//            localSendOnce(SpLick, 1);
//        }
//        if (Flags.pun == 1 && Flags.punable == 1) {
//            Valve_ON(4, period * 2 * 1);
//            DelayNmSecNew(200);
//            Valve_OFF(4);
//            Flags.punable = 0;
//        }
//    }
//    Flags.pun = 0;
//    Flags.punable = 1;
//    Flags.lick = 0;
//    timerCounterI = 0;
//    while (timerCounterI < WaterLIn * 1000) {
//        if (LickOnPin) {
//            localSendOnce(SpLick, 1);
//            Valve_ON(water_sweet, period * 2 * 1);
//            localSendOnce(SpWater_sweet, 1);
//            Flags.lick = 1;
//        }
//    }
//    Valve_OFF(water_sweet);
//    localSendOnce(SpWater_sweet, 0);
//    if (Flags.lick) {
//        Flags.lick = 0;
//        currentMiss = 0;
//        hit++;
//        localSendOnce(SpHit, 1);
//        counts_dispose(hit);
//        LCD_set_xy(6, 1);
//        lcd_data(hunds);
//        lcd_data(tens);
//        lcd_data(ones);
//    } else {
//        currentMiss++;
//        miss++;
//        localSendOnce(SpMiss, 1);
//        counts_dispose(miss);
//        LCD_set_xy(10, 1);
//        lcd_data(hunds);
//        lcd_data(tens);
//        lcd_data(ones);
//    }
//    int totalTrials = hit + correctRejection + miss + falseAlarm;
//    counts_dispose(totalTrials);
//    LCD_set_xy(14, 2);
//    lcd_data(hunds);
//    lcd_data(tens);
//    lcd_data(ones);
//    if (hit + correctRejection > 0) {
//        correctRatio = 100 * (hit + correctRejection) / (hit + miss + correctRejection + falseAlarm);
//        counts_dispose(correctRatio);
//        LCD_set_xy(14, 1);
//        lcd_data(tens);
//        lcd_data(ones);
//    }
//    localSendOnce(SpITI, 1);
//    timerCounterI = 0;
//    while (timerCounterI < IntervalIn * 1000) {
//        if (LickOnPin) {
//            localSendOnce(SpLick, 1);
//        }
//        if (Flags.pun == 1 && Flags.punable == 1) {
//            Valve_ON(4, period * 2 * 1);
//            DelayNmSecNew(200);
//            Valve_OFF(4);
//            Flags.punable = 0;
//        }
//    }
//    Flags.pun = 0;
//    Flags.punable = 1;
//    localSendOnce(SpITI, 0);
//}
//
//void NewBurstStimOut2(int FreqStim, int DuraStim, int totalStimTime) {
//    int Interval = (1000 / FreqStim) - DuraStim;
//    int ind;
//    int NumSpikeBurst = (totalStimTime / 1000) * FreqStim;
//    for (ind = 1; ind <= NumSpikeBurst; ind++) {
//        Out3 = 1;
//        timerCounterI = 0;
//        while (timerCounterI < DuraStim) {
//            if (LickOnPin) localSendOnce(SpLick, 1);
//        }
//        Out3 = 0;
//        timerCounterI = 0;
//        while (timerCounterI < Interval) {
//            if (LickOnPin) localSendOnce(SpLick, 1);
//        }
//    }
//}
//
//void test_lick(void) {
//    Valve_ON(water_sweet, period * 2 * 1);
//    while (1) {
//        if (LickOnPin) {
//            localSendOnce(SpLick, 1);
//        }
//    }
//}

//void test_Laser(void) {
//    int i = 1;
//    while (1) {
//        Out4 = i;
//        getFuncNumber(1, "Toggle Laser");
//        i = (i + 1) % 2;
//    }
//}

