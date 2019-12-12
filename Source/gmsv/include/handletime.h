#ifndef __HANDLETIME_H__ 
#define __HANDLETIME_H__

#if PLATFORM_WINDOWS
#include <time.h>
#include <WinSock2.h>
/*
#ifdef _WINDEF_
#else
typedef struct timeval
{
	long tv_sec;
	long tv_usec;
} timeval;
#endif
//*/
#else
#include <sys/time.h>
#endif

#undef EXTERN
#ifdef __HANDLETIME_C__
#define EXTERN
#else  /*__HANDLETIME_C__*/
#define EXTERN extern
#endif

EXTERN struct timeval NowTime;
EXTERN int     DEBUG_ADJUSTTIME;

BOOL setNewTime( void );
#ifdef _ASSESS_SYSEFFICACY
void Assess_InitSysEfficacy( void);
void Assess_SysEfficacy( int flg);
void ASSESS_getSysEfficacy( float *TVsec);
#ifdef _ASSESS_SYSEFFICACY_SUB
void Assess_SysEfficacy_sub( int flg, int loop);
void ASSESS_getSysEfficacy_sub( float *TVsec, int loop);
#endif
#endif
// WON REM
//struct tm *localtime(const time_t *timep);

/*ここから下、HiO。LS時間関係。*/
/*  実時間150(分)*60=9000秒=750*12 で   LS1日   */
/*  LS1200時間  LS1日   */
/*  LS100日で   LS1年   */
typedef struct tagLSTIME
{
	int year;
	int day;
	int hour;
}LSTIME;

#define NIGHT_TO_MORNING	700
#define MORNING_TO_NOON		930
#define NOON_TO_EVENING		200
#define EVENING_TO_NIGHT	300

typedef enum
{
	LS_NIGHT = 0,
	LS_MORNING ,
	LS_NOON ,
	LS_EVENING ,
}LSTIME_SECTION;

void RealTimeToLSTime(long t , LSTIME *lstime);
/*void LSTimeToRealTime( LSTIME *lstime, long *t);*/
LSTIME_SECTION getLSTime (LSTIME *lstime);

#endif  
