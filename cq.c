#include "commons.h"
#include "tasks.h"
#include "zxLib.h"
#include "cq.h"

int MissNum=0;
int StopCriterion=10;//Miss + CR >=10 in concensive trials
int StopSession=5;
int StopTrialNum=100; //set the maximal trial number per training day
int lickwateron =50;
int SlidingWindow = 20;
int LastSwitchTrialNum=0;

int H[600] ={0};//the size was defined by StopTrialNum
int M[600] ={0};
int F[600] ={0};
int C[600] ={0};

void CQ_Digt_out(unsigned int Nchan, unsigned int Duration){
	switch(Nchan)
	{
		case 1:
                        Out1 = 1;
                        DelayNmSecNew(Duration);
	 		Out1 = 0;
			break;
		case 2:
			Out2 = 1;
                        DelayNmSecNew(Duration);
	 		Out2 = 0;
			break;
		case 3:
			Out3 = 1;
	 		DelayNmSecNew(Duration);
	 		Out3 = 0;; //
			break;
		case 4:
			Out4 = 1;
	 		DelayNmSecNew(Duration);
	 		Out4 = 0;; //
			break;
		case 5:
			Out5 = 1;
	 		DelayNmSecNew(Duration);
	 		Out5 = 0;; //
			break;
		case 6:
			Out6 = 1;
	 		DelayNmSecNew(Duration);
	 		Out6 = 0;; //
			break;
		case 7:
			Out7 = 1;
	 		DelayNmSecNew(Duration);
	 		Out7 = 0;; //
			break;
		case 8:
			Out8 = 1;
	 		DelayNmSecNew(Duration);
	 		Out8 = 0;; //
		default:
			break;
	}
}

int CQ_Digt_on(unsigned int Nchan, unsigned int Duration){
    int IsBNCOn=0;
	switch(Nchan)
	{
		case 1:
			Out1 = 1;
                        localSendOnce(Splaser, 1);
                        IsBNCOn=1;
                        DelayNmSecNew(Duration);
			break;
		case 2:
			Out2 = 1;
                        localSendOnce(Splaser, 1);
                        IsBNCOn=1;
                        DelayNmSecNew(Duration);
			break;
		case 3:
			Out3 = 1;
                        localSendOnce(Splaser, 1);
                        IsBNCOn=1;
                        DelayNmSecNew(Duration);
			break;
		case 4:
			Out4 = 1;
                        localSendOnce(Splaser, 1);
                        IsBNCOn=1;
                        DelayNmSecNew(Duration);
			break;
		case 5:
			Out5 = 1;
                        localSendOnce(Splaser, 1);
                        IsBNCOn=1;
                        DelayNmSecNew(Duration);
			break;
		case 6:
			Out6 = 1;
                        localSendOnce(Splaser, 1);
                        IsBNCOn=1;
                        DelayNmSecNew(Duration);
			break;
		case 7:
			Out7 = 1;
                        localSendOnce(Splaser, 1);
                        IsBNCOn=1;
                        DelayNmSecNew(Duration);
			break;
		case 8:
			Out8 = 1;
                        localSendOnce(Splaser, 1);
                        IsBNCOn=1;
                        DelayNmSecNew(Duration);
                        break;
		default:
                    break;
	}
        return IsBNCOn;
}

void CQ_Digt_off(unsigned int Nchan){
	switch(Nchan)
	{
		case 1:
			Out1 = 0;
                        localSendOnce(Splaser, 1);
			break;
		case 2:
			Out2 = 0;
                        localSendOnce(Splaser, 1);
			break;
		case 3:
			Out3 = 0;
                        localSendOnce(Splaser, 1);
			break;
		case 4:
			Out4 = 0;
                        localSendOnce(Splaser, 1);
			break;
		case 5:
			Out5 = 0;
                        localSendOnce(Splaser, 1);
			break;
		case 6:
			Out6 = 0;
                        localSendOnce(Splaser, 1);
			break;
		case 7:
			Out7 = 0;
                        localSendOnce(Splaser, 1);
			break;
		case 8:
			Out8 = 0;
                        localSendOnce(Splaser, 1);
                        break;
		default:
			break;
	}
}

void CQ_Valve_ON(unsigned int N, unsigned int rate, int Nchan) {
    switch (N) {
        case 1: PORTGbits.RG0 = 1;
            CQ_Digt_out(Nchan, 1);
            PDC1 = rate;
            break;
        case 2: PORTFbits.RF1 = 1;
            CQ_Digt_out(Nchan, 1);
            PDC1 = rate;
            break;
        case 3: PORTGbits.RG1 = 1;
            CQ_Digt_out(Nchan, 1);
            PDC2 = rate;
            break;
        case 4: PORTFbits.RF0 = 1;
            CQ_Digt_out(Nchan, 1);
            PDC2 = rate;
            break;
        case 5: PORTGbits.RG7 = 1;
            CQ_Digt_out(Nchan, 1);
            PDC3 = rate;
            break;
        case 6: PORTCbits.RC3 = 1;
            CQ_Digt_out(Nchan, 1);
            PDC3 = rate;
            break;
        case 7: PORTGbits.RG6 = 1;
            CQ_Digt_out(Nchan, 1);
            PDC4 = rate;
            break;
        case 8: PORTCbits.RC1 = 1;
            CQ_Digt_out(Nchan, 1);
            PDC4 = rate;
            break;
    }
}

void CQ_LickSignalSender(int LickOrNot,int LickInterval){
    if (LickOrNot==1){
        localSendOnce(SpLick, 1);
        CQ_Digt_out(1, 1);
        Valve_ON(4, period * 2);        
        DelayNmSecNew(LickInterval);
        Valve_OFF(4);
    }
}

void CQ_OdorSPSignalSender(int Odor, int value) {
    if (Odor == odor_A) {
        localSendOnce(SpOdor_A, value);
    } else if (Odor == odor_B) {
        localSendOnce(SpOdor_B, value);
    } else if (Odor == odor_C) {
        localSendOnce(SpOdor_C, value);
    } else if (Odor == odor_D) {
        localSendOnce(SpOdor_D, value);
    }
}

void CQ_Valve_OFF(unsigned int N, int Nchan) {
    switch (N) {
        case 1: PORTGbits.RG0 = 0;CQ_Digt_out(Nchan, 1);
            break;
        case 2: PORTFbits.RF1 = 0;CQ_Digt_out(Nchan, 1);
            break;
        case 3: PORTGbits.RG1 = 0;CQ_Digt_out(Nchan, 1);
            break;
        case 4: PORTFbits.RF0 = 0;CQ_Digt_out(Nchan, 1);
            break;
        case 5: PORTGbits.RG7 = 0;CQ_Digt_out(Nchan, 1);
            break;
        case 6: PORTCbits.RC3 = 0;CQ_Digt_out(Nchan, 1);
            break;
        case 7: PORTGbits.RG6 = 0;CQ_Digt_out(Nchan, 1);
            break;
        case 8: PORTCbits.RC1 = 0;CQ_Digt_out(Nchan, 1);
            break;
    }
}  

int CQ_OneOdorJudger(int Odor, int rule) {
    if (rule == 3) {
        return 1;
    } else if (rule == 4) {
        return 0;
    } else if (rule == 1) {
        if (Odor == odor_B || Odor == odor_D ) {
            return 0;
        } else {
            return 1;
        }
    } else if (rule == 2) {
        if (Odor == odor_B || Odor == odor_D ) {
            return 1;
        } else {
            return 0;
        }
    } else if (rule == 5) {
        if (Odor == odor_B || Odor == odor_A) {
            return 0;
        } else {
            return 1;
        }
    } else if (rule == 6) {
        if (Odor == odor_B) {
            return 0;
        } else {
            return 1;
        }
    }
    return 0;
}

