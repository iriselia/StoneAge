#include "version.h"
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <assert.h>

#if PLATFORM_WINDOWS
#else
#include <unistd.h>
#endif

#include "init.h"
#include "net.h"
#include "handletime.h"
#include "char.h"
#include "map_warppoint.h"
#include "npcgen.h"
#include "log.h"
#include "configfile.h"
#include "battle.h"
#include "petmail.h"
#include "autil.h"
#include "family.h"
#include "defend.h"
#include "trade.h"

#ifdef _NPCSERVER_NEW
#include "npcserver.h"
#endif

#ifdef _RECAL_SEND_COUNT		// WON 傳送GS資訊 
#include "mclient.h"
#endif

#ifdef _DEATH_CONTEND
#include "deathcontend.h"
#endif
#ifdef _CHATROOMPROTOCOL			// (不可開) Syu ADD 聊天室頻道
#include "chatroom.h"
#endif

static void ShutdownProc( void);
void mainloop( void );
struct  tm tmNow, tmOld;
void family_proc();
// Terry add 2001/10/11
#ifdef _SERVICE
extern int g_nServiceSocket;
#endif

#ifdef _CHANNEL_MODIFY
extern int InitOccChannel(void);
#endif

#ifdef _ANGEL_SUMMON
#define ANGELTIMELIMIT	3*24*60*60 // 完成任務時限(秒)
int AngelReady =0;
int AngelTimelimit = ANGELTIMELIMIT;
time_t AngelNextTime;
void AngelReadyProc();
#endif

// TODO: genver.h
// #include "genver.h"

void warplog_proc();

int main( int argc , char** argv, char** env )
{
    /*  とりあえず時間を設定しておく    */
    setNewTime();

	// TODO: genver.h
	/*
    if ( argc > 1 && 0==strcmp(argv[1],"-v"))
    {
        printf("%s",genver);
        exit(0);
    }
    else fprintf(stderr,"%s",genver);
	*/

    EXITWITHEXITCODEIFFALSE( util_Init() , 1);

    LoadAnnounce();	// Arminius 7.12 loginannounce

	/* 初期設定 */
#if PLATFORM_WINDOWS
	struct tm* newtime = _localtime32((time_t*)&NowTime.tv_sec);
#else
	struct tm* newtime = localtime((time_t*)&NowTime.tv_sec);
#endif
	assert(newtime);
	memcpy( &tmOld, newtime, sizeof( tmNow ) );

    EXITWITHEXITCODEIFFALSE( init(argc , argv ,env ) , 1);

    LoadPetTalk();	// Arminius 8.14 pet talk

#ifdef _GAMBLE_BANK
	Load_GambleBankItems();
#endif

#ifdef _CFREE_petskill
	Load_PetSkillCodes();
#endif
#ifdef _ITEM_PILEFORTRADE
	TRADE_InitTradeList();
#endif

#ifdef _DEATH_CONTEND
	if( PKLIST_InitPkTeamList( MAXTEAMNUM ) == -1 ) return 1;
#endif

#if USE_MTIO
    /* マルチスレッドのときはここで分裂 */    
    if( MTIO_setup() < 0 ){
        print( "cannot setup MT environment\n" );
        return 1;
    }

    /* ここで join して終了 */
    MTIO_join();
#else
    mainloop();
#endif
    
    return 0;
}

