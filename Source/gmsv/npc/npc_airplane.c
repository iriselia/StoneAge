#include "version.h"
#include <string.h>
#include "char.h"
#include "object.h"
#include "char_base.h"
#include "npcutil.h"
#include "lssproto_serv.h"
#include "npc_airplane.h"
#include "handletime.h"

/* 
 * 加美航空 (Made from Bus)
 */
 
enum {
	NPC_WORK_ROUTETOX = CHAR_NPCWORKINT1,		/* どこへ？ｘ座標 */
	NPC_WORK_ROUTETOY = CHAR_NPCWORKINT2,		/* どこへ？ｙ座標 */
	NPC_WORK_ROUTEPOINT = CHAR_NPCWORKINT3,		/* 今何番目か */
	NPC_WORK_ROUNDTRIP = CHAR_NPCWORKINT4,		/* 行きか帰りか（０：行き １：帰り）*/
	NPC_WORK_MODE = CHAR_NPCWORKINT5,
	NPC_WORK_CURRENTROUTE = CHAR_NPCWORKINT6, 
	NPC_WORK_ROUTEMAX = CHAR_NPCWORKINT7,
	NPC_WORK_WAITTIME = CHAR_NPCWORKINT8,
	NPC_WORK_CURRENTTIME = CHAR_NPCWORKINT9,
	NPC_WORK_SEFLG = CHAR_NPCWORKINT10,
	NPC_WORK_ONEWAYFLG = CHAR_NPCWORKINT11,
	NPC_WORK_RUNWAVE = CHAR_NPCWORKINT13,
};

/* 拒否メッセージのenum */
enum {
	NPC_AIR_MSG_GETTINGON,
	NPC_AIR_MSG_NOTPARTY,
	NPC_AIR_MSG_OVERPARTY,
	NPC_AIR_MSG_DENIEDITEM,
	NPC_AIR_MSG_ALLOWITEM,
	NPC_AIR_MSG_LEVEL,
	NPC_AIR_MSG_GOLD,
	NPC_AIR_MSG_EVENT,
	NPC_AIR_MSG_START,
	NPC_AIR_MSG_END,
#ifdef _NPC_AIRDELITEM
	NPC_AIR_MSG_DELITEM,
#endif
#ifdef _NPC_AIRLEVEL
	NPC_AIR_MSG_MAXLEVEL,
#endif
};
typedef struct {
	char	option[32];
	char	defaultmsg[128];
}NPC_AIR_MSG;
NPC_AIR_MSG		airmsg[] = {
	{ "msg_gettingon",	"PAON！（你無法於中途加入我們唷！）"},
	{ "msg_notparty",	"PAPAON！！無法以團隊加入唷！"},
	{ "msg_overparty",	"PAON！！人數已滿。"},
	{ "msg_denieditem",		"PAPAON！！我可不要這個道具！"},
	{ "msg_allowitem",		"哇喔~(想要那個道具啊!)"},
	{ "msg_level",		"PAPAON！！你的等級還不夠唷！"},
	{ "msg_stone",		"PAPAON！！金錢不足唷！"},
	{ "msg_event",		"PAON！！你無法加入唷！"},
	{ "msg_start",		"哇喔~(出發進行)"},
	{ "msg_end",		"哇喔~(到囉)"}
#ifdef _NPC_AIRDELITEM
	,{ "msg_delitem",  "你沒有搭乘的道具"}
#endif
#ifdef _NPC_AIRLEVEL
    ,{ "msg_maxlevel",  "你的等級過高哦"}
#endif
};

static int NPC_AirSetPoint( int meindex, char *argstr);
static void NPC_AirSetDestPoint( int meindex, char *argstr);
static BOOL NPC_AirCheckDeniedItem( int meindex, int charaindex, char *argstr);
static BOOL NPC_AirCheckLevel( int meindex, int charaindex, char *argstr);
static int NPC_AirCheckStone( int meindex, int charaindex, char *argstr);
static void NPC_AirSendMsg( int meindex, int talkerindex, int tablenum);
static int NPC_AirGetRoutePointNum( int meindex, char *argstr );
static void NPC_Air_walk( int meindex);
#ifdef _NPC_AIRLEVEL
static BOOL NPC_AirCheckMaxLevel( int meindex, int charaindex, char *argstr);
#endif

#define		NPC_AIR_LOOPTIME		100
#define		NPC_AIR_WAITTIME_DEFAULT	180
#define		NPC_AIR_WAITINGMODE_WAITTIME	5000