int TwoOdorJudger(int Odor1,int Odor2,int rule){
	if (rule==1)
	{
		if (Odor1==Odor2){return 0;}//rule==1, nonmatch rule
		else {return 1;}
	}
	else if (rule==2)
	{
		if (Odor1==Odor2){return 1;}//rule==2, match rule
		else {return 0;}
	}
	else if (rule==3)
	{
		return 1;
	}
	else if (rule==4)
	{
		if (Odor2==odor_B){return 0;}//rule==4, AA,BA go
		else {return 1;}
	}
        return 0;
}

int SlidingWindowSumation(int *PerArray,int trialnum, int SlidingWindow){

    int SumPer=0;
    int Temp;
    if (trialnum<SlidingWindow){
        for(Temp=1;Temp<=trialnum;Temp++){
        SumPer=SumPer+PerArray[Temp-1];
        }
    }else{
        for(Temp=trialnum-SlidingWindow+1;Temp<=trialnum;Temp++){
        SumPer=SumPer+PerArray[Temp-1];
        }
    }
    return(SumPer);
}

int CalculateSlidingPer(int *H,int *C,int trialnum, int SlidingWindow){

    int SumH,SumC;    
    if (trialnum<SlidingWindow){
        SumH=SlidingWindowSumation(H,trialnum,SlidingWindow);
        SumC=SlidingWindowSumation(C,trialnum,SlidingWindow);

        correctRatio=100*(SumH+SumC)/trialnum;
    }else{
        SumH=SlidingWindowSumation(H,trialnum,SlidingWindow);
        SumC=SlidingWindowSumation(C,trialnum,SlidingWindow);

        correctRatio=100*(SumH+SumC)/SlidingWindow;
    }
    return correctRatio;
}

int TimeLapsedEventSender(int laseryes,int SOneLaserSwitchOrnot, int BNCNum, float Duration, int IsWaterPer,int tempResult,int TransWaterDura) {

    int IsBNCOn=0;
    int TransResult = tempResult;
    timerCounterI = 0;
    int reward=0;
    int TempTime;
    switch (IsWaterPer) {
        case 1:
            while(timerCounterI < Duration * 1000) {
                if (CQ_lickOrAll&& reward==0 ) {
                    localSendOnce(SpLick, 1);
                    TempTime=timerCounterI;
                    while (timerCounterI<=TempTime+TransWaterDura){CQ_Valve_ON(water_sweet, period * 2 * 1, 6);reward=1;localSendOnce(SpLick, 1);}
                    localSendOnce(SpWater_sweet, 1);
                    CQ_Valve_OFF(water_sweet, 6);
                    TransResult = 1; //Hit
                }
                if (laseryes == 1 && SOneLaserSwitchOrnot == 1&&timerCounterI <=1) {
                    IsBNCOn=CQ_Digt_on(BNCNum, 1);
                }
                CQ_LickSignalSender(CQ_lickOrAll, 20);
            }
            break;
        case 2:
            while (timerCounterI < Duration * 1000) {
                if (CQ_lickOrAll){
                    TransResult = 3; //FC
                }
                if (laseryes == 1 && SOneLaserSwitchOrnot == 1&&timerCounterI <=1) {
                    IsBNCOn=CQ_Digt_on(BNCNum, 1);
                }
                CQ_LickSignalSender(CQ_lickOrAll, 20);
            }
            break;
        case 0:
            while (timerCounterI < Duration * 1000) {
                if (laseryes == 1 && SOneLaserSwitchOrnot == 1&&timerCounterI <=1) {
                    IsBNCOn=CQ_Digt_on(BNCNum, 1);
                }
                CQ_LickSignalSender(CQ_lickOrAll, 20);
            }
            break;
    }
    if (IsBNCOn){
    CQ_Digt_off(BNCNum);IsBNCOn=0;
    }
    return TransResult;
}

void TransResultsSum(int tempResult,int trialnum){

   int CurrentTrial = 0;
   switch (tempResult) {
        case 1:
            H[trialnum - 1] = 1;
            CurrentTrial = 1;
            localSendOnce(SpHit, 1);
            break;
        case 2:
            M[trialnum - 1] = 1;
            MissNum++;
            localSendOnce(SpMiss, 1);
            break;
        case 3:
            F[trialnum - 1] = 1;
            CurrentTrial = 1;
            localSendOnce(SpFalseAlarm, 1);
            break;
        case 4:
            C[trialnum - 1] = 1;
            MissNum++;
            localSendOnce(SpCorrectRejection, 1);
            break;
    }
    if (CurrentTrial == 1) {
        MissNum = 0;
    }

    int TransH, TransM, TransF, TransC;
    TransH = SlidingWindowSumation(H, trialnum, SlidingWindow);
    counts_dispose(TransH);
    LCD_set_xy(6, 1);
    lcd_data(hunds);
    lcd_data(tens);
    lcd_data(ones);

    TransM = SlidingWindowSumation(M, trialnum, SlidingWindow);
    counts_dispose(TransM);
    LCD_set_xy(10, 1);
    lcd_data(hunds);
    lcd_data(tens);
    lcd_data(ones);

    TransF = SlidingWindowSumation(F, trialnum, SlidingWindow);
    counts_dispose(TransF);
    LCD_set_xy(6, 2);
    lcd_data(hunds);
    lcd_data(tens);
    lcd_data(ones);

    TransC = SlidingWindowSumation(C, trialnum, SlidingWindow);
    counts_dispose(TransC);
    LCD_set_xy(10, 2);
    lcd_data(hunds);
    lcd_data(tens);
    lcd_data(ones);

    // Discrimination rate
    correctRatio = CalculateSlidingPer(H, C, trialnum, SlidingWindow);
    counts_dispose(correctRatio);
    LCD_set_xy(14, 1);
    lcd_data(tens);
    lcd_data(ones);

}

void CQ_TwoOdorResponseWithTwoTrigger(int Odor1,int Odor1BNC,int Odor2,int Odor2BNC, float odorLen, float delay1, float delay2, float ResponseCue,float waterlen, float ITI,int rule, int SOneLaserSwitchOrnot, int RuleSwitch, int trialnum,int odor1laseryes, int odor2laseryes, int delay1laseryes, int delay2laseryes,int ResponseCueyes, int waterlaseryes, int ITIlaseryes, float DelaylaserStart,float DelayLaserEnd,int TransWaterDura) {

    localSendOnce(59, 0);

	 if(rule==1)
             {localSendOnce(SpStepN, 1);}
          else if(rule==2)
             {localSendOnce(SpStepN, 2);}//send rule signals to LABVIEW, 1Nonmatch, 2Match

          int tempResult;
	  if (TwoOdorJudger(Odor1,Odor2,rule)){tempResult=2;}//Miss
    else {
        tempResult = 4;
    }//Correct rejection

    //---odor1--//
    CQ_Valve_ON(Odor1, period * 2 * 1.0, Odor1BNC);
    CQ_OdorSPSignalSender(Odor1, 1);
    tempResult=TimeLapsedEventSender(odor1laseryes,SOneLaserSwitchOrnot, 4,odorLen,0,tempResult,TransWaterDura);
    CQ_Valve_OFF(Odor1, Odor1BNC);
    CQ_OdorSPSignalSender(Odor1, 1);

    //---delay1--//
     timerCounterI = 0;
     tempResult=TimeLapsedEventSender(0,SOneLaserSwitchOrnot, 4,DelaylaserStart,0,tempResult,TransWaterDura);
     timerCounterI = 0;
     tempResult=TimeLapsedEventSender(delay1laseryes,SOneLaserSwitchOrnot, 4,DelayLaserEnd-DelaylaserStart,0,tempResult,TransWaterDura);
     timerCounterI = 0;
     tempResult=TimeLapsedEventSender(0,SOneLaserSwitchOrnot, 4,delay1-DelayLaserEnd,0,tempResult,TransWaterDura);

    //---odor2--//
    CQ_Valve_ON(Odor2, period * 2 * 1.0, Odor2BNC);
    CQ_OdorSPSignalSender(Odor2, 1);
    tempResult=TimeLapsedEventSender(odor2laseryes,SOneLaserSwitchOrnot, 4,odorLen,0,tempResult,TransWaterDura);
    CQ_Valve_OFF(Odor2, Odor2BNC);
    CQ_OdorSPSignalSender(Odor2, 1);

    //---delay2--//
    tempResult=TimeLapsedEventSender(delay2laseryes, SOneLaserSwitchOrnot, 4,delay2 + ResponseCue,0,tempResult,TransWaterDura);

    //---water--//
    timerCounterI = 0;
    localSendOnce(SpWater_sweet, 1);
    if (TwoOdorJudger(Odor1, Odor2, rule)) {
        tempResult=TimeLapsedEventSender(waterlaseryes, SOneLaserSwitchOrnot, 4,waterlen,1,tempResult,TransWaterDura);
    } else {
        tempResult=TimeLapsedEventSender(waterlaseryes, SOneLaserSwitchOrnot, 4,waterlen,2,tempResult,TransWaterDura);
    }
    CQ_Valve_OFF(water_sweet, 6);
    localSendOnce(SpWater_sweet, 1);

    // summarize the results //
    TransResultsSum(tempResult, trialnum);

    //--iti--//
    tempResult=TimeLapsedEventSender(ITIlaseryes,SOneLaserSwitchOrnot, 4,4.5,0,tempResult,TransWaterDura);

    timerCounterI = 0;
    while (timerCounterI < (ITI -4.5) * 1000) {
        CQ_LickSignalSender(CQ_lickOrAll, 20);
    }
}

