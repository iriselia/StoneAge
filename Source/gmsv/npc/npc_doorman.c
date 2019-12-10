#include "version.h"
#include <string.h>
#include "object.h"
#include "char_base.h"
#include "char.h"
#include "item.h"
#include "util.h"
#include "handletime.h"
#include "npc_doorman.h"
#include "npc_door.h"
#include "npcutil.h"
#include "configfile.h"
/*
 *
 *  隣りにドアがあるときにそのドアを何らかの条件によって開くNPC.
 *  ドアのとなりにcreateするだけで、そのドアを操作させることができる。
 *  はなしかけられたときに全キャラを検索して、まわり8マスにドアがいる
 *  場合はそのすべてに対して影響する。ふたつのドアが同時に開くことになる。
 *
 *  インターフェイスはTalkで
 *
 * ドアを開くために
 *
 * 1 お金を徴収する。徴収できたらひらく     gold|100
 * 2 アイテムを1個徴収する 。徴収できたらひらく  item|45
 * 3 アイテムを持っているかどうか調べる。 持っていたら開く。itemhave|44
 * 4 アイテムを持っていないかどうか調べる。持っていなかったら開く。
 *          itemnothave|333
 * 5 称号をもっているかどうか調べる。持っていたら開く。 titlehave|string
 * 6 称号をもっていないかどうか調べる。持っていなかったら開く。
 *      titlenothave|string
 *
 * かならず質問に答えると開く。金の場合は、
 *「100ゴールドいただきますがいいですか？」で「はい」というと100ゴールド
 * とられる。いきなり「はい」だけ言ってもとられる。で、「100ゴールド
 * いただきました。」と言われる。
 *
 * アイテム徴収の場合は、「何々を一個いただきますがいいですか？」ときく。
 *  3から6の場合は、何かはなしかけて条件がそろってたら開く。
 *
 *
 *
 *  テストの方法
 *
 *1  ドアをてきとうに置く
 *2  このNPCを適当にドアのとなりに置く。引数を gold|100 にする
 *3  このNPCに対して、100ゴールド以上もっている状態で「はい」と言う
 *4  ドアがひらいて金が減ったら成功。
 *
 */

static void NPC_DoormanOpenDoor( char *nm  );

BOOL NPC_DoormanInit( int meindex )
{
	char	arg[NPC_UTIL_GETARGSTR_BUFSIZE];
    char dname[1024];

	/* イベントのタイプ設定 */
	CHAR_setWorkInt( meindex, CHAR_WORKEVENTTYPE,CHAR_EVENT_NPC);

    CHAR_setInt( meindex , CHAR_HP , 0 );
    CHAR_setInt( meindex , CHAR_MP , 0 );
    CHAR_setInt( meindex , CHAR_MAXMP , 0 );
    CHAR_setInt( meindex , CHAR_STR , 0 );
    CHAR_setInt( meindex , CHAR_TOUGH, 0 );
    CHAR_setInt( meindex , CHAR_LV , 0 );

    CHAR_setInt( meindex , CHAR_WHICHTYPE , CHAR_TYPETOWNPEOPLE );
    CHAR_setFlg( meindex , CHAR_ISOVERED , 0 );
    CHAR_setFlg( meindex , CHAR_ISATTACKED , 0 );  /* 攻撃されないよん */

	NPC_Util_GetArgStr( meindex, arg, sizeof( arg));

    if(!getStringFromIndexWithDelim( arg, "|", 3, dname, sizeof(dname ))){
        print("RINGO: 設定看門者時需要門的名字唷！:%s:\n",
              arg );
        return FALSE;
    }
    print( "RINGO: Doorman create: arg: %s dname: %s\n",arg,dname);
    CHAR_setWorkChar( meindex , CHAR_WORKDOORMANDOORNAME , dname );

    return TRUE;
}

