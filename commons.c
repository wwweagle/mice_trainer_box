#include "commons.h"
#include "zxlib.h"

unsigned int period;

unsigned int ones, tens, hunds, thous;
volatile unsigned int timerCounterI = 0, timerCounterJ = 0;
unsigned char key_val;
int u2Received = -1;

unsigned char hardwareKey;
unsigned char hardwareKeyPressed;
unsigned char LCD_num[] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x2A, 0X23};


unsigned int hit = 0, miss = 0, falseAlarm = 0, correctRejection = 0, correctRatio = 0;
int currentMiss = 0;

void safe_wait_ms(int duration) {
    int i;
    while (duration-- > 0) {
        for (i = 0; i < 1000; i++);
    }
}

void __attribute__((__interrupt__, no_auto_psv)) _T2Interrupt(void) {
    IFS0bits.T2IF = 0;
    timerCounterI++;
    timerCounterJ++;
}

//void __attribute__((__interrupt__, no_auto_psv)) _T3Interrupt(void) {
//    IFS0bits.T3IF = 0;
//    key_port |= 0x70;
//    row4 = 0;
//    if (col2 == 0) {
//        //        localSendOnce(SpTrain, 127);
//        asm("RESET");
//    }
//}

void __attribute__((interrupt, no_auto_psv)) _U2RXInterrupt(void) {
    if (U2STAbits.OERR == 1) {
        U2STAbits.OERR = 0;
    }
    if (0x2a == (u2Received = U2RXREG)) {
        //        localSendOnce(SpTrain, 127);
        protectedSerialSend(SpSess, 0);
        safe_wait_ms(50);
        asm("RESET");
    }
    IFS1bits.U2RXIF = 0;
    //    localSendOnce(1, received);
}

void __attribute__((interrupt, no_auto_psv)) _U2TXInterrupt(void) {
    IFS1bits.U2TXIF = 0;
}

void __attribute__((__interrupt__, no_auto_psv)) _PWMInterrupt(void) {
    IFS2bits.PWMIF = 0;
}

void InitTMR(void) {
    TMR2 = 0;
    PR2 = TMR_RATE / 1000;
    IFS0bits.T2IF = 0;
    IEC0bits.T2IE = 1;
    T2CON = 0x8020;
    ConfigIntTimer2(T2_INT_PRIOR_5 & T2_INT_ON);

//    InitTMR3();
    Init_Keypad();
    //    Valve_ON(8, period + period / 2);
}

//void InitTMR3(void) {
//    TMR3 = 0;
//    PR3 = 65535;
//    IFS0bits.T3IF = 0;
//    IEC0bits.T3IE = 1;
//    T3CON = 0x8020;
//    ConfigIntTimer3(T3_INT_PRIOR_4 & T3_INT_ON);
//}

void Init_LCD(void) {
    safe_wait_ms(15);

    DATA &= 0xFF00;
    RW = 0;
    RS = 0;
    E = 0;
    /* set data and control pins to outputs */
    TRIS_DATA &= 0xFF00;
    RW_TRIS = 0;
    RS_TRIS = 0;
    E_TRIS = 0;


    /* 1st LCD initialization sequence */
    DATA &= 0xFF00;
    DATA |= 0x0038;
    E = 1;
    Nop();
    Nop();
    Nop();
    E = 0;
    safe_wait_ms(5);

    /* 2nd LCD initialization sequence */
    DATA &= 0xFF00;
    DATA |= 0x0038;
    E = 1;
    Nop();
    Nop();
    Nop();
    E = 0;
    DelayNuSec(200);

    /* 3rd LCD initialization sequence */
    DATA &= 0xFF00;
    DATA |= 0x0038;
    E = 1;
    Nop();
    Nop();
    Nop();
    E = 0;
    DelayNuSec(200);

    lcd_cmd(0x38);
    lcd_cmd(0x06);
    lcd_cmd(0x0c);

}

void lcd_cmd(char cmd) {
    RS = 0;
    RW = 0;
    Nop();
    Nop();
    Nop();
    DATA &= 0xFF00;
    DATA |= cmd;
    E = 1;
    Nop();
    Nop();
    Nop();
    E = 0;
    safe_wait_ms(5);
}

void lcd_data(char data) {
    Nop();
    Nop();
    Nop();
    RS = 1;
    RW = 0;
    Nop();
    Nop();
    Nop();
    DATA &= 0xFF00;
    DATA |= data;
    E = 1;
    Nop();
    Nop();
    Nop();
    E = 0;
    RS = 0;
    DelayNuSec(400);
}

void puts_lcd(unsigned char *data, unsigned char count) {
    while (count) {
        lcd_data(*data++);
        count--;
    }
}