void CQ_Match_Nonmatch_Shaping(int rule, int OdorPairSelection, int StopOrNot, int Odor1, int Odor1BNC, int Odor2, int Odor2BNC, float odorLen, float delay1, float delay2, float ResponseCue, float waterlen, float ITI, int TotalTrialNum, int Odor3, int Odor4,int TransWaterDura) {

    home_clr();
    puts_lcd((unsigned char*) &text7[0], sizeof (text7));
    line_2();
    puts_lcd((unsigned char*) &text8[0], sizeof (text8));

    LCD_set_xy(1, 1);
    if (rule == 1) {
        lcdWriteString("NS");
    } else if (rule == 2) {
        lcdWriteString("MS");
    }

    DelayNmSecNew(2 * 1000);

    int MissNum=0;
    int trialnum;
    int RuleSwitch = 0;
    int *FirstOdor;
    int *SecondOdor;
    int *FirstOdorBNC;
    int *SecondOdorBNC;

    int OdorPair1[4] = {Odor1, Odor2, Odor1, Odor2};
    int OdorPair2[4] = {Odor2, Odor1, Odor2, Odor1};
    int BNCPair1[4] = {Odor1BNC, Odor2BNC, Odor1BNC, Odor2BNC};
    int BNCPair2[4] = {Odor2BNC, Odor1BNC, Odor2BNC, Odor1BNC};
    int OdorPair3[4] = {Odor3, Odor4, Odor3, Odor4};
    int OdorPair4[4] = {Odor4, Odor3, Odor4, Odor3};

    int OdorCom = 2 * (OdorPairSelection + 1) / rule;
    switch (OdorCom) {
        case 4:
            FirstOdor = OdorPair1;
            SecondOdor = OdorPair2;
            FirstOdorBNC = BNCPair1;
            SecondOdorBNC = BNCPair2;
            break;
        case 2:
            FirstOdor = OdorPair1;
            SecondOdor = OdorPair1;
            FirstOdorBNC = BNCPair1;
            SecondOdorBNC = BNCPair1;
            break;
        case 6:
            FirstOdor = OdorPair3;
            SecondOdor = OdorPair4;
            FirstOdorBNC = BNCPair1;
            SecondOdorBNC = BNCPair2;
            break;
        case 3:
            FirstOdor = OdorPair3;
            SecondOdor = OdorPair3;
            FirstOdorBNC = BNCPair1;
            SecondOdorBNC = BNCPair1;
            break;
    }
    if (MissNum <StopCriterion ) {
        int sequence = 4;
        int ind;
        int tempInt;
        for (trialnum = 1; trialnum <= TotalTrialNum; trialnum++) {

            counts_dispose(trialnum);
            LCD_set_xy(1, 2);
            lcd_data(hunds);
            lcd_data(tens);
            lcd_data(ones);

            if (sequence == 0) {
                sequence = 4;
            }
            ind = rand() % sequence;
            CQ_TwoOdorResponseWithTwoTrigger(FirstOdor[ind], FirstOdorBNC[ind], SecondOdor[ind], SecondOdorBNC[ind], odorLen, delay1, delay2, ResponseCue, waterlen, ITI, rule, 2, RuleSwitch, trialnum, 0, 0, 0, 0, 0, 0, 0,0,delay1,TransWaterDura);
            tempInt = FirstOdor[sequence - 1];
            FirstOdor[sequence - 1] = FirstOdor[ind];
            FirstOdor[ind] = tempInt;
            tempInt = SecondOdor[sequence - 1];
            SecondOdor[sequence - 1] = SecondOdor[ind];
            SecondOdor[ind] = tempInt;
            tempInt = FirstOdorBNC[sequence - 1];
            FirstOdorBNC[sequence - 1] = FirstOdorBNC[ind];
            FirstOdorBNC[ind] = tempInt;
            tempInt = SecondOdorBNC[sequence - 1];
            SecondOdorBNC[sequence - 1] = SecondOdorBNC[ind];
            SecondOdorBNC[ind] = tempInt;
            sequence--;
            if (trialnum % SlidingWindow == 0) {
                localSendOnce(SpSess, 0);
                localSendOnce(59, 0);
            }
            if ((StopOrNot==1&&MissNum >= StopCriterion) || trialnum >= TotalTrialNum) {
                localSendOnce(SpTrain, 0); // send the end signal
                break;
            }
        }
    } else {
        localSendOnce(SpTrain, 0);
    }
}

