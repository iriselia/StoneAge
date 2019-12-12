#include "version.h"
#include <string.h>
#include "char.h"
#include "object.h"
#include "char_base.h"
#include "npcutil.h"
#include "lssproto_serv.h"
#include "npc_charm.h"

//魅力代を導く計算式は

//レベル＊ＲＡＴＥ＊（現在の魅力/ＷＡＲＵ）

//回復量は　５です。
/*
#define RATE  4		//レート？
#define CHARMHEAL 5 //魅力の回復量
#define WARU	3	//魅力を割る値

*/

#define RATE  10	//レート？
#define CHARMHEAL 5 //魅力の回復量
#define WARU	3	//魅力を割る値


static void NPC_Charm_selectWindow( int meindex, int toindex, int num);
int NPC_CharmCost(int meindex,int talker);
void NPC_CharmUp(int meindex,int talker);


/*********************************
* 初期処理
*********************************/
BOOL NPC_CharmInit( int meindex )
{
	/*--キャラのタイプを設定--*/
    CHAR_setInt( meindex , CHAR_WHICHTYPE , CHAR_TYPECHARM );
	return TRUE;

}


/*********************************
* 話しかけられた時の処理
*********************************/
void NPC_CharmTalked( int meindex , int talkerindex , char *szMes ,int color )
{

    /* プレイヤーに対してだけ反応する */
    if( CHAR_getInt( talkerindex , CHAR_WHICHTYPE ) != CHAR_TYPEPLAYER )
    {
    	return;
    }
	
	/*--目の前にいるかどうか？--*/
	if(NPC_Util_isFaceToFace( meindex ,talkerindex , 2) == FALSE) {
		/* １グリッド以内のみ */
		if(NPC_Util_isFaceToChara( talkerindex, meindex, 1) == FALSE) return;
	}

	NPC_Charm_selectWindow( meindex, talkerindex, 0);
}


/*
 * 各処理に分ける
 */
