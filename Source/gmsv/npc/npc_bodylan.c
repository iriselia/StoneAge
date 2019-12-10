#include "version.h"
#include <string.h>
#include "char.h"
#include "object.h"
#include "char_base.h"
#include "npcutil.h"
#include "lssproto_serv.h"
#include "npc_windowhealer.h"




/*
 *①　ボディランゲージを使えば反応するNPC
 * まず話しかけられたら、そのプレイヤーに
 * このNPCのINDEXを保存する。
 *
 *②　NPCの周りでアクションがあったら、アクションを起こしたプレイヤーが
 *NPCのINDEXを保持しているかチェック
 *チェックＯＫなら次はアクションのシーケンスチェックする
 *希望のアクションをしていたらプレイヤーのカウントをアップさせる。
 *カウントアップ後、全シーケンスＯＫなら指定の場所へワープさせる
 *
 */

enum {
	BODYLAN_E_COMMANDNUM = CHAR_NPCWORKINT1,	// コマンドの長さ
};


// ウインドウモード
enum{
	BODYLAN_WIN_FIRST,
	BODYLAN_WIN_LAST_GOOD,
	BODYLAN_WIN_LAST_NG,
	BODYLAN_WIN_GOOD_NO,
	BODYLAN_WIN_ALREADY,
	BODYLAN_WIN_NOT_PREEVENT,
	BODYLAN_WIN_END
};

static void NPC_BodyLan_Profit( int meindex, int playerindex );

static void NPC_BodyLan_Window(
	int meindex,
	int talkerindex,
	int mode
);


/*********************************
* 初期処理
*********************************/
BOOL NPC_BodyLanInit( int meindex )
{

	char szP[256], szArg[4096];
	char buf[256];
	int i, needSeq;

    CHAR_setInt( meindex , CHAR_WHICHTYPE , CHAR_TYPEEVENT );

    if( NPC_Util_GetArgStr( meindex, szArg, sizeof( szArg ) ) == NULL ){
    	print( "npc_bodylan.c:沒有引數(%s)\n",
    		CHAR_getChar(meindex,CHAR_NPCARGUMENT) );
    	return FALSE;
    }


	// シーケンスを探す
	if( NPC_Util_GetStrFromStrWithDelim( szArg, "Act", szP, sizeof( szP ) ) == NULL ){
		print( "npc_bodylan:動作文字列尚未設定(%s)\n",	szArg );
		return FALSE;
	}

	for( i = 0 ; ; i ++ ){
		// 必要な順番
		if( getStringFromIndexWithDelim( szP, ",", i, buf, sizeof( buf)) != FALSE ){
			needSeq = atoi(buf);
			// マイナスが来たらここまで
			if( needSeq < 0 ){
				if( i <= 0 ){
					print( "npc_bodylan:動作列尚未設定(%s)\n", szArg );
				}
				// 最大数をセット
				CHAR_setWorkInt( meindex, BODYLAN_E_COMMANDNUM, i );
				break;
			}else{
				// ループして数える
			}
		}else{
			if( i <= 0 ){
				print( "npc_bodylan:動作列尚未設定(%s)\n", szArg );
			}
			// 最大数をセット
			CHAR_setWorkInt( meindex, BODYLAN_E_COMMANDNUM, i );
			break;
		}
	}



    return TRUE;

}