/*********************************
* 初期処理
*********************************/
BOOL NPC_AirInit( int meindex )
{
	char	argstr[NPC_UTIL_GETARGSTR_BUFSIZE];
	int	i;
	char	buf[256],buf1[256];
	int	routenum;
	int	waittime;
	int	seflg;
	int	onewayflg;
	
	NPC_Util_GetArgStr( meindex, argstr, sizeof( argstr));
	
	/* なければいけない引数のチェック */
	routenum = NPC_Util_GetNumFromStrWithDelim( argstr, "routenum");
	if( routenum == -1 ) {
		print( "npcair:nothing routenum \n");
		return FALSE;
	}
	CHAR_setWorkInt( meindex, NPC_WORK_ROUTEMAX, routenum);
	
	for( i = 1; i <= routenum; i ++ ) {
		char routetostring[64];
		snprintf( routetostring, sizeof( routetostring), "routeto%d", i);
		if( NPC_Util_GetStrFromStrWithDelim( argstr, routetostring,buf, sizeof(buf))
			== NULL ) 
		{
			print( "npcair:nothing route to \n");
			return FALSE;
		}
	}
	//ANDY_ADD	NPC_WORK_RUNWAVE
	if( NPC_Util_GetStrFromStrWithDelim( argstr, "WAVE",buf1, sizeof(buf1)) == NULL )	{
		CHAR_setWorkInt( meindex, NPC_WORK_RUNWAVE, 77);
	}else	{
		CHAR_setWorkInt( meindex, NPC_WORK_RUNWAVE, atoi( buf1) );
	}

	waittime = NPC_Util_GetNumFromStrWithDelim( argstr, "waittime");
	if( waittime == -1 ) waittime = NPC_AIR_WAITTIME_DEFAULT;
	CHAR_setWorkInt( meindex, NPC_WORK_WAITTIME, waittime);

	seflg = NPC_Util_GetNumFromStrWithDelim( argstr, "seflg");
	if( seflg == -1 ) seflg = TRUE;
	CHAR_setWorkInt( meindex, NPC_WORK_SEFLG, seflg);
	
	onewayflg = NPC_Util_GetNumFromStrWithDelim( argstr, "oneway");
	if( onewayflg == -1 ) onewayflg = FALSE;	// default
	CHAR_setWorkInt( meindex, NPC_WORK_ONEWAYFLG, onewayflg);
	
	CHAR_setInt( meindex , CHAR_WHICHTYPE , CHAR_TYPEBUS );
	
	CHAR_setWorkInt( meindex, NPC_WORK_MODE, 0);
	CHAR_setWorkInt( meindex, NPC_WORK_ROUTEPOINT, 2);
	CHAR_setWorkInt( meindex, NPC_WORK_ROUNDTRIP, 0);
	CHAR_setWorkInt( meindex, NPC_WORK_CURRENTROUTE, 0);
			
	CHAR_setInt( meindex, CHAR_LOOPINTERVAL, 
		NPC_AIR_WAITINGMODE_WAITTIME);
    
    /* 現在の時間をセット */
    CHAR_setWorkInt( meindex, NPC_WORK_CURRENTTIME, NowTime.tv_sec);

    for( i = 0; i < CHAR_PARTYMAX; i ++) {
    	CHAR_setWorkInt( meindex, CHAR_WORKPARTYINDEX1 + i, -1);
    }
	
	/* ルート決定する */
{
	int rev;
	int r = CHAR_getWorkInt( meindex, NPC_WORK_ROUTEMAX);
	CHAR_setWorkInt( meindex, NPC_WORK_CURRENTROUTE, RAND( 1, r));
	//print( "route:%d\n",CHAR_getWorkInt( meindex, NPC_WORK_CURRENTROUTE));

	/* 後ろスタート */
	rev = NPC_Util_GetNumFromStrWithDelim( argstr, "reverse");
	if( rev == 1 ) {
		int num = NPC_AirGetRoutePointNum( meindex, argstr);
		if( num <= 0 ) {
			print( "npcairplane:真奇怪！\n");
			return FALSE;
		}
		CHAR_setWorkInt( meindex, NPC_WORK_ROUTEPOINT, num-1);
		CHAR_setWorkInt( meindex, NPC_WORK_ROUNDTRIP, 1);
	}
	/* ルートをセットする */
	NPC_AirSetPoint( meindex, argstr);
	/* 行き先を表示する */
	NPC_AirSetDestPoint( meindex, argstr);
}

    return TRUE;
}