void CQ_Match_Nonmatch_autoSwitch(int rule, int LaserPeriod, int SOneLaserSwitchOrnot, int OdorPairSelection, int ShapeAfterSwitch, int Odor1, int Odor1BNC, int Odor2, int Odor2BNC, float odorLen, float delay1, float delay2, float ResponseCue, float waterlen, float ITI, int TrialNuminSession, int Odor3, int Odor4,float DelaylaserStart,float DelayLaserEnd,int TransWaterDura,int SessionNum) {

    home_clr();
    puts_lcd((unsigned char*) &text7[0], sizeof (text7));
    line_2();
    puts_lcd((unsigned char*) &text8[0], sizeof (text8));

    DelayNmSecNew(2 * 1000);

    int odor1laseryes = 0;
    int odor2laseryes = 0;
    int delay1laseryes = 0;
    int delay2laseryes = 0;
    int waterlaseryes = 0;
    int ITIlaseryes = 0;
    int ResponseCueyes = 0;
    switch (LaserPeriod) {
        case 1:
            break;
        case 2:
            odor1laseryes = 1;
            odor2laseryes = 1;
            break;
        case 3:
            delay1laseryes = 1;
            break;
        case 4:
            odor2laseryes = 1;
            delay2laseryes = 1;
            waterlaseryes = 1;
            break;
        case 5:
            waterlaseryes = 1;
            ITIlaseryes = 1;
            break;
    }
    int sessionnum = 0;
    int trialnum;
    int RuleSwitch = 0;
    int *FirstOdor;
    int *SecondOdor;

    int OdorPair1[4] = {Odor1, Odor2, Odor1, Odor2};
    int OdorPair2[4] = {Odor2, Odor1, Odor1, Odor2};
    int FirstOdorBNC[4] = {Odor1BNC, Odor2BNC, Odor1BNC, Odor2BNC};
    int SecondOdorBNC[4] = {Odor2BNC, Odor1BNC, Odor2BNC, Odor1BNC};
    int OdorPair3[4] = {Odor3, Odor4, Odor3, Odor4};
    int OdorPair4[4] = {Odor4, Odor3, Odor4, Odor3};
    switch (OdorPairSelection) {
        case 1:
            FirstOdor = OdorPair1;
            SecondOdor = OdorPair2;
            break;
        case 2:
            FirstOdor = OdorPair3;
            SecondOdor = OdorPair4;
            break;
    }

    int sequence = 4;
    int ind;
    int tempInt;
    while (sessionnum<=SessionNum) {
        hit = 0;
        miss = 0;
        falseAlarm = 0;
        correctRejection = 0;
        home_clr();
        puts_lcd((unsigned char*) &text7[0], sizeof (text7));
        line_2();
        puts_lcd((unsigned char*) &text8[0], sizeof (text8));

        LCD_set_xy(1, 1);
        if (rule == 1 ) {
            lcdWriteString("N");
        }  else if (rule == 2 ) {
            lcdWriteString("M");
        }

        counts_dispose(RuleSwitch);
        LCD_set_xy(3, 1);
        lcd_data(tens);
        lcd_data(ones);

        sessionnum++;
        counts_dispose(sessionnum);
        LCD_set_xy(1, 2);
        lcd_data(tens);
        lcd_data(ones); //Show session number in LCD

        if (sequence != 4) {
            sequence = 4;
        }
        int StartTrialNum;
        for (trialnum = 1; trialnum <= TrialNuminSession; trialnum++) {
            if (sequence == 0) {
                sequence = 4;
            }
            ind = rand() % sequence;
            StartTrialNum=TrialNuminSession*(sessionnum-1)+trialnum;
            CQ_TwoOdorResponseWithTwoTrigger(FirstOdor[ind], FirstOdorBNC[ind], SecondOdor[ind], SecondOdorBNC[ind], odorLen, delay1, delay2, ResponseCue, waterlen, ITI, rule, SOneLaserSwitchOrnot, RuleSwitch, StartTrialNum, odor1laseryes, odor2laseryes, delay1laseryes, delay2laseryes, ResponseCueyes, waterlaseryes, ITIlaseryes, DelaylaserStart, DelayLaserEnd,TransWaterDura);
            tempInt = FirstOdor[sequence - 1];
            FirstOdor[sequence - 1] = FirstOdor[ind];
            FirstOdor[ind] = tempInt;
            tempInt = SecondOdor[sequence - 1];
            SecondOdor[sequence - 1] = SecondOdor[ind];
            SecondOdor[ind] = tempInt;
            tempInt = FirstOdorBNC[sequence - 1];
            FirstOdorBNC[sequence - 1] = FirstOdorBNC[ind];
            FirstOdorBNC[ind] = tempInt;
            tempInt = SecondOdorBNC[sequence - 1];
            SecondOdorBNC[sequence - 1] = SecondOdorBNC[ind];
            SecondOdorBNC[ind] = tempInt;
            sequence--;
        }
        localSendOnce(SpSess, 0);

//        if (sessionnum==SessionNum)//||MissNum >= StopCriterion
//        {break;}

//        if (SOneLaserSwitchOrnot != 2&&SOneLaserSwitchOrnot != 1) {
//            if (correctRatio >= 80) {
//                int RandTrial = rand() % 15 + 15; //
//                if (RandTrial == 20 || RandTrial == 0) {
//                    RandTrial = 25;
//                }
//                hit = 0;
//                miss = 0;
//                falseAlarm = 0;
//                correctRejection = 0;
//
//                home_clr();
//                puts_lcd((unsigned char*) &text7[0], sizeof (text7));
//                line_2();
//                puts_lcd((unsigned char*) &text8[0], sizeof (text8));
//
//                LCD_set_xy(1, 1);
//                if (rule == 1 ) {
//                    lcdWriteString("NR");
//                }  else if (rule == 2 ) {
//                    lcdWriteString("MR");
//                }
//
//                counts_dispose(RuleSwitch);
//                LCD_set_xy(3, 1);
//                lcd_data(tens);
//                lcd_data(ones);
//
//                counts_dispose(RandTrial);
//                LCD_set_xy(3, 2);
//                lcd_data(tens);
//                lcd_data(ones);
//
//                sessionnum++;
//                counts_dispose(sessionnum);
//                LCD_set_xy(1, 2);
//                lcd_data(tens);
//                lcd_data(ones); //Show session number in LCD
//
//                if (sessionnum==SessionNum)//||MissNum >= StopCriterion
//                {break;}
//
//                for (trialnum = 1; trialnum < RandTrial; trialnum++) {
//                    if (sequence == 0) {
//                        sequence = 4;
//                    }
//                    ind = rand() % sequence;
//                    StartTrialNum=TrialNuminSession*(sessionnum-1)+trialnum;
//                    CQ_TwoOdorResponseWithTwoTrigger(FirstOdor[ind], FirstOdorBNC[ind], SecondOdor[ind], SecondOdorBNC[ind], odorLen, delay1, delay2, ResponseCue, waterlen, ITI, rule, SOneLaserSwitchOrnot, RuleSwitch,StartTrialNum, odor1laseryes, odor2laseryes, delay1laseryes, delay2laseryes, ResponseCueyes, waterlaseryes, ITIlaseryes,DelaylaserStart,DelayLaserEnd, TransWaterDura);
//                    tempInt = FirstOdor[sequence - 1];
//                    FirstOdor[sequence - 1] = FirstOdor[ind];
//                    FirstOdor[ind] = tempInt;
//                    tempInt = SecondOdor[sequence - 1];
//                    SecondOdor[sequence - 1] = SecondOdor[ind];
//                    SecondOdor[ind] = tempInt;
//                    tempInt = FirstOdorBNC[sequence - 1];
//                    FirstOdorBNC[sequence - 1] = FirstOdorBNC[ind];
//                    FirstOdorBNC[ind] = tempInt;
//                    tempInt = SecondOdorBNC[sequence - 1];
//                    SecondOdorBNC[sequence - 1] = SecondOdorBNC[ind];
//                    SecondOdorBNC[ind] = tempInt;
//                    sequence--;
//                }
//                localSendOnce(SpSess, 0);
//
//                if (correctRatio >= 80) {
//                    RuleSwitch++;
//                    if (rule == 1) {
//                        rule = 2;
//                    } else if (rule == 2) {
//                        rule = 1;
//                    }//1Nonmatch, 2Match
//                    if (ShapeAfterSwitch == 1) {
//                        CQ_Match_Nonmatch_Shaping(rule, OdorPairSelection,1, Odor1, Odor1BNC, Odor2, Odor2BNC, odorLen, delay1, delay2, ResponseCue, waterlen, ITI, 40, Odor3, Odor4, TransWaterDura);
//                        sessionnum = sessionnum + 2;
//                    }
//                    localSendOnce(58, 1);//send out the rule switch time point
//                } //if the correct rate of two successive sessions reach 80%, switch the rule
//            }
//        }
    }
}