void LCD_set_xy(unsigned int x, unsigned int y) {
    lcd_cmd((y == 1 ? 0x80 : 0xc0) - 1 + x);
}

void Init_Keypad(void) {
    key_tris = 0x0F;
    key_port |= 0x00F0;
    hardwareKey = 0;
    hardwareKeyPressed = 0;
    key_val = 0;
}

void Check_Key(void) {
    unsigned int tmp;

    key_port |= 0x00F0;
    Nop();
    row1 = 0;
    Nop();
    tmp = key_port;
    if ((tmp & 0x000F) < 0x000F) {
        if (col1 == 0)
            key_val = 1;
        else if (col2 == 0)
            key_val = 2;
        else if (col3 == 0)
            key_val = 3;
    }

    key_port |= 0x00F0;
    Nop();
    row2 = 0;
    Nop();
    tmp = key_port;
    if ((tmp & 0x000F) < 0x000F) {
        if (col1 == 0)
            key_val = 4;
        else if (col2 == 0)
            key_val = 5;
        else if (col3 == 0)
            key_val = 6;
    }


    key_port |= 0x00F0;
    Nop();
    row3 = 0;
    Nop();
    tmp = key_port;
    if ((tmp & 0x000F) < 0x000F) {
        if (col1 == 0)
            key_val = 7;
        else if (col2 == 0)
            key_val = 8;
        else if (col3 == 0)
            key_val = 9;


    }


    key_port |= 0x00F0;
    Nop();
    row4 = 0;
    Nop();
    tmp = key_port;
    if ((tmp & 0x000F) < 0x000F) {
        if (col1 == 0)
            key_val = 0;
        else if (col2 == 0) {
            asm("RESET");
        } else if (col3 == 0)
            key_val = 0;
    }
    safe_wait_ms(300);

}

void Key_Event(void) {
    unsigned int tmp;
    key_port &= 0xFF0F;
    tmp = key_port;
    if ((hardwareKeyPressed == 0) && ((tmp & 0x000F) != 0x000F)) {
        hardwareKeyPressed = 1;
        safe_wait_ms(15);
        Check_Key();

    } else if ((hardwareKeyPressed == 1) && ((tmp & 0x000F) == 0x000F)) {
        hardwareKeyPressed = 0;
        hardwareKey = 1;

    } else {
        Nop();
    }
}

void SetupPorts(void) {
    ADPCFG = 0xFFFF;

    PORTA = 0;
    TRISA = 0xF9FF;
    PORTB = 0;
    TRISB = 0x000F;
    PORTC = 0;
    TRISC = 0xFFF5;
    PORTD = 0;
    TRISD = 0xFFFF;
    PORTE = 0;
    TRISE = 0xFCFF;
    PORTF = 0;
    TRISF = 0xFF3C;
    PORTG = 0;
    TRISG = 0xFC34;
}

void InitUART2(void) {
    unsigned int baudvalue;
    unsigned int U2MODEvalue;
    unsigned int U2STAvalue;
    CloseUART2();
    ConfigIntUART2(UART_RX_INT_EN & UART_RX_INT_PR6 &
            UART_TX_INT_DIS & UART_TX_INT_PR2);
    U2MODEvalue = UART_EN & UART_IDLE_CON &
            UART_DIS_WAKE & UART_DIS_LOOPBACK &
            UART_DIS_ABAUD & UART_NO_PAR_8BIT&
            UART_1STOPBIT;

    U2STAvalue = UART_INT_TX_BUF_EMPTY &
            UART_TX_PIN_NORMAL &
            UART_TX_ENABLE & UART_INT_RX_CHAR &
            UART_ADR_DETECT_DIS &
            UART_RX_OVERRUN_CLEAR;

    baudvalue = ((FCY / 16) / BAUDRATE) - 1;
    OpenUART2(U2MODEvalue, U2STAvalue, baudvalue);

    int i;
    for (i = 0; i < 4; i++) {
        while (BusyUART2());
        U2TXREG = i;
    }
}

void Puts_UART2(unsigned char *data, unsigned char count) {
    while (count--) {
        while (BusyUART2());
        U2TXREG = *data++;
    }
}