/*********************************
* 話しかけられた時の処理
*********************************/
void NPC_AirTalked( int meindex , int talkerindex , char *szMes ,
                     int color )
{
    int i;
    int	partyflg = FALSE;
	int npc_wave = CHAR_getWorkInt( meindex, NPC_WORK_RUNWAVE);
	
    /* プレイヤーに対してだけ反応する */
    if( CHAR_getInt( talkerindex , CHAR_WHICHTYPE ) != CHAR_TYPEPLAYER ) {
    	return;
    }
    /* 自分のパーティ（乗客）かどうか調べる */
    for( i = 0; i < CHAR_PARTYMAX; i ++ ) {
	int index = CHAR_getWorkInt( meindex, CHAR_WORKPARTYINDEX1+i);
	if( CHAR_CHECKINDEX(index)){
		if( index == talkerindex) {
			partyflg = TRUE;
		}
	}
    }
	if( !partyflg ) {
		//NPC_AirCheckJoinParty( meindex, talkerindex, TRUE);
	}
	else {
		if( CHAR_getWorkInt( meindex, NPC_WORK_MODE) == 0 ) {
			int i;
	//		#define NPC_AIR_DEBUGROUTINTG	"routingtable:"
			if( strstr( szMes, "出發" )  ||
				strstr( szMes, "出發" )  ||
				strstr( szMes, "Go" )  ||
				strstr( szMes, "go" ))
			{
				CHAR_setWorkInt( meindex, NPC_WORK_MODE,1);
				
				/* ループ関数の呼出しを歩く速度にする */
	 			CHAR_setInt( meindex, CHAR_LOOPINTERVAL, NPC_AIR_LOOPTIME);
				
				/* SE 鳴らす（マンモスの叫び） */
				if( CHAR_getWorkInt( meindex, NPC_WORK_SEFLG )) {
					//andy_reEdit	NPC_WORK_RUNWAVE
					CHAR_sendSEoArroundCharacter( 
									CHAR_getInt( meindex, CHAR_FLOOR),
									CHAR_getInt( meindex, CHAR_X),
									CHAR_getInt( meindex, CHAR_Y),
									npc_wave,
									TRUE);
				}
				/* 出発する時のメッセージ*/
				for( i = 1; i < CHAR_PARTYMAX; i ++ ) {
					int partyindex = CHAR_getWorkInt( meindex, CHAR_WORKPARTYINDEX1+i);
					if( CHAR_CHECKINDEX( partyindex)) {
						NPC_AirSendMsg( meindex, partyindex, NPC_AIR_MSG_START);
					}
				}
			}
		}
#if 0
		else if( strstr( szMes, "停止" )  ||
			strstr( szMes, "停止" )  ||
			strstr( szMes, "stop" )  ||
			strstr( szMes, "Stop" ))
		{
			CHAR_setWorkInt( meindex, NPC_WORK_MODE,2);

			/* ループ関数のインターバルを多くする  */
			CHAR_setInt( meindex, CHAR_LOOPINTERVAL, 
						NPC_AIR_WAITINGMODE_WAITTIME);
		    /* 現在の時間をセット */
		    CHAR_setWorkInt( meindex, NPC_WORK_CURRENTTIME, NowTime.tv_sec);
		}
		else if( strstr( szMes, NPC_AIR_DEBUGROUTINTG )) {
			/* デバッグ用 */
			char *p = strstr( szMes,NPC_AIR_DEBUGROUTINTG);
			char	argstr[NPC_UTIL_GETARGSTR_BUFSIZE];

			NPC_Util_GetArgStr( meindex, argstr, sizeof( argstr));
			if( p) {
				int a = atoi( p+strlen(NPC_AIR_DEBUGROUTINTG));
				if( a <0 ) a = 1;
				CHAR_setWorkInt( meindex, NPC_WORK_CURRENTROUTE, a);
			}
			//print( "route:%d\n",CHAR_getWorkInt( meindex, NPC_WORK_CURRENTROUTE));
			/* ルートをセットする */
			NPC_AirSetPoint( meindex, argstr);
		}
#endif
	}
}
/**************************************
 * ループ関数
 **************************************/
