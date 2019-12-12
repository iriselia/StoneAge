#include "version.h"
#include <string.h>
#include "char.h"
#include "object.h"
#include "char_base.h"
#include "npcutil.h"
#include "lssproto_serv.h"
#include "npc_luckyman.h"


static void NPC_LuckyMan_selectWindow( int meindex, int toindex, int num,char *msg);
void NPC_LuckyManAllHeal( int talker,int mode );
BOOL NPC_LuckyManLevelCheck(int meindex,int talker);
void NPC_LuckyDisp(int meindex,int talker);
BOOL NPC_LuckyCostCheck(int meindex,int talker,int cost);
int NPC_GetMoney(int meindex,int talker,char *buf);



/*********************************
* 初期処理
*********************************/
BOOL NPC_LuckyManInit( int meindex )
{

	/*--キャラのタイプを設定--**/
    CHAR_setInt( meindex , CHAR_WHICHTYPE , CHAR_TYPELUCKYMAN );

	return TRUE;
}




/*********************************
* 話しかけられた時の処理
*********************************/
void NPC_LuckyManTalked( int meindex , int talkerindex , char *szMes ,int color )
{

	char argstr[NPC_UTIL_GETARGSTR_BUFSIZE];
	char buf[256];
	char token[512];
	int cost;



	/*--目の前にいるかどうか？--*/
	if(NPC_Util_isFaceToFace( meindex ,talkerindex ,2) == FALSE) {
		/* １グリッド以内のみ */
		if(NPC_Util_isFaceToChara( talkerindex, meindex, 1) == FALSE) return;
	}
	
	/*--設定ファイルを読み込み--*/
	if(NPC_Util_GetArgStr( meindex, argstr, sizeof( argstr)) == NULL) {
		print("NPC_ExChange.c TypeCheck: GetArgStrErr\n");
		print("NPCName=%s\n", CHAR_getChar( meindex, CHAR_NAME));
		return;
	}


	NPC_Util_GetStrFromStrWithDelim( argstr,"Stone", buf, sizeof( buf));
	cost = NPC_GetMoney( meindex, talkerindex, buf);

	NPC_Util_GetStrFromStrWithDelim( argstr,"main_msg", buf, sizeof( buf));
	sprintf( token, buf, cost);
	
	NPC_LuckyMan_selectWindow( meindex, talkerindex, 2, token);

}


static void NPC_LuckyMan_selectWindow( int meindex, int toindex, int num,char *msg)
{

	int fd = getfdFromCharaIndex( toindex);
	char token[1024];
	int buttontype = 0,windowtype = 0,windowno = 0;


	switch( num){
		case 1:
		sprintf(token,
			"�@�@�@�@�@�@�@☆今日的運勢☆�@"
			"\n\n%s", msg
		);
				buttontype = WINDOW_BUTTONTYPE_OK;
				windowtype = WINDOW_MESSAGETYPE_MESSAGE;
				windowno = CHAR_WINDOWTYPE_WINDOWPETSHOP_START; 
		break;
	
		case 2:
		sprintf(token,
			"�@�@�@�@�@�@�@�@☆占卜師☆�@"
			"\n\n%s", msg
		);
				buttontype = WINDOW_BUTTONTYPE_YESNO;
				windowtype = WINDOW_MESSAGETYPE_MESSAGE;
				windowno = CHAR_WINDOWTYPE_WINDOWPETSHOP_START; 
		break;

		case 3:
		sprintf(token,
			"�@�@�@�@�@�@�@�@☆占卜師☆�@"
			"\n\n%s", msg
		);
				buttontype = WINDOW_BUTTONTYPE_OK;
				windowtype = WINDOW_MESSAGETYPE_MESSAGE;
				windowno = CHAR_WINDOWTYPE_WINDOWPETSHOP_START; 
		break;
	}


	/*--送信--*/
	lssproto_WN_send( fd, windowtype, 
				buttontype, 
				windowno,
				CHAR_getWorkInt( meindex, CHAR_WORKOBJINDEX),
				token);


}	