void mainloop( void )
{
    NPC_generateLoop( 1 );
	// TODO: Fix Signals
	/*
	signal(SIGUSR1,sigusr1);
    signal(SIGUSR2,sigusr2);
	*/
#ifdef _MAP_WARPPOINT
	MAPPOINT_InitMapWarpPoint();
	if( !MAPPOINT_loadMapWarpPoint() ){
		return;
	}
#endif

#ifdef _ASSESS_SYSEFFICACY
	Assess_InitSysEfficacy();
#endif

#ifdef _CHECK_BATTLETIME
	check_battle_com_init();
#endif

#ifdef _CHATROOMPROTOCOL			// (不可開) Syu ADD 聊天室頻道
	InitChatRoom();
#endif

#ifdef _CHANNEL_MODIFY
	if(!InitOccChannel()) return;			// 初始化職業頻道
#endif

#ifdef _GM_BROADCAST					// WON ADD 客服公告系統
	Init_GM_BROADCAST( 0, 0, 0, "" );
#endif

#ifdef _DEATH_FAMILY_STRUCT		// WON ADD 家族戰存放勝負資料
	Init_FM_PK_STRUCT();			
#endif

#ifdef _ANGEL_SUMMON
	AngelReady = 0;
	AngelNextTime = time(NULL) + 1*60;
#endif

//#ifdef _ALLDOMAN
//	InitHeroList();
//#endif
    while(1){
#ifdef _ASSESS_SYSEFFICACY
		Assess_SysEfficacy( 0 );
#endif
        setNewTime();
        memcpy(&tmNow, localtime( (time_t *)&NowTime.tv_sec),
               sizeof( tmNow ) );
		if( tmOld.tm_hour != getLogHour( ) && tmNow.tm_hour == getLogHour( ) ){
			backupAllLogFile( &tmOld );
		}
        setNewTime();

#ifdef _ASSESS_SYSEFFICACY_SUB //顯示LOOP時間
Assess_SysEfficacy_sub( 0, 1);
        netloop_faster();
Assess_SysEfficacy_sub( 1, 1);

//Assess_SysEfficacy_sub( 0, 2);
        NPC_generateLoop( 0 );
//Assess_SysEfficacy_sub( 1, 2);

Assess_SysEfficacy_sub( 0, 3);
        BATTLE_Loop();
Assess_SysEfficacy_sub( 1, 3);

Assess_SysEfficacy_sub( 0, 4);
        CHAR_Loop();
Assess_SysEfficacy_sub( 1, 4);

//Assess_SysEfficacy_sub( 0, 5);
        PETMAIL_proc();
//Assess_SysEfficacy_sub( 1, 5);

//Assess_SysEfficacy_sub( 0, 6);
        family_proc();
//Assess_SysEfficacy_sub( 1, 6);

//Assess_SysEfficacy_sub( 0, 7);
        chardatasavecheck();
//Assess_SysEfficacy_sub( 1, 7);
#ifdef _GM_BROADCAST					// WON ADD 客服公告系統
//Assess_SysEfficacy_sub( 0, 8);
		GM_BROADCAST();
//Assess_SysEfficacy_sub( 1, 8);
#endif

#else	//不顯示LOOP時間
        netloop_faster();
        NPC_generateLoop( 0 );
        BATTLE_Loop();
        CHAR_Loop();
        PETMAIL_proc();
        family_proc();
        chardatasavecheck();
#ifdef _GM_BROADCAST					// WON ADD 客服公告系統
		GM_BROADCAST();
#endif
#endif

#ifdef _ANGEL_SUMMON
		AngelReadyProc();
#endif

		if( tmOld.tm_sec != tmNow.tm_sec ) {
			CHAR_checkEffectLoop();
		}
        if( SERVSTATE_getShutdown()> 0 ) {
            ShutdownProc();
        }

		tmOld = tmNow;
#ifdef _ASSESS_SYSEFFICACY
		Assess_SysEfficacy( 1);
#endif
    }
#ifdef _SERVICE
    close(g_nServiceSocket);
#endif    
}

static void sendmsg_toall( char *msg )
{
    int     i;
    int     playernum = CHAR_getPlayerMaxNum();

    for( i = 0 ; i < playernum ; i++) {
        if( CHAR_getCharUse(i) != FALSE ) {
			CHAR_talkToCli( i, -1, msg, CHAR_COLORYELLOW);
		}
	}
}
static void ShutdownProc( void)
{
#define		SYSINFO_SHUTDOWN_MSG		"再過 %d 分鐘後，即開始進行server系統維護。"
#define		SYSINFO_SHUTDOWN_MSG_COMP	"server已關閉。"
	int diff,hun;

	diff = NowTime.tv_sec - SERVSTATE_getShutdown();
	hun = SERVSTATE_getLimittime() - (diff/60);
	if( hun != SERVSTATE_getDsptime() ){
		char	buff[256];
		if( hun != 0 ) {
			snprintf( buff, sizeof(buff), SYSINFO_SHUTDOWN_MSG, hun);
		}
		else {
			strcpy( buff, SYSINFO_SHUTDOWN_MSG_COMP);
		}
		sendmsg_toall( buff);
		SERVSTATE_setDsptime(hun);
		if( hun == 1 ) {
		    SERVSTATE_SetAcceptMore( 0 );
		}
	}
	/* closesallsockets */
	if( hun == 0) {
    	closeAllConnectionandSaveData();
		SERVSTATE_setShutdown(0);
		SERVSTATE_setDsptime(0);
		SERVSTATE_setLimittime(0);
#ifdef _KILL_12_STOP_GMSV      // WON ADD 下sigusr2後關閉GMSV
		//andy_reEdit 2003/04/28不准開...
//		system("./stop.sh"); 
#endif
	}
	
}