void NPC_AirLoop( int meindex)
{
	int	i;
	int npc_wave = CHAR_getWorkInt( meindex, NPC_WORK_RUNWAVE);
	switch( CHAR_getWorkInt( meindex, NPC_WORK_MODE )) {
	  case 0:
	    /* 待ちモードの時，時間をチェックする */
		/* 時間が経ったので，出発する */
		if( CHAR_getWorkInt( meindex, NPC_WORK_CURRENTTIME) 
			+ CHAR_getWorkInt( meindex, NPC_WORK_WAITTIME) 
			< NowTime.tv_sec)
		{
			/* SE 鳴らす（マンモスの叫び） */
			if( CHAR_getWorkInt( meindex, NPC_WORK_SEFLG )) {
				//ANDY_reEdit
				CHAR_sendSEoArroundCharacter( 
								CHAR_getInt( meindex, CHAR_FLOOR),
								CHAR_getInt( meindex, CHAR_X),
								CHAR_getInt( meindex, CHAR_Y),
								npc_wave,
								TRUE);
			}
			/* 出発する時のメッセージ*/
			for( i = 1; i < CHAR_PARTYMAX; i ++ ) {
				int partyindex = CHAR_getWorkInt( meindex, CHAR_WORKPARTYINDEX1+i);
				if( CHAR_CHECKINDEX( partyindex)) {
					NPC_AirSendMsg( meindex, partyindex, NPC_AIR_MSG_START);
				}
			}
			
			CHAR_setWorkInt( meindex, NPC_WORK_MODE,1);
			/* ループ関数の呼出しを歩く速度にする */
			CHAR_setInt( meindex, CHAR_LOOPINTERVAL, NPC_AIR_LOOPTIME);
		}
		return;
	  case 1:
	  	/* 歩く */
	  	NPC_Air_walk( meindex);
	  case 2:
		/* 止まっているモード */
		/* 時間が経ったので，出発する */
		if( CHAR_getWorkInt( meindex, NPC_WORK_CURRENTTIME) 
			+ (CHAR_getWorkInt( meindex, NPC_WORK_WAITTIME) /3)
			< NowTime.tv_sec)
		{
			CHAR_setWorkInt( meindex, NPC_WORK_MODE,1);
			/* ループ関数の呼出しを歩く速度にする */
			CHAR_setInt( meindex, CHAR_LOOPINTERVAL, NPC_AIR_LOOPTIME);
		
		}
		return;
	  case 3:
		/* 到着しても，クライアントの描写待ちの為に，
		 * 少しここでウェイトをいれてやる
		 */
		if( CHAR_getWorkInt( meindex, NPC_WORK_CURRENTTIME) + 3	< NowTime.tv_sec){
			char	argstr[NPC_UTIL_GETARGSTR_BUFSIZE];
			NPC_Util_GetArgStr( meindex, argstr, sizeof( argstr));
			CHAR_setInt( meindex, CHAR_LOOPINTERVAL, NPC_AIR_WAITINGMODE_WAITTIME);
			{
				int r = CHAR_getWorkInt( meindex, NPC_WORK_ROUTEMAX);
				CHAR_setWorkInt( meindex, NPC_WORK_CURRENTROUTE, RAND( 1, r));
			}
			CHAR_setWorkInt( meindex, NPC_WORK_ROUNDTRIP, 
				CHAR_getWorkInt( meindex, NPC_WORK_ROUNDTRIP)^1);
			if( CHAR_getWorkInt( meindex, NPC_WORK_ROUNDTRIP) == 1)  {
				int num = NPC_AirGetRoutePointNum( meindex, argstr);
				CHAR_setWorkInt( meindex, NPC_WORK_ROUTEPOINT, num-1);
			}else {
				CHAR_setWorkInt( meindex, NPC_WORK_ROUTEPOINT, 
					CHAR_getWorkInt( meindex, NPC_WORK_ROUTEPOINT) +1);
			}
			NPC_AirSetPoint( meindex, argstr);
			NPC_AirSetDestPoint( meindex, argstr);
			CHAR_DischargeParty( meindex, 0);
		    CHAR_setWorkInt( meindex, NPC_WORK_CURRENTTIME, NowTime.tv_sec);
			if ((CHAR_getWorkInt(meindex, NPC_WORK_ONEWAYFLG) == 1) &&
			    (CHAR_getWorkInt(meindex, NPC_WORK_ROUNDTRIP) == 1) ){
			  CHAR_setInt( meindex, CHAR_LOOPINTERVAL, NPC_AIR_LOOPTIME);
			  CHAR_setWorkInt( meindex, NPC_WORK_MODE, 1);
			} else
			  CHAR_setWorkInt( meindex, NPC_WORK_MODE, 0);
		}
		return;
	  default:
	    break;
	}
}
/**************************************
 * 歩く。
 **************************************/