/*-----------------------------------------
クライアントから返ってきた時に呼び出される。
-------------------------------------------*/
void NPC_LuckyManWindowTalked( int meindex, int talkerindex, 
								int seqno, int select, char *data)
{
	char argstr[NPC_UTIL_GETARGSTR_BUFSIZE];
	char buf[256];
	int money;
	int level;
	
	/*--設定ファイルを読み込み--*/
	if(NPC_Util_GetArgStr( meindex, argstr, sizeof(argstr)) == NULL) {
		print("NPC_ExChange.c TypeCheck: GetArgStrErr\n");
		print("NPCName=%s\n", CHAR_getChar(meindex,CHAR_NAME));
		return;
	}

	switch( seqno){
		case CHAR_WINDOWTYPE_WINDOWPETSHOP_START:
			if(select == WINDOW_BUTTONTYPE_YES) {
				NPC_Util_GetStrFromStrWithDelim( argstr, "Stone", buf,sizeof(buf));
			
				if(strstr(buf,"LV") != NULL) {
					char buff2[32];
					level = CHAR_getInt( talkerindex, CHAR_LV);
					getStringFromIndexWithDelim( buf, "*", 2, buff2,sizeof( buff2));
					money = level * atoi( buff2);

					if(NPC_LuckyCostCheck(meindex,talkerindex,money) == FALSE) {
						NPC_Util_GetStrFromStrWithDelim( argstr,"NoMoney", buf, sizeof( buf));
						NPC_LuckyMan_selectWindow( meindex, talkerindex, 3, buf);
						return;
					}
				}else{
					money = atoi( buf);
					if(NPC_LuckyCostCheck(meindex,talkerindex ,money) == FALSE) {
						NPC_Util_GetStrFromStrWithDelim( argstr,"NoMoney", buf,sizeof( buf));
						NPC_LuckyMan_selectWindow( meindex, talkerindex, 3, buf);
						return;
					}
				}

				CHAR_DelGold( talkerindex, money );

				CHAR_send_P_StatusString( talkerindex, CHAR_P_STRING_GOLD);
				NPC_LuckyDisp( meindex, talkerindex);

			}else if(select == WINDOW_BUTTONTYPE_OK) {

			}
		break;
	}
}



void NPC_LuckyDisp(int meindex,int talker)
{
	char argstr[NPC_UTIL_GETARGSTR_BUFSIZE];
	char buf[16];
	char token[1024];
	char buf2[512];
	int i = 1;

	/*--設定ファイルを読み込み--*/
	if(NPC_Util_GetArgStr( meindex, argstr, sizeof( argstr)) == NULL) {
		print("NPC_Savepoint.c Init: GetArgStrErr");
		return;
	}

	/*--運を見る--*/
	sprintf( buf, "luck%d", CHAR_getInt( talker, CHAR_LUCK));
	NPC_Util_GetStrFromStrWithDelim( argstr, buf, buf2, sizeof( buf2));

	while(getStringFromIndexWithDelim( buf2, ",", i, token, sizeof( token))
	!= FALSE)
	{
		i++;
	}
	i--;
	i = rand()%i + 1;

	/*--ランダムでメッセージを表示させる--*/
	getStringFromIndexWithDelim( buf2,",", i, token, sizeof( token));
	NPC_LuckyMan_selectWindow( meindex, talker, 1, token);
}


/*---お金のチェック-**/
BOOL NPC_LuckyCostCheck(int meindex,int talker,int cost)
{
	/*--現在は仮設定---*/
	/*---お金が足りるかどうかのチェック---*/
	if(CHAR_getInt( talker, CHAR_GOLD) < cost) {
		return FALSE;
	}
	return TRUE;

}


int NPC_GetMoney(int meindex,int talker,char *buf)
{
	int level;
	int money;
	
	if(strstr(buf,"LV") != NULL) {
		char buff2[32];
		level = CHAR_getInt(talker,CHAR_LV);
		getStringFromIndexWithDelim( buf, "*" ,2, buff2,sizeof( buff2));
		money = level * atoi( buff2);
	}else{
		money = atoi( buf);
	}
	return money;

}
