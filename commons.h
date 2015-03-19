/* 
 * File:   commons.h
 * Author: Administrator
 *
 * Created on 2013?8?24?, ??11:05
 */

#ifndef COMMONS_H
#define	COMMONS_H

#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

#include <p30f6010A.h>
#include <uart.h>
#include <pwm.h>
#include <ports.h>
#include <timer.h>
#include <spi.h>
#include <stdlib.h>
#include <stdio.h>
#include <xlcd.h>
#include <time.h>
#include <string.h>


#define PCBGen "3Gen"
#define FCY  8000000        
#define MILLISEC FCY/8000			
#define uSEC FCY/8000/1000
#define BAUDRATE 9600
#define TMR_RATE	FCY/64			
#define HALF_SEC	TMR_RATE/2		
#define SEC	TMR_RATE



#define key_port PORTB    
#define key_tris TRISB
#define row1     PORTBbits.RB4  
#define row2     PORTBbits.RB5
#define row3     PORTBbits.RB6
#define row4     PORTBbits.RB7

#define col1     PORTBbits.RB0  
#define col2     PORTBbits.RB1
#define col3     PORTBbits.RB2
#define col4     PORTBbits.RB3
#define RS	LATFbits.LATF8 
#define RW	LATFbits.LATF2 
#define E	LATFbits.LATF3

#define DATA	LATD
#define TRIS_DATA  TRISD


#define set_function()	lcd_cmd( 0x38 ) 
#define cursor_right()  lcd_cmd( 0x14 ) 
#define cursor_left()   lcd_cmd( 0x10 ) 
#define display_shift() lcd_cmd( 0x1C ) 
#define home_clr()      lcd_cmd( 0x01 ) 
#define home_it()       lcd_cmd( 0x02 ) 
#define line_2()        lcd_cmd( 0xC0 ) 
#define RW_TRIS	        TRISFbits.TRISF2
#define RS_TRIS	        TRISFbits.TRISF8
#define E_TRIS		    TRISFbits.TRISF3


#define Infrared_LED1   PORTDbits.RD14
#define Infrared_LED2   PORTDbits.RD15

#define LickOnPin  PORTDbits.RD12
#define Lick2  PORTDbits.RD13


#define Out1 PORTGbits.RG8
#define Out2 PORTGbits.RG9
#define Out3 PORTEbits.RE8
#define Out4 PORTEbits.RE9
#define Out5 PORTAbits.RA9
#define Out6 PORTAbits.RA10
#define Out7 PORTAbits.RA9
#define Out8 PORTAbits.RA10

#define  water_sweet    1    //pwm1L		//same as old definition
#define  odor_A           2       //pwm1H		//same as old definition
#define  odor_B           3       //pwm2l		//same as old definition
#define  water_bitter   4    //pwm2h		//same as old definition
#define  odor_C           5
#define  odor_D           6
#define  odor_E           7
#define  odor_F           8


// serial port

//  define status: valve
// every status change must be registered in the send word, otherwise it will be incorrect
// first word is for licking and coming, lick: 0x01; not lick: 0x00
// position pointer cannot be 0x55 and  0xAA?
#define SpLick 	 0 	//        !PORTDbits.RD8  // ???????P2??????P4???D27?
#define SpCome 	 1  	// Head fixed, always come
#define SpLick2	 2   // lick2    //!PORTDbits.RD9  // ???????P6??????P7???D28?#define Come 1
#define SpCome2	 3   	// Head fixed, always come
// the second word is for performance, false alarm,
#define SpFalseAlarm		 4	// false alarm
#define SpCorrectRejection		 5	// correct rejection
#define SpMiss		 6	// miss
#define SpHit		 7 	// hit
// the third word is for the first set of valves
#define SpWater_sweet	 8     // 1    //pwm1L		//same as old definition
#define SpOdor_A             9         // #define  2    //pwm1H		//same as old definition
#define SpOdor_B	10    //      3    //pwm2l		//same as old definition
#define SpWater_bitter	 11   // 4    //pwm2h		//same as old definition
// the fourth word is for the second set of vales
#define SpValve5	12
#define SpValve6 	13
#define SpValve7 	14
#define SpValve8 	15  // overall eight valves in the current design, may be more later