static void NPC_Air_walk( int meindex)
{
	POINT	start, end;
	int dir;
	int ret;
	int i;
	int npc_wave = CHAR_getWorkInt( meindex, NPC_WORK_RUNWAVE );

	/* 歩く関係 */
	/* 到着した時の処理 */
	start.x = CHAR_getInt( meindex, CHAR_X);
	start.y = CHAR_getInt( meindex, CHAR_Y);
	end.x = CHAR_getWorkInt( meindex, NPC_WORK_ROUTETOX);
	end.y = CHAR_getWorkInt( meindex, NPC_WORK_ROUTETOY);

	/* 到着したので次のポイントに */
	if( start.x == end.x && start.y == end.y ) {
		int add = 1;
		char	argstr[NPC_UTIL_GETARGSTR_BUFSIZE];

		NPC_Util_GetArgStr( meindex, argstr, sizeof( argstr));

		if( CHAR_getWorkInt( meindex, NPC_WORK_ROUNDTRIP ) == 1 ) {
			add *= -1;
		}
		CHAR_setWorkInt( meindex, NPC_WORK_ROUTEPOINT, 
			CHAR_getWorkInt( meindex, NPC_WORK_ROUTEPOINT) +add);
		if( NPC_AirSetPoint( meindex, argstr) == FALSE ) {
			/* 最後に到着*/
			/* 待ちモードにする */
			CHAR_setWorkInt( meindex, NPC_WORK_MODE,3);
			
			/* SE 鳴らす（マンモスの叫び） */
			if( CHAR_getWorkInt( meindex, NPC_WORK_SEFLG )) {
				//ANDY_reEdit
				CHAR_sendSEoArroundCharacter( 
					CHAR_getInt( meindex, CHAR_FLOOR),
					CHAR_getInt( meindex, CHAR_X),
					CHAR_getInt( meindex, CHAR_Y),
					npc_wave,
					TRUE);
			}
			/* 着いた時のメッセージ*/
			for( i = 1; i < CHAR_PARTYMAX; i ++ ) {
				int partyindex = CHAR_getWorkInt( meindex, CHAR_WORKPARTYINDEX1+i);
				if( CHAR_CHECKINDEX( partyindex)) {
					NPC_AirSendMsg( meindex, partyindex, NPC_AIR_MSG_END);
				}
			}
			/* 現在の時間をセット */
			CHAR_setWorkInt( meindex, NPC_WORK_CURRENTTIME, NowTime.tv_sec);
			return;
		}
		else {
			return;
		}
	}
	/*-------------------------------------------------------*/
	/* 歩かせる処理 */
	
	/* 方向を求める */
	dir = NPC_Util_getDirFromTwoPoint( &start,&end );

	/* 今いる場所の待避（パーティ歩きで使う） */
	end.x = CHAR_getInt( meindex, CHAR_X);
	end.y = CHAR_getInt( meindex, CHAR_Y);

#if 0
	/* ひっかかった時の為の処理 */
	for( i = 0; i < 100; i ++ ) {	
		if( dir < 0 ) {
			dir = RAND( 0,7);
		}	
		dir = NPC_Util_SuberiWalk( meindex, dir);
		if( dir >= 0 && dir <= 7) break;
	}
#endif
	
	if( dir >= 0 && dir <= 7 ) {
		/* 歩く */
		ret = CHAR_walk( meindex, dir, 0);

		if( ret == CHAR_WALKSUCCESSED ) {
			/* 自分が親なら仲間を歩かせる */
			int	i;
			int	mefl=CHAR_getInt( meindex, CHAR_FLOOR);
			for( i = 1; i < CHAR_PARTYMAX; i ++ ) {
				int toindex = CHAR_getWorkInt( meindex, i + CHAR_WORKPARTYINDEX1);
				int fl = CHAR_getInt( toindex, CHAR_FLOOR);
				int xx = CHAR_getInt( toindex, CHAR_X);
				int yy = CHAR_getInt( toindex, CHAR_Y);
				if( CHAR_CHECKINDEX(toindex) &&
				    (mefl==fl) && (abs(xx-end.x)+abs(yy-end.y)<10) ) {
					int	parent_dir;
					/* 子の位置と，親の歩き前の位置から方向を求める */
					/* 歩く */
					start.x = xx;
					start.y = yy;
					parent_dir = NPC_Util_getDirFromTwoPoint( &start,&end );
					/* グラディウスオプション歩きを実現する為に，
					 * 次の子は前の子の後を追うようにする
					 */
					end = start;
					if( parent_dir != -1 ) {
						CHAR_walk( toindex, parent_dir, 0);
					}
				}
			}
		}
	}
}
/**************************************
 * 次の場所をセットする
 **************************************/
static int NPC_AirSetPoint( int meindex, char *argstr)
{
	char	buf[4096];
	char	buf2[256];
	char	buf3[256];
	int floor,warpx,warpy;
	int ret;
	char routetostring[64];
	
	snprintf( routetostring, sizeof( routetostring), "routeto%d", 
				CHAR_getWorkInt( meindex, NPC_WORK_CURRENTROUTE));
	
	if( NPC_Util_GetStrFromStrWithDelim( argstr, routetostring,buf, sizeof(buf))
		== NULL ) 
	{
		print( "npcair:nothing route \n");
		return FALSE;
	}

	ret = getStringFromIndexWithDelim( buf, ";", 
		CHAR_getWorkInt( meindex, NPC_WORK_ROUTEPOINT),
		buf2, sizeof(buf2));
	if( ret == FALSE ) return FALSE;

	// Arminius: add floor
	
	ret = getStringFromIndexWithDelim( buf2, ",", 1,
		buf3, sizeof(buf3));
	if( ret == FALSE) return FALSE;
	floor = atoi(buf3);

	ret = getStringFromIndexWithDelim( buf2, ",", 2,
		buf3, sizeof(buf3));
	if( ret == FALSE) return FALSE;
	CHAR_setWorkInt( meindex, NPC_WORK_ROUTETOX, atoi( buf3));
	warpx = atoi(buf3);
	
	ret = getStringFromIndexWithDelim( buf2, ",", 3,
		buf3, sizeof(buf3));
	if( ret == FALSE) return FALSE;
	CHAR_setWorkInt( meindex, NPC_WORK_ROUTETOY, atoi( buf3));
	warpy = atoi(buf3);

	if (floor!=CHAR_getInt(meindex, CHAR_FLOOR)) {
		int	i;
		CHAR_warpToSpecificPoint(meindex, floor, warpx, warpy);

		for( i = 1; i < CHAR_PARTYMAX; i ++ ) {
			int toindex = CHAR_getWorkInt( meindex, i + CHAR_WORKPARTYINDEX1);
			if( CHAR_CHECKINDEX(toindex) ) {
				CHAR_warpToSpecificPoint(toindex, floor, warpx, warpy);
			}
		}
		CHAR_setWorkInt( meindex, NPC_WORK_ROUTETOX, warpx);
		CHAR_setWorkInt( meindex, NPC_WORK_ROUTETOY, warpy);
	}

	return TRUE;
}
/**************************************
 * route番号から，名前があったらそれを
 * 称号のとこにセットする。
 **************************************/
