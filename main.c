#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/malloc.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <math.h>

#include "fidlib.h"

#ifndef T_MSVC
#include <unistd.h>
#include <sys/time.h>
#endif

#ifdef T_MSVC
#include <float.h>
#define NAN nan_global
#endif

#ifndef NAN
#define NAN (0.0/0.0)
#endif

#ifndef M_PI
#define M_PI           3.14159265358979323846 
#endif

#include <sys/ioctl.h>
#include <sys/times.h>

enum Filter_Type {
	PkBq,
	BpRe,
	maxq
};
enum Filter_Type filter_type;

#define NUM_FREQS 21
#define NUM_SCALES 11

enum Scale_Type {
	OCTAVE_AND_ONE_INTERVAL,
	INDIAN,
	INDIAN_SPREAD1,
	INDIAN_SPREAD2,
	INDIAN_SPREAD3,
	SEVENTEEN_ET,
	SEVENTEEN_JT,
	GAMMA,
	GAMMA_SPREAD1,
	ALPHA_SPREAD1,
	ALPHA_SPREAD2,
	DIATONIC,
	DIATONIC2,
	TWELVETONE,
	OCTAVE_AND_TWO_INTERVALS,
	MESOPOTAMIAN,
	SHRUTIS,
	B296,
	GAMELAN,
	BOHLEN_PIERCE,
	VIDEOH
};
enum Scale_Type scale_type;