static void NPC_Charm_selectWindow( int meindex, int toindex, int num)
{

	char token[1024];
	char escapedname[1024];
	int fd = getfdFromCharaIndex( toindex);
	int buttontype = 0;
	int windowtype = 0;
	int windowno = 0;
	int cost = 0;
	int chartype;
	
	/*--ウインドウタイプメッセージがおおいので先に設定--*/
  	windowtype = WINDOW_MESSAGETYPE_MESSAGE;

	switch( num) {
		case 0:
		sprintf(token,
			"4\n �@�@�@   = = 美容師 = = "
			"\n\n �@歡迎光臨，今天要做什麼呢？"
			"\n "
			"\n\n�@�@�@�@《�@給我魅力�@》 "
			"\n\n�@�@�@�@《 什麼也不做 》 ");


		buttontype = WINDOW_BUTTONTYPE_NONE;
		windowtype = WINDOW_MESSAGETYPE_SELECT;
		windowno = CHAR_WINDOWTYPE_CHARM_START;
		break;

	case 1:
		cost = NPC_CharmCost( meindex, toindex);
		if(cost == -1){
			sprintf(token,
				"�@�@�@�@     = = 美容師 = =  "
				"\n\n�@�@�@�@你的魅力真是完美"
				"\n\n�@�@�@請永遠保持你的魅力唷。"
			);

		  	buttontype = WINDOW_BUTTONTYPE_OK;
		}else{
			sprintf(token,
				"�@�@�@�@     = = 美容師 = =  "
				"\n�@�幘{在,千禧特惠價實施中��"
				"\n\n 要將你的魅力上升五點的話"
				"\n�@�@�@�@需要%6d的stone唷！"
				"\n\n�@�@�@即使這樣也可以嗎？", cost
			);
		  	buttontype = WINDOW_BUTTONTYPE_YESNO;

		}
	  	windowtype = WINDOW_MESSAGETYPE_MESSAGE;
	  	windowno = CHAR_WINDOWTYPE_CHARM_END; 

		break;

	case 2:
		cost = NPC_CharmCost( meindex, toindex);
		chartype = CHAR_getInt( toindex, CHAR_IMAGETYPE);
		
		if(cost > CHAR_getInt( toindex, CHAR_GOLD)) {
			sprintf(token,
				"�@�@�@�@     = = 美容師 = =  "
				"\n\n�@�@ 哎呀!你的錢不夠唷！"
				"\n�@�@    先去存好錢下次再來吧！"
			);

		}else{
			NPC_CharmUp( meindex, toindex);

			/*--キャラのタイプによってメッセージを変えてみた--*/
			switch( chartype) {
			  case CHAR_IMAGETYPE_GIRL:
			  sprintf(token,
				  "�@�@�@�@     = = 美容師 = =  "
				  "\n\n �@嗯,這樣你的魅力就上升囉！"
				  "\n\n�@�@�@愈來愈可愛了呢！"
			  );

				break;
			  case CHAR_IMAGETYPE_BOY:
			  sprintf(token,
				  "�@�@�@�@     = = 美容師 = =  "
				  "\n\n �@嗯,這樣你的魅力就上升囉！"
				  "\n\n�@�@是不是比剛才要帥多了呢？"
			  );
			  	break;
			  	
			  case CHAR_IMAGETYPE_CHILDBOY:
			  case CHAR_IMAGETYPE_CHILDGIRL:
			  sprintf(token,
				  "�@�@�@�@     = = 美容師 = =  "
				  "\n\n �@嗯,這樣你的魅力就上升囉！"
				  "\n\n�@�@  真的變得好可愛唷！"
			  );
			 	break;
			 	
			   case CHAR_IMAGETYPE_MAN:
			   sprintf(token,
				   "�@�@�@�@     = = 美容師 = =  "
				   "\n\n �@嗯,這樣你的魅力就上升囉！"
				   "\n\n �@�@�@變得更酷了呢！"
			   );
			 	break;
			 	
			   case CHAR_IMAGETYPE_WOMAN:
			   sprintf(token,
				   "�@�@�@�@     = = 美容師 = =  "
				   "\n\n �@嗯,這樣你的魅力就上升囉！"
				   "\n\n    可真是變得愈來愈美了呢！"
			   );
			 	break;
			 
			 }
		}

		buttontype = WINDOW_BUTTONTYPE_OK;
		windowtype = WINDOW_MESSAGETYPE_MESSAGE;
	  	windowno = CHAR_WINDOWTYPE_CHARM_END; 
		break;
	}
	
	makeEscapeString( token, escapedname, sizeof( escapedname));
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
void NPC_CharmWindowTalked( int meindex, int talkerindex, 
								int seqno, int select, char *data)
{
	if( NPC_Util_CharDistance( talkerindex, meindex ) > 2) return;

	switch( seqno){
	  case CHAR_WINDOWTYPE_CHARM_START:
	  	if(atoi( data) == 2) {
			NPC_Charm_selectWindow( meindex, talkerindex, 1 );
		}
		break;

	  case CHAR_WINDOWTYPE_CHARM_END:
	  	if(select == WINDOW_BUTTONTYPE_YES) {
			NPC_Charm_selectWindow( meindex, talkerindex, 2 );
		}
		break;
	}
	
}



/*--魅力ＵＰ--*/
void NPC_CharmUp(int meindex,int talker)
{
	int cost;
	int i;
	int petindex;
	char petsend[64];	

	/*--お金を減らしましょう--*/
	cost = NPC_CharmCost( meindex, talker);
	CHAR_setInt( talker, CHAR_GOLD,
			CHAR_getInt( talker, CHAR_GOLD) - cost);
	CHAR_send_P_StatusString( talker, CHAR_P_STRING_GOLD);

	/*--魅力が１００以上になる場合は強引に１００にする--*/
	if(CHAR_getInt( talker, CHAR_CHARM) + CHARMHEAL >= 100) {
		CHAR_setInt( talker, CHAR_CHARM, 100);
	}else{
		/*--魅力をセット--*/
		CHAR_setInt(talker, CHAR_CHARM,
	 			(CHAR_getInt( talker, CHAR_CHARM) + CHARMHEAL));
	}
	
	/*--ステータスの更新--*/
	CHAR_complianceParameter( talker );
	CHAR_send_P_StatusString( talker, CHAR_P_STRING_CHARM);


	/*--ペットのパラメータを更新--*/
	for( i = 0 ; i < CHAR_MAXPETHAVE ; i++){
    	petindex = CHAR_getCharPet( talker, i);

		if( petindex == -1  )  continue;

	   /*  キャラの配列チェック    */
		if( !CHAR_CHECKINDEX( talker ) )  continue;

		/*--パラメータ調整--*/
		CHAR_complianceParameter( petindex );
		sprintf( petsend, "K%d", i );
		CHAR_sendStatusString( talker , petsend );
	}
}


/*--お金の計算--*/
int NPC_CharmCost(int meindex,int talker)
{
	int cost;
	int level;
	int charm;
	int trans;

	level = CHAR_getInt( talker, CHAR_LV);
	charm = CHAR_getInt( talker, CHAR_CHARM);
	trans = CHAR_getInt( talker, CHAR_TRANSMIGRATION);

	if(charm >= 100) return -1;
	
	if(charm <= 1) charm = WARU;
	
	/*-- 計算式 --*/
	cost = level * RATE * (charm / WARU) * (trans+1);

	return cost;

}