static void NPC_AirSetDestPoint( int meindex, char *argstr)
{
	char 	buf[256];
	char	routename[256];

	snprintf( routename, sizeof( routename), "routename%d", 
				CHAR_getWorkInt( meindex, NPC_WORK_CURRENTROUTE));

	if( NPC_Util_GetStrFromStrWithDelim( argstr, routename, buf, sizeof( buf))
		!= NULL ) 
	{
		CHAR_setChar( meindex, CHAR_OWNTITLE, buf);
		CHAR_sendCToArroundCharacter( CHAR_getWorkInt( meindex, CHAR_WORKOBJINDEX));
	}
}
/**************************************
 * 指定されたアイテムを持っているかチェックする
 * 持っていたらだめ
 **************************************/
static BOOL NPC_AirCheckDeniedItem( int meindex, int charaindex, char *argstr)
{
	char	buf[1024];
	BOOL	found = TRUE;

	if( NPC_Util_GetStrFromStrWithDelim( argstr, "denieditem", buf, sizeof( buf))
		!= NULL ) 
	{
		int	i;
		int ret;
		for( i = 1; ; i ++) {
			int itemid;
			char buf2[64];
			int j;
			ret = getStringFromIndexWithDelim( buf, ",", i, buf2, sizeof(buf2));
			if( ret == FALSE ) break;
			itemid = atoi( buf2);
			for( j = 0; j < CHAR_MAXITEMHAVE; j ++) {
				int itemindex = CHAR_getItemIndex( charaindex, j);
				if( ITEM_CHECKINDEX( itemindex)) {
					if( ITEM_getInt( itemindex, ITEM_ID) == itemid) {
						found = FALSE;
						break;
					}
				}
			}
		}
	}
	return found;
}
/**************************************
 * 指定されたアイテムを持っているかチェックする
 * 持っていないとだめ
 **************************************/
BOOL NPC_AirCheckAllowItem( int meindex, int charaindex, BOOL pickupmode)
{
	char	buf[1024];
	BOOL	found = TRUE;
	BOOL	pickup = FALSE;
	char	argstr[NPC_UTIL_GETARGSTR_BUFSIZE];
	
	NPC_Util_GetArgStr( meindex, argstr, sizeof( argstr));
	
	if( NPC_Util_GetStrFromStrWithDelim( argstr, "pickupitem", buf, sizeof( buf))
		!= NULL ) 
	{
		pickup = TRUE;
	}
	if( NPC_Util_GetStrFromStrWithDelim( argstr, "allowitem", buf, sizeof( buf))
		!= NULL ) 
	{
		int	i;
		int ret;
		for( i = 1; ; i ++) {
			int itemid;
			char buf2[64];
			int j;
			BOOL	getflg;
			ret = getStringFromIndexWithDelim( buf, ",", i, buf2, sizeof(buf2));
			if( ret == FALSE ) break;
			itemid = atoi( buf2);
			getflg = FALSE;
			for( j = 0; j < CHAR_MAXITEMHAVE; j ++) {
				int itemindex = CHAR_getItemIndex( charaindex, j);
				if( ITEM_CHECKINDEX( itemindex)) {
					if( ITEM_getInt( itemindex, ITEM_ID) == itemid) {
						/* 条件が揃っているから，そのアイテムを取る */
						if( pickupmode && pickup && !getflg) {
							CHAR_DelItem( charaindex, j);
							getflg = TRUE;
						}
						break;
					}
				}
			}
			if( j == CHAR_MAXITEMHAVE) {
				found = FALSE;
				break;
			}
		}
	}
	return found;
}