int main(void){
	FidFilter *filt;
	char *desc;
	char str[80];
	double *coef;
	int len,i,j;
	int t;
	double gain_adj;
	double val0;
	int Qval;
	uint32_t sample_rate;
	int filter_type_i=0;
	double freqs[21];
	double start_freq;
	double freq0;
	double rootA;
	double gain_q;
	int scale_i;
	int q;
	double interval[NUM_SCALES];
	double interval2[NUM_SCALES];
	double two_intervals[NUM_SCALES][2];
	double n_intervals[NUM_SCALES][21];
	double cents;
	char interval_string[NUM_SCALES][80];
	char scale_name[80];
	short num_qs;

	double I0=1.0535, I1=1.125, I18=1.1852, I2=1.2656, I3=1.3333, I4=1.4047, I5=1.5, I58=1.5802, I6=1.6875, I7=1.7777, I8=1.8984;
	double b296_freqs[21]={20, 40, 60, 80, 100, 150, 250, 350, 500, 630,800,1000,1300,1600,2000,2600,3500,5000,8000,10000,20000};
	double BP_intervals[14]={1, (27.0/25), (25.0/21.0), (9.0/7.0), (7.0/5.0), (75.0/49.0), (5.0/3.0), (9.0/5.0), 49.0/25.0, 15.0/7.0, 7.0/3.0, 63.0/25.0, 25.0/9.0, 3};


	//set filter characteristics here: (these should be arguments)
	sample_rate=96000;

	//scale_type=OCTAVE_AND_ONE_INTERVAL;
	//scale_type=INDIAN;
	//scale_type=SEVENTEEN_ET;
				//scale_type=GAMMA;//not used
	//scale_type=GAMMA_SPREAD1;
	//scale_type=ALPHA_SPREAD1;
	//scale_type=ALPHA_SPREAD2;
	//scale_type=DIATONIC;
	//scale_type=DIATONIC2;
	//scale_type=TWELVETONE;
	//scale_type=OCTAVE_AND_TWO_INTERVALS;
	//scale_type=MESOPOTAMIAN;
	//scale_type=SHRUTIS;
	scale_type=B296;
	//scale_type=GAMELAN;
	//scale_type=BOHLEN_PIERCE;
	//scale_type=VIDEOH; //not used


	if (scale_type==VIDEOH){
		strcpy(scale_name,"video_h");
		start_freq=59.94;
	}

	if (scale_type==BOHLEN_PIERCE){
		start_freq=32.7031956626; //C1
		strcpy(scale_name,"bohlen_pierce"); //13 notes per "tritave", equal tempered
		t=0;
		n_intervals[t][0]=BP_intervals[3];
		n_intervals[t][1]=BP_intervals[7];
		n_intervals[t][2]=BP_intervals[10];
		t++;
		n_intervals[t][0]=BP_intervals[3];
		n_intervals[t][1]=BP_intervals[7];
		n_intervals[t][2]=BP_intervals[11];
		t++;
		n_intervals[t][0]=BP_intervals[4];
		n_intervals[t][1]=BP_intervals[6];
		n_intervals[t][2]=BP_intervals[10];
		t++;
		n_intervals[t][0]=BP_intervals[4];
		n_intervals[t][1]=BP_intervals[7];
		n_intervals[t][2]=BP_intervals[9];
		t++;
		n_intervals[t][0]=BP_intervals[4];
		n_intervals[t][1]=BP_intervals[7];
		n_intervals[t][2]=BP_intervals[10];
		t++;
		n_intervals[t][0]=BP_intervals[4];
		n_intervals[t][1]=BP_intervals[7];
		n_intervals[t][2]=BP_intervals[11];
		t++;
		n_intervals[t][0]=BP_intervals[6];
		n_intervals[t][1]=BP_intervals[7];
		n_intervals[t][2]=BP_intervals[10];
		t++;
		n_intervals[t][0]=BP_intervals[6];
		n_intervals[t][1]=BP_intervals[7];
		n_intervals[t][2]=BP_intervals[11];
		t++;
		n_intervals[t][0]=BP_intervals[6];
		n_intervals[t][1]=BP_intervals[10];
		n_intervals[t][2]=BP_intervals[11];
		t++;
		n_intervals[t][0]=BP_intervals[6];
		n_intervals[t][1]=BP_intervals[8];
		n_intervals[t][2]=BP_intervals[12];
		t++;
		n_intervals[t][0]=BP_intervals[5];
		n_intervals[t][1]=BP_intervals[9];
		n_intervals[t][2]=BP_intervals[12];
	}

	if (scale_type==GAMELAN){
		strcpy(scale_name,"gamelan");
		start_freq=32.7031956626; //C1
		n_intervals[0][0]=pow(2,1.0/5);
		n_intervals[0][1]=pow(2,2.0/5);
		n_intervals[0][2]=pow(2,3.0/5);
		n_intervals[0][3]=pow(2,4.0/5);

		n_intervals[2][0]=pow(2,1.0/7);
		n_intervals[2][1]=pow(2,2.0/7);
		n_intervals[2][2]=pow(2,3.0/7);
		n_intervals[2][3]=pow(2,4.0/7);
		n_intervals[2][4]=pow(2,5.0/7);
		n_intervals[2][5]=pow(2,6.0/7);

		//1 2 3 5 6
		n_intervals[5][0]=pow(2,1.0/7);
		n_intervals[5][1]=pow(2,2.0/7);
		n_intervals[5][2]=pow(2,4.0/7);
		n_intervals[5][3]=pow(2,5.0/7);
		n_intervals[6][0]=pow(2,1.0/7);
		n_intervals[6][1]=pow(2,2.0/7);
		n_intervals[6][2]=pow(2,4.0/7);
		n_intervals[6][3]=pow(2,5.0/7);

		//1 2 4 5 7
		n_intervals[7][0]=pow(2,1.0/7);
		n_intervals[7][1]=pow(2,3.0/7);
		n_intervals[7][2]=pow(2,4.0/7);
		n_intervals[7][3]=pow(2,6.0/7);
		n_intervals[8][0]=pow(2,1.0/7);
		n_intervals[8][1]=pow(2,3.0/7);
		n_intervals[8][2]=pow(2,4.0/7);
		n_intervals[8][3]=pow(2,6.0/7);

		//1 3 4 5 6
		n_intervals[9][0]=pow(2,2.0/7);
		n_intervals[9][1]=pow(2,3.0/7);
		n_intervals[9][2]=pow(2,4.0/7);
		n_intervals[9][3]=pow(2,5.0/7);
		n_intervals[10][0]=pow(2,2.0/7);
		n_intervals[10][1]=pow(2,3.0/7);
		n_intervals[10][2]=pow(2,4.0/7);
		n_intervals[10][3]=pow(2,5.0/7);


	}

	if (scale_type==B296){
		strcpy(scale_name,"B296");
	}

	if (scale_type==SHRUTIS){
		strcpy(scale_name,"shrutis");
		start_freq=16.3515978313; //C0
		t=0;
		n_intervals[0][t++]=1.0;
		n_intervals[0][t++]=16/15.0;
		n_intervals[0][t++]=10/9.0;
		n_intervals[0][t++]=9/8.0;
		n_intervals[0][t++]=32/27.0;
		n_intervals[0][t++]=6/5.0;
		n_intervals[0][t++]=5/4.0;
		n_intervals[0][t++]=81/64.0;
		n_intervals[0][t++]=4/3.0;
		n_intervals[0][t++]=27/20.0;
		n_intervals[0][t++]=45/32.0;
		n_intervals[0][t++]=729/512.0;
		n_intervals[0][t++]=3/2.0;
		n_intervals[0][t++]=128/81.0;
		n_intervals[0][t++]=8/5.0;
		n_intervals[0][t++]=5/3.0;
		n_intervals[0][t++]=27/16.0;
		n_intervals[0][t++]=16/9.0;
		n_intervals[0][t++]=9/5.0;
		n_intervals[0][t++]=15/8.0;
		n_intervals[0][t++]=243/128.0;
	}

	if (scale_type==MESOPOTAMIAN){
		strcpy(scale_name,"mesopotamian");
		start_freq=55; //A2
		t=0;
		strcpy(interval_string[t],"ISHARTUM");
		n_intervals[t][0]=I0;
		n_intervals[t][1]=I18;
		n_intervals[t][2]=I3;
		n_intervals[t][3]=I5;
		n_intervals[t][4]=I58;
		n_intervals[t][5]=I7;
		t++;
		strcpy(interval_string[t],"ISHARTUM");
		n_intervals[t][0]=I0;
		n_intervals[t][1]=I18;
		n_intervals[t][2]=I3;
		n_intervals[t][3]=I5;
		n_intervals[t][4]=I58;
		n_intervals[t][5]=I7;
		t++;
		strcpy(interval_string[t],"EMBULUM");
		n_intervals[t][0]=I1;
		n_intervals[t][1]=I18;
		n_intervals[t][2]=I3;
		n_intervals[t][3]=I5;
		n_intervals[t][4]=I6;
		n_intervals[t][5]=I7;
		t++;
		strcpy(interval_string[t],"EMBULUM");
		n_intervals[t][0]=I1;
		n_intervals[t][1]=I18;
		n_intervals[t][2]=I3;
		n_intervals[t][3]=I5;
		n_intervals[t][4]=I6;
		n_intervals[t][5]=I7;
		t++;
		strcpy(interval_string[t],"NIDMURUB");
		n_intervals[t][0]=I1;
		n_intervals[t][1]=I2;
		n_intervals[t][2]=I3;
		n_intervals[t][3]=I5;
		n_intervals[t][4]=I6;
		n_intervals[t][5]=I8;
		t++;
		strcpy(interval_string[t],"NIDMURUB");
		n_intervals[t][0]=I1;
		n_intervals[t][1]=I2;
		n_intervals[t][2]=I3;
		n_intervals[t][3]=I5;
		n_intervals[t][4]=I6;
		n_intervals[t][5]=I8;
		t++;
		strcpy(interval_string[t],"QUABLITUM");
		n_intervals[t][0]=I0;
		n_intervals[t][1]=I18;
		n_intervals[t][2]=I4;
		n_intervals[t][3]=I5;
		n_intervals[t][4]=I58;
		n_intervals[t][5]=I7;
		t++;
		strcpy(interval_string[t],"QUABLITUM");
		n_intervals[t][0]=I0;
		n_intervals[t][1]=I18;
		n_intervals[t][2]=I4;
		n_intervals[t][3]=I5;
		n_intervals[t][4]=I58;
		n_intervals[t][5]=I7;
		t++;
		strcpy(interval_string[t],"KITMUN");
		n_intervals[t][0]=I1;
		n_intervals[t][1]=I2;
		n_intervals[t][2]=I3;
		n_intervals[t][3]=I5;
		n_intervals[t][4]=I6;
		n_intervals[t][5]=I7;
		t++;
		strcpy(interval_string[t],"KITMUN");
		n_intervals[t][0]=I1;
		n_intervals[t][1]=I2;
		n_intervals[t][2]=I3;
		n_intervals[t][3]=I5;
		n_intervals[t][4]=I6;
		n_intervals[t][5]=I7;
		t++;
		strcpy(interval_string[t],"MITUM");
		n_intervals[t][0]=I1;
		n_intervals[t][1]=I2;
		n_intervals[t][2]=I4;
		n_intervals[t][3]=I5;
		n_intervals[t][4]=I58;
		n_intervals[t][5]=I7;

	}
	if (scale_type==ALPHA_SPREAD1){
		start_freq=20.60172231; //E0
		cents=78.0;
		strcpy(scale_name,"alpha_spread1");

		interval[0]=4; interval2[0]=10;
		interval[1]=5; interval2[1]=10;
		interval[2]=4; interval2[2]=12;
		interval[3]=5; interval2[3]=11;
		interval[4]=6; interval2[4]=10;
		interval[5]=7; interval2[5]=8;
		interval[6]=8; interval2[6]=7;
		interval[7]=9; interval2[7]=7;
		interval[8]=10; interval2[8]=5;
		interval[9]=11; interval2[9]=5;
		interval[10]=12; interval2[10]=4;
	}
	if (scale_type==ALPHA_SPREAD2){
		start_freq=82.4068892282; //E2
		cents=78.0;
		strcpy(scale_name,"alpha_spread2");

		interval[0]=1; interval2[0]=11;
		interval[1]=2; interval2[1]=10;
		interval[2]=3; interval2[2]=9;
		interval[3]=4; interval2[3]=8;
		interval[4]=5; interval2[4]=7;
		interval[5]=6; interval2[5]=6;
		interval[6]=7; interval2[6]=5;
		interval[7]=8; interval2[7]=4;
		interval[8]=9; interval2[8]=3;
		interval[9]=10; interval2[9]=2;
		interval[10]=11; interval2[10]=1;
	}
	
	if (scale_type==GAMMA){
		start_freq=110; //A3
		cents=35.099;
		strcpy(scale_name,"gamma");
	}
	if (scale_type==GAMMA_SPREAD1){
		start_freq=20.60172231; //E0
		cents=35.099;
		strcpy(scale_name,"gammaspread1");

		interval[0]=3; interval2[0]=30;
		interval[1]=5; interval2[1]=29;
		interval[2]=9; interval2[2]=25;
		interval[3]=11; interval2[3]=23;
		interval[4]=13; interval2[4]=21;
		interval[5]=15; interval2[5]=19;
		interval[6]=18; interval2[6]=16;
		interval[7]=20; interval2[7]=14;
		interval[8]=24; interval2[8]=10;
		interval[9]=29; interval2[9]=5;
		interval[10]=31; interval2[10]=3;
	}
	if (scale_type==SEVENTEEN_ET){
		start_freq=13.75;
		n_intervals[0][0]=pow(2,1.0/17);
		strcpy(scale_name,"17ET");
	}
	if (scale_type==INDIAN){
		start_freq=20.60172231; //E0
		strcpy(scale_name,"indian");
	}
	if (scale_type==DIATONIC){
		rootA=110; //A2
		strcpy(scale_name,"diatonic");
	}
	if (scale_type==DIATONIC2){
		rootA=110; //A2
		strcpy(scale_name,"diatonic2");
	}
	if (scale_type==TWELVETONE){
		start_freq=82.4068892282; //E2
		strcpy(scale_name,"twelvetone");
	}
	if (scale_type==OCTAVE_AND_TWO_INTERVALS){
		strcpy(scale_name,"western_twointerval");
		start_freq=48.9994294977; //G1
		t=0;
		two_intervals[t][0]=(9.0/8.0); strcpy(interval_string[t],"M2_5");
		two_intervals[t][1]=(3.0/2.0);t++;
		two_intervals[t][0]=(5.0/4.0); strcpy(interval_string[t],"M3_b5");
		two_intervals[t][1]=(10.0/7.0);t++;
		two_intervals[t][0]=(6.0/5.0); strcpy(interval_string[t],"m3_M5");
		two_intervals[t][1]=(15.0/8.0);t++;
		two_intervals[t][0]=(5.0/4.0); strcpy(interval_string[t],"M3_5");
		two_intervals[t][1]=(3.0/2.0);t++;
		two_intervals[t][0]=(6.0/5.0); strcpy(interval_string[t],"m3_#5");
		two_intervals[t][1]=(10.0/7.0);t++;
		two_intervals[t][0]=(4.0/3.0); strcpy(interval_string[t],"4_5");
		two_intervals[t][1]=(3.0/2.0);t++;
		two_intervals[t][0]=(5.0/4.0); strcpy(interval_string[t],"M3_M6");
		two_intervals[t][1]=(5.0/3.0);t++;
		two_intervals[t][0]=(6.0/5.0); strcpy(interval_string[t],"m3_b6");
		two_intervals[t][1]=(8.0/5.0);t++;
		two_intervals[t][0]=(5.0/4.0); strcpy(interval_string[t],"M3_#5");
		two_intervals[t][1]=(8.0/5.0);t++;
		two_intervals[t][0]=(6.0/5.0); strcpy(interval_string[t],"m3_m7");
		two_intervals[t][1]=(16.0/9.0);t++;
		two_intervals[t][0]=(3.0/2.0); strcpy(interval_string[t],"5_M7");
		two_intervals[t][1]=(15.0/8.0);t++;
	}

	if (scale_type==OCTAVE_AND_ONE_INTERVAL){
		strcpy(scale_name,"western");
		start_freq=27.5; //A1
		interval[10]=(15.0/8.0); strcpy(interval_string[10],"majorseventh");
		//interval[9]=(9.0/5.0); interval_string="minorseventh";
		interval[9]=(16.0/9.0); strcpy(interval_string[9],"minorseventh");
		interval[8]=(5.0/3.0); strcpy(interval_string[8],"majorsixth");
		interval[7]=(8.0/5.0); strcpy(interval_string[7],"minorsixth");
		interval[6]=(3.0/2.0);strcpy(interval_string[6],"perfectfifth");
		interval[5]=(10.0/7.0);strcpy(interval_string[5],"tritone");
		interval[4]=(4.0/3.0); strcpy(interval_string[4],"perfectfourth");
		interval[3]=(5.0/4.0); strcpy(interval_string[3],"majorthird");
		interval[2]=(6.0/5.0); strcpy(interval_string[2],"minorthird");
		interval[1]=(9.0/8.0); strcpy(interval_string[1],"majorsecond");
		interval[0]=(16.0/15.0); strcpy(interval_string[0],"minorsecond");
	}
	



	for (filter_type_i=0;filter_type_i<2;filter_type_i++){
		if (filter_type_i==0) {
			filter_type=maxq;
		} else {
			filter_type=BpRe;
		}

		if (filter_type==BpRe) num_qs=2;
		else num_qs=1;

		for (q=0;q<num_qs;q++){
			freq0=start_freq;
			if (scale_type==INDIAN_SPREAD3) scale_type=INDIAN;

			if (q==0){
				Qval=800;gain_q=40;
			//	Qval=200;gain_q=10;
			} else {
				Qval=2;gain_q=2.0;
			}

			if (filter_type==BpRe)
				printf("const float filter_bpre_coefs_%s_%dQ[%d][3]={ // %d \n", scale_name, Qval, NUM_FREQS*NUM_SCALES, sample_rate);
			else if (filter_type==maxq)
				printf("const float filter_maxq_coefs_%s[%d][3]={ // %d \n", scale_name, NUM_FREQS*NUM_SCALES, sample_rate);



			for (scale_i=0;scale_i<NUM_SCALES;scale_i++){

				if (scale_type==VIDEOH){

					if (scale_i<10){
						sprintf(interval_string[scale_i],"video_H_%d",scale_i);
						for(i=0;i<21;i++)
							freqs[i]=start_freq*(i+1)*(scale_i+1);
					} else {
						sprintf(interval_string[scale_i],"video_V");
						for(i=0;i<11;i++)
							freqs[i]=15729+i;
						for(i=11;i<21;i++)
							freqs[i]=31463+i;


					}
				}

				if (scale_type==BOHLEN_PIERCE){
					sprintf(interval_string[scale_i],"bohlen_pierce_%g:%g:%g",n_intervals[scale_i][0], n_intervals[scale_i][1], n_intervals[scale_i][2] );
					freqs[0]=start_freq;
					freqs[1]=freqs[0]*n_intervals[scale_i][0];
					freqs[2]=freqs[0]*n_intervals[scale_i][1];
					freqs[3]=freqs[0]*n_intervals[scale_i][2];
					for (i=4;i<21;i++)
						freqs[i]=freqs[i-4]*3; //tritave
				}

				if (scale_type==B296){
					sprintf(interval_string[scale_i],"B296_#%d",scale_i);
					start_freq=pow(2,scale_i/24.0); //freq increases with each scale
					for (i=0;i<21;i++){
						freqs[i]=start_freq * b296_freqs[i];
					}

				}

				if (scale_type==SHRUTIS){
					sprintf(interval_string[scale_i],"shrutis 22-note ocatve #%d",scale_i);

					for (i=0;i<21;i++){
						freqs[i]=start_freq * n_intervals[0][i] * pow(2,scale_i);
					}
				}

				if (scale_type== MESOPOTAMIAN){
					if (scale_i & 1) freqs[0]=start_freq*8;
					else freqs[0]=start_freq;

					freqs[1]=freqs[0]*n_intervals[scale_i][0];
					freqs[2]=freqs[0]*n_intervals[scale_i][1];
					freqs[3]=freqs[0]*n_intervals[scale_i][2];
					freqs[4]=freqs[0]*n_intervals[scale_i][3];
					freqs[5]=freqs[0]*n_intervals[scale_i][4];
					freqs[6]=freqs[0]*n_intervals[scale_i][5];
					for (i=7;i<21;i++){
						freqs[i]=freqs[i-7]*2;
					}

				}

				if (scale_type==OCTAVE_AND_ONE_INTERVAL){
					freqs[0]=freq0;
					freqs[1]=freq0*interval[scale_i];
					for (i=2;i<21;i++){
						freqs[i]=freqs[i-2]*2;
					}
				}
				if (scale_type==OCTAVE_AND_TWO_INTERVALS){
					freqs[0]=freq0;
					freqs[1]=freq0*two_intervals[scale_i][0];
					freqs[2]=freq0*two_intervals[scale_i][1];
					for (i=3;i<21;i++){
						freqs[i]=freqs[i-3]*2;
					}
				}

				else if (scale_type==INDIAN){
					sprintf(interval_string[scale_i],"indian%g",freq0);
					freqs[0]=freq0;
					freqs[1]=freq0*(9.0/8.0);
					freqs[2]=freq0*(5.0/4.0);
					freqs[3]=freq0*(4.0/3.0);
					freqs[4]=freq0*(3.0/2.0);
					freqs[5]=freq0*(5.0/3.0);
					freqs[6]=freq0*(15.0/8.0);
					for (i=7;i<21;i++){
						freqs[i]=freqs[i-7]*2;
					}
					freq0=freq0*2;
					if (freq0>2560)
						scale_type=INDIAN_SPREAD1;
				}
				else if (scale_type==INDIAN_SPREAD1){
					freq0=120;
					sprintf(interval_string[scale_i],"indiansp1_%g",freq0);
					freqs[0]=freq0;
					freqs[1]=freq0*(9.0/8.0);
					freqs[2]=freq0*(4.0/3.0);
					freqs[3]=freq0*(15.0/8.0);
					for (i=4;i<21;i++){
						freqs[i]=freqs[i-4]*2;
					}
					scale_type=INDIAN_SPREAD2;
				}
				else if (scale_type==INDIAN_SPREAD2){
					freq0=120;
					sprintf(interval_string[scale_i],"indiansp2_%g",freq0);
					freqs[0]=freq0;
					freqs[1]=freq0*(5.0/4.0);
					freqs[2]=freq0*(5.0/3.0);
					freqs[3]=freq0*(15.0/8.0);
					freqs[4]=freq0*(9.0/8.0)*2.0;
					freqs[5]=freq0*(4.0/3.0)*2.0;
					freqs[6]=freq0*(3.0/2.0)*2.0;
					for (i=7;i<21;i++){
						freqs[i]=freqs[i-7]*4.0;
					}
					scale_type=INDIAN_SPREAD3;
				}
				else if (scale_type==INDIAN_SPREAD3){
					freq0=20;
					sprintf(interval_string[scale_i],"indiansp3_%g",freq0);
					freqs[0]=freq0;
					freqs[1]=freq0*(9.0/8.0);
					freqs[2]=freq0*2.0;
					freqs[3]=freq0*(5.0/4.0)*2.0;
					freqs[4]=freq0*4.0;
					freqs[5]=freq0*(4.0/3.0)*4.0;
					freqs[6]=freq0*8.0;
					freqs[7]=freq0*(3.0/2.0)*8.0;
					freqs[8]=freq0*16.0;
					freqs[9]=freq0*(5.0/3.0)*16.0;
					freqs[10]=freq0*32.0;
					freqs[11]=freq0*(15.0/8.0)*32.0;
					for (i=12;i<21;i++){
						freqs[i]=freqs[i-12]*64.0;
					}
				}
				else if (scale_type==DIATONIC){
					if (scale_i==0 || scale_i==1){
						freq0=rootA; //A major
						if (scale_i & 1) freq0=freq0 * 4; //go up two octave
						sprintf(interval_string[scale_i],"diatonic_Amaj_%g",freq0);
					}
					if (scale_i==2 || scale_i==3){
						freq0=rootA*pow(2,1.0/12.0); //A# Major
						if (scale_i & 1) freq0=freq0 * 4; //go up two octave
						sprintf(interval_string[scale_i],"diatonic_Bbmaj_%g",freq0);
					}
					if (scale_i==4 || scale_i==5){
						freq0=rootA*pow(2,2.0/12.0); //B Major
						if (scale_i & 1) freq0=freq0 * 4; //go up two octave
						sprintf(interval_string[scale_i],"diatonic_Bmaj_%g",freq0);
					}
					if (scale_i==6 || scale_i==7){
						freq0=rootA*pow(2,3.0/12.0); //C Major
						if (scale_i & 1) freq0=freq0 * 4; //go up two octave
						sprintf(interval_string[scale_i],"diatonic_Cmaj_%g",freq0);
					}
					if (scale_i==8 || scale_i==9){
						freq0=rootA*pow(2,4.0/12.0); //C# Major
						if (scale_i & 1) freq0=freq0 * 4; //go up two octave
						sprintf(interval_string[scale_i],"diatonic_Dbmaj_%g",freq0);
					}
					if (scale_i==10 || scale_i==11){
						freq0=rootA*pow(2,5.0/12.0)*2; //D Major
						if (scale_i & 1) freq0=freq0 * 4; //go up two octave
						sprintf(interval_string[scale_i],"diatonic_Dmaj_%g",freq0);

					}
					freqs[0]=freq0;
					freqs[1]=freq0*(9.0/8.0);
					freqs[2]=freq0*(5.0/4.0);
					freqs[3]=freq0*(4.0/3.0);
					freqs[4]=freq0*(3.0/2.0);
					freqs[5]=freq0*(5.0/3.0);
					freqs[6]=freq0*(15.0/8.0);

					for (i=7;i<21;i++){
						freqs[i]=freqs[i-7]*2.0;
					}

				}
				else if (scale_type==DIATONIC2){
					if (scale_i==0 || scale_i==1){
						freq0=rootA*pow(2,6.0/12.0); //C# major
						if (scale_i & 1) freq0=freq0 * 4; //go up two octave
						sprintf(interval_string[scale_i],"diatonic_Dbmaj_%g",freq0);
					}
					if (scale_i==2 || scale_i==3){
						freq0=rootA*pow(2,7.0/12.0); //E Major
						if (scale_i & 1) freq0=freq0 * 4; //go up two octave
						sprintf(interval_string[scale_i],"diatonic_Emaj_%g",freq0);
					}
					if (scale_i==4 || scale_i==5){
						freq0=rootA*pow(2,8.0/12.0); //F Major
						if (scale_i & 1) freq0=freq0 * 4; //go up two octave
						sprintf(interval_string[scale_i],"diatonic_Fmaj_%g",freq0);
					}
					if (scale_i==6 || scale_i==7){
						freq0=rootA*pow(2,9.0/12.0); //F# Major
						if (scale_i & 1) freq0=freq0 * 4; //go up two octave
						sprintf(interval_string[scale_i],"diatonic_Gbmaj_%g",freq0);
					}
					if (scale_i==8 || scale_i==9){
						freq0=rootA*pow(2,10.0/12.0); //G Major
						if (scale_i & 1) freq0=freq0 * 4; //go up two octave
						sprintf(interval_string[scale_i],"diatonic_Gmaj_%g",freq0);
					}
					if (scale_i==10 || scale_i==11){
						freq0=rootA*pow(2,11.0/12.0)*2; //G# Major
						if (scale_i & 1) freq0=freq0 * 4; //go up two octave
						sprintf(interval_string[scale_i],"diatonic_Abmaj_%g",freq0);

					}
					freqs[0]=freq0;
					freqs[1]=freq0*(9.0/8.0);
					freqs[2]=freq0*(5.0/4.0);
					freqs[3]=freq0*(4.0/3.0);
					freqs[4]=freq0*(3.0/2.0);
					freqs[5]=freq0*(5.0/3.0);
					freqs[6]=freq0*(15.0/8.0);

					for (i=7;i<21;i++){
						freqs[i]=freqs[i-7]*2.0;
					}

				}

				else if (scale_type==GAMELAN){
					if (scale_i<=1){
						sprintf(interval_string[0],"gamelan_5note1");
						sprintf(interval_string[1],"gamelan_5note2");
						freq0=start_freq;
						if (scale_i & 1) freq0=freq0 * 16;

						freqs[0]=freq0;
						freqs[1]=freq0*n_intervals[0][0];
						freqs[2]=freq0*n_intervals[0][1];
						freqs[3]=freq0*n_intervals[0][2];
						freqs[4]=freq0*n_intervals[0][3];
						for (i=5;i<21;i++){
							freqs[i]=freqs[i-5]*2.0;
						}
					}

					if (scale_i>1 && scale_i<=4){
						sprintf(interval_string[2],"gamelan_7note1");
						sprintf(interval_string[3],"gamelan_7note2");
						sprintf(interval_string[4],"gamelan_7note3");
						freq0=start_freq+scale_i;
						if (scale_i==3) freq0=freq0 * 8;
						if (scale_i==4) freq0=freq0 * 16;
						freqs[0]=freq0;
						freqs[1]=freq0*n_intervals[2][0];
						freqs[2]=freq0*n_intervals[2][1];
						freqs[3]=freq0*n_intervals[2][2];
						freqs[4]=freq0*n_intervals[2][3];
						freqs[5]=freq0*n_intervals[2][4];
						freqs[6]=freq0*n_intervals[2][5];
						for (i=7;i<21;i++){
							freqs[i]=freqs[i-7]*2.0;
						}
					}

					if (scale_i>4){ //5 6, 7 8, 9 10
						sprintf(interval_string[scale_i],"gamelan_5of7_%d",scale_i-4);
						freq0=start_freq;
						if (!(scale_i & 1)) freq0=freq0 * 16;

						freqs[0]=freq0+scale_i;
						freqs[1]=freq0*n_intervals[scale_i][0];
						freqs[2]=freq0*n_intervals[scale_i][1];
						freqs[3]=freq0*n_intervals[scale_i][2];
						freqs[4]=freq0*n_intervals[scale_i][3];
						for (i=5;i<21;i++){
							freqs[i]=freqs[i-5]*2.0;
						}
					}
				}

				else if (scale_type==GAMMA){
					sprintf(interval_string[scale_i],"gamma_%d",scale_i);
					freqs[0]=freq0;
					for (i=1;i<21;i++)
						freqs[i]=freqs[i-1]*pow(2,cents/1200.0);

					freq0=freqs[20];
				}

				else if (scale_type==GAMMA_SPREAD1){
					sprintf(interval_string[scale_i],"gamma_spread_%g/%g",interval[scale_i], interval2[scale_i]);
					freqs[0]=freq0;
					for (i=1;i<21;i+=2){
						freqs[i]=freqs[i-1]*pow(2,cents*interval[scale_i]/1200.0);
						freqs[i+1]=freqs[i]*pow(2,cents*interval2[scale_i]/1200.0);
					}
				}


				else if (scale_type==ALPHA_SPREAD1 || scale_type==ALPHA_SPREAD2){
					sprintf(interval_string[scale_i],"alpha_spread_%g/%g",interval[scale_i], interval2[scale_i]);
					freqs[0]=freq0;
					for (i=1;i<21;i+=2){
						freqs[i]=freqs[i-1]*pow(2,cents*interval[scale_i]/1200.0);
						freqs[i+1]=freqs[i]*pow(2,cents*interval2[scale_i]/1200.0);
					}
				}


				else if (scale_type==TWELVETONE){
					sprintf(interval_string[scale_i],"twelvetone_%d",scale_i);
					freqs[0]=freq0;
					for (i=1;i<21;i++)
						freqs[i]=freqs[i-1]*pow(2,1.0/12.0);
					freq0=freqs[6];
				}


				else if (scale_type==SEVENTEEN_ET){
					sprintf(interval_string[scale_i],"17ET_%g",freq0);
					freqs[0]=freq0;
					for (i=1;i<21;i++)
						freqs[i]=freqs[i-1]*n_intervals[0][0];
					freq0=freq0*2;
				}
					





				if (filter_type==maxq){
					printf("//%s: \n", interval_string[scale_i]);
					for (i=0;i<NUM_FREQS;i++){

						printf("\t%.16g", 2.0*M_PI*(double)freqs[i]/sample_rate);
						if (i<(NUM_FREQS-1) || scale_i<(NUM_SCALES-1)) printf(",");
						printf("\t//%.2fHz\n", freqs[i]);

					}

				}
				if (filter_type==BpRe){
					printf("//%s: \n", interval_string[scale_i]);
					for (i=0;i<NUM_FREQS;i++){

						sprintf(str, "BpRe/%d/%g", Qval, freqs[i]);

						filt= fid_design(str, sample_rate, 0, 0, 0, &desc);

						filt=FFNEXT(filt);
						len=filt->len;

						gain_adj=fid_response(filt, ((double)freqs[i]/sample_rate));
						gain_adj=gain_q/gain_adj;
	
						printf("\t{%.16g, %.16g, %.16g}%c //%gHz\n", gain_adj, filt->val[2], filt->val[1], ((scale_i<NUM_SCALES-1) || (i<NUM_FREQS-1))? ',' : ' ' , freqs[i]);
	
					}
				}
			} //scale
			printf("};\n");
		} //Qval
	} //filter_type_i
}