// the fifth word is for the output value for
#define SpOut1		16  // PORTBbits.RB12	// lick (1ms) or come (2ms)
#define SpOut2	 	17  // PORTBbits.RB13	// odor A on (1ms) and off (2ms)
#define SpOut3	 	18  // PORTBbits.RB14	// odor B on (1ms) and off (2ms)
#define SpOut4		19 // PORTBbits.RB15	// normal water on (1 ms) or off (2ms), bitter water on (3ms) or off (4ms)


//press key, s1s1 through s5s5
#define SpS1S1 	 20
#define SpS1S2   21
#define SpS1S3   22
#define SpS1S4	 23
#define SpS1S5   24
#define SpS2S1 	 25
#define SpS2S2   26
#define SpS2S3   27
#define SpS2S4	 28
#define SpS2S5   29
#define SpS3S1 	 30
#define SpS3S2   31
#define SpS3S3   32
#define SpS3S4	 33
#define SpS3S5   34
#define SpS4S1 	 35
#define SpS4S2   36
#define SpS4S3   37
#define SpS4S4	 38
#define SpS4S5   39
#define SpS5S1 	 40
#define SpS5S2   41
#define SpS5S3   42
#define SpS5S4	 43
#define SpS5S5   44


#define SpStepN         51
#define SpSessN         52
#define SptrialNperSess 49
#define SpSessHit       45
#define SpSessMiss      46
#define SpSessFA	    47
#define SpSessCR        48
#define SpSessCorrRate 	50

#define SpFirOdor       53
#define SpFirOdorL      54
#define SpOdorDelay     55
#define SpSecOdor       56
#define SpSecOdorL      57
#define Sptrialtype     58
#define SpITI           59  // 1 start 0 end
#define SpproLpun       60
#define SpSess          61  // 1 start 0 end
#define SpTrain         62  // 1 start 0 end
#define Splaser         65

#define SpOdor_C        66
#define SpOdor_D        64

#define SpHit2         67
#define SpMiss2           68
#define SpFalseAlarm2         69
#define SpCorrectRejection2         70

#define SpHit3         71
#define SpMiss3          72
#define SpFalseAlarm3         73
#define SpCorrectRejection3        74

#define SpHit4         75
#define SpMiss4         76
#define SpFalseAlarm4         77
#define SpCorrectRejection4        78
#define SpLaserSwitch 79

void SetupPorts(void);
void InitUART2(void);
void Init_PWM(void);
void UART2PutChar(unsigned char Ch);
void Puts_UART2(unsigned char *data, unsigned char count);
void DelayNmSec(unsigned int N);
void DelayNuSec(unsigned int N);
void Init_LCD(void);
void lcd_cmd(char cmd);
void lcd_data(char data);
void puts_lcd(unsigned char *data, unsigned char count);
void InitTMR(void);
void InitTMR3(void);
void LCD_set_xy(unsigned int x, unsigned int y);

void Valve_ON(unsigned int N, unsigned int rate);
void Valve_OFF(unsigned int N);

void localSendOnce(int thisSp, int thisValue);
void NewDigt_out(unsigned int Nchan, float Duration);

void Key_Event(void);
void Init_Keypad(void);
void Check_Key(void);

void DelayNmSecNew(unsigned int N);
void counts_dispose(unsigned int counter);

extern unsigned int period;
extern unsigned int ones, tens, hunds, thous;
extern unsigned int timerCounterI, timerCounterJ;
extern unsigned char key_val;

extern unsigned char hardwareKey;
extern unsigned char hardwareKeyPressed;
extern int u2Received;


unsigned int getFuncNumber(int targetDigits, char* input);


#endif	/* COMMONS_H */