/**************************************
 * 指定されたレベル以上かチェックする
 **************************************/
static BOOL NPC_AirCheckLevel( int meindex, int charaindex, char *argstr)
{
	int		level;
	
	/* なければいけない引数のチェック */
	level = NPC_Util_GetNumFromStrWithDelim( argstr, "needlevel");
	if( level == -1 ) {
		return TRUE;
	}
	if( CHAR_getInt( charaindex, CHAR_LV) >= level ) return TRUE;
	
	return FALSE;
}

#ifdef _NPC_AIRLEVEL
static BOOL NPC_AirCheckMaxLevel( int meindex, int charaindex, char *argstr)
{
	int		level;
	
	/* なければいけない引数のチェック */
	level = NPC_Util_GetNumFromStrWithDelim( argstr, "maxlevel");
	if( level == -1 ) {
		return TRUE;
	}
	if( CHAR_getInt( charaindex, CHAR_LV) < level ) return TRUE;
	
	return FALSE;
}
#endif

/**************************************
 * 御金をチェックする
 * -1 駄目 0以上；ＯＫ，かつ必要Stone
 **************************************/
static int NPC_AirCheckStone( int meindex, int charaindex, char *argstr)
{
	int		gold;
	
	/* なければいけない引数のチェック */
	gold = NPC_Util_GetNumFromStrWithDelim( argstr, "needstone");
	if( gold == -1 ) {
		return 0;
	}
	if( CHAR_getInt( charaindex, CHAR_GOLD) >= gold ) return gold;
	
	return -1;
}
/**************************************
 * メッセージを送る
 * 引数のメッセージがなければデフォルトメッセージを送る
 **************************************/
static void NPC_AirSendMsg( int meindex, int talkerindex, int tablenum)
{
	char	argstr[NPC_UTIL_GETARGSTR_BUFSIZE];
	char	buf[256];
	char	msg[256];
	if( tablenum < 0 || tablenum >= arraysizeof( airmsg)) return;
	
	NPC_Util_GetArgStr( meindex, argstr, sizeof( argstr));
	
	if( NPC_Util_GetStrFromStrWithDelim( argstr, airmsg[tablenum].option, buf, sizeof( buf))
		!= NULL ) 
	{
		strcpy( msg, buf);
	}
	else {
		snprintf( msg, sizeof(msg),airmsg[tablenum].defaultmsg);
	}
	CHAR_talkToCli( talkerindex, meindex, msg, CHAR_COLORYELLOW);
}
/**************************************
 * ルートテーブルのポイントの数を取得する
 **************************************/