int CQ_AutoGoNogoTask_ReversalDefined(int rule, int LaserPeriod, int SOneLaserReverseOrnot, int Odor1, int Odor1BNC, int Odor2, int Odor2BNC, float odorLen, float delay, float ResponseCue, float waterlen, float ITI, int RuleSwitch, int StartTrialNum,int TransWaterDura) {

    home_clr();
    puts_lcd((unsigned char*) &text7[0], sizeof (text7));
    line_2();
    puts_lcd((unsigned char*) &text8[0], sizeof (text8));

    int odorlaseryes = 0;
    int delaylaseryes = 0;
    int waterlaseryes = 0;
    int ITIlaseryes = 0;
    int ResponseCueyes = 0;
    switch (LaserPeriod) {
        case 1:
            break;
        case 2:
            odorlaseryes = 1;
            break;
        case 3:
            delaylaseryes = 1;
            break;
        case 4:
            odorlaseryes = 1;
            delaylaseryes = 1;
            waterlaseryes = 1;
            ResponseCueyes = 1;
            break;
        case 5:
            ITIlaseryes = 1;
            waterlaseryes = 1;
            break;
    }

    int currentTrial = 0;
    //int RuleSwitch = 0;
    int trialnum=StartTrialNum;
    int sequence = 0;
    int ind;
    int tempInt;

    int Odor[4] = {Odor1,Odor2,Odor1,Odor2};
    int OdorBNC[4] = {Odor1BNC,Odor2BNC,Odor1BNC,Odor2BNC};

    int TargetReversal = RuleSwitch;

    while (RuleSwitch == TargetReversal) {

        home_clr();
        puts_lcd((unsigned char*) &text7[0], sizeof (text7));
        line_2();
        puts_lcd((unsigned char*) &text8[0], sizeof (text8));

        counts_dispose(RuleSwitch);
        LCD_set_xy(3, 1);
        lcd_data(tens);
        lcd_data(ones);

        if (MissNum >= StopCriterion||trialnum >= StopTrialNum) {
            break;
        }

        LCD_set_xy(1, 1);
        if (rule == 3) {
            lcdWriteString("BG");
        } else {
            lcdWriteString("JG");
        }
        currentTrial = 0;
        hit = 0;
        miss = 0;
        falseAlarm = 0;
        correctRejection = 0;
        if (sequence != 4) {
            sequence = 4;
        }
        for (trialnum = StartTrialNum+1; trialnum <= StopTrialNum; trialnum++) {

            counts_dispose(trialnum);
            LCD_set_xy(1, 2);
            lcd_data(hunds);
            lcd_data(tens);
            lcd_data(ones);

            counts_dispose(LastSwitchTrialNum);
            LCD_set_xy(14, 2);
            lcd_data(hunds);
            lcd_data(tens);
            lcd_data(ones);

            if (sequence == 0) {
                sequence = 4;
            }
            ind = rand() % sequence;
            //Recording_OneOdorResponseWithTwoTrigger(Odor[ind],OdorBNC[ind],0.0, 1.0,5,2.0,2.5, 7.5,5,1.0,2.0,6,2.5,7,rule);
            CQ_OneOdorResponseWithTwoTrigger(RuleSwitch, trialnum, SOneLaserReverseOrnot, Odor[ind], OdorBNC[ind], odorLen, delay, ResponseCue, waterlen, ITI, odorlaseryes, delaylaseryes, ResponseCueyes, waterlaseryes, ITIlaseryes, rule,TransWaterDura);
            tempInt = Odor[sequence - 1];
            Odor[sequence - 1] = Odor[ind];
            Odor[ind] = tempInt;
            tempInt = OdorBNC[sequence - 1];
            OdorBNC[sequence - 1] = OdorBNC[ind];
            OdorBNC[ind] = tempInt;
            sequence--;

            if (trialnum % SlidingWindow == 0) {
                localSendOnce(SpSess, 0);
                localSendOnce(59, 0);
            }

            if (MissNum >= StopCriterion||trialnum>=StopTrialNum) {
                localSendOnce(SpSess, 0);
                break;
            }
            if (correctRatio >= 85 && SOneLaserReverseOrnot != 2 && (trialnum - LastSwitchTrialNum) >= SlidingWindow) {
                RuleSwitch++;
                LastSwitchTrialNum = trialnum;
                localSendOnce(59, 0);
                localSendOnce(58, 1);//send out the rule switch time point
                return trialnum;
            }
        }
    }
    return trialnum;
}

int CQ_Match_Nonmatch_autoSwitch_ReversalDefined(int rule, int LaserPeriod, int SOneLaserReverseOrnot, int OdorPairSelection,int Odor1, int Odor1BNC, int Odor2, int Odor2BNC, float odorLen, float delay1, float delay2, float ResponseCue, float waterlen, float ITI, int Odor3, int Odor4, int RuleSwitch, int StartTrialNum,float DelaylaserStart,float DelayLaserEnd,int TransWaterDura) {
    home_clr();
    puts_lcd((unsigned char*) &text7[0], sizeof (text7));
    line_2();
    puts_lcd((unsigned char*) &text8[0], sizeof (text8));

    //DelayNmSecNew(2 * 1000);

    int odor1laseryes = 0;
    int odor2laseryes = 0;
    int delay1laseryes = 0;
    int delay2laseryes = 0;
    int waterlaseryes = 0;
    int ITIlaseryes = 0;
    int ResponseCueyes = 0;
    switch (LaserPeriod) {
        case 1:
            break;
        case 2:
            odor1laseryes = 1;
            odor2laseryes = 1;
            break;
        case 3:
            delay1laseryes = 1;
            break;
        case 4:
            odor2laseryes = 1;
            delay2laseryes = 1;
            waterlaseryes = 1;
            break;
        case 5:
            ITIlaseryes = 1;
            waterlaseryes = 1;
            break;
    }

    int trialnum=StartTrialNum;
    int *FirstOdor;
    int *SecondOdor;
    int OdorPair1[4]={Odor1,Odor2,Odor1,Odor2};
    int OdorPair2[4]={Odor2,Odor1,Odor1,Odor2};
    int FirstOdorBNC[4]={Odor1BNC,Odor2BNC,Odor1BNC,Odor2BNC};
    int SecondOdorBNC[4]={Odor2BNC,Odor1BNC,Odor1BNC,Odor2BNC};
    int OdorPair3[4]={Odor3,Odor4,Odor3,Odor4};
    int OdorPair4[4]={Odor4,Odor3,Odor3,Odor4};

    if (OdorPairSelection == 1) {
        FirstOdor=OdorPair1;
        SecondOdor=OdorPair2;
    } else if (OdorPairSelection == 2) {
        FirstOdor=OdorPair3;
        SecondOdor=OdorPair4;
    }

    int sequence = 4;
    int ind;
    int tempInt;
    int TargetReversal = RuleSwitch;

    while (RuleSwitch == TargetReversal) {

        LCD_set_xy(1, 1);
        if (rule == 1 ) {
            lcdWriteString("N");
        }  else if (rule == 2 ) {
            lcdWriteString("M");
        }

        counts_dispose(RuleSwitch);
        LCD_set_xy(3, 1);
        lcd_data(tens);
        lcd_data(ones);

         if (MissNum >= StopCriterion||trialnum >= StopTrialNum) {
                break;
         }

        if (sequence != 4) {
            sequence = 4;
        }
        for (trialnum = StartTrialNum+1; trialnum <= StopTrialNum; trialnum++) {
            
            counts_dispose(trialnum);
            LCD_set_xy(1, 2);
            lcd_data(hunds);
            lcd_data(tens);
            lcd_data(ones); //Show session number in LCD

            counts_dispose(LastSwitchTrialNum);
            LCD_set_xy(14, 2);
            lcd_data(hunds);
            lcd_data(tens);
            lcd_data(ones);

            if (sequence == 0) {
                sequence = 4;
            }
            ind = rand() % sequence;
            //Recording_TwoOdorResponseWithTwoTrigger(FirstOdor[ind],FirstOdorBNC[ind],0.0, 1.0,SecondOdor[ind],SecondOdorBNC[ind],4.0, 5.0,6,6.0,6.5, 11.5,5,1.0,4.0,6,0.0,0.0,rule);
            CQ_TwoOdorResponseWithTwoTrigger(FirstOdor[ind], FirstOdorBNC[ind], SecondOdor[ind], SecondOdorBNC[ind], odorLen, delay1, delay2, ResponseCue, waterlen, ITI, rule, SOneLaserReverseOrnot, RuleSwitch, trialnum, odor1laseryes, odor2laseryes, delay1laseryes, delay2laseryes, ResponseCueyes, waterlaseryes, ITIlaseryes,DelaylaserStart, DelayLaserEnd,TransWaterDura);
            tempInt = FirstOdor[sequence - 1];
            FirstOdor[sequence - 1] = FirstOdor[ind];
            FirstOdor[ind] = tempInt;
            tempInt = SecondOdor[sequence - 1];
            SecondOdor[sequence - 1] = SecondOdor[ind];
            SecondOdor[ind] = tempInt;
            tempInt = FirstOdorBNC[sequence - 1];
            FirstOdorBNC[sequence - 1] = FirstOdorBNC[ind];
            FirstOdorBNC[ind] = tempInt;
            tempInt = SecondOdorBNC[sequence - 1];
            SecondOdorBNC[sequence - 1] = SecondOdorBNC[ind];
            SecondOdorBNC[ind] = tempInt;
            sequence--;

            if (trialnum%SlidingWindow==0){
            localSendOnce(SpSess, 0);
            localSendOnce(59, 0);
            }

            if (MissNum >= StopCriterion||trialnum>=StopTrialNum) {
                localSendOnce(SpSess, 0);
                break;
            }

            if (correctRatio >= 85&&(trialnum-LastSwitchTrialNum)>=SlidingWindow&&SOneLaserReverseOrnot!=2&&SOneLaserReverseOrnot!=1) {
                RuleSwitch++;
                LastSwitchTrialNum=trialnum;
                localSendOnce(59, 0);
                localSendOnce(58, 1);//send out the rule switch time point
                return trialnum;
            }
        }
    }
    return trialnum;
}