/*********************************
* 話しかけられた時の処理
*********************************/
void NPC_BodyLanTalked( int meindex , int talkerindex , char *szMes ,int color )
{
	char szP[256], szArg[4096];
	int EventNo = -1,Pre_Event = -1;

    /* プレイヤーに対してだけ反応する */
    if( CHAR_getInt( talkerindex , CHAR_WHICHTYPE ) != CHAR_TYPEPLAYER ) {
    	return;
    }

	/* １グリッド以内のみ */
	if( NPC_Util_CharDistance( talkerindex, meindex ) > 2 )
	{
		return;
	}

	// 引数文字列
    if( NPC_Util_GetArgStr( meindex, szArg, sizeof( szArg ) ) == NULL ){
    	print( "npc_bodylan.c:沒有引數(%s)\n",
    		CHAR_getChar(meindex,CHAR_NPCARGUMENT) );
    	return ;
    }else{
		// イベント番号取得
		if( NPC_Util_GetStrFromStrWithDelim( szArg, "EventNo", szP, sizeof( szP ) ) != NULL ){
			EventNo = atoi(szP);
		}
		// 事前必要イベント番号取得
		if( NPC_Util_GetStrFromStrWithDelim( szArg, "Pre_Event", szP, sizeof( szP ) ) != NULL ){
			Pre_Event = atoi(szP);
		}
	}

	// 事前に必要イベント番号があれば
	if( Pre_Event >= 0 ){
		// イベントに対してどうしているかチェック
		if( NPC_EventCheckFlg( talkerindex, Pre_Event ) == FALSE ){
			// 必要イベントをクリアしていなかったらこのセリフ
			NPC_BodyLan_Window( meindex, talkerindex, BODYLAN_WIN_NOT_PREEVENT );
			return;
		}
	}
	// イベント番号があれば
	if( EventNo >= 0 ){
		// イベントに対してどうしているかチェック
		if( NPC_EventCheckFlg( talkerindex, EventNo ) == TRUE ){
			// 立っていたらこのセリフ
			NPC_BodyLan_Window( meindex, talkerindex, BODYLAN_WIN_ALREADY );
			return;
		}
	}

	// 前回このプレイヤーは自分と喋っていたか
	if( CHAR_getWorkInt( talkerindex, CHAR_WORKTRADER ) == meindex ){

		// さらにコマンドが全部成功していたら
		if( CHAR_getWorkInt( talkerindex, CHAR_WORKSHOPRELEVANT )
		 >= CHAR_getWorkInt( meindex, BODYLAN_E_COMMANDNUM )
		){
			// 成功！！ご褒美か？
			NPC_BodyLan_Window( meindex, talkerindex, BODYLAN_WIN_LAST_GOOD );
			return;
		}else{
			// 失敗
			NPC_BodyLan_Window( meindex, talkerindex, BODYLAN_WIN_LAST_NG );
			return;
		}
	}else{
		// 初めてなので覚える
		// プレイヤーに自分のインデックスを保存させる
		CHAR_setWorkInt( talkerindex, CHAR_WORKTRADER, meindex );
		// シーケンスは最初からに初期化
		CHAR_setWorkInt( talkerindex, CHAR_WORKSHOPRELEVANT, 1 );

		// その際セリフとか喋るべきかなあ・・・
		NPC_BodyLan_Window( meindex, talkerindex, BODYLAN_WIN_FIRST );
		return;
	}
}


/*=======================================
 * watch 処理
 *======================================*/
void NPC_BodyLanWatch(
	int objmeindex,
	int objmoveindex,
    CHAR_ACTION act,
    int x,
    int y,
    int dir,
    int* opt,
    int optlen
)
{
	char szP[256], szArg[4096];
	char buf[256];
	int actindex;
	int meindex;
	int seqNo, needSeq;

	// キャラクタ以外はリターン
	if( OBJECT_getType(objmoveindex) != OBJTYPE_CHARA ) return;
	actindex = OBJECT_getIndex(objmoveindex);
	// プレイヤー以外はリターン
	if( CHAR_getInt( actindex, CHAR_WHICHTYPE ) != CHAR_TYPEPLAYER ) return;

	// 自分のインデックス
	meindex = OBJECT_getIndex(objmeindex);

	// 自分のINDEXを保持しているか？いなければリターン
	if( CHAR_getWorkInt( actindex, CHAR_WORKTRADER ) != meindex ){
		return;
	}

	// どこまでアクションをしているか
	seqNo = CHAR_getWorkInt( actindex, CHAR_WORKSHOPRELEVANT );
	// 変な場合は最初から
	if( seqNo < 1 )seqNo = 1;

	// 引数文字列
    if( NPC_Util_GetArgStr( meindex, szArg, sizeof( szArg ) ) == NULL ){
    	print( "npc_bodylan.c:沒有引數(%s)\n",
    		CHAR_getChar(meindex,CHAR_NPCARGUMENT) );
    	return ;
    }


	// シーケンスを探す
	if( NPC_Util_GetStrFromStrWithDelim( szArg, "Act", szP, sizeof( szP ) ) == NULL ){
		print( "npc_bodylan:動作文字列尚未設定(%s)\n",	szArg );
		return;
	}

	// 必要な順番
	if(getStringFromIndexWithDelim( szP, ",", seqNo, buf, sizeof( buf)) != FALSE ){
		needSeq = atoi(buf);
	}else{
		// なぜかなかったら最初から
		CHAR_setWorkInt( actindex, CHAR_WORKSHOPRELEVANT, 1 );

//		print( "做過頭了。回到最初。\n", seqNo, needSeq );
		// 何か喋るべきか・・・
		return;
	}

	// 今回必要なアクションが一致した
	if( needSeq == act ){
		// 一致した。これが最後か
//		print( "成功\(%d次數是%d)\n", seqNo, needSeq );
		seqNo ++;
		if( seqNo >= CHAR_getWorkInt( meindex, BODYLAN_E_COMMANDNUM ) ){
//			print( "在此結束。\n" );
		}
		// この位置を保存
		CHAR_setWorkInt( actindex, CHAR_WORKSHOPRELEVANT, seqNo );
	}else{
		// 失敗した場合は最初からやり直し
		CHAR_setWorkInt( actindex, CHAR_WORKSHOPRELEVANT, 1 );
//		print( "(%d次數是%d)\n", seqNo, needSeq );
	}

}