void Init_PWM(void) {
    unsigned int config;
    unsigned int sptime;
    unsigned int config1;
    unsigned int config2;
    unsigned int config3;

    config = (PWM_INT_EN & PWM_FLTA_DIS_INT & PWM_INT_PR1
            & PWM_FLTA_INT_PR0
            & PWM_FLTB_DIS_INT & PWM_FLTB_INT_PR0);
    ConfigIntMCPWM(config);


    SetMCPWMDeadTimeAssignment(PWM_DTS1A_UA &
            PWM_DTS1I_UB & PWM_DTS2A_UA &
            PWM_DTS2I_UB);

    SetMCPWMDeadTimeGeneration(PWM_DTBPS8 &
            PWM_DTA54 & PWM_DTAPS8 & PWM_DTB54);

    period = 0x7e;
    sptime = 0x0;
    config1 = (PWM_EN & PWM_IDLE_STOP & PWM_OP_SCALE16 &
            PWM_IPCLK_SCALE4 & PWM_MOD_FREE);

    config2 = (PWM_PEN4L & PWM_PEN3H & PWM_PEN2H & PWM_PEN1H &
            PWM_PEN4H & PWM_PEN3L & PWM_PEN2L & PWM_PEN1L &
            PWM_MOD1_IND & PWM_MOD2_IND & PWM_MOD3_IND &
            PWM_MOD4_IND);

    config3 = (PWM_SEVOPS1 & PWM_OSYNC_PWM & PWM_UEN);
    OpenMCPWM(period, sptime, config1, config2, config3);

#define FULLDUTY 0xfe

    PORTFbits.RF0 = 1;
    Nop();
    Nop();
//    PORTCbits.RC1 = 1;
//    Nop();
//    Nop();
    PDC2 = FULLDUTY;
//    Nop();
//    Nop();
//    PDC4 = FULLDUTY; //ZX 20160503 TEMP Distractor work around-
}
#undef FULLDUTY

void Valve_ON(unsigned int N, unsigned int rate) {
    switch (N) {
        case 1: PORTGbits.RG0 = 1;
            PDC1 = rate;
            break;
        case 2: PORTFbits.RF1 = 1;
            PDC1 = rate;
            break;
        case 3: PORTGbits.RG1 = 1;
            PDC2 = rate;
            break;
        case 4: PORTFbits.RF0 = 1;
            PDC2 = rate;
            break;
        case 5: PORTGbits.RG7 = 1;
            PDC3 = rate;
            break;
        case 6: PORTCbits.RC3 = 1;
            PDC3 = rate;
            break;
        case 7: PORTGbits.RG6 = 1;
            PDC4 = rate;
            break;
        case 8: PORTCbits.RC1 = 1;
            PDC4 = rate;
            break;
        default:
            break;
    }
}

void Valve_OFF(unsigned int N) {
    switch (N) {
        case 1: PORTGbits.RG0 = 0;
            break;
        case 2: PORTFbits.RF1 = 0;
            break;
        case 3: PORTGbits.RG1 = 0;
            break;
        case 4: PORTFbits.RF0 = 0;
            break;
        case 5: PORTGbits.RG7 = 0;
            break;
        case 6: PORTCbits.RC3 = 0;
            break;
        case 7: PORTGbits.RG6 = 0;
            break;
        case 8: PORTCbits.RC1 = 0;
            break;
        default:
            break;
    }
}

void DelayNuSec(unsigned int n) {
#ifndef DEBUG
    int i;
    for (i = 0; i < n; i++);
#endif
}

void localSendOnce(int thisSp, int thisValue) {
    static unsigned char UartSend[] = {0x55, 0x00, 0x00, 0xAA};
    UartSend[1] = thisSp;
    UartSend[2] = thisValue;
    Puts_UART2(UartSend, sizeof (UartSend));
}

void counts_dispose(unsigned int counter) {
    ones = 0;
    tens = 0;
    hunds = 0;
    thous = 0;

    while (counter > 0) {
        if (counter > 999) {
            thous++;
            counter -= 1000;
        } else if (counter > 99) {
            hunds++;
            counter -= 100;
        } else if (counter > 9) {
            tens++;
            counter -= 10;
        } else {
            ones++;
            counter--;

        }
    };

    ones += 0x30;
    tens += 0x30;
    hunds += 0x30;
    thous += 0x30;

}

unsigned int getFuncNumber(int targetDigits, char* input) {
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
    lcdWriteString(input);

    for (iter = 0; iter < targetDigits; iter++) {
        while (!bitSet[iter]) {
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
        home_clr();
        lcdWriteString(input);
        line_2();
        for (iter1 = 0; iter1 < targetDigits; iter1++) {
            lcd_data(bitValue[iter1] + 0x30);
        }
        safe_wait_ms(300);
    }
    n = 0;
    for (iter1 = 0; iter1 < targetDigits; iter1++) {
        n = n * 10 + bitValue[iter1];
    }
    return n;
#endif
}


#ifdef CQ_lickOrAll

//THIS IS BAD PRACTICE, YOU SHOULD AVOID USE THIS IF POSSIBLE
//ONLY FOR BACKWARD COMPATIBILITY WITH CQ
void DelayNmSecNew(unsigned int N) {
#ifdef DEBUG
    return;
#endif
    timerCounterJ = 0;
    while (timerCounterJ < N) {
    }
}
#endif