void CQ_DNMS_GONOGO_Switch(int FirstRule, int SecondRule, int GoNoGoLaserPer, int SOneLaserReverseOrnot, int DNMSLaserPer) {

    int Rule1[9]={1,3,4,1,4,3,2,1,3};//RuleCom =28  (1 for Nonmatch, 2 for Match, 3 for A-Go, 4 for B-Go)
    int Rule2[9]={1,4,3,1,3,4,2,1,4};//21
    int Rule3[9]={1,2,3,4,1,4,3,2,1};//42
    int Rule4[9]={2,3,4,2,4,3,1,2,3};//32
    int Rule5[9]={2,4,3,2,3,4,1,2,4};//24
    int Rule6[9]={2,1,4,3,2,3,4,1,2};//96
    int Rule7[9]={3,2,3,4,2,4,3,2,1};//54
    int Rule8[9]={3,1,3,4,1,4,3,1,2};//108
    int Rule9[9]={3,4,2,4,3,1,3,4,1};//27
    int Rule10[9]={4,2,4,3,2,3,4,2,1};//60
    int Rule11[9]={4,1,4,3,1,3,4,1,2};//120
    int Rule12[9]={4,3,2,3,4,2,4,3,1};//40

    int *RuleSequence;

    int RuleCom = 12 * (FirstRule + 6) / SecondRule;
    switch (RuleCom) {
        case 28:
            RuleSequence = Rule1;
            break;
        case 21:
            RuleSequence = Rule2;
            break;
        case 42:
            RuleSequence = Rule3;
            break;
        case 32:
            RuleSequence = Rule4;
            break;
        case 24:
            RuleSequence = Rule5;
            break;
        case 96:
            RuleSequence = Rule6;
            break;
        case 54:
            RuleSequence = Rule7;
            break;
        case 108:
            RuleSequence = Rule8;
            break;
        case 27:
            RuleSequence = Rule9;
            break;
        case 60:
            RuleSequence = Rule10;
            break;
        case 120:
            RuleSequence = Rule11;
            break;
        case 40:
            RuleSequence = Rule12;
            break;
    }

    float correctRatio;
    int SwitchTimes;
    int StartTrialNum = 0;
    for (SwitchTimes = 1; SwitchTimes <= 9; SwitchTimes++) {
        if (RuleSequence[SwitchTimes - 1] == 1 || RuleSequence[SwitchTimes - 1] == 2) {
            StartTrialNum =CQ_Match_Nonmatch_autoSwitch_ReversalDefined(RuleSequence[SwitchTimes - 1], DNMSLaserPer, SOneLaserReverseOrnot, 1, odor_A, 2, odor_B, 3, 1.0, 3.0, 0.3, 0.2, 0.5, 10.0, odor_C, odor_D, SwitchTimes - 1,StartTrialNum,0,3.0,100);
            correctRatio = 0.0;
        } else if (RuleSequence[SwitchTimes - 1] == 3 || RuleSequence[SwitchTimes - 1] == 4) {
            StartTrialNum =CQ_AutoGoNogoTask_ReversalDefined(RuleSequence[SwitchTimes - 1], GoNoGoLaserPer, 1, odor_A, 2, odor_B, 3, 1.0, 0.3, 0.2, 0.5, 10.0,  SwitchTimes - 1,StartTrialNum,100);
            correctRatio = 0.0;
        }
        if (MissNum >= StopCriterion) {
            localSendOnce(SpTrain, 0); // send the end signal
            break;
        }
    }
}

void CQ_NM_GNG_NM_Switch(int AgoOrBgo, int GoNoGoLaserPer, int SOneLaserReverseOrnot, int DNMSLaserPer, int MOrN)  {

    float correctRatio;
    int Reversal;
    int StartTrialNum = 0;

    StartTrialNum = CQ_Match_Nonmatch_autoSwitch_ReversalDefined(MOrN, DNMSLaserPer, SOneLaserReverseOrnot,1, odor_A, 2, odor_B, 3, 1.0, 5.0, 0.8, 0.2, 0.5, 10.0, odor_C, odor_D, 0,StartTrialNum,0.0,4.8,100);
    correctRatio = 0.0;

    if (MissNum < StopCriterion&&StartTrialNum<StopTrialNum ) {
        for (Reversal = 1; Reversal < 15; Reversal++) {
            if (Reversal % 2 == 1) {
                StartTrialNum = CQ_AutoGoNogoTask_ReversalDefined(AgoOrBgo, GoNoGoLaserPer, 1, odor_A, 2, odor_B, 3, 1.0, 0.3, 0.2, 0.5, 5.0, Reversal,StartTrialNum,100);
                correctRatio = 0.0;
            } else if (Reversal % 2 == 0) {
                StartTrialNum = CQ_Match_Nonmatch_autoSwitch_ReversalDefined(MOrN, DNMSLaserPer,1,1 ,odor_A, 2, odor_B, 3, 1.0, 5.0, 0.3, 0.2, 0.5, 10.0,odor_C, odor_D, Reversal,StartTrialNum,1.0,5.0,100);
                correctRatio = 0.0;
            }
            if (MissNum >= StopCriterion||StartTrialNum>=StopTrialNum) {
                localSendOnce(SpTrain, 0); // send the end signal
                break;
            }
        }
    }else{
    localSendOnce(SpTrain, 0); // send the end signal
    }
}