//********* 成功時のご褒美 *********
static void NPC_BodyLan_Profit( int meindex, int playerindex )
{
	char szArg[4096], szP[256];
	int fl, x, y, pmode, i, subindex, parent;

	// 引数文字列
    if( NPC_Util_GetArgStr( meindex, szArg, sizeof( szArg ) ) == NULL ){
    	print( "npc_bodylan.c:沒有引數(%s)\n",
    		CHAR_getChar(meindex,CHAR_NPCARGUMENT) );
    	return ;
    }

	//*********************************************
	//
	//   ご褒美その１。ワープ設定
	//
	//*********************************************
	if( NPC_Util_GetStrFromStrWithDelim( szArg, "Warp", szP, sizeof( szP ) ) != NULL ){
		// ご褒美にワープ。座標取る
		if( sscanf( szP, "%d,%d,%d", &fl, &x, &y ) == 3 ){
		}else{
			print( "npc_bodylan: 無法讀取空間座標(%s)\n", szP );
			return;
		}
		// パーティ組んでる場合子供もワープ
		pmode = CHAR_getWorkInt( playerindex, CHAR_WORKPARTYMODE );
		switch( pmode ){
		case 1: // 自分が親
			parent = playerindex;
			break;
		case 2: // 自分が子供。親を取得
			parent = CHAR_getWorkInt( playerindex, CHAR_WORKPARTYINDEX1 );
			break;
		default:
			// パーティじゃなかった。自分だけワープ
			CHAR_warpToSpecificPoint( playerindex, fl, x, y );
			return;
		}
		// 全員ワープ
		for( i = 0; i < CHAR_PARTYMAX; i ++ ){
			subindex = CHAR_getWorkInt( parent, CHAR_WORKPARTYINDEX1+i );
			if( CHAR_CHECKINDEX( subindex ) == FALSE )continue;
			// パーティ全員ワープ
			CHAR_warpToSpecificPoint( subindex, fl, x, y );
		}
	}



}