void NPC_DoormanTalked( int meindex , int talkerindex , char *msg ,
                     int color )
{
    char mode[128];
    char opt[256];
    char	arg[NPC_UTIL_GETARGSTR_BUFSIZE];

    /* プレイヤーがドアマンの1グリッド以内ならはんのう */
    if(NPC_Util_CharDistance( talkerindex, meindex ) > 1)return;

	NPC_Util_GetArgStr( meindex, arg, sizeof( arg));

    if( !getStringFromIndexWithDelim( arg, "|", 1, mode, sizeof( mode )))
        return;

    if( !getStringFromIndexWithDelim( arg, "|", 2, opt, sizeof( opt ) ))
        return;

    if( strcmp( mode , "gold" ) == 0 ){
        int g = atoi( opt );
        int yn = NPC_Util_YN( msg );
        /*char *nm = CHAR_getChar( meindex , CHAR_NAME );*/
        char msg[256];

        if( g > 0 && yn < 0 ){
            snprintf( msg ,sizeof( msg ) ,
                      "打開門需要給我%d的金子這樣可以嗎？", g );
            CHAR_talkToCli( talkerindex, meindex , msg, CHAR_COLORWHITE );
        } else if( g > 0 && yn == 0 ){
            snprintf( msg , sizeof( msg ),
                      "打開門 %d的金子是必要的。", g );
        } else if( g > 0 && yn == 1 ){
            int now_g = CHAR_getInt( talkerindex, CHAR_GOLD );
            if( now_g < g ){
                snprintf( msg , sizeof( msg ) ,
                          "打開門 %d的金子是必要的。", g );
            	CHAR_talkToCli( talkerindex, meindex , msg, CHAR_COLORWHITE );
            } else {
                snprintf( msg , sizeof( msg ),
                          "%d 收到金子了。現在就來開門。", g );
            	CHAR_talkToCli( talkerindex, meindex , msg, CHAR_COLORWHITE );

                /* お金をゲット */
                now_g -= g;
                CHAR_setInt( talkerindex , CHAR_GOLD , now_g );
                /* あたらしいステータスを送信 */
                CHAR_send_P_StatusString(talkerindex, CHAR_P_STRING_GOLD);

                /* ドアひらく */
                NPC_DoormanOpenDoor(
                    CHAR_getWorkChar( meindex, CHAR_WORKDOORMANDOORNAME));
            }
        }
    } else if( strcmp( mode , "item" ) == 0 ){
        CHAR_talkToCli( talkerindex, meindex ,
                        "尚在未支援模式。",
                        CHAR_COLORWHITE);
    } else if( strcmp( mode , "itemhave" ) == 0 ){
        CHAR_talkToCli( talkerindex, meindex ,
                        "尚在未支援模式。",
                        CHAR_COLORWHITE);
    } else if( strcmp( mode , "itemnothave" ) == 0 ){
        CHAR_talkToCli( talkerindex, meindex ,
                        "尚在未支援模式。",
                        CHAR_COLORWHITE);
    } else if( strcmp( mode , "titlehave" ) == 0 ){
        CHAR_talkToCli( talkerindex, meindex ,
                        "尚在未支援模式。",
                        CHAR_COLORWHITE);

    } else if( strcmp( mode , "roomlimit" ) == 0 ){

		/* 部屋の人数制限がある場合 */
		char szOk[256], szNg[256], szBuf[32];
		int checkfloor;
		int maxnum, i, iNum;

	    if( !getStringFromIndexWithDelim( arg, "|", 2, szBuf, sizeof( szBuf ) ))
    	    return;

		/* 調べるフロアと最大人数 */
		if( sscanf( szBuf, "%d:%d", &checkfloor, &maxnum ) != 2 ){
			return;
		}

		for( iNum = 0,i = 0; i < getFdnum(); i ++ ){
			/* プレイヤー以外には興味が無い */
			if( CHAR_getCharUse( i ) == FALSE )continue;
			if( CHAR_getInt( i, CHAR_WHICHTYPE ) != CHAR_TYPEPLAYER )continue;
			/* 指定のフロア以外に興味が無い */
			if( CHAR_getInt( i, CHAR_FLOOR ) != checkfloor )continue;
			iNum++;
		}
	    if( !getStringFromIndexWithDelim( arg, "|", 5, szNg, sizeof( szNg ))){
   			strcpy( szNg, "。。。。" );	/* 鳳傘卅仄及本伉白 */
		}
    	if( !getStringFromIndexWithDelim( arg, "|", 4, szOk, sizeof( szOk ))){
   			strcpy( szOk, "開門吧。。。" );	/* 鳳傘丐曰及本伉白 */
   		}

		if( iNum >= maxnum ){
			/* 最大を超えている場合 */
	        CHAR_talkToCli( talkerindex, meindex ,szNg, CHAR_COLORWHITE);
		}else{
			/* 最大に満たない場合 */
	        CHAR_talkToCli( talkerindex, meindex ,szOk, CHAR_COLORWHITE);
            NPC_DoormanOpenDoor(
                    CHAR_getWorkChar( meindex, CHAR_WORKDOORMANDOORNAME));
		}

    } else if( strcmp( mode , "titlenothave" ) == 0 ){
        CHAR_talkToCli( talkerindex, meindex ,
                        "尚在未支援模式。",
                        CHAR_COLORWHITE);
    }
}

/*
 *  名前で検索してヒットしたのをすべて開く。
 *
 */
static void NPC_DoormanOpenDoor( char *nm)
{
    int doori = NPC_DoorSearchByName( nm );
    print( "RINGO: Doorman's Door: index: %d\n", doori );

    NPC_DoorOpen( doori , -1 );

}

