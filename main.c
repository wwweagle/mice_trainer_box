
#include "commons.h"
#include "zxLib.h"

void FuncID(int n);
_FOSC(CSW_FSCM_OFF & XT_PLL4);
_FWDT(WDT_OFF);
_FBORPOR(PBOR_ON & 0xFFFF & PWRT_64 & MCLR_EN); //0xFFFF=BORV20=BORV_20

int main(void) {

    SetupPorts();
    Init_PWM();
    InitTMR();
    Init_LCD();
    InitUART2();
    //    unsigned char StartText[] = {0x00, 0x01, 0x02, 0x03};
    //    Puts_UART2((unsigned char*) &StartText[0], sizeof (StartText));



    //    Flags.pun = 0;

    //splash("Guys, WRITE YOUR", "HEADER FILE !!!");
    splash("Mice Trainer", getVer());


    while (1) {
        unsigned int n = getFuncNumber(4, "Main Function ?");
        srand(timerCounterI);
        switch (n) {
//            case 1111:
//                n = getFuncNumber(1, " 2  5  10 30 600");
//                feedWater(n);
//                break;
                //            case 1112:
                //                test_odorA();
                //                break;
                //            case 1113:
                //                test_odorB();
                //                break;
                //            case 1115:
                //                ppLaserSessions(20, 20, 20);
                //                break;
                //            case 1122:
                //                test_lick();
                //                break;

                //            case 1124:
                //                n = getFuncNumber(1, "IntOdr 1=5s 2=2s");
                //                if (n == 2) {
                //                    AB_BA_Go(2, 5, 1.0, 0.5);
                //                } else {
                //                    AB_BA_Go(5, 10, 1.0, 0.5);
                //                }
                //                break;
                //            case 1135:
                //                AA_BB_Go(5, 10, 1.0, 0.5);
                //                break;

                ///////Ld functions


                //          case 1511 ... 1555:
                //                callLDFunction(n);
                //                break;
                /////////////CQ functions//////////////////*/
                //            case 2111 ... 2222:
                //                CQFunction(n);
                //                break;
                ////////////////LD functions/////////////
                //           case 3354:
                //               pico(1000, 1000);
                //             break;
                //            case 3355:
                //                pico(100, 900);
                //                break;
                //hz's functions
                //            case 2511 ... 2555:
                //                FuncID(n);
                //                break;
                //zx's functions
                //            case 2311 ... 2455:
                //                XW_TaskMenu(n) ;
                //                break;
            case 4300 ... 4499:
                callFunction(n);
                break;
                //            case 5111 ... 5255://gcf function
                //               GCFFunction(n);
                //               break;
                /*   case 5554:
                       cue_lyd ();
                      break;
                   case 5553:
                      n = getFuncNumber(1, "1=Const 2=Rand");
                       lydGoNogoSessions(n==1);
                       break;*/
            case 5555:
                odorDepeltion(120, 1);
                break;

            case 5556:
                odorDepeltion(120, 0);
                break;

        }
    }
    return 0;
}