void family_proc()
{
	static	unsigned long gettime = 0;
	static  unsigned long checktime = 0;
	static  unsigned long proctime = 0;

#ifdef _CK_ONLINE_PLAYER_COUNT    // WON ADD 計算線上人數	
	static	unsigned long player_count_time = 0;
    int PLAYER_COUNT_TIME = 60*5;	  // 30秒傳一次人數至 AC
#endif

#ifdef _RECAL_SEND_COUNT		// WON 傳送GS資訊 
	static	unsigned long recal_count_time = 0;
    int RECAL_COUNT_TIME = 60;	  
	if( (unsigned long)NowTime.tv_sec > recal_count_time  ){
		recal_get_count();
#ifdef _GSERVER_RUNTIME //傳送GSERVER執行多少時間給MSERVER
	    gserver_runtime();
#endif
		recal_count_time = (unsigned long)NowTime.tv_sec + RECAL_COUNT_TIME;
	}
#endif

	if( time(NULL) < proctime ) return;
	proctime = time(NULL)+5;

	if( (unsigned long)NowTime.tv_sec > gettime ){
		getNewFMList();
		gettime = (unsigned long)NowTime.tv_sec + 60*10;
	}

	if( (unsigned long)NowTime.tv_sec > checktime ){
		checkFamilyIndex();
#ifdef _ADD_FAMILY_TAX			   // WON ADD 增加莊園稅收	
		GS_ASK_TAX();			   // GS 向 AC 要求莊園稅率
#endif
		checktime = (unsigned long)NowTime.tv_sec + 60*30;
	}

#ifdef _CK_ONLINE_PLAYER_COUNT    // WON ADD 計算線上人數
	if( (unsigned long)NowTime.tv_sec > player_count_time  ){
		GS_SEND_PLAYER_COUNT();
		player_count_time = (unsigned long)NowTime.tv_sec + PLAYER_COUNT_TIME;
	}
#endif
}

void warplog_proc()
{
	static  unsigned long checktime = 0;
	
	if( (unsigned long)NowTime.tv_sec > checktime ){
		warplog_to_file();
		checktime = (unsigned long)NowTime.tv_sec + 300;
	}
}

#ifdef _ANGEL_SUMMON

extern int player_online;

void AngelReadyProc()
{
	//static time_t lastCreateTime = time(NULL);
	time_t nowTime;
	//static unsigned long AngelNextTime = 30*60;
	struct tm *temptime;
	char msg[1024];

	nowTime = time(NULL);

	if( nowTime < AngelNextTime )
		return;

	if( player_online <= 10 )
	{
		//print(" ANGEL:線上人數不足=%d ", player_online);
		return;
	}

	AngelReady = 1;
	//AngelNextTime = min( (int)(10000/player_online), 100)*60 + (unsigned long)nowTime;
	AngelNextTime = min( (int)(5000/player_online), 100)*60 + (unsigned long)nowTime;

	temptime = localtime( &AngelNextTime );
	sprintf( msg, " ANGEL:產生一位缺額  下次產生時間=(%d/%d %d:%d) 目前人數=%d ",
		temptime->tm_mon+1, temptime->tm_mday, temptime->tm_hour, temptime->tm_min, player_online );
	print( msg);
	//LogAngel( msg);
	
}

#endif