#if 1
static void NPC_BodyLan_Window(
	int meindex,
	int talkerindex,
	int mode
)
{
	char token[1024];
	char escapedname[2048];
	char szArg[4096];
	char szP[256];
	int fd;
	int buttontype = 0, windowtype = 0, windowno = 0;

	if( CHAR_CHECKINDEX( talkerindex ) == FALSE )return;
	fd = getfdFromCharaIndex( talkerindex );

	// 引数文字列
    if( NPC_Util_GetArgStr( meindex, szArg, sizeof( szArg ) ) == NULL ){
    	print( "npc_bodylan.c:沒有引數(%s)\n",
    		CHAR_getChar(meindex,CHAR_NPCARGUMENT) );
    	return ;
    }

	szP[0] = 0;

	switch( mode ){
	  case BODYLAN_WIN_FIRST:
		// 最初に話された場合のセリフ
		if( NPC_Util_GetStrFromStrWithDelim( szArg, "First", szP, sizeof( szP ) ) == NULL ){
			print( "npc_bodylan:一開始講話的文字沒有輸入(%s)\n",	szArg );
			return;
		}
		sprintf( token,"%s", szP );
		buttontype=WINDOW_BUTTONTYPE_YES|WINDOW_BUTTONTYPE_NO;
	  	windowtype=WINDOW_MESSAGETYPE_MESSAGE;
	  	windowno=mode;
	  	break;

	  case BODYLAN_WIN_LAST_GOOD:
		// 最後で正解だったら
		if( NPC_Util_GetStrFromStrWithDelim( szArg, "Good", szP, sizeof( szP ) ) == NULL ){
			print( "npc_bodylan:答對時說的文字沒有輸入(%s)\n",	szArg );
			return;
		}
		sprintf( token,"%s", szP );
		buttontype=WINDOW_BUTTONTYPE_YESNO;// YES|NO
	  	windowtype=WINDOW_MESSAGETYPE_MESSAGE;
	  	windowno=mode;
	  	break;

	  case BODYLAN_WIN_LAST_NG:
		// 最後で正解だったら
		if( NPC_Util_GetStrFromStrWithDelim( szArg, "Ng", szP, sizeof( szP ) ) == NULL ){
			print( "npc_bodylan:答錯時說的文字沒有輸入(%s)\n",	szArg );
			return;
		}
		sprintf( token,"%s", szP );
		buttontype=WINDOW_BUTTONTYPE_OK;// OK
	  	windowtype=WINDOW_MESSAGETYPE_MESSAGE;
	  	windowno=mode;
	  	break;

	  case BODYLAN_WIN_GOOD_NO:
		// 正解時に褒美をキャンセル
		if( NPC_Util_GetStrFromStrWithDelim( szArg, "Good_No", szP, sizeof( szP ) ) == NULL ){
			print( "npc_bodylan:答對時取消獎品的文字沒有輸入(%s)\n",	szArg );
			return;
		}
		sprintf( token,"%s", szP );
		buttontype=WINDOW_BUTTONTYPE_OK;// OK
	  	windowtype=WINDOW_MESSAGETYPE_MESSAGE;
	  	windowno=mode;
	  	break;

	  case BODYLAN_WIN_ALREADY:
		// すでにイベントを終了していた場合
		if( NPC_Util_GetStrFromStrWithDelim( szArg, "Good_No", szP, sizeof( szP ) ) == NULL ){
			print( "npc_bodylan:事件結束時的文字沒有輸入(%s)\n",	szArg );
			return;
		}
		sprintf( token,"%s", szP );
		buttontype=WINDOW_BUTTONTYPE_OK;	// OK
	  	windowtype=WINDOW_MESSAGETYPE_MESSAGE;
	  	windowno=mode;
	  	break;

	case BODYLAN_WIN_NOT_PREEVENT:
		// 事前に必要なイベントをこなしていない場合
		if( NPC_Util_GetStrFromStrWithDelim( szArg, "Pre_Not", szP, sizeof( szP ) ) == NULL ){
			print( "npc_bodylan:?事前事件結束的文字沒有輸入(%s)\n",	szArg );
			return;
		}
		sprintf( token,"%s", szP );
		buttontype=WINDOW_BUTTONTYPE_OK;	// OK
	  	windowtype=WINDOW_MESSAGETYPE_MESSAGE;
	  	windowno=mode;
	  	break;
	  default:
	  	return;
	}

	makeEscapeString( token, escapedname, sizeof(escapedname));
	/*-ここで送信する--*/
	lssproto_WN_send( fd, windowtype,
					buttontype,
					windowno,
					CHAR_getWorkInt( meindex, CHAR_WORKOBJINDEX),
					escapedname);


}



/*-----------------------------------------
クライアントから返ってきた時に呼び出される。
-------------------------------------------*/
void NPC_BodyLanWindowTalked(
	int meindex,
	int talkerindex,
	int seqno,
	int select,
	char *data
)
{


	if( NPC_Util_CharDistance( talkerindex, meindex ) > 2) return;

	switch( seqno){
	case BODYLAN_WIN_LAST_GOOD:	// 正解時にOK押されたら
		if(select==WINDOW_BUTTONTYPE_YES ){
			NPC_BodyLan_Profit( meindex, talkerindex );
			// プレイヤーに自分のインデックスを忘れさせる
			CHAR_setWorkInt( talkerindex, CHAR_WORKTRADER, -1 );
			// シーケンスは最初からに初期化
			CHAR_setWorkInt( talkerindex, CHAR_WORKSHOPRELEVANT, 1 );
		}else
		if( select == WINDOW_BUTTONTYPE_NO ){
			// 正解時にキャンセルされたら
			NPC_BodyLan_Window( meindex, talkerindex, BODYLAN_WIN_GOOD_NO );
			// プレイヤーに自分のインデックスを忘れさせる
			CHAR_setWorkInt( talkerindex, CHAR_WORKTRADER, -1 );
			// シーケンスは最初からに初期化
			CHAR_setWorkInt( talkerindex, CHAR_WORKSHOPRELEVANT, 1 );
		}
		break;
	default:
		break;
	}

}



#endif