void CQ_GNG_NM_GNG_Switch(int AgoOrBgo, int GoNoGoLaserPer, int SOneLaserReverseOrnot, int DNMSLaserPer, int MOrN) {

    float correctRatio;
    int Reversal;
    int StartTrialNum = 0;

    StartTrialNum = CQ_AutoGoNogoTask_ReversalDefined(AgoOrBgo, GoNoGoLaserPer, SOneLaserReverseOrnot, odor_A, 2, odor_B, 3, 1.0, 0.3, 0.2, 0.5, 5.0, 0,StartTrialNum,100);
    correctRatio = 0.0;
    if (MissNum < StopCriterion&&StartTrialNum<StopTrialNum) {
        for (Reversal = 1; Reversal < 15; Reversal++) {
            if (Reversal % 2 == 1) {
                StartTrialNum = CQ_Match_Nonmatch_autoSwitch_ReversalDefined(MOrN, DNMSLaserPer, 1, 1,odor_A, 2, odor_B, 3, 1.0, 3.0, 0.3, 0.2, 0.5, 5.0, odor_C, odor_D, Reversal,StartTrialNum,1.0,3.0,100);
                correctRatio = 0.0;
            } else if (Reversal % 2 == 0) {
                StartTrialNum = CQ_AutoGoNogoTask_ReversalDefined(AgoOrBgo, GoNoGoLaserPer, 1, odor_A, 2, odor_B, 3, 1.0, 0.3, 0.2, 0.5, 5.0, Reversal,StartTrialNum,100);
                correctRatio = 0.0;
            }
            if (MissNum >= StopCriterion||StartTrialNum>=StopTrialNum) {
                localSendOnce(SpTrain, 0); // send the end signal
                break;
            }
        }
    }else{
    localSendOnce(SpTrain, 0); // send the end signal
    }
}

void CQ_OneOdorResponseWithTwoTrigger(int RuleSwitch, int trialnum, int SOneLaserReverseOrnot, int Odor, int OdorBNC, float odorLen, float delay, float ResponseCue, float waterlen, float ITI, int odorlaseryes, int delaylaseryes, int ResponseCueyes, int waterlaseryes, int ITIlaseryes, int rule, int TransWaterDura) {

    localSendOnce(59, 0);

    if (rule == 3) {
        localSendOnce(SpStepN, 3);
        localSendOnce(SpStepN, 3);
    } else if (rule == 4) {
        localSendOnce(SpStepN, 4);
        localSendOnce(SpStepN, 4);
    }
    int tempResult;
    if (CQ_OneOdorJudger(Odor, rule)) {
        tempResult = 2;
    } else {
        tempResult = 4;
    }

    //---odor--//
    CQ_Valve_ON(Odor, period * 2 * 1.0, OdorBNC);
    CQ_OdorSPSignalSender(Odor, 1);
    tempResult=TimeLapsedEventSender(odorlaseryes, SOneLaserReverseOrnot, 4,odorLen,0,tempResult,TransWaterDura);
    CQ_Valve_OFF(Odor, OdorBNC);
    CQ_OdorSPSignalSender(Odor, 1);

    //---delay--//
    tempResult=TimeLapsedEventSender(delaylaseryes, SOneLaserReverseOrnot, 4,delay + ResponseCue,0,tempResult,TransWaterDura);

    // water //
    timerCounterI = 0;
    localSendOnce(SpWater_sweet, 1);
    if (CQ_OneOdorJudger(Odor, rule)) {
        tempResult=TimeLapsedEventSender(waterlaseryes, SOneLaserReverseOrnot, 4,waterlen,1,tempResult,TransWaterDura);
    } else {
        tempResult=TimeLapsedEventSender(waterlaseryes, SOneLaserReverseOrnot, 4,waterlen,2,tempResult,TransWaterDura);
    }
    CQ_Valve_OFF(water_sweet, 6);
    localSendOnce(SpWater_sweet, 1);

    // summarize the results //
    TransResultsSum(tempResult, trialnum);

    //--iti--//
    tempResult=TimeLapsedEventSender(ITIlaseryes, SOneLaserReverseOrnot, 4,0.5 * ITI,0,tempResult,TransWaterDura);

    timerCounterI = 0;
    while (timerCounterI < 0.5 * ITI * 1000) {
        CQ_LickSignalSender(CQ_lickOrAll, 20);
    }
}

void CQ_AutoGoNogoTask(int rule, int LaserPeriod, int SOneLaserReverseOrnot, int Odor1, int Odor1BNC, int Odor2, int Odor2BNC, float odorLen, float delay, float ResponseCue, float waterlen, float ITI, int TrialNuminSession, int SessionNum,int TransWaterDura) {

    home_clr();
    puts_lcd((unsigned char*) &text7[0], sizeof (text7));
    line_2();
    puts_lcd((unsigned char*) &text8[0], sizeof (text8));

    int odorlaseryes = 0;
    int delaylaseryes = 0;
    int waterlaseryes = 0;
    int ITIlaseryes = 0;
    int ResponseCueyes = 0;
    switch (LaserPeriod) {
        case 1:
            break;
        case 2:
            odorlaseryes = 1;
            break;
        case 3:
            delaylaseryes = 1;
            break;
        case 4:
            odorlaseryes = 1;
            delaylaseryes = 1;
            waterlaseryes = 1;
            break;
        case 5:
            ITIlaseryes = 1;
            break;
    }
    int currentTrial = 0;
    int sessionnum = 0;
    int RuleSwitch = 0;
    int trialnum;
    int sequence = 0;
    int ind;
    int tempInt;

    int Odor[4] = {Odor1,Odor2,Odor1,Odor2};
    int OdorBNC[4] = {Odor1BNC,Odor2BNC,Odor1BNC,Odor2BNC};

    DelayNmSecNew(2 * 1000);

    while (sessionnum<=SessionNum) {
        home_clr();
        puts_lcd((unsigned char*) &text7[0], sizeof (text7));
        line_2();
        puts_lcd((unsigned char*) &text8[0], sizeof (text8));

        counts_dispose(RuleSwitch);
        LCD_set_xy(3, 1);
        lcd_data(tens);
        lcd_data(ones);

        sessionnum++;
        localSendOnce(SpSess, 1);
        localSendOnce(59, 0);

        counts_dispose(sessionnum);
        LCD_set_xy(1, 2);
        lcd_data(tens);
        lcd_data(ones);

        LCD_set_xy(1, 1);
        if (rule == 3) {
            lcdWriteString("BG");
        } else {
            lcdWriteString("JG");
        }
        currentTrial = 0;
        hit = 0;
        miss = 0;
        falseAlarm = 0;
        correctRejection = 0;
        if (sequence != 4) {
            sequence = 4;
        }
        for (trialnum = 1; trialnum <= TrialNuminSession; trialnum++) {
            if (sequence == 0) {
                sequence = 4;
            }
            ind = rand() % sequence;
            CQ_OneOdorResponseWithTwoTrigger(RuleSwitch, trialnum, SOneLaserReverseOrnot, Odor[ind], OdorBNC[ind], odorLen, delay, ResponseCue, waterlen, ITI, odorlaseryes, delaylaseryes, ResponseCueyes, waterlaseryes, ITIlaseryes, rule,TransWaterDura);
            tempInt = Odor[sequence - 1];
            Odor[sequence - 1] = Odor[ind];
            Odor[ind] = tempInt;
            tempInt = OdorBNC[sequence - 1];
            OdorBNC[sequence - 1] = OdorBNC[ind];
            OdorBNC[ind] = tempInt;
            sequence--;
        }
        localSendOnce(SpSess, 0);

        if (SOneLaserReverseOrnot != 2) {
            if (correctRatio >= 85) {
                int RandTrial = rand() % 35 + 5; //
                if (RandTrial == 20 || RandTrial == 0) {
                    RandTrial = 21;
                }
                hit = 0;
                miss = 0;
                falseAlarm = 0;
                correctRejection = 0;
                home_clr();
                puts_lcd((unsigned char*) &text7[0], sizeof (text7));
                line_2();
                puts_lcd((unsigned char*) &text8[0], sizeof (text8));

                LCD_set_xy(1, 1);
                if (rule == 3) {
                    lcdWriteString("BR");
                } else {
                    lcdWriteString("JR");
                }
                counts_dispose(RandTrial);
                LCD_set_xy(3, 2);
                lcd_data(tens);
                lcd_data(ones);

                counts_dispose(RuleSwitch);
                LCD_set_xy(3, 1);
                lcd_data(tens);
                lcd_data(ones); //LCD����ʾsession��

                sessionnum++;
                localSendOnce(SpSess, 1);
                localSendOnce(59, 0);

                counts_dispose(sessionnum);
                LCD_set_xy(1, 2);
                lcd_data(tens);
                lcd_data(ones); //LCD����ʾsession��

                if (sequence != 4) {
                        sequence = 4;
                    }
                for (trialnum = 1; trialnum < RandTrial; trialnum++) {

                    if (sequence == 0) {
                        sequence = 4;
                    }
                    ind = rand() % sequence;
                    CQ_OneOdorResponseWithTwoTrigger(RuleSwitch, trialnum, SOneLaserReverseOrnot, Odor[ind], OdorBNC[ind], odorLen, delay, ResponseCue, waterlen, ITI, odorlaseryes, delaylaseryes, ResponseCueyes, waterlaseryes, ITIlaseryes, rule,TransWaterDura);
                    tempInt = Odor[sequence - 1];
                    Odor[sequence - 1] = Odor[ind];
                    Odor[ind] = tempInt;
                    tempInt = OdorBNC[sequence - 1];
                    OdorBNC[sequence - 1] = OdorBNC[ind];
                    OdorBNC[ind] = tempInt;
                    sequence--;
                }
                localSendOnce(SpSess, 0);

                if (correctRatio >= 85) {
                    RuleSwitch++;
                    if (rule == 3) {
                        rule = 4;
                    } else if (rule == 4) {
                        rule = 3;
                    }//switch the task rule: 3-BGo, 4-JGo
                    localSendOnce(58, 1);//send out the rule switch time point
                }
                // NewDigt_out(6, 0.001);
            }
        }
    }
    localSendOnce(SpTrain, 0); // send it's the end
}

