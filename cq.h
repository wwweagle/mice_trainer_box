#define CQ_lickOrAll (PORTDbits.RD12 || PORTDbits.RD13 || !PORTDbits.RD14)// 


void CQ_Valve_ON(unsigned int N, unsigned int rate, int Nchan);
void CQ_Valve_OFF(unsigned int N, int Nchan);
void CQ_LickSignalSender(int LickOrNot,int LickInterval);
int CQ_OneOdorJudger(int Odor, int rule);
int TwoOdorJudger(int Odor1, int Odor2, int rule);
void CQ_OdorSPSignalSender(int Odor, int value);
void TwoOdorResponseWithTwoTrigger(int Odor1, int Odor1BNC, float FirstOdorStart, float FirstOdorEnd, int Odor2, int Odor2BNC, float SecondOdorStart, float SecondOdorEnd, int WaterBNC, float WaterStart, float WaterEnd, float WholeLen, int Trigger1, float Trigger1Start, float Trigger1End, int Trigger2, float Trigger2Start, float Trigger2End, int rule, int SOneLaserSwitchOrnot, int RuleSwitch, int LaserPeriod);

void CQ_Match_Nonmatch_Shaping(int rule, int OdorPairSelection,int StopOrNot,int Odor1,int Odor1BNC, int Odor2,int Odor2BNC, float odorLen, float delay1,float delay2, float ResponseCue, float waterlen, float ITI, int TotalTrialNum,int Odor3, int  Odor4 ,int TransWaterDura);
void CQ_Match_Nonmatch_autoSwitch(int rule, int LaserPeriod, int SOneLaserSwitchOrnot, int OdorPairSelection, int ShapeAfterSwitch, int Odor1, int Odor1BNC, int Odor2, int Odor2BNC, float odorLen, float delay1, float delay2, float ResponseCue, float waterlen, float ITI, int TrialNuminSession, int Odor3, int Odor4,float DelaylaserStart,float DelayLaserEnd,int TransWaterDura,int SessionNum);


int CQ_OneOdorJudger(int Odor, int rule);
void CQ_OneOdorResponseWithTwoTrigger(int reversal, int sessionNum, int SOneLaserReverseOrnot, int Odor, int OdorBNC, float odorLen, float delay, float RespinseCue, float waterlen, float ITI, int odorlaseryes, int delaylaseryes, int ResponseCueyes, int waterlaseryes, int ITIlaseryes, int rule, int TransWaterDura);
void CQ_AutoGoNogoTask(int rule, int LaserPeriod, int SOneLaserReverseOrnot, int Odor1, int Odor1BNC, int Odor2, int Odor2BNC, float odorLen, float delay, float RespinseCue, float waterlen, float ITI, int TrialNuminSession, int SessionNum, int TransWaterDura);

void Stimulus(int Duration,int Interval,int Pulse);

void CQFunction(int n);