static int NPC_AirGetRoutePointNum( int meindex, char *argstr )
{
	int		i;
	char	buf[4096];
	char	buf2[256];
	int ret;
	char routetostring[64];
	
	snprintf( routetostring, sizeof( routetostring), "routeto%d", 
				CHAR_getWorkInt( meindex, NPC_WORK_CURRENTROUTE));
	
	if( NPC_Util_GetStrFromStrWithDelim( argstr, routetostring,buf, sizeof(buf))
		== NULL ) 
	{
		print( "npcair:nothing route \n");
		return -1;
	}
	for( i = 1; ; i ++ ) {
		ret = getStringFromIndexWithDelim( buf, ";", i, buf2, sizeof(buf2));
		if( ret == FALSE) break;
	}
	return( i -1);
}
BOOL NPC_AirCheckJoinParty( int meindex, int charaindex, BOOL msgflg)
{
    //int		fd;
	char	argstr[NPC_UTIL_GETARGSTR_BUFSIZE];
	int		ret;
	NPC_Util_GetArgStr( meindex, argstr, sizeof( argstr));
	
	/* １グリッド以内のみ */
	if( !NPC_Util_charIsInFrontOfChar( charaindex, meindex, 1 )) return FALSE; 
	/* 途中乗車は拒否する */
	if( CHAR_getWorkInt( meindex, NPC_WORK_MODE) != 0 ) {
		if( msgflg) NPC_AirSendMsg( meindex, charaindex, NPC_AIR_MSG_GETTINGON);
		return FALSE;
	}
	/* ぱーてぃだったらだめ */
	if( CHAR_getWorkInt( charaindex, CHAR_WORKPARTYMODE ) != CHAR_PARTY_NONE) {
		if( msgflg) NPC_AirSendMsg( meindex, charaindex, NPC_AIR_MSG_NOTPARTY);
		return FALSE;
	}
	/* パーティの人数をチェックする */
	if( CHAR_getEmptyPartyArray( meindex) == -1 ) {
		if( msgflg) NPC_AirSendMsg( meindex, charaindex, NPC_AIR_MSG_OVERPARTY);
		return FALSE;
	}
	/* アイテムのチェックをする(禁止アイテム) */
	if( !NPC_AirCheckDeniedItem( meindex, charaindex, argstr)) {
		if( msgflg) NPC_AirSendMsg( meindex, charaindex, NPC_AIR_MSG_DENIEDITEM);
		return FALSE;
	}
#ifdef _ITEM_CHECKWARES
	if( CHAR_CheckInItemForWares( charaindex, 0) == FALSE )	{
		CHAR_talkToCli( charaindex, -1, "無法攜帶貨物上機。", CHAR_COLORYELLOW);
		return FALSE;
	}
#endif

	/* アイテムのチェックをする(必要アイテム) */
	if( !NPC_AirCheckAllowItem( meindex, charaindex, FALSE)) {
		if( msgflg) NPC_AirSendMsg( meindex, charaindex, NPC_AIR_MSG_ALLOWITEM);
		return FALSE;
	}
#ifdef _NPC_AIRDELITEM
	if( !NPC_AirCheckDelItem( meindex, charaindex, FALSE) ){ //若是沒扣除了道具
		if( msgflg ) NPC_AirSendMsg( meindex, charaindex, NPC_AIR_MSG_DELITEM);
	    return FALSE;
	}
#endif
	/* レベルのチェックをする */
	if( !NPC_AirCheckLevel( meindex, charaindex, argstr)) {
		if( msgflg) NPC_AirSendMsg( meindex, charaindex, NPC_AIR_MSG_LEVEL);
		return FALSE;
	}
#ifdef _NPC_AIRLEVEL
    if( !NPC_AirCheckMaxLevel( meindex, charaindex, argstr)) {
		if( msgflg) NPC_AirSendMsg( meindex, charaindex, NPC_AIR_MSG_MAXLEVEL);
		return FALSE;
	}
#endif
	/* イベント中かチェックする */
//	if( CHAR_getInt( charaindex, CHAR_NOWEVENT) != 0 ||
//		CHAR_getInt( charaindex, CHAR_NOWEVENT2) != 0 ||
//		CHAR_getInt( charaindex, CHAR_NOWEVENT3) != 0 )
//	{
//		if( msgflg) NPC_AirSendMsg( meindex, charaindex, NPC_AIR_MSG_EVENT);
//		return FALSE;
//	}
	/* 御金のチェックをする（お金を取るので，最終チェックにすること！） */
	ret = NPC_AirCheckStone( meindex, charaindex, argstr);
	if( ret == -1 ) {
		if( msgflg) NPC_AirSendMsg( meindex, charaindex, NPC_AIR_MSG_GOLD);
		return FALSE;
	}
	if( ret != 0 ) {
		char msgbuf[128];
		/* 御金をとる */
		CHAR_setInt( charaindex, CHAR_GOLD, 
					CHAR_getInt( charaindex, CHAR_GOLD) - ret);
		/* 送信 */
		CHAR_send_P_StatusString( charaindex, CHAR_P_STRING_GOLD);
		snprintf( msgbuf, sizeof( msgbuf), "支付了%d Stone！", ret);
		CHAR_talkToCli( charaindex, -1, msgbuf, CHAR_COLORYELLOW);
	}
	/* パーティに入る */
	//CHAR_JoinParty_Main( charaindex, meindex);
	
	//fd = getfdFromCharaIndex( charaindex );
	
	//lssproto_PR_send( fd, 1, 1);
	
	return TRUE;
}

#ifdef _NPC_AIRDELITEM //上飛機時,檢查是否要扣除道具
BOOL NPC_AirCheckDelItem( int meindex, int charaindex, BOOL pickupmode)
{
	char	buf[1024];
	BOOL	found = TRUE;
	BOOL	pickup = FALSE;
	char	argstr[NPC_UTIL_GETARGSTR_BUFSIZE];
	
	NPC_Util_GetArgStr( meindex, argstr, sizeof( argstr));
	if( NPC_Util_GetStrFromStrWithDelim( argstr, "delitem", buf, sizeof( buf))
		!= NULL ) 
	{
		int	i;
		int ret;
		for( i = 1; ; i ++) {
			int itemid;
			char buf2[64];
			int j;
			ret = getStringFromIndexWithDelim( buf, ",", i, buf2, sizeof(buf2));
			if( ret == FALSE ) break;
			itemid = atoi( buf2);
			for( j = 0; j < CHAR_MAXITEMHAVE; j ++) {
				int itemindex = CHAR_getItemIndex( charaindex, j);
				if( ITEM_CHECKINDEX( itemindex)) {
					if( ITEM_getInt( itemindex, ITEM_ID) == itemid) {
						CHAR_DelItem( charaindex, j);
						break;
					}
				}
			}
			if( j == CHAR_MAXITEMHAVE) {
				found = FALSE;
				break;
			}
		}
	}
	return found;
}
#endif