void Stimulus(int Duration, int Interval, int Pulse) {
    int Times;
    for (Times = 1; Times < Pulse; Times++) {
        timerCounterI = 0;
        while (timerCounterI <= Duration) {
            Out4 = 1;
        }
        Out4 = 0;
        DelayNmSecNew(Interval);
    }
}

void CQFunction(int FunctionID) {
    switch (FunctionID) {
            int n, m,StopOrNot;
            int SOneLaserReverseOrnot;
            int DNMSLaserPer, MOrN, GoNoGoLaserPer, FirstRule, SecondRule;
            int OdorPairSelection;
            int Duration, Interval, Pulse;
        case 2111:
            n = getFuncNumber(1, "BGo3 JGo4 All1");
            m = getFuncNumber(1, "NL1 O2 D3 W4 I5");
            SOneLaserReverseOrnot = getFuncNumber(1, "SOneL1 RevOrnot2");
            CQ_AutoGoNogoTask(n, m, SOneLaserReverseOrnot, odor_A, 2, odor_B, 3, 1.0, 0.3, 0.2, 0.5, 5.0, SlidingWindow, 99,100);
            break;
        case 2112:
            Duration = getFuncNumber(1, "1000 5 10 10");
            Interval = getFuncNumber(1, "1000 45 40 10");
            Pulse = getFuncNumber(1, "20 40 60 80 100");
            Stimulus(Duration, Interval, Pulse);
            break;
        case 2113:
            FirstRule = getFuncNumber(1, "N1 M2 BG3 JG4");
            DNMSLaserPer = getFuncNumber(1, "NL1 O2 D3 W4 I5");
            SOneLaserReverseOrnot = getFuncNumber(1, "SOneL1 RevOrnot2");
            SecondRule = getFuncNumber(1, "N1 M2 BG3 JG4");
            GoNoGoLaserPer = getFuncNumber(1, "NL1 O2 D3 W4 I5");
            CQ_DNMS_GONOGO_Switch(FirstRule, SecondRule, GoNoGoLaserPer, SOneLaserReverseOrnot, DNMSLaserPer);
            break;
        case 2114:
            n = getFuncNumber(1, "Non1 Match2 All3");
            OdorPairSelection = getFuncNumber(1, "BJ1 MK2");
            StopOrNot=getFuncNumber(1, "Stop1 NoStop2");
            CQ_Match_Nonmatch_Shaping(n, OdorPairSelection,StopOrNot, odor_A, 2, odor_B, 3, 1.0, 5.0, 0.8, 0.2,0.5,10,100, odor_C, odor_D,100);
            break;
        case 2115:
            n = getFuncNumber(1, "Non1 Match2 All3");
            m = getFuncNumber(1, "NL1 O2 D3 W4 I5");
            SOneLaserReverseOrnot = getFuncNumber(1, "SOneL1 SwiOrnot2");
            OdorPairSelection = getFuncNumber(1, "BJ1 MK2");
            int ShapeAfterSwitch = getFuncNumber(1, "Shape1 NoShape2");
            CQ_Match_Nonmatch_autoSwitch(n, m, SOneLaserReverseOrnot, OdorPairSelection, ShapeAfterSwitch, odor_A, 2, odor_B, 3, 1.0, 5.0, 0.8, 0.2, 0.5, 10.0, SlidingWindow, odor_C, odor_D,0,4.8,50,15);
            break;
        case 2121:
            MOrN = getFuncNumber(1, "Nonmatch1 Match2");
            DNMSLaserPer = getFuncNumber(1, "NL1 O2 D3 W4 I5");
            SOneLaserReverseOrnot = getFuncNumber(1, "SOneL1 RevOrnot2");
            n = getFuncNumber(1, "BGo3 JGo4 All1");
            m = getFuncNumber(1, "NL1 O2 D3 W4 I5");
            CQ_NM_GNG_NM_Switch(n, m, SOneLaserReverseOrnot, DNMSLaserPer, MOrN);
            break;
        case 2122:
            n = getFuncNumber(1, "BGo3 JGo4 All1");
            m = getFuncNumber(1, "NL1 O2 D3 W4 I5");
            SOneLaserReverseOrnot = getFuncNumber(1, "SOneL1 RevOrnot2");
            MOrN = getFuncNumber(1, "Nonmatch1 Match2");
            DNMSLaserPer = getFuncNumber(1, "NL1 O2 D3 W4 I5");
            CQ_GNG_NM_GNG_Switch(n, m, SOneLaserReverseOrnot, DNMSLaserPer, MOrN);
            break;
        case 2123:
            CQ_Match_Nonmatch_autoSwitch(1, 3, 1, 1, 2, odor_A, 2, odor_B, 3, 0.0, 2.0, 0.8, 0.2, 0.5, 6.5, 120, odor_C, odor_D,0,2.0,10,1);
            break;
        case 2124:
            CQ_Match_Nonmatch_autoSwitch(1, 3, 1, 1, 2, odor_A, 2, odor_B, 3, 0.0, 2.0, 0.8, 0.2, 0.5, 6.5, 240, odor_C, odor_D,0,2.0,10,1);
            break;
        case 2125:
            CQ_Match_Nonmatch_autoSwitch(1, 3, 1, 1, 2, odor_A, 2, odor_B, 3, 0.0, 2.0, 0.8, 0.2, 0.5, 6.5, 480, odor_C, odor_D,0,2.0,10,1);
            break;
        case 2131:
            CQ_Match_Nonmatch_autoSwitch(1, 3, 1, 1, 2, odor_A, 2, odor_B, 3, 0.0, 2.0, 0.8, 0.2, 0.5, 6.5, 720, odor_C, odor_D,0,2.0,10,1);
            break;
    }
}